#ifndef MENU_NAVIGATION_H
#define MENU_NAVIGATION_H

#include "config.h"
#include <Arduino.h>

// ============================================================================
// Braids Parameter Mapping Enums
// ============================================================================
enum BraidsParam {
  BRAIDS_SHAPE = 0,
  BRAIDS_TIMBRE = 1,
  BRAIDS_COLOR = 2,
  BRAIDS_COARSE = 3,
  BRAIDS_AMP_ATTACK = 4,
  BRAIDS_AMP_DECAY = 5,
  BRAIDS_AMP_SUSTAIN = 6,
  BRAIDS_AMP_RELEASE = 7,
  BRAIDS_FILTER_CUTOFF = 8,     
  BRAIDS_FILTER_RESONANCE = 9,
  BRAIDS_FILTER_STRENGTH = 10,
  BRAIDS_FILTER_ATTACK = 11,
  BRAIDS_FILTER_DECAY = 12,
  BRAIDS_FILTER_SUSTAIN = 13,
  BRAIDS_FILTER_RELEASE = 14,
  BRAIDS_VOLUME = 15,         
  BRAIDS_NONE = -1  // Use this to disable an encoder
};

// ============================================================================
// Menu State Enums
// ============================================================================

enum MenuState {
  PARENT_MENU,
  BRAIDS_MENU,
  ENVELOPES,
  FILTER,
  LFO,
  SETTINGS,
  
  // Braids parameter sub-menus
  MENU_BRAIDS_SHAPE,
  MENU_BRAIDS_TIMBRE,
  MENU_BRAIDS_COLOR,
  MENU_BRAIDS_COARSE,
  MENU_BRAIDS_VOLUME,
  
  // Amp Envelope sub-menus
  MENU_BRAIDS_AMP_ATTACK,
  MENU_BRAIDS_AMP_DECAY,
  MENU_BRAIDS_AMP_SUSTAIN,
  MENU_BRAIDS_AMP_RELEASE,
  
  // Filter sub-menus
  MENU_BRAIDS_FILTER_CUTOFF,
  MENU_BRAIDS_FILTER_RESONANCE,
  MENU_BRAIDS_FILTER_STRENGTH,
  MENU_BRAIDS_FILTER_ATTACK,
  MENU_BRAIDS_FILTER_DECAY,
  MENU_BRAIDS_FILTER_SUSTAIN,
  MENU_BRAIDS_FILTER_RELEASE,
  
  // LFO sub-menus
  MENU_BRAIDS_LFO_RATE,
  MENU_BRAIDS_LFO_TIMBRE,
  MENU_BRAIDS_LFO_COLOR,
  MENU_BRAIDS_LFO_PITCH,
  MENU_BRAIDS_LFO_FILTER,
  MENU_BRAIDS_LFO_VOLUME,
  
  // Settings sub-menus
  MIDI_CHANNEL
};

// ============================================================================
// Data Structures
// ============================================================================

struct BraidsPreset {
  const char* name;
  float parameters[22]; // Fixed size array to match NUM_PARAMETERS
};

// ============================================================================
// External Variables
// ============================================================================
extern const int encoderMapping[19];

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
void resetEncoderBaselines();

// Preset functions
void loadPreset(int presetIndex);
void printCurrentPresetValues();
const char* getPresetName(int presetIndex);

// Parameter adjustment functions
void updateEncoderParameter(int paramIndex, int change);

#endif // MENU_NAVIGATION_H