#include "config.h"

#define NUM_PARAMETERS 22  
#define NUM_PRESETS 12
#define VOICES 6

#include "MenuNavigation.h"
#include <Arduino.h>
#include <String.h>
#include <Encoder.h>

#ifdef USE_LCD_DISPLAY
  #include <LiquidCrystal_I2C.h>
#endif

#ifdef USE_OLED_DISPLAY
  #include <U8g2lib.h>
  #include <Wire.h>
#endif

#ifdef USE_LCD_DISPLAY
  extern LiquidCrystal_I2C lcd;
#endif

#ifdef USE_OLED_DISPLAY
  extern U8G2_SH1106_128X64_NONAME_F_HW_I2C display;
#endif

extern bool inMenu;
extern bool inPresetBrowse;
extern MenuState currentMenuState;
extern int menuIndex;
extern int presetBrowseIndex;
extern float braidsParameters[NUM_PARAMETERS];
extern const char* controlNames[];
extern const char* shapeNames[];
extern int midiChannel;
extern int currentPreset;
extern bool parameterChanged;

const char* parentMenuItems[] = {"Presets", "Parameters", "Settings", "< Exit"};
const char* braidsMenuItems[] = {"Shape", "Timbre", "Color", "Coarse", "Volume", "Envelopes", "Filter", "LFO", "< Back"};
const char* envelopeMenuItems[] = {"Amp Attack", "Amp Decay", "Amp Sustain", "Amp Release", "Filt Attack", "Filt Decay", "Filt Sustain", "Filt Release", "< Back"};
const char* filterMenuItems[] = {"Filter Cutoff", "Filter Res", "Filter Strength", "< Back"};
const char* lfoMenuItems[] = {"LFO Rate", "LFO>Timbre", "LFO>Color", "LFO>Pitch", "LFO>Filter", "LFO>Volume", "< Back"};
const char* settingsMenuItems[] = {"MIDI Channel", "< Back"};

extern long encoderValues[];
extern long lastEncoderValues[];
extern Encoder menuEncoder;
extern Encoder enc1, enc2, enc3, enc4, enc5, enc6, enc7, enc8, enc9, enc10, enc11, enc13, enc14, enc15, enc16, enc17, enc18, enc19, enc20;
extern void updateBraidsParameter(int paramIndex, float val);
extern const int encoderMapping[19];

const BraidsPreset braidsPresets[NUM_PRESETS] = {
  {"Supersaw", {9.0, 69.0, 61.0, 0.0, 0.0, 100.0, 50.0, 39.0, 75.1, 3.0, 127.0, 0.0, 36.0, 0.0, 36.0, 127.0, 3.0, 2.0, 2.0, 3.0, 0.0, 1.0}},   
  {"Chiptune", {10.0, 68.0, 62.0, 0.0, 0.0, 6.0, 70.0, 33.0, 72.1, 0.0, 127.0, 0.0, 36.0, 0.0, 36.0, 127.0, 3.0, 0.0, 0.0, 0.0, 0.0, 1.0}},
  {"CSaw", {0.0, 58.0, 54.0, 0.0, 0.0, 100.0, 49.0, 10.0, 81.1, 4.0, 127.0, 0.0, 4.0, 25.0, 4.0, 127.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0}},  
  {"Fold Bells", {3.0, 67.0, 21.0, 0.0, 0.0, 51.0, 39.0, 51.0, 69.1, 3.0, 127.0, 0.0, 51.0, 0.0, 51.0, 127.0, 25.0, 1.0, 0.0, 7.0, 13.0, 9.0}},  
  {"PWM Drift", {5.0, 52.0, 68.0, 0.0, 0.0, 100.0, 50.0, 40.0, 80.1, 4.0, 127.0, 2.0, 36.0, 0.0, 60.0, 127.0, 3.0, 44.0, 0.0, 6.0, 0.0, 0.0}},       
  {"Sine Pad", {12.0, 68.0, 62.0, 0.0, 18.0, 41.0, 71.0, 60.0, 84.1, 8.0, 127.0, 5.0, 37.0, 96.0, 60.0, 127.0, 2.0, 5.0, 8.0, 6.0, 19.0, 1.0}}, 
  {"Auto-Wah", {21.0, 63.0, 92.0, 0.0, 0.0, 41.0, 71.0, 60.0, 70.1, 5.0, 127.0, 0.0, 27.0, 0.0, 61.0, 127.0, 2.0, 57.0, 65.0, 6.0, 0.0, 1.0}},   
  {"Synth Strings", {27.0, 42.0, 85.0, 0.0, 2.0, 45.0, 81.0, 10.0, 109.0, 17.0, 127.0, 0.0, 7.0, 0.0, 51.0, 127.0, 34.0, 0.0, 0.0, 13.0, 0.0, 0.0}},   
  {"LFO Keys", {26.0, 45.0, 76.0, 0.0, 0.0, 61.0, 0.0, 19.0, 64.0, 6.0, 127.0, 0.0, 14.0, 1.0, 25.0, 127.0, 3.0, 7.0, 4.0, 5.0, 0.0, 0.0}},  
  {"Evolving Pad", {17.0, 40.0, 110.0, 0.0, 36.0, 72.0, 127.0, 82.0, 46.0, 12.0, 127.0, 52.0, 62.0, 71.0, 67.0, 127.0, 0.0, 24.0, 30.0, 5.0, 19.0, 0.0}},   
  {"Init Saw", {0.0, 64.0, 64.0, 0.0, 0.0, 7.0, 127.0, 2.0, 127.0, 0.0, 127.0, 0.0, 7.0, 0.0, 127.0, 127.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0}},       
  {"Init Supersaw", {9.0, 68.0, 61.0, 0.0, 0.0, 7.0, 127.0, 2.0, 127.0, 0.0, 127.0, 0.0, 7.0, 0.0, 127.0, 127.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0}}
};

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
  static unsigned long lastDisplayUpdate = 0;
  unsigned long now = millis();
  if (now - lastDisplayUpdate < 25) return; // Limit to 40Hz updates
  lastDisplayUpdate = now;
  String line1 = "";
  String line2 = "";
  if (inMenu) {
    if (inPresetBrowse) {
      line1 = "Presets";
      if (presetBrowseIndex == NUM_PRESETS) {
        line2 = "< Back";
      } else {
        line2 = String(presetBrowseIndex + 1) + ". " + String(braidsPresets[presetBrowseIndex].name);
      }
    } else {
      switch(currentMenuState) {
        case PARENT_MENU:
          line1 = "Menu";
          line2 = parentMenuItems[menuIndex];
          break;
        case BRAIDS_MENU:
          line1 = "Parameters";
          line2 = braidsMenuItems[menuIndex];
          break;
        case ENVELOPES:
          line1 = "Envelopes";
          line2 = envelopeMenuItems[menuIndex];
          break;
        case FILTER:
          line1 = "Filter";
          line2 = filterMenuItems[menuIndex];
          break;
        case LFO:
          line1 = "LFO";
          line2 = lfoMenuItems[menuIndex];
          break;
        case SETTINGS:
          line1 = "Settings";
          line2 = settingsMenuItems[menuIndex];
          break;
        case MIDI_CHANNEL:
          line1 = "MIDI Channel:";
          line2 = (midiChannel == 0) ? "Omni" : String(midiChannel);
          break;
        default:
          {
            int paramIndex = getParameterIndex(currentMenuState);
            if (paramIndex >= 0 && paramIndex < NUM_PARAMETERS) {
              line1 = controlNames[paramIndex];
              float val = braidsParameters[paramIndex];
              
              if (paramIndex == 0) { // Shape parameter - show algorithm name
                int shapeIndex = (int)val;
                line2 = String(shapeNames[shapeIndex]) + " (" + String(shapeIndex) + ")";
              } else if (paramIndex == 3) { // Coarse transpose
                int transpose = (int)val;
                line2 = String(transpose) + "st";
              } else if (paramIndex == 8) { // Filter Cutoff - show 0-127 range 
                int displayValue = (int)val;
                line2 = String(displayValue);
              } else if (paramIndex == 16) { // LFO Rate - show Hz 
                float rate = 0.1 + (val / 127.0) * 19.9;
                line2 = String(rate, 1) + " Hz";
              } else if (paramIndex >= 17 && paramIndex <= 21) { // LFO Depth parameters
                int percent = (int)(val / 127.0 * 100);
                line2 = String(percent) + "%";
              } else {
                // Standard 0-127 parameter display
                int displayValue = (int)val;
                line2 = String(displayValue);
              }
            }
          }
          break;
      }
    }
    displayText(line1, line2);
  } else {
    // Only show default message if no parameter was recently changed via MIDI
    if (!parameterChanged) {
      line1 = "MacroOSC Synth";
      line2 = "Press for menu";
      displayText(line1, line2);
    }
  }
}

int getParameterIndex(MenuState state) {
  switch(state) {
    case MENU_BRAIDS_SHAPE: return 0;
    case MENU_BRAIDS_TIMBRE: return 1;
    case MENU_BRAIDS_COLOR: return 2;
    case MENU_BRAIDS_COARSE: return 3;
    case MENU_BRAIDS_AMP_ATTACK: return 4;
    case MENU_BRAIDS_AMP_DECAY: return 5;
    case MENU_BRAIDS_AMP_SUSTAIN: return 6;
    case MENU_BRAIDS_AMP_RELEASE: return 7;
    case MENU_BRAIDS_FILTER_CUTOFF: return 8;    // Moved up, removed Filter Mode
    case MENU_BRAIDS_FILTER_RESONANCE: return 9;
    case MENU_BRAIDS_FILTER_STRENGTH: return 10;
    case MENU_BRAIDS_FILTER_ATTACK: return 11;
    case MENU_BRAIDS_FILTER_DECAY: return 12;
    case MENU_BRAIDS_FILTER_SUSTAIN: return 13;
    case MENU_BRAIDS_FILTER_RELEASE: return 14;
    case MENU_BRAIDS_VOLUME: return 15;          // Moved up to index 15
    case MENU_BRAIDS_LFO_RATE: return 16;
    case MENU_BRAIDS_LFO_TIMBRE: return 17;
    case MENU_BRAIDS_LFO_COLOR: return 18;
    case MENU_BRAIDS_LFO_PITCH: return 19;
    case MENU_BRAIDS_LFO_FILTER: return 20;
    case MENU_BRAIDS_LFO_VOLUME: return 21;
    default: return -1;
  }
}

void handleEncoder() {
#ifdef USE_OLED_DISPLAY
  long newMenuValue = menuEncoder.read() / 4; // Less sensitive for OLED encoder
#else
  long newMenuValue = menuEncoder.read() / 2; // Standard sensitivity for separate encoder
#endif
  static long oldMenuValue = 0;
  if (newMenuValue != oldMenuValue) {
    // If menu encoder is set to -1 (menu-only mode), auto-enter menu on rotation
    if (!inMenu && MENU_ENCODER_PARAM == -1) {
      inMenu = true;
      currentMenuState = PARENT_MENU;
      menuIndex = 0;
      inPresetBrowse = false;
      printCurrentPresetValues();
      updateDisplay();
    }
    
    if (inMenu) {
      if (inPresetBrowse) {
        if (newMenuValue > oldMenuValue) {
          presetBrowseIndex++;
          if (presetBrowseIndex > NUM_PRESETS) { // NUM_PRESETS = "Back" option
            presetBrowseIndex = 0;
          }
        } else {
          presetBrowseIndex--;
          if (presetBrowseIndex < 0) {
            presetBrowseIndex = NUM_PRESETS; // Wrap to "Back"
          }
        }
        updateDisplay();
      } else if (getParameterIndex(currentMenuState) >= 0) {
        int paramIndex = getParameterIndex(currentMenuState);
        if (newMenuValue > oldMenuValue) {
          // Adjust Braids parameter with proper increment
          if (paramIndex == 0) { // Shape parameter (0-42)
            braidsParameters[paramIndex] = constrain(braidsParameters[paramIndex] + 1.0, 0.0, 42.0);
          } else {
            // Standard 0-127 parameter (Filter Cutoff now at index 8)
            braidsParameters[paramIndex] = constrain(braidsParameters[paramIndex] + 1.0, 0.0, 127.0);
          }
          updateParameterFromMenu(paramIndex, braidsParameters[paramIndex]);
        } else {
          // Adjust Braids parameter with proper decrement
          if (paramIndex == 0) { // Shape parameter (0-42)
            braidsParameters[paramIndex] = constrain(braidsParameters[paramIndex] - 1.0, 0.0, 42.0);
          } else {
            // Standard 0-127 parameter (Filter Cutoff now at index 8)
            braidsParameters[paramIndex] = constrain(braidsParameters[paramIndex] - 1.0, 0.0, 127.0);
          }
          updateParameterFromMenu(paramIndex, braidsParameters[paramIndex]);
        }
        updateDisplay();
      } else {
        // In navigation mode: move through menu options
        if (newMenuValue > oldMenuValue) {
          incrementMenuIndex();
        } else {
          decrementMenuIndex();
        }
        updateDisplay();
      }
    } else {
      // When not in menu and menu encoder is assigned to a parameter, handle it like other encoders
      if (MENU_ENCODER_PARAM >= 0 && MENU_ENCODER_PARAM < NUM_PARAMETERS) {
#ifdef USE_OLED_DISPLAY
        encoderValues[11] = menuEncoder.read() / 4; // Adjusted sensitivity for OLED encoder
#else
        encoderValues[11] = menuEncoder.read() / 2; // Standard sensitivity for separate encoder
#endif
      }
    }
    oldMenuValue = newMenuValue;
  }
  
  static bool lastButtonState = HIGH;
  static unsigned long lastPressTime = 0;
  
  bool currentButtonState = digitalRead(MENU_ENCODER_SW);
  unsigned long now = millis();
  
  // Simple debouncing: only register press if enough time has passed since last press
  if (currentButtonState == LOW && lastButtonState == HIGH && (now - lastPressTime) > 50) {
    lastPressTime = now;
    
    if (!inMenu) {
      inMenu = true;
      currentMenuState = PARENT_MENU;
      menuIndex = 0;
      inPresetBrowse = false;
      printCurrentPresetValues();
      updateDisplay();
    } else if (inPresetBrowse) {
      if (presetBrowseIndex == NUM_PRESETS) {
        inPresetBrowse = false;
      } else {
        loadPreset(presetBrowseIndex);
      }
      updateDisplay();
    } else {
      if (getParameterIndex(currentMenuState) >= 0) {
        backMenuAction();
      } else {
        navigateMenuForward();
      }
      updateDisplay();
    }
  }
  
  lastButtonState = currentButtonState;
}

void navigateMenuForward() {
  switch(currentMenuState) {
    case PARENT_MENU:
      if (menuIndex == 0) {
        inPresetBrowse = true;
        presetBrowseIndex = 0;
        return; 
      }
      else if (menuIndex == 1) currentMenuState = BRAIDS_MENU;
      else if (menuIndex == 2) currentMenuState = SETTINGS;
      else if (menuIndex == 3) {
        inMenu = false;
        inPresetBrowse = false;
        return;
      }
      menuIndex = 0;
      break;
    case BRAIDS_MENU:
      if (menuIndex == 0) {
        currentMenuState = MENU_BRAIDS_SHAPE;
        menuIndex = 0;
        return;
      }
      else if (menuIndex == 1) {
        currentMenuState = MENU_BRAIDS_TIMBRE;
        menuIndex = 0;
        return;
      }
      else if (menuIndex == 2) {
        currentMenuState = MENU_BRAIDS_COLOR;
        menuIndex = 0;
        return;
      }
      else if (menuIndex == 3) {
        currentMenuState = MENU_BRAIDS_COARSE;
        menuIndex = 0;
        return;
      }
      else if (menuIndex == 4) {
        currentMenuState = MENU_BRAIDS_VOLUME;
        menuIndex = 0;
        return;
      }
      else if (menuIndex == 5) {
        currentMenuState = ENVELOPES;
        menuIndex = 0;
        return;
      }
      else if (menuIndex == 6) {
        currentMenuState = FILTER;
        menuIndex = 0;
        return;
      }
      else if (menuIndex == 7) {
        currentMenuState = LFO;
        menuIndex = 0;
        return;
      }
      else if (menuIndex == 8) {
        currentMenuState = PARENT_MENU;
        menuIndex = 1;
        return;
      }
      break;
    case ENVELOPES:
      if (menuIndex == 0) {
        currentMenuState = MENU_BRAIDS_AMP_ATTACK;
        menuIndex = 0;
      }
      else if (menuIndex == 1) {
        currentMenuState = MENU_BRAIDS_AMP_DECAY;
        menuIndex = 0;
      }
      else if (menuIndex == 2) {
        currentMenuState = MENU_BRAIDS_AMP_SUSTAIN;
        menuIndex = 0;
      }
      else if (menuIndex == 3) {
        currentMenuState = MENU_BRAIDS_AMP_RELEASE;
        menuIndex = 0;
      }
      else if (menuIndex == 4) {
        currentMenuState = MENU_BRAIDS_FILTER_ATTACK;
        menuIndex = 0;
      }
      else if (menuIndex == 5) {
        currentMenuState = MENU_BRAIDS_FILTER_DECAY;
        menuIndex = 0;
      }
      else if (menuIndex == 6) {
        currentMenuState = MENU_BRAIDS_FILTER_SUSTAIN;
        menuIndex = 0;
      }
      else if (menuIndex == 7) {
        currentMenuState = MENU_BRAIDS_FILTER_RELEASE;
        menuIndex = 0;
      }
      else if (menuIndex == 8) {
        currentMenuState = BRAIDS_MENU;
        menuIndex = 5;
        return;
      }
      break;
    case FILTER:
      if (menuIndex == 0) {
        currentMenuState = MENU_BRAIDS_FILTER_CUTOFF;
        menuIndex = 0;
      }
      else if (menuIndex == 1) {
        currentMenuState = MENU_BRAIDS_FILTER_RESONANCE;
        menuIndex = 0;
      }
      else if (menuIndex == 2) {
        currentMenuState = MENU_BRAIDS_FILTER_STRENGTH;
        menuIndex = 0;
      }
      else if (menuIndex == 3) {
        currentMenuState = BRAIDS_MENU;
        menuIndex = 6;
        return;
      }
      break;
    case LFO:
      if (menuIndex == 0) {
        currentMenuState = MENU_BRAIDS_LFO_RATE;
        menuIndex = 0;
      }
      else if (menuIndex == 1) {
        currentMenuState = MENU_BRAIDS_LFO_TIMBRE;
        menuIndex = 0;
      }
      else if (menuIndex == 2) {
        currentMenuState = MENU_BRAIDS_LFO_COLOR;
        menuIndex = 0;
      }
      else if (menuIndex == 3) {
        currentMenuState = MENU_BRAIDS_LFO_PITCH;
        menuIndex = 0;
      }
      else if (menuIndex == 4) {
        currentMenuState = MENU_BRAIDS_LFO_FILTER;
        menuIndex = 0;
      }
      else if (menuIndex == 5) {
        currentMenuState = MENU_BRAIDS_LFO_VOLUME;
        menuIndex = 0;
      }
      else if (menuIndex == 6) {
        currentMenuState = BRAIDS_MENU;
        menuIndex = 7;
        return;
      }
      break;
    case SETTINGS:
      if (menuIndex == 0) {
        currentMenuState = MIDI_CHANNEL;
        menuIndex = 0;
      }
      else if (menuIndex == 1) {
        currentMenuState = PARENT_MENU;
        menuIndex = 2;
        return;
      }
      break;
    default:
      break;
  }
}

void incrementMenuIndex() {
  switch(currentMenuState) {
    case PARENT_MENU:
      menuIndex++;
      if (menuIndex > 3) menuIndex = 0; // Parent menu: Presets, Braids, Settings, Exit
      break;
    case BRAIDS_MENU:
      menuIndex++;
      if (menuIndex > 8) menuIndex = 0; // Braids has 9 items (0-8) including Back
      break;
    case ENVELOPES:
      menuIndex++;
      if (menuIndex > 8) menuIndex = 0; // Envelopes has 9 items (0-8) including Back (Amp + Filter envelopes)
      break;
    case FILTER:
      menuIndex++;
      if (menuIndex > 3) menuIndex = 0; // Filter has 4 items (0-3) including Back
      break;
    case LFO:
      menuIndex++;
      if (menuIndex > 6) menuIndex = 0; // LFO has 7 items (0-6) including Back
      break;
    case SETTINGS:
      menuIndex++;
      if (menuIndex > 1) menuIndex = 0; // Settings has 2 items (0-1) including Back
      break;
    default:
      break;
  }
}

void decrementMenuIndex() {
  switch(currentMenuState) {
    case PARENT_MENU:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 3; // Parent menu: Presets, Braids, Settings, Exit
      break;
    case BRAIDS_MENU:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 8; // Braids has 9 items (0-8) including Back
      break;
    case ENVELOPES:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 8; // Envelopes has 9 items (0-8) including Back (Amp + Filter envelopes)
      break;
    case FILTER:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 3; // Filter has 4 items (0-3) including Back
      break;
    case LFO:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 6; // LFO has 7 items (0-6) including Back
      break;
    case SETTINGS:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 1; // Settings has 2 items (0-1) including Back
      break;
    default:
      break;
  }
}

void navigateMenuBackward() {
  switch(currentMenuState) {
    case BRAIDS_MENU:
      currentMenuState = PARENT_MENU;
      menuIndex = 1; // BRAIDS_MENU position
      break;
    case ENVELOPES:
      currentMenuState = BRAIDS_MENU;
      menuIndex = 5; // ENVELOPES position in Braids menu
      break;
    case FILTER:
      currentMenuState = BRAIDS_MENU;
      menuIndex = 6; // FILTER position in Braids menu
      break;
    case LFO:
      currentMenuState = BRAIDS_MENU;
      menuIndex = 7; // LFO position in Braids menu
      break;
    case SETTINGS:
      currentMenuState = PARENT_MENU;
      menuIndex = 2; // SETTINGS position
      break;
    default:
      break;
  }
}

void backMenuAction() {
  switch(currentMenuState) {
    // Braids core parameters go back to BRAIDS_MENU
    case MENU_BRAIDS_SHAPE:
    case MENU_BRAIDS_TIMBRE:
    case MENU_BRAIDS_COLOR:
    case MENU_BRAIDS_COARSE:
    case MENU_BRAIDS_VOLUME:
      {
        MenuState previousState = currentMenuState;
        currentMenuState = BRAIDS_MENU;
        
        if (previousState == MENU_BRAIDS_SHAPE) menuIndex = 0;
        else if (previousState == MENU_BRAIDS_TIMBRE) menuIndex = 1;
        else if (previousState == MENU_BRAIDS_COLOR) menuIndex = 2;
        else if (previousState == MENU_BRAIDS_COARSE) menuIndex = 3;
        else if (previousState == MENU_BRAIDS_VOLUME) menuIndex = 4;
        break;
      }
    
    // All envelope parameters (amp + filter) go back to ENVELOPES submenu
    case MENU_BRAIDS_AMP_ATTACK:
    case MENU_BRAIDS_AMP_DECAY:
    case MENU_BRAIDS_AMP_SUSTAIN:
    case MENU_BRAIDS_AMP_RELEASE:
    case MENU_BRAIDS_FILTER_ATTACK:
    case MENU_BRAIDS_FILTER_DECAY:
    case MENU_BRAIDS_FILTER_SUSTAIN:
    case MENU_BRAIDS_FILTER_RELEASE:
      {
        MenuState previousState = currentMenuState;
        currentMenuState = ENVELOPES;
        
        if (previousState == MENU_BRAIDS_AMP_ATTACK) menuIndex = 0;
        else if (previousState == MENU_BRAIDS_AMP_DECAY) menuIndex = 1;
        else if (previousState == MENU_BRAIDS_AMP_SUSTAIN) menuIndex = 2;
        else if (previousState == MENU_BRAIDS_AMP_RELEASE) menuIndex = 3;
        else if (previousState == MENU_BRAIDS_FILTER_ATTACK) menuIndex = 4;
        else if (previousState == MENU_BRAIDS_FILTER_DECAY) menuIndex = 5;
        else if (previousState == MENU_BRAIDS_FILTER_SUSTAIN) menuIndex = 6;
        else if (previousState == MENU_BRAIDS_FILTER_RELEASE) menuIndex = 7;
        break;
      }
    
    // Filter parameters go back to FILTER submenu
    case MENU_BRAIDS_FILTER_CUTOFF:
    case MENU_BRAIDS_FILTER_RESONANCE:
    case MENU_BRAIDS_FILTER_STRENGTH:
      {
        MenuState previousState = currentMenuState;
        currentMenuState = FILTER;
        
        if (previousState == MENU_BRAIDS_FILTER_CUTOFF) menuIndex = 0;
        else if (previousState == MENU_BRAIDS_FILTER_RESONANCE) menuIndex = 1;
        else if (previousState == MENU_BRAIDS_FILTER_STRENGTH) menuIndex = 2;
        break;
      }
    
    // LFO parameters go back to LFO submenu
    case MENU_BRAIDS_LFO_RATE:
    case MENU_BRAIDS_LFO_TIMBRE:
    case MENU_BRAIDS_LFO_COLOR:
    case MENU_BRAIDS_LFO_PITCH:
    case MENU_BRAIDS_LFO_FILTER:
    case MENU_BRAIDS_LFO_VOLUME:
      {
        MenuState previousState = currentMenuState;
        currentMenuState = LFO;
        
        if (previousState == MENU_BRAIDS_LFO_RATE) menuIndex = 0;
        else if (previousState == MENU_BRAIDS_LFO_TIMBRE) menuIndex = 1;
        else if (previousState == MENU_BRAIDS_LFO_COLOR) menuIndex = 2;
        else if (previousState == MENU_BRAIDS_LFO_PITCH) menuIndex = 3;
        else if (previousState == MENU_BRAIDS_LFO_FILTER) menuIndex = 4;
        else if (previousState == MENU_BRAIDS_LFO_VOLUME) menuIndex = 5;
        break;
      }
    
    case MIDI_CHANNEL: 
      currentMenuState = SETTINGS;
      menuIndex = 0;
      break;
    default:
      navigateMenuBackward();
      break;
  }
}

void updateParameterFromMenu(int paramIndex, float val) {
  updateBraidsParameter(paramIndex, val);
  // Display update removed - now handled once per loop in main()
}

void resetEncoderBaselines() {
  for (int i = 0; i < 19; i++) {
    int paramIndex = encoderMapping[i]; // Use configurable mapping
    
    // Only reset encoders that are mapped to valid parameters
    if (paramIndex >= 0 && paramIndex < NUM_PARAMETERS) {
      long targetEncoderValue = (long)(braidsParameters[paramIndex]);
      switch(i) {
        case 0: enc1.write(targetEncoderValue * 4); break;
        case 1: enc2.write(targetEncoderValue * 4); break;
        case 2: enc3.write(targetEncoderValue * 4); break;
        case 3: enc4.write(targetEncoderValue * 4); break;
        case 4: enc5.write(targetEncoderValue * 4); break;
        case 5: enc6.write(targetEncoderValue * 4); break;
        case 6: enc7.write(targetEncoderValue * 4); break;
        case 7: enc8.write(targetEncoderValue * 4); break;
        case 8: enc9.write(targetEncoderValue * 4); break;
        case 9: enc10.write(targetEncoderValue * 4); break;
        case 10: enc11.write(targetEncoderValue * 4); break;
        case 11: enc13.write(targetEncoderValue * 4); break;
        case 12: enc14.write(targetEncoderValue * 4); break;
        case 13: enc15.write(targetEncoderValue * 4); break;
        case 14: enc16.write(targetEncoderValue * 4); break;
        case 15: enc17.write(targetEncoderValue * 4); break;
        case 16: enc18.write(targetEncoderValue * 4); break;
        case 17: enc19.write(targetEncoderValue * 4); break;
        case 18: enc20.write(targetEncoderValue * 4); break;
      }
      encoderValues[i] = targetEncoderValue;
      lastEncoderValues[i] = targetEncoderValue;
    }
  }
}

void loadPreset(int presetNum) {
  presetNum = constrain(presetNum, 0, NUM_PRESETS - 1);
  currentPreset = presetNum;
  
  const BraidsPreset& preset = braidsPresets[presetNum];
  
  // Update parameter array to match preset values (all 22 parameters)
  for (int i = 0; i < NUM_PARAMETERS; i++) {
    braidsParameters[i] = preset.parameters[i];
  }
  
  // Apply all parameter values to synthesis engine
  for (int i = 0; i < NUM_PARAMETERS; i++) {
    updateBraidsParameter(i, braidsParameters[i]);
  }
  
  Serial.print("Loaded Braids preset ");
  Serial.print(presetNum + 1);
  Serial.print(": ");
  Serial.println(preset.name);
}

void printCurrentPresetValues() {
  Serial.println("\n=== CURRENT BRAIDS PRESET DEBUG ===");
  Serial.print("Active Preset: ");
  Serial.print(currentPreset + 1);
  Serial.print(" (");
  Serial.print(braidsPresets[currentPreset].name);
  Serial.println(")");
  
  Serial.println("\nCurrent Braids Parameter Values:");
  Serial.print("{");
  for (int i = 0; i < NUM_PARAMETERS; i++) {
    Serial.print(braidsParameters[i], 1); // 1 decimal place for Braids values
    if (i < NUM_PARAMETERS - 1) Serial.print(", ");
  }
  Serial.println("}");
  Serial.println("Copy this line into your Braids parameter array!");
  
  Serial.println("\nKey Parameters:");
  Serial.print("Shape: "); Serial.print((int)braidsParameters[0]);
  Serial.print(" | Timbre: "); Serial.print((int)braidsParameters[1]);
  Serial.print(" | Color: "); Serial.println((int)braidsParameters[2]);
  Serial.print("Coarse: "); Serial.print((int)braidsParameters[3]);
  Serial.print(" | Filter Cutoff: "); Serial.print((int)braidsParameters[8]);  // Moved to index 8
  Serial.print(" | Volume: "); Serial.println((int)braidsParameters[15]);     // Moved to index 15
  
  Serial.println("\nEncoder Raw Values:");
  for (int i = 0; i < 5; i++) {
    Serial.print("Enc");
    Serial.print(i+1);
    Serial.print(": ");
    Serial.print(encoderValues[i]);
    Serial.print(" -> ");
    int paramIndex = encoderMapping[i];
    if (paramIndex >= 0 && paramIndex < NUM_PARAMETERS) {
      Serial.println((int)braidsParameters[paramIndex]);
    } else {
      Serial.println("[DISABLED]");
    }
  }
  Serial.println("============================\n");
  
  Serial.println("Type 'r' in Serial Monitor to reset encoder baselines to current values");
}

const char* getPresetName(int presetIndex) {
  if (presetIndex >= 0 && presetIndex < NUM_PRESETS) {
    return braidsPresets[presetIndex].name;
  }
  return "Unknown";
}