#ifndef MENU_NAVIGATION_H
#define MENU_NAVIGATION_H

#include "config.h"
#include <Arduino.h>

// ============================================================================
// Menu State Enums
// ============================================================================

enum MenuState {
  PARENT_MENU,
  PARAMETERS,
  VOICE_MODE,
  SETTINGS,
  
  // Parameters sub-menus
  VCO,
  SUB_NOISE, 
  LFO,
  FILTER,
  ENVELOPES,
  CHORUS_MODE,
  
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

struct DCOTeensyPreset {
  const char* name;
  float parameters[31]; // Fixed size to match presets
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