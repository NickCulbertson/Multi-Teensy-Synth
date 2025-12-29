#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// Hardware Configuration - EPiano Teensy Synth
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

// ============================================================================
// EPIANO ENCODER MAPPING (Based on MiniTeensy Box Labels)
// Maps physical encoders to EPiano parameter indices for sensible layout
// ============================================================================

// EPiano Parameters:
// 0: MDA_EP_DECAY - Note decay time
// 1: MDA_EP_RELEASE - Note release time  
// 2: MDA_EP_HARDNESS - Velocity sensitivity and timbre
// 3: MDA_EP_TREBLE - High frequency boost
// 4: MDA_EP_PAN_TREM - Stereo panning and tremolo effects
// 5: MDA_EP_LFO_RATE - Modulation speed
// 6: MDA_EP_VELOCITY_SENSE - How velocity affects sound
// 7: MDA_EP_STEREO - Stereo width control
// 8: MDA_EP_MAX_POLY - Polyphony (fixed at 16)
// 9: MDA_EP_TUNE - Master tuning
// 10: MDA_EP_DETUNE - Slight detuning for chorus effect
// 11: MDA_EP_OVERDRIVE - Tube-style saturation

#define ENC_1_PARAM    0   // DECAY           // enc1 → Decay (Envelope control)
#define ENC_2_PARAM    1   // RELEASE         // enc2 → Release (Envelope control)  
#define ENC_3_PARAM    2   // HARDNESS        // enc3 → Hardness (Timbre/Touch)
#define ENC_4_PARAM    3   // TREBLE          // enc4 → Treble (Tone control)
#define ENC_5_PARAM    4   // PAN_TREM        // enc5 → Pan/Tremolo (Modulation)
#define ENC_6_PARAM    5   // LFO_RATE        // enc6 → LFO Rate (Modulation speed)
#define ENC_7_PARAM    6   // VELOCITY_SENSE  // enc7 → Velocity Sense (Touch response)
#define ENC_8_PARAM    7   // STEREO          // enc8 → Stereo Width (Imaging)
#define ENC_9_PARAM    9   // TUNE            // enc9 → Master Tune (Pitch)
#define ENC_10_PARAM   10  // DETUNE          // enc10 → Detune (Chorus effect)
#define ENC_11_PARAM   11  // OVERDRIVE       // enc11 → Overdrive (Saturation)
#define ENC_13_PARAM   -1  // DISABLED        // enc13 → Disabled
#define ENC_14_PARAM   -1  // DISABLED        // enc14 → Disabled
#define ENC_15_PARAM   -1  // DISABLED        // enc15 → Disabled
#define ENC_16_PARAM   -1  // DISABLED        // enc16 → Disabled
#define ENC_17_PARAM   -1  // DISABLED        // enc17 → Disabled
#define ENC_18_PARAM   -1  // DISABLED        // enc18 → Disabled
#define ENC_19_PARAM   -1  // DISABLED        // enc19 → Disabled
#define ENC_20_PARAM   -1  // DISABLED        // enc20 → Disabled

// ============================================================================
// Menu Encoder Parameter Configuration
// ============================================================================
// Set which parameter the menu encoder controls when NOT in menu mode
// Use -1 for menu-only mode (turning encoder auto-enters menu, no button press needed)
// Available parameters: 0-11 (see list above) 
#define MENU_ENCODER_PARAM  -1  // Treble (traditional filter-like control)

// Display Configuration (Choose ONE - comment out the other)
#define USE_LCD_DISPLAY
// #define USE_OLED_DISPLAY

// MIDI Configuration - Enable the MIDI sources you want to use
#define USE_USB_DEVICE_MIDI // USB Device MIDI for DAW/computer connection (default)
// #define USE_MIDI_HOST       // USB Host MIDI for external controllers connected to Teensy 

// TODO: Test DIN MIDI
// Uncomment to enable DIN MIDI support (requires moving enc3 from pin 0)
// #define ENABLE_DIN_MIDI

#endif // CONFIG_H