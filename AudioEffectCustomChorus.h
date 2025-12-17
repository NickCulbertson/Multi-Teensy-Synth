#ifndef AudioEffectCustomChorus_h_
#define AudioEffectCustomChorus_h_

#include "Arduino.h"
#include "AudioStream.h"

class AudioEffectCustomChorus : public AudioStream
{
public:
    AudioEffectCustomChorus(void)
      : AudioStream(1, inputQueueArray) {}

  boolean begin(short *delayline_l, short *delayline_r, uint16_t delay_length);
  virtual void update(void);
  virtual void set_rate(float rate);
  virtual void set_depth(float depth);
  virtual void set_mix(float mix);
  virtual void set_bypass(bool bypass);
  virtual uint16_t get_delay_length(void);

private:
  audio_block_t *inputQueueArray[1];
  int16_t *_delayline[2];    // L and R delay buffers
  uint16_t _cb_index[2];     // current write pointer for L/R buffers
  uint16_t _delay_length;    // delay buffer length in samples
  uint16_t _delay_offset;    // base delay offset (center point)
  
  // Chorus parameters
  float _rate;               // LFO rate in Hz
  float _depth;              // modulation depth (0.0-1.0)
  float _mix;                // wet/dry mix (0.0-1.0)
  bool _bypass;              // bypass flag
  
  // Internal LFO state
  float _lfo_phase[2];       // LFO phase for L/R channels
  float _lfo_increment;      // LFO phase increment per sample
};

#endif // AudioEffectCustomChorus_h_
