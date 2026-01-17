#include "MenuNavigation.h"

#ifdef USE_LCD_DISPLAY
#include <LiquidCrystal_I2C.h>
extern LiquidCrystal_I2C lcd;
#endif

#ifdef USE_OLED_DISPLAY
  #include <U8g2lib.h>
  #include <Wire.h>
#endif

#include <Encoder.h>


extern Encoder menuEncoder;
extern bool parameterChanged;

#ifdef USE_OLED_DISPLAY
  extern U8G2_SH1106_128X64_NONAME_F_HW_I2C display;
#endif

MenuState currentMenuState = PARENT_MENU;
int menuIndex = 0;
bool inMenu = false;
int currentEditParam = -1;


const EPianoPreset epianoPresets[] = {
  {"Init",     {0.500f, 0.500f, 0.500f, 0.500f, 0.500f, 0.650f, 0.250f, 0.500f, 16.0f, 0.500f, 0.146f, 0.000f, 1.000f, 0.000f}},
  {"Dreamy",   {0.500f, 0.500f, 0.500f, 0.500f, 0.750f, 0.650f, 0.250f, 0.500f, 16.0f, 0.500f, 0.246f, 0.000f, 1.000f, 0.000f}},
  {"Bright",   {0.500f, 0.500f, 1.000f, 0.800f, 0.500f, 0.650f, 0.250f, 0.500f, 16.0f, 0.500f, 0.146f, 0.500f, 1.000f, 0.000f}},
  {"Mellow",   {0.500f, 0.500f, 0.000f, 0.000f, 0.500f, 0.650f, 0.250f, 0.500f, 16.0f, 0.500f, 0.246f, 0.000f, 1.000f, 0.000f}},
  {"Autopan",  {0.500f, 0.500f, 0.500f, 0.500f, 0.250f, 0.650f, 0.250f, 0.500f, 16.0f, 0.500f, 0.246f, 0.000f, 1.000f, 0.000f}},
  {"Tremolo",  {0.500f, 0.500f, 0.500f, 0.500f, 0.750f, 0.650f, 0.250f, 0.500f, 16.0f, 0.500f, 0.246f, 0.000f, 1.000f, 0.000f}},
  {"Chill",  {0.602f, 0.867f, 0.305f, 0.320f, 0.484f, 0.603f, 0.797f, 0.508f, 1.000f, 0.500f, 0.023f, 0.10f, 1.000f, 0.000f}},
  {"Felt",     {0.133f, 0.703f, 0.195f, 0.086f, 0.508f, 0.447f, 0.797f, 0.508f, 1.000f, 0.500f, 0.086f, 0.000f, 1.000f, 0.000f}},
  {"Bell",     {0.148f, 0.500f, 0.242f, 1.000f, 0.461f, 0.572f, 0.250f, 0.500f, 16.0f, 0.500f, 0.146f, 0.000f, 1.000f, 0.000f}},
  {"Overdrive",    {0.500, 0.500, 0.242, 0.750, 0.445, 0.556, 0.242, 0.500, 16.000, 0.500, 0.146, 0.701, 1.000f, 0.000}}
};


const char* parentMenuItems[] = {"Parameters", "Presets", "Settings", "< Exit"};
const char* presetMenuItems[] = {"Init", "Dreamy", "Bright", "Mellow", "Autopan", "Tremolo", "Chill", "Felt", "Bell", "Overdrive", "< Back"};
const char* settingsMenuItems[] = {"MIDI Channel", "< Back"};

// All EPiano parameters for unified parameter menu
const char* parameterMenuItems[] = {
  "Decay", "Release", "Hardness", "Treble", "Pan/Tremolo", 
  "LFO Rate", "Velocity", "Stereo", "Polyphony", "Master Tune", 
  "Detune", "Overdrive", "Volume", "< Back"
};

// Parameter names for display
const char* controlNames[] = {
  "Decay", "Release", "Hardness", "Treble", "Pan/Tremolo", 
  "LFO Rate", "Velocity", "Stereo", "Polyphony", "Master Tune", 
  "Detune", "Overdrive", "Volume", "MIDI Channel"
};


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
        if (currentEditParam >= 0 && currentEditParam < 14) {
          line1 = controlNames[currentEditParam];
          float val = allParameterValues[currentEditParam];
          
          // Special display formatting for certain parameters
          if (currentEditParam == 9) { // Master Tune
            float cents = (val - 0.5) * 100; // -50 to +50 cents
            line2 = (cents >= 0 ? "+" : "") + String((int)cents) + "c";
          } else if (currentEditParam == 10) { // Detune  
            float detune = val * 20; // 0 to 20 cents
            line2 = String(detune, 1) + "c";
          } else if (currentEditParam == 13) { // MIDI Channel
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
  printCurrentPresetValues();
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
      if (menuIndex < 13) { // Parameter edit (0-12)
        currentEditParam = menuIndex;
        currentMenuState = PARAM_EDIT;
      } else { // Back
        backToParentMenu();
      }
      break;
      
    case PRESETS:
      if (menuIndex < 10) { // Preset 0-4
        loadPreset(menuIndex);
        printCurrentPresetValues();
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
      if (menuIndex > 13) menuIndex = 0; // 14 items: 13 parameters + Back
      break;
    case PRESETS:
      menuIndex++;
      if (menuIndex > 10) menuIndex = 0; // 6 items: 5 presets + Back
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
      if (menuIndex < 0) menuIndex = 13; // 14 items: 13 parameters + Back
      break;
    case PRESETS:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 10; // 6 items: 5 presets + Back
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
    for (int i = 0; i < NUM_PARAMETERS; i++) {
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
    } else if (paramIndex == 13) { // MIDI Channel
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

void printCurrentPresetValues() {
  Serial.println("\n=== CURRENT PRESET DEBUG ===");
  Serial.print("Active Preset: ");
  Serial.print(currentPreset + 1);
  Serial.print(" (");
  Serial.print(epianoPresets[currentPreset].name);
  Serial.println(")");
  
  Serial.println("\nCurrent Parameter Values:");
  Serial.print("{");
  for (int i = 0; i < NUM_PARAMETERS; i++) {
    Serial.print(allParameterValues[i], 3); // 3 decimal places
    if (i < NUM_PARAMETERS - 1) Serial.print(", ");
  }
  Serial.println("}");
  Serial.println("Copy this line into your preset array!");
  
  Serial.println("\nKey Parameters:");
  Serial.print("Decay: "); Serial.print(allParameterValues[0], 3);
  Serial.print(" | Release: "); Serial.print(allParameterValues[1], 3);
  Serial.print(" | Hardness: "); Serial.println(allParameterValues[2], 3);
  Serial.print("Treble: "); Serial.print(allParameterValues[3], 3);
  Serial.print(" | Pan/Tremolo: "); Serial.print(allParameterValues[4], 3);
  Serial.print(" | LFO Rate: "); Serial.println(allParameterValues[5], 3);
  Serial.print("Volume: "); Serial.print(allParameterValues[12], 3);
  Serial.print(" | MIDI Ch: "); Serial.println(allParameterValues[13], 3);
  
  Serial.println("\nAll Parameters:");
  for (int i = 0; i < NUM_PARAMETERS; i++) {
    Serial.print(controlNames[i]); Serial.print(": ");
    Serial.print(allParameterValues[i], 3); Serial.print(" | ");
    if ((i + 1) % 3 == 0) Serial.println(); // New line every 3 parameters
  }
  Serial.println();
  Serial.println("=== DEBUG END ===\n");
}

const char* getPresetName(int presetIndex) {
  if (presetIndex >= 0 && presetIndex < getNumPresets()) {
    return epianoPresets[presetIndex].name;
  }
  return "Unknown";
}

// resetEncoderBaselines() is implemented in main sketch