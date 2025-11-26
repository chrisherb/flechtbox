#include "dsp.hpp"
#include "clock.hpp"
#include "sequencer.hpp"
#include <array>
#include <cstdlib>
#include <random>

float plaits::kSampleRate = SAMPLERATE;
float plaits::kCorrectedSampleRate = SAMPLERATE;
float plaits::a0 = (440.0f / 8.0f) / plaits::kCorrectedSampleRate;

void dsp_init(std::shared_ptr<flechtbox_dsp> dsp)
{
	dsp->clock.samplerate = SAMPLERATE;

	for (int i = 0; i < NUM_TRACKS; i++) { plaits_voice_init(dsp->tracks[i]); }

	track_seq_init(dsp->pitch_sequence);
	track_seq_init(dsp->octave_sequence);
	dsp->velocity_sequence.data.fill(100);
}

void plaits_voice_init(flechtbox_track& p)
{
	p.frames = new plaits::Voice::Frame[16];
	p.voice = new plaits::Voice();
	p.shared_buffer = new char[PLAITS_BLOCKSIZE * 1024];

	stmlib::BufferAllocator allocator(p.shared_buffer, PLAITS_BLOCKSIZE * 1024);
	p.voice->Init(&allocator);

	p.plaits_patch.engine = 8;
	p.plaits_patch.note = 48.0f;
	p.plaits_patch.harmonics = 0.5f;
	p.plaits_patch.timbre = 0.5f;
	p.plaits_patch.morph = 0.5f;
	p.plaits_patch.frequency_modulation_amount = 0.0f;
	p.plaits_patch.timbre_modulation_amount = 0.0f;
	p.plaits_patch.morph_modulation_amount = 0.0f;
	p.plaits_patch.lpg_colour = 0.5f;
	p.plaits_patch.decay = 0.5f;

	p.plaits_mods.engine = 0;
	p.plaits_mods.frequency = 0;
	p.plaits_mods.frequency_patched = false;
	p.plaits_mods.harmonics = 0;
	p.plaits_mods.level = 0;
	p.plaits_mods.level_patched = false;
	p.plaits_mods.morph = 0;
	p.plaits_mods.morph_patched = false;
	p.plaits_mods.note = 0;
	p.plaits_mods.timbre = 0;
	p.plaits_mods.timbre_patched = false;
	p.plaits_mods.trigger = 0;
	p.plaits_mods.trigger_patched = true;
	p.plaits_mods.sustain_level = 0;

	track_seq_init(p.sequencer);
}

bool rand_bool(int probability)
{
	// Generate a random number from 0 to 99
	int randomValue = rand() % 100;

	// Return true if random value is less than the probability
	return randomValue < probability;
}

float randf(float amount, float min = -0.5f, float max = 0.5f)
{
	if (amount > 0.f) {
		static thread_local std::mt19937 rng(std::random_device {}());
		std::uniform_real_distribution<float> dist(min, max);
		return dist(rng) * amount;
	}
	return 0.f;
}

void dsp_process_block(std::shared_ptr<flechtbox_dsp> dsp, float* out, int frames)
{
	// clear audio
	std::fill(out, out + frames * 2, 0.0f);

	// convert from internal block size to whatever size the host is running
	for (int block_count = 0; block_count < frames; block_count += PLAITS_BLOCKSIZE) {
		// clock states for this block
		clock_process_block(dsp->clock, PLAITS_BLOCKSIZE);

		track_seq_process_step(dsp->pitch_sequence, dsp->clock.cur_clock_states);
		track_seq_process_step(dsp->octave_sequence, dsp->clock.cur_clock_states);
		track_seq_process_step(dsp->velocity_sequence, dsp->clock.cur_clock_states);

		int global_pitch = dsp->pitch_sequence.last_value;
		int global_octave = dsp->octave_sequence.last_value;
		int global_velocity = dsp->velocity_sequence.last_value;

		// render all tracks
		for (int i = 0; i < NUM_TRACKS; i++) {
			auto& t = dsp->tracks[i];

			if (!t.enabled) continue;

			int step_probability =
				track_seq_process_step(t.sequencer, dsp->clock.cur_clock_states);

			// TRIGGERED
			if (!t.muted && rand_bool(step_probability)) {

				// generate random numbers
				t.harmonics_rand_val = randf(t.harmonics_rand_amt);
				t.timbre_rand_val = randf(t.timbre_rand_amt);
				t.morph_rand_val = randf(t.morph_rand_amt);

				// apply global parameters
				t.plaits_patch.note = t.pitch;
				if (t.global_pitch_enabled) t.plaits_patch.note += global_pitch;
				if (t.global_octave_enabled) t.plaits_patch.note += global_octave;
				if (t.global_velocity_enabled)
					t.current_velocity = global_velocity / 100.f;
				else t.current_velocity = 1.f;
				t.plaits_mods.trigger = 1.f;
			}

			// update plaits patch
			t.plaits_patch.harmonics = t.harmonics + t.harmonics_rand_val;
			t.plaits_patch.timbre = t.timbre + t.timbre_rand_val;
			t.plaits_patch.morph = t.morph + t.morph_rand_val;

			t.voice->Render(t.plaits_patch, t.plaits_mods, t.frames, PLAITS_BLOCKSIZE);

			t.plaits_mods.trigger = 0.f;
		}

		// print voices to output
		for (int i = 0; i < PLAITS_BLOCKSIZE; i++) {
			float voice_out = 0.f;
			for (int t = 0; t < NUM_TRACKS; t++) {
				auto& p = dsp->tracks[t];
				voice_out += p.frames[i].out / 32768.0f * p.current_velocity * p.volume;
				// float voice_aux = p.frames[i].aux / 32768.0f;
			}
			*out++ = soft_clip(voice_out); /* left */
			*out++ = soft_clip(voice_out); /* right */
		}
	}
}
