#ifndef AudioEffectCustomChorus_h_
#define AudioEffectCustomChorus_h_

#include "Arduino.h"
#include "AudioStream.h"
#include "arm_math.h"
#include <math.h>

class AudioEffectCustomChorus : public AudioStream
{
public:
    AudioEffectCustomChorus(void)
      : AudioStream(1, inputQueueArray) {}

  boolean begin(short *delayline, uint16_t delay_length, bool is_right_channel);
  virtual void update(void);
  virtual void set_mode(int mode);  // 0=off, 1=I, 2=II, 3=I+II
  virtual void set_bypass(bool bypass);
  virtual uint16_t get_delay_length(void);

  // Optional external sync helpers
  static void sync_lfo_phase(float phase) { shared_lfo_phase = phase; }
  static float get_lfo_phase() { return shared_lfo_phase; }

private:
  audio_block_t *inputQueueArray[1];

  // Delay buffer
  int16_t *_delayline = nullptr;
  uint16_t _cb_index = 0;
  uint16_t _delay_length = 0;

  // Mode/bypass
  int  _mode = 1;
  bool _bypass = false;

  // Channel flag (true for right)
  bool _is_right_channel = false;

  // Shared LFO phase between L/R instances
  static float shared_lfo_phase;

  // Block-sync handshake so L/R use the same phase start each block
  static float   shared_block_phase_start;
  static float   shared_block_phase_end;
  static uint8_t shared_block_consumers;
  static bool    shared_block_valid;
  static bool    shared_block_end_ready;

  // LFO increment (phase is 0..1 for one full cycle)
  float _lfo_increment = 0.0f;

  // Mode tables (Hz + delay range)
  static const float LFO_RATES[4];
  static const float DELAY_MIN[4]; // seconds
  static const float DELAY_MAX[4]; // seconds

  // Filters
  float _pre_filter_state  = 0.0f;
  float _post_filter_state = 0.0f;
  float _pre_a  = 0.0f;
  float _post_a = 0.0f;

private:
  static float onePoleA(float fc_hz);

  // LFO output 0..1 from phase 0..1
  float lfoValue01(float phase01) const;
};

#endif // AudioEffectCustomChorus_h_
