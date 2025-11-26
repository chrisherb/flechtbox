#include "dsp.hpp"
#include "clock.hpp"
#include "sequencer.hpp"
#include <array>

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

	p.patch.engine = 8;
	p.patch.note = 48.0f;
	p.patch.harmonics = 0.5f;
	p.patch.timbre = 0.5f;
	p.patch.morph = 0.5f;
	p.patch.frequency_modulation_amount = 0.0f;
	p.patch.timbre_modulation_amount = 0.0f;
	p.patch.morph_modulation_amount = 0.0f;
	p.patch.lpg_colour = 0.5f;
	p.patch.decay = 0.5f;

	p.modulations.engine = 0;
	p.modulations.frequency = 0;
	p.modulations.frequency_patched = false;
	p.modulations.harmonics = 0;
	p.modulations.level = 0;
	p.modulations.level_patched = false;
	p.modulations.morph = 0;
	p.modulations.morph_patched = false;
	p.modulations.note = 0;
	p.modulations.timbre = 0;
	p.modulations.timbre_patched = false;
	p.modulations.trigger = 0;
	p.modulations.trigger_patched = true;
	p.modulations.sustain_level = 0;

	track_seq_init(p.sequencer);
}

bool rand_bool(int probability)
{
	// Generate a random number from 0 to 99
	int randomValue = rand() % 100;

	// Return true if random value is less than the probability
	return randomValue < probability;
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
		for (int t = 0; t < NUM_TRACKS; t++) {
			auto& p = dsp->tracks[t];

			if (!p.enabled) continue;

			int step_probability =
				track_seq_process_step(p.sequencer, dsp->clock.cur_clock_states);

			if (!p.muted && rand_bool(step_probability)) {

				p.patch.note = p.pitch;
				if (p.global_pitch_enabled) p.patch.note += global_pitch;
				if (p.global_octave_enabled) p.patch.note += global_octave;
				if (p.global_velocity_enabled)
					p.current_velocity = global_velocity / 100.f;
				else p.current_velocity = 1.f;
				p.modulations.trigger = 1.f;
			}

			p.voice->Render(p.patch, p.modulations, p.frames, PLAITS_BLOCKSIZE);

			p.modulations.trigger = 0.f;
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
