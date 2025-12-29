#ifndef MENU_NAVIGATION_H
#define MENU_NAVIGATION_H

#include "config.h"
#include <Arduino.h>

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
  float parameters[12]; // 12 EPiano parameters (0-11)
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

// Functions implemented in main sketch
void resetEncoderBaselines();

#endif // MENU_NAVIGATION_H