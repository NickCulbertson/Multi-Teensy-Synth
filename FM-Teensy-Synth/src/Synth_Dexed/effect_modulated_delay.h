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

#ifndef effect_modulated_chorus_h_
#define effect_modulated_chorus_h_

#include "Arduino.h"
#include "AudioStream.h"

/*************************************************************************/
//                A u d i o E f f e c t M o d u l a t e d D e l a y
// Written by Pete (El Supremo) Jan 2014
// 140219 - correct storage class (not static)
// 190527 - added modulation input handling (Aug 2019 by Holger Wirtz)

class AudioEffectModulatedDelay : public AudioStream
{
public:
  AudioEffectModulatedDelay(void)
      : AudioStream(2, inputQueueArray) {}

  boolean begin(short *delayline, uint16_t delay_length);
  virtual void update(void);
  virtual uint16_t get_delay_length(void);
  virtual void set_bypass(bool b);
  virtual bool get_bypass(void);

private:
  audio_block_t *inputQueueArray[2];
  int16_t *_delayline;    // pointer for the circular buffer
  uint16_t _cb_index;     // current write pointer of the circular buffer
  uint16_t _delay_length; // calculated number of samples of the delay
  int16_t cb_mod_index;   // current read pointer with modulation for the circular buffer
  uint16_t _delay_offset;
  bool bypass;
};

class AudioEffectModulatedDelayStereo : public AudioStream
{
public:
  AudioEffectModulatedDelayStereo(void)
      : AudioStream(3, inputQueueArray) {}

  boolean begin(short *delayline_l, short *delayline_r, uint16_t delay_length);
  virtual void update(void);
  virtual uint16_t get_delay_length(void);
  virtual void set_stereo(bool s);
  virtual bool get_stereo(void);
  virtual void set_bypass(bool b);
  virtual bool get_bypass(void);

private:
  audio_block_t *inputQueueArray[3];
  int16_t *_delayline[2];  // pointer for the circular buffer
  uint16_t _cb_index[2];   // current write pointer of the circular buffer
  uint16_t _delay_length;  // calculated number of samples of the delay
  int16_t cb_mod_index[2]; // current read pointer with modulation for the circular buffer
  uint16_t _delay_offset;
  bool stereo;
  bool bypass;
};
#endif
