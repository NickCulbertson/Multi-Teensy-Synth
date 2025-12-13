// Copyright 2012 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Settings

#ifndef BRAIDS_SETTINGS_H_
#define BRAIDS_SETTINGS_H_

#include "stmlib.h"

namespace braids {

  enum MacroOscillatorShape {
    MACRO_OSC_SHAPE_CSAW,
    MACRO_OSC_SHAPE_MORPH,
    MACRO_OSC_SHAPE_SAW_SQUARE,
    MACRO_OSC_SHAPE_SINE_TRIANGLE,
    MACRO_OSC_SHAPE_BUZZ,

    MACRO_OSC_SHAPE_SQUARE_SUB,
    MACRO_OSC_SHAPE_SAW_SUB,
    MACRO_OSC_SHAPE_SQUARE_SYNC,
    MACRO_OSC_SHAPE_SAW_SYNC,
    MACRO_OSC_SHAPE_TRIPLE_SAW,
    MACRO_OSC_SHAPE_TRIPLE_SQUARE,
    MACRO_OSC_SHAPE_TRIPLE_TRIANGLE,
    MACRO_OSC_SHAPE_TRIPLE_SINE,
    MACRO_OSC_SHAPE_TRIPLE_RING_MOD,
    MACRO_OSC_SHAPE_SAW_SWARM,
    MACRO_OSC_SHAPE_SAW_COMB,
    MACRO_OSC_SHAPE_TOY,

    MACRO_OSC_SHAPE_DIGITAL_FILTER_LP,
    MACRO_OSC_SHAPE_DIGITAL_FILTER_PK,
    MACRO_OSC_SHAPE_DIGITAL_FILTER_BP,
    MACRO_OSC_SHAPE_DIGITAL_FILTER_HP,
    MACRO_OSC_SHAPE_VOSIM,
    MACRO_OSC_SHAPE_VOWEL,
    MACRO_OSC_SHAPE_VOWEL_FOF,

    MACRO_OSC_SHAPE_HARMONICS,

    MACRO_OSC_SHAPE_FM,
    MACRO_OSC_SHAPE_FEEDBACK_FM,
    MACRO_OSC_SHAPE_CHAOTIC_FEEDBACK_FM,

    MACRO_OSC_SHAPE_PLUCKED,
    MACRO_OSC_SHAPE_BOWED,
    MACRO_OSC_SHAPE_BLOWN,
    MACRO_OSC_SHAPE_FLUTED,
    MACRO_OSC_SHAPE_STRUCK_BELL,
    MACRO_OSC_SHAPE_STRUCK_DRUM,
    MACRO_OSC_SHAPE_KICK,
    MACRO_OSC_SHAPE_CYMBAL,
    MACRO_OSC_SHAPE_SNARE,

    //  MACRO_OSC_SHAPE_WAVETABLES,
    //  MACRO_OSC_SHAPE_WAVE_MAP,
    //  MACRO_OSC_SHAPE_WAVE_LINE,
    //  MACRO_OSC_SHAPE_WAVE_PARAPHONIC,

    MACRO_OSC_SHAPE_FILTERED_NOISE,
    MACRO_OSC_SHAPE_TWIN_PEAKS_NOISE,
    MACRO_OSC_SHAPE_CLOCKED_NOISE,
    MACRO_OSC_SHAPE_GRANULAR_CLOUD,
    MACRO_OSC_SHAPE_PARTICLE_NOISE,

    MACRO_OSC_SHAPE_DIGITAL_MODULATION,

    //  MACRO_OSC_SHAPE_QUESTION_MARK,
      // MACRO_OSC_SHAPE_YOUR_ALGO
    MACRO_OSC_SHAPE_LAST,
    MACRO_OSC_SHAPE_LAST_ACCESSIBLE_FROM_META = MACRO_OSC_SHAPE_DIGITAL_MODULATION
  };

  enum Resolution {
    RESOLUTION_16_BIT,
    RESOLUTION_LAST
  };

  enum SampleRate {
    SAMPLE_RATE_44K,
    SAMPLE_RATE_LAST
  };

  enum PitchRange {
    PITCH_RANGE_EXTERNAL,
    PITCH_RANGE_FREE,
    PITCH_RANGE_EXTENDED,
    PITCH_RANGE_440,
    PITCH_RANGE_LFO  // This setting is hidden by default!
  };

  enum Setting {
    SETTING_OSCILLATOR_SHAPE,
    SETTING_RESOLUTION,
    SETTING_SAMPLE_RATE,
    //   SETTING_META_MODULATION,
    SETTING_PITCH_RANGE,
    SETTING_PITCH_OCTAVE,
    SETTING_LAST_EDITABLE_SETTING = SETTING_PITCH_OCTAVE,

    // Not settings per-se, but used for menu display!

    SETTING_LAST
  };

  struct SettingsData {
    uint8_t shape;
    uint8_t resolution;
    uint8_t sample_rate;
    //   uint8_t meta_modulation;
    uint8_t pitch_range;
    uint8_t pitch_octave;

    char magic_byte;
  };

  struct SettingMetadata {
    uint8_t min_value;
    uint8_t max_value;
    const char name[5];
    const char* const* strings;
  };

  class Settings {
  public:
    Settings() { }
    ~Settings() { }

    void Init();
    void Reset();

    void SetValue(Setting setting, uint8_t value) {
      uint8_t* data = static_cast<uint8_t*>(static_cast<void*>(&data_));
      data[setting] = value;
    }

    uint8_t GetValue(Setting setting) const {
      const uint8_t* data = static_cast<const uint8_t*>(
        static_cast<const void*>(&data_));
      return data[setting];
    }

    inline MacroOscillatorShape shape() const {
      return static_cast<MacroOscillatorShape>(data_.shape);
    }

    inline const SettingsData& data() const { return data_; }
    inline SettingsData* mutable_data() { return &data_; }

    inline int32_t pitch_transposition() const {
      int32_t t = data_.pitch_range == PITCH_RANGE_LFO ? -36 << 7 : 0;
      t += (static_cast<int32_t>(data_.pitch_octave) - 2) * 12 * 128;
      return t;
    }

    static const SettingMetadata& metadata(Setting setting) {
      return metadata_[setting];
    }

    static const Setting setting_at_index(int16_t index) {
      return settings_order_[index];
    }

  private:

    SettingsData data_;

    static const SettingMetadata metadata_[SETTING_LAST];
    static const Setting settings_order_[SETTING_LAST];
    //DISALLOW_COPY_AND_ASSIGN(Settings);
  };

  extern Settings settings;

}  // namespace braids

#endif  // BRAIDS_SETTINGS_H_
