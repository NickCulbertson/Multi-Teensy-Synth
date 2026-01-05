#ifndef MENU_NAVIGATION_H
#define MENU_NAVIGATION_H

#include "config.h"
#include <Arduino.h>

// Note: NUM_PARAMETERS and NUM_PRESETS are expected to be defined
// in the main project file before including this header

// ============================================================================
// Menu State Enums
// ============================================================================

enum MenuState {
  PARENT_MENU,
  FM_MENU,
  SETTINGS,
  BANKS, 
  PATCHES,  
  
  // FM parameter sub-menus
  FM_ALGORITHM,
  FM_FEEDBACK,
  FM_LFO_SPEED,
  FM_MASTER_VOL,
  FM_OP1_LEVEL,
  FM_OP2_LEVEL,
  FM_OP3_LEVEL,
  FM_OP4_LEVEL,
  FM_OP5_LEVEL,
  FM_OP6_LEVEL,
  
  // Settings sub-menus
  MIDI_CHANNEL
};

// ============================================================================
// Data Structures
// ============================================================================

struct FMPreset {
  const char* name;
  float parameters[10]; // FM has 10 parameters
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

extern int currentBank;    
extern int BankIndex;      
extern int PatchIndex;     // Patch browser index (0-32)
extern const char* BankNames[];   // Bank names (size determined by sysex2c.py)

extern void loadPreset(int presetIndex);
void printCurrentPresetValues();

#endif // MENU_NAVIGATION_H