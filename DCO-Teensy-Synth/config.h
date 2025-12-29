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

// JUNO-60 ENCODER MAPPING (Based on MiniTeensy Box Labels)
// Maps physical encoders to Juno-60 parameter indices for sensible layout
#define ENC_1_PARAM    1   // PWM_WIDTH       // enc1 → PWM Width (Wave control)
#define ENC_2_PARAM    5   // LFO_RATE        // enc2 → LFO Rate  
#define ENC_3_PARAM    7   // LFO_PWM_AMT     // enc3 → LFO PWM Amount
#define ENC_4_PARAM    8   // LFO_PITCH_AMT   // enc4 → LFO Pitch Amount
#define ENC_5_PARAM    9   // LFO_FILTER_AMT  // enc5 → LFO Filter Amount
#define ENC_6_PARAM    10  // HPF_CUTOFF      // enc6 → High-pass Filter Cutoff
#define ENC_7_PARAM    12  // RESONANCE       // enc7 → Resonance
#define ENC_8_PARAM    22  // CHORUS_MODE     // enc8 → Chorus Mode
#define ENC_9_PARAM    0   // PWM_VOLUME      // enc9 → PWM Volume (Osc1 Volume)
#define ENC_10_PARAM   2   // SAW_VOLUME      // enc10 → Saw Volume (Osc2 Volume)
#define ENC_11_PARAM   3   // SUB_VOLUME      // enc11 → Sub Volume (Osc3 Volume)
#define ENC_13_PARAM   11  // LPF_CUTOFF      // enc13 → Low-pass Filter Cutoff
#define ENC_14_PARAM   18  // AMP_ATTACK      // enc14 → Amp Attack
#define ENC_15_PARAM   19  // AMP_DECAY       // enc15 → Amp Decay
#define ENC_16_PARAM   20  // AMP_SUSTAIN     // enc16 → Amp Sustain
#define ENC_17_PARAM   4   // NOISE_VOLUME    // enc17 → Noise Volume
#define ENC_18_PARAM   14  // FILTER_ATTACK   // enc18 → Filter Attack
#define ENC_19_PARAM   15  // FILTER_DECAY    // enc19 → Filter Decay
#define ENC_20_PARAM   21  // AMP_RELEASE     // enc20 → Amp Release

// Additional Juno-60 parameters available for menu control:
//   - 11: LPF Cutoff (Low-pass Filter) - MENU_ENCODER_PARAM
//   - 13: Filter Env Amount
//   - 19: Amp Decay
//   - 22: Chorus Bypass
//   - 23: Chorus Rate
//   - 24: Chorus Depth
//   - 25: Play Mode (Mono/Poly/Legato)
//   - 26: Glide Time (portamento speed)
//   - 27: Macro Mode (toggles macro assignments)
//   - 28: MIDI Channel (Omni/1-16)

// ============================================================================
// Menu Encoder Parameter Configuration
// ============================================================================
// Set which parameter the menu encoder controls when NOT in menu mode
// Use -1 for menu-only mode (turning encoder auto-enters menu, no button press needed)
// Available parameters: 0-28 (see list above) 
#define MENU_ENCODER_PARAM  11  // LPF Cutoff (traditional - button press to enter menu)

// EXAMPLES:
// #define MENU_ENCODER_PARAM  11  // LPF Cutoff (traditional - button press to enter menu)
// #define MENU_ENCODER_PARAM  13  // Filter Env Amount (button press to enter menu)
// #define MENU_ENCODER_PARAM  23  // Chorus Rate (button press to enter menu) 
// #define MENU_ENCODER_PARAM  -1  // Menu-only (turn encoder to auto-enter menu)


// Display Configuration (Choose ONE - comment out the other)
#define USE_LCD_DISPLAY
// #define USE_OLED_DISPLAY

// MIDI Configuration - Enable the MIDI sources you want to use
#define USE_USB_DEVICE_MIDI // USB Device MIDI for DAW/computer connection (default)
// #define USE_MIDI_HOST       // USB Host MIDI for external controllers connected to Teensy 

/*
 * EXAMPLE: DIN MIDI Pin Conflict Resolution
 * 
 * If you enable DIN MIDI, it uses pin 0 (Serial1 RX), which conflicts with ENC_3_CLK.
 * To resolve this, move enc3 to a different pin pair:
 * 
 * #define ENC_3_CLK    42     // Move enc3 to surface mount pins
 * #define ENC_3_DT     43     // (or any other available pins)
 * 
 * Then enable DIN MIDI below.
 */

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
 * 3. IMPORTANT: Move enc3 (Osc3 Range) CLK wire from Pin 0 to surface mount pin (42-47)
 * 
 * USAGE:
 * - Install "MIDI Library" by Francois Best via Arduino Library Manager
 * - Uncomment #define ENABLE_DIN_MIDI above
 * - Supports both USB and DIN MIDI simultaneously
 * - Uses same MIDI channel setting from Settings menu
 * - Receives Note On/Off, Control Change, and Pitch Bend
 */

#endif // CONFIG_H