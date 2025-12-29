#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// Hardware Configuration Template
// ============================================================================

// Menu Encoder Pin Assignments
#define MENU_ENCODER_CLK 14
#define MENU_ENCODER_DT 15  
#define MENU_ENCODER_SW 13

// ============================================================================
// Encoder Pin Definitions (Mini-Teensy Standard Layout)
// ============================================================================

#define ENC_1_CLK    4      // enc1 pins
#define ENC_1_DT     5
#define ENC_2_CLK    2      // enc2 pins  
#define ENC_2_DT     3
#define ENC_3_CLK    0      // enc3 pins
#define ENC_3_DT     1
#define ENC_4_CLK    8      // enc4 pins
#define ENC_4_DT     9
#define ENC_5_CLK    6      // enc5 pins
#define ENC_5_DT     7
#define ENC_6_CLK    25     // enc6 pins
#define ENC_6_DT     27
#define ENC_7_CLK    12     // enc7 pins
#define ENC_7_DT     24
#define ENC_8_CLK    10     // enc8 pins
#define ENC_8_DT     11
#define ENC_9_CLK    29     // enc9 pins
#define ENC_9_DT     30
#define ENC_10_CLK   28     // enc10 pins
#define ENC_10_DT    26
#define ENC_11_CLK   21     // enc11 pins
#define ENC_11_DT    20
#define ENC_13_CLK   34     // enc13 pins
#define ENC_13_DT    33
#define ENC_14_CLK   50     // enc14 pins
#define ENC_14_DT    41
#define ENC_15_CLK   23     // enc15 pins
#define ENC_15_DT    22
#define ENC_16_CLK   36     // enc16 pins
#define ENC_16_DT    35
#define ENC_17_CLK   31     // enc17 pins
#define ENC_17_DT    32
#define ENC_18_CLK   17     // enc18 pins
#define ENC_18_DT    16
#define ENC_19_CLK   38     // enc19 pins
#define ENC_19_DT    37
#define ENC_20_CLK   40     // enc20 pins
#define ENC_20_DT    39

// Display Configuration (Choose ONE - comment out the other)
#define USE_LCD_DISPLAY
// #define USE_OLED_DISPLAY

// MIDI Configuration - Enable the MIDI sources you want to use
#define USE_USB_DEVICE_MIDI // USB Device MIDI for DAW/computer connection (default)
#define USE_MIDI_HOST    // USB Host MIDI for external controllers connected to Teensy

// ============================================================================
// Braids Knob Mapping Configuration
// ============================================================================
// Easily customize which physical knobs control which Braids parameters!
// Just change the parameter numbers next to each encoder to remap your knobs.

// Available Braids Parameters (0-15) - Updated for Moog filter:
// 0   // BRAIDS_SHAPE       - Oscillator algorithm/shape (0-42)
// 1   // BRAIDS_TIMBRE      - Timbre parameter (0-127)
// 2   // BRAIDS_COLOR       - Color parameter (0-127) 
// 3   // BRAIDS_COARSE      - Coarse transpose (semitones) [MENU-ONLY]
// 4   // BRAIDS_AMP_ATTACK  - Amp envelope attack (0-127)
// 5   // BRAIDS_AMP_DECAY   - Amp envelope decay (0-127)
// 6   // BRAIDS_AMP_SUSTAIN - Amp envelope sustain (0-127)
// 7   // BRAIDS_AMP_RELEASE - Amp envelope release (0-127)
// 8   // BRAIDS_FILTER_FREQ - Filter cutoff frequency (0-127) [Moog Ladder]
// 9   // BRAIDS_FILTER_RES  - Filter resonance (0-127)
// 10  // BRAIDS_FILTER_STR  - Filter envelope strength (0-127)
// 11  // BRAIDS_FILT_ATTACK - Filter envelope attack (0-127)
// 12  // BRAIDS_FILT_DECAY  - Filter envelope decay (0-127)
// 13  // BRAIDS_FILT_SUSTAIN- Filter envelope sustain (0-127)
// 14  // BRAIDS_FILT_RELEASE- Filter envelope release (0-127)
// 15  // BRAIDS_VOLUME      - Overall volume (0-127)

#define ENC_1_PARAM    0   // BRAIDS_SHAPE       // enc1 (pins 4,5) → Shape/Algorithm
#define ENC_2_PARAM    1   // BRAIDS_TIMBRE      // enc2 (pins 2,3) → Timbre  
#define ENC_3_PARAM    2   // BRAIDS_COLOR       // enc3 (pins 0,1) → Color
#define ENC_4_PARAM    -1  // DISABLED (Coarse)  // enc4 (pins 8,9) → Coarse now menu-only
#define ENC_5_PARAM    8   // BRAIDS_FILTER_FREQ // enc5 (pins 6,7) → Filter Cutoff
#define ENC_6_PARAM    9   // BRAIDS_FILTER_RES  // enc6 (pins 25,27) → Filter Resonance
#define ENC_7_PARAM    10  // BRAIDS_FILTER_STR  // enc7 (pins 12,24) → Filter Strength
#define ENC_8_PARAM    -1  // DISABLED           // enc8 (pins 10,11) → Available for LFO
#define ENC_9_PARAM    4   // BRAIDS_AMP_ATTACK  // enc9 (pins 29,30) → Amp Attack
#define ENC_10_PARAM   5   // BRAIDS_AMP_DECAY   // enc10 (pins 28,26) → Amp Decay
#define ENC_11_PARAM   6   // BRAIDS_AMP_SUSTAIN // enc11 (pins 21,20) → Amp Sustain
#define ENC_13_PARAM   7   // BRAIDS_AMP_RELEASE // enc13 (pins 34,33) → Amp Release
#define ENC_14_PARAM   11  // BRAIDS_FILT_ATTACK // enc14 (pins 50,41) → Filter Attack
#define ENC_15_PARAM   12  // BRAIDS_FILT_DECAY  // enc15 (pins 23,22) → Filter Decay
#define ENC_16_PARAM   13  // BRAIDS_FILT_SUSTAIN// enc16 (pins 36,35) → Filter Sustain
#define ENC_17_PARAM   14  // BRAIDS_FILT_RELEASE// enc17 (pins 31,32) → Filter Release
#define ENC_18_PARAM   15  // BRAIDS_VOLUME      // enc18 (pins 17,16) → Volume
#define ENC_19_PARAM   -1  // DISABLED           // enc19 (pins 38,37) → Available for LFO
#define ENC_20_PARAM   -1  // DISABLED           // enc20 (pins 40,39) → Available for LFO

// ============================================================================
// Menu Encoder Parameter Configuration
// ============================================================================
// Set which parameter the menu encoder controls when NOT in menu mode
// Use -1 for menu-only mode (turning encoder auto-enters menu, no button press needed)
// Available parameters: 0-15 (see list above) 
#define MENU_ENCODER_PARAM  -1  // Shape (traditional - button press to enter menu)

// EXAMPLES:
// #define MENU_ENCODER_PARAM  7  // Filter Cutoff (button press to enter menu) 
// #define MENU_ENCODER_PARAM  -1  // Menu-only (turn encoder to auto-enter menu)

// TODO: Test DIN MIDI
// Uncomment to enable DIN MIDI support (requires moving enc3 from pin 0)
// #define ENABLE_DIN_MIDI

/*
 * DIN MIDI Setup Instructions:
 * 
 * HARDWARE REQUIRED:
 * - 6N138 optocoupler IC
 * - 220Ω resistor  
 * - 5-pin DIN MIDI connector
 * - Standard MIDI interface circuit (see MIDI specification)
 * 
 * WIRING:
 * 1. Build MIDI input circuit: DIN connector → 6N138 optocoupler → 220Ω resistor
 * 2. Connect MIDI circuit output to Teensy Serial1 RX (Pin 0)
 * 3. IMPORTANT: Move enc3 (Color) CLK wire from Pin 0 to surface mount pin (42-47)
 * 
 * USAGE:
 * - Install "MIDI Library" by Francois Best via Arduino Library Manager
 * - Uncomment #define ENABLE_DIN_MIDI above
 * - Can work with both USB Device MIDI (default) and USB Host MIDI
 * - Supports USB and DIN MIDI simultaneously
 * - Uses same MIDI channel setting from Settings menu
 * - Receives Note On/Off, Control Change, and Pitch Bend
 */

#endif // CONFIG_H