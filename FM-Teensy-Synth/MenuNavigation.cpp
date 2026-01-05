#include "config.h"

#define NUM_PARAMETERS 10
#define NUM_PRESETS 32
#define VOICES 16

#include "MenuNavigation.h"
#include "roms_unpacked.h"
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
extern MenuState currentMenuState;
extern int menuIndex;
extern float allParameterValues[NUM_PARAMETERS];
extern const char* controlNames[];
extern int midiChannel;
extern bool parameterChanged;

extern int currentBank;
extern int BankIndex;
extern int PatchIndex;

const char* parentMenuItems[] = {"Presets", "Parameters", "Settings", "< Exit"};
const char* fmMenuItems[] = {"Algorithm", "Feedback", "LFO Speed", "Master Vol", "OP1 Level", "OP2 Level", "OP3 Level", "OP4 Level", "OP5 Level", "OP6 Level", "< Back"};
const char* settingsMenuItems[] = {"MIDI Channel", "< Back"};

extern long encoderValues[];
extern long lastEncoderValues[];
extern Encoder menuEncoder;
extern Encoder enc1, enc2, enc3, enc4, enc5, enc6, enc7, enc8, enc9, enc10, enc11, enc13, enc14, enc15, enc16, enc17, enc18, enc19, enc20;
extern void updateSynthParameter(int paramIndex, float val);
extern const int encoderMapping[19];

extern const char* BankNames[8];
extern uint8_t progmem_bank[8][32][156];

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
    if (currentMenuState == BANKS) {
      // Bank selection menu
      line1 = "Banks";
      if (BankIndex < NUM_BANKS) {
        line2 = String(BankIndex + 1) + ". " + String(BankNames[BankIndex]);
      } else {
        line2 = "< Back";
      }
    } else if (currentMenuState == PATCHES) {
      // Patch selection menu
      line1 = String(BankNames[currentBank]) + " Patches";
      if (PatchIndex == 32) {
        line2 = "< Back";
      } else {
        // Extract and display patch name being browsed
        char voice_name[11];
        memset(voice_name, 0, 11);
        memcpy(voice_name, &progmem_bank[currentBank][PatchIndex][144], 10);
        // Clean up non-printable characters
        for (int i = 0; i < 10; i++) {
          if (voice_name[i] < 32 || voice_name[i] > 126) voice_name[i] = ' ';
        }
        line2 = String(PatchIndex + 1) + ". " + String(voice_name).trim();
      }
    } else {
      switch(currentMenuState) {
        case PARENT_MENU:
          line1 = "Menu";
          line2 = parentMenuItems[menuIndex];
          break;
        case FM_MENU:
          line1 = "FM Parameters";
          line2 = fmMenuItems[menuIndex];
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
              float val = allParameterValues[paramIndex];
              
              // Special formatting for different parameter types
              if (paramIndex == 0) { // Algorithm (0-31)
                line2 = String((int)(val * 31));
              } else if (paramIndex == 1) { // Feedback (0-7)
                line2 = String((int)(val * 7));
              } else if (paramIndex == 3) { // Master Volume (0-200%)
                line2 = String((int)(val * 200)) + "%";
              } else { // Standard 0-99 range for LFO speed and OP levels
                line2 = String((int)(val * 99));
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
      line1 = "FM Synth";
      line2 = "Press for menu";
      displayText(line1, line2);
    }
  }
}

int getParameterIndex(MenuState state) {
  switch(state) {
    case FM_ALGORITHM: return 0;
    case FM_FEEDBACK: return 1;
    case FM_LFO_SPEED: return 2;
    case FM_MASTER_VOL: return 3;
    case FM_OP1_LEVEL: return 4;
    case FM_OP2_LEVEL: return 5;
    case FM_OP3_LEVEL: return 6;
    case FM_OP4_LEVEL: return 7;
    case FM_OP5_LEVEL: return 8;
    case FM_OP6_LEVEL: return 9;
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
            printCurrentPresetValues();
      updateDisplay();
    }
    
    if (inMenu) {
      if (currentMenuState == BANKS) {
        // Navigate banks (0 to NUM_BANKS-1, then Back)
        if (newMenuValue > oldMenuValue) {
          BankIndex++;
          if (BankIndex > NUM_BANKS) BankIndex = 0; // 0 to NUM_BANKS-1, then Back
        } else {
          BankIndex--;
          if (BankIndex < 0) BankIndex = NUM_BANKS;
        }
        updateDisplay();
      } else if (currentMenuState == PATCHES) {
        // Navigate patches (0-31, then Back)
        if (newMenuValue > oldMenuValue) {
          PatchIndex++;
          if (PatchIndex > 32) PatchIndex = 0; // 0-31=patches, 32=Back
        } else {
          PatchIndex--;
          if (PatchIndex < 0) PatchIndex = 32;
        }
        updateDisplay();
      } else if (getParameterIndex(currentMenuState) >= 0) {
        int paramIndex = getParameterIndex(currentMenuState);
        float increment = 1.0/128.0; // Standard increment
        
        // Special increments for specific parameters
        if (paramIndex == 0) { // Algorithm - larger steps for 32 values
          increment = 1.0/32.0;
        } else if (paramIndex == 1) { // Feedback - larger steps for 8 values
          increment = 1.0/8.0;
        } else if (paramIndex == 3) { // Master Volume - fine steps for smooth volume control
          increment = 1.0/200.0;
        }
        
        if (newMenuValue > oldMenuValue) {
          allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] + increment, 0.0, 1.0);
          updateParameterFromMenu(paramIndex, allParameterValues[paramIndex]);
        } else {
          allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] - increment, 0.0, 1.0);
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
            printCurrentPresetValues();
      updateDisplay();
    } else if (currentMenuState == BANKS) {
      if (BankIndex == NUM_BANKS) {
        // Back button pressed
        currentMenuState = PARENT_MENU;
        menuIndex = 0; // Return to Presets position
      } else {
        // Bank selected, go to patch selection
        currentBank = BankIndex;
        currentMenuState = PATCHES;
        PatchIndex = 0;
      }
      updateDisplay();
    } else if (currentMenuState == PATCHES) {
      if (PatchIndex == 32) {
        // Back button pressed, return to bank selection
        currentMenuState = BANKS;
      } else {
        // Patch selected, load it and stay in patch menu
        loadPreset(PatchIndex);
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
        currentMenuState = BANKS;
        BankIndex = 0;
        return; 
      }
      else if (menuIndex == 1) currentMenuState = FM_MENU;
      else if (menuIndex == 2) currentMenuState = SETTINGS;
      else if (menuIndex == 3) {
        inMenu = false;
        return;
      }
      menuIndex = 0;
      break;
    case FM_MENU:
      if (menuIndex == 0) currentMenuState = FM_ALGORITHM;
      else if (menuIndex == 1) currentMenuState = FM_FEEDBACK;
      else if (menuIndex == 2) currentMenuState = FM_LFO_SPEED;
      else if (menuIndex == 3) currentMenuState = FM_MASTER_VOL;
      else if (menuIndex == 4) currentMenuState = FM_OP1_LEVEL;
      else if (menuIndex == 5) currentMenuState = FM_OP2_LEVEL;
      else if (menuIndex == 6) currentMenuState = FM_OP3_LEVEL;
      else if (menuIndex == 7) currentMenuState = FM_OP4_LEVEL;
      else if (menuIndex == 8) currentMenuState = FM_OP5_LEVEL;
      else if (menuIndex == 9) currentMenuState = FM_OP6_LEVEL;
      else if (menuIndex == 10) {
        currentMenuState = PARENT_MENU;
        menuIndex = 1;
        return;
      }
      break;
    case SETTINGS:
      if (menuIndex == 0) currentMenuState = MIDI_CHANNEL;
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
      if (menuIndex > 3) menuIndex = 0; // FM parent menu: Presets, Parameters, Settings, Exit
      break;
    case BANKS:
    case PATCHES:
      // These are handled by encoder movement, not menuIndex
      break;
    case FM_MENU:
      menuIndex++;
      if (menuIndex > 10) menuIndex = 0; // FM has 11 items (0-10) including Back
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
      if (menuIndex < 0) menuIndex = 3; // FM parent menu: Presets, Parameters, Settings, Exit
      break;
    case BANKS:
    case PATCHES:
      // These are handled by encoder movement, not menuIndex
      break;
    case FM_MENU:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 10; // FM has 11 items (0-10) including Back
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
    case FM_MENU:
      currentMenuState = PARENT_MENU;
      menuIndex = 1; // FM_MENU position
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
    case BANKS:
      currentMenuState = PARENT_MENU;
      menuIndex = 0; // Return to Presets position
      break;
    case PATCHES:
      currentMenuState = BANKS; // Go back to bank selection
      break;
    case FM_ALGORITHM:
    case FM_FEEDBACK:
    case FM_LFO_SPEED:
    case FM_MASTER_VOL:
    case FM_OP1_LEVEL:
    case FM_OP2_LEVEL:
    case FM_OP3_LEVEL:
    case FM_OP4_LEVEL:
    case FM_OP5_LEVEL:
    case FM_OP6_LEVEL:
      {
        // Get current state before changing it
        MenuState previousState = currentMenuState;
        currentMenuState = FM_MENU;
        
        // Set menuIndex based on which parameter we came from
        if (previousState == FM_ALGORITHM) menuIndex = 0;
        else if (previousState == FM_FEEDBACK) menuIndex = 1;
        else if (previousState == FM_LFO_SPEED) menuIndex = 2;
        else if (previousState == FM_MASTER_VOL) menuIndex = 3;
        else if (previousState == FM_OP1_LEVEL) menuIndex = 4;
        else if (previousState == FM_OP2_LEVEL) menuIndex = 5;
        else if (previousState == FM_OP3_LEVEL) menuIndex = 6;
        else if (previousState == FM_OP4_LEVEL) menuIndex = 7;
        else if (previousState == FM_OP5_LEVEL) menuIndex = 8;
        else if (previousState == FM_OP6_LEVEL) menuIndex = 9;
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

void printCurrentPresetValues() {
  Serial.println("\nCurrent Parameter Values:");
  Serial.print("{");
  for (int i = 0; i < NUM_PARAMETERS; i++) {
    Serial.print(allParameterValues[i], 3); // 3 decimal places
    if (i < NUM_PARAMETERS - 1) Serial.print(", ");
  }
  Serial.println("}");
  Serial.println("Copy this line into your preset array!");
  
  Serial.println("\nKey Parameters:");
  Serial.print("Algorithm: "); Serial.print((int)(allParameterValues[0] * 31));
  Serial.print(" | Feedback: "); Serial.print((int)(allParameterValues[1] * 7));
  Serial.print(" | LFO Speed: "); Serial.println((int)(allParameterValues[2] * 99));
  Serial.print("OP1 Level: "); Serial.print((int)(allParameterValues[6] * 99));
  Serial.print(" | OP2 Level: "); Serial.print((int)(allParameterValues[7] * 99));
  Serial.print(" | Transpose: "); 
  int transpose = (int)((allParameterValues[5] - 0.5) * 48);
  Serial.println((transpose >= 0) ? "+" + String(transpose) : String(transpose));
  
  Serial.println("\nEncoder Raw Values:");
  for (int i = 0; i < 5; i++) {
    Serial.print("Enc");
    Serial.print(i+1);
    Serial.print(": ");
    Serial.print(encoderValues[i]);
    Serial.print(" -> ");
    int paramIndex = encoderMapping[i];
    if (paramIndex >= 0 && paramIndex < NUM_PARAMETERS) {
      Serial.println(allParameterValues[paramIndex], 3);
    } else {
      Serial.println("[DISABLED]");
    }
  }
  Serial.println("=============================\n");
}


void updateEncoderParameter(int paramIndex, int change) {
  if (paramIndex < 0 || paramIndex >= NUM_PARAMETERS) return;
  
  float increment = 1.0/128.0; // Standard increment
  
  // Special increments for specific parameters
  if (paramIndex == 0) { // Algorithm - 32 steps (0-31)
    increment = 1.0/31.0;  // 31 steps to cover 0.0 to 1.0
  } else if (paramIndex == 1) { // Feedback - 8 steps (0-7) 
    increment = 1.0/7.0;   // 7 steps to cover 0.0 to 1.0
  } else if (paramIndex == 3) { // Master Volume - fine steps for smooth volume control
    increment = 1.0/200.0;
  }
  
  allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] + (change * increment), 0.0, 1.0);
  float val = allParameterValues[paramIndex];
  
  updateSynthParameter(paramIndex, val);
  
  if (!inMenu) {
    String line1 = controlNames[paramIndex];
    String line2;
    
    // Special formatting for different parameter types
    if (paramIndex == 0) { // Algorithm (0-31)
      line2 = String((int)(val * 31));
    } else if (paramIndex == 1) { // Feedback (0-7)
      line2 = String((int)(val * 7));
    } else if (paramIndex == 3) { // Master Volume (0-200%)
      line2 = String((int)(val * 200)) + "%";
    } else { // Standard 0-99 range for LFO speed and OP levels
      line2 = String((int)(val * 99));
    }
    
    displayText(line1, line2);
  }
}


void resetEncoderBaselines() {
  for (int i = 0; i < 19; i++) {
    int paramIndex = encoderMapping[i]; // Use configurable mapping
    
    // Only reset encoders that are mapped to valid parameters
    if (paramIndex >= 0 && paramIndex < NUM_PARAMETERS) {
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

void updateParameterFromMenu(int paramIndex, float val) {
  updateSynthParameter(paramIndex, val);
  // Display update removed - now handled once per loop in main()
}