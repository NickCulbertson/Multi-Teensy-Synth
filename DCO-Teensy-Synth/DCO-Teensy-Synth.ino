/*
 * DCO-Teensy Synth v1.0
 * A 6-voice polyphonic virtual analog synthesizer built with the Teensy 4.1 microcontroller, 
 * inspired by the classic Roland Juno-60. Features analog-style synthesis with chorus,
 * high-pass filter, and classic Juno sound characteristics.
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

#define NUM_PARAMETERS 31
#define NUM_PRESETS 20
#define VOICES 6

#include "config.h"
#include "MenuNavigation.h"

const char* PROJECT_NAME = "DCO-Teensy Synth";
const char* PROJECT_SUBTITLE = "6-Voice Poly";

#include <USBHost_t36.h>
#include <Audio.h>
#include <Wire.h>
#include <Encoder.h>
#include "AudioEffectCustomChorus.h"

#ifdef USE_LCD_DISPLAY
  #include <LiquidCrystal_I2C.h>
#endif

#ifdef USE_OLED_DISPLAY
  #include <U8g2lib.h>
  #define OLED_WIDTH 128
  #define OLED_HEIGHT 32
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
// Array indices: 0=enc1, 1=enc2, ..., 10=enc11, 11=menuEncoder, 12=enc13, 13=enc14, ..., 19=enc20
const int encoderMapping[20] = {
  ENC_1_PARAM, ENC_2_PARAM, ENC_3_PARAM, ENC_4_PARAM, ENC_5_PARAM,              // 0-4: enc1-enc5
  ENC_6_PARAM, ENC_7_PARAM, ENC_8_PARAM, ENC_9_PARAM, ENC_10_PARAM,             // 5-9: enc6-enc10
  ENC_11_PARAM, MENU_ENCODER_PARAM, ENC_13_PARAM, ENC_14_PARAM, ENC_15_PARAM,   // 10-14: enc11,menuEncoder,enc13-enc15
  ENC_16_PARAM, ENC_17_PARAM, ENC_18_PARAM, ENC_19_PARAM, ENC_20_PARAM          // 15-19: enc16-enc20
};

long encoderValues[20] = {0};
long lastEncoderValues[20] = {0};
// Default parameter values - Juno-60 "Init" preset
float allParameterValues[NUM_PARAMETERS] = {
  0.5,   // 0: PWM Volume
  0.5,   // 1: PWM Width 
  0.8,   // 2: Saw Volume
  0.0,   // 3: Sub Volume
  0.0,   // 4: Noise Volume
  0.05,  // 5: LFO Rate (0.5 Hz)
  0.0,   // 6: LFO Delay
  0.0,   // 7: LFO to PWM
  0.0,   // 8: LFO to Pitch 
  0.0,   // 9: LFO to Filter
  0.01,  // 10: HPF Cutoff (20 Hz)
  0.75,  // 11: LPF Cutoff (6000 Hz)
  0.0,   // 12: Resonance
  0.5,   // 13: Filter Env Amount
  0.0,   // 14: Filter Attack
  0.04,  // 15: Filter Decay (200ms)
  0.5,   // 16: Filter Sustain
  0.04,  // 17: Filter Release (200ms)
  0.0,   // 18: Amp Attack
  0.02,  // 19: Amp Decay (100ms)
  0.8,   // 20: Amp Sustain
  0.02,  // 21: Amp Release (100ms)
  0.0,   // 22: Chorus Mode (0=off, 1=I, 2=II, 3=I+II)
  0.0,   // 23: Reserved
  0.0,   // 24: Reserved
  0.5,   // 25: Play Mode (Poly)
  0.0,   // 26: Glide Time (off)
  0.0,   // 27: Reserved
  0.0,   // 28: Reserved
  0.0,   // 29: Reserved
  0.0    // 30: MIDI Channel (omni)
};

// Audio synthesis - Juno-60 authentic architecture
AudioSynthWaveform       pwmOsc[VOICES];   // PWM oscillator per voice
AudioSynthWaveform       sawOsc[VOICES];   // Sawtooth oscillator per voice
AudioSynthWaveform       subOsc[VOICES];   // Sub-oscillator (1 octave down, square wave)
AudioSynthNoiseWhite     noise1;          // White noise source
AudioSynthWaveformDc     dcFilter[VOICES]; // DC source for filter envelope per voice
AudioSynthWaveformSine   lfo;             // LFO for modulation
AudioMixer4              oscMix[VOICES];   // Mix VCO + Sub + noise per voice
AudioFilterLadder        lpFilter[VOICES]; // Low-pass filter per voice (24dB/oct)
AudioFilterBiquad        hpFilter[VOICES]; // High-pass filter per voice (Juno signature feature)
AudioEffectEnvelope      ampEnv[VOICES], filtEnv[VOICES]; // Envelopes per voice
AudioMixer4              voiceMix1, voiceMix2; // Mix voices together
AudioMixer4              preChorusMix;    // Pre-chorus voice mixer (mono)
AudioEffectCustomChorus  chorusL, chorusR; // Authentic Juno-60 stereo BBD chorus
AudioMixer4              finalMixL, finalMixR; // Stereo final mix (dry + chorus)
AudioOutputUSB           usb1;

// Authentic Juno-60 BBD chorus delay line buffers (10ms max at 44.1kHz = ~441 samples)
short chorusDelayLineL[500]; // Left channel BBD delay line
short chorusDelayLineR[500]; // Right channel BBD delay line

// Audio connections - Juno-60 authentic polyphonic chain
// Note: Juno-60 uses PWM + Saw on main VCO, we'll route both to mixer

// Voice 0 connections
AudioConnection patchCord1_0(pwmOsc[0], 0, oscMix[0], 0);      // PWM oscillator to mixer ch 0
AudioConnection patchCord1_0b(sawOsc[0], 0, oscMix[0], 3);      // Sawtooth oscillator to mixer ch 3
AudioConnection patchCord2_0(subOsc[0], 0, oscMix[0], 1);       // Sub-osc to mixer ch 1
AudioConnection patchCord3_0(noise1, 0, oscMix[0], 2);          // Noise to mixer ch 2
AudioConnection patchCord4_0(oscMix[0], 0, ampEnv[0], 0);       // Mixer to amp envelope
AudioConnection patchCord5_0(ampEnv[0], 0, lpFilter[0], 0);     // Amp env to low-pass filter
AudioConnection patchCord6_0(dcFilter[0], filtEnv[0]);          // DC to filter envelope
AudioConnection patchCord7_0(filtEnv[0], 0, lpFilter[0], 1);    // Filter env to LP filter freq
AudioConnection patchCord8_0(lpFilter[0], 0, hpFilter[0], 0);   // LP filter to HP filter

// Voice 1 connections
AudioConnection patchCord1_1(pwmOsc[1], 0, oscMix[1], 0);
AudioConnection patchCord1_1b(sawOsc[1], 0, oscMix[1], 3);
AudioConnection patchCord2_1(subOsc[1], 0, oscMix[1], 1);
AudioConnection patchCord3_1(noise1, 0, oscMix[1], 2);
AudioConnection patchCord4_1(oscMix[1], 0, ampEnv[1], 0);
AudioConnection patchCord5_1(ampEnv[1], 0, lpFilter[1], 0);
AudioConnection patchCord6_1(dcFilter[1], filtEnv[1]);
AudioConnection patchCord7_1(filtEnv[1], 0, lpFilter[1], 1);
AudioConnection patchCord8_1(lpFilter[1], 0, hpFilter[1], 0);

// Voice 2 connections
AudioConnection patchCord1_2(pwmOsc[2], 0, oscMix[2], 0);
AudioConnection patchCord1_2b(sawOsc[2], 0, oscMix[2], 3);
AudioConnection patchCord2_2(subOsc[2], 0, oscMix[2], 1);
AudioConnection patchCord3_2(noise1, 0, oscMix[2], 2);
AudioConnection patchCord4_2(oscMix[2], 0, ampEnv[2], 0);
AudioConnection patchCord5_2(ampEnv[2], 0, lpFilter[2], 0);
AudioConnection patchCord6_2(dcFilter[2], filtEnv[2]);
AudioConnection patchCord7_2(filtEnv[2], 0, lpFilter[2], 1);
AudioConnection patchCord8_2(lpFilter[2], 0, hpFilter[2], 0);

// Voice 3 connections
AudioConnection patchCord1_3(pwmOsc[3], 0, oscMix[3], 0);
AudioConnection patchCord1_3b(sawOsc[3], 0, oscMix[3], 3);
AudioConnection patchCord2_3(subOsc[3], 0, oscMix[3], 1);
AudioConnection patchCord3_3(noise1, 0, oscMix[3], 2);
AudioConnection patchCord4_3(oscMix[3], 0, ampEnv[3], 0);
AudioConnection patchCord5_3(ampEnv[3], 0, lpFilter[3], 0);
AudioConnection patchCord6_3(dcFilter[3], filtEnv[3]);
AudioConnection patchCord7_3(filtEnv[3], 0, lpFilter[3], 1);
AudioConnection patchCord8_3(lpFilter[3], 0, hpFilter[3], 0);

// Voice 4 connections
AudioConnection patchCord1_4(pwmOsc[4], 0, oscMix[4], 0);
AudioConnection patchCord1_4b(sawOsc[4], 0, oscMix[4], 3);
AudioConnection patchCord2_4(subOsc[4], 0, oscMix[4], 1);
AudioConnection patchCord3_4(noise1, 0, oscMix[4], 2);
AudioConnection patchCord4_4(oscMix[4], 0, ampEnv[4], 0);
AudioConnection patchCord5_4(ampEnv[4], 0, lpFilter[4], 0);
AudioConnection patchCord6_4(dcFilter[4], filtEnv[4]);
AudioConnection patchCord7_4(filtEnv[4], 0, lpFilter[4], 1);
AudioConnection patchCord8_4(lpFilter[4], 0, hpFilter[4], 0);

// Voice 5 connections
AudioConnection patchCord1_5(pwmOsc[5], 0, oscMix[5], 0);
AudioConnection patchCord1_5b(sawOsc[5], 0, oscMix[5], 3);
AudioConnection patchCord2_5(subOsc[5], 0, oscMix[5], 1);
AudioConnection patchCord3_5(noise1, 0, oscMix[5], 2);
AudioConnection patchCord4_5(oscMix[5], 0, ampEnv[5], 0);
AudioConnection patchCord5_5(ampEnv[5], 0, lpFilter[5], 0);
AudioConnection patchCord6_5(dcFilter[5], filtEnv[5]);
AudioConnection patchCord7_5(filtEnv[5], 0, lpFilter[5], 1);
AudioConnection patchCord8_5(lpFilter[5], 0, hpFilter[5], 0);

// Mix all voices together
AudioConnection patchCordMix1(hpFilter[0], 0, voiceMix1, 0);
AudioConnection patchCordMix2(hpFilter[1], 0, voiceMix1, 1);
AudioConnection patchCordMix3(hpFilter[2], 0, voiceMix1, 2);
AudioConnection patchCordMix4(hpFilter[3], 0, voiceMix2, 0);
AudioConnection patchCordMix5(hpFilter[4], 0, voiceMix2, 1);
AudioConnection patchCordMix6(hpFilter[5], 0, voiceMix2, 2);

// Combine voice mixers into pre-chorus mix (mono bus)
AudioConnection patchCordPreChorus1(voiceMix1, 0, preChorusMix, 0);
AudioConnection patchCordPreChorus2(voiceMix2, 0, preChorusMix, 1);

// Feed the same mono bus into both chorus processors (wet-only outputs)
AudioConnection patchCordChorusInL(preChorusMix, 0, chorusL, 0);
AudioConnection patchCordChorusInR(preChorusMix, 0, chorusR, 0);

// Final stereo mix: dry + wet (each on its own mixer input)
AudioConnection patchCordDryL   (preChorusMix, 0, finalMixL, 0);   // dry -> input 0
AudioConnection patchCordWetL   (chorusL,      0, finalMixL, 1);   // wet L -> input 1

AudioConnection patchCordDryR   (preChorusMix, 0, finalMixR, 0);   // dry -> input 0
AudioConnection patchCordWetR   (chorusR,      0, finalMixR, 1);   // wet R -> input 1

// Stereo output
AudioConnection patchCordOutL(finalMixL, 0, usb1, 0);
AudioConnection patchCordOutR(finalMixR, 0, usb1, 1);
// AudioConnection patchCordOut3(finalMix, 0, i2s1, 0);
// AudioConnection patchCordOut4(finalMix, 0, i2s1, 1);

// AudioControlSGTL5000     sgt15000_1;

// ===== SYNTH PARAMETERS =====
struct PolyVoice {
  int note;
  bool active;
  unsigned long noteOnTime;
};

PolyVoice voices[VOICES];
int currentVoice = 0;  // Round-robin voice allocation

// Mono mode note stack for proper note priority
int monoNoteStack[16];  // Stack of held notes in mono mode
int monoStackSize = 0;

// Control values - Juno-60 specific parameters
float pwmVolume = 0.5;     // PWM volume level (0-1)
float pwmWidth = 0.5;      // PWM pulse width (0.05-0.95) 
float sawVolume = 0.8;     // Sawtooth volume level (0-1)
float subVolume = 0.0;     // Sub-oscillator volume (0-1)
float noiseVolume = 0.0;   // White noise volume (0-1)
float lfoRate = 0.5;       // LFO rate in Hz (0.1 - 20Hz)
float lfoDelay = 0.0;      // LFO delay time (0-2.5s)
float lfoPWMAmount = 0.0;  // LFO to PWM amount (0-1)
float lfoPitchAmount = 0.0; // LFO to pitch amount (0-1)
float lfoFilterAmount = 0.0; // LFO to filter amount (0-1)
float hpfCutoff = 20.0;    // High-pass filter cutoff (20-2000Hz)
float lpfCutoff = 8000.0;  // Low-pass filter cutoff (50-8000Hz)
float resonance = 0.0;     // Filter resonance (0-1)
float filterEnvAmount = 0.5; // Filter envelope amount (0-1)
float filtAttack = 10;     // Filter envelope attack (1-3000ms)
float filtDecay = 200;     // Filter envelope decay (10-5000ms)
float filtSustain = 0.5;   // Filter envelope sustain (0-1)
float filtRelease = 200;   // Filter envelope release (10-5000ms)
float ampAttack = 1;       // Amp envelope attack (1-3000ms)
float ampDecay = 100;      // Amp envelope decay (10-5000ms)
float ampSustain = 0.8;    // Amp envelope sustain (0-1)
float ampRelease = 100;    // Amp envelope release (10-5000ms)
int chorusMode = 0;        // Chorus mode (0=off, 1=I, 2=II, 3=I+II)
float modWheelValue = 0.0; // MIDI mod wheel (CC#1) 0-1
float pitchWheelValue = 0.0; // MIDI pitch wheel -1 to +1
int midiChannel = 0;       // MIDI channel (1-16, 0 = omni)
int playMode = 1;          // 0=Mono, 1=Poly, 2=Legato
float glideTime = 0.0;     // Glide/portamento time (0 = off, 0.1-1.0 = 100ms to 10s)
float targetFreq[VOICES];  // Target frequencies for glide
float currentFreq[VOICES]; // Current frequencies during glide
bool gliding[VOICES];      // Whether each voice is gliding

// Internal LFO state
float lfoPhase = 0.0;
float lfoOutput = 0.0;
unsigned long lfoStartTime = 0;
bool lfoDelayActive = false;

// Control system
#ifdef USE_LCD_DISPLAY
  LiquidCrystal_I2C lcd(0x27, 16, 2);
#endif

#ifdef USE_OLED_DISPLAY
  U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
#endif

int menuIndex = 0;
bool inMenu = false;

// Control names - Juno-60 specific parameters
const char* controlNames[] = {
  "PWM Volume", "PWM Width", "Saw Volume", "Sub Volume", "Noise Volume",
  "LFO Rate", "LFO Delay", "LFO>PWM", "LFO>Pitch", "LFO>Filter", 
  "HPF Cutoff", "LPF Cutoff", "Resonance", "Filter Env", "Filt Attack",
  "Filt Decay", "Filt Sustain", "Filt Release", "Amp Attack", "Amp Decay",
  "Amp Sustain", "Amp Release", "Chorus Mode", "Reserved", "Reserved", "Play Mode", "Glide Time", "Reserved", "Reserved", "Reserved", "MIDI Channel"
};

bool macroMode = false;

extern const MiniTeensyPreset presets[];
int currentPreset = 0;

MenuState currentMenuState = PARENT_MENU;
bool inPresetBrowse = false; // When browsing individual presets
int presetBrowseIndex = 0; // Which preset we're browsing


void updateLFOModulation() {
  // Juno-60 style LFO modulation with delay and multiple targets
  static unsigned long lastLFOUpdate = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastLFOUpdate < 2) return; 
  
  // Calculate time elapsed before updating lastLFOUpdate
  float timeElapsed = (currentTime - lastLFOUpdate) / 1000.0; // Convert ms to seconds
  lastLFOUpdate = currentTime;
  
  // Handle LFO delay - Juno-60 feature
  if (!lfoDelayActive && currentTime - lfoStartTime < (lfoDelay * 1000)) {
    lfoOutput = 0.0;  // LFO silent during delay period
  } else {
    lfoDelayActive = true;
    // Calculate LFO phase and output based on actual time elapsed
    lfoPhase += lfoRate * 2.0 * PI * timeElapsed;
    if (lfoPhase >= 2.0 * PI) lfoPhase -= 2.0 * PI;
    lfoOutput = sin(lfoPhase);  // Sine wave LFO (authentic Juno)
  }
  
  // Apply pitch wheel to all active voices
  float pitchWheelMultiplier = pow(2.0, pitchWheelValue * 2.0 / 12.0);
  
  // Apply LFO to different targets simultaneously (Juno style)
  for (int v = 0; v < VOICES; v++) {
    if (voices[v].active) {
      float baseFreq;
      if (gliding[v]) {
        baseFreq = currentFreq[v];
      } else {
        baseFreq = 440.0 * pow(2.0, (voices[v].note - 69) / 12.0);
      }
      
      // Apply LFO to pitch if enabled
      float pitchMultiplier = pitchWheelMultiplier;
      if (lfoPitchAmount > 0.01) {
        float pitchMod = lfoOutput * lfoPitchAmount * 0.1; // ±10% max
        pitchMultiplier *= (1.0 + pitchMod);
      }
      
      // Apply mod wheel to pitch (authentic Juno behavior)
      if (modWheelValue > 0.01) {
        float modPitchMod = lfoOutput * modWheelValue * 0.05; // ±5% for mod wheel
        pitchMultiplier *= (1.0 + modPitchMod);
      }
      
      // Update PWM, Sawtooth, and sub-oscillator frequencies
      pwmOsc[v].frequency(baseFreq * pitchMultiplier);
      sawOsc[v].frequency(baseFreq * pitchMultiplier);
      subOsc[v].frequency(baseFreq * 0.5 * pitchMultiplier);
    }
  }
  
  // Apply LFO to PWM width if enabled
  if (lfoPWMAmount > 0.01) {
    float pwmMod = lfoOutput * lfoPWMAmount * 0.3; // ±30% PWM modulation
    float modulatedWidth = constrain(pwmWidth + pwmMod, 0.05, 0.95);
    for (int v = 0; v < VOICES; v++) {
      pwmOsc[v].pulseWidth(modulatedWidth);
    }
  } else {
    // Reset to base PWM width
    for (int v = 0; v < VOICES; v++) {
      pwmOsc[v].pulseWidth(pwmWidth);
    }
  }
  
  // Apply LFO to filter if enabled
  if (lfoFilterAmount > 0.01) {
    float filterMod = lfoOutput * lfoFilterAmount * 2000.0; // ±2000 Hz
    float modulatedCutoff = constrain(lpfCutoff + filterMod, 50.0, 8000.0);
    for (int v = 0; v < VOICES; v++) {
      lpFilter[v].frequency(modulatedCutoff);
    }
  } else {
    // Reset to base cutoff
    for (int v = 0; v < VOICES; v++) {
      lpFilter[v].frequency(lpfCutoff);
    }
  }
}

void updateGlide() {
  if (glideTime == 0.0) return; // Glide is off
  
  static unsigned long lastGlideUpdate = 0;
  unsigned long currentTime = millis();
  
  // Update glide every 3ms for smooth transitions
  if (currentTime - lastGlideUpdate < 3) return; 
  lastGlideUpdate = currentTime;
  
  float glideTimeMs = 50 + (glideTime * 950); // 50ms to 1000ms (1 second max)
  float glideRate = 10.0 / glideTimeMs; 
  
  for (int v = 0; v < VOICES; v++) {
    if (voices[v].active && gliding[v]) {
      // Calculate frequency delta
      float freqDelta = targetFreq[v] - currentFreq[v];
      
      // If we're very close, snap to target and stop gliding
      if (abs(freqDelta) < 0.1) {
        currentFreq[v] = targetFreq[v];
        gliding[v] = false;
      } else {
        // Move towards target frequency
        currentFreq[v] += freqDelta * glideRate * 50.0;
      }
      
      // Apply the current frequency to Juno oscillators
      // LFO modulation will be applied on top of this in updateLFOModulation()
      float pitchWheelMultiplier = pow(2.0, pitchWheelValue * 2.0 / 12.0);
      pwmOsc[v].frequency(currentFreq[v] * pitchWheelMultiplier);
      sawOsc[v].frequency(currentFreq[v] * pitchWheelMultiplier);
      subOsc[v].frequency(currentFreq[v] * 0.5 * pitchWheelMultiplier);
    }
  }
}

#ifdef ENABLE_DIN_MIDI
// DIN MIDI callback handlers
void OnNoteOn(byte channel, byte note, byte velocity) {
  if (midiChannel != 0 && channel != midiChannel) return;
  noteOn(note, velocity);
}

void OnNoteOff(byte channel, byte note, byte velocity) {
  if (midiChannel != 0 && channel != midiChannel) return;
  noteOff(note);
}

void OnControlChange(byte channel, byte number, byte value) {
  if (midiChannel != 0 && channel != midiChannel) return;
  if (number == 1) modWheelValue = value / 127.0;
}

void OnPitchBend(byte channel, int bend) {
  if (midiChannel != 0 && channel != midiChannel) return;
  pitchWheelValue = (bend - 8192) / 8192.0;
}
#endif

#ifdef USE_MIDI_HOST
// USB Host MIDI callback handlers
void OnUSBHostNoteOn(byte channel, byte note, byte velocity) {
  if (midiChannel != 0 && channel != midiChannel) return;
  noteOn(note, velocity);
}
void OnUSBHostNoteOff(byte channel, byte note, byte velocity) {
  if (midiChannel != 0 && channel != midiChannel) return;
  noteOff(note);
}
void OnUSBHostControlChange(byte channel, byte number, byte value) {
  if (midiChannel != 0 && channel != midiChannel) return;
  if (number == 1) modWheelValue = value / 127.0;
}
void OnUSBHostPitchBend(byte channel, int bend) {
  if (midiChannel != 0 && channel != midiChannel) return;
  // USB Host MIDI uses signed range -8192 to +8191, center = 0
  pitchWheelValue = bend / 8192.0;
}
#endif

void setup() {
  Serial.begin(9600);
  AudioMemory(48); // Reduced from 60 to minimize latency
  

  // Optionally initialize USB Host MIDI for external controllers
#ifdef USE_MIDI_HOST
  myusb.begin();
  midi1.setHandleNoteOn(OnUSBHostNoteOn);
  midi1.setHandleNoteOff(OnUSBHostNoteOff);
  midi1.setHandleControlChange(OnUSBHostControlChange);
  midi1.setHandlePitchChange(OnUSBHostPitchBend);
  Serial.println("USB Host MIDI also enabled");
#endif

#ifdef ENABLE_DIN_MIDI
  // Initialize DIN MIDI
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(OnNoteOn);
  MIDI.setHandleNoteOff(OnNoteOff);
  MIDI.setHandleControlChange(OnControlChange);
  MIDI.setHandlePitchBend(OnPitchBend);
#endif
  
  // Initialize LFO
  lfo.frequency(lfoRate);
  lfo.amplitude(1.0);
  
  pinMode(MENU_ENCODER_SW, INPUT_PULLUP);
  
  // Initialize encoder values
  for (int i = 0; i < 20; i++) {
    encoderValues[i] = 0;
    lastEncoderValues[i] = 0;
  }
  
  // Initialize all parameter values to their current states
  for (int i = 0; i < 29; i++) {
    updateSynthParameter(i, allParameterValues[i]);
  }
  
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
  
  // Initialize all voice arrays for Juno-60 architecture
  for (int v = 0; v < VOICES; v++) {
    // Initialize VCO (main oscillator with PWM and SAW outputs)
    pwmOsc[v].begin(WAVEFORM_BANDLIMIT_PULSE);  // Bandwidth-limited PWM oscillator
    pwmOsc[v].amplitude(0.8);
    pwmOsc[v].pulseWidth(pwmWidth);  // Set initial PWM width
    
    sawOsc[v].begin(WAVEFORM_BANDLIMIT_SAWTOOTH);  // Bandwidth-limited sawtooth oscillator
    sawOsc[v].amplitude(0.8);
    
    // Initialize sub-oscillator (1 octave down, square wave)
    subOsc[v].begin(WAVEFORM_BANDLIMIT_SQUARE);
    subOsc[v].amplitude(0.8);
    
    // Configure mixer gains for Juno-60 oscillator setup
    oscMix[v].gain(0, pwmVolume * 0.8);     // PWM volume
    oscMix[v].gain(1, subVolume * 0.8);     // Sub oscillator volume  
    oscMix[v].gain(2, noiseVolume * 0.6);   // Noise volume
    oscMix[v].gain(3, sawVolume * 0.8);     // Sawtooth volume
    
    // Configure DC source for filter envelope
    dcFilter[v].amplitude(filterEnvAmount);
    
    // Configure low-pass filter (24dB/oct ladder filter)
    lpFilter[v].frequency(lpfCutoff);
    lpFilter[v].resonance(resonance);
    lpFilter[v].octaveControl(7.0); // Higher octave control for Juno-style filter tracking
    
    // Configure high-pass filter (12dB/oct)
    hpFilter[v].setHighpass(0, hpfCutoff, 0.707); // Q=0.707 for smooth response
    
    // Configure envelopes with Juno-60 timing
    ampEnv[v].attack(ampAttack);
    ampEnv[v].sustain(ampSustain);
    ampEnv[v].decay(ampDecay);
    ampEnv[v].release(ampRelease);
    
    filtEnv[v].attack(filtAttack);
    filtEnv[v].sustain(filtSustain);
    filtEnv[v].decay(filtDecay);
    filtEnv[v].release(filtRelease);
    
    // Initialize voice state
    voices[v].note = 0;
    voices[v].active = false;
    voices[v].noteOnTime = 0;
    
    // Initialize glide state
    targetFreq[v] = 0.0;
    currentFreq[v] = 0.0;
    gliding[v] = false;
  }
  
  // Initialize white noise generator
  noise1.amplitude(0.5);
  
  // Configure voice mixers
  voiceMix1.gain(0, 1.0);
  voiceMix1.gain(1, 1.0);
  voiceMix1.gain(2, 1.0);
  voiceMix1.gain(3, 0.0);
  
  voiceMix2.gain(0, 1.0);
  voiceMix2.gain(1, 1.0);
  voiceMix2.gain(2, 1.0);
  voiceMix2.gain(3, 0.0);
  
  // Configure pre-chorus mixer
  preChorusMix.gain(0, 1.0);  // voiceMix1
  preChorusMix.gain(1, 1.0);  // voiceMix2
  preChorusMix.gain(2, 0.0);
  preChorusMix.gain(3, 0.0);
  
  // Initialize authentic Juno-60 BBD chorus effect
  chorusL.begin(chorusDelayLineL, 500, false); // Left channel (master LFO)
  chorusR.begin(chorusDelayLineR, 500, true);  // Right channel (180° phase offset)
  chorusL.set_mode(chorusMode);
  chorusR.set_mode(chorusMode);
  
  // Configure stereo final mixers (dry + chorus)
  updateChorusMix(); // Set initial chorus mix based on bypass state
  finalMixL.gain(2, 0.0);
  finalMixL.gain(3, 0.0);
  finalMixR.gain(2, 0.0);
  finalMixR.gain(3, 0.0);
  
  // Initialize LFO
  lfo.frequency(lfoRate);
  lfo.amplitude(1.0);
  lfoStartTime = millis(); // Start LFO delay timer
    
// sgt15000_1.enable();
//     sgt15000_1.volume(1);

  delay(2000);
  updateDisplay();
  Serial.print(PROJECT_NAME);
  Serial.println(" Ready!");
}


// Read all 20 Mini-Teensy encoders (excluding menu encoder - handled in MenuNavigation.cpp)
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
  // encoderValues[11] handled separately for cutoff (uses menuEncoder hardware) in MenuNavigation.cpp
  encoderValues[12] = enc13.read() / 4; // enc13 (note: no enc12)
  encoderValues[13] = enc14.read() / 4; // enc14
  encoderValues[14] = enc15.read() / 4; // enc15
  encoderValues[15] = enc16.read() / 4; // enc16
  encoderValues[16] = enc17.read() / 4; // enc17
  encoderValues[17] = enc18.read() / 4; // enc18
  encoderValues[18] = enc19.read() / 4; // enc19
  encoderValues[19] = enc20.read() / 4; // enc20
}


void readAllControls() {
  // Read all encoders
  readDirectEncoders();
  
  // Check for encoder changes and update parameters using configurable mapping
  for (int i = 0; i < 20; i++) {
    if (encoderValues[i] != lastEncoderValues[i]) {
      // If any physical knob is turned, exit menu mode
      if (inMenu) {
        inMenu = false;
      }
      
      int change = encoderValues[i] - lastEncoderValues[i];
      int paramIndex = encoderMapping[i]; // Use configurable mapping
      
      // Only update if encoder is mapped to a valid parameter (not -1)
      if (paramIndex >= 0 && paramIndex < NUM_PARAMETERS) {
        // Handle macro mode for mapped parameters (Juno-60 style)
        int targetParam = paramIndex;
        if (macroMode && paramIndex == 14) targetParam = 5;   // Filter Attack -> LFO Rate  
        if (macroMode && paramIndex == 15) targetParam = 22;  // Filter Decay -> Chorus Mode
        if (macroMode && paramIndex == 16) targetParam = 26;  // Filter Sustain -> Glide Time
        
        updateEncoderParameter(targetParam, change);
      }
      
      lastEncoderValues[i] = encoderValues[i];
    }
  }
}

// Core synthesis parameter update function - Juno-60 specific
void updateSynthParameter(int paramIndex, float val) {
  switch (paramIndex) {
    case 0: // PWM Volume
      pwmVolume = val;
      updateOscillatorMix();
      break;
    case 1: // PWM Width  
      pwmWidth = 0.05 + val * 0.9; // 5% to 95% pulse width
      updatePWMWidth();
      break;
    case 2: // Saw Volume
      sawVolume = val;
      updateOscillatorMix();
      break;
    case 3: // Sub Volume
      subVolume = val;
      updateOscillatorMix();
      break;
    case 4: // Noise Volume
      noiseVolume = val;
      updateOscillatorMix();
      break;
    case 5: // LFO Rate
      lfoRate = 0.1 + val * 19.9; // 0.1 to 20 Hz (much faster for audio rate effects)
      lfo.frequency(lfoRate);
      break;
    case 6: // LFO Delay
      lfoDelay = val * 2.5; // 0 to 2.5 seconds
      break;
    case 7: // LFO to PWM Amount
      lfoPWMAmount = val; // 0.0 to 1.0
      break;
    case 8: // LFO to Pitch Amount
      lfoPitchAmount = val; // 0.0 to 1.0
      break;
    case 9: // LFO to Filter Amount
      lfoFilterAmount = val; // 0.0 to 1.0
      break;
    case 10: // HPF Cutoff
      hpfCutoff = 20 + val * 1980; // 20Hz to 2000Hz
      updateFilters();
      break;
    case 11: // LPF Cutoff
      lpfCutoff = 50 + val * 7950; // 50Hz to 8000Hz (authentic Juno range)
      updateFilters();
      break;
    case 12: // Resonance
      resonance = val * 4.0; // 0.0 to 4.0 (higher range for authentic Juno self-oscillation)
      updateFilters();
      break;
    case 13: // Filter Envelope Amount
      filterEnvAmount = val; // 0.0 to 1.0
      updateFilters();
      break;
    case 14: // Filter Attack
      filtAttack = 1 + val * 2999; // 1ms to 3000ms
      updateEnvelopes();
      break;
    case 15: // Filter Decay
      filtDecay = 10 + val * 4990; // 10ms to 5000ms
      updateEnvelopes();
      break;
    case 16: // Filter Sustain
      filtSustain = val; // 0.0 to 1.0
      updateEnvelopes();
      break;
    case 17: // Filter Release
      filtRelease = 10 + val * 4990; // 10ms to 5000ms
      updateEnvelopes();
      break;
    case 18: // Amp Attack
      ampAttack = 1 + val * 2999; // 1ms to 3000ms
      updateEnvelopes();
      break;
    case 19: // Amp Decay
      ampDecay = 10 + val * 4990; // 10ms to 5000ms
      updateEnvelopes();
      break;
    case 20: // Amp Sustain
      ampSustain = val; // 0.0 to 1.0
      updateEnvelopes();
      break;
    case 21: // Amp Release
      ampRelease = 10 + val * 4990; // 10ms to 5000ms
      updateEnvelopes();
      break;
    case 22: // Chorus Mode
      if (val < 0.25f) chorusMode = 0;      // 0.0-0.249 = Off
      else if (val < 0.5f) chorusMode = 1;  // 0.25-0.499 = Chorus I  
      else if (val < 0.75f) chorusMode = 2; // 0.5-0.749 = Chorus II
      else chorusMode = 3;                  // 0.75-1.0 = Chorus I+II
      chorusL.set_mode(chorusMode);
      chorusR.set_mode(chorusMode);
      updateChorusMix();
      break;
    case 23: // Reserved
      break;
    case 24: // Reserved
      break;
    case 25: // Play Mode
      if (val < 0.33) playMode = 0; // Mono
      else if (val < 0.66) playMode = 1; // Poly
      else playMode = 2; // Legato
      break;
    case 26: // Glide Time
      glideTime = val; // 0.0 to 1.0
      break;
    case 30: // MIDI Channel
      midiChannel = (int)(val * 16.0); // 0-16 (0 = omni, 1-16 = channels)
      break;
  }
}


int getMiniTeensyWaveform(float val, int osc) {
  if (osc == 1 || osc == 2) {
    if (val < 0.167) return WAVEFORM_TRIANGLE;
    else if (val < 0.333) return WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE;
    else if (val < 0.5) return WAVEFORM_BANDLIMIT_SAWTOOTH;
    else if (val < 0.667) return WAVEFORM_BANDLIMIT_SQUARE;
    else if (val < 0.833) return WAVEFORM_BANDLIMIT_PULSE;
    else return WAVEFORM_BANDLIMIT_PULSE;
  } else {
    if (val < 0.167) return WAVEFORM_TRIANGLE;
    else if (val < 0.333) return WAVEFORM_BANDLIMIT_SAWTOOTH;
    else if (val < 0.5) return WAVEFORM_BANDLIMIT_SAWTOOTH;
    else if (val < 0.667) return WAVEFORM_BANDLIMIT_SQUARE;
    else if (val < 0.833) return WAVEFORM_BANDLIMIT_PULSE;
    else return WAVEFORM_BANDLIMIT_PULSE;
  }
}

// Juno-60 specific helper functions

void updateOscillatorMix() {
  // Update oscillator mixer levels for all voices
  for (int v = 0; v < VOICES; v++) {
    oscMix[v].gain(0, pwmVolume * 0.8);     // PWM oscillator
    oscMix[v].gain(1, subVolume * 0.8);     // Sub oscillator
    oscMix[v].gain(2, noiseVolume * 0.6);   // White noise
    oscMix[v].gain(3, sawVolume * 0.8);     // Sawtooth (using mixer channel 3)
  }
}

void updatePWMWidth() {
  // Update PWM pulse width for all voices
  for (int v = 0; v < VOICES; v++) {
    pwmOsc[v].pulseWidth(pwmWidth);
  }
}

void updateFilters() {
  // Update both low-pass and high-pass filters for all voices
  for (int v = 0; v < VOICES; v++) {
    lpFilter[v].frequency(lpfCutoff);
    lpFilter[v].resonance(resonance);
    lpFilter[v].octaveControl(7.0); // Higher octave control for Juno-style filter tracking
    
    hpFilter[v].setHighpass(0, hpfCutoff, 0.707); // 12dB/oct high-pass with Q=0.707
    
    // Update filter envelope amount
    dcFilter[v].amplitude(filterEnvAmount);
  }
}

void updateChorusMix() {
  // Update stereo chorus wet/dry mix with authentic Juno-60 behavior
  if (chorusMode == 0) {
    // Chorus off - dry signal only (mono)
    finalMixL.gain(0, 1.0);  // Left dry signal
    finalMixL.gain(1, 0.0);  // No left chorus
    finalMixR.gain(0, 1.0);  // Right dry signal  
    finalMixR.gain(1, 0.0);  // No right chorus
  } else {
    // Chorus on - authentic Juno-60 stereo imaging and warmth
    // The original Juno-60 created wide stereo by mixing chorus differently L/R
    finalMixL.gain(0, 0.6);  // Left: strong dry signal for clarity
    finalMixL.gain(1, 0.8);  // Left: strong left BBD chorus
    finalMixR.gain(0, 0.6);  // Right: strong dry signal for clarity  
    finalMixR.gain(1, 0.8);  // Right: strong right BBD chorus (180° phase difference)
    
    // The 180° phase difference between L/R BBDs creates the wide stereo image
    // Combined with the different modulation creates that classic "swirling" effect
  }
}

void updateOscillatorFrequencies() {
  // Update all active voices with Juno-60 style oscillator setup
  for (int v = 0; v < VOICES; v++) {
    if (voices[v].active) {
      float baseFreq = 440.0 * pow(2.0, (voices[v].note - 69) / 12.0);
      float pitchWheelMultiplier = pow(2.0, pitchWheelValue * 2.0 / 12.0);
      
      // Main VCO (can be PWM or SAW via waveform selection)
      pwmOsc[v].frequency(baseFreq * pitchWheelMultiplier);
      sawOsc[v].frequency(baseFreq * pitchWheelMultiplier);
      
      // Sub-oscillator (one octave down, always square wave)
      subOsc[v].frequency(baseFreq * 0.5 * pitchWheelMultiplier);
    }
  }
}

void updateEnvelopes() {
  // Update envelopes for all voices
  for (int v = 0; v < VOICES; v++) {
    ampEnv[v].attack(ampAttack);
    ampEnv[v].sustain(ampSustain);
    ampEnv[v].decay(ampDecay);
    ampEnv[v].release(ampDecay);
    
    filtEnv[v].attack(filtAttack);
    filtEnv[v].sustain(filtSustain);
    filtEnv[v].decay(filtDecay);
    filtEnv[v].release(filtDecay);
  }
}

// Find next voice using round-robin allocation
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

// Find voice playing a specific note
int findVoiceForNote(int note) {
  for (int v = 0; v < VOICES; v++) {
    if (voices[v].active && voices[v].note == note) {
      return v;
    }
  }
  return -1; // Note not found
}

// Mono mode note stack management
void addToMonoStack(int note) {
  // Don't add if already in stack
  for (int i = 0; i < monoStackSize; i++) {
    if (monoNoteStack[i] == note) return;
  }
  // Add to top of stack
  if (monoStackSize < 16) {
    monoNoteStack[monoStackSize] = note;
    monoStackSize++;
  }
}

void removeFromMonoStack(int note) {
  // Find and remove note from stack
  for (int i = 0; i < monoStackSize; i++) {
    if (monoNoteStack[i] == note) {
      // Shift remaining notes down
      for (int j = i; j < monoStackSize - 1; j++) {
        monoNoteStack[j] = monoNoteStack[j + 1];
      }
      monoStackSize--;
      break;
    }
  }
}

int getTopMonoNote() {
  if (monoStackSize > 0) {
    return monoNoteStack[monoStackSize - 1]; // Return most recent note
  }
  return -1; // No notes in stack
}

void noteOn(int note, int velocity) {
  // Reset LFO delay on new note (Juno-60 behavior)
  lfoStartTime = millis();
  lfoDelayActive = false;
  
  int voiceNum = -1;
  
  if (playMode == 0) {
    // Mono mode - use note stack but retrigger envelope for every new note
    addToMonoStack(note);
    
    // Turn off other voices if they're somehow active
    for (int v = 1; v < VOICES; v++) {
      if (voices[v].active) {
        ampEnv[v].noteOff();
        filtEnv[v].noteOff();
        voices[v].active = false;
      }
    }
    
    // Always use voice 0 for mono mode
    voiceNum = 0;
    
    // If this is not the top note, don't trigger it yet
    if (getTopMonoNote() != note) {
      return; // Wait until this becomes the top note
    }
    
    // Stop current envelope if active
    if (voices[0].active) {
      ampEnv[0].noteOff();
      filtEnv[0].noteOff();
    }
    
    // Check if voice was active before setting up new note (for glide)
    bool wasActive = voices[0].active;
    
    // Set up the voice
    voices[0].note = note;
    voices[0].active = true;
    voices[0].noteOnTime = millis();
    
    // Calculate and set frequencies
    float baseFreq = 440.0 * pow(2.0, (note - 69) / 12.0);
    float pitchWheelMultiplier = pow(2.0, pitchWheelValue * 2.0 / 12.0);
    
    if (glideTime > 0.0 && wasActive) {
      // Glide enabled and voice was already active - start glide to new frequency
      targetFreq[0] = baseFreq;
      gliding[0] = true;
      // currentFreq[0] keeps its current value to glide from
    } else {
      // No glide or first note - set frequency directly
      currentFreq[0] = baseFreq;
      targetFreq[0] = baseFreq;
      gliding[0] = false;
      pwmOsc[0].frequency(baseFreq * pitchWheelMultiplier);
      sawOsc[0].frequency(baseFreq * pitchWheelMultiplier);
      subOsc[0].frequency(baseFreq * 0.5 * pitchWheelMultiplier);
    }
    
    // Always trigger envelopes in mono mode (retrigger for every note)
    ampEnv[0].noteOn();
    filtEnv[0].noteOn();
  } 
  else if (playMode == 2) {
    // Legato mode - use note stack for smooth transitions without envelope retrigger
    addToMonoStack(note);
    
    // Always use voice 0 for legato mode
    voiceNum = 0;
    
    // Turn off other voices if they're somehow active
    for (int v = 1; v < VOICES; v++) {
      if (voices[v].active) {
        ampEnv[v].noteOff();
        filtEnv[v].noteOff();
        voices[v].active = false;
      }
    }
    
    // If this is not the top note, don't trigger it yet
    if (getTopMonoNote() != note) {
      return; // Wait until this becomes the top note
    }
    
    // Don't retrigger envelope if voice 0 is already active (legato behavior)
    bool wasActive = voices[0].active;
    voices[0].note = note;
    voices[0].active = true;
    voices[0].noteOnTime = millis();
    
    // Calculate and set frequencies
    float baseFreq = 440.0 * pow(2.0, (note - 69) / 12.0);
    float pitchWheelMultiplier = pow(2.0, pitchWheelValue * 2.0 / 12.0);
    
    if (glideTime > 0.0 && wasActive) {
      // Glide enabled and voice was already active - start glide to new frequency
      targetFreq[0] = baseFreq;
      gliding[0] = true;
      // currentFreq[0] keeps its current value to glide from
    } else {
      // No glide or first note - set frequency directly
      currentFreq[0] = baseFreq;
      targetFreq[0] = baseFreq;
      gliding[0] = false;
      pwmOsc[0].frequency(baseFreq * pitchWheelMultiplier);
      sawOsc[0].frequency(baseFreq * pitchWheelMultiplier);
      subOsc[0].frequency(baseFreq * 0.5 * pitchWheelMultiplier);
    }
    
    // Only trigger envelopes if no note was previously active
    if (!wasActive) {
      ampEnv[0].noteOn();
      filtEnv[0].noteOn();
    }
  } 
  else {
    // Poly mode - normal polyphonic behavior
    voiceNum = findAvailableVoice();
    
    // Check if voice was active before stealing (for glide)
    bool wasActive = voices[voiceNum].active;
    
    // If voice stealing, turn off the old note
    if (voices[voiceNum].active) {
      ampEnv[voiceNum].noteOff();
      filtEnv[voiceNum].noteOff();
    }
    
    // Set up the voice
    voices[voiceNum].note = note;
    voices[voiceNum].active = true;
    voices[voiceNum].noteOnTime = millis();
    
    // Calculate and set frequencies
    float baseFreq = 440.0 * pow(2.0, (note - 69) / 12.0);
    float pitchWheelMultiplier = pow(2.0, pitchWheelValue * 2.0 / 12.0);
    
    if (glideTime > 0.0 && wasActive) {
      // Glide enabled and voice was being reused - start glide to new frequency
      targetFreq[voiceNum] = baseFreq;
      gliding[voiceNum] = true;
      // currentFreq[voiceNum] keeps its current value to glide from
    } else {
      // No glide or new voice - set frequency directly
      currentFreq[voiceNum] = baseFreq;
      targetFreq[voiceNum] = baseFreq;
      gliding[voiceNum] = false;
      pwmOsc[voiceNum].frequency(baseFreq * pitchWheelMultiplier);
      sawOsc[voiceNum].frequency(baseFreq * pitchWheelMultiplier);
      subOsc[voiceNum].frequency(baseFreq * 0.5 * pitchWheelMultiplier);
    }
    
    // Always trigger envelopes in poly mode
    ampEnv[voiceNum].noteOn();
    filtEnv[voiceNum].noteOn();
  }
}

void noteOff(int note) {
  if (playMode == 0) {
    // Mono mode - use note stack and retrigger envelope for next note
    removeFromMonoStack(note);
    
    // If the released note was the currently playing note
    if (voices[0].active && voices[0].note == note) {
      // Stop current envelope
      ampEnv[0].noteOff();
      filtEnv[0].noteOff();
      
      int nextNote = getTopMonoNote();
      if (nextNote != -1) {
        // Play the next note in the stack WITH envelope retrigger (mono behavior)
        voices[0].note = nextNote;
        float baseFreq = 440.0 * pow(2.0, (nextNote - 69) / 12.0);
        float pitchWheelMultiplier = pow(2.0, pitchWheelValue * 2.0 / 12.0);
        
        if (glideTime > 0.0) {
          // Start glide to the next note
          targetFreq[0] = baseFreq;
          gliding[0] = true;
          // currentFreq[0] keeps its current value to glide from
        } else {
          // No glide - set frequency directly
          currentFreq[0] = baseFreq;
          targetFreq[0] = baseFreq;
          gliding[0] = false;
          pwmOsc[0].frequency(baseFreq * pitchWheelMultiplier);
          sawOsc[0].frequency(baseFreq * pitchWheelMultiplier);
          subOsc[0].frequency(baseFreq * 0.5 * pitchWheelMultiplier);
        }
        
        // Retrigger envelopes for the next note (mono behavior)
        ampEnv[0].noteOn();
        filtEnv[0].noteOn();
      } else {
        // No more notes - voice stays off
        voices[0].active = false;
      }
    }
  } 
  else if (playMode == 2) {
    // Legato mode - use note stack for proper priority
    removeFromMonoStack(note);
    
    // If the released note was the currently playing note
    if (voices[0].active && voices[0].note == note) {
      int nextNote = getTopMonoNote();
      if (nextNote != -1) {
        // Play the next note in the stack without retriggering envelopes
        voices[0].note = nextNote;
        float baseFreq = 440.0 * pow(2.0, (nextNote - 69) / 12.0);
        float pitchWheelMultiplier = pow(2.0, pitchWheelValue * 2.0 / 12.0);
        
        if (glideTime > 0.0) {
          // Start glide to the next note
          targetFreq[0] = baseFreq;
          gliding[0] = true;
          // currentFreq[0] keeps its current value to glide from
        } else {
          // No glide - set frequency directly
          currentFreq[0] = baseFreq;
          targetFreq[0] = baseFreq;
          gliding[0] = false;
          pwmOsc[0].frequency(baseFreq * pitchWheelMultiplier);
          sawOsc[0].frequency(baseFreq * pitchWheelMultiplier);
          subOsc[0].frequency(baseFreq * 0.5 * pitchWheelMultiplier);
        }
      } else {
        // No more notes - turn off envelopes
        ampEnv[0].noteOff();
        filtEnv[0].noteOff();
        voices[0].active = false;
      }
    }
  } 
  else {
    // Poly mode - normal note off behavior
    int voiceNum = findVoiceForNote(note);
    if (voiceNum >= 0) {
      // Turn off envelopes
      ampEnv[voiceNum].noteOff();
      filtEnv[voiceNum].noteOff();
      voices[voiceNum].active = false;
    }
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
        modWheelValue = data2 / 127.0; // Convert to 0.0-1.0 range
        // No serial output for performance
      }
    } else if (type == usbMIDI.PitchBend) {
      // Handle pitch wheel (14-bit value)
      int pitchBendValue = (data2 << 7) | data1; // Combine MSB and LSB
      pitchWheelValue = (pitchBendValue - 8192) / 8192.0; // Convert to -1.0 to +1.0 range
      // No serial output for performance
    }
  }
#endif

#ifdef USE_MIDI_HOST
  myusb.Task();
  midi1.read();
#endif
  
#ifdef ENABLE_DIN_MIDI
  MIDI.read();
#endif
  
  readAllControls();
  handleEncoder();
  updateLFOModulation();
  updateGlide();
  
  // Minimal serial input check for performance
  if (Serial.available()) {
    char input = Serial.read();
    if (input == 'r' || input == 'R') {
      resetEncoderBaselines();
    }
  }
  delay(5); // Reduced delay for better responsiveness
}