#ifndef MENU_NAVIGATION_H
#define MENU_NAVIGATION_H

#include "config.h"
#include <Arduino.h>

// Define NUM_PARAMETERS if not already defined
#ifndef NUM_PARAMETERS
#define NUM_PARAMETERS 13
#endif

// ============================================================================
// Menu State Enums - EPiano Specific
// ============================================================================

enum MenuState {
  PARENT_MENU,
  PARAMETERS,
  PRESETS,
  SETTINGS,
  PARAM_EDIT
};

// ============================================================================
// EPiano Preset Structure
// ============================================================================

struct EPianoPreset {
  char name[20];
  float parameters[13]; // 12 EPiano parameters (0-11)
};

// ============================================================================
// External Variable Declarations
// ============================================================================

extern MenuState currentMenuState;
extern int menuIndex;
extern bool inMenu;
extern float allParameterValues[];
extern int currentPreset;
extern int currentEditParam;
extern const EPianoPreset epianoPresets[];

// ============================================================================
// Function Declarations
// ============================================================================

void initializeMenu();
void handleMenuEncoder();
void updateDisplay();
void displayText(String line1, String line2);
void enterMenu();
void exitMenu();
void selectMenuItem();
void incrementMenuIndex();
void decrementMenuIndex();
void backToParentMenu();
void handleEncoderChange(int encoderIndex, int change);
void updateEncoderParameter(int paramIndex, int change);
void updateParameterFromMenu(int paramIndex, float value);
void loadPreset(int presetIndex);
int getNumPresets();
void printCurrentPresetValues();

#endif // MENU_NAVIGATION_H