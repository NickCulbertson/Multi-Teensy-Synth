/*
 * Braids-Teensy Synth v1.0
 * A polyphonic macro-oscillator synthesizer built with the Teensy 4.1 microcontroller,
 * using the Mutable Instruments Braids sound engine with intuitive menu control.
 * 
 * REQUIRED LIBRARIES (install via Arduino Library Manager):
 * - LiquidCrystal I2C (by Frank de Brabander) - for LCD display
 * - Adafruit SSD1306 (by Adafruit) - for OLED display  
 * - Adafruit GFX Library (by Adafruit) - for OLED display
 * - Encoder (by Paul Stoffregen) 
 * - MIDI Library (by Francois Best) - only needed if enabling DIN MIDI
 * 
 * Built-in Teensy libraries (no installation needed):
 * - Audio, Wire, USBHost_t36
 */

#define NUM_PARAMETERS 22  // Braids with Moog filter (removed Filter Mode, using fixed lowpass)
#define NUM_PRESETS 8      // Braids has 8 built-in presets
#define VOICES 6           // Braids supports up to 6 voices (resource intensive)

#include "config.h"
#include "MenuNavigation.h"

// Braids presets (exact copy from working Multi-Teensy-Synth-copy)
const BraidsPreset braidsPresets[NUM_PRESETS] = {
  {"CSaw Lead", 0, 64, 32, 5, 20, 100, 50, 40, 3000, 30},       // Classic saw lead
  {"Sine Pad", 8, 20, 80, 40, 80, 120, 100, 60, 1500, 10},     // Sine wave pad
  {"FM Bell", 16, 90, 60, 10, 60, 80, 80, 50, 4000, 20},       // FM-style bell
  {"Noise Drum", 24, 127, 100, 1, 30, 0, 20, 30, 2000, 50},    // Percussion
  {"Vocal Formant", 32, 80, 90, 15, 40, 90, 60, 50, 2500, 40}, // Vocal-like
  {"Digital Harsh", 40, 120, 127, 5, 25, 70, 30, 40, 5000, 80}, // Digital/harsh
  {"Analog Sync", 4, 100, 70, 8, 35, 85, 45, 50, 3500, 60},    // Sync sweep
  {"Pluck Bass", 12, 60, 40, 3, 15, 60, 25, 30, 800, 20},      // Bass pluck
};

// Project strings
const char* PROJECT_NAME = "Braids Synth";
const char* PROJECT_SUBTITLE = "Macro Oscillator";

#include <USBHost_t36.h>
#include <Audio.h>
#include <Wire.h>
#include <Encoder.h>

// Braids Macro-Oscillator Engine
#include "src/synth_braids.h"

#ifdef USE_LCD_DISPLAY
  #include <LiquidCrystal_I2C.h>
#endif

#ifdef USE_OLED_DISPLAY
  #include <U8g2lib.h>
  #define OLED_WIDTH 128
  #define OLED_HEIGHT 64
  #define OLED_RESET -1
#endif

#ifdef ENABLE_DIN_MIDI
#include <MIDI.h>
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);
#endif

#ifdef USE_MIDI_HOST
// USB Host MIDI device for external controllers
USBHost myusb;
MIDIDevice midi1(myusb);
#endif

// All 19 Mini-Teensy Encoders (using configurable pin definitions from config.h)
Encoder enc1(ENC_1_CLK, ENC_1_DT);
Encoder enc2(ENC_2_CLK, ENC_2_DT);
Encoder enc3(ENC_3_CLK, ENC_3_DT);
Encoder enc4(ENC_4_CLK, ENC_4_DT);
Encoder enc5(ENC_5_CLK, ENC_5_DT);
Encoder enc6(ENC_6_CLK, ENC_6_DT);
Encoder enc7(ENC_7_CLK, ENC_7_DT);
Encoder enc8(ENC_8_CLK, ENC_8_DT);
Encoder enc9(ENC_9_CLK, ENC_9_DT);
Encoder enc10(ENC_10_CLK, ENC_10_DT);
Encoder enc11(ENC_11_CLK, ENC_11_DT);
Encoder enc13(ENC_13_CLK, ENC_13_DT);  
Encoder enc14(ENC_14_CLK, ENC_14_DT);
Encoder enc15(ENC_15_CLK, ENC_15_DT);
Encoder enc16(ENC_16_CLK, ENC_16_DT);
Encoder enc17(ENC_17_CLK, ENC_17_DT);
Encoder enc18(ENC_18_CLK, ENC_18_DT);
Encoder enc19(ENC_19_CLK, ENC_19_DT);
Encoder enc20(ENC_20_CLK, ENC_20_DT);
Encoder menuEncoder(MENU_ENCODER_DT, MENU_ENCODER_CLK);

// Configurable encoder to parameter mapping (defined in config.h)
// Array indices: 0=enc1, 1=enc2, ..., 10=enc11, 11=enc13, 12=enc14, ..., 18=enc20
const int encoderMapping[19] = {
  ENC_1_PARAM, ENC_2_PARAM, ENC_3_PARAM, ENC_4_PARAM, ENC_5_PARAM,      // 0-4: enc1-enc5
  ENC_6_PARAM, ENC_7_PARAM, ENC_8_PARAM, ENC_9_PARAM, ENC_10_PARAM,     // 5-9: enc6-enc10
  ENC_11_PARAM, ENC_13_PARAM, ENC_14_PARAM, ENC_15_PARAM, ENC_16_PARAM, // 10-14: enc11,enc13-enc16
  ENC_17_PARAM, ENC_18_PARAM, ENC_19_PARAM, ENC_20_PARAM                // 15-18: enc17-enc20
};

long encoderValues[19] = {0};
long lastEncoderValues[19] = {0};

// Braids preset selection
int currentPreset = 0;

// Braids parameters (removed Filter Mode, using fixed Moog lowpass)
float braidsParameters[NUM_PARAMETERS] = {
  30.0,   // 0: Shape (0-42)
  64.0,   // 1: Timbre (0-127)  
  32.0,   // 2: Color (0-127)
  0.0,    // 3: Coarse (transpose)
  20.0,   // 4: Amp Attack (0-127)
  60.0,   // 5: Amp Decay (0-127)
  100.0,  // 6: Amp Sustain (0-127)  
  40.0,   // 7: Amp Release (0-127)
  80.0,   // 8: Filter Cutoff (0-127)
  10.0,   // 9: Filter Resonance (0-127)
  50.0,   // 10: Filter Strength (0-127)
  10.0,   // 11: Filter Attack (0-127)
  30.0,   // 12: Filter Decay (0-127)
  80.0,   // 13: Filter Sustain (0-127)
  50.0,   // 14: Filter Release (0-127)
  80.0,   // 15: Volume (0-127)
  5.0,    // 16: LFO Rate (0.1-20 Hz)
  0.0,    // 17: LFO>Timbre (0-100%)
  0.0,    // 18: LFO>Color (0-100%)
  0.0,    // 19: LFO>Pitch (0-100%)
  0.0,    // 20: LFO>Filter (0-100%)
  0.0     // 21: LFO>Volume (0-100%)
};

// Braids synthesis objects (polyphonic)
AudioSynthBraids         braidsOsc[VOICES];
AudioEffectEnvelope      braidsEnvelope[VOICES]; 
AudioSynthWaveformDc     dcFilter[VOICES];       // DC source for filter envelope per voice
AudioEffectEnvelope      filtEnv[VOICES];        // Filter envelope per voice
AudioFilterLadder        braidsFilter[VOICES];   // Moog-style ladder filter
AudioMixer4              braidsMix1, braidsMix2; // First 4 voices, voices 4-5
AudioMixer4              braidsFinalMix;          // Final mono mix of all voices
AudioOutputUSB           usb1;

// Audio connections - Polyphonic Braids chain with filter envelopes
// Voice 0 connections
AudioConnection patchCord1_0(braidsOsc[0], 0, braidsEnvelope[0], 0);
AudioConnection patchCord2_0(braidsEnvelope[0], 0, braidsFilter[0], 0);
AudioConnection patchCord3_0(dcFilter[0], filtEnv[0]);
AudioConnection patchCord4_0(filtEnv[0], 0, braidsFilter[0], 1);
AudioConnection patchCord5_0(braidsFilter[0], 0, braidsMix1, 0);

// Voice 1 connections  
AudioConnection patchCord1_1(braidsOsc[1], 0, braidsEnvelope[1], 0);
AudioConnection patchCord2_1(braidsEnvelope[1], 0, braidsFilter[1], 0);
AudioConnection patchCord3_1(dcFilter[1], filtEnv[1]);
AudioConnection patchCord4_1(filtEnv[1], 0, braidsFilter[1], 1);
AudioConnection patchCord5_1(braidsFilter[1], 0, braidsMix1, 1);

// Voice 2 connections
AudioConnection patchCord1_2(braidsOsc[2], 0, braidsEnvelope[2], 0);
AudioConnection patchCord2_2(braidsEnvelope[2], 0, braidsFilter[2], 0);
AudioConnection patchCord3_2(dcFilter[2], filtEnv[2]);
AudioConnection patchCord4_2(filtEnv[2], 0, braidsFilter[2], 1);
AudioConnection patchCord5_2(braidsFilter[2], 0, braidsMix1, 2);

// Voice 3 connections
AudioConnection patchCord1_3(braidsOsc[3], 0, braidsEnvelope[3], 0);
AudioConnection patchCord2_3(braidsEnvelope[3], 0, braidsFilter[3], 0);
AudioConnection patchCord3_3(dcFilter[3], filtEnv[3]);
AudioConnection patchCord4_3(filtEnv[3], 0, braidsFilter[3], 1);
AudioConnection patchCord5_3(braidsFilter[3], 0, braidsMix1, 3);

// Voice 4 connections
AudioConnection patchCord1_4(braidsOsc[4], 0, braidsEnvelope[4], 0);
AudioConnection patchCord2_4(braidsEnvelope[4], 0, braidsFilter[4], 0);
AudioConnection patchCord3_4(dcFilter[4], filtEnv[4]);
AudioConnection patchCord4_4(filtEnv[4], 0, braidsFilter[4], 1);
AudioConnection patchCord5_4(braidsFilter[4], 0, braidsMix2, 0);

// Voice 5 connections
AudioConnection patchCord1_5(braidsOsc[5], 0, braidsEnvelope[5], 0);
AudioConnection patchCord2_5(braidsEnvelope[5], 0, braidsFilter[5], 0);
AudioConnection patchCord3_5(dcFilter[5], filtEnv[5]);
AudioConnection patchCord4_5(filtEnv[5], 0, braidsFilter[5], 1);
AudioConnection patchCord5_5(braidsFilter[5], 0, braidsMix2, 1);

// Mix the two sub-mixers into final mix, then send to both L and R channels
AudioConnection patchCord_mix1(braidsMix1, 0, braidsFinalMix, 0); // Voices 0-3
AudioConnection patchCord_mix2(braidsMix2, 0, braidsFinalMix, 1); // Voices 4-5

// Final mono-to-stereo output - same signal to both channels
AudioConnection patchCord_finalL(braidsFinalMix, 0, usb1, 0); // Left channel
AudioConnection patchCord_finalR(braidsFinalMix, 0, usb1, 1); // Right channel

// Control parameter names for menu display (removed Filter Mode)
const char* controlNames[NUM_PARAMETERS] = {
  "Shape", "Timbre", "Color", "Coarse", 
  "Amp Attack", "Amp Decay", "Amp Sustain", "Amp Release",
  "Filter Cutoff", "Filter Res", "Filter Strength",
  "Filt Attack", "Filt Decay", "Filt Sustain", "Filt Release",
  "Volume", "LFO Rate", "LFO>Timbre", "LFO>Color", "LFO>Pitch", "LFO>Filter", "LFO>Volume"
};

// Braids shape/algorithm names for display
const char* shapeNames[48] = {
  "CSAW", "MORPH", "SAW_SQR", "FOLD", "USAW", "SYNC", "RINGMOD", "FM",
  "SQR_SUB", "SAW_SUB", "SQR_SYN", "SAW_SYN", "TRIPLE", "SAW_SWM", "SAW_CYC", "SQR_CYC",
  "SIN_SWM", "SIN_CYC", "BUZZ", "SAW_DIP", "SYN_DIP", "TRIPLE2", "SAW_TRI", "SQR_TRI",
  "SAW_TRP", "SQR_TRP", "WTBL", "WMAP", "WLIN", "WTx4", "NOIS", "TWNQ",
  "CLKN", "CLOU", "PRTC", "QPSK", "VOWL", "VFIL", "VDHR", "HARM",
  "FM2", "WTFM", "PLUK", "BOWD", "BLOW", "FLUT", "BELL", "DRUM"
};

bool macroMode = false;

MenuState currentMenuState = PARENT_MENU;
bool inMenu = false;
bool inPresetBrowse = false;
int menuIndex = 0;
int presetBrowseIndex = 0;

// Voice allocation
struct Voice {
  bool active;
  uint8_t note;
  uint8_t velocity;
  unsigned long noteOnTime;
};

Voice voices[VOICES];
int currentVoice = 0; // Round-robin voice allocation counter

// Filter envelope parameters (matching Mini-Teensy)
float filtAttack = 100, filtSustain = 0.5, filtDecay = 2500; // Filter envelope timing
float filterStrength = 0.5; // DC amplitude for filter envelope

// Display objects
#ifdef USE_LCD_DISPLAY
  LiquidCrystal_I2C lcd(0x27, 16, 2);
#endif

#ifdef USE_OLED_DISPLAY
  U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);
#endif

// MIDI variables
float pitchWheelValue = 0.0;
float modWheelValue = 0.0;
int midiChannel = 0; // 0 = omni, 1-16 = specific channel


// LFO variables (Juno-style with separate depth controls)
AudioSynthWaveformSine   lfo;             // LFO for modulation
float lfoRate = 5.0; // LFO rate in Hz (0.1 - 20Hz)
float lfoTimbreDepth = 0.0; // LFO>Timbre depth (0-1)
float lfoColorDepth = 0.0;  // LFO>Color depth (0-1)
float lfoPitchDepth = 0.0;  // LFO>Pitch depth (0-1)
float lfoFilterDepth = 0.0; // LFO>Filter depth (0-1)
float lfoVolumeDepth = 0.0; // LFO>Volume depth (0-1)

#ifdef ENABLE_DIN_MIDI
void OnNoteOn(byte channel, byte note, byte velocity) {
  noteOn(note, velocity);
}

void OnNoteOff(byte channel, byte note, byte velocity) {
  noteOff(note);
}

void OnControlChange(byte channel, byte number, byte value) {
  if (number == 1) { // Mod wheel
    modWheelValue = value / 127.0;
    // Apply mod wheel to timbre parameter
    float modAmount = modWheelValue * 32.0f; // Mod depth for 0-127 range
    updateBraidsParameter(1, constrain(braidsParameters[1] + modAmount, 0.0f, 127.0f));
  }
}

void OnPitchBend(byte channel, int bend) {
  pitchWheelValue = (bend - 8192) / 8192.0;
  // Apply pitch bend to all active voices
  for (int v = 0; v < VOICES; v++) {
    if (voices[v].active) {
      int transposedNote = voices[v].note + (int)braidsParameters[3]; // Apply coarse transpose
      float pitchBend = pitchWheelValue * 2.0; // ±2 semitones
      transposedNote += (int)pitchBend;
      int pitch = transposedNote << 7; // Simple left shift by 7 bits
      braidsOsc[v].set_braids_pitch(pitch);
    }
  }
}
#endif

#ifdef USE_MIDI_HOST
// USB Host MIDI callback handlers
void OnUSBHostNoteOn(byte channel, byte note, byte velocity) {
  noteOn(note, velocity);
}
void OnUSBHostNoteOff(byte channel, byte note, byte velocity) {
  noteOff(note);
}
void OnUSBHostControlChange(byte channel, byte number, byte value) {
  if (number == 1) { // Mod wheel
    modWheelValue = value / 127.0;
    // Apply mod wheel to timbre parameter
    float modAmount = modWheelValue * 32.0f; // Mod depth for 0-127 range
    updateBraidsParameter(1, constrain(braidsParameters[1] + modAmount, 0.0f, 127.0f));
  }
}
void OnUSBHostPitchBend(byte channel, int bend) {
  // USB Host MIDI uses signed range -8192 to +8191, center = 0
  pitchWheelValue = bend / 8192.0;
  // Apply pitch bend to all active voices
  for (int v = 0; v < VOICES; v++) {
    if (voices[v].active) {
      int transposedNote = voices[v].note + (int)braidsParameters[3]; // Apply coarse transpose
      float pitchBend = pitchWheelValue * 2.0; // ±2 semitones
      transposedNote += (int)pitchBend;
      int pitch = transposedNote << 7; // Simple left shift by 7 bits
      braidsOsc[v].set_braids_pitch(pitch);
    }
  }
}
#endif

void setup() {
  Serial.begin(9600);
  AudioMemory(60); // Braids needs more memory due to wavetables

  // Initialize Braids oscillators
  for (int v = 0; v < VOICES; v++) {
    braidsOsc[v].init_braids();
    braidsOsc[v].set_braids_shape(braidsParameters[0]); // Default shape
    braidsOsc[v].set_braids_timbre(braidsParameters[1] * 512); // 0-65535 range
    braidsOsc[v].set_braids_color(braidsParameters[2] * 512);  // 0-65535 range
    
    // Initialize amplitude envelopes
    braidsEnvelope[v].attack(braidsParameters[4]);
    braidsEnvelope[v].decay(braidsParameters[5]);
    braidsEnvelope[v].sustain(braidsParameters[6] / 127.0);
    braidsEnvelope[v].release(braidsParameters[7]);
    
    // Initialize filter envelope system (matching Mini-Teensy)
    dcFilter[v].amplitude(filterStrength);
    filtEnv[v].attack(filtAttack);
    filtEnv[v].sustain(filtSustain);
    filtEnv[v].decay(filtDecay);
    filtEnv[v].release(filtDecay);
    
    // Configure Moog ladder filter (matching Mini-Teensy)
    float val = braidsParameters[8] / 127.0;
    float cutoff = 20 * pow(1000.0, val); // Logarithmic like Mini-Teensy
    braidsFilter[v].frequency(cutoff);
    braidsFilter[v].resonance((braidsParameters[9] / 127.0) * 3.0); // 0-3.0 like Mini-Teensy
    braidsFilter[v].octaveControl(3.0);
  }
  
  // Initialize LFO (matching Mini-Teensy)
  lfo.frequency(lfoRate);
  lfo.amplitude(1.0);
  
  // Set up mixer gains
  braidsMix1.gain(0, 0.25); braidsMix1.gain(1, 0.25); braidsMix1.gain(2, 0.25); braidsMix1.gain(3, 0.25);
  braidsMix2.gain(0, 0.25); braidsMix2.gain(1, 0.25); braidsMix2.gain(2, 0.0); braidsMix2.gain(3, 0.0);
  braidsFinalMix.gain(0, braidsParameters[15] / 127.0);
  braidsFinalMix.gain(1, braidsParameters[15] / 127.0);

  // USB Device MIDI is initialized automatically
#ifdef USE_USB_DEVICE_MIDI
  Serial.println("USB Device MIDI enabled");
#endif

  // USB Host MIDI support disabled - would need additional MIDI functions
  // TODO: Implement USB Host MIDI support if needed
  
#ifdef ENABLE_DIN_MIDI
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(OnNoteOn);
  MIDI.setHandleNoteOff(OnNoteOff);
  MIDI.setHandleControlChange(OnControlChange);
  MIDI.setHandlePitchBend(OnPitchBend);
#endif
  
  pinMode(MENU_ENCODER_SW, INPUT_PULLUP);
  
  // Initialize encoder values
  for (int i = 0; i < 19; i++) {
    encoderValues[i] = 0;
    lastEncoderValues[i] = 0;
  }
  
  // Initialize voice allocation
  for (int v = 0; v < VOICES; v++) {
    voices[v].active = false;
    voices[v].note = 0;
    voices[v].velocity = 0;
    voices[v].noteOnTime = 0;
    
    // Configure DC source for filter envelope (like Mini-Teensy)
    dcFilter[v].amplitude(filterStrength);
    
    // Configure filter envelopes (like Mini-Teensy)
    filtEnv[v].attack(filtAttack);
    filtEnv[v].sustain(filtSustain);
    filtEnv[v].decay(filtDecay);
    filtEnv[v].release(filtDecay);
  }
  
  // Initialize Braids parameters
  for (int i = 0; i < NUM_PARAMETERS; i++) {
    updateBraidsParameter(i, braidsParameters[i]);
  }
  
  // Set mixer gains for 6 voices
  braidsMix1.gain(0, 0.25); // Voice 0
  braidsMix1.gain(1, 0.25); // Voice 1  
  braidsMix1.gain(2, 0.25); // Voice 2
  braidsMix1.gain(3, 0.25); // Voice 3
  braidsMix2.gain(0, 0.25); // Voice 4
  braidsMix2.gain(1, 0.25); // Voice 5
  braidsMix2.gain(2, 0.0);  // Unused
  braidsMix2.gain(3, 0.0);  // Unused
  
  // Final mixer gains - mix both sub-mixers
  braidsFinalMix.gain(0, 0.5); // braidsMix1 (voices 0-3)
  braidsFinalMix.gain(1, 0.5); // braidsMix2 (voices 4-5)
  braidsFinalMix.gain(2, 0.0); // Unused
  braidsFinalMix.gain(3, 0.0); // Unused
  
  Serial.println("Loading default Braids preset...");
  loadPreset(0);
  
  Serial.println("Braids initialization complete");
  
  delay(100);
  
#ifdef USE_LCD_DISPLAY
  lcd.init();
  lcd.backlight();
  displayText(PROJECT_NAME, PROJECT_SUBTITLE);
#endif

#ifdef USE_OLED_DISPLAY
  Wire.begin();
  delay(50);
  display.begin();
  Serial.println("OLED initialized");
  
  display.clearBuffer();
  display.setFont(u8g2_font_8x13_tf);
  display.drawStr(3, 20, PROJECT_NAME);
  display.drawStr(3, 40, PROJECT_SUBTITLE);
  display.sendBuffer();
#endif
  
  delay(2000);
  updateDisplay();
  Serial.print(PROJECT_NAME);
  Serial.println(" Ready!");
}

// Read all 19 Mini-Teensy encoders
void readDirectEncoders() {
  encoderValues[0] = enc1.read() / 4;   // enc1
  encoderValues[1] = enc2.read() / 4;   // enc2
  encoderValues[2] = enc3.read() / 4;   // enc3
  encoderValues[3] = enc4.read() / 4;   // enc4
  encoderValues[4] = enc5.read() / 4;   // enc5
  encoderValues[5] = enc6.read() / 4;   // enc6
  encoderValues[6] = enc7.read() / 4;   // enc7
  encoderValues[7] = enc8.read() / 4;   // enc8
  encoderValues[8] = enc9.read() / 4;   // enc9
  encoderValues[9] = enc10.read() / 4;  // enc10
  encoderValues[10] = enc11.read() / 4; // enc11
  encoderValues[11] = enc13.read() / 4; // enc13 (note: no enc12)
  encoderValues[12] = enc14.read() / 4; // enc14
  encoderValues[13] = enc15.read() / 4; // enc15
  encoderValues[14] = enc16.read() / 4; // enc16
  encoderValues[15] = enc17.read() / 4; // enc17
  encoderValues[16] = enc18.read() / 4; // enc18
  encoderValues[17] = enc19.read() / 4; // enc19
  encoderValues[18] = enc20.read() / 4; // enc20
}

void readAllControls() {
  readDirectEncoders();
  
  // Check for encoder changes and update parameters using configurable mapping
  for (int i = 0; i < 19; i++) {
    if (encoderValues[i] != lastEncoderValues[i]) {
      // If any physical knob is turned, exit menu mode
      if (inMenu) {
        inMenu = false;
      }
      
      int change = encoderValues[i] - lastEncoderValues[i];
      int paramIndex = encoderMapping[i]; // Use configurable mapping
      
      // Only update if encoder is mapped to a valid parameter (not -1 for disabled)
      if (paramIndex >= 0 && paramIndex < NUM_PARAMETERS) {
        updateEncoderParameter(paramIndex, change);
      }
      
      lastEncoderValues[i] = encoderValues[i];
    }
  }
  
  // Handle menu encoder parameter control (if configured) - ONLY when not in menu
  // This is handled separately from the 19 main encoders and follows EPiano pattern
  static long lastMenuEncoderValue = 0;
  long currentMenuEncoderValue = 0;
  
#ifdef USE_OLED_DISPLAY
  currentMenuEncoderValue = menuEncoder.read() / 4; // Adjusted sensitivity for OLED encoder
#else
  currentMenuEncoderValue = menuEncoder.read() / 2; // Standard sensitivity for separate encoder
#endif
  
  // ONLY handle menu encoder as parameter control when NOT in menu AND encoder is assigned to a parameter
  if (!inMenu && MENU_ENCODER_PARAM >= 0 && MENU_ENCODER_PARAM < NUM_PARAMETERS && currentMenuEncoderValue != lastMenuEncoderValue) {
    int change = currentMenuEncoderValue - lastMenuEncoderValue;
    updateEncoderParameter(MENU_ENCODER_PARAM, change);
    lastMenuEncoderValue = currentMenuEncoderValue;
  }
}

void updateBraidsParameter(int paramIndex, float value) {
  // Store the value in the braids parameter array
  braidsParameters[paramIndex] = value;
  
  // Update Braids synthesis parameters for all voices
  for (int v = 0; v < VOICES; v++) {
    switch (paramIndex) {
      case 0: // Shape (0-42)
        braidsOsc[v].set_braids_shape((int16_t)value);
        break;
      case 1: // Timbre (0-127)
        braidsOsc[v].set_braids_timbre((int16_t)(value * 258)); // Scale to 16-bit
        break;
      case 2: // Color (0-127)
        braidsOsc[v].set_braids_color((int16_t)(value * 258)); // Scale to 16-bit
        break;
      case 3: // Coarse (transpose) - handled in noteOn
        break;
      case 4: // Amp Attack (0-127)
        braidsEnvelope[v].attack((value / 127.0) * 4000.0); // 0-4 seconds
        break;
      case 5: // Amp Decay (0-127)
        braidsEnvelope[v].decay((value / 127.0) * 4000.0); // 0-4 seconds
        break;
      case 6: // Amp Sustain (0-127)
        braidsEnvelope[v].sustain(value / 127.0); // 0.0-1.0
        break;
      case 7: // Amp Release (0-127)
        braidsEnvelope[v].release((value / 127.0) * 4000.0); // 0-4 seconds
        break;
      case 8: // Filter Cutoff (0-127) - moved from index 9
        {
          // Logarithmic frequency response like analog synth (20Hz to 20kHz) - matching Mini-Teensy
          float val = value / 127.0; // Convert to 0.0-1.0 range
          float freq = 20 * pow(1000.0, val); // 20Hz to 20kHz logarithmic
          braidsFilter[v].frequency(freq);  // Moog ladder filter
        }
        break;
      case 9: // Filter Resonance (0-127) - moved from index 10
        {
          float res = (value / 127.0) * 3.0; // 0 to 3.0 resonance like Mini-Teensy
          braidsFilter[v].resonance(res);  // Moog ladder filter
        }
        break;
      case 10: // Filter Strength (0-127) - moved from index 11
        filterStrength = value / 127.0; // 0.0 to 1.0 range
        for (int v = 0; v < VOICES; v++) {
          dcFilter[v].amplitude(filterStrength);
        }
        break;
      case 11: // Filter Attack (0-127) - moved from index 12
        filtAttack = 1 + (value / 127.0) * 3000; // 1ms to 3000ms like Mini-Teensy
        for (int v = 0; v < VOICES; v++) {
          filtEnv[v].attack(filtAttack);
        }
        break;
      case 12: // Filter Decay (0-127) - moved from index 13
        filtDecay = 10 + (value / 127.0) * 5000; // 10ms to 5000ms like Mini-Teensy
        for (int v = 0; v < VOICES; v++) {
          filtEnv[v].decay(filtDecay);
          filtEnv[v].release(filtDecay); // Use same value for release
        }
        break;
      case 13: // Filter Sustain (0-127) - moved from index 14
        filtSustain = value / 127.0; // 0.0 to 1.0 range like Mini-Teensy
        for (int v = 0; v < VOICES; v++) {
          filtEnv[v].sustain(filtSustain);
        }
        break;
      case 14: // Filter Release (0-127) - moved from index 15
        // In Mini-Teensy, release uses filtDecay value, so update both
        filtDecay = 10 + (value / 127.0) * 5000; // 10ms to 5000ms like Mini-Teensy
        for (int v = 0; v < VOICES; v++) {
          filtEnv[v].decay(filtDecay);
          filtEnv[v].release(filtDecay);
        }
        break;
      case 15: // Volume (0-127) - moved from index 16
        {
          // Apply volume to final mixer output
          float volume = value / 127.0;
          braidsFinalMix.gain(0, 0.5 * volume); // braidsMix1 (voices 0-3)
          braidsFinalMix.gain(1, 0.5 * volume); // braidsMix2 (voices 4-5)
        }
        break;
      case 16: // LFO Rate (0.1-20 Hz)
        lfoRate = 0.1 + (value / 127.0) * 19.9; // 0.1 to 20 Hz like Mini-Teensy
        lfo.frequency(lfoRate);
        break;
      case 17: // LFO>Timbre (0-100%)
        lfoTimbreDepth = value / 127.0; // 0.0 to 1.0 range like Juno-style
        break;
      case 18: // LFO>Color (0-100%)
        lfoColorDepth = value / 127.0; // 0.0 to 1.0 range like Juno-style
        break;
      case 19: // LFO>Pitch (0-100%)
        lfoPitchDepth = value / 127.0; // 0.0 to 1.0 range like Juno-style
        break;
      case 20: // LFO>Filter (0-100%)
        lfoFilterDepth = value / 127.0; // 0.0 to 1.0 range like Juno-style
        break;
      case 21: // LFO>Volume (0-100%)
        lfoVolumeDepth = value / 127.0; // 0.0 to 1.0 range like Juno-style
        break;
    }
  }
}

// Voice allocation - round-robin like Mini-Teensy-Synth
int findAvailableVoice() {
  // Start from current voice and look for next available
  for (int i = 0; i < VOICES; i++) {
    int v = (currentVoice + i) % VOICES;
    if (!voices[v].active) {
      currentVoice = (v + 1) % VOICES; // Set next voice for next time
      return v;
    }
  }
  
  // If all voices are active, use round-robin for voice stealing
  int voiceToSteal = currentVoice;
  currentVoice = (currentVoice + 1) % VOICES; // Advance for next time
  return voiceToSteal;
}

void noteOn(uint8_t note, uint8_t velocity) {
  int voice = findAvailableVoice();
  
  voices[voice].active = true;
  voices[voice].note = note;
  voices[voice].velocity = velocity;
  voices[voice].noteOnTime = millis();
  
  
  // Apply coarse transpose and pitch bend
  int transposedNote = note + (int)braidsParameters[3]; // Apply coarse transpose
  float pitchBend = pitchWheelValue * 2.0; // ±2 semitones
  transposedNote += (int)pitchBend;
  int pitch = transposedNote << 7; // Simple left shift by 7 bits
  braidsOsc[voice].set_braids_pitch(pitch);
  
  
  // Trigger envelopes
  braidsEnvelope[voice].noteOn();
  filtEnv[voice].noteOn(); // Trigger filter envelope
}

// Find voice playing a specific note
int findVoiceForNote(uint8_t note) {
  for (int v = 0; v < VOICES; v++) {
    if (voices[v].active && voices[v].note == note) {
      return v;
    }
  }
  return -1; // Note not found
}

void noteOff(uint8_t note) {
  // Find voice with this note and release it
  int voiceNum = findVoiceForNote(note);
  if (voiceNum >= 0) {
    // Turn off envelopes but let them complete their release naturally (like Mini-Teensy)
    braidsEnvelope[voiceNum].noteOff();
    filtEnv[voiceNum].noteOff(); // Trigger filter envelope release
    voices[voiceNum].active = false; // Mark as inactive for voice allocation
  }
}

// Load Braids preset (exact copy from working Multi-Teensy-Synth-copy)
void loadPreset(int presetNum) {
  presetNum = constrain(presetNum, 0, NUM_PRESETS - 1);
  currentPreset = presetNum;
  
  const BraidsPreset& preset = braidsPresets[presetNum];
  
  // Update parameter array to match preset values
  braidsParameters[0] = preset.shape;
  braidsParameters[1] = preset.timbre;
  braidsParameters[2] = preset.color;
  braidsParameters[3] = preset.coarse;
  braidsParameters[4] = preset.attack;
  braidsParameters[5] = preset.decay;
  braidsParameters[6] = preset.sustain;
  braidsParameters[7] = preset.release;
  braidsParameters[8] = (preset.filterFreq - 50) * 127.0 / (8000 - 50); // Filter Cutoff - moved to index 8
  braidsParameters[9] = preset.filterRes; // Filter Resonance - moved to index 9
  braidsParameters[10] = 50.0; // Filter strength (default)
  braidsParameters[11] = 10.0; // Filter attack (default)
  braidsParameters[12] = 30.0; // Filter decay (default)
  braidsParameters[13] = 80.0; // Filter sustain (default)
  braidsParameters[14] = 50.0; // Filter release (default)
  braidsParameters[15] = 80.0; // Volume (default)
  
  // Apply all parameter values to synthesis engine
  for (int i = 0; i < NUM_PARAMETERS; i++) {
    updateBraidsParameter(i, braidsParameters[i]);
  }
  
  Serial.print("Loaded Braids preset ");
  Serial.print(presetNum + 1);
  Serial.print(": ");
  Serial.println(preset.name);
}

// Update encoder parameter with proper scaling (matches EPiano pattern)
void updateEncoderParameter(int paramIndex, int change) {
  if (paramIndex < 0 || paramIndex >= NUM_PARAMETERS) return;
  
  float increment = 1.0; // Direct increment for raw Braids values (not normalized)
  
  // Special handling for specific parameters
  if (paramIndex == 0) { // Shape - limit to 0-42 range
    braidsParameters[paramIndex] = constrain(braidsParameters[paramIndex] + (change * increment), 0.0, 42.0);
  } else if (paramIndex == 8) { // Filter Cutoff - standard 0-127 range
    braidsParameters[paramIndex] = constrain(braidsParameters[paramIndex] + (change * increment), 0.0, 127.0);
  } else {
    // Standard 0-127 parameter range
    braidsParameters[paramIndex] = constrain(braidsParameters[paramIndex] + (change * increment), 0.0, 127.0);
  }
  
  updateBraidsParameter(paramIndex, braidsParameters[paramIndex]);
  
  // Display parameter change when not in menu
  if (!inMenu) {
    String line1 = controlNames[paramIndex];
    String line2;
    
    if (paramIndex == 0) { // Shape parameter - show algorithm name
      int shapeIndex = (int)braidsParameters[paramIndex];
      line2 = String(shapeNames[shapeIndex]) + " (" + String(shapeIndex) + ")";
    } else if (paramIndex == 3) { // Coarse transpose
      int transpose = (int)braidsParameters[paramIndex];
      line2 = String(transpose) + "st";
    } else if (paramIndex == 8) { // Filter Cutoff - show 0-127 range like Mini-Teensy
      int displayValue = (int)braidsParameters[paramIndex];
      line2 = String(displayValue);
    } else if (paramIndex == 9) { // Filter Resonance - show 0-127 range like Mini-Teensy
      int displayValue = (int)braidsParameters[paramIndex];
      line2 = String(displayValue);
    } else {
      // Standard 0-127 parameter display
      int displayValue = (int)braidsParameters[paramIndex];
      line2 = String(displayValue);
    }
    
    displayText(line1, line2);
  }
}

// LFO modulation update (Juno-style with simultaneous modulation)
void updateLFOModulation() {
  // Throttle LFO updates to reduce CPU load and avoid conflicts with knob updates
  static unsigned long lastLFOUpdate = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastLFOUpdate < 10) return; // Slower updates to avoid glitches (10ms = 100Hz)
  lastLFOUpdate = currentTime;
  
  // Apply pitch wheel to all active voices first
  float pitchWheelMultiplier = pow(2.0, pitchWheelValue * 2.0 / 12.0);
  
  // Calculate LFO signal
  float phase = (currentTime * lfoRate * 2 * PI) / 1000.0;
  float lfoSignal = sin(phase);
  
  // Apply modulation to each target if depth > 0.01
  bool anyLfoActive = false;
  
  // Timbre modulation (Braids-specific) - need to scale to 16-bit like normal updates
  if (lfoTimbreDepth > 0.01) {
    anyLfoActive = true;
    float timbreModulation = lfoSignal * lfoTimbreDepth * 20.0; // ±20 timbre units
    float baseTimbre = braidsParameters[1];
    float modTimbre = constrain(baseTimbre + timbreModulation, 0.0, 127.0);
    for (int v = 0; v < VOICES; v++) {
      if (voices[v].active) {
        braidsOsc[v].set_braids_timbre((int16_t)(modTimbre * 258)); // Scale to 16-bit like normal updates
      }
    }
  } else {
    // When LFO is not active, ensure base value is set correctly
    for (int v = 0; v < VOICES; v++) {
      if (voices[v].active) {
        braidsOsc[v].set_braids_timbre((int16_t)(braidsParameters[1] * 258));
      }
    }
  }
  
  // Color modulation (Braids-specific) - need to scale to 16-bit like normal updates
  if (lfoColorDepth > 0.01) {
    anyLfoActive = true;
    float colorModulation = lfoSignal * lfoColorDepth * 20.0; // ±20 color units
    float baseColor = braidsParameters[2];
    float modColor = constrain(baseColor + colorModulation, 0.0, 127.0);
    for (int v = 0; v < VOICES; v++) {
      if (voices[v].active) {
        braidsOsc[v].set_braids_color((int16_t)(modColor * 258)); // Scale to 16-bit like normal updates
      }
    }
  } else {
    // When LFO is not active, ensure base value is set correctly
    for (int v = 0; v < VOICES; v++) {
      if (voices[v].active) {
        braidsOsc[v].set_braids_color((int16_t)(braidsParameters[2] * 258));
      }
    }
  }
  
  // Pitch modulation
  if (lfoPitchDepth > 0.01) {
    anyLfoActive = true;
    float pitchModulation = lfoSignal * lfoPitchDepth * 0.02; // ±2% for Braids pitch format
    float pitchLFOMultiplier = 1.0 + pitchModulation;
    for (int v = 0; v < VOICES; v++) {
      if (voices[v].active) {
        float basePitch = (voices[v].note + (int)braidsParameters[3]) << 7; // Include coarse transpose
        float modPitch = basePitch * pitchLFOMultiplier * pitchWheelMultiplier;
        braidsOsc[v].set_braids_pitch((int)modPitch);
      }
    }
  } else {
    // No pitch LFO - apply only pitch wheel
    for (int v = 0; v < VOICES; v++) {
      if (voices[v].active) {
        float basePitch = (voices[v].note + (int)braidsParameters[3]) << 7; // Include coarse transpose
        float modPitch = basePitch * pitchWheelMultiplier;
        braidsOsc[v].set_braids_pitch((int)modPitch);
      }
    }
  }
  
  // Filter modulation
  float val = braidsParameters[8] / 127.0;
  float baseCutoff = 20 * pow(1000.0, val); // Base filter cutoff
  if (lfoFilterDepth > 0.01) {
    anyLfoActive = true;
    float filterModulation = lfoSignal * lfoFilterDepth * 1000.0; // ±1000 Hz like Juno
    float modulatedCutoff = constrain(baseCutoff + filterModulation, 20.0, 20000.0);
    for (int v = 0; v < VOICES; v++) {
      braidsFilter[v].frequency(modulatedCutoff);
    }
  } else {
    // No filter LFO - use base cutoff
    for (int v = 0; v < VOICES; v++) {
      braidsFilter[v].frequency(baseCutoff);
    }
  }
  
  // Volume modulation
  float baseVolume = braidsParameters[15] / 127.0;
  if (lfoVolumeDepth > 0.01) {
    anyLfoActive = true;
    float volumeModulation = lfoSignal * lfoVolumeDepth * 0.3; // ±30% amplitude like Juno
    float ampMultiplier = 1.0 + volumeModulation;
    float modVolume = constrain(baseVolume * ampMultiplier, 0.0, 1.0);
    braidsFinalMix.gain(0, 0.5 * modVolume); // braidsMix1 (voices 0-3)
    braidsFinalMix.gain(1, 0.5 * modVolume); // braidsMix2 (voices 4-5)
  } else {
    // No volume LFO - use base volume
    braidsFinalMix.gain(0, 0.5 * baseVolume); // braidsMix1 (voices 0-3)
    braidsFinalMix.gain(1, 0.5 * baseVolume); // braidsMix2 (voices 4-5)
  }
}





void loop() {
  // Process USB Device MIDI messages (if enabled)
#ifdef USE_USB_DEVICE_MIDI
  while (usbMIDI.read()) {
    uint8_t type = usbMIDI.getType();
    uint8_t channel = usbMIDI.getChannel();
    uint8_t data1 = usbMIDI.getData1();
    uint8_t data2 = usbMIDI.getData2();
    
    // Filter by MIDI channel (0 = omni mode, receive all channels)
    if (midiChannel != 0 && channel != midiChannel) {
      continue; // Skip messages not on our channel
    }
    
    if (type == usbMIDI.NoteOn && data2 > 0) {
      noteOn(data1, data2);
    } else if (type == usbMIDI.NoteOff || (type == usbMIDI.NoteOn && data2 == 0)) {
      noteOff(data1);
    } else if (type == usbMIDI.ControlChange) {
      // Handle MIDI Control Change messages
      if (data1 == 1) { // Mod wheel (CC#1)
        modWheelValue = data2 / 127.0;
        // Apply mod wheel to timbre parameter
        float modAmount = modWheelValue * 32.0f; // Mod depth for 0-127 range
        updateBraidsParameter(1, constrain(braidsParameters[1] + modAmount, 0.0f, 127.0f));
      }
    } else if (type == usbMIDI.PitchBend) {
      // Handle MIDI Pitch Bend messages
      int16_t bendValue = (data1) | (data2 << 7); // Reconstruct 14-bit value
      pitchWheelValue = (bendValue - 8192) / 8192.0;
      // Apply pitch bend to all active voices
      for (int v = 0; v < VOICES; v++) {
        if (voices[v].active) {
          float pitchBend = pitchWheelValue * 2.0; // ±2 semitones
          int transposedNote = voices[v].note + (int)pitchBend;
          int pitch = transposedNote << 7; // Simple left shift by 7 bits
          braidsOsc[v].set_braids_pitch(pitch);
        }
      }
    } else if (type == usbMIDI.ProgramChange) {
      // Map program change 0-127 to Braids presets 0-7
      int presetNum = data1 % NUM_PRESETS;
      loadPreset(presetNum);
    }
  }
#endif

#ifdef USE_MIDI_HOST
  // Additionally process USB Host MIDI messages (if enabled)
  myusb.Task();
  midi1.read();
#endif
  
#ifdef ENABLE_DIN_MIDI
  MIDI.read();
#endif
  
  readAllControls();
  handleEncoder();
  
  // Update LFO modulation (matching Mini-Teensy)
  updateLFOModulation();
  
  
  // Minimal serial input check for performance
  if (Serial.available()) {
    char input = Serial.read();
    if (input == 'r' || input == 'R') {
      resetEncoderBaselines();
    }
  }
  delay(5);
}