#include "MenuNavigation.h"

#ifdef USE_LCD_DISPLAY
#include <LiquidCrystal_I2C.h>
extern LiquidCrystal_I2C lcd;
#endif

#ifdef USE_OLED_DISPLAY
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
extern Adafruit_SSD1306 display;
#endif

#include <Encoder.h>

// ============================================================================
// External Variables
// ============================================================================

extern Encoder menuEncoder;
extern bool parameterChanged;

// ============================================================================
// Menu Variables
// ============================================================================

MenuState currentMenuState = PARENT_MENU;
int menuIndex = 0;
bool inMenu = false;
int currentEditParam = -1;

// ============================================================================
// EPiano Built-in Presets (from MDA EPiano engine)
// ============================================================================

const EPianoPreset epianoPresets[] = {
  {"Default",  {0.500f, 0.500f, 0.500f, 0.500f, 0.500f, 0.650f, 0.250f, 0.500f, 16.0f, 0.500f, 0.146f, 0.000f}},
  {"Bright",   {0.500f, 0.500f, 1.000f, 0.800f, 0.500f, 0.650f, 0.250f, 0.500f, 16.0f, 0.500f, 0.146f, 0.500f}},
  {"Mellow",   {0.500f, 0.500f, 0.000f, 0.000f, 0.500f, 0.650f, 0.250f, 0.500f, 16.0f, 0.500f, 0.246f, 0.000f}},
  {"Autopan",  {0.500f, 0.500f, 0.500f, 0.500f, 0.250f, 0.650f, 0.250f, 0.500f, 16.0f, 0.500f, 0.246f, 0.000f}},
  {"Tremolo",  {0.500f, 0.500f, 0.500f, 0.500f, 0.750f, 0.650f, 0.250f, 0.500f, 16.0f, 0.500f, 0.246f, 0.000f}}
};

// ============================================================================
// Menu Structure
// ============================================================================

const char* parentMenuItems[] = {"Parameters", "Presets", "Settings", "< Exit"};
const char* presetMenuItems[] = {"Default", "Bright", "Mellow", "Autopan", "Tremolo", "< Back"};
const char* settingsMenuItems[] = {"MIDI Channel", "< Back"};

// All EPiano parameters for unified parameter menu
const char* parameterMenuItems[] = {
  "Decay", "Release", "Hardness", "Treble", "Pan/Tremolo", 
  "LFO Rate", "Velocity", "Stereo", "Polyphony", "Master Tune", 
  "Detune", "Overdrive", "< Back"
};

// Parameter names for display
const char* controlNames[] = {
  "Decay", "Release", "Hardness", "Treble", "Pan/Tremolo", 
  "LFO Rate", "Velocity", "Stereo", "Polyphony", "Master Tune", 
  "Detune", "Overdrive", "MIDI Channel"
};

// ============================================================================
// Function Implementations
// ============================================================================

void initializeMenu() {
  currentMenuState = PARENT_MENU;
  menuIndex = 0;
  inMenu = false;
}

void displayText(String line1, String line2) {
#ifdef USE_LCD_DISPLAY
  lcd.clear();
  delayMicroseconds(500);
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);  
  lcd.print(line2);
#endif

#ifdef USE_OLED_DISPLAY
  display.clearBuffer();
  display.setFont(u8g2_font_8x13_tf);
  if (line1.length() > 0) {
    display.drawStr(3, 20, line1.c_str());
  }
  if (line2.length() > 0) {
    display.drawStr(3, 40, line2.c_str());
  }
  display.sendBuffer();
#endif
}

void updateDisplay() {
  String line1 = "";
  String line2 = "";
  
  if (inMenu) {
    switch(currentMenuState) {
      case PARENT_MENU:
        line1 = "Main Menu";
        line2 = parentMenuItems[menuIndex];
        break;
      case PARAMETERS:
        line1 = "Parameters";
        line2 = parameterMenuItems[menuIndex];
        break;
      case PRESETS:
        line1 = "Presets";
        line2 = presetMenuItems[menuIndex];
        break;
      case SETTINGS:
        line1 = "Settings";
        line2 = settingsMenuItems[menuIndex];
        break;
      case PARAM_EDIT:
        if (currentEditParam >= 0 && currentEditParam < 13) {
          line1 = controlNames[currentEditParam];
          float val = allParameterValues[currentEditParam];
          
          // Special display formatting for certain parameters
          if (currentEditParam == 9) { // Master Tune
            float cents = (val - 0.5) * 100; // -50 to +50 cents
            line2 = (cents >= 0 ? "+" : "") + String((int)cents) + "c";
          } else if (currentEditParam == 10) { // Detune  
            float detune = val * 20; // 0 to 20 cents
            line2 = String(detune, 1) + "c";
          } else if (currentEditParam == 12) { // MIDI Channel
            extern int midiChannel;
            line2 = (midiChannel == 0) ? "Omni" : String(midiChannel);
          } else {
            int displayValue = (int)(val * 127); // 0-127 scale
            line2 = String(displayValue);
          }
        }
        break;
    }
    displayText(line1, line2);
  } else {
    // Only show default message if no parameter was recently changed via MIDI
    if (!parameterChanged) {
      line1 = "EPiano Synth";
      line2 = "Press for menu";
      displayText(line1, line2);
    }
  }
}

void enterMenu() {
  inMenu = true;
  currentMenuState = PARENT_MENU;
  menuIndex = 0;
  updateDisplay();
}

void exitMenu() {
  inMenu = false;
  updateDisplay();
}

void selectMenuItem() {
  switch(currentMenuState) {
    case PARENT_MENU:
      switch(menuIndex) {
        case 0: // Parameters
          currentMenuState = PARAMETERS;
          menuIndex = 0;
          break;
        case 1: // Presets
          currentMenuState = PRESETS;
          menuIndex = 0;
          break;
        case 2: // Settings
          currentMenuState = SETTINGS;
          menuIndex = 0;
          break;
        case 3: // Exit
          exitMenu();
          return;
      }
      break;
      
    case PARAMETERS:
      if (menuIndex < 12) { // Parameter edit (0-11)
        currentEditParam = menuIndex;
        currentMenuState = PARAM_EDIT;
      } else { // Back
        backToParentMenu();
      }
      break;
      
    case PRESETS:
      if (menuIndex < 5) { // Preset 0-4
        loadPreset(menuIndex);
        // Stay in preset menu (don't exit)
      } else { // Back
        backToParentMenu();
      }
      break;
      
    case SETTINGS:
      if (menuIndex < 1) { // MIDI Channel edit
        currentEditParam = 12; // MIDI Channel parameter
        currentMenuState = PARAM_EDIT;
      } else { // Back
        backToParentMenu();
      }
      break;
      
    case PARAM_EDIT:
      // Exit parameter edit mode and go back to parameter list
      currentMenuState = PARAMETERS;
      menuIndex = currentEditParam; // Return to the parameter that was being edited
      currentEditParam = -1;
      break;
  }
  updateDisplay();
}

void backToParentMenu() {
  currentMenuState = PARENT_MENU;
  menuIndex = 0;
}

void incrementMenuIndex() {
  switch(currentMenuState) {
    case PARENT_MENU:
      menuIndex++;
      if (menuIndex > 3) menuIndex = 0; // 4 items: Parameters, Presets, Settings, Exit
      break;
    case PARAMETERS:
      menuIndex++;
      if (menuIndex > 12) menuIndex = 0; // 13 items: 12 parameters + Back
      break;
    case PRESETS:
      menuIndex++;
      if (menuIndex > 5) menuIndex = 0; // 6 items: 5 presets + Back
      break;
    case SETTINGS:
      menuIndex++;
      if (menuIndex > 1) menuIndex = 0; // 2 items: MIDI Channel + Back
      break;
    case PARAM_EDIT:
      // Handle parameter value changes
      if (currentEditParam >= 0) {
        updateEncoderParameter(currentEditParam, 1);
      }
      break;
  }
}

void decrementMenuIndex() {
  switch(currentMenuState) {
    case PARENT_MENU:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 3; // 4 items: Parameters, Presets, Settings, Exit
      break;
    case PARAMETERS:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 12; // 13 items: 12 parameters + Back
      break;
    case PRESETS:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 5; // 6 items: 5 presets + Back
      break;
    case SETTINGS:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 1; // 2 items: MIDI Channel + Back
      break;
    case PARAM_EDIT:
      // Handle parameter value changes
      if (currentEditParam >= 0) {
        updateEncoderParameter(currentEditParam, -1);
      }
      break;
  }
}


void loadPreset(int presetIndex) {
  if (presetIndex >= 0 && presetIndex < getNumPresets()) {
    for (int i = 0; i < 12; i++) {
      allParameterValues[i] = epianoPresets[presetIndex].parameters[i];
      updateParameterFromMenu(i, allParameterValues[i]);
    }
    currentPreset = presetIndex;
  }
}

int getNumPresets() {
  return sizeof(epianoPresets) / sizeof(epianoPresets[0]);
}

void updateEncoderParameter(int paramIndex, int change) {
  if (paramIndex < 0) return; // Disabled encoder
  
  float increment = 1.0/128.0; // Fine control for all EPiano parameters
  
  if (change > 0) {
    allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] + increment, 0.0, 1.0);
  } else {
    allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] - increment, 0.0, 1.0);
  }
  
  updateParameterFromMenu(paramIndex, allParameterValues[paramIndex]);
  
  if (!inMenu) {
    String line1 = controlNames[paramIndex];
    String line2 = "";
    
    // Special display formatting for certain parameters
    if (paramIndex == 9) { // Master Tune
      float cents = (allParameterValues[paramIndex] - 0.5) * 100; // -50 to +50 cents
      line2 = (cents >= 0 ? "+" : "") + String((int)cents) + "c";
    } else if (paramIndex == 10) { // Detune  
      float detune = allParameterValues[paramIndex] * 20; // 0 to 20 cents
      line2 = String(detune, 1) + "c";
    } else if (paramIndex == 12) { // MIDI Channel
      extern int midiChannel;
      line2 = (midiChannel == 0) ? "Omni" : String(midiChannel);
    } else {
      int displayValue = (int)(allParameterValues[paramIndex] * 127); // 0-127 scale
      line2 = String(displayValue);
    }
    
    displayText(line1, line2);
  }
}

void handleEncoderChange(int encoderIndex, int change) {
  // Map encoder index to parameter based on config
  int paramIndex = -1;
  
  switch(encoderIndex) {
    case 1: paramIndex = ENC_1_PARAM; break;
    case 2: paramIndex = ENC_2_PARAM; break;
    case 3: paramIndex = ENC_3_PARAM; break;
    case 4: paramIndex = ENC_4_PARAM; break;
    case 5: paramIndex = ENC_5_PARAM; break;
    case 6: paramIndex = ENC_6_PARAM; break;
    case 7: paramIndex = ENC_7_PARAM; break;
    case 8: paramIndex = ENC_8_PARAM; break;
    case 9: paramIndex = ENC_9_PARAM; break;
    case 10: paramIndex = ENC_10_PARAM; break;
    case 11: paramIndex = ENC_11_PARAM; break;
    case 13: paramIndex = ENC_13_PARAM; break;
    case 14: paramIndex = ENC_14_PARAM; break;
    case 15: paramIndex = ENC_15_PARAM; break;
    case 16: paramIndex = ENC_16_PARAM; break;
    case 17: paramIndex = ENC_17_PARAM; break;
    case 18: paramIndex = ENC_18_PARAM; break;
    case 19: paramIndex = ENC_19_PARAM; break;
    case 20: paramIndex = ENC_20_PARAM; break;
  }
  
  if (paramIndex >= 0) {
    updateEncoderParameter(paramIndex, change);
  }
}

// resetEncoderBaselines() is implemented in main sketch