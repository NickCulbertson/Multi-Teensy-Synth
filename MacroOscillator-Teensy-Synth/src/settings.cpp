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
#include "Arduino.h"
#include "settings.h"
#include <cstring>

namespace braids {

  using namespace stmlib;

  const SettingsData kInitSettings = {
    MACRO_OSC_SHAPE_CSAW,

    RESOLUTION_16_BIT,
    SAMPLE_RATE_44K,
    PITCH_RANGE_440,
    2,
   
  };

  FLASHMEM void Settings::Init() {

    bool settings_within_range = true;
    for (int32_t i = 0; i <= SETTING_LAST_EDITABLE_SETTING; ++i) {
      const Setting setting = static_cast<Setting>(i);
      const SettingMetadata& setting_metadata = metadata(setting);
      uint8_t value = GetValue(setting);
      settings_within_range = settings_within_range && \
        value >= setting_metadata.min_value && \
        value <= setting_metadata.max_value;
    }
    settings_within_range = settings_within_range && data_.magic_byte == 'M';

    if (!settings_within_range) {
      Reset();
    }
  }

  FLASHMEM void Settings::Reset() {
    memcpy(&data_, &kInitSettings, sizeof(SettingsData));
    data_.magic_byte = 'M';
  }

  const char* const algo_values[] = {
      "CSAW",
      "^\x0D\x12_",
      "\x0D\x0F\x11\x12",
      "FOLD",
      "\x13\x13\x13\x13",
      "SUB\x11",
      "SUB\x0D",
      "SYN\x11",
      "SYN\x0D",
      "\x0D\x0Dx3",
      "\x12_x3",
      "/\\x3",
      "SIx3",
      "RING",
      "\x0D\x0E\x0D\x0E",
      "\x0D\x0D\x12\x12",
      "TOY*",
      "ZLPF",
      "ZPKF",
      "ZBPF",
      "ZHPF",
      "VOSM",
      "VOWL",
      "VFOF",
      "HARM",
      "FM  ",
      "FBFM",
      "WTFM",
      "PLUK",
      "BOWD",
      "BLOW",
      "FLUT",
      "BELL",
      "DRUM",
      "KICK",
      "CYMB",
      "SNAR",
      //    "WTBL",
      //    "WMAP",
      //    "WLIN",
      //    "WTx4",
          "NOIS",
          "TWNQ",
          "CLKN",
          "CLOU",
          "PRTC",
          "QPSK",
          // "NAME" // For your algorithm
  };

  const char* const bits_values[] = {
      // "2BIT",
      // "3BIT",
      // "4BIT",
      // "6BIT",
      // "8BIT",
      // "12B",
      // "16B "
       };

  const char* const rates_values[] = {
      // "4KHZ",
      // "8KHZ",
      // "16K ",
      // "24K ",
      // "32K ",
      // "48K ",
      // "96K " 
      } ;

  const char* const trig_source_values[] = { "EXT.", "AUTO" };

  const char* const pitch_range_values[] = {
      // "EXT.",
      // "FREE",
      // "XTND",
      // "440 ",
      // "LFO "
  };

  const char* const octave_values[] = { "-2", "-1", "0", "1", "2" };


  /* static */
  const SettingMetadata Settings::metadata_[] = {
    { 0, MACRO_OSC_SHAPE_LAST - 2, "WAVE", algo_values },
    { 0, RESOLUTION_LAST - 1, "BITS", bits_values },
    { 0, SAMPLE_RATE_LAST - 1, "RATE", rates_values },
    // { 0, 1, "META", boolean_values },
    { 0, 3, "RANG", pitch_range_values },
    { 0, 4, "OCTV", octave_values },

  };

  /* static */
  const Setting Settings::settings_order_[] = {
    SETTING_OSCILLATOR_SHAPE,
   // SETTING_META_MODULATION,
    SETTING_RESOLUTION,
    SETTING_SAMPLE_RATE,
       SETTING_PITCH_RANGE,
    SETTING_PITCH_OCTAVE,

  };

  /* extern */
  Settings settings;

}  // namespace braids
