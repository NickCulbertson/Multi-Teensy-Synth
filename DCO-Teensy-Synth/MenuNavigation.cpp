#include "config.h"

#define NUM_PARAMETERS 31
#define NUM_PRESETS 20
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
extern float allParameterValues[NUM_PARAMETERS];
extern const char* controlNames[];
extern bool macroMode;
extern int midiChannel;
extern int playMode;
extern float glideTime;
extern int currentPreset;
// Juno-60 specific variables
extern float pwmVolume, pwmWidth, sawVolume, subVolume, noiseVolume;
extern float lfoRate, lfoDelay, lfoPWMAmount, lfoPitchAmount, lfoFilterAmount;
extern float hpfCutoff, lpfCutoff, resonance, filterEnvAmount;
extern float filtAttack, filtDecay, filtSustain, filtRelease;
extern float ampAttack, ampDecay, ampSustain, ampRelease;
extern int chorusMode;
const char* parentMenuItems[] = {"Presets", "VCO", "Sub+Noise", "LFO", "Filter", "Envelopes", "Chorus Mode", "Voice Mode", "Settings", "< Exit"};
const char* vcoMenuItems[] = {"PWM Volume", "PWM Width", "Saw Volume", "< Back"};
const char* subNoiseMenuItems[] = {"Sub Volume", "Noise Volume", "< Back"};
const char* lfoMenuItems[] = {"Rate", "Delay", "LFO>PWM", "LFO>Pitch", "LFO>Filter", "< Back"};
const char* filterMenuItems[] = {"HPF Cutoff", "LPF Cutoff", "Resonance", "Filter Env", "< Back"};
const char* envelopeMenuItems[] = {"Filt Attack", "Filt Decay", "Filt Sustain", "Filt Release", "Amp Attack", "Amp Decay", "Amp Sustain", "Amp Release", "< Back"};
const char* voiceModeMenuItems[] = {"Play Mode", "Glide Time", "< Back"};
const char* settingsMenuItems[] = {"Macro Knobs", "MIDI Channel", "< Back"};
extern long encoderValues[];
extern long lastEncoderValues[];
extern Encoder menuEncoder;
extern Encoder enc1, enc2, enc3, enc4, enc5, enc6, enc7, enc8, enc9, enc10, enc11, enc13, enc14, enc15, enc16, enc17, enc18, enc19, enc20;
extern void updateSynthParameter(int paramIndex, float val);
extern int currentPreset;


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
        line2 = String(presetBrowseIndex + 1) + ". " + String(presets[presetBrowseIndex].name);
      }
    } else {
      switch(currentMenuState) {
        case PARENT_MENU:
          line1 = "Menu";
          line2 = parentMenuItems[menuIndex];
          break;
        case VCO:
          line1 = "VCO";
          line2 = vcoMenuItems[menuIndex];
          break;
        case SUB_NOISE:
          line1 = "Sub+Noise";
          line2 = subNoiseMenuItems[menuIndex];
          break;
        case CHORUS_MODE: {
          line1 = "Chorus Mode";
          const char* modeNames[] = {"OFF", "Chorus I", "Chorus II", "Chorus I+II"};
          line2 = modeNames[chorusMode];
          break;
        }
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
        case VOICE_MODE:
          line1 = "Voice Mode";
          line2 = voiceModeMenuItems[menuIndex];
          break;
        case SETTINGS:
          line1 = "Settings";
          line2 = settingsMenuItems[menuIndex];
          break;
        case MACRO_KNOBS:
          line1 = "Filter Knobs:";
          line2 = macroMode ? "LFO Controls" : "Filter Env";
          break;
        case MIDI_CHANNEL:
          line1 = "MIDI Channel:";
          line2 = (midiChannel == 0) ? "Omni" : String(midiChannel);
          break;
        default:
          {
            int paramIndex = getParameterIndex(currentMenuState);
            if (paramIndex >= 0) {
              line1 = controlNames[paramIndex];
              if (paramIndex == 3 || paramIndex == 4 || paramIndex == 20) { // Extended fine tuning
                float val = allParameterValues[paramIndex];
                if (val <= 0.25) {
                  float semiRange = val / 0.25;
                  int semitones = (int)(-12 + (semiRange * 11)); // -12 to -1
                  line2 = String(semitones) + "st";
                } else if (val >= 0.75) {
                  float semiRange = (val - 0.75) / 0.25;
                  int semitones = (int)(1 + (semiRange * 11)); // +1 to +12
                  line2 = "+" + String(semitones) + "st";
                } else {
                  int cents = (int)((val - 0.5) * 100); // -25 to +25 cents
                  line2 = (cents >= 0 ? "+" : "") + String(cents) + "c";
                }
              } else if (paramIndex == 5) { // LFO Rate - show frequency
                float val = allParameterValues[paramIndex];
                float frequency = 0.1 + val * 19.9; // 0.1 to 20 Hz
                line2 = String(frequency, 1) + " Hz";
              } else if (paramIndex == 22) { // Chorus Mode
                const char* modeNames[] = {"OFF", "Chorus I", "Chorus II", "Chorus I+II"};
                int currentMode;
                float val = allParameterValues[paramIndex];
                if (val < 0.25f) currentMode = 0;
                else if (val < 0.5f) currentMode = 1;
                else if (val < 0.75f) currentMode = 2;
                else currentMode = 3;
                line2 = modeNames[currentMode];
              } else if (paramIndex == 23) { // Reserved
                line2 = "---";
              } else if (paramIndex == 24) { // Reserved  
                line2 = "---";
              } else if (paramIndex == 25) { // Play Mode
                if (playMode == 0) line2 = "Mono";
                else if (playMode == 1) line2 = "Poly";
                else line2 = "Legato";
              } else if (paramIndex == 26) { // Glide Time
                if (glideTime == 0.0) {
                  line2 = "OFF";
                } else {
                  float timeMs = 50 + (glideTime * 950); // 50ms to 1000ms
                  line2 = String((int)timeMs) + "ms";
                }
              } else if (paramIndex == 27) { // Reserved
                line2 = "---";
              } else if (paramIndex == 28) { // Reserved
                line2 = "---";
              } else if (paramIndex == 29) { // Reserved
                line2 = "---";
              } else if (paramIndex == 30) { // MIDI Channel
                line2 = (midiChannel == 0) ? "Omni" : String(midiChannel);
              } else if (paramIndex == 22) { // Chorus Mode
                const char* modeNames[] = {"OFF", "Chorus I", "Chorus II", "Chorus I+II"};
                line2 = modeNames[chorusMode];
              } else {
                int displayValue = (int)(allParameterValues[paramIndex] * 127);
                line2 = String(displayValue);
              }
            }
          }
          break;
      }
    }
  } else {
    line1 = "Juno-Teensy";
    line2 = "Press for menu";
  }
  displayText(line1, line2);
}

int getParameterIndex(MenuState state) {
  switch(state) {
    // VCO parameters
    case PWM_VOLUME: return 0;
    case PWM_WIDTH: return 1;
    case SAW_VOLUME: return 2;
    
    // Sub+Noise parameters
    case SUB_VOLUME: return 3;
    case NOISE_VOLUME: return 4;
    
    // LFO parameters
    case LFO_RATE: return 5;
    case LFO_DELAY: return 6;
    case LFO_PWM_AMT: return 7;
    case LFO_PITCH_AMT: return 8;
    case LFO_FILTER_AMT: return 9;
    
    // Filter parameters
    case HPF_CUTOFF: return 10;
    case LPF_CUTOFF: return 11;
    case RESONANCE: return 12;
    case FILTER_ENV_AMT: return 13;
    
    // Filter Envelope parameters
    case FILTER_ATTACK: return 14;
    case FILTER_DECAY: return 15;
    case FILTER_SUSTAIN: return 16;
    case FILTER_RELEASE: return 17;
    
    // Amp Envelope parameters
    case AMP_ATTACK: return 18;
    case AMP_DECAY: return 19;
    case AMP_SUSTAIN: return 20;
    case AMP_RELEASE: return 21;
    
    // Chorus parameter
    case CHORUS_MODE: return 22;
    
    // Voice Mode parameters
    case PLAY_MODE: return 25;
    case GLIDE_TIME: return 26;
    
    // Settings parameters
    case MACRO_KNOBS: return 27;
    case MIDI_CHANNEL: return 28;
    
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
          if (paramIndex == 22) { // Chorus Mode - step through modes
            float currentVal = allParameterValues[paramIndex];
            if (currentVal < 0.25f) allParameterValues[paramIndex] = 0.375f;       // Off -> Chorus I
            else if (currentVal < 0.5f) allParameterValues[paramIndex] = 0.625f;   // Chorus I -> Chorus II
            else if (currentVal < 0.75f) allParameterValues[paramIndex] = 0.875f;  // Chorus II -> Chorus I+II
            else allParameterValues[paramIndex] = 0.125f;                          // Chorus I+II -> Off
          } else if (paramIndex == 24) { // LFO Toggle - instant toggle with single turn
            allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] + 0.5, 0.0, 1.0);
          } else if (paramIndex == 25) { // LFO Target - medium increment for 3 positions 
            allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] + 0.03, 0.0, 1.0);
          } else if (paramIndex == 26) { // Play Mode - large increment for 1-turn switching
            allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] + 0.05, 0.0, 1.0);
          } else if (paramIndex == 28) { // Noise Type - instant toggle with single turn
            allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] + 0.5, 0.0, 1.0);
          } else if (paramIndex == 29) { // Macro Mode - instant toggle with single turn
            allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] + 0.5, 0.0, 1.0);
          } else if (paramIndex == 30) { // MIDI Channel - step through channels
            allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] + (1.0/16.0), 0.0, 1.0);
          } else {
            allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] + 1.0/128.0, 0.0, 1.0);
          }
          updateParameterFromMenu(paramIndex, allParameterValues[paramIndex]);
        } else {
          if (paramIndex == 22) { // Chorus Mode - step through modes
            float currentVal = allParameterValues[paramIndex];
            if (currentVal < 0.25f) allParameterValues[paramIndex] = 0.875f;       // Off -> Chorus I+II
            else if (currentVal < 0.5f) allParameterValues[paramIndex] = 0.125f;   // Chorus I -> Off
            else if (currentVal < 0.75f) allParameterValues[paramIndex] = 0.375f;  // Chorus II -> Chorus I
            else allParameterValues[paramIndex] = 0.625f;                          // Chorus I+II -> Chorus II
          } else if (paramIndex == 24) { // LFO Toggle - instant toggle with single turn
            allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] - 0.5, 0.0, 1.0);
          } else if (paramIndex == 25) { // LFO Target - medium increment for 3 positions
            allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] - 0.03, 0.0, 1.0);
          } else if (paramIndex == 26) { // Play Mode - large increment for 1-turn switching
            allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] - 0.05, 0.0, 1.0);
          } else if (paramIndex == 28) { // Noise Type - instant toggle with single turn
            allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] - 0.5, 0.0, 1.0);
          } else if (paramIndex == 29) { // Macro Mode - instant toggle with single turn
            allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] - 0.5, 0.0, 1.0);
          } else if (paramIndex == 30) { // MIDI Channel - step through channels
            allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] - (1.0/16.0), 0.0, 1.0);
          } else {
            allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] - 1.0/128.0, 0.0, 1.0);
          }
          updateParameterFromMenu(paramIndex, allParameterValues[paramIndex]);
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
#ifdef USE_OLED_DISPLAY
      encoderValues[11] = menuEncoder.read() / 4; // Adjusted sensitivity for OLED encoder
#else
      encoderValues[11] = menuEncoder.read() / 2; // Standard sensitivity for separate encoder
#endif
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
      } else if (currentMenuState == MACRO_KNOBS) {
        macroMode = !macroMode;
      } else {
        navigateMenuForward();
      }
      updateDisplay();
    }
  }
  
  lastButtonState = currentButtonState;
}

// Fast button-only check (can be called more frequently)
void checkEncoderButton() {
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
      } else if (currentMenuState == MACRO_KNOBS) {
        macroMode = !macroMode;
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
      else if (menuIndex == 1) currentMenuState = VCO;
      else if (menuIndex == 2) currentMenuState = SUB_NOISE;
      else if (menuIndex == 3) currentMenuState = LFO;
      else if (menuIndex == 4) currentMenuState = FILTER;
      else if (menuIndex == 5) currentMenuState = ENVELOPES;
      else if (menuIndex == 6) currentMenuState = CHORUS_MODE;
      else if (menuIndex == 7) currentMenuState = VOICE_MODE;
      else if (menuIndex == 8) currentMenuState = SETTINGS;
      else if (menuIndex == 9) {
        inMenu = false;
        inPresetBrowse = false;
        return;
      }
      menuIndex = 0;
      break;
    case VCO:
      if (menuIndex == 0) currentMenuState = PWM_VOLUME;
      else if (menuIndex == 1) currentMenuState = PWM_WIDTH;
      else if (menuIndex == 2) currentMenuState = SAW_VOLUME;
      else if (menuIndex == 3) {
        currentMenuState = PARENT_MENU;
        menuIndex = 1; 
        return;
      }
      break;
    case SUB_NOISE:
      if (menuIndex == 0) currentMenuState = SUB_VOLUME;
      else if (menuIndex == 1) currentMenuState = NOISE_VOLUME;
      else if (menuIndex == 2) {
        currentMenuState = PARENT_MENU;
        menuIndex = 2;
        return;
      }
      break;
    case ENVELOPES:
      if (menuIndex == 0) currentMenuState = FILTER_ATTACK;
      else if (menuIndex == 1) currentMenuState = FILTER_DECAY;
      else if (menuIndex == 2) currentMenuState = FILTER_SUSTAIN;
      else if (menuIndex == 3) currentMenuState = FILTER_RELEASE;
      else if (menuIndex == 4) currentMenuState = AMP_ATTACK;
      else if (menuIndex == 5) currentMenuState = AMP_DECAY;
      else if (menuIndex == 6) currentMenuState = AMP_SUSTAIN;
      else if (menuIndex == 7) currentMenuState = AMP_RELEASE;
      else if (menuIndex == 8) {
        currentMenuState = PARENT_MENU;
        menuIndex = 5;
        return;
      }
      break;
    case FILTER:
      if (menuIndex == 0) currentMenuState = HPF_CUTOFF;
      else if (menuIndex == 1) currentMenuState = LPF_CUTOFF;
      else if (menuIndex == 2) currentMenuState = RESONANCE;
      else if (menuIndex == 3) currentMenuState = FILTER_ENV_AMT;
      else if (menuIndex == 4) {
        currentMenuState = PARENT_MENU;
        menuIndex = 4;
        return;
      }
      break;
    case LFO:
      if (menuIndex == 0) currentMenuState = LFO_RATE;
      else if (menuIndex == 1) currentMenuState = LFO_DELAY;
      else if (menuIndex == 2) currentMenuState = LFO_PWM_AMT;
      else if (menuIndex == 3) currentMenuState = LFO_PITCH_AMT;
      else if (menuIndex == 4) currentMenuState = LFO_FILTER_AMT;
      else if (menuIndex == 5) {
        currentMenuState = PARENT_MENU;
        menuIndex = 3;
        return;
      }
      break;
    case VOICE_MODE:
      if (menuIndex == 0) currentMenuState = PLAY_MODE;
      else if (menuIndex == 1) currentMenuState = GLIDE_TIME;
      else if (menuIndex == 2) {
        currentMenuState = PARENT_MENU;
        menuIndex = 7;
        return;
      }
      break;
    case SETTINGS:
      if (menuIndex == 0) currentMenuState = MACRO_KNOBS;
      else if (menuIndex == 1) currentMenuState = MIDI_CHANNEL;
      else if (menuIndex == 2) {
        currentMenuState = PARENT_MENU;
        menuIndex = 8;
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
      if (menuIndex > 9) menuIndex = 0; // Wrap to first item (now 9 items: 0-8)
      break;
    case VCO:
      menuIndex++;
      if (menuIndex > 3) menuIndex = 0; // VCO has 4 items (0-3) including Back
      break;
    case SUB_NOISE:
      menuIndex++;
      if (menuIndex > 2) menuIndex = 0; // Sub+Noise has 3 items (0-2) including Back
      break;
    case ENVELOPES:
      menuIndex++;
      if (menuIndex > 8) menuIndex = 0; // Envelopes has 9 items (0-8) including Back
      break;
    case FILTER:
      menuIndex++;
      if (menuIndex > 4) menuIndex = 0; // Filter has 5 items (0-4) including Back
      break;
    case LFO:
      menuIndex++;
      if (menuIndex > 5) menuIndex = 0; // LFO has 6 items (0-5) including Back
      break;
    case VOICE_MODE:
      menuIndex++;
      if (menuIndex > 2) menuIndex = 0; // Voice Mode has 3 items (0-2) including Back
      break;
    case SETTINGS:
      menuIndex++;
      if (menuIndex > 2) menuIndex = 0; // Settings has 3 items (0-2) including Back
      break;
    default:
      break;
  }
}

void decrementMenuIndex() {
  switch(currentMenuState) {
    case PARENT_MENU:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 9; // Wrap to last item (now 9 items: 0-8)
      break;
    case VCO:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 3; // VCO has 4 items (0-3) including Back
      break;
    case SUB_NOISE:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 2; // Sub+Noise has 3 items (0-2) including Back
      break;
    case ENVELOPES:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 8; // Envelopes has 9 items (0-8) including Back
      break;
    case FILTER:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 4; // Filter has 5 items (0-4) including Back
      break;
    case LFO:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 5; // LFO has 6 items (0-5) including Back
      break;
    case VOICE_MODE:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 2; // Voice Mode has 3 items (0-2) including Back
      break;
    case SETTINGS:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 2; // Settings has 3 items (0-2) including Back
      break;
    default:
      break;
  }
}

void navigateMenuBackward() {
  switch(currentMenuState) {
    case VCO:
      currentMenuState = PARENT_MENU;
      menuIndex = 1; // VCO position
      break;
    case SUB_NOISE:
      currentMenuState = PARENT_MENU;
      menuIndex = 2; // SUB_NOISE position
      break;
    case LFO:
      currentMenuState = PARENT_MENU;
      menuIndex = 3; // LFO position
      break;
    case FILTER:
      currentMenuState = PARENT_MENU;
      menuIndex = 4; // FILTER position
      break;
    case ENVELOPES:
      currentMenuState = PARENT_MENU;
      menuIndex = 5; // ENVELOPES position
      break;
    case CHORUS_MODE:
      currentMenuState = PARENT_MENU;
      menuIndex = 6; // CHORUS_MODE position
      break;
    case VOICE_MODE:
      currentMenuState = PARENT_MENU;
      menuIndex = 7; // VOICE_MODE position
      break;
    case SETTINGS:
      currentMenuState = PARENT_MENU;
      menuIndex = 8; // SETTINGS position
      break;
    default:
      break;
  }
}

void backMenuAction() {
  switch(currentMenuState) {
    // VCO parameters
    case PWM_VOLUME:
    case PWM_WIDTH:
    case SAW_VOLUME:
      currentMenuState = VCO;
      if (currentMenuState == PWM_VOLUME) menuIndex = 0;
      else if (currentMenuState == PWM_WIDTH) menuIndex = 1;
      else if (currentMenuState == SAW_VOLUME) menuIndex = 2;
      break;
    
    // Sub+Noise parameters
    case SUB_VOLUME:
    case NOISE_VOLUME:
      currentMenuState = SUB_NOISE;
      if (currentMenuState == SUB_VOLUME) menuIndex = 0;
      else if (currentMenuState == NOISE_VOLUME) menuIndex = 1;
      break;
    
    // LFO parameters
    case LFO_RATE:
    case LFO_DELAY:
    case LFO_PWM_AMT:
    case LFO_PITCH_AMT:
    case LFO_FILTER_AMT:
      currentMenuState = LFO;
      if (currentMenuState == LFO_RATE) menuIndex = 0;
      else if (currentMenuState == LFO_DELAY) menuIndex = 1;
      else if (currentMenuState == LFO_PWM_AMT) menuIndex = 2;
      else if (currentMenuState == LFO_PITCH_AMT) menuIndex = 3;
      else if (currentMenuState == LFO_FILTER_AMT) menuIndex = 4;
      break;
    
    // Filter parameters
    case HPF_CUTOFF:
    case LPF_CUTOFF:
    case RESONANCE:
    case FILTER_ENV_AMT:
      currentMenuState = FILTER;
      if (currentMenuState == HPF_CUTOFF) menuIndex = 0;
      else if (currentMenuState == LPF_CUTOFF) menuIndex = 1;
      else if (currentMenuState == RESONANCE) menuIndex = 2;
      else if (currentMenuState == FILTER_ENV_AMT) menuIndex = 3;
      break;
    
    // Envelope parameters
    case FILTER_ATTACK:
    case FILTER_DECAY:
    case FILTER_SUSTAIN:
    case FILTER_RELEASE:
    case AMP_ATTACK:
    case AMP_DECAY:
    case AMP_SUSTAIN:
    case AMP_RELEASE:
      currentMenuState = ENVELOPES;
      if (currentMenuState == FILTER_ATTACK) menuIndex = 0;
      else if (currentMenuState == FILTER_DECAY) menuIndex = 1;
      else if (currentMenuState == FILTER_SUSTAIN) menuIndex = 2;
      else if (currentMenuState == FILTER_RELEASE) menuIndex = 3;
      else if (currentMenuState == AMP_ATTACK) menuIndex = 4;
      else if (currentMenuState == AMP_DECAY) menuIndex = 5;
      else if (currentMenuState == AMP_SUSTAIN) menuIndex = 6;
      else if (currentMenuState == AMP_RELEASE) menuIndex = 7;
      break;
    
    // Chorus parameter (now direct edit)
    case CHORUS_MODE:
      currentMenuState = PARENT_MENU;
      menuIndex = 6; // CHORUS_MODE position
      break;
    
    // Voice Mode parameters
    case PLAY_MODE:
    case GLIDE_TIME:
      currentMenuState = VOICE_MODE;
      if (currentMenuState == PLAY_MODE) menuIndex = 0;
      else if (currentMenuState == GLIDE_TIME) menuIndex = 1;
      break;
    
    // Settings parameters
    case MACRO_KNOBS:
    case MIDI_CHANNEL:
      currentMenuState = SETTINGS;
      if (currentMenuState == MACRO_KNOBS) menuIndex = 0;
      else if (currentMenuState == MIDI_CHANNEL) menuIndex = 1;
      break;
    
    default:
      navigateMenuBackward();
      break;
  }
}

void loadPreset(int presetIndex) {
  if (presetIndex >= 0 && presetIndex < NUM_PRESETS) {
    currentPreset = presetIndex;
    for (int i = 0; i < NUM_PARAMETERS; i++) {
      allParameterValues[i] = presets[presetIndex].parameters[i];
      updateSynthParameter(i, allParameterValues[i]);
    }
    updateDisplay();
  }
}

void updateParameterFromMenu(int paramIndex, float val) {
  updateSynthParameter(paramIndex, val);
  updateDisplay();
}

void updateEncoderParameter(int paramIndex, int change) {
  float increment = 0.01; // Base increment
  switch (paramIndex) {
    // VCO Volume controls - moderate sensitivity 
    case 0: case 2: case 3: case 4: // PWM Volume, Saw Volume, Sub Volume, Noise Volume
      increment = 0.02; // Less sensitive for smooth volume control
      break;
    case 1: // PWM Width - fine control
      increment = 1.0/128.0; // Fine control for pulse width
      break;
    
    // LFO controls - fine control
    case 5: case 6: case 7: case 8: case 9: // LFO Rate, Delay, PWM Amt, Pitch Amt, Filter Amt
      increment = 1.0/128.0; 
      break;
    
    // Filter controls - fine control
    case 10: case 11: case 12: case 13: // HPF Cutoff, LPF Cutoff, Resonance, Filter Env Amt
      increment = 1.0/128.0;
      break;
    
    // Envelope controls - fine control
    case 14: case 15: case 16: case 17: case 18: case 19: case 20: case 21: // All envelope parameters
      increment = 1.0/128.0;
      break;
    
    // Chorus Mode - special discrete stepping
    case 22: // Chorus Mode
      increment = 0.0; // Special handling below - don't use normal increment
      break;
    
    // Toggle/discrete controls - instant response  
    case 25: case 27: case 28: // Play Mode, Macro Mode, MIDI Channel
      increment = 0.5; // Large steps for immediate toggle response
      break;
    
    // Chorus controls - moderate sensitivity
    case 23: case 24: // Chorus Rate, Chorus Depth
      increment = 0.02;
      break;
    
    // Glide Time - moderate sensitivity
    case 26: // Glide Time
      increment = 0.02;
      break;
    
    default:
      increment = 0.01; // Standard increment
      break;
  }
  
  // Special handling for Chorus Mode (discrete stepping)
  if (paramIndex == 22) { // Chorus Mode
    if (change > 0) { // Clockwise
      float currentVal = allParameterValues[paramIndex];
      if (currentVal < 0.25f) allParameterValues[paramIndex] = 0.375f;       // Off -> Chorus I
      else if (currentVal < 0.5f) allParameterValues[paramIndex] = 0.625f;   // Chorus I -> Chorus II
      else if (currentVal < 0.75f) allParameterValues[paramIndex] = 0.875f;  // Chorus II -> Chorus I+II
      else allParameterValues[paramIndex] = 0.125f;                          // Chorus I+II -> Off
    } else { // Counter-clockwise
      float currentVal = allParameterValues[paramIndex];
      if (currentVal < 0.25f) allParameterValues[paramIndex] = 0.875f;       // Off -> Chorus I+II
      else if (currentVal < 0.5f) allParameterValues[paramIndex] = 0.125f;   // Chorus I -> Off
      else if (currentVal < 0.75f) allParameterValues[paramIndex] = 0.375f;  // Chorus II -> Chorus I
      else allParameterValues[paramIndex] = 0.625f;                          // Chorus I+II -> Chorus II
    }
  } else {
    // Normal parameter handling
    allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] + (change * increment), 0.0, 1.0);
  }
  
  float val = allParameterValues[paramIndex];
  
  updateSynthParameter(paramIndex, val);
  if (!inMenu) {
    String line1 = controlNames[paramIndex];
    String line2 = "";
    
    if (paramIndex == 22) { // Chorus Mode - show mode names
      const char* modeNames[] = {"OFF", "Chorus I", "Chorus II", "Chorus I+II"};
      int currentMode;
      if (val < 0.25f) currentMode = 0;      // 0.0-0.249 = Off
      else if (val < 0.5f) currentMode = 1;  // 0.25-0.499 = Chorus I  
      else if (val < 0.75f) currentMode = 2; // 0.5-0.749 = Chorus II
      else currentMode = 3;                  // 0.75-1.0 = Chorus I+II
      line2 = modeNames[currentMode];
    } else if (paramIndex == 5) { // LFO Rate - show frequency in Hz
      float frequency = 0.1 + val * 19.9; // 0.1 to 20 Hz
      line2 = String(frequency, 1) + " Hz";
    } else {
      int displayValue = (int)(val * 127); // 0-127 MIDI scale
      line2 = String(displayValue);
    }
    
    displayText(line1, line2);
  }
}

void resetEncoderBaselines() {
  for (int i = 0; i < 20; i++) {
    int paramIndex = encoderMapping[i]; // Use configurable mapping
    
    // Only reset encoders that are mapped to valid parameters
    if (paramIndex != -1 && paramIndex >= 0 && paramIndex < NUM_PARAMETERS) {
      long targetEncoderValue = (long)(allParameterValues[paramIndex] * 100);
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
        case 11: /* menuEncoder handled by MenuNavigation.cpp */ break;
        case 12: enc13.write(targetEncoderValue * 4); break;
        case 13: enc14.write(targetEncoderValue * 4); break;
        case 14: enc15.write(targetEncoderValue * 4); break;
        case 15: enc16.write(targetEncoderValue * 4); break;
        case 16: enc17.write(targetEncoderValue * 4); break;
        case 17: enc18.write(targetEncoderValue * 4); break;
        case 18: enc19.write(targetEncoderValue * 4); break;
        case 19: enc20.write(targetEncoderValue * 4); break;
      }
      encoderValues[i] = targetEncoderValue;
      lastEncoderValues[i] = targetEncoderValue;
    }
  }
}

void printCurrentPresetValues() {
  Serial.println("\n=== CURRENT PRESET DEBUG ===");
  Serial.print("Active Preset: ");
  Serial.print(currentPreset + 1);
  Serial.print(" (");
  Serial.print(presets[currentPreset].name);
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
  Serial.print("Osc1 Range: "); Serial.print(allParameterValues[0], 3);
  Serial.print(" | Osc2 Range: "); Serial.print(allParameterValues[1], 3);
  Serial.print(" | Osc3 Range: "); Serial.println(allParameterValues[2], 3);
  Serial.print("Osc1 Wave: "); Serial.print(allParameterValues[5], 3);
  Serial.print(" | Osc2 Wave: "); Serial.print(allParameterValues[6], 3);
  Serial.print(" | Filter: "); Serial.println(allParameterValues[11], 3);
  
  Serial.println("\nEncoder Raw Values:");
  for (int i = 0; i < 5; i++) {
    Serial.print("Enc");
    Serial.print(i+1);
    Serial.print(": ");
    Serial.print(encoderValues[i]);
    Serial.print(" -> ");
    int paramIndex = encoderMapping[i];
    if (paramIndex != -1 && paramIndex >= 0 && paramIndex < NUM_PARAMETERS) {
      Serial.println(allParameterValues[paramIndex], 3);
    } else {
      Serial.println("[DISABLED]");
    }
  }
  Serial.println("=============================\n");
  
  Serial.println("Type 'r' in Serial Monitor to reset encoder baselines to current values");
}