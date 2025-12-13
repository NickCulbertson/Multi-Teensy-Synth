#include "config.h"
#include "MenuNavigation.h"
#include "dx7_rom1a_unpacked.h"  // For NUM_DX7_BANKS definition
#include <Encoder.h>
#include <Audio.h>

#ifdef USE_OLED_DISPLAY
#include <U8g2lib.h>
#include <Wire.h>
#endif

// External display objects that need to be declared in main file
#ifdef USE_LCD_DISPLAY
extern LiquidCrystal_I2C lcd;
#endif

#ifdef USE_OLED_DISPLAY
extern U8G2_SH1106_128X64_NONAME_F_HW_I2C display;
#endif

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

void updateParameterFromMenu(int paramIndex, float val) {
  // Handle Braids parameters separately
  if (paramIndex >= 100 && paramIndex <= 116) {
    int braidsParamIndex = paramIndex - 100; // Convert to 0-16 range
    updateBraidsParameter(braidsParamIndex, val);
    return;
  }
  
  // Apply Juno-60 parameter mapping first if needed
  if (currentEngine == ENGINE_JUNO) {
    applyJunoParameterMapping();
  }
  
  // Update the synthesis parameter
  updateSynthParameter(paramIndex, val);
  
  // Update display
  if (!inMenu) {
    String line1 = controlNames[paramIndex];
    String line2;
    
    if (paramIndex >= 5 && paramIndex <= 7) { // Waveform controls
      int waveIndex = getWaveformIndex(val, (paramIndex == 5) ? 1 : ((paramIndex == 6) ? 2 : 3));
      line2 = waveformNames[waveIndex];
    }
    else if (paramIndex >= 0 && paramIndex <= 2) { // Range controls
      int rangeIndex = getRangeIndex(val);
      line2 = rangeNames[rangeIndex];
    }
    else if (paramIndex == 3 || paramIndex == 4) { // Extended fine tuning controls
      // Calculate display value based on range
      if (val <= 0.25) {
        // Semitone range (negative)
        float semiRange = val / 0.25;
        int semitones = (int)(-12 + (semiRange * 11)); // -12 to -1
        line2 = String(semitones) + "st";
      } else if (val >= 0.75) {
        // Semitone range (positive)
        float semiRange = (val - 0.75) / 0.25;
        int semitones = (int)(1 + (semiRange * 11)); // +1 to +12
        line2 = "+" + String(semitones) + "st";
      } else {
        // Cents range (±25 cents)
        int cents = (int)((val - 0.5) * 100); // -25 to +25 cents
        line2 = (cents >= 0 ? "+" : "") + String(cents) + "c";
      }
    }
    else if (paramIndex == 31) { // Effects Bypass
      line2 = (val > 0.5) ? "BYPASSED" : "ACTIVE";
    }
    else if (paramIndex == 32) { // Flanger Rate
      float flangerHz = val; // Direct 0-1 mapping 
      line2 = String(flangerHz, 3) + " Hz";
    }
    else if (paramIndex == 33) { // Flanger Depth
      int depthPercent = (int)(val * 100);
      line2 = String(depthPercent) + "%";
    }
    else if (paramIndex >= 34 && paramIndex <= 38) { // Reverb parameters
      int displayValue = (int)(val * 100); // 0-100 scale for reverb
      line2 = String(displayValue) + "%";
    }
    else {
      // Show raw parameter value for other controls
      int displayValue = (int)(val * 127); // 0-127 MIDI scale
      line2 = String(displayValue);
    }
    
    displayText(line1, line2);
  }
}

// External variable access for synthesis parameters
extern float allParameterValues[41];
extern const char* controlNames[41];
extern const char* waveformNames[6];
extern const char* rangeNames[6];

// External menu state variables
extern bool inMenu;
extern bool inParentMenu;
extern ParentMenuState currentParentMenu;
extern MenuState currentMenuState;
extern int menuIndex;
extern EngineType currentEngine;
extern bool inPresetBrowse;
extern int presetBrowseIndex;
extern int junoPresetBrowseIndex;
extern int dx7BrowseIndex;
extern int engineBrowseIndex;

// External synthesis variables
extern bool macroMode;
extern int midiChannel;
extern bool lfoEnabled;
extern int lfoTarget;
extern int playMode;
extern float glideTime;
extern int noiseType;
extern bool chorusBypass;
extern bool reverbBypass;
extern float chorusRate;
extern float chorusDepth;
extern float reverbSize;
extern float reverbHidamp;

// VOICES now defined in config.h

// External encoder arrays and variables
extern long encoderValues[20];
extern long lastEncoderValues[20];

// External encoder objects
extern Encoder enc1, enc2, enc3, enc4, enc5;
extern Encoder enc6, enc7, enc8, enc9, enc10;
extern Encoder enc11, enc13, enc14, enc15, enc16;
extern Encoder enc17, enc18, enc19, enc20;

// External voice structure
struct Voice {
  bool active;
  int note;
  // Add other voice fields as needed
};
extern Voice voices[6];  // Matches VOICES = 6 from main file

// Removed incorrect waveform constants - now using correct ones from synth_waveform.h


// External DX7 bank data
extern unsigned char progmem_bank[][32][156];   // Size determined by sysex2c.py

int getParameterIndex(MenuState state) {
  switch(state) {
    case OSC1_RANGE: return 0;
    case OSC2_RANGE: return 1;
    case OSC3_RANGE: return 2;
    case OSC2_FINE: return 3;
    case OSC3_FINE: return 4;
    case OSC1_WAVE: return 5;
    case OSC2_WAVE: return 6;
    case OSC3_WAVE: return 7;
    case OSC1_VOLUME: return 8;
    case OSC2_VOLUME: return 9;
    case OSC3_VOLUME: return 10;
    case CUTOFF: return 11;
    case RESONANCE: return 12;
    case FILTER_ATTACK: return 13;
    case FILTER_DECAY: return 14;
    case FILTER_SUSTAIN: return 15;
    case NOISE_VOLUME: return 16;
    case AMP_ATTACK: return 17;
    case AMP_SUSTAIN: return 18;
    case AMP_DECAY: return 19;
    case OSC1_FINE: return 20;  // New menu-only parameter
    case FILTER_STRENGTH: return 21;  // New menu-only parameter
    case LFO_RATE: return 22;
    case LFO_DEPTH: return 23;
    case LFO_TOGGLE: return 24;
    case LFO_TARGET: return 25;
    case PLAY_MODE: return 26;
    case GLIDE_TIME: return 27;
    case NOISE_TYPE: return 28;
    case MACRO_KNOBS: return 29;
    case MIDI_CHANNEL: return 30;
    case CHORUS_BYPASS: return 31;
    case CHORUS_RATE: return 32;
    case CHORUS_DEPTH: return 33;
    case REVERB_BYPASS: return 34;
    case REVERB_SIZE: return 35;
    case REVERB_HIDAMP: return 36;
    case REVERB_LODAMP: return 37;
    case REVERB_LOWPASS: return 38;
    case REVERB_DIFFUSION: return 39;
    // Braids parameters - use custom parameter system
    case BRAIDS_SHAPE: return 100;  // Custom indices for Braids
    case BRAIDS_TIMBRE: return 101;
    case BRAIDS_COLOR: return 102;
    case BRAIDS_COARSE: return 103;
    case BRAIDS_AMP_ATTACK: return 104;
    case BRAIDS_AMP_DECAY: return 105;
    case BRAIDS_AMP_SUSTAIN: return 106;
    case BRAIDS_AMP_RELEASE: return 107;
    case BRAIDS_FILTER_MODE: return 108;
    case BRAIDS_FILTER_CUTOFF: return 109;
    case BRAIDS_FILTER_RESONANCE: return 110;
    case BRAIDS_FILTER_STRENGTH: return 111;
    case BRAIDS_FILTER_ATTACK: return 112;
    case BRAIDS_FILTER_DECAY: return 113;
    case BRAIDS_FILTER_SUSTAIN: return 114;
    case BRAIDS_FILTER_RELEASE: return 115;
    case BRAIDS_VOLUME: return 116;
    default: return -1;
  }
}

void updateEncoderParameter(int paramIndex, int change) {
  // Different increments for different parameter types
  float increment = 0.01; // Base increment
  
  switch (paramIndex) {
    case 0: case 1: case 2: // Range controls - discrete steps
      increment = 0.16; // 6 ranges
      break;
    case 3: case 4: // Fine tuning controls - 128 steps (enc4, enc5 /4)
      increment = 1.0/128.0; // = 0.0078125 - exact 128-step resolution
      break;
    case 5: case 6: case 7: // Waveform controls - discrete steps
      increment = 0.16; // 6 waveforms
      break;
    case 8: case 9: case 10: // Volume controls - 128 steps (enc9, enc10, enc11 /4)
      increment = 1.0/128.0; // = 0.0078125 - exact 128-step resolution
      break;
    case 11: // Filter cutoff - consistent 128-step feel (menuEncoder /2)
      increment = 1.0/128.0; // = 0.0078125 - consistent with all other controls
      break;
    case 12: // Filter resonance - optimized for 128 steps (enc13 /4) 
      increment = 1.0/128.0; // = 0.0078125 - exact 128-step resolution
      break;
    case 13: case 14: case 15: case 17: case 18: case 19: // Envelope controls - 128 steps (enc14-enc20 /4)
      increment = 1.0/128.0; // = 0.0078125 - exact 128-step resolution
      break;
    case 24: case 25: case 26: case 28: case 29: case 30: case 31: case 34: // Toggle/discrete controls (LFO Toggle, LFO Target, Play Mode, Noise Type, Macro Mode, MIDI Channel, Chorus Bypass, Reverb Bypass)
      increment = 0.5; // Large steps for immediate toggle response
      break;
    default:
      increment = 0.01; // Standard increment
      break;
  }
  
  // Update the global parameter array
  allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] + (change * increment), 0.0, 1.0);
  float val = allParameterValues[paramIndex];
  
  // Snap discrete controls to exact threshold values
  if (paramIndex >= 0 && paramIndex <= 2) { // Range controls
    if (val < 0.167) val = 0.083;        // 32' center
    else if (val < 0.333) val = 0.25;    // 16' center  
    else if (val < 0.5) val = 0.417;     // 8' center
    else if (val < 0.667) val = 0.583;   // 4' center
    else if (val < 0.833) val = 0.75;    // 2' center
    else val = 0.917;                    // LO center
    allParameterValues[paramIndex] = val; // Store snapped value
  } 
  else if (paramIndex >= 5 && paramIndex <= 7) { // Waveform controls
    if (val < 0.167) val = 0.083;        // Triangle center
    else if (val < 0.333) val = 0.25;    // Reverse Saw center
    else if (val < 0.5) val = 0.417;     // Sawtooth center  
    else if (val < 0.667) val = 0.583;   // Square center
    else if (val < 0.833) val = 0.75;    // Pulse center
    else val = 0.917;                    // Pulse center
    allParameterValues[paramIndex] = val; // Store snapped value
  }
  
  
  // Update the synthesis parameter
  updateSynthParameter(paramIndex, val);
  
  // Update display
  if (!inMenu) {
    String line1 = controlNames[paramIndex];
    String line2;
    
    if (paramIndex >= 5 && paramIndex <= 7) { // Waveform controls
      int waveIndex = getWaveformIndex(val, (paramIndex == 5) ? 1 : ((paramIndex == 6) ? 2 : 3));
      line2 = waveformNames[waveIndex];
    }
    else if (paramIndex >= 0 && paramIndex <= 2) { // Range controls
      int rangeIndex = getRangeIndex(val);
      line2 = rangeNames[rangeIndex];
    }
    else if (paramIndex == 3 || paramIndex == 4) { // Extended fine tuning controls
      // Calculate display value based on range
      if (val <= 0.25) {
        // Semitone range (negative)
        float semiRange = val / 0.25;
        int semitones = (int)(-12 + (semiRange * 11)); // -12 to -1
        line2 = String(semitones) + "st";
      } else if (val >= 0.75) {
        // Semitone range (positive)
        float semiRange = (val - 0.75) / 0.25;
        int semitones = (int)(1 + (semiRange * 11)); // +1 to +12
        line2 = "+" + String(semitones) + "st";
      } else {
        // Cents range (±25 cents)
        int cents = (int)((val - 0.5) * 100); // -25 to +25 cents
        line2 = (cents >= 0 ? "+" : "") + String(cents) + "c";
      }
    }
    else if (paramIndex == 31) { // Effects Bypass
      line2 = (val > 0.5) ? "BYPASSED" : "ACTIVE";
    }
    else if (paramIndex == 32) { // Flanger Rate
      float flangerHz = val; // Direct 0-1 mapping 
      line2 = String(flangerHz, 3) + " Hz";
    }
    else if (paramIndex == 33) { // Flanger Depth
      int depthPercent = (int)(val * 100);
      line2 = String(depthPercent) + "%";
    }
    else if (paramIndex >= 34 && paramIndex <= 38) { // Reverb parameters
      int displayValue = (int)(val * 100); // 0-100 scale for reverb
      line2 = String(displayValue) + "%";
    }
    else {
      // Show raw parameter value for other controls
      int displayValue = (int)(val * 127); // 0-127 MIDI scale
      line2 = String(displayValue);
    }
    
    displayText(line1, line2);
  }
}

float getOscillatorRange(float val) {
  if (val < 0.167) return 0.25;        // 32'
  else if (val < 0.333) return 0.5;    // 16'
  else if (val < 0.5) return 1.0;      // 8'
  else if (val < 0.667) return 2.0;    // 4'
  else if (val < 0.833) return 4.0;    // 2'
  else return 0.0625;                  // LO
}

int getRangeIndex(float val) {
  if (val < 0.167) return 0;
  else if (val < 0.333) return 1;
  else if (val < 0.5) return 2;
  else if (val < 0.667) return 3;
  else if (val < 0.833) return 4;
  else return 5;
}

int getWaveformIndex(float val, int osc) {
  if (osc == 1 || osc == 2) {
    if (val < 0.167) return 0;
    else if (val < 0.333) return 1;
    else if (val < 0.5) return 2;
    else if (val < 0.667) return 3;
    else if (val < 0.833) return 4;
    else return 5;
  } else {
    if (val < 0.167) return 0;
    else if (val < 0.333) return 2;
    else if (val < 0.5) return 2;
    else if (val < 0.667) return 3;
    else if (val < 0.833) return 4;
    else return 5;
  }
}

int getMiniTeensyWaveform(float val, int osc) {
  // Fixed waveform mapping - was returning wrong constants
  // Now properly returns bandlimited waveforms
  
  if (osc == 1 || osc == 2) {
    if (val < 0.167) return WAVEFORM_TRIANGLE;                    // 3
    else if (val < 0.333) return WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE; // 10  
    else if (val < 0.5) return WAVEFORM_BANDLIMIT_SAWTOOTH;       // 9
    else if (val < 0.667) return WAVEFORM_BANDLIMIT_SQUARE;       // 11
    else if (val < 0.833) return WAVEFORM_BANDLIMIT_PULSE;        // 12
    else return WAVEFORM_BANDLIMIT_PULSE;                         // 12
  } else {
    if (val < 0.167) return WAVEFORM_TRIANGLE;                    // 3
    else if (val < 0.333) return WAVEFORM_BANDLIMIT_SAWTOOTH;     // 9
    else if (val < 0.5) return WAVEFORM_BANDLIMIT_SAWTOOTH;       // 9
    else if (val < 0.667) return WAVEFORM_BANDLIMIT_SQUARE;       // 11
    else if (val < 0.833) return WAVEFORM_BANDLIMIT_PULSE;        // 12
    else return WAVEFORM_BANDLIMIT_PULSE;                         // 12
  }
}

void navigateMenuForward() {
  switch(currentMenuState) {
    case PARAMETERS:
      if (currentEngine == ENGINE_VA) {
        // VA parameter navigation
        if (menuIndex == 0) currentMenuState = OSC_1;
        else if (menuIndex == 1) currentMenuState = OSC_2;
        else if (menuIndex == 2) currentMenuState = OSC_3;
        else if (menuIndex == 3) currentMenuState = NOISE;
        else if (menuIndex == 4) currentMenuState = ENVELOPES;
        else if (menuIndex == 5) currentMenuState = FILTER;
        else if (menuIndex == 6) currentMenuState = LFO;
        else if (menuIndex == 7) currentMenuState = VOICE_MODE;
        else if (menuIndex == 8) {
          // Back to parent menu
          enterParentMenuLevel();
          currentParentMenu = PARENT_PARAMETERS; // Ensure we're positioned on Parameters
          return;
        }
      } else if (currentEngine == ENGINE_BRAIDS) {
        // Braids parameter navigation
        if (menuIndex == 0) currentMenuState = BRAIDS_SHAPE;
        else if (menuIndex == 1) currentMenuState = BRAIDS_TIMBRE;
        else if (menuIndex == 2) currentMenuState = BRAIDS_COLOR;
        else if (menuIndex == 3) currentMenuState = BRAIDS_COARSE;
        else if (menuIndex == 4) currentMenuState = BRAIDS_AMP_ATTACK;
        else if (menuIndex == 5) currentMenuState = BRAIDS_AMP_DECAY;
        else if (menuIndex == 6) currentMenuState = BRAIDS_AMP_SUSTAIN;
        else if (menuIndex == 7) currentMenuState = BRAIDS_AMP_RELEASE;
        else if (menuIndex == 8) currentMenuState = BRAIDS_FILTER_MODE;
        else if (menuIndex == 9) currentMenuState = BRAIDS_FILTER_CUTOFF;
        else if (menuIndex == 10) currentMenuState = BRAIDS_FILTER_RESONANCE;
        else if (menuIndex == 11) currentMenuState = BRAIDS_FILTER_STRENGTH;
        else if (menuIndex == 12) currentMenuState = BRAIDS_FILTER_ATTACK;
        else if (menuIndex == 13) currentMenuState = BRAIDS_FILTER_DECAY;
        else if (menuIndex == 14) currentMenuState = BRAIDS_FILTER_SUSTAIN;
        else if (menuIndex == 15) currentMenuState = BRAIDS_FILTER_RELEASE;
        else if (menuIndex == 16) currentMenuState = BRAIDS_VOLUME;
        else if (menuIndex == 17) {
          // Back to parent menu
          enterParentMenuLevel();
          currentParentMenu = PARENT_PARAMETERS; // Ensure we're positioned on Parameters
          return;
        }
      } else {
        // Default/other engines - just go back
        enterParentMenuLevel();
        currentParentMenu = PARENT_PARAMETERS;
        return;
      }
      menuIndex = 0; // Reset index for sub-menu
      break;
    case OSC_1:
      if (menuIndex == 0) currentMenuState = OSC1_RANGE;
      else if (menuIndex == 1) currentMenuState = OSC1_WAVE;
      else if (menuIndex == 2) currentMenuState = OSC1_VOLUME;
      else if (menuIndex == 3) currentMenuState = OSC1_FINE;
      else if (menuIndex == 4) {
        // Back to main menu
        currentMenuState = PARENT_MENU;
        menuIndex = 1; // Set to OSC_1 position in main menu
        return;
      }
      break;
    case OSC_2:
      if (menuIndex == 0) currentMenuState = OSC2_RANGE;
      else if (menuIndex == 1) currentMenuState = OSC2_WAVE;
      else if (menuIndex == 2) currentMenuState = OSC2_VOLUME;
      else if (menuIndex == 3) currentMenuState = OSC2_FINE;
      else if (menuIndex == 4) {
        // Back to main menu
        currentMenuState = PARENT_MENU;
        menuIndex = 2; // Set to OSC_2 position in main menu
        return;
      }
      break;
    case OSC_3:
      if (menuIndex == 0) currentMenuState = OSC3_RANGE;
      else if (menuIndex == 1) currentMenuState = OSC3_WAVE;
      else if (menuIndex == 2) currentMenuState = OSC3_VOLUME;
      else if (menuIndex == 3) currentMenuState = OSC3_FINE;
      else if (menuIndex == 4) {
        // Back to main menu
        currentMenuState = PARENT_MENU;
        menuIndex = 3; // Set to OSC_3 position in main menu
        return;
      }
      break;
    case NOISE:
      if (menuIndex == 0) currentMenuState = NOISE_VOLUME;
      else if (menuIndex == 1) currentMenuState = NOISE_TYPE;
      else if (menuIndex == 2) {
        // Back to main menu
        currentMenuState = PARENT_MENU;
        menuIndex = 4; // Set to NOISE position in main menu
        return;
      }
      break;
    case ENVELOPES:
      if (menuIndex == 0) currentMenuState = AMP_ATTACK;
      else if (menuIndex == 1) currentMenuState = AMP_SUSTAIN;
      else if (menuIndex == 2) currentMenuState = AMP_DECAY;
      else if (menuIndex == 3) currentMenuState = FILTER_ATTACK;
      else if (menuIndex == 4) currentMenuState = FILTER_DECAY;
      else if (menuIndex == 5) currentMenuState = FILTER_SUSTAIN;
      else if (menuIndex == 6) {
        // Back to main menu
        currentMenuState = PARENT_MENU;
        menuIndex = 5; // Set to ENVELOPES position in main menu
        return;
      }
      break;
    case FILTER:
      if (menuIndex == 0) currentMenuState = CUTOFF;
      else if (menuIndex == 1) currentMenuState = RESONANCE;
      else if (menuIndex == 2) currentMenuState = FILTER_STRENGTH;
      else if (menuIndex == 3) {
        // Back to main menu
        currentMenuState = PARENT_MENU;
        menuIndex = 6; // Set to FILTER position in main menu
        return;
      }
      break;
    case LFO:
      if (menuIndex == 0) currentMenuState = LFO_RATE;
      else if (menuIndex == 1) currentMenuState = LFO_DEPTH;
      else if (menuIndex == 2) currentMenuState = LFO_TOGGLE;
      else if (menuIndex == 3) currentMenuState = LFO_TARGET;
      else if (menuIndex == 4) {
        // Back to main menu
        currentMenuState = PARENT_MENU;
        menuIndex = 7; // Set to LFO position in main menu
        return;
      }
      break;
    case VOICE_MODE:
      if (menuIndex == 0) currentMenuState = PLAY_MODE;
      else if (menuIndex == 1) currentMenuState = GLIDE_TIME;
      else if (menuIndex == 2) {
        // Back to Parameters menu
        currentMenuState = PARAMETERS;
        menuIndex = 7; // Voice Mode is at index 7 in Parameters
        return;
      }
      break;
    case EFFECTS:
      if (menuIndex == 0) currentMenuState = CHORUS_BYPASS;
      else if (menuIndex == 1) currentMenuState = CHORUS_RATE;
      else if (menuIndex == 2) currentMenuState = CHORUS_DEPTH;
      else if (menuIndex == 3) currentMenuState = REVERB_BYPASS;
      else if (menuIndex == 4) currentMenuState = REVERB_SIZE;
      else if (menuIndex == 5) currentMenuState = REVERB_HIDAMP;
      else if (menuIndex == 6) currentMenuState = REVERB_LODAMP;
      else if (menuIndex == 7) currentMenuState = REVERB_LOWPASS;
      else if (menuIndex == 8) currentMenuState = REVERB_DIFFUSION;
      else if (menuIndex == 9) {
        // Back to parent menu
        enterParentMenuLevel();
        currentParentMenu = PARENT_EFFECTS; // Ensure we're positioned on Effects
        return;
      }
      break;
    case SETTINGS:
      if (menuIndex == 0) currentMenuState = MACRO_KNOBS;
      else if (menuIndex == 1) currentMenuState = MIDI_CHANNEL;
      else if (menuIndex == 2) {
        currentMenuState = PARENT_MENU;
        menuIndex = 9;
        return;
      }
      break;
    default:
      // Already in a parameter, can't go deeper
      break;
  }
}

void incrementMenuIndex() {
  switch(currentMenuState) {
    case PARENT_MENU:
      menuIndex++;
      if (menuIndex > 18) menuIndex = 0; // Wrap to first item (now 19 items: 0-18)
      break;
    case OSC_1:
      menuIndex++;
      if (menuIndex > 4) menuIndex = 0; // OSC1 has 5 items (0-4) including Back
      break;
    case OSC_2:
      menuIndex++;
      if (menuIndex > 4) menuIndex = 0; // OSC2 has 5 items (0-4) including Back
      break;
    case OSC_3:
      menuIndex++;
      if (menuIndex > 4) menuIndex = 0; // OSC3 has 5 items (0-4) including Back
      break;
    case NOISE:
      menuIndex++;
      if (menuIndex > 2) menuIndex = 0; // Noise has 3 items (0-2) including Back
      break;
    case ENVELOPES:
      menuIndex++;
      if (menuIndex > 6) menuIndex = 0; // Envelopes has 7 items (0-6) including Back
      break;
    case FILTER:
      menuIndex++;
      if (menuIndex > 3) menuIndex = 0; // Filter has 4 items (0-3) including Back
      break;
    case LFO:
      menuIndex++;
      if (menuIndex > 4) menuIndex = 0; // LFO has 5 items (0-4) including Back
      break;
    case VOICE_MODE:
      menuIndex++;
      if (menuIndex > 2) menuIndex = 0; // Voice Mode has 3 items (0-2) including Back
      break;
    case PARAMETERS:
      menuIndex++;
      if (currentEngine == ENGINE_VA) {
        if (menuIndex > 8) menuIndex = 0; // VA has 9 items (0-8) including Back
      } else if (currentEngine == ENGINE_BRAIDS) {
        if (menuIndex > 17) menuIndex = 0; // Braids has 18 items (0-17) including Back
      } else {
        if (menuIndex > 8) menuIndex = 0; // Default for other engines
      }
      break;
    case EFFECTS:
      menuIndex++;
      if (menuIndex > 9) menuIndex = 0; // Effects has 10 items (0-9) including Back
      break;
    case SETTINGS:
      menuIndex++;
      if (menuIndex > 2) menuIndex = 0; // Settings has 3 items (0-2) including Back
      break;
    default:
      // In a parameter menu, no navigation
      break;
  }
}

void decrementMenuIndex() {
  switch(currentMenuState) {
    case PARENT_MENU:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 18; // Wrap to last item (Exit)
      break;
    case OSC_1:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 4; // Wrap to Back button (0-4)
      break;
    case OSC_2:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 4; // Wrap to Back button (0-4)
      break;
    case OSC_3:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 4; // Wrap to Back button (0-4)
      break;
    case NOISE:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 2; // Wrap to Back button (0-2)
      break;
    case ENVELOPES:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 6; // Wrap to Back button (0-6)
      break;
    case FILTER:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 3; // Wrap to Back button (0-3)
      break;
    case LFO:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 4; // Wrap to Back button (0-4)
      break;
    case VOICE_MODE:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 2; // Wrap to Back button (0-2)
      break;
    case PARAMETERS:
      menuIndex--;
      if (currentEngine == ENGINE_VA) {
        if (menuIndex < 0) menuIndex = 8; // Wrap to Back button (0-8)
      } else if (currentEngine == ENGINE_BRAIDS) {
        if (menuIndex < 0) menuIndex = 17; // Wrap to Back button (0-17)
      } else {
        if (menuIndex < 0) menuIndex = 8; // Default for other engines
      }
      break;
    case EFFECTS:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 9; // Wrap to Back button (0-9)
      break;
    case SETTINGS:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 2; // Wrap to Back button (0-2)
      break;
    default:
      // In a parameter menu, no navigation
      break;
  }
}

void navigateMenuBackward() {
  switch(currentMenuState) {
    case PARENT_MENU:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 7; // Wrap to last item (LFO)
      break;
    case OSC_1:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 2; // Wrap to last OSC1 item
      break;
    case OSC_2:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 3; // Wrap to last OSC2 item
      break;
    case OSC_3:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 3; // Wrap to last OSC3 item
      break;
    case NOISE:
      menuIndex = 0; // Only one item, stay at 0
      break;
    case ENVELOPES:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 5; // Wrap to last envelope item
      break;
    case FILTER:
      menuIndex--;
      if (menuIndex < 0) menuIndex = 1; // Wrap to last filter item
      break;
    default:
      // In a parameter menu, can't navigate backward with encoder
      break;
  }
}

void backMenuAction() {
  switch(currentMenuState) {
    case PARENT_MENU:
      // Already at top level
      break;
    case PARAMETERS:
      exitParentMenuLevel(); // Go back to parent level (engines, presets, etc.)
      break;
    case OSC_1:
      currentMenuState = PARAMETERS; // Go back to Parameters menu
      menuIndex = 0; // Oscillator 1 is at index 0
      break;
    case OSC_2:
      currentMenuState = PARAMETERS; // Go back to Parameters menu
      menuIndex = 1; // Oscillator 2 is at index 1
      break;
    case OSC_3:
      currentMenuState = PARAMETERS; // Go back to Parameters menu
      menuIndex = 2; // Oscillator 3 is at index 2
      break;
    case NOISE:
      currentMenuState = PARAMETERS; // Go back to Parameters menu
      menuIndex = 3; // Noise is at index 3
      break;
    case ENVELOPES:
      currentMenuState = PARAMETERS; // Go back to Parameters menu
      menuIndex = 4; // Envelopes is at index 4
      break;
    case FILTER:
      currentMenuState = PARAMETERS; // Go back to Parameters menu
      menuIndex = 5; // Filter is at index 5
      break;
    case LFO:
      currentMenuState = PARAMETERS; // Go back to Parameters menu
      menuIndex = 6; // LFO is at index 6
      break;
    case VOICE_MODE:
      currentMenuState = PARAMETERS; // Go back to Parameters menu
      menuIndex = 7; // Voice Mode is at index 7
      break;
    case EFFECTS:
    case SETTINGS:
      currentMenuState = PARENT_MENU;
      break;
    case OSC1_RANGE:
    case OSC1_WAVE:
    case OSC1_VOLUME:
    case OSC1_FINE:
      currentMenuState = OSC_1;
      break;
    case OSC2_RANGE:
    case OSC2_WAVE:
    case OSC2_VOLUME:
    case OSC2_FINE:
      currentMenuState = OSC_2;
      break;
    case OSC3_RANGE:
    case OSC3_WAVE:
    case OSC3_VOLUME:
    case OSC3_FINE:
      currentMenuState = OSC_3;
      break;
    case NOISE_VOLUME:
      currentMenuState = NOISE;
      break;
    case AMP_ATTACK:
      currentMenuState = ENVELOPES;
      menuIndex = 0; // Amp Attack is at index 0
      break;
    case AMP_SUSTAIN:
      currentMenuState = ENVELOPES;
      menuIndex = 1; // Amp Sustain is at index 1
      break;
    case AMP_DECAY:
      currentMenuState = ENVELOPES;
      menuIndex = 2; // Amp Decay is at index 2
      break;
    case FILTER_ATTACK:
      currentMenuState = ENVELOPES;
      menuIndex = 3; // Filter Attack is at index 3
      break;
    case FILTER_DECAY:
      currentMenuState = ENVELOPES;
      menuIndex = 4; // Filter Decay is at index 4
      break;
    case FILTER_SUSTAIN:
      currentMenuState = ENVELOPES;
      menuIndex = 5; // Filter Sustain is at index 5
      break;
    case CUTOFF:
      currentMenuState = FILTER;
      menuIndex = 0; // Cutoff is at index 0
      break;
    case RESONANCE:
      currentMenuState = FILTER;
      menuIndex = 1; // Resonance is at index 1
      break;
    case FILTER_STRENGTH:
      currentMenuState = FILTER;
      menuIndex = 2; // Filter Strength is at index 2
      break;
    case LFO_RATE:
      currentMenuState = LFO;
      menuIndex = 0; // LFO Rate is at index 0
      break;
    case LFO_DEPTH:
      currentMenuState = LFO;
      menuIndex = 1; // LFO Depth is at index 1
      break;
    case LFO_TOGGLE:
      currentMenuState = LFO;
      menuIndex = 2; // LFO Toggle is at index 2
      break;
    case LFO_TARGET:
      currentMenuState = LFO;
      menuIndex = 3; // LFO Target is at index 3
      break;
    case PLAY_MODE:
      currentMenuState = VOICE_MODE;
      menuIndex = 0; // Play Mode is at index 0
      break;
    case GLIDE_TIME:
      currentMenuState = VOICE_MODE;
      menuIndex = 1; // Glide Time is at index 1
      break;
    case NOISE_TYPE:
      currentMenuState = NOISE;
      menuIndex = 1; // Noise Type is at index 1 in NOISE menu
      break;
    case MACRO_KNOBS:
    case MIDI_CHANNEL:
      currentMenuState = SETTINGS;
      break;
    case CHORUS_BYPASS:
    case CHORUS_RATE:
    case CHORUS_DEPTH:
    case REVERB_BYPASS:
    case REVERB_SIZE:
    case REVERB_HIDAMP:
    case REVERB_LODAMP:
    case REVERB_LOWPASS:
    case REVERB_DIFFUSION:
      currentMenuState = EFFECTS;
      break;
    case DX7_BANKS:
      exitParentMenuLevel(); // Go back to parent level (presets)
      break;
    case DX7_PATCHES:
      currentMenuState = DX7_BANKS; // Go back to bank selection
      dx7PatchIndex = 0; // Reset patch index
      break;
    // Braids parameter cases
    case BRAIDS_SHAPE:
      currentMenuState = PARAMETERS;
      menuIndex = 0; // Shape is at index 0
      break;
    case BRAIDS_TIMBRE:
      currentMenuState = PARAMETERS;
      menuIndex = 1; // Timbre is at index 1
      break;
    case BRAIDS_COLOR:
      currentMenuState = PARAMETERS;
      menuIndex = 2; // Color is at index 2
      break;
    case BRAIDS_COARSE:
      currentMenuState = PARAMETERS;
      menuIndex = 3; // Coarse is at index 3
      break;
    case BRAIDS_AMP_ATTACK:
      currentMenuState = PARAMETERS;
      menuIndex = 4; // Amp Attack is at index 4
      break;
    case BRAIDS_AMP_DECAY:
      currentMenuState = PARAMETERS;
      menuIndex = 5; // Amp Decay is at index 5
      break;
    case BRAIDS_AMP_SUSTAIN:
      currentMenuState = PARAMETERS;
      menuIndex = 6; // Amp Sustain is at index 6
      break;
    case BRAIDS_AMP_RELEASE:
      currentMenuState = PARAMETERS;
      menuIndex = 7; // Release is at index 7
      break;
    case BRAIDS_FILTER_MODE:
      currentMenuState = PARAMETERS;
      menuIndex = 8; // Filter Mode is at index 8
      break;
    case BRAIDS_FILTER_CUTOFF:
      currentMenuState = PARAMETERS;
      menuIndex = 9; // Filter Cutoff is at index 9
      break;
    case BRAIDS_FILTER_RESONANCE:
      currentMenuState = PARAMETERS;
      menuIndex = 10; // Filter Resonance is at index 10
      break;
    case BRAIDS_FILTER_STRENGTH:
      currentMenuState = PARAMETERS;
      menuIndex = 11; // Filter Strength is at index 11
      break;
    case BRAIDS_FILTER_ATTACK:
      currentMenuState = PARAMETERS;
      menuIndex = 12; // Filter Attack is at index 12
      break;
    case BRAIDS_FILTER_DECAY:
      currentMenuState = PARAMETERS;
      menuIndex = 13; // Filter Decay is at index 13
      break;
    case BRAIDS_FILTER_SUSTAIN:
      currentMenuState = PARAMETERS;
      menuIndex = 14; // Filter Sustain is at index 14
      break;
    case BRAIDS_FILTER_RELEASE:
      currentMenuState = PARAMETERS;
      menuIndex = 15; // Filter Release is at index 15
      break;
    case BRAIDS_VOLUME:
      currentMenuState = PARAMETERS;
      menuIndex = 16; // Volume is at index 16
      break;
  }
}

// ===== PARENT MENU FUNCTIONS =====

void handleParentMenu(int direction) {
  if (direction > 0) {
    currentParentMenu = (ParentMenuState)((currentParentMenu + 1) % 5);
  } else {
    currentParentMenu = (ParentMenuState)((currentParentMenu + 4) % 5); // +4 instead of -1 to handle wrap
  }
  updateDisplay();
}

void handleEngineMenu(int direction) {
  // Only browse, don't auto-switch
  if (direction > 0) {
    engineBrowseIndex = (engineBrowseIndex + 1) % 5; // 4 engines + 1 "Back" option
  } else {
    engineBrowseIndex = (engineBrowseIndex + 4) % 5; // Handle wrap
  }
  updateDisplay();
}

// DX7 preset menu removed - now using bank/patch system

void enterParentMenuLevel() {
  inMenu = true;
  inParentMenu = true;
  updateDisplay();
}

void exitParentMenuLevel() {
  inParentMenu = false;
}

void handleEncoder() {
#ifdef USE_OLED_DISPLAY
  long newMenuValue = menuEncoder.read() / 4; // Less sensitive for OLED encoder
#else
  long newMenuValue = menuEncoder.read() / 2; // Standard sensitivity for separate encoder
#endif 
  static long oldMenuValue = 0;
  
  
  if (newMenuValue != oldMenuValue) {
    int direction = (newMenuValue > oldMenuValue) ? 1 : -1;
    
    if (inMenu) {
      if (inParentMenu) {
        // In parent menu - navigate between Engines/Parameters/Presets/Settings
        handleParentMenu(direction);
      } else if (currentParentMenu == PARENT_ENGINES) {
        // In engine selection
        handleEngineMenu(direction);
      } else if (currentParentMenu == PARENT_PRESETS) {
        if (currentEngine == ENGINE_VA) {
          // VA preset browsing (existing system)
          if (inPresetBrowse) {
            if (direction > 0) {
              presetBrowseIndex++;
              if (presetBrowseIndex > NUM_PRESETS) presetBrowseIndex = 0;
            } else {
              presetBrowseIndex--;
              if (presetBrowseIndex < 0) presetBrowseIndex = NUM_PRESETS;
            }
            updateDisplay();
          }
        } else if (currentEngine == ENGINE_JUNO) {
          // Juno preset browsing
          if (inPresetBrowse) {
            if (direction > 0) {
              junoPresetBrowseIndex++;
              if (junoPresetBrowseIndex > NUM_JUNO_PRESETS) junoPresetBrowseIndex = 0;
            } else {
              junoPresetBrowseIndex--;
              if (junoPresetBrowseIndex < 0) junoPresetBrowseIndex = NUM_JUNO_PRESETS;
            }
            updateDisplay();
          }
        } else if (currentEngine == ENGINE_BRAIDS) {
          // Braids preset browsing
          if (inPresetBrowse) {
            if (direction > 0) {
              braidsPresetBrowseIndex++;
              if (braidsPresetBrowseIndex > NUM_BRAIDS_PRESETS) braidsPresetBrowseIndex = 0;
            } else {
              braidsPresetBrowseIndex--;
              if (braidsPresetBrowseIndex < 0) braidsPresetBrowseIndex = NUM_BRAIDS_PRESETS;
            }
            updateDisplay();
          }
        } else if (currentEngine == ENGINE_DX7) {
          // DX7 bank/patch navigation
          if (currentMenuState == DX7_BANKS) {
            // Navigate banks (0 to NUM_DX7_BANKS-1, Back)
            if (direction > 0) {
              dx7BankIndex++;
              if (dx7BankIndex > NUM_DX7_BANKS) dx7BankIndex = 0; // 0 to NUM_DX7_BANKS-1, then Back
            } else {
              dx7BankIndex--;
              if (dx7BankIndex < 0) dx7BankIndex = NUM_DX7_BANKS;
            }
          } else if (currentMenuState == DX7_PATCHES) {
            // Navigate patches (0-31, Back)
            if (direction > 0) {
              dx7PatchIndex++;
              if (dx7PatchIndex > 32) dx7PatchIndex = 0; // 0-31=patches, 32=Back
            } else {
              dx7PatchIndex--;
              if (dx7PatchIndex < 0) dx7PatchIndex = 32;
            }
          }
          updateDisplay();
        }
      } else if (currentParentMenu == PARENT_PARAMETERS) {
        // In parameter menus - use existing system for VA, simple for DX7
        if (currentEngine == ENGINE_VA) {
          // Use existing parameter system
          if (getParameterIndex(currentMenuState) >= 0) {
            // On a parameter - adjust value
            int paramIndex = getParameterIndex(currentMenuState);
            if (direction > 0) {
              if (paramIndex == 24) allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] + 0.5, 0.0, 1.0);
              else if (paramIndex == 25) allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] + 0.03, 0.0, 1.0);
              else if (paramIndex == 26) allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] + 0.05, 0.0, 1.0);
              else if (paramIndex == 28) allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] + 0.5, 0.0, 1.0);
              else if (paramIndex == 29) allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] + 0.5, 0.0, 1.0);
              else if (paramIndex == 30) allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] + (1.0/16.0), 0.0, 1.0);
              else allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] + 1.0/128.0, 0.0, 1.0);
              updateParameterFromMenu(paramIndex, allParameterValues[paramIndex]);
            } else {
              if (paramIndex == 24) allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] - 0.5, 0.0, 1.0);
              else if (paramIndex == 25) allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] - 0.03, 0.0, 1.0);
              else if (paramIndex == 26) allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] - 0.05, 0.0, 1.0);
              else if (paramIndex == 28) allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] - 0.5, 0.0, 1.0);
              else if (paramIndex == 29) allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] - 0.5, 0.0, 1.0);
              else if (paramIndex == 30) allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] - (1.0/16.0), 0.0, 1.0);
              else allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] - 1.0/128.0, 0.0, 1.0);
              updateParameterFromMenu(paramIndex, allParameterValues[paramIndex]);
            }
            updateDisplay();
          } else {
            // Navigate through VA menu structure
            if (direction > 0) incrementMenuIndex();
            else decrementMenuIndex();
            updateDisplay();
          }
        } else if (currentEngine == ENGINE_BRAIDS) {
          // Braids parameter navigation
          if (getParameterIndex(currentMenuState) >= 0) {
            // On a Braids parameter - adjust value
            int paramIndex = getParameterIndex(currentMenuState);
            int braidsParamIndex = paramIndex - 100; // Convert to 0-16 range
            
            if (direction > 0) {
              // Increase parameter value
              if (braidsParamIndex == 0) { // Shape - discrete 43 steps (0-42, QPSK is the last)
                braidsParameters[braidsParamIndex] = constrain(braidsParameters[braidsParamIndex] + 1, 0, 42);
              } else if (braidsParamIndex == 8) { // Filter mode - discrete 4 modes
                braidsParameters[braidsParamIndex] = constrain(braidsParameters[braidsParamIndex] + 1, 0, 3);
              } else {
                // Other parameters - smooth adjustment
                braidsParameters[braidsParamIndex] = constrain(braidsParameters[braidsParamIndex] + 1, 0, 127);
              }
            } else {
              // Decrease parameter value
              if (braidsParamIndex == 0) { // Shape
                braidsParameters[braidsParamIndex] = constrain(braidsParameters[braidsParamIndex] - 1, 0, 42);
              } else if (braidsParamIndex == 8) { // Filter mode
                braidsParameters[braidsParamIndex] = constrain(braidsParameters[braidsParamIndex] - 1, 0, 3);
              } else {
                braidsParameters[braidsParamIndex] = constrain(braidsParameters[braidsParamIndex] - 1, 0, 127);
              }
            }
            updateBraidsParameter(braidsParamIndex, braidsParameters[braidsParamIndex]);
            updateDisplay();
          } else {
            // Navigate through Braids parameter menu structure
            if (direction > 0) incrementMenuIndex();
            else decrementMenuIndex();
            updateDisplay();
          }
        }
        // For DX7 parameters, we can add simple parameter control later
      } else if (currentParentMenu == PARENT_EFFECTS) {
        // Effects navigation (similar to settings)
        if (getParameterIndex(currentMenuState) >= 0) {
          int paramIndex = getParameterIndex(currentMenuState);
          if (direction > 0) {
            updateEncoderParameter(paramIndex, 1);
          } else {
            updateEncoderParameter(paramIndex, -1);
          }
          updateParameterFromMenu(paramIndex, allParameterValues[paramIndex]);
          updateDisplay();
        } else {
          if (direction > 0) incrementMenuIndex();
          else decrementMenuIndex();
          updateDisplay();
        }
      } else if (currentParentMenu == PARENT_SETTINGS) {
        // Settings navigation (existing system)
        if (getParameterIndex(currentMenuState) >= 0) {
          int paramIndex = getParameterIndex(currentMenuState);
          if (direction > 0) {
            if (paramIndex == 30) allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] + (1.0/16.0), 0.0, 1.0);
            else allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] + 0.5, 0.0, 1.0);
            updateParameterFromMenu(paramIndex, allParameterValues[paramIndex]);
          } else {
            if (paramIndex == 30) allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] - (1.0/16.0), 0.0, 1.0);
            else allParameterValues[paramIndex] = constrain(allParameterValues[paramIndex] - 0.5, 0.0, 1.0);
            updateParameterFromMenu(paramIndex, allParameterValues[paramIndex]);
          }
          updateDisplay();
        } else {
          if (direction > 0) incrementMenuIndex();
          else decrementMenuIndex(); 
          updateDisplay();
        }
      }
    } else {
      // Not in menu - handle cutoff control
#ifdef USE_OLED_DISPLAY
      encoderValues[11] = menuEncoder.read() / 4; // Adjusted sensitivity for OLED encoder
#else
      encoderValues[11] = menuEncoder.read() / 2; // Standard sensitivity for separate encoder
#endif
    }
    oldMenuValue = newMenuValue;
  }
  
  // Button handling
  static bool lastButtonState = HIGH;
  bool currentButtonState = digitalRead(MENU_ENCODER_SW);
  
  // Debug: Print button state occasionally
  static unsigned long lastButtonDebug = 0;
  if (millis() - lastButtonDebug > 1000) {
    Serial.print("Button pin ");
    Serial.print(MENU_ENCODER_SW);
    Serial.print(" reads: ");
    Serial.println(currentButtonState);
    lastButtonDebug = millis();
  }
  
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    Serial.println("Button clicked!");
    if (!inMenu) {
      Serial.println("Entering menu mode...");
      // Enter parent menu
      enterParentMenuLevel();
    } else if (inParentMenu) {
      // From parent menu, enter selected submenu
      exitParentMenuLevel();
      if (currentParentMenu == PARENT_ENGINES) {
        // Enter engine browse mode
        engineBrowseIndex = (int)currentEngine; // Start with current engine
      } else if (currentParentMenu == PARENT_PRESETS) {
        if (currentEngine == ENGINE_VA) {
          inPresetBrowse = true;
          presetBrowseIndex = 0;
        } else if (currentEngine == ENGINE_JUNO) {
          inPresetBrowse = true;
          junoPresetBrowseIndex = 0;
        } else if (currentEngine == ENGINE_BRAIDS) {
          inPresetBrowse = true;
          braidsPresetBrowseIndex = 0;
        } else if (currentEngine == ENGINE_DX7) {
          // Enter DX7 bank selection mode
          currentMenuState = DX7_BANKS;
          dx7BankIndex = 0;
        }
      } else if (currentParentMenu == PARENT_PARAMETERS) {
        // Enter parameters submenu - stay in parameters list, don't jump to OSC_1
        currentMenuState = PARAMETERS;
        menuIndex = 0;
      } else if (currentParentMenu == PARENT_EFFECTS) {
        // Enter effects submenu
        currentMenuState = EFFECTS;
        menuIndex = 0; // Start at first effects option
      } else if (currentParentMenu == PARENT_SETTINGS) {
        // Enter settings submenu
        currentMenuState = SETTINGS;
        menuIndex = 0;
      }
      updateDisplay();
    } else {
      // In submenu - existing navigation logic
      if (currentParentMenu == PARENT_PRESETS) {
        if (currentEngine == ENGINE_VA && inPresetBrowse) {
          if (presetBrowseIndex == NUM_PRESETS) {
            enterParentMenuLevel(); // Back to parent
          } else {
            loadPreset(presetBrowseIndex); // Stay in presets
          }
          updateDisplay();
        } else if (currentEngine == ENGINE_JUNO && inPresetBrowse) {
          if (junoPresetBrowseIndex == NUM_JUNO_PRESETS) {
            enterParentMenuLevel(); // Back to parent
          } else {
            loadJunoPreset(junoPresetBrowseIndex); // Stay in presets
          }
          updateDisplay();
        } else if (currentEngine == ENGINE_BRAIDS && inPresetBrowse) {
          // Braids preset loading
          if (braidsPresetBrowseIndex == NUM_BRAIDS_PRESETS) {
            enterParentMenuLevel(); // Back to parent
          } else {
            loadBraidsPreset(braidsPresetBrowseIndex); // Stay in presets
          }
          updateDisplay();
        } else if (currentEngine == ENGINE_DX7) {
          // Handle DX7 bank/patch navigation
          if (currentMenuState == DX7_BANKS) {
            // In bank selection
            if (dx7BankIndex == NUM_DX7_BANKS) {
              // Back button - go back to parent menu (Presets)
              enterParentMenuLevel();
            } else {
              // Select bank and go to patches
              currentDX7Bank = dx7BankIndex;
              currentMenuState = DX7_PATCHES;
              dx7PatchIndex = 0;
            }
          } else if (currentMenuState == DX7_PATCHES) {
            // In patch selection
            if (dx7PatchIndex == 32) {
              // Back to banks
              currentMenuState = DX7_BANKS;
            } else {
              // Load selected patch
              loadDX7Preset(dx7PatchIndex);
            }
          }
          updateDisplay();
        }
      } else if (currentParentMenu == PARENT_ENGINES) {
        // Engine click-to-select handling
        if (engineBrowseIndex == 4) {
          // "Back" option selected
          enterParentMenuLevel();
        } else {
          // Engine selection - switch to the browsed engine
          EngineType newEngine;
          switch(engineBrowseIndex) {
            case 0: newEngine = ENGINE_VA; break;
            case 1: newEngine = ENGINE_DX7; break;
            case 2: newEngine = ENGINE_JUNO; break;
            case 3: newEngine = ENGINE_BRAIDS; break;
            default: newEngine = ENGINE_VA; break;
          }
          switchEngine(newEngine);
          // Stay in engines menu after switching
        }
        updateDisplay();
      } else if (currentParentMenu == PARENT_PARAMETERS) {
        if (currentEngine == ENGINE_VA) {
          // VA parameter navigation
          if (getParameterIndex(currentMenuState) >= 0) {
            backMenuAction();
          } else if (currentMenuState == MACRO_KNOBS) {
            macroMode = !macroMode;
          } else {
            // Check if we're in a submenu with "< Back" option
            bool isBackButtonSelected = false;
            if ((currentMenuState == OSC_1 || currentMenuState == OSC_2 || currentMenuState == OSC_3 || 
                 currentMenuState == LFO || currentMenuState == VOICE_MODE) && menuIndex == 4) {
              isBackButtonSelected = true; // These menus have "< Back" at index 4
            } else if (currentMenuState == ENVELOPES && menuIndex == 6) {
              isBackButtonSelected = true; // Envelopes has "< Back" at index 6
            } else if (currentMenuState == NOISE && menuIndex == 2) {
              isBackButtonSelected = true; // Noise has "< Back" at index 2
            } else if (currentMenuState == FILTER && menuIndex == 3) {
              isBackButtonSelected = true; // Filter has "< Back" at index 3
            }
            
            if (isBackButtonSelected) {
              // "< Back" option selected
              backMenuAction();
            } else if (currentMenuState == PARAMETERS && menuIndex == 8) {
              // "< Back" from main Parameters menu
              enterParentMenuLevel();
            } else {
              // Normal forward navigation
              navigateMenuForward();
            }
          }
        } else if (currentEngine == ENGINE_BRAIDS) {
          // Braids parameter navigation
          if (getParameterIndex(currentMenuState) >= 0) {
            // On a parameter, go back to menu
            backMenuAction();
          } else if (currentMenuState == PARAMETERS && menuIndex == 17) {
            // "< Back" from main Parameters menu for Braids (17 params + back = index 17)
            enterParentMenuLevel();
          } else {
            // Navigate forward in Braids parameter menu
            navigateMenuForward();
          }
        }
        updateDisplay();
      } else if (currentParentMenu == PARENT_EFFECTS) {
        // Effects navigation (similar to settings)
        if (getParameterIndex(currentMenuState) >= 0) {
          backMenuAction();
        } else {
          navigateMenuForward();
        }
        updateDisplay();
      } else if (currentParentMenu == PARENT_SETTINGS) {
        // Settings navigation
        if (getParameterIndex(currentMenuState) >= 0) {
          backMenuAction();
        } else {
          navigateMenuForward();
        }
        updateDisplay();
      } else {
        // Default back to parent
        enterParentMenuLevel();
      }
    }
  }
  
  lastButtonState = currentButtonState;
}

void resetEncoderBaselines() {
  // Reset all encoder positions to match current parameter values
  // This syncs the encoder counting with the actual parameter state
  Serial.println("Resetting encoder baselines to current parameter values...");
  
  for (int i = 0; i < 20; i++) {
    // Calculate what the encoder value should be based on current parameter
    long targetEncoderValue = (long)(allParameterValues[i] * 100);
    
    // Reset the encoder objects to this baseline value
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
      case 12: enc13.write(targetEncoderValue * 4); break;
      case 13: enc14.write(targetEncoderValue * 4); break;
      case 14: enc15.write(targetEncoderValue * 4); break;
      case 15: enc16.write(targetEncoderValue * 4); break;
      case 16: enc17.write(targetEncoderValue * 4); break;
      case 17: enc18.write(targetEncoderValue * 4); break;
      case 18: enc19.write(targetEncoderValue * 4); break;
      case 19: enc20.write(targetEncoderValue * 4); break;
    }
    
    // Update our tracking arrays
    encoderValues[i] = targetEncoderValue;
    lastEncoderValues[i] = targetEncoderValue;
  }
  
  Serial.println("Encoder baselines reset. Physical knob positions now match parameter values.");
}

void updateDisplay() {
  // Throttle display updates to prevent corruption
  static unsigned long lastDisplayUpdate = 0;
  unsigned long now = millis();
  if (now - lastDisplayUpdate < 25) return; // Limit to 40Hz updates
  lastDisplayUpdate = now;
   String line1, line2;
  if (inMenu) {
   
    
    if (inParentMenu) {
      // Parent menu display
      line1 = "Menu";
      switch(currentParentMenu) {
        case PARENT_ENGINES:
          line2 = "Engines";
          break;
        case PARENT_PRESETS:
          line2 = "Presets";
          break;
        case PARENT_PARAMETERS:
          line2 = "Parameters";
          break;
        case PARENT_EFFECTS:
          line2 = "Effects";
          break;
        case PARENT_SETTINGS:
          line2 = "Settings";
          break;
      }
      displayText(line1, line2);
    } else {
      // Submenu displays
      if (currentParentMenu == PARENT_ENGINES) {
        // Engine selection display
        line1 = "Select Engine:";
        if (engineBrowseIndex == 0) {
          line2 = "VA Analog";
        } else if (engineBrowseIndex == 1) {
          line2 = "DX7 FM";
        } else if (engineBrowseIndex == 2) {
          line2 = "Juno-60";
        } else if (engineBrowseIndex == 3) {
          line2 = "Macro-Oscillator";
        } else {
          line2 = "< Back";
        }
        displayText(line1, line2);
      } else if (currentParentMenu == PARENT_PRESETS) {
        if (currentEngine == ENGINE_VA) {
          // VA preset browsing (existing system)
          if (inPresetBrowse) {
            line1 = "VA Presets";
            if (presetBrowseIndex == NUM_PRESETS) {
              line2 = "< Back";
            } else {
              line2 = String(presetBrowseIndex + 1) + ". " + presets[presetBrowseIndex].name;
            }
            displayText(line1, line2);
          } else {
            // Should not reach here if logic is correct, but just in case
            displayText("VA Presets", "Loading...");
            // Auto-enter browse mode if somehow we're here
            inPresetBrowse = true;
            presetBrowseIndex = 0;
          }
        } else if (currentEngine == ENGINE_JUNO) {
          // Juno preset browsing
          if (inPresetBrowse) {
            line1 = "Juno Presets";
            if (junoPresetBrowseIndex == NUM_JUNO_PRESETS) {
              line2 = "< Back";
            } else {
              line2 = String(junoPresetBrowseIndex + 1) + ". " + junoPresets[junoPresetBrowseIndex].name;
            }
            displayText(line1, line2);
          } else {
            displayText("Juno Presets", "Loading...");
            inPresetBrowse = true;
            junoPresetBrowseIndex = 0;
          }
        } else if (currentEngine == ENGINE_BRAIDS) {
          // Braids preset browsing
          if (inPresetBrowse) {
            line1 = "Macro-Osc Presets";
            if (braidsPresetBrowseIndex == NUM_BRAIDS_PRESETS) {
              line2 = "< Back";
            } else {
              line2 = String(braidsPresetBrowseIndex + 1) + ". " + String(braidsPresets[braidsPresetBrowseIndex].name);
            }
            displayText(line1, line2);
          } else {
            displayText("Macro-Osc Presets", "Loading...");
            inPresetBrowse = true;
            braidsPresetBrowseIndex = 0;
          }
        } else if (currentEngine == ENGINE_DX7) {
          // DX7 bank/patch browsing
          if (currentMenuState == DX7_BANKS) {
            // Bank selection menu
            line1 = "DX7 Banks";
            if (dx7BankIndex < NUM_DX7_BANKS) {
              line2 = String(dx7BankIndex + 1) + ". " + String(dx7BankNames[dx7BankIndex]);
            } else {
              line2 = "< Back";
            }
            displayText(line1, line2);
          } else if (currentMenuState == DX7_PATCHES) {
            // Patch selection menu
            line1 = String(dx7BankNames[currentDX7Bank]) + " Patches";
            if (dx7PatchIndex == 32) {
              line2 = "< Back";
            } else {
              // Extract and display patch name being browsed
              char voice_name[11];
              memset(voice_name, 0, 11);
              memcpy(voice_name, &progmem_bank[currentDX7Bank][dx7PatchIndex][144], 10);
              line2 = String(dx7PatchIndex + 1) + ". " + String(voice_name);
            }
            displayText(line1, line2);
          }
        }
      } else if (currentParentMenu == PARENT_PARAMETERS) {
        if (currentEngine == ENGINE_VA) {
          // Use existing VA menu system
          switch(currentMenuState) {
            case PARAMETERS:
              line1 = "Parameters";
              if (menuIndex == 0) line2 = "Oscillator 1";
              else if (menuIndex == 1) line2 = "Oscillator 2";
              else if (menuIndex == 2) line2 = "Oscillator 3";
              else if (menuIndex == 3) line2 = "Noise";
              else if (menuIndex == 4) line2 = "Envelopes";
              else if (menuIndex == 5) line2 = "Filter";
              else if (menuIndex == 6) line2 = "LFO";
              else if (menuIndex == 7) line2 = "Voice Mode";
              else if (menuIndex == 8) line2 = "< Back";
              displayText(line1, line2);
              break;
              
            case OSC_1:
              line1 = "Oscillator 1";
              if (menuIndex == 0) line2 = "Range";
              else if (menuIndex == 1) line2 = "Waveform";
              else if (menuIndex == 2) line2 = "Volume";
              else if (menuIndex == 3) line2 = "Fine Tune";
              else if (menuIndex == 4) line2 = "< Back";
              displayText(line1, line2);
              break;
          
        case OSC_2:
          line1 = "Oscillator 2";
          if (menuIndex == 0) line2 = "Range";
          else if (menuIndex == 1) line2 = "Waveform";
          else if (menuIndex == 2) line2 = "Volume";
          else if (menuIndex == 3) line2 = "Fine Tune";
          else if (menuIndex == 4) line2 = "< Back";
          displayText(line1, line2);
          break;
          
        case OSC_3:
          line1 = "Oscillator 3";
          if (menuIndex == 0) line2 = "Range";
          else if (menuIndex == 1) line2 = "Waveform";
          else if (menuIndex == 2) line2 = "Volume";
          else if (menuIndex == 3) line2 = "Fine Tune";
          else if (menuIndex == 4) line2 = "< Back";
          displayText(line1, line2);
          break;
          
        case NOISE:
          line1 = "Noise";
          if (menuIndex == 0) line2 = "Volume";
          else if (menuIndex == 1) line2 = "Type";
          else if (menuIndex == 2) line2 = "< Back";
          displayText(line1, line2);
          break;
          
        case ENVELOPES:
          line1 = "Envelopes";
          if (menuIndex == 0) line2 = "Amp Attack";
          else if (menuIndex == 1) line2 = "Amp Sustain";
          else if (menuIndex == 2) line2 = "Amp Decay";
          else if (menuIndex == 3) line2 = "Filter Attack";
          else if (menuIndex == 4) line2 = "Filter Decay";
          else if (menuIndex == 5) line2 = "Filter Sustain";
          else if (menuIndex == 6) line2 = "< Back";
          displayText(line1, line2);
          break;
          
        case FILTER:
          line1 = "Filter";
          if (menuIndex == 0) line2 = "Cutoff";
          else if (menuIndex == 1) line2 = "Resonance";
          else if (menuIndex == 2) line2 = "Strength";
          else if (menuIndex == 3) line2 = "< Back";
          displayText(line1, line2);
          break;
          
        case LFO:
          line1 = "LFO";
          if (menuIndex == 0) line2 = "Rate";
          else if (menuIndex == 1) line2 = "Depth";
          else if (menuIndex == 2) line2 = "Toggle";
          else if (menuIndex == 3) line2 = "Target";
          else if (menuIndex == 4) line2 = "< Back";
          displayText(line1, line2);
          break;
          
        case VOICE_MODE:
          line1 = "Voice Mode";
          if (menuIndex == 0) line2 = "Play Mode";
          else if (menuIndex == 1) line2 = "Glide Time";
          else if (menuIndex == 2) line2 = "< Back";
          displayText(line1, line2);
          break;
          
        case EFFECTS:
          line1 = "Effects";
          if (menuIndex == 0) line2 = "Chorus Bypass";
          else if (menuIndex == 1) line2 = "Chorus Rate";
          else if (menuIndex == 2) line2 = "Chorus Depth";
          else if (menuIndex == 3) line2 = "Reverb Bypass";
          else if (menuIndex == 4) line2 = "Reverb Size";
          else if (menuIndex == 5) line2 = "Reverb HiDamp";
          else if (menuIndex == 6) line2 = "Reverb LoDamp";
          else if (menuIndex == 7) line2 = "Reverb Lowpass";
          else if (menuIndex == 8) line2 = "Reverb Diffusion";
          else if (menuIndex == 9) line2 = "< Back";
          displayText(line1, line2);
          break;
          
        case SETTINGS:
          line1 = "Settings";
          if (menuIndex == 0) line2 = "Macro Knobs";
          else if (menuIndex == 1) line2 = "MIDI Channel";
          else if (menuIndex == 2) line2 = "< Back";
          displayText(line1, line2);
          break;
          
        case MACRO_KNOBS:
          line1 = "Filter Knobs:";
          if (macroMode) {
            line2 = "LFO Controls";
          } else {
            line2 = "Filter Env";
          }
          displayText(line1, line2);
          break;
          
        case MIDI_CHANNEL:
          line1 = "MIDI Channel:";
          if (midiChannel == 0) {
            line2 = "Omni";
          } else {
            line2 = String(midiChannel);
          }
          displayText(line1, line2);
          break;
          
        default:
          // Parameter editing
          int paramIndex = getParameterIndex(currentMenuState);
          if (paramIndex >= 0) {
            line1 = controlNames[paramIndex];
            // Always show current value (no "Click to edit" screen)
            if (paramIndex == 3 || paramIndex == 4 || paramIndex == 20) { // Extended fine tuning
              float val = allParameterValues[paramIndex];
              if (val <= 0.25) {
                // Semitone range (negative)
                float semiRange = val / 0.25;
                int semitones = (int)(-12 + (semiRange * 11)); // -12 to -1
                line2 = String(semitones) + "st";
              } else if (val >= 0.75) {
                // Semitone range (positive)
                float semiRange = (val - 0.75) / 0.25;
                int semitones = (int)(1 + (semiRange * 11)); // +1 to +12
                line2 = "+" + String(semitones) + "st";
              } else {
                // Cents range (±25 cents)
                int cents = (int)((val - 0.5) * 100); // -25 to +25 cents
                if (cents >= 0) {
                  line2 = "+" + String(cents) + "c";
                } else {
                  line2 = String(cents) + "c";
                }
              }
            } else if (paramIndex == 22) { // LFO Rate
              float rate = 0.1 + allParameterValues[paramIndex] * 19.9;
              line2 = String(rate, 1) + " Hz";
            } else if (paramIndex == 23) { // LFO Depth
              int depth = (int)(allParameterValues[paramIndex] * 100);
              line2 = String(depth) + "%";
            } else if (paramIndex == 24) { // LFO Toggle
              line2 = lfoEnabled ? "ON" : "OFF"; // Use actual variable instead of parameter
            } else if (paramIndex == 25) { // LFO Target
              if (lfoTarget == 0) line2 = "Pitch";
              else if (lfoTarget == 1) line2 = "Filter";
              else line2 = "Amp";
            } else if (paramIndex == 26) { // Play Mode
              if (playMode == 0) line2 = "Mono";
              else if (playMode == 1) line2 = "Poly";
              else line2 = "Legato";
            } else if (paramIndex == 27) { // Glide Time
              if (glideTime == 0.0) {
                line2 = "OFF";
              } else {
                float timeMs = 50 + (glideTime * 950); // 50ms to 1000ms
                line2 = String((int)timeMs) + "ms";
              }
            } else if (paramIndex == 28) { // Noise Type
              if (noiseType == 0) {
                line2 = "White";
              } else {
                line2 = "Pink";
              }
            } else if (paramIndex == 29) { // Macro Mode
              line2 = macroMode ? "LFO Controls" : "Filter Env";
            } else if (paramIndex == 30) { // MIDI Channel
              if (midiChannel == 0) {
                line2 = "Omni";
              } else {
                line2 = String(midiChannel);
              }
            } else if (paramIndex == 31) { // Chorus Bypass
              line2 = chorusBypass ? "BYPASSED" : "ACTIVE";
            } else if (paramIndex == 32) { // Chorus Rate
              line2 = String(chorusRate, 2) + " Hz";
            } else if (paramIndex == 33) { // Chorus Depth
              int depthPercent = (int)(chorusDepth * 100);
              line2 = String(depthPercent) + "%";
            } else if (paramIndex == 34) { // Reverb Bypass
              line2 = reverbBypass ? "BYPASSED" : "ACTIVE";
            } else if (paramIndex == 35) { // Reverb Size
              int sizePercent = (int)(reverbSize * 100);
              line2 = String(sizePercent) + "%";
            } else if (paramIndex == 36) { // Reverb HiDamp
              int dampPercent = (int)(reverbHidamp * 100);
              line2 = String(dampPercent) + "%";
            } else if (paramIndex >= 37 && paramIndex <= 39) { // Other reverb parameters
              int displayValue = (int)(allParameterValues[paramIndex] * 100);
              line2 = String(displayValue) + "%";
            } else {
              // Show 0-127 values (MIDI standard)
              int displayValue = (int)(allParameterValues[paramIndex] * 127);
              line2 = String(displayValue);
            }
            displayText(line1, line2);
          }
          break;
          }
        } else if (currentEngine == ENGINE_BRAIDS) {
          // Braids parameters display system
          switch(currentMenuState) {
            case PARAMETERS:
              line1 = "Macro-Osc Params";
              if (menuIndex == 0) line2 = "Shape";
              else if (menuIndex == 1) line2 = "Timbre";
              else if (menuIndex == 2) line2 = "Color";
              else if (menuIndex == 3) line2 = "Coarse";
              else if (menuIndex == 4) line2 = "Amp Attack";
              else if (menuIndex == 5) line2 = "Amp Decay";
              else if (menuIndex == 6) line2 = "Amp Sustain";
              else if (menuIndex == 7) line2 = "Amp Release";
              else if (menuIndex == 8) line2 = "Filter Mode";
              else if (menuIndex == 9) line2 = "Filter Cutoff";
              else if (menuIndex == 10) line2 = "Filter Res";
              else if (menuIndex == 11) line2 = "Filter Strength";
              else if (menuIndex == 12) line2 = "Filt Attack";
              else if (menuIndex == 13) line2 = "Filt Decay";
              else if (menuIndex == 14) line2 = "Filt Sustain";
              else if (menuIndex == 15) line2 = "Filt Release";
              else if (menuIndex == 16) line2 = "Volume";
              else if (menuIndex == 17) line2 = "< Back";
              displayText(line1, line2);
              break;
              
            default:
              // Parameter editing for Braids
              int paramIndex = getParameterIndex(currentMenuState);
              if (paramIndex >= 0) {
                int braidsParamIndex = paramIndex - 100; // Convert to 0-16 range
                line1 = braidsControlNames[braidsParamIndex];
                
                // Braids-specific value displays
                if (braidsParamIndex == 0) { // Shape
                  int shape = (int)braidsParameters[braidsParamIndex];
                  if (shape >= 0 && shape <= 42) {
                    line2 = braidsAlgorithmNames[shape];
                  } else {
                    line2 = "Shape " + String(shape);
                  }
                } else if (braidsParamIndex == 8) { // Filter Mode
                  int mode = (int)braidsParameters[braidsParamIndex];
                  if (mode == 0) line2 = "Off";
                  else if (mode == 1) line2 = "LowPass";
                  else if (mode == 2) line2 = "BandPass";
                  else line2 = "HighPass";
                } else {
                  // Show 0-127 values for other parameters
                  int displayValue = (int)braidsParameters[braidsParamIndex];
                  line2 = String(displayValue);
                }
                displayText(line1, line2);
              }
              break;
          }
        } else {
          // DX7 parameters - simple display for now
          displayText("DX7 Parameters", "Coming soon...");
        }
      } else if (currentParentMenu == PARENT_EFFECTS) {
        // Effects menu (similar to settings)
        switch(currentMenuState) {
          case EFFECTS:
            line1 = "Effects";
            if (menuIndex == 0) line2 = "Chorus Bypass";
            else if (menuIndex == 1) line2 = "Chorus Rate";
            else if (menuIndex == 2) line2 = "Chorus Depth";
            else if (menuIndex == 3) line2 = "Reverb Bypass";
            else if (menuIndex == 4) line2 = "Reverb Size";
            else if (menuIndex == 5) line2 = "Reverb HiDamp";
            else if (menuIndex == 6) line2 = "Reverb LoDamp";
            else if (menuIndex == 7) line2 = "Reverb Lowpass";
            else if (menuIndex == 8) line2 = "Reverb Diffusion";
            else if (menuIndex == 9) line2 = "< Back";
            displayText(line1, line2);
            break;
            
          default:
            // Parameter editing for effects
            int paramIndex = getParameterIndex(currentMenuState);
            if (paramIndex >= 0) {
              line1 = controlNames[paramIndex];
              // Effect-specific value displays
              if (paramIndex == 31) { // Chorus Bypass
                line2 = chorusBypass ? "BYPASSED" : "ACTIVE";
              } else if (paramIndex == 34) { // Reverb Bypass
                line2 = reverbBypass ? "BYPASSED" : "ACTIVE";
              } else {
                // Show percentage for other effects
                int percentage = (int)(allParameterValues[paramIndex] * 100);
                line2 = String(percentage) + "%";
              }
              displayText(line1, line2);
            }
            break;
        }
      } else if (currentParentMenu == PARENT_SETTINGS) {
        // Settings menu (existing system)
        switch(currentMenuState) {
          case SETTINGS:
            line1 = "Settings";
            if (menuIndex == 0) line2 = "Macro Knobs";
            else if (menuIndex == 1) line2 = "MIDI Channel";
            else if (menuIndex == 2) line2 = "< Back";
            displayText(line1, line2);
            break;
            
          case MACRO_KNOBS:
            line1 = "Filter Knobs:";
            if (macroMode) {
              line2 = "LFO Controls";
            } else {
              line2 = "Filter Env";
            }
            displayText(line1, line2);
            break;
            
          case MIDI_CHANNEL:
            line1 = "MIDI Channel:";
            if (midiChannel == 0) {
              line2 = "Omni";
            } else {
              line2 = String(midiChannel);
            }
            displayText(line1, line2);
            break;
            
          default:
            // Parameter editing for settings
            int paramIndex = getParameterIndex(currentMenuState);
            if (paramIndex >= 0) {
              line1 = controlNames[paramIndex];
              if (paramIndex == 30) { // MIDI Channel
                if (midiChannel == 0) {
                  line2 = "Omni";
                } else {
                  line2 = String(midiChannel);
                }
              } else {
                line2 = macroMode ? "LFO Controls" : "Filter Env";
              }
              displayText(line1, line2);
            }
            break;
        }
      }
    }
  } else {
    line1 = "Multi-Teensy";
    
    // Count active voices
    int activeVoices = 0;
    for (int v = 0; v < VOICES; v++) {
      if (voices[v].active) activeVoices++;
    }
    
    if (activeVoices > 0) {
      line2 = "Voices: " + String(activeVoices);
    } else {
      line2 = "Press for menu";
    }
    displayText(line1, line2);
  }
}