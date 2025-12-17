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
  DX7_PATCHES,   // DX7 patch selection within a bank
  // Braids parameter sub-menus
  BRAIDS_SHAPE,
  BRAIDS_TIMBRE,
  BRAIDS_COLOR,
  BRAIDS_COARSE,
  BRAIDS_AMP_ATTACK,
  BRAIDS_AMP_DECAY,
  BRAIDS_AMP_SUSTAIN,
  BRAIDS_AMP_RELEASE,
  BRAIDS_FILTER_MODE,
  BRAIDS_FILTER_CUTOFF,
  BRAIDS_FILTER_RESONANCE,
  BRAIDS_FILTER_STRENGTH,
  BRAIDS_FILTER_ATTACK,
  BRAIDS_FILTER_DECAY,
  BRAIDS_FILTER_SUSTAIN,
  BRAIDS_FILTER_RELEASE,
  BRAIDS_VOLUME,
  // Juno-60 parameter sub-menus (individual volume controls + LFO)
  JUNO_PWM_VOLUME,
  JUNO_PWM_WIDTH,
  JUNO_SAW_VOLUME,
  JUNO_SUB_VOLUME,
  JUNO_NOISE_VOLUME,
  JUNO_LFO_RATE,
  JUNO_LFO_DELAY,
  JUNO_LFO_PWM,
  JUNO_LFO_PITCH,
  JUNO_LFO_FILTER,
  JUNO_HPF_CUTOFF,
  JUNO_LPF_CUTOFF,
  JUNO_LPF_RESONANCE,
  JUNO_FILTER_ENV,
  JUNO_FILTER_ATTACK,
  JUNO_FILTER_DECAY,
  JUNO_FILTER_SUSTAIN,
  JUNO_FILTER_RELEASE,
  JUNO_AMP_ATTACK,
  JUNO_AMP_DECAY,
  JUNO_AMP_SUSTAIN,
  JUNO_AMP_RELEASE
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
void printCurrentPresetValues();

// Utility functions
int getParameterIndex(MenuState state);
float getOscillatorRange(float val);
int getRangeIndex(float val);
int getWaveformIndex(float val, int osc);
int getMiniTeensyWaveform(float val, int osc);

// Engine-specific parameter index functions
int getVAParameterIndex(MenuState state);
int getBraidsParameterIndex(MenuState state);
int getDX7ParameterIndex(MenuState state);
int getJunoParameterIndex(MenuState state);
int getEffectsParameterIndex(MenuState state);

// Engine-specific menu helpers
int getEngineParameterMenuCount();
void navigateVAParameterMenu(int menuIndex);
void navigateBraidsParameterMenu(int menuIndex);
void navigateJunoParameterMenu(int menuIndex);

// Engine-specific parameter adjustment helpers
void adjustVAParameter(int paramIndex, int direction);
void adjustBraidsParameter(int paramIndex, int direction);
void adjustJunoParameter(int paramIndex, int direction);

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
extern int braidsPresetBrowseIndex;
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
extern void loadBraidsPreset(int presetIndex);

// Preset constants now defined in config.h

// Forward declarations for preset structures  
struct MiniTeensyPreset {
  const char* name;
  float parameters[41]; // Unified 41-parameter structure including effects
};

struct JunoPreset {
  const char* name;
  float parameters[22]; // Native Juno 22-parameter structure (added 3 LFO amounts)
};

extern const MiniTeensyPreset presets[];
extern const JunoPreset junoPresets[];

// Braids preset structure (shared between .ino and MenuNavigation.cpp)
struct BraidsPreset {
  const char* name;
  uint8_t shape;     // Oscillator shape (0-47)
  uint8_t timbre;    // Timbre parameter (0-127)
  uint8_t color;     // Color parameter (0-127)
  uint8_t attack;    // Envelope attack (0-127)
  uint8_t decay;     // Envelope decay (0-127)
  uint8_t sustain;   // Envelope sustain (0-127)
  uint8_t release;   // Envelope release (0-127)
  uint16_t filterFreq; // Filter frequency (50-8000)
  uint8_t filterRes; // Filter resonance (0-127)
};

extern const BraidsPreset braidsPresets[];

// Braids parameter system
extern float braidsParameters[];
extern const char* braidsControlNames[];
extern const char* braidsAlgorithmNames[];
extern void updateBraidsParameter(int paramIndex, float value);

// Juno parameter system
extern float junoParameters[];
extern const char* junoControlNames[];
extern void updateJunoParameter(int paramIndex, float value);

#endif // MENU_NAVIGATION_H