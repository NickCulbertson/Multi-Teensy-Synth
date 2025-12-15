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

#ifndef effect_freeverb_f_h_
#define effect_freeverb_f_h_
#include <Arduino.h>
#include "AudioStream.h"

class AudioEffectFreeverbFloat : public AudioStream
{
  public:
    AudioEffectFreeverbFloat();
    virtual void update();
    void roomsize(float n) {
      if (n > 1.0f) n = 1.0f;
      else if (n < 0.0) n = 0.0f;
      //combfeeback = (n * 9175.04f) + 22937.0;
      combfeeback = (n * 9175.04f / 32768.0f) + 22937.0 / 32768.0f;
    }
    void damping(float n) {
      if (n > 1.0f) n = 1.0f;
      else if (n < 0.0) n = 0.0f;
      //float x1 = (n * 13107.2f);
      //float x2 = 32768.0 - x1;
      float x1 = (n * 13107.2f / 32768.0f);
      float x2 = 1.0 - x1;
      __disable_irq();
      combdamp1 = x1;
      combdamp2 = x2;
      __enable_irq();
    }
  private:
    audio_block_t *inputQueueArray[1];
    float comb1buf[1116];
    float comb2buf[1188];
    float comb3buf[1277];
    float comb4buf[1356];
    float comb5buf[1422];
    float comb6buf[1491];
    float comb7buf[1557];
    float comb8buf[1617];
    uint16_t comb1index;
    uint16_t comb2index;
    uint16_t comb3index;
    uint16_t comb4index;
    uint16_t comb5index;
    uint16_t comb6index;
    uint16_t comb7index;
    uint16_t comb8index;
    float comb1filter;
    float comb2filter;
    float comb3filter;
    float comb4filter;
    float comb5filter;
    float comb6filter;
    float comb7filter;
    float comb8filter;
    float combdamp1;
    float combdamp2;
    float combfeeback;
    float allpass1buf[556];
    float allpass2buf[441];
    float allpass3buf[341];
    float allpass4buf[225];
    uint16_t allpass1index;
    uint16_t allpass2index;
    uint16_t allpass3index;
    uint16_t allpass4index;
};


class AudioEffectFreeverbStereoFloat : public AudioStream
{
  public:
    AudioEffectFreeverbStereoFloat();
    virtual void update();
    void roomsize(float n) {
      if (n > 1.0f) n = 1.0f;
      else if (n < 0.0) n = 0.0f;
      //combfeeback = (n * 9175.04f) + 22937.0;
      combfeeback = (n * 9175.04f / 32768.0f) + 22937.0 / 32768.0f;
    }
    void damping(float n) {
      if (n > 1.0f) n = 1.0f;
      else if (n < 0.0) n = 0.0f;
      //float x1 = (n * 13107.2f);
      //float x2 = 32768.0 - x1;
      float x1 = (n * 13107.2f / 32768.0f);
      float x2 = 1.0 - x1;
      __disable_irq();
      combdamp1 = x1;
      combdamp2 = x2;
      __enable_irq();
    }
  private:
    audio_block_t *inputQueueArray[1];
    float comb1bufL[1116];
    float comb2bufL[1188];
    float comb3bufL[1277];
    float comb4bufL[1356];
    float comb5bufL[1422];
    float comb6bufL[1491];
    float comb7bufL[1557];
    float comb8bufL[1617];
    uint16_t comb1indexL;
    uint16_t comb2indexL;
    uint16_t comb3indexL;
    uint16_t comb4indexL;
    uint16_t comb5indexL;
    uint16_t comb6indexL;
    uint16_t comb7indexL;
    uint16_t comb8indexL;
    float comb1filterL;
    float comb2filterL;
    float comb3filterL;
    float comb4filterL;
    float comb5filterL;
    float comb6filterL;
    float comb7filterL;
    float comb8filterL;
    float comb1bufR[1139];
    float comb2bufR[1211];
    float comb3bufR[1300];
    float comb4bufR[1379];
    float comb5bufR[1445];
    float comb6bufR[1514];
    float comb7bufR[1580];
    float comb8bufR[1640];
    uint16_t comb1indexR;
    uint16_t comb2indexR;
    uint16_t comb3indexR;
    uint16_t comb4indexR;
    uint16_t comb5indexR;
    uint16_t comb6indexR;
    uint16_t comb7indexR;
    uint16_t comb8indexR;
    float comb1filterR;
    float comb2filterR;
    float comb3filterR;
    float comb4filterR;
    float comb5filterR;
    float comb6filterR;
    float comb7filterR;
    float comb8filterR;
    float combdamp1;
    float combdamp2;
    float combfeeback;
    float allpass1bufL[556];
    float allpass2bufL[441];
    float allpass3bufL[341];
    float allpass4bufL[225];
    uint16_t allpass1indexL;
    uint16_t allpass2indexL;
    uint16_t allpass3indexL;
    uint16_t allpass4indexL;
    float allpass1bufR[579];
    float allpass2bufR[464];
    float allpass3bufR[364];
    float allpass4bufR[248];
    uint16_t allpass1indexR;
    uint16_t allpass2indexR;
    uint16_t allpass3indexR;
    uint16_t allpass4indexR;
};


#endif
