#include "dsp.hpp"
#include "audio_buffer.h"
#include "clock.hpp"
#include "reverb.hpp"
#include "sequencer.hpp"
#include <array>
#include <cstdlib>
#include <random>

float plaits::kSampleRate = SAMPLERATE;
float plaits::kCorrectedSampleRate = SAMPLERATE;
float plaits::a0 = (440.0f / 8.0f) / plaits::kCorrectedSampleRate;

static constexpr size_t kReverbBufferSize = 16384;
static uint16_t reverb_buffer[kReverbBufferSize] = {0};

void dsp_init(std::shared_ptr<flechtbox_dsp> dsp)
{
	dsp->clock.samplerate = SAMPLERATE;

	for (int i = 0; i < NUM_TRACKS; i++) { flechtbox_track_init(dsp->tracks[i]); }

	track_seq_init(dsp->pitch_sequence);
	track_seq_init(dsp->octave_sequence);
	dsp->velocity_sequence.data.fill(100);

	trnr::audio_buffer_init(dsp->reverb_buffer, 2, PLAITS_BLOCKSIZE);
	trnr::audio_buffer_init(dsp->mix_buffer, 2, PLAITS_BLOCKSIZE);
	clouds_reverb_init(dsp->reverb, reverb_buffer);
}

void flechtbox_track_init(flechtbox_track& p)
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
	if (probability < 100) {
		// Generate a random number from 0 to 99
		static thread_local std::mt19937 rng(std::random_device {}());
		std::uniform_int_distribution<int> dist(0, 100);

		// Return true if random value is less than the probability
		return dist(rng) < probability;
	} else {
		return true;
	}
}

float randf(float amount, float min = -0.5f, float max = 0.5f)
{
	if (amount > 0.f) {
		static thread_local std::mt19937 rng(std::random_device {}());
		std::uniform_real_distribution<float> dist(min, max);
		return dist(rng) * amount;
	} else {
		return 0.f;
	}
}

void dsp_process_block(std::shared_ptr<flechtbox_dsp> dsp, float* out, int block_size)
{
	// if block_size isn't multiple of plaits block size (16), do nothing
	// TODO: show error
	if (block_size % PLAITS_BLOCKSIZE != 0) return;

	// convert from internal block size to whatever size the host is running
	for (int block_count = 0; block_count < block_size; block_count += PLAITS_BLOCKSIZE) {
		// clock states for this block
		auto& clock_state = clock_process_block(dsp->clock, PLAITS_BLOCKSIZE);

		track_seq_process_step(dsp->pitch_sequence, clock_state);
		track_seq_process_step(dsp->octave_sequence, clock_state);
		track_seq_process_step(dsp->velocity_sequence, clock_state);

		int global_pitch = dsp->pitch_sequence.last_value;
		int global_octave = dsp->octave_sequence.last_value;
		int global_velocity = dsp->velocity_sequence.last_value;

		// render all tracks
		for (int i = 0; i < NUM_TRACKS; i++) {
			auto& t = dsp->tracks[i];

			if (!t.enabled) continue;

			int step_probability = track_seq_process_step(t.sequencer, clock_state);

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
			float mix_send = 0.f;
			float reverb_send = 0.f;
			for (int t = 0; t < NUM_TRACKS; t++) {
				auto& track = dsp->tracks[t];
				float voice_out = track.frames[i].out / 32768.0f *
								  track.current_velocity * track.volume;

				mix_send += voice_out;
				reverb_send += voice_out * track.reverb_send_amt;
			}
			dsp->reverb_buffer.channel_ptrs[0][i] = reverb_send;
			dsp->reverb_buffer.channel_ptrs[1][i] = reverb_send;

			dsp->mix_buffer.channel_ptrs[0][i] = mix_send;
			dsp->mix_buffer.channel_ptrs[1][i] = mix_send;
		}

		clouds_reverb_process(dsp->reverb, dsp->reverb_buffer.channel_ptrs,
							  PLAITS_BLOCKSIZE);

		for (int i = 0; i < PLAITS_BLOCKSIZE; i++) {
			// print reverb signal to mix buffer
			dsp->mix_buffer.channel_ptrs[0][i] += dsp->reverb_buffer.channel_ptrs[0][i];
			dsp->mix_buffer.channel_ptrs[1][i] += dsp->reverb_buffer.channel_ptrs[1][i];

			// soft clip mix and write to output
			*out++ = soft_clip(dsp->mix_buffer.channel_ptrs[0][i]); /* left */
			*out++ = soft_clip(dsp->mix_buffer.channel_ptrs[1][i]); /* right */
		}
	}
}
