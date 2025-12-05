/* Audio Library for Teensy 3.X
   Copyright (c) 2018, Paul Stoffregen, paul@pjrc.com

   Development of this audio library was funded by PJRC.COM, LLC by sales of
   Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
   open source software by purchasing Teensy or other PJRC products.

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice, development funding notice, and this permission
   notice shall be included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

// A Floating  point implementation of Freeverb by Jezar at Dreampoint
//  http://blog.bjornroche.com/2012/06/freeverb-original-public-domain-code-by.html
//  https://music.columbia.edu/pipermail/music-dsp/2001-October/045433.html

#include <Arduino.h>
#include "effect_freeverbf.h"
#include "utility/dspinst.h"

AudioEffectFreeverbFloat::AudioEffectFreeverbFloat() : AudioStream(1, inputQueueArray)
{
  for (unsigned int i = 0; i < sizeof(comb1buf) / sizeof(float); i++) comb1buf[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(comb2buf) / sizeof(float); i++) comb2buf[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(comb3buf) / sizeof(float); i++) comb3buf[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(comb4buf) / sizeof(float); i++) comb4buf[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(comb5buf) / sizeof(float); i++) comb5buf[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(comb6buf) / sizeof(float); i++) comb6buf[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(comb7buf) / sizeof(float); i++) comb7buf[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(comb8buf) / sizeof(float); i++) comb8buf[i] = 0.0;
  comb1index = 0;
  comb2index = 0;
  comb3index = 0;
  comb4index = 0;
  comb5index = 0;
  comb6index = 0;
  comb7index = 0;
  comb8index = 0;
  comb1filter = 0.0;
  comb2filter = 0.0;
  comb3filter = 0.0;
  comb4filter = 0.0;
  comb5filter = 0.0;
  comb6filter = 0.0;
  comb7filter = 0.0;
  comb8filter = 0.0;
  combdamp1 = 6553.0;
  combdamp2 = 26215.0;
  combfeeback = 27524.0;
  for (unsigned int i = 0; i < sizeof(allpass1buf) / sizeof(float); i++) allpass1buf[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(allpass2buf) / sizeof(float); i++) allpass2buf[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(allpass3buf) / sizeof(float); i++) allpass3buf[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(allpass4buf) / sizeof(float); i++) allpass4buf[i] = 0.0;
  allpass1index = 0;
  allpass2index = 0;
  allpass3index = 0;
  allpass4index = 0;
}

#if 0
#define sat16i(n, rshift) signed_saturate_rshift((n), 16, (rshift))
#else
// cleaner sat16 by http://www.moseleyinstruments.com/
static int16_t sat16i(int32_t n, int rshift) {
  // we should always round towards 0
  // to avoid recirculating round-off noise
  //
  // a 2s complement positive number is always
  // rounded down, so we only need to take
  // care of negative numbers
  if (n < 0) {
    n = n + (~(0xFFFFFFFFUL << rshift));
  }
  n = n >> rshift;
  if (n > 32767) {
    return 32767;
  }
  if (n < -32768) {
    return -32768;
  }
  return n;
}
#endif

/* static float sat16(float n, int rshift) {
    // we should always round towards 0
    // to avoid recirculating round-off noise
    //
    // a 2s complement positive number is always
    // rounded down, so we only need to take
    // care of negative numbers
    n = n / (float) (1<<rshift);
    return n;
  } */

// TODO: move this to one of the data files, use in output_adat.cpp, output_tdm.cpp, etc
static const audio_block_t zeroblock = {
  0, 0, 0, {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#if AUDIO_BLOCK_SAMPLES > 16
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#endif
#if AUDIO_BLOCK_SAMPLES > 32
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#endif
#if AUDIO_BLOCK_SAMPLES > 48
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#endif
#if AUDIO_BLOCK_SAMPLES > 64
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#endif
#if AUDIO_BLOCK_SAMPLES > 80
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#endif
#if AUDIO_BLOCK_SAMPLES > 96
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#endif
#if AUDIO_BLOCK_SAMPLES > 112
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
#endif
  }
};

void AudioEffectFreeverbFloat::update()
{
#if defined(__ARM_ARCH_7EM__)
  const audio_block_t *block;
  audio_block_t *outblock;
  int i;
  float input, bufout, output;
  float sum;

  outblock = allocate();
  if (!outblock) {
    audio_block_t *tmp = receiveReadOnly(0);
    if (tmp) release(tmp);
    return;
  }
  block = receiveReadOnly(0);
  if (!block) block = &zeroblock;

  for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
    // TODO: scale numerical range depending on roomsize & damping
    //input = sat16(block->data[i] * 8738, 17); // for numerical headroom
    input = (float)block->data[i] / 32768.0f;
    sum = 0;

    bufout = comb1buf[comb1index];
    sum += bufout;
    comb1filter = bufout * combdamp2 + comb1filter * combdamp1;
    comb1buf[comb1index] = input + comb1filter * combfeeback;
    if (++comb1index >= sizeof(comb1buf) / sizeof(float)) comb1index = 0;

    bufout = comb2buf[comb2index];
    sum += bufout;
    comb2filter = bufout * combdamp2 + comb2filter * combdamp1;
    comb2buf[comb2index] = input + comb2filter * combfeeback;
    if (++comb2index >= sizeof(comb2buf) / sizeof(float)) comb2index = 0;

    bufout = comb3buf[comb3index];
    sum += bufout;
    comb3filter = bufout * combdamp2 + comb3filter * combdamp1;
    comb3buf[comb3index] = input + comb3filter * combfeeback;
    if (++comb3index >= sizeof(comb3buf) / sizeof(float)) comb3index = 0;

    bufout = comb4buf[comb4index];
    sum += bufout;
    comb4filter = bufout * combdamp2 + comb4filter * combdamp1;
    comb4buf[comb4index] = input + comb4filter * combfeeback;
    if (++comb4index >= sizeof(comb4buf) / sizeof(float)) comb4index = 0;

    bufout = comb5buf[comb5index];
    sum += bufout;
    comb5filter = bufout * combdamp2 + comb5filter * combdamp1;
    comb5buf[comb5index] = input + comb5filter * combfeeback;
    if (++comb5index >= sizeof(comb5buf) / sizeof(float)) comb5index = 0;

    bufout = comb6buf[comb6index];
    sum += bufout;
    comb6filter = bufout * combdamp2 + comb6filter * combdamp1;
    comb6buf[comb6index] = input + comb6filter * combfeeback;
    if (++comb6index >= sizeof(comb6buf) / sizeof(float)) comb6index = 0;

    bufout = comb7buf[comb7index];
    sum += bufout;
    comb7filter = bufout * combdamp2 + comb7filter * combdamp1;
    comb7buf[comb7index] = input + comb7filter * combfeeback;
    if (++comb7index >= sizeof(comb7buf) / sizeof(float)) comb7index = 0;

    bufout = comb8buf[comb8index];
    sum += bufout;
    comb8filter = bufout * combdamp2 + comb8filter * combdamp1;
    comb8buf[comb8index] = input + comb8filter * combfeeback;
    if (++comb8index >= sizeof(comb8buf) / sizeof(float)) comb8index = 0;

    //output = sat16(sum * 31457.0, 17);
    //output = sum * 31457.0/131072.0f;
    output = sum;

    bufout = allpass1buf[allpass1index];
    allpass1buf[allpass1index] = output + (bufout / 2.0);
    output = (bufout - output) / 2.0;
    if (++allpass1index >= sizeof(allpass1buf) / sizeof(float)) allpass1index = 0;

    bufout = allpass2buf[allpass2index];
    allpass2buf[allpass2index] = output + (bufout / 2.0);
    output = (bufout - output) / 2.0;
    if (++allpass2index >= sizeof(allpass2buf) / sizeof(float)) allpass2index = 0;

    bufout = allpass3buf[allpass3index];
    allpass3buf[allpass3index] = output + (bufout / 2.0);
    output = (bufout - output) / 2.0;
    if (++allpass3index >= sizeof(allpass3buf) / sizeof(float)) allpass3index = 0;

    bufout = allpass4buf[allpass4index];
    allpass4buf[allpass4index] = output + (bufout / 2.0);
    output = (bufout - output) / 2.0;
    if (++allpass4index >= sizeof(allpass4buf) / sizeof(float)) allpass4index = 0;

    //outblock->data[i] = sat16i(output * 30.0, 0);
    outblock->data[i] = sat16i(output * 32768.0, 0);
  }
  transmit(outblock);
  release(outblock);
  if (block != &zeroblock) release((audio_block_t *)block);

#elif defined(KINETISL)
  audio_block_t *block;
  block = receiveReadOnly(0);
  if (block) release(block);
#endif
}


AudioEffectFreeverbStereoFloat::AudioEffectFreeverbStereoFloat() : AudioStream(1, inputQueueArray)
{
  for (unsigned int i = 0; i < sizeof(comb1bufL) / sizeof(float); i++) comb1bufL[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(comb2bufL) / sizeof(float); i++) comb2bufL[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(comb3bufL) / sizeof(float); i++) comb3bufL[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(comb4bufL) / sizeof(float); i++) comb4bufL[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(comb5bufL) / sizeof(float); i++) comb5bufL[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(comb6bufL) / sizeof(float); i++) comb6bufL[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(comb7bufL) / sizeof(float); i++) comb7bufL[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(comb8bufL) / sizeof(float); i++) comb8bufL[i] = 0.0;
  comb1indexL = 0;
  comb2indexL = 0;
  comb3indexL = 0;
  comb4indexL = 0;
  comb5indexL = 0;
  comb6indexL = 0;
  comb7indexL = 0;
  comb8indexL = 0;
  comb1filterL = 0.0;
  comb2filterL = 0.0;
  comb3filterL = 0.0;
  comb4filterL = 0.0;
  comb5filterL = 0.0;
  comb6filterL = 0.0;
  comb7filterL = 0.0;
  comb8filterL = 0.0;
  for (unsigned int i = 0; i < sizeof(comb1bufR) / sizeof(float); i++) comb1bufR[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(comb2bufR) / sizeof(float); i++) comb2bufR[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(comb3bufR) / sizeof(float); i++) comb3bufR[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(comb4bufR) / sizeof(float); i++) comb4bufR[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(comb5bufR) / sizeof(float); i++) comb5bufR[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(comb6bufR) / sizeof(float); i++) comb6bufR[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(comb7bufR) / sizeof(float); i++) comb7bufR[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(comb8bufR) / sizeof(float); i++) comb8bufR[i] = 0.0;
  comb1indexR = 0;
  comb2indexR = 0;
  comb3indexR = 0;
  comb4indexR = 0;
  comb5indexR = 0;
  comb6indexR = 0;
  comb7indexR = 0;
  comb8indexR = 0;
  comb1filterR = 0.0;
  comb2filterR = 0.0;
  comb3filterR = 0.0;
  comb4filterR = 0.0;
  comb5filterR = 0.0;
  comb6filterR = 0.0;
  comb7filterR = 0.0;
  comb8filterR = 0.0;
  combdamp1 = 6553.0;
  combdamp2 = 26215.0;
  combfeeback = 27524.0;
  for (unsigned int i = 0; i < sizeof(allpass1bufL) / sizeof(float); i++) allpass1bufL[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(allpass2bufL) / sizeof(float); i++) allpass2bufL[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(allpass3bufL) / sizeof(float); i++) allpass3bufL[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(allpass4bufL) / sizeof(float); i++) allpass4bufL[i] = 0.0;
  allpass1indexL = 0;
  allpass2indexL = 0;
  allpass3indexL = 0;
  allpass4indexL = 0;
  for (unsigned int i = 0; i < sizeof(allpass1bufR) / sizeof(float); i++) allpass1bufR[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(allpass2bufR) / sizeof(float); i++) allpass2bufR[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(allpass3bufR) / sizeof(float); i++) allpass3bufR[i] = 0.0;
  for (unsigned int i = 0; i < sizeof(allpass4bufR) / sizeof(float); i++) allpass4bufR[i] = 0.0;
  allpass1indexR = 0;
  allpass2indexR = 0;
  allpass3indexR = 0;
  allpass4indexR = 0;
}

void AudioEffectFreeverbStereoFloat::update()
{
#if defined(__ARM_ARCH_7EM__)
  const audio_block_t *block;
  audio_block_t *outblockL;
  audio_block_t *outblockR;
  int i;
  float input, bufout, outputL, outputR;
  float sum;

  block = receiveReadOnly(0);
  outblockL = allocate();
  outblockR = allocate();
  if (!outblockL || !outblockR) {
    if (outblockL) release(outblockL);
    if (outblockR) release(outblockR);
    if (block) release((audio_block_t *)block);
    return;
  }
  if (!block) block = &zeroblock;

  for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
    // TODO: scale numerical range depending on roomsize & damping
    //input = sat16(block->data[i] * 8738.0, 17); // for numerical headroom
    input = block->data[i] / 32768.0;
    sum = 0;

    bufout = comb1bufL[comb1indexL];
    sum += bufout;
    comb1filterL = bufout * combdamp2 + comb1filterL * combdamp1;
    comb1bufL[comb1indexL] = input + comb1filterL * combfeeback;
    if (++comb1indexL >= sizeof(comb1bufL) / sizeof(float)) comb1indexL = 0;

    bufout = comb2bufL[comb2indexL];
    sum += bufout;
    comb2filterL = bufout * combdamp2 + comb2filterL * combdamp1;
    comb2bufL[comb2indexL] = input + comb2filterL * combfeeback;
    if (++comb2indexL >= sizeof(comb2bufL) / sizeof(float)) comb2indexL = 0;

    bufout = comb3bufL[comb3indexL];
    sum += bufout;
    comb3filterL = bufout * combdamp2 + comb3filterL * combdamp1;
    comb3bufL[comb3indexL] = input + comb3filterL * combfeeback;
    if (++comb3indexL >= sizeof(comb3bufL) / sizeof(float)) comb3indexL = 0;

    bufout = comb4bufL[comb4indexL];
    sum += bufout;
    comb4filterL = bufout * combdamp2 + comb4filterL * combdamp1;
    comb4bufL[comb4indexL] = input + comb4filterL * combfeeback;
    if (++comb4indexL >= sizeof(comb4bufL) / sizeof(float)) comb4indexL = 0;

    bufout = comb5bufL[comb5indexL];
    sum += bufout;
    comb5filterL = bufout * combdamp2 + comb5filterL * combdamp1;
    comb5bufL[comb5indexL] = input + comb5filterL * combfeeback;
    if (++comb5indexL >= sizeof(comb5bufL) / sizeof(float)) comb5indexL = 0;

    bufout = comb6bufL[comb6indexL];
    sum += bufout;
    comb6filterL = bufout * combdamp2 + comb6filterL * combdamp1;
    comb6bufL[comb6indexL] = input + comb6filterL * combfeeback;
    if (++comb6indexL >= sizeof(comb6bufL) / sizeof(float)) comb6indexL = 0;

    bufout = comb7bufL[comb7indexL];
    sum += bufout;
    comb7filterL = bufout * combdamp2 + comb7filterL * combdamp1;
    comb7bufL[comb7indexL] = input + comb7filterL * combfeeback;
    if (++comb7indexL >= sizeof(comb7bufL) / sizeof(float)) comb7indexL = 0;

    bufout = comb8bufL[comb8indexL];
    sum += bufout;
    comb8filterL = bufout * combdamp2 + comb8filterL * combdamp1;
    comb8bufL[comb8indexL] = input + comb8filterL * combfeeback;
    if (++comb8indexL >= sizeof(comb8bufL) / sizeof(float)) comb8indexL = 0;

    //outputL = sat16(sum * 31457, 17);
    //outputL = sum * 31457.0/131072.0f;
    outputL = sum;
    sum = 0.0;

    bufout = comb1bufR[comb1indexR];
    sum += bufout;
    comb1filterR = bufout * combdamp2 + comb1filterR * combdamp1;
    comb1bufR[comb1indexR] = input + comb1filterR * combfeeback;
    if (++comb1indexR >= sizeof(comb1bufR) / sizeof(float)) comb1indexR = 0;

    bufout = comb2bufR[comb2indexR];
    sum += bufout;
    comb2filterR = bufout * combdamp2 + comb2filterR * combdamp1;
    comb2bufR[comb2indexR] = input + comb2filterR * combfeeback;
    if (++comb2indexR >= sizeof(comb2bufR) / sizeof(float)) comb2indexR = 0;

    bufout = comb3bufR[comb3indexR];
    sum += bufout;
    comb3filterR = bufout * combdamp2 + comb3filterR * combdamp1;
    comb3bufR[comb3indexR] = input + comb3filterR * combfeeback;
    if (++comb3indexR >= sizeof(comb3bufR) / sizeof(float)) comb3indexR = 0;

    bufout = comb4bufR[comb4indexR];
    sum += bufout;
    comb4filterR = bufout * combdamp2 + comb4filterR * combdamp1;
    comb4bufR[comb4indexR] = input + comb4filterR * combfeeback;
    if (++comb4indexR >= sizeof(comb4bufR) / sizeof(float)) comb4indexR = 0;

    bufout = comb5bufR[comb5indexR];
    sum += bufout;
    comb5filterR = bufout * combdamp2 + comb5filterR * combdamp1;
    comb5bufR[comb5indexR] = input + comb5filterR * combfeeback;
    if (++comb5indexR >= sizeof(comb5bufR) / sizeof(float)) comb5indexR = 0;

    bufout = comb6bufR[comb6indexR];
    sum += bufout;
    comb6filterR = bufout * combdamp2 + comb6filterR * combdamp1;
    comb6bufR[comb6indexR] = input + comb6filterR * combfeeback;
    if (++comb6indexR >= sizeof(comb6bufR) / sizeof(float)) comb6indexR = 0;

    bufout = comb7bufR[comb7indexR];
    sum += bufout;
    comb7filterR = bufout * combdamp2 + comb7filterR * combdamp1;
    comb7bufR[comb7indexR] = input + comb7filterR * combfeeback;
    if (++comb7indexR >= sizeof(comb7bufR) / sizeof(float)) comb7indexR = 0;

    bufout = comb8bufR[comb8indexR];
    sum += bufout;
    comb8filterR = bufout * combdamp2 + comb8filterR * combdamp1;
    comb8bufR[comb8indexR] = input + comb8filterR * combfeeback;
    if (++comb8indexR >= sizeof(comb8bufR) / sizeof(float)) comb8indexR = 0;

    //outputR = sat16(sum * 31457, 17);
    //outputR = sum * 31457.0/131072.0f;
    outputR = sum;

    bufout = allpass1bufL[allpass1indexL];
    allpass1bufL[allpass1indexL] = outputL + (bufout / 2.0);
    outputL = (bufout - outputL) / 2.0;
    if (++allpass1indexL >= sizeof(allpass1bufL) / sizeof(float)) allpass1indexL = 0;

    bufout = allpass2bufL[allpass2indexL];
    allpass2bufL[allpass2indexL] = outputL + (bufout / 2.0);
    outputL = (bufout - outputL) / 2.0;
    if (++allpass2indexL >= sizeof(allpass2bufL) / sizeof(float)) allpass2indexL = 0;

    bufout = allpass3bufL[allpass3indexL];
    allpass3bufL[allpass3indexL] = outputL + (bufout / 2.0);
    outputL = (bufout - outputL) / 2.0;
    if (++allpass3indexL >= sizeof(allpass3bufL) / sizeof(float)) allpass3indexL = 0;

    bufout = allpass4bufL[allpass4indexL];
    allpass4bufL[allpass4indexL] = outputL + (bufout / 2.0);
    outputL = (bufout - outputL) / 2.0;
    if (++allpass4indexL >= sizeof(allpass4bufL) / sizeof(float)) allpass4indexL = 0;

    //outblockL->data[i] = sat16i(outputL * 30.0, 0);
    outblockL->data[i] = sat16i(outputL * 32768.0, 0);

    bufout = allpass1bufR[allpass1indexR];
    allpass1bufR[allpass1indexR] = outputR + (bufout / 2.0);
    //outputR = sat16(bufout - outputR, 1);
    outputR = (bufout - outputR) / 2.0;
    if (++allpass1indexR >= sizeof(allpass1bufR) / sizeof(float)) allpass1indexR = 0;

    bufout = allpass2bufR[allpass2indexR];
    allpass2bufR[allpass2indexR] = outputR + (bufout / 2.0);
    outputR = (bufout - outputR) / 2.0;
    if (++allpass2indexR >= sizeof(allpass2bufR) / sizeof(float)) allpass2indexR = 0;

    bufout = allpass3bufR[allpass3indexR];
    allpass3bufR[allpass3indexR] = outputR + (bufout / 2.0);
    outputR = (bufout - outputR) / 2.0;
    if (++allpass3indexR >= sizeof(allpass3bufR) / sizeof(float)) allpass3indexR = 0;

    bufout = allpass4bufR[allpass4indexR];
    allpass4bufR[allpass4indexR] = outputR + (bufout / 2.0);
    outputR = (bufout - outputR) / 2.0;
    if (++allpass4indexR >= sizeof(allpass4bufR) / sizeof(float)) allpass4indexR = 0;

    //outblockR->data[i] = sat16i(outputR * 30.0, 0);
    outblockR->data[i] = sat16i(outputR * 32768.0, 0);
  }
  transmit(outblockL, 0);
  transmit(outblockR, 1);
  release(outblockL);
  release(outblockR);
  if (block != &zeroblock) release((audio_block_t *)block);

#elif defined(KINETISL)
  audio_block_t *block;
  block = receiveReadOnly(0);
  if (block) release(block);
#endif
}
