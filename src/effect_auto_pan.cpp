/*
   Copyright (c) 2019-2021 H. Wirtz

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#include <Arduino.h>
#include <Audio.h>
#include "effect_auto_pan.h"

/*************************************************************************/
//                A u d i o E f f e c t A u t o P a n
// Written by Holger Wirtz
// 20191205 - initial version

void AudioEffectAutoPan::update(void)
{
  audio_block_t *in;
  audio_block_t *mod;
  audio_block_t *out[2];

  in = receiveReadOnly(0);
  mod = receiveReadOnly(1);

  out[0] = allocate();
  out[1] = allocate();

  if (in && mod && out[0] && out[1])
  {
    arm_q15_to_float(in->data, in_f, AUDIO_BLOCK_SAMPLES);
    arm_q15_to_float(mod->data, pan_f, AUDIO_BLOCK_SAMPLES);
    arm_offset_f32(pan_f, 1.0, pan_f, AUDIO_BLOCK_SAMPLES);
    arm_scale_f32(pan_f, PI / 4.0, pan_f, AUDIO_BLOCK_SAMPLES); // PI/4

    // right
    for (uint8_t n = 0; n < AUDIO_BLOCK_SAMPLES; n++)
      out_f[0][n] = in_f[n] * _pseudo_log * arm_sin_f32(pan_f[n]);
    arm_float_to_q15 (out_f[0], out[0]->data, AUDIO_BLOCK_SAMPLES);
    // left
    for (uint8_t n = 0; n < AUDIO_BLOCK_SAMPLES; n++)
      out_f[1][n] = in_f[n] * _pseudo_log * arm_cos_f32(pan_f[n]);
    arm_float_to_q15 (out_f[1], out[1]->data, AUDIO_BLOCK_SAMPLES);

    if (in)
    {
      release(in);
    }
    if (mod)
    {
      release(mod);
    }
    for (uint8_t i = 0; i < 2; i++)
    {
      if (out[i])
      {
        transmit(out[i], i);
        release(out[i]);
      }
    }
  }
}
