// Copyright 2014 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// - Modified to accept float** instead of FloatFrame
// - Converted to procudral programming style
//
// Reverb.

#pragma once

#include <clouds/dsp/fx/fx_engine.h>
#include <stmlib/stmlib.h>
#include <vector>

struct clouds_reverb {
	typedef clouds::FxEngine<16384, clouds::FORMAT_12_BIT> E;
	E engine_;

	const float amount_ = 1.f; // fully wet
	float input_gain_ = 0.2f;
	float reverb_time_ = 0.25f;
	float diffusion_ = 0.7f;
	float lp_ = 0.5f;

	float lp_decay_1_;
	float lp_decay_2_;

	// DISALLOW_COPY_AND_ASSIGN(clouds_reverb);
};

inline void clouds_reverb_init(clouds_reverb& r, uint16_t* buffer)
{
	r.engine_.Init(buffer);
	r.engine_.SetLFOFrequency(clouds::LFO_1, 0.5f / 32000.0f);
	r.engine_.SetLFOFrequency(clouds::LFO_2, 0.3f / 32000.0f);
	r.lp_ = 0.7f;
	r.diffusion_ = 0.625f;
}

inline void clouds_reverb_process(clouds_reverb& r, std::vector<float*> in_out,
								  size_t block_size)
{
	// This is the Griesinger topology described in the Dattorro paper
	// (4 AP diffusers on the input, then a loop of 2x 2AP+1Delay).
	// Modulation is applied in the loop of the first diffuser AP for additional
	// smearing; and to the two long delays for a slow shimmer/chorus effect.
	typedef clouds_reverb::E::Reserve<
		113,
		clouds_reverb::E::Reserve<
			162,
			clouds_reverb::E::Reserve<
				241,
				clouds_reverb::E::Reserve<
					399,
					clouds_reverb::E::Reserve<
						1653, clouds_reverb::E::Reserve<
								  2038, clouds_reverb::E::Reserve<
											3411, clouds_reverb::E::Reserve<
													  1913, clouds_reverb::E::Reserve<
																1663,
																clouds_reverb::E::Reserve<
																	4782>>>>>>>>>>
		Memory;
	clouds_reverb::E::DelayLine<Memory, 0> ap1;
	clouds_reverb::E::DelayLine<Memory, 1> ap2;
	clouds_reverb::E::DelayLine<Memory, 2> ap3;
	clouds_reverb::E::DelayLine<Memory, 3> ap4;
	clouds_reverb::E::DelayLine<Memory, 4> dap1a;
	clouds_reverb::E::DelayLine<Memory, 5> dap1b;
	clouds_reverb::E::DelayLine<Memory, 6> del1;
	clouds_reverb::E::DelayLine<Memory, 7> dap2a;
	clouds_reverb::E::DelayLine<Memory, 8> dap2b;
	clouds_reverb::E::DelayLine<Memory, 9> del2;
	clouds_reverb::E::Context c;

	const float kap = r.diffusion_;
	const float klp = r.lp_;
	const float krt = r.reverb_time_;
	const float amount = r.amount_;
	const float gain = r.input_gain_;

	float lp_1 = r.lp_decay_1_;
	float lp_2 = r.lp_decay_2_;

	for (int i = 0; i < block_size; i++) {
		float wet;
		float apout = 0.0f;
		r.engine_.Start(&c);

		// Smear AP1 inside the loop.
		c.Interpolate(ap1, 10.0f, clouds::LFO_1, 60.0f, 1.0f);
		c.Write(ap1, 100, 0.0f);

		c.Read(in_out[0][i] + in_out[1][i], gain);

		// Diffuse through 4 allpasses.
		c.Read(ap1 TAIL, kap);
		c.WriteAllPass(ap1, -kap);
		c.Read(ap2 TAIL, kap);
		c.WriteAllPass(ap2, -kap);
		c.Read(ap3 TAIL, kap);
		c.WriteAllPass(ap3, -kap);
		c.Read(ap4 TAIL, kap);
		c.WriteAllPass(ap4, -kap);
		c.Write(apout);

		// Main reverb loop.
		c.Load(apout);
		c.Interpolate(del2, 4680.0f, clouds::LFO_2, 100.0f, krt);
		c.Lp(lp_1, klp);
		c.Read(dap1a TAIL, -kap);
		c.WriteAllPass(dap1a, kap);
		c.Read(dap1b TAIL, kap);
		c.WriteAllPass(dap1b, -kap);
		c.Write(del1, 2.0f);
		c.Write(wet, 0.0f);

		in_out[0][i] += (wet - in_out[0][i]) * amount;

		c.Load(apout);
		// c.Interpolate(del1, 4450.0f, LFO_1, 50.0f, krt);
		c.Read(del1 TAIL, krt);
		c.Lp(lp_2, klp);
		c.Read(dap2a TAIL, kap);
		c.WriteAllPass(dap2a, -kap);
		c.Read(dap2b TAIL, -kap);
		c.WriteAllPass(dap2b, kap);
		c.Write(del2, 2.0f);
		c.Write(wet, 0.0f);

		in_out[1][i] += (wet - in_out[1][i]) * amount;
	}

	r.lp_decay_1_ = lp_1;
	r.lp_decay_2_ = lp_2;
}
