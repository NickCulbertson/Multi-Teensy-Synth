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
// Customize these if you have a different pin layout or want to remap encoders
// 
// COMMON CUSTOMIZATION SCENARIOS:
// 1. Different hardware build with different pin layout
// 2. Want to swap encoder positions physically 
// 3. Need to avoid pin conflicts (e.g. DIN MIDI on pin 0)
// 4. Building a custom control surface with fewer encoders
//
// Just change the pin numbers below to match your hardware!

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
// FM Knob Mapping Configuration
// ============================================================================
// Easily customize which physical knobs control which FM parameters!
// Just change the parameter numbers next to each encoder to remap your knobs.

// Available FM Parameters (0-9):
// 0: Algorithm (operator routing)
// 1: Feedback 
// 2: LFO Speed
// 3: LFO Pitch Depth
// 4: LFO Amp Depth  
// 5: Transpose
// 6: OP1 Output Level
// 7: OP2 Output Level
// 8: OP3 Output Level
// 9: OP4 Output Level

// MINI-TEENSY ENCODER MAPPING (19 encoders available!)
// Set any encoder to -1 to disable it, or assign multiple encoders to the same parameter
// Note: Mini-Teensy hardware has enc1-enc11, enc13-enc20 (no enc12)

#define ENC_1_PARAM    0   // Algorithm        // enc1 (pins 4,5) → Algorithm
#define ENC_2_PARAM    1   // Feedback         // enc2 (pins 2,3) → Feedback  
#define ENC_3_PARAM    2   // LFO Speed        // enc3 (pins 0,1) → LFO Speed
#define ENC_4_PARAM    3   // LFO Pitch        // enc4 (pins 8,9) → LFO Pitch Depth
#define ENC_5_PARAM    4   // LFO Amp          // enc5 (pins 6,7) → LFO Amp Depth
#define ENC_6_PARAM    5   // Transpose        // enc6 (pins 25,27) → Transpose
#define ENC_7_PARAM    6   // OP1 Level        // enc7 (pins 12,24) → OP1 Output Level
#define ENC_8_PARAM    7   // OP2 Level        // enc8 (pins 10,11) → OP2 Output Level
#define ENC_9_PARAM    8   // OP3 Level        // enc9 (pins 29,30) → OP3 Output Level
#define ENC_10_PARAM   9   // OP4 Level        // enc10 (pins 28,26) → OP4 Output Level
#define ENC_11_PARAM   -1  // Disabled         // enc11 (pins 21,20) → Disabled
#define ENC_13_PARAM   -1  // Disabled         // enc13 (pins 34,33) → Disabled
#define ENC_14_PARAM   -1  // Disabled         // enc14 (pins 50,41) → Disabled
#define ENC_15_PARAM   -1  // Disabled         // enc15 (pins 23,22) → Disabled
#define ENC_16_PARAM   -1  // Disabled         // enc16 (pins 36,35) → Disabled
#define ENC_17_PARAM   -1  // Disabled         // enc17 (pins 31,32) → Disabled
#define ENC_18_PARAM   -1  // Disabled         // enc18 (pins 17,16) → Disabled
#define ENC_19_PARAM   -1  // Disabled         // enc19 (pins 38,37) → Disabled
#define ENC_20_PARAM   -1  // Disabled         // enc20 (pins 40,39) → Disabled

// ============================================================================
// Menu Encoder Parameter Configuration
// ============================================================================
// Set which parameter the menu encoder controls when NOT in menu mode
// Use -1 for menu-only mode (turning encoder auto-enters menu, no button press needed)
// Available FM parameters: 0-9 (Algorithm, Feedback, LFO Speed, LFO Pitch, LFO Amp,
// Transpose, OP1 Level, OP2 Level, OP3 Level, OP4 Level)
#define MENU_ENCODER_PARAM  -1  // Menu-only mode (auto-enter menu on turn)

// EXAMPLES:
// #define MENU_ENCODER_PARAM  0   // Algorithm (traditional - button press to enter menu)
// #define MENU_ENCODER_PARAM  2   // LFO Speed (button press to enter menu)
// #define MENU_ENCODER_PARAM  6   // OP1 Level (button press to enter menu)
// #define MENU_ENCODER_PARAM  -1  // Menu-only (turn encoder to auto-enter menu)


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
 * 3. IMPORTANT: Move enc3 (LFO Speed) CLK wire from Pin 0 to surface mount pin (42-47)
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