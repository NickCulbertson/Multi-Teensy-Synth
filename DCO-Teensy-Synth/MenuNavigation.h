#ifndef MENU_NAVIGATION_H
#define MENU_NAVIGATION_H

#include "config.h"
#include <Arduino.h>

// ============================================================================
// Menu State Enums
// ============================================================================

enum MenuState {
  PARENT_MENU,
  VCO,
  SUB_NOISE, 
  LFO,
  FILTER,
  ENVELOPES,
  CHORUS_MODE,
  VOICE_MODE,
  SETTINGS,
  
  // VCO sub-menus
  PWM_VOLUME,
  PWM_WIDTH,
  SAW_VOLUME,
  
  // Sub+Noise sub-menus  
  SUB_VOLUME,
  NOISE_VOLUME,
  
  // LFO sub-menus
  LFO_RATE,
  LFO_DELAY,
  LFO_PWM_AMT,
  LFO_PITCH_AMT,
  LFO_FILTER_AMT,
  
  // Filter sub-menus
  HPF_CUTOFF,
  LPF_CUTOFF,
  RESONANCE,
  FILTER_ENV_AMT,
  
  // Envelope sub-menus (Juno-60 style - separate filter and amp envelopes)
  FILTER_ATTACK,
  FILTER_DECAY,
  FILTER_SUSTAIN,
  FILTER_RELEASE,
  AMP_ATTACK,
  AMP_DECAY,
  AMP_SUSTAIN,
  AMP_RELEASE,
  
  // Chorus is now a direct parameter edit (CHORUS_MODE above)
  
  // Voice Mode sub-menus
  PLAY_MODE,
  GLIDE_TIME,
  
  // Settings sub-menus
  MACRO_KNOBS,
  MIDI_CHANNEL
};

// ============================================================================
// Data Structures
// ============================================================================

struct MiniTeensyPreset {
  const char* name;
  float parameters[31]; // Fixed size to match presets
};

// ============================================================================
// Juno-60 Preset Definitions
// ============================================================================
const MiniTeensyPreset presets[] = {
  // Preset 0: Init Patch
  {"Init", {
    0.5,   // 0: PWM Volume
    0.5,   // 1: PWM Width
    0.8,   // 2: Saw Volume (main oscillator)
    0.0,   // 3: Sub Volume
    0.0,   // 4: Noise Volume
    0.05,  // 5: LFO Rate (0.5 Hz)
    0.0,   // 6: LFO Delay
    0.0,   // 7: LFO to PWM
    0.0,   // 8: LFO to Pitch
    0.0,   // 9: LFO to Filter
    0.01,  // 10: HPF Cutoff (20 Hz)
    0.75,  // 11: LPF Cutoff (6000 Hz)
    0.0,   // 12: Resonance
    0.5,   // 13: Filter Env Amount
    0.0,   // 14: Filter Attack
    0.04,  // 15: Filter Decay
    0.5,   // 16: Filter Sustain
    0.04,  // 17: Filter Release
    0.0,   // 18: Amp Attack
    0.02,  // 19: Amp Decay
    0.8,   // 20: Amp Sustain
    0.02,  // 21: Amp Release
    0.0,   // 22: Chorus Mode (off)
    0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.0  // 23-30: reserved/unused
  }},
  
  // Preset 1: Strings
  {"Strings", {
    0.6, 0.3, 0.8, 0.3, 0.0,     // PWM, Width, Saw, Sub, Noise
    0.08, 0.0, 0.0, 0.15, 0.2,   // LFO Rate, Delay, PWM, Pitch, Filter
    0.01, 0.6, 0.1, 0.7,         // HPF, LPF, Res, Filter Env
    0.3, 0.15, 0.9, 0.4,         // Filter ADSR
    0.4, 0.1, 0.8, 0.6,          // Amp ADSR
    0.35, // Chorus I
    0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.0
  }},
  
  // Preset 2: Brass
  {"Brass", {
    0.8, 0.6, 0.4, 0.0, 0.0,     // PWM, Width, Saw, Sub, Noise
    0.12, 0.0, 0.1, 0.0, 0.0,    // LFO Rate, Delay, PWM, Pitch, Filter
    0.01, 0.8, 0.3, 0.8,         // HPF, LPF, Res, Filter Env
    0.0, 0.08, 0.6, 0.2,         // Filter ADSR
    0.0, 0.05, 1.0, 0.15,        // Amp ADSR
    0.0,  // Chorus off
    0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.0
  }},
  
  // Preset 3: Hoover
  {"Hoover", {
    0.9, 0.8, 0.6, 0.4, 0.1,     // PWM, Width, Saw, Sub, Noise
    0.2, 0.0, 0.3, 0.4, 0.6,     // LFO Rate, Delay, PWM, Pitch, Filter
    0.01, 0.4, 0.8, 0.9,         // HPF, LPF, Res, Filter Env
    0.0, 0.2, 0.3, 0.1,          // Filter ADSR
    0.0, 0.1, 0.9, 0.2,          // Amp ADSR
    0.65, // Chorus II
    0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.0
  }},
  
  // Preset 4: Bass
  {"Bass", {
    0.3, 0.4, 0.9, 0.6, 0.0,     // PWM, Width, Saw, Sub, Noise
    0.0, 0.0, 0.0, 0.0, 0.0,     // LFO off
    0.01, 0.9, 0.0, 0.3,         // HPF, LPF, Res, Filter Env
    0.0, 0.03, 0.0, 0.05,        // Filter ADSR (punchy)
    0.0, 0.02, 0.6, 0.08,        // Amp ADSR
    0.0,  // Chorus off
    0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.0
  }},
  
  // Preset 5: Poly Sync
  {"Poly Sync", {
    0.7, 0.7, 0.8, 0.0, 0.0,     // PWM, Width, Saw, Sub, Noise
    0.15, 0.0, 0.2, 0.0, 0.4,    // LFO Rate, Delay, PWM, Pitch, Filter
    0.01, 0.7, 0.4, 0.6,         // HPF, LPF, Res, Filter Env
    0.0, 0.1, 0.5, 0.2,          // Filter ADSR
    0.0, 0.05, 0.8, 0.3,         // Amp ADSR
    0.35, // Chorus I
    0.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.0
  }}
};

// ============================================================================
// External Variables
// ============================================================================
extern const int encoderMapping[20];

// ============================================================================
// Menu Navigation Function Declarations
// ============================================================================

// Parameter mapping
int getParameterIndex(MenuState state);

// Display functions
void displayText(String line1, String line2);
void updateDisplay();

// Menu navigation functions
void handleEncoder();
void navigateMenuForward();
void navigateMenuBackward();
void incrementMenuIndex();
void decrementMenuIndex();
void backMenuAction();

// Parameter handling functions
void updateParameterFromMenu(int paramIndex, float val);
void updateEncoderParameter(int paramIndex, int change);

// Utility functions

// Preset functions
void loadPreset(int presetIndex);
void printCurrentPresetValues();

#endif // MENU_NAVIGATION_H