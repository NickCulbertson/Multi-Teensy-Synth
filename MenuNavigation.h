#ifndef MENU_NAVIGATION_H
#define MENU_NAVIGATION_H

#include "config.h"
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

// Menu state enums  
typedef enum {
  PARENT_ENGINES = 0,
  PARENT_PRESETS = 1,
  PARENT_PARAMETERS = 2,
  PARENT_EFFECTS = 3,
  PARENT_SETTINGS = 4
} ParentMenuState;

enum MenuState {
  PARENT_MENU,
  PARAMETERS,  // Parameters submenu (lists OSC_1, OSC_2, etc.)
  OSC_1, 
  OSC_2,
  OSC_3,
  NOISE,
  ENVELOPES,
  FILTER,
  LFO,
  VOICE_MODE,
  // Oscillator 1 sub-menus
  OSC1_RANGE,
  OSC1_WAVE,
  OSC1_VOLUME,
  OSC1_FINE,
  // Oscillator 2 sub-menus  
  OSC2_RANGE,
  OSC2_WAVE,
  OSC2_VOLUME,
  OSC2_FINE,
  // Oscillator 3 sub-menus
  OSC3_RANGE,
  OSC3_WAVE,
  OSC3_VOLUME,
  OSC3_FINE,
  // Noise sub-menu
  NOISE_VOLUME,
  // Envelope sub-menus
  AMP_ATTACK,
  AMP_SUSTAIN,
  AMP_DECAY,
  FILTER_ATTACK,
  FILTER_DECAY,
  FILTER_SUSTAIN,
  // Filter sub-menus
  CUTOFF,
  RESONANCE,
  FILTER_STRENGTH,
  // LFO sub-menus
  LFO_RATE,
  LFO_DEPTH,
  LFO_TOGGLE,
  LFO_TARGET,
  // Voice Mode sub-menus
  PLAY_MODE,
  GLIDE_TIME,
  NOISE_TYPE,
  // Settings sub-menus
  SETTINGS,
  MACRO_KNOBS,
  MIDI_CHANNEL,
  // Effects sub-menus
  EFFECTS,
  CHORUS_BYPASS,
  CHORUS_RATE,
  CHORUS_DEPTH,
  REVERB_BYPASS,
  REVERB_SIZE,
  REVERB_HIDAMP,
  REVERB_LODAMP,
  REVERB_LOWPASS,
  REVERB_DIFFUSION,
  DX7_BANKS,     // DX7 bank selection menu
  DX7_PATCHES    // DX7 patch selection within a bank
};

// EngineType now defined in config.h

// Menu navigation functions
void displayText(String line1, String line2);
void updateParameterFromMenu(int paramIndex, float val);
void updateEncoderParameter(int paramIndex, int change);
void navigateMenuForward();
void incrementMenuIndex();
void decrementMenuIndex();
void navigateMenuBackward();
void backMenuAction();
void handleParentMenu(int direction);
void handleEngineMenu(int direction);
void handleDX7PresetMenu(int direction);
void enterParentMenuLevel();
void exitParentMenuLevel();
void handleEncoder();
void resetEncoderBaselines();
void updateDisplay();

// Utility functions
int getParameterIndex(MenuState state);
float getOscillatorRange(float val);
int getRangeIndex(float val);
int getWaveformIndex(float val, int osc);
int getMiniTeensyWaveform(float val, int osc);

// External variables that menu system needs access to
extern bool inMenu;
extern bool inParentMenu;
extern ParentMenuState currentParentMenu;
extern MenuState currentMenuState;
extern int menuIndex;
extern EngineType currentEngine;

// Engine state variables
extern bool inPresetBrowse;
extern int presetBrowseIndex;
extern int junoPresetBrowseIndex;
extern int dx7BrowseIndex;
extern int engineBrowseIndex;

// Parameter arrays and names
extern float allParameterValues[41];
extern const char* controlNames[41];
extern const char* waveformNames[6];
extern const char* rangeNames[6];

// Settings variables
extern bool macroMode;
extern int midiChannel;
extern bool lfoEnabled;
extern int lfoTarget;
extern int playMode;
extern float glideTime;

// Effects variables
extern bool chorusBypass;
extern bool reverbBypass;
extern float chorusRate;
extern float chorusDepth;

// DX7 bank/patch selection
extern int currentDX7Bank;    // Currently selected bank (0-1)
extern int dx7BankIndex;      // Bank browser index  
extern int dx7PatchIndex;     // Patch browser index
extern const char* dx7BankNames[];   // Bank names (size determined by sysex2c.py)

// Hardware pin definitions now in config.h

// Hardware encoder objects
extern Encoder menuEncoder;

// External functions that menu system needs to call
extern void updateSynthParameter(int paramIndex, float val);
extern void applyJunoParameterMapping();
extern void switchEngine(EngineType newEngine);
extern void loadPreset(int presetIndex);
extern void loadJunoPreset(int presetIndex);
extern void loadDX7Preset(int presetIndex);

// Preset constants now defined in config.h

// Forward declarations for preset structures  
struct MiniTeensyPreset {
  const char* name;
  float parameters[41]; // Unified 41-parameter structure including effects
};

struct JunoPreset {
  const char* name;
  float parameters[41]; // Unified 41-parameter structure including effects
};

extern const MiniTeensyPreset presets[];
extern const JunoPreset junoPresets[];

#endif // MENU_NAVIGATION_H