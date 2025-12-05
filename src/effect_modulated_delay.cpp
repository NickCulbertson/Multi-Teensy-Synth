/*
   Copyright (c) 2014, Pete (El Supremo)
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
#include "arm_math.h"
#include "effect_modulated_delay.h"

/******************************************************************/

// Based on;      A u d i o E f f e c t D e l a y
// Written by Pete (El Supremo) Jan 2014
// 140529 - change to handle mono stream - change modify() to voices()
// 140219 - correct storage class (not static)
// 190527 - added modulation input (by Holger Wirtz)

static const audio_block_t zeroblock = {
    0, 0, 0, {
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
#if AUDIO_BLOCK_SAMPLES > 16
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
#endif
#if AUDIO_BLOCK_SAMPLES > 32
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
#endif
#if AUDIO_BLOCK_SAMPLES > 48
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
#endif
#if AUDIO_BLOCK_SAMPLES > 64
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
#endif
#if AUDIO_BLOCK_SAMPLES > 80
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
#endif
#if AUDIO_BLOCK_SAMPLES > 96
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
#endif
#if AUDIO_BLOCK_SAMPLES > 112
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
                 0,
#endif
             }};

boolean AudioEffectModulatedDelay::begin(short *delayline, uint16_t d_length)
{
#if 0
  LOG.print(F("AudioEffectModulatedDelay.begin(modulated-delay line length = "));
  LOG.print(d_length);
  LOG.println(F(")"));
#endif

  _cb_index = 0;

  if (delayline == NULL)
    return (false);
  if (d_length < 10)
    return (false);

  _delayline = delayline;
  _delay_length = d_length;
  memset(_delayline, 0, _delay_length * sizeof(int16_t));
  _delay_offset = _delay_length >> 1;

  return (true);
}

uint16_t AudioEffectModulatedDelay::get_delay_length(void)
{
  return (_delay_length);
}

void AudioEffectModulatedDelay::update(void)
{
  audio_block_t *block;
  audio_block_t *modulation;

  if (_delayline == NULL)
    return;

  block = receiveWritable(0);
  if (!block)
    block = (audio_block_t *)&zeroblock;

  modulation = receiveReadOnly(1);
  if (!modulation)
    modulation = (audio_block_t *)&zeroblock;

  if (bypass == true)
  {
    if (modulation != (audio_block_t *)&zeroblock)
      release(modulation);

    if (block != (audio_block_t *)&zeroblock)
    {
      transmit(block, 0);
      release(block);
    }
    return;
  }

  if (block && modulation)
  {
    int16_t *bp;
    int16_t cb_mod_index_neighbor;
    float *mp;
    float mod_index;
    float mod_number;
    float mod_fraction;
    float modulation_f32[AUDIO_BLOCK_SAMPLES];

    bp = block->data;
    arm_q15_to_float(modulation->data, modulation_f32, AUDIO_BLOCK_SAMPLES);
    mp = modulation_f32;

    for (uint16_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
    {
      // write data into circular buffer (delayline)
      if (_cb_index >= _delay_length)
        _cb_index = 0;
      _delayline[_cb_index] = *bp;

      // calculate the modulation-index as a floating point number for interpolation
      mod_index = *mp * _delay_offset;
      mod_fraction = modff(mod_index, &mod_number); // split float of mod_index into integer (= mod_number) and fraction part

      // calculate modulation index into circular buffer
      cb_mod_index = _cb_index - (_delay_offset + mod_number);
      if (cb_mod_index < 0) // check for negative offsets and correct them
        cb_mod_index += _delay_length;

      if (cb_mod_index == _delay_length - 1)
        cb_mod_index_neighbor = 0;
      else
        cb_mod_index_neighbor = cb_mod_index + 1;

      *bp = round(float(_delayline[cb_mod_index]) * mod_fraction + float(_delayline[cb_mod_index_neighbor]) * (1.0 - mod_fraction));

      // push the pointers forward
      bp++;        // next audio data
      mp++;        // next modulation data
      _cb_index++; // next circular buffer index
    }
  }

  if (modulation != (audio_block_t *)&zeroblock)
    release(modulation);

  if (block != (audio_block_t *)&zeroblock)
  {
    transmit(block, 0);
    release(block);
  }
}

void AudioEffectModulatedDelay::set_bypass(bool b)
{
  bypass = b;
}

bool AudioEffectModulatedDelay::get_bypass(void)
{
  return (bypass);
}

boolean AudioEffectModulatedDelayStereo::begin(short *delayline_l, short *delayline_r, uint16_t d_length)
{
#if 0
  LOG.print(F("AudioEffectModulatedDelayStereo.begin(modulated-delay line length = "));
  LOG.print(d_length);
  LOG.println(F(")"));
#endif

  _cb_index[0] = 0;
  _cb_index[1] = 0;

  if (delayline_r == NULL)
    return (false);
  if (delayline_l == NULL)
    return (false);
  if (d_length < 10)
    return (false);

  _delay_length = d_length;
  _delay_offset = _delay_length >> 1;

  _delayline[0] = delayline_l;
  memset(_delayline[0], 0, _delay_length * sizeof(int16_t));

  _delayline[1] = delayline_r;
  memset(_delayline[1], 0, _delay_length * sizeof(int16_t));

  stereo = true;

  return (true);
}

uint16_t AudioEffectModulatedDelayStereo::get_delay_length(void)
{
  return (_delay_length);
}

void AudioEffectModulatedDelayStereo::update(void)
{
  audio_block_t *block[2];
  audio_block_t *modulation;

  if (_delayline[0] == NULL || _delayline[1] == NULL)
    return;

  block[0] = receiveWritable(0);
  if (!block[0])
    block[0] = (audio_block_t *)&zeroblock;

  block[1] = receiveWritable(1);
  if (!block[1])
    block[1] = (audio_block_t *)&zeroblock;

  modulation = receiveReadOnly(2);
  if (!modulation)
    modulation = (audio_block_t *)&zeroblock;

  if (bypass == true)
  {
    if (modulation != (audio_block_t *)&zeroblock)
      release(modulation);

    if (block[0] != (audio_block_t *)&zeroblock)
    {
      transmit(block[0], 0);
      release(block[0]);
    }

    if (block[1] != (audio_block_t *)&zeroblock)
    {
      transmit(block[1], 1);
      release(block[1]);
    }
    return;
  }

  if (block[0] && block[1] && modulation)
  {
    int16_t *bp[2];
    int16_t cb_mod_index_neighbor[2];
    float *mp;
    float mod_index;
    float mod_number;
    float mod_fraction;
    float modulation_f32[AUDIO_BLOCK_SAMPLES];

    bp[0] = block[0]->data;
    bp[1] = block[1]->data;

    arm_q15_to_float(modulation->data, modulation_f32, AUDIO_BLOCK_SAMPLES);
    mp = modulation_f32;

    for (uint16_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
    {
      // LEFT
      // calculate the modulation-index as a floating point number for interpolation
      mod_index = *mp * _delay_offset;
      mod_fraction = modff(mod_index, &mod_number); // split float of mod_index into integer (= mod_number) and fraction part

      // write data into circular buffer (delayline)
      if (_cb_index[0] >= _delay_length)
        _cb_index[0] = 0;
      _delayline[0][_cb_index[0]] = *bp[0];

      // calculate modulation index into circular buffer
      cb_mod_index[0] = _cb_index[0] - (_delay_offset + mod_number);
      if (cb_mod_index[0] < 0) // check for negative offsets and correct them
        cb_mod_index[0] += _delay_length;

      if (cb_mod_index[0] == _delay_length - 1)
        cb_mod_index_neighbor[0] = 0;
      else
        cb_mod_index_neighbor[0] = cb_mod_index[0] + 1;

      *bp[0] = round(float(_delayline[0][cb_mod_index[0]]) * mod_fraction + float(_delayline[0][cb_mod_index_neighbor[0]]) * (1.0 - mod_fraction));

      // push the pointers forward
      bp[0]++;        // next audio data
      _cb_index[0]++; // next circular buffer index

      // RIGHT
      // calculate the modulation-index as a floating point number for interpolation
      if (stereo == true)
        mod_index *= -1.0;

      mod_fraction = modff(mod_index, &mod_number); // split float of mod_index into integer (= mod_number) and fraction part

      // write data into circular buffer (delayline)
      if (_cb_index[1] >= _delay_length)
        _cb_index[1] = 0;
      _delayline[1][_cb_index[1]] = *bp[1];

      // calculate modulation index into circular buffer
      cb_mod_index[1] = _cb_index[1] - (_delay_offset + mod_number);
      if (cb_mod_index[1] < 0) // check for negative offsets and correct them
        cb_mod_index[1] += _delay_length;

      if (cb_mod_index[1] == _delay_length - 1)
        cb_mod_index_neighbor[1] = 0;
      else
        cb_mod_index_neighbor[1] = cb_mod_index[1] + 1;

      *bp[1] = round(float(_delayline[1][cb_mod_index[1]]) * mod_fraction + float(_delayline[1][cb_mod_index_neighbor[1]]) * (1.0 - mod_fraction));

      // push the pointers forward
      bp[1]++;        // next audio data
      _cb_index[1]++; // next circular buffer index

      mp++; // next modulation data
    }
  }

  if (modulation != (audio_block_t *)&zeroblock)
    release(modulation);

  if (block[0] != (audio_block_t *)&zeroblock)
  {
    transmit(block[0], 0);
    release(block[0]);
  }
  if (block[1] != (audio_block_t *)&zeroblock)
  {
    transmit(block[1], 1);
    release(block[1]);
  }
}

void AudioEffectModulatedDelayStereo::set_stereo(bool s)
{
  stereo = s;
}

bool AudioEffectModulatedDelayStereo::get_stereo(void)
{
  return (stereo);
}

void AudioEffectModulatedDelayStereo::set_bypass(bool b)
{
  bypass = b;
}

bool AudioEffectModulatedDelayStereo::get_bypass(void)
{
  return (bypass);
}
