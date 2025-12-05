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

#ifndef effect_auto_pan_h_
#define effect_auto_pan_h_

#include "Arduino.h"
#include "AudioStream.h"

/*************************************************************************/
//                A u d i o E f f e c t A u t o P a n
// Written by Holger Wirtz
// 20191205 - inital version

class AudioEffectAutoPan : public AudioStream
{
  public:
    AudioEffectAutoPan(void):
      AudioStream(2, inputQueueArray)
    {
      ;
    }

    virtual void update(void);

  private:
    audio_block_t *inputQueueArray[2];
    audio_block_t *out[2];
    float in_f[AUDIO_BLOCK_SAMPLES];
    float out_f[2][AUDIO_BLOCK_SAMPLES];
    float pan_f[AUDIO_BLOCK_SAMPLES];
    const float _pseudo_log = 1048575 / (float)(1 << 20);
};

#endif
