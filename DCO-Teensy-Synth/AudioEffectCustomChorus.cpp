// AudioEffectCustomChorus.cpp
//
// NOTE: This effect outputs 100% WET. Mix dry externally (your finalMixL/R)
// on different mixer inputs (0=dry, 1=wet).

#include "AudioEffectCustomChorus.h"

// ---- Static member definitions ----
float   AudioEffectCustomChorus::shared_lfo_phase         = 0.0f;
float   AudioEffectCustomChorus::shared_block_phase_start = 0.0f;
float   AudioEffectCustomChorus::shared_block_phase_end   = 0.0f;
uint8_t AudioEffectCustomChorus::shared_block_consumers   = 0;
bool    AudioEffectCustomChorus::shared_block_valid       = false;
bool    AudioEffectCustomChorus::shared_block_end_ready   = false;

const float AudioEffectCustomChorus::LFO_RATES[4] = {
  0.0f,  // off
  0.5f,  // I  (triangle)
  0.8f,  // II (triangle)
  0.65f  // I+II (approximation with beating-like modulation)
};

// Delay range (seconds) — close to your original
const float AudioEffectCustomChorus::DELAY_MIN[4] = {
  0.0f,
  0.00166f,
  0.00166f,
  0.00166f
};

const float AudioEffectCustomChorus::DELAY_MAX[4] = {
  0.0f,
  0.00535f,
  0.00535f,
  0.00535f
};

// Zero block: global statics are zero-initialized
static audio_block_t zeroblock;

float AudioEffectCustomChorus::onePoleA(float fc_hz)
{
  // a = 1 - exp(-2*pi*fc/fs)
  const float kTwoPi = 6.2831853071795864769f;
  float a = 1.0f - expf(-(kTwoPi * fc_hz) / AUDIO_SAMPLE_RATE_EXACT);
  if (a < 0.0f) a = 0.0f;
  if (a > 1.0f) a = 1.0f;
  return a;
}

float AudioEffectCustomChorus::lfoValue01(float phase01) const
{
  float p = phase01;

  // Stereo behavior (meter-friendly and sounds wide):
  // - Mode 1/2: right is 180° offset
  // - Mode 3: ALSO 180° offset (this is the change that makes it look stereo again)
  if (_is_right_channel) {
    p += 0.5f;
    if (p >= 1.0f) p -= 1.0f;
  }

  if (_mode == 3) {
    // Complex waveform to approximate I+II beating effect
    // Mix triangle with subtle secondary modulation
    float tri;
    if (p < 0.5f) tri = p * 2.0f;
    else tri = 2.0f - (p * 2.0f);
    
    // Add secondary beating component (simulates interaction of 0.5Hz + 0.8Hz)
    const float kTwoPi = 6.2831853071795864769f;
    float secondary = arm_sin_f32(kTwoPi * p * 2.6f) * 0.15f; // ~2.6x for beating effect
    
    float result = tri + secondary;
    if (result < 0.0f) result = 0.0f;
    if (result > 1.0f) result = 1.0f;
    return result;
  } else {
    // triangle LFO: 0..1
    if (p < 0.5f) return p * 2.0f;
    return 2.0f - (p * 2.0f);
  }
}

boolean AudioEffectCustomChorus::begin(short *delayline, uint16_t delay_length, bool is_right_channel)
{
  _cb_index = 0;
  _is_right_channel = is_right_channel;

  if (delayline == NULL) return false;
  if (delay_length < 32) return false;

  _delayline = (int16_t *)delayline;
  _delay_length = delay_length;

  memset(_delayline, 0, _delay_length * sizeof(int16_t));

  _mode = 1;
  _bypass = false;

  // Correct LFO increment: Hz / Fs (NO *4)
  _lfo_increment = LFO_RATES[_mode] / AUDIO_SAMPLE_RATE_EXACT;

  // Filter coeffs
  _pre_a  = onePoleA(18000.0f); // fairly bright
  _post_a = onePoleA(7000.0f);  // creamy rolloff-ish

  _pre_filter_state = 0.0f;
  _post_filter_state = 0.0f;

  return true;
}

void AudioEffectCustomChorus::update(void)
{
  if (_delayline == NULL) return;

  audio_block_t *block = receiveWritable(0);
  if (!block) block = &zeroblock;

  if (_bypass || _mode == 0) {
    if (block != &zeroblock) {
      transmit(block, 0);
      release(block);
    }
    return;
  }

  // --- Block phase latch (keeps L/R in sync regardless of update() call order) ---
  if (!shared_block_valid) {
    shared_block_phase_start = shared_lfo_phase;
    shared_block_valid = true;
    shared_block_consumers = 0;
    shared_block_end_ready = false;
  }

  float phase = shared_block_phase_start;

  const float min_delay_samp = DELAY_MIN[_mode] * AUDIO_SAMPLE_RATE_EXACT;
  const float max_delay_samp = DELAY_MAX[_mode] * AUDIO_SAMPLE_RATE_EXACT;

  // Depths:
  // Mode 1 subtle, Mode 2 richer, Mode 3 enhanced depth for combined effect
  float depth = 1.0f;
  if (_mode == 1) depth = 0.65f;
  else if (_mode == 2) depth = 0.90f;
  else if (_mode == 3) depth = 0.78f;  // Higher depth for combined effect  

  const float center = 0.5f * (min_delay_samp + max_delay_samp);
  const float half_range = 0.5f * (max_delay_samp - min_delay_samp) * depth;

  int16_t *bp = block->data;


  const float kStereoOffsetSamples = (_is_right_channel ? 6.0f : 0.0f);

  for (uint16_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {

    int16_t in = *bp;

    // Pre filter
    float x = (float)in;
    _pre_filter_state += (x - _pre_filter_state) * _pre_a;
    float filtered_in = _pre_filter_state;

    // Write delay
    uint16_t w = _cb_index;
    if (w >= _delay_length) w = 0;
    _delayline[w] = (int16_t)filtered_in;

    // LFO 0..1 from per-sample phase
    float lfo01 = lfoValue01(phase);

    // Delay modulation
    float delay_samp = center + (lfo01 - 0.5f) * (2.0f * half_range);

    // Add tiny static offset to R to reduce correlation (meter-friendly)
    delay_samp += kStereoOffsetSamples;

    // Clamp
    if (delay_samp < min_delay_samp) delay_samp = min_delay_samp;
    if (delay_samp > max_delay_samp) delay_samp = max_delay_samp;

    // Fractional read
    float read_pos = (float)w - delay_samp;
    while (read_pos < 0.0f) read_pos += (float)_delay_length;
    while (read_pos >= (float)_delay_length) read_pos -= (float)_delay_length;

    int32_t i0 = (int32_t)read_pos;
    float frac = read_pos - (float)i0;
    int32_t i1 = i0 + 1;
    if (i1 >= _delay_length) i1 = 0;

    float s0 = (float)_delayline[i0];
    float s1 = (float)_delayline[i1];
    float delayed = (1.0f - frac) * s0 + frac * s1;

    // Post filter
    _post_filter_state += (delayed - _post_filter_state) * _post_a;

    // Gentle saturation
    float y = _post_filter_state;
    if (y > 16000.0f)  y = 16000.0f + (y - 16000.0f) * 0.30f;
    if (y < -16000.0f) y = -16000.0f + (y + 16000.0f) * 0.30f;

    // 100% wet out
    int32_t out = (int32_t)y;
    if (out > 32767) out = 32767;
    if (out < -32768) out = -32768;
    *bp++ = (int16_t)out;

    // Advance ring
    _cb_index = w + 1;

    // Advance phase EVERY sample (both channels)
    phase += _lfo_increment;
    if (phase >= 1.0f) phase -= 1.0f;
  }

  // Left channel provides the “end of block” phase
  if (!_is_right_channel) {
    shared_block_phase_end = phase;
    shared_block_end_ready = true;
  }

  // Count this instance as finished
  shared_block_consumers++;

  // When both channels finished, commit phase and reset latch
  // (Assumes two chorus instances: L and R)
  if (shared_block_consumers >= 2) {
    if (shared_block_end_ready) {
      shared_lfo_phase = shared_block_phase_end;
    }
    shared_block_valid = false;
    shared_block_consumers = 0;
    shared_block_end_ready = false;
  }

  if (block != &zeroblock) {
    transmit(block, 0);
    release(block);
  }
}

void AudioEffectCustomChorus::set_mode(int mode)
{
  if (mode < 0) mode = 0;
  if (mode > 3) mode = 3;
  _mode = mode;

  _lfo_increment = LFO_RATES[_mode] / AUDIO_SAMPLE_RATE_EXACT;
}

void AudioEffectCustomChorus::set_bypass(bool bypass)
{
  _bypass = bypass;
}

uint16_t AudioEffectCustomChorus::get_delay_length(void)
{
  return _delay_length;
}
