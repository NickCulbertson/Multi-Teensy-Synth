#include <Arduino.h>
#include <Audio.h>
#include "arm_math.h"
#include "AudioEffectCustomChorus.h"

// Zero block for silent inputs
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
             }};

boolean AudioEffectCustomChorus::begin(short *delayline_l, short *delayline_r, uint16_t delay_length)
{
  _cb_index[0] = 0;
  _cb_index[1] = 0;

  if (delayline_l == NULL || delayline_r == NULL)
    return false;
  if (delay_length < 10)
    return false;

  _delayline[0] = delayline_l;
  _delayline[1] = delayline_r;
  _delay_length = delay_length;
  _delay_offset = 132; // 3ms base delay (132 samples @ 44.1kHz) - BBD center point

  // Clear delay buffers
  memset(_delayline[0], 0, _delay_length * sizeof(int16_t));
  memset(_delayline[1], 0, _delay_length * sizeof(int16_t));

  // Initialize parameters (Juno-60 Chorus I authentic BBD settings)
  _rate = 0.5f;        // 0.5 Hz (authentic Chorus I rate)
  _depth = 1.0f;       // Fixed depth (not user-adjustable on original)
  _mix = 1.0f;         // 100% wet (authentic Juno)
  _bypass = false;

  // Initialize LFO
  _lfo_phase[0] = 0.0f;
  _lfo_phase[1] = 0.25f; // 90 degree phase offset for stereo
  _lfo_increment = (_rate * 2.0f * M_PI) / AUDIO_SAMPLE_RATE_EXACT;

  return true;
}

void AudioEffectCustomChorus::update(void)
{
  audio_block_t *block;

  if (_delayline[0] == NULL || _delayline[1] == NULL) return;

  block = receiveWritable(0);
  if (!block) block = (audio_block_t *)&zeroblock;

  if (_bypass)
  {
    if (block != (audio_block_t *)&zeroblock)
    {
      transmit(block, 0);
      release(block);
    }
    return;
  }

  // Precompute mix as fixed-point once per block
  int32_t wet256 = (int32_t)(_mix * 256.0f);
  if (wet256 < 0) wet256 = 0;
  if (wet256 > 256) wet256 = 256;
  int32_t dry256 = 256 - wet256;

  // BBD-ish modulation range/limits (your same intent)
  const float modulation_range = 88.0f; // ~±2ms @ 44.1k
  const float minDelay = 66.0f;         // ~1.5ms
  const float maxDelay = 220.0f;        // ~5ms

  // Helpers: triangle from radians phase (phase is in radians in your code)
  const float invTwoPi = 0.15915494309189535f; // 1/(2*pi)

  int16_t *bp = block->data;

  for (uint16_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
  {
    int16_t input_sample = *bp;

    // Write input to delay buffer (you’re using one buffer for both taps)
    uint16_t w = _cb_index[0];
    if (w >= _delay_length) w = 0;
    _delayline[0][w] = input_sample;

    // ----- Triangle LFO, per sample -----
    // Convert radians phase to [0,1) cycle position
    float p0 = _lfo_phase[0] * invTwoPi;
    p0 = p0 - floorf(p0); // wrap 0..1

    float p1 = _lfo_phase[1] * invTwoPi;
    p1 = p1 - floorf(p1);

    // Triangle in [-1, +1]
    float tri0 = (p0 < 0.5f) ? (4.0f * p0 - 1.0f) : (3.0f - 4.0f * p0);
    float tri1 = (p1 < 0.5f) ? (4.0f * p1 - 1.0f) : (3.0f - 4.0f * p1);

    float lfo_l = tri0 * _depth;
    float lfo_r = tri1 * _depth;

    float delay_l = _delay_offset + (lfo_l * modulation_range);
    float delay_r = _delay_offset + (lfo_r * modulation_range);

    if (delay_l < minDelay) delay_l = minDelay;
    if (delay_l > maxDelay) delay_l = maxDelay;
    if (delay_r < minDelay) delay_r = minDelay;
    if (delay_r > maxDelay) delay_r = maxDelay;

    // ----- Fractional delay read w/ linear interpolation -----
    // readPos = writeIndex - delay (may be fractional)
    float readPosL = (float)w - delay_l;
    float readPosR = (float)w - delay_r;

    // Wrap into buffer range
    while (readPosL < 0.0f) readPosL += (float)_delay_length;
    while (readPosL >= (float)_delay_length) readPosL -= (float)_delay_length;
    while (readPosR < 0.0f) readPosR += (float)_delay_length;
    while (readPosR >= (float)_delay_length) readPosR -= (float)_delay_length;

    int32_t i0L = (int32_t)readPosL;
    int32_t i0R = (int32_t)readPosR;
    float fracL = readPosL - (float)i0L;
    float fracR = readPosR - (float)i0R;

    int32_t i1L = i0L + 1; if (i1L >= _delay_length) i1L = 0;
    int32_t i1R = i0R + 1; if (i1R >= _delay_length) i1R = 0;

    float s0L = (float)_delayline[0][i0L];
    float s1L = (float)_delayline[0][i1L];
    float s0R = (float)_delayline[0][i0R];
    float s1R = (float)_delayline[0][i1R];

    float wetL_f = (1.0f - fracL) * s0L + fracL * s1L;
    float wetR_f = (1.0f - fracR) * s0R + fracR * s1R;

    // Your “two tap” thickness (still mono output for this instance)
    int32_t wet_signal = (int32_t)((wetL_f + wetR_f) * 0.5f);

    // Mix (fixed-point-ish)
    int32_t out = (int32_t)input_sample * dry256 + wet_signal * wet256;
    out >>= 8;

    if (out > 32767) out = 32767;
    if (out < -32768) out = -32768;

    *bp = (int16_t)out;

    // Advance pointers
    bp++;
    _cb_index[0] = w + 1;

    // Advance LFO phase per sample (this is what stops it sounding like a static delay)
    _lfo_phase[0] += _lfo_increment;
    _lfo_phase[1] += _lfo_increment;

    if (_lfo_phase[0] >= 2.0f * M_PI) _lfo_phase[0] -= 2.0f * M_PI;
    if (_lfo_phase[1] >= 2.0f * M_PI) _lfo_phase[1] -= 2.0f * M_PI;
  }

  if (block != (audio_block_t *)&zeroblock)
  {
    transmit(block, 0);
    release(block);
  }
}

void AudioEffectCustomChorus::set_rate(float rate)
{
  if (rate < 0.1f) rate = 0.1f;
  if (rate > 5.0f) rate = 5.0f;
  _rate = rate;
  _lfo_increment = (_rate * 2.0f * M_PI) / AUDIO_SAMPLE_RATE_EXACT;
}

void AudioEffectCustomChorus::set_depth(float depth)
{
  if (depth < 0.0f) depth = 0.0f;
  if (depth > 1.0f) depth = 1.0f;
  _depth = depth;
}

void AudioEffectCustomChorus::set_mix(float mix)
{
  if (mix < 0.0f) mix = 0.0f;
  if (mix > 1.0f) mix = 1.0f;
  _mix = mix;
}

void AudioEffectCustomChorus::set_bypass(bool bypass)
{
  _bypass = bypass;
}

uint16_t AudioEffectCustomChorus::get_delay_length(void)
{
  return _delay_length;
}
