/*
 * MiniTeensy Synth
 * A 6-voice polyphonic virtual analog synthesizer built with the Teensy 4.1 microcontroller, 
 * inspired by the classic Minimoog. Features comprehensive synthesis with USB audio/MIDI 
 * and intuitive menu control.
 * 
 * REQUIRED LIBRARIES (install via Arduino Library Manager):
 * - LiquidCrystal I2C (by Frank de Brabander)
 * - Encoder (by Paul Stoffregen) 
 * - MIDI Library (by Francois Best) - only needed if enabling DIN MIDI
 * 
 * Built-in Teensy libraries (no installation needed):
 * - Audio, Wire, SPI, SD, SerialFlash, USBHost_t36
 */

#include "config.h"
#include <USBHost_t36.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "MenuNavigation.h"
#include <Encoder.h>

#define DX7_IMPLEMENTATION
#include "src/synth_dexed.h"
#include "src/Synth_Braids/synth_braids.h"
#include "src/effect_platervbstereo.h"
#include "dx7_rom1a_unpacked.h"

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

#ifdef ENABLE_TEENSY_DAC
// TODO: Add DAC Support
#endif


// TODO: Test DIN MIDI
// Uncomment to enable DIN MIDI support (requires moving enc3 from pin 0)
// #define ENABLE_DIN_MIDI

// Build Configuration Options
// Uncomment for minimal build with only menu encoder + LCD (no 20 parameter encoders)
// #define MINIMAL_BUILD

/*
 * DIN MIDI Setup Instructions:
 * 
 * HARDWARE REQUIRED:
 * - 6N138 optocoupler IC
 * - 220Ω resistor  
 * - 5-pin DIN MIDI connector
 * - Standard MIDI interface circuit (see MIDI specification)
 * 
 * WIRING:
 * 1. Build MIDI input circuit: DIN connector → 6N138 optocoupler → 220Ω resistor
 * 2. Connect MIDI circuit output to Teensy Serial1 RX (Pin 0)
 * 3. IMPORTANT: Move enc3 (Osc3 Range) CLK wire from Pin 0 to surface mount pin (42-47)
 * 
 * USAGE:
 * - Install "MIDI Library" by Francois Best via Arduino Library Manager
 * - Uncomment #define ENABLE_DIN_MIDI above
 * - Supports both USB and DIN MIDI simultaneously
 * - Uses same MIDI channel setting from Settings menu
 * - Receives Note On/Off, Control Change, and Pitch Bend
 */

EngineType currentEngine = ENGINE_VA;
ParentMenuState currentParentMenu = PARENT_PARAMETERS;
bool inParentMenu = false;

// FM synthesis components
AudioSynthDexed          dexed(8, AUDIO_SAMPLE_RATE);
bool dx7BankLoaded = false;
int currentDX7Preset = 0;
int currentDX7Bank = 0;
int dx7BankIndex = 0;
int dx7PatchIndex = 0;
uint8_t dx7EngineType = 0; // MSFA engine (warmest, no pops)

// Modular synthesis components (polyphonic)
AudioSynthBraids         braidsOsc[VOICES];
AudioEffectEnvelope      braidsEnvelope[VOICES]; 
AudioFilterBiquad        braidsFilter[VOICES];
AudioMixer4              braidsMixer;  // Final mix of all voices
int currentBraidsPreset = 0;
int braidsPresetBrowseIndex = 0; 

AudioMixer4              engineMix; 
int engineBrowseIndex = 0; 

// Encoder definitions
#ifndef MINIMAL_BUILD
// Full build: 20 parameter encoders + menu encoder
Encoder enc1(4, 5);
Encoder enc2(2, 3);
Encoder enc3(0, 1);
Encoder enc4(8, 9);
Encoder enc5(6, 7);
Encoder enc6(25, 27);
Encoder enc7(12, 24);
Encoder enc8(10, 11);
Encoder enc9(29, 30);
Encoder enc10(28, 26);
Encoder enc11(21, 20);
Encoder enc13(34, 33);
Encoder enc14(50, 41);
Encoder enc15(23, 22);
Encoder enc16(36, 35);
Encoder enc17(31, 32);
Encoder enc18(17, 16);
Encoder enc19(38, 37);
Encoder enc20(40, 39);
long encoderValues[20] = {0};
long lastEncoderValues[20] = {0};
#endif

Encoder menuEncoder(MENU_ENCODER_DT, MENU_ENCODER_CLK);
// Default parameter values - matches "Init" preset
float allParameterValues[41] = {
  0.417, 0.417, 0.417, 0.500, 0.500, 0.417, 0.417, 0.417, 0.789, 0.789,
  0.789, 1.000, 0.000, 0.000, 0.160, 1.000, 0.000, 0.000, 1.000, 0.016,
  0.500, 1.000, 0.250, 0.000, 0.000, 0.330, 0.330, 0.000, 0.000, 0.000,
  0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.500, 0.500, 0.500, 0.500,
  0.000
};

// Audio synthesis
AudioSynthWaveform       osc1[VOICES], osc2[VOICES], osc3[VOICES];
AudioSynthNoiseWhite     noise1;           // White noise source
AudioSynthNoisePink      noisePink;        // Pink noise source
AudioMixer4              noiseMix;         // Mix white/pink noise
AudioSynthWaveformDc     dcFilter[VOICES]; // DC source for filter envelope per voice
AudioSynthWaveformSine   lfo;              // LFO for modulation
AudioMixer4              oscMix[VOICES];   // Mix 3 oscs + noise per voice
AudioFilterLadder        filter1[VOICES];  // Filter per voice
AudioEffectEnvelope      ampEnv[VOICES], filtEnv[VOICES]; // Envelopes per voice
AudioMixer4              voiceMix1, voiceMix2, finalMix;  // Mix voices together

// J60 specific audio components
AudioSynthWaveformSine   pwmLfo;           // PWM LFO for pulse width modulation
AudioFilterBiquad        junoHPF[VOICES];  // High-pass filter per voice (J60 style)

// Shared effects chain for all engines
AudioEffectFlange        sharedFlangeL, sharedFlangeR;    // Shared stereo flanger/chorus for all engines
AudioEffectPlateReverb   sharedPlateReverb;               // Shared plate reverb (stereo)
AudioMixer4              junoMix, junoMix2, junoFinalMix; // Juno voice mixing and final mix before flange
AudioMixer4              effectsInputL, effectsInputR;    // Mix all engines before effects
AudioMixer4              finalMixL, finalMixR;            // Final stereo output mixers (effects + dry)
short sharedFlangeDelayBufferL[FLANGE_DELAY_LENGTH];
short sharedFlangeDelayBufferR[FLANGE_DELAY_LENGTH];



AudioOutputUSB           usb1;

// Audio connections - Polyphonic chain
// Voice 0 connections
AudioConnection patchCord1_0(osc1[0], 0, oscMix[0], 0);
AudioConnection patchCord2_0(osc2[0], 0, oscMix[0], 1);
AudioConnection patchCord3_0(osc3[0], 0, oscMix[0], 2);
AudioConnection patchCord4_0(noiseMix, 0, oscMix[0], 3);
AudioConnection patchCord5_0(oscMix[0], 0, ampEnv[0], 0);
AudioConnection patchCord6_0(ampEnv[0], 0, filter1[0], 0);
AudioConnection patchCord7_0(dcFilter[0], filtEnv[0]);
AudioConnection patchCord12_0(filtEnv[0], 0, filter1[0], 1);

// Voice 1 connections
AudioConnection patchCord1_1(osc1[1], 0, oscMix[1], 0);
AudioConnection patchCord2_1(osc2[1], 0, oscMix[1], 1);
AudioConnection patchCord3_1(osc3[1], 0, oscMix[1], 2);
AudioConnection patchCord4_1(noiseMix, 0, oscMix[1], 3);
AudioConnection patchCord5_1(oscMix[1], 0, ampEnv[1], 0);
AudioConnection patchCord6_1(ampEnv[1], 0, filter1[1], 0);
AudioConnection patchCord7_1(dcFilter[1], filtEnv[1]);
AudioConnection patchCord12_1(filtEnv[1], 0, filter1[1], 1);

// Voice 2 connections
AudioConnection patchCord1_2(osc1[2], 0, oscMix[2], 0);
AudioConnection patchCord2_2(osc2[2], 0, oscMix[2], 1);
AudioConnection patchCord3_2(osc3[2], 0, oscMix[2], 2);
AudioConnection patchCord4_2(noiseMix, 0, oscMix[2], 3);
AudioConnection patchCord5_2(oscMix[2], 0, ampEnv[2], 0);
AudioConnection patchCord6_2(ampEnv[2], 0, filter1[2], 0);
AudioConnection patchCord7_2(dcFilter[2], filtEnv[2]);
AudioConnection patchCord12_2(filtEnv[2], 0, filter1[2], 1);

// Voice 3 connections
AudioConnection patchCord1_3(osc1[3], 0, oscMix[3], 0);
AudioConnection patchCord2_3(osc2[3], 0, oscMix[3], 1);
AudioConnection patchCord3_3(osc3[3], 0, oscMix[3], 2);
AudioConnection patchCord4_3(noiseMix, 0, oscMix[3], 3);
AudioConnection patchCord5_3(oscMix[3], 0, ampEnv[3], 0);
AudioConnection patchCord6_3(ampEnv[3], 0, filter1[3], 0);
AudioConnection patchCord7_3(dcFilter[3], filtEnv[3]);
AudioConnection patchCord12_3(filtEnv[3], 0, filter1[3], 1);

// Voice 4 connections
AudioConnection patchCord1_4(osc1[4], 0, oscMix[4], 0);
AudioConnection patchCord2_4(osc2[4], 0, oscMix[4], 1);
AudioConnection patchCord3_4(osc3[4], 0, oscMix[4], 2);
AudioConnection patchCord4_4(noiseMix, 0, oscMix[4], 3);
AudioConnection patchCord5_4(oscMix[4], 0, ampEnv[4], 0);
AudioConnection patchCord6_4(ampEnv[4], 0, filter1[4], 0);
AudioConnection patchCord7_4(dcFilter[4], filtEnv[4]);
AudioConnection patchCord12_4(filtEnv[4], 0, filter1[4], 1);

// Voice 5 connections
AudioConnection patchCord1_5(osc1[5], 0, oscMix[5], 0);
AudioConnection patchCord2_5(osc2[5], 0, oscMix[5], 1);
AudioConnection patchCord3_5(osc3[5], 0, oscMix[5], 2);
AudioConnection patchCord4_5(noiseMix, 0, oscMix[5], 3);
AudioConnection patchCord5_5(oscMix[5], 0, ampEnv[5], 0);
AudioConnection patchCord6_5(ampEnv[5], 0, filter1[5], 0);
AudioConnection patchCord7_5(dcFilter[5], filtEnv[5]);
AudioConnection patchCord12_5(filtEnv[5], 0, filter1[5], 1);

AudioConnection patchCordNoiseWhite(noise1, 0, noiseMix, 0);
AudioConnection patchCordNoisePink(noisePink, 0, noiseMix, 1);

// Mix all voices together
AudioConnection patchCordMix1(filter1[0], 0, voiceMix1, 0);
AudioConnection patchCordMix2(filter1[1], 0, voiceMix1, 1);
AudioConnection patchCordMix3(filter1[2], 0, voiceMix1, 2);
AudioConnection patchCordMix4(filter1[3], 0, voiceMix2, 0);
AudioConnection patchCordMix5(filter1[4], 0, voiceMix2, 1);
AudioConnection patchCordMix6(filter1[5], 0, voiceMix2, 2);

// Combine both voice mixers into final mono mix
AudioConnection patchCordFinal1(voiceMix1, 0, finalMix, 0);
AudioConnection patchCordFinal2(voiceMix2, 0, finalMix, 1);

// J60 high-pass filter connections (parallel to main filter chain)
AudioConnection patchCord_juno_hpf0(filter1[0], 0, junoHPF[0], 0);
AudioConnection patchCord_juno_hpf1(filter1[1], 0, junoHPF[1], 0);
AudioConnection patchCord_juno_hpf2(filter1[2], 0, junoHPF[2], 0);
AudioConnection patchCord_juno_hpf3(filter1[3], 0, junoHPF[3], 0);
AudioConnection patchCord_juno_hpf4(filter1[4], 0, junoHPF[4], 0);
AudioConnection patchCord_juno_hpf5(filter1[5], 0, junoHPF[5], 0);

// Juno voice mixing through high-pass filters (need additional mixers for 6 voices)
AudioConnection patchCord_juno_mix1(junoHPF[0], 0, junoMix, 0);
AudioConnection patchCord_juno_mix2(junoHPF[1], 0, junoMix, 1);
AudioConnection patchCord_juno_mix3(junoHPF[2], 0, junoMix, 2);
AudioConnection patchCord_juno_mix4(junoHPF[3], 0, junoMix, 3);
AudioConnection patchCord_juno_mix5(junoHPF[4], 0, junoMix2, 0);
AudioConnection patchCord_juno_mix6(junoHPF[5], 0, junoMix2, 1);

// Combine both Juno mixers
AudioConnection patchCord_juno_final1(junoMix, 0, junoFinalMix, 0);
AudioConnection patchCord_juno_final2(junoMix2, 0, junoFinalMix, 1);

// All engines route to shared effects input mixers
AudioConnection patchCord_va_to_effectsL(finalMix, 0, effectsInputL, 0);       // VA to effects input left
AudioConnection patchCord_va_to_effectsR(finalMix, 0, effectsInputR, 0);       // VA to effects input right
AudioConnection patchCord_juno_to_effectsL(junoFinalMix, 0, effectsInputL, 1); // Juno to effects input left  
AudioConnection patchCord_juno_to_effectsR(junoFinalMix, 0, effectsInputR, 1); // Juno to effects input right
AudioConnection patchCord_dx7_to_effectsL(dexed, 0, effectsInputL, 2);         // DX7 to effects input left
AudioConnection patchCord_dx7_to_effectsR(dexed, 0, effectsInputR, 2);         // DX7 to effects input right

// Braids audio signal chain: Oscillator → Envelope → Filter → Mixer → Effects
AudioConnection patchCord_braids_osc_env0(braidsOsc[0], 0, braidsEnvelope[0], 0);
AudioConnection patchCord_braids_env_flt0(braidsEnvelope[0], 0, braidsFilter[0], 0);
AudioConnection patchCord_braids_flt_mix0(braidsFilter[0], 0, braidsMixer, 0);

AudioConnection patchCord_braids_osc_env1(braidsOsc[1], 0, braidsEnvelope[1], 0);
AudioConnection patchCord_braids_env_flt1(braidsEnvelope[1], 0, braidsFilter[1], 0);
AudioConnection patchCord_braids_flt_mix1(braidsFilter[1], 0, braidsMixer, 1);

AudioConnection patchCord_braids_osc_env2(braidsOsc[2], 0, braidsEnvelope[2], 0);
AudioConnection patchCord_braids_env_flt2(braidsEnvelope[2], 0, braidsFilter[2], 0);
AudioConnection patchCord_braids_flt_mix2(braidsFilter[2], 0, braidsMixer, 2);

AudioConnection patchCord_braids_osc_env3(braidsOsc[3], 0, braidsEnvelope[3], 0);
AudioConnection patchCord_braids_env_flt3(braidsEnvelope[3], 0, braidsFilter[3], 0);
AudioConnection patchCord_braids_flt_mix3(braidsFilter[3], 0, braidsMixer, 3);

AudioConnection patchCord_braids_osc_env4(braidsOsc[4], 0, braidsEnvelope[4], 0);
AudioConnection patchCord_braids_env_flt4(braidsEnvelope[4], 0, braidsFilter[4], 0);
AudioConnection patchCord_braids_osc_env5(braidsOsc[5], 0, braidsEnvelope[5], 0);
AudioConnection patchCord_braids_env_flt5(braidsEnvelope[5], 0, braidsFilter[5], 0);

// Mix remaining voices into the main braidsMixer through submixers
AudioMixer4             braidsVoiceMixer45;  // Mix voices 4 and 5
AudioMixer4             braidsFinalMixer;    // Mix first 4 voices with voices 4-5
AudioConnection patchCord_braids_flt_v45_0(braidsFilter[4], 0, braidsVoiceMixer45, 0);
AudioConnection patchCord_braids_flt_v45_1(braidsFilter[5], 0, braidsVoiceMixer45, 1);

// Final mixing: braidsMixer (voices 0-3) + braidsVoiceMixer45 (voices 4-5)
AudioConnection patchCord_braids_main_final(braidsMixer, 0, braidsFinalMixer, 0);
AudioConnection patchCord_braids_v45_final(braidsVoiceMixer45, 0, braidsFinalMixer, 1);

// Connect final Braids mixer to effects (input 3)
AudioConnection patchCord_braids_to_effectsL(braidsFinalMixer, 0, effectsInputL, 3);
AudioConnection patchCord_braids_to_effectsR(braidsFinalMixer, 0, effectsInputR, 3);

// Simplified effects chain: Input → Flanger → Plate Reverb → Output
AudioConnection patchCord_effects_inL(effectsInputL, 0, sharedFlangeL, 0);     // Input to flanger
AudioConnection patchCord_effects_inR(effectsInputR, 0, sharedFlangeR, 0);
AudioConnection patchCord_flange_reverbL(sharedFlangeL, 0, sharedPlateReverb, 0);   // Flanger left to plate reverb
AudioConnection patchCord_flange_reverbR(sharedFlangeR, 0, sharedPlateReverb, 1);   // Flanger right to plate reverb

// Final output - effects + dry signal mixing  
AudioConnection patchCord_effects_outL(sharedPlateReverb, 0, finalMixL, 0);         // Effects left (from plate reverb)
AudioConnection patchCord_effects_outR(sharedPlateReverb, 1, finalMixR, 0);         // Effects right (from plate reverb)
AudioConnection patchCord_dry_outL(effectsInputL, 0, finalMixL, 1);                 // Dry left
AudioConnection patchCord_dry_outR(effectsInputR, 0, finalMixR, 1);                 // Dry right

AudioConnection patchCordOut1(finalMixL, 0, usb1, 0);  // True stereo left
AudioConnection patchCordOut2(finalMixR, 0, usb1, 1);  // True stereo right

#ifdef ENABLE_TEENSY_DAC
AudioOutputI2S           i2s1;
AudioControlSGTL5000     sgtl5000_1;
AudioConnection          patchCordOut3(finalMixL, 0, i2s1, 0);
AudioConnection          patchCordOut4(finalMixR, 0, i2s1, 1);
#endif

// ===== SYNTH PARAMETERS =====
// Polyphonic voice state
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

// Control values
float osc1Range = 1.0, osc2Range = 1.0, osc3Range = 1.0;
float osc1Fine = 1.0, osc2Fine = 1.0, osc3Fine = 1.0;
int osc1Wave = WAVEFORM_BANDLIMIT_SAWTOOTH, osc2Wave = WAVEFORM_BANDLIMIT_SAWTOOTH, osc3Wave = WAVEFORM_BANDLIMIT_SAWTOOTH;
float vol1 = 0.3, vol2 = 0.3, vol3 = 0.3, noiseVol = 0.0;
float ampAttack = 0, ampSustain = 0.8, ampDecay = 100, ampRelease = 100;
float filtAttack = 100, filtSustain = 0.5, filtDecay = 2500, filtRelease = 2500; 
float cutoff = 1000, resonance = 0.0; 
float filterStrength = 0.5; // Filter envelope strength (0-1)
float lfoRate = 5.0;     // LFO rate in Hz (0.1 - 20Hz)
float lfoDepth = 0.0;    // LFO depth (0-1)
bool lfoEnabled = false; // LFO on/off toggle
int lfoTarget = 1;       // 0=Pitch, 1=Filter, 2=Amp

// Effect parameters
bool chorusBypass = true;  // Chorus bypass toggle (start bypassed)
float chorusRate = 0.27;   // Chorus modulation rate (0-1, maps to 0-1 Hz) - J60 chorus speed
float chorusDepth = 0.6;   // Chorus modulation depth (0-1) - deeper for chorus effect
bool reverbBypass = true;  // Reverb bypass toggle (start bypassed)
float reverbSize = 0.3;    // Reverb size/time (0-1)
float reverbHidamp = 0.5;  // Reverb high frequency damping (0-1)
float reverbLodamp = 0.0;  // Reverb low frequency damping (0-1)
float reverbLowpass = 0.7; // Reverb lowpass filter (0-1)
float reverbDiffusion = 0.65;    // Reverb diffusion (0-1)
float modWheelValue = 0.0;       // MIDI mod wheel (CC#1) 0-1
float pitchWheelValue = 0.0;     // MIDI pitch wheel -1 to +1
float lastPitchWheelValue = 0.0; // Track changes to prevent unnecessary updates
int midiChannel = 0;             // MIDI channel (1-16, 0 = omni)
unsigned long lastMidiTime = 0;  // For MIDI throttling
int playMode = 1;                // 0=Mono, 1=Poly, 2=Legato
float glideTime = 0.0;           // Glide/portamento time (0 = off, 0.1-1.0 = 100ms to 10s)
int noiseType = 0;               // 0 = White, 1 = Pink
float targetFreq[VOICES];        // Target frequencies for glide
float currentFreq[VOICES];       // Current frequencies during glide
bool gliding[VOICES];            // Whether each voice is gliding

// Control system
#ifdef USE_LCD_DISPLAY
  LiquidCrystal_I2C lcd(0x27, 16, 2);
#endif

#ifdef USE_OLED_DISPLAY
  U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
#endif

int menuIndex = 0;
bool inMenu = false;

// Control names
const char* controlNames[] = {
  "Osc1 Range", "Osc2 Range", "Osc3 Range", "Osc2 Fine", "Osc3 Fine",
  "Osc1 Wave", "Osc2 Wave", "Osc3 Wave", "Volume 1", "Volume 2", 
  "Volume 3", "Cutoff", "Resonance", "Filt Attack", "Filt Decay",
  "Filt Sustain", "Noise Vol", "Amp Attack", "Amp Sustain", "Amp Decay",
  "Osc1 Fine", "Filt Strength", "LFO Rate", "LFO Depth", "LFO Toggle", "LFO Target", "Play Mode", "Glide Time", "Noise Type", "Macro Mode", "MIDI Channel", "Chorus Bypass", "Chorus Rate", "Chorus Depth", "Reverb Bypass", "Reverb Size", "Reverb HiDamp", "Reverb LoDamp", "Reverb Lowpass", "Reverb Diffusion"
};

bool macroMode = false;

// Braids engine parameters (separate from VA/Juno parameter system)
float braidsParameters[NUM_BRAIDS_PARAMETERS] = {
  30.0,   // 0: Shape (0-42)
  64.0,   // 1: Timbre (0-127)  
  32.0,   // 2: Color (0-127)
  0.0,    // 3: Coarse (transpose)
  20.0,   // 4: Amp Attack (0-127)
  60.0,   // 5: Amp Decay (0-127)
  100.0,  // 6: Amp Sustain (0-127)  
  40.0,   // 7: Amp Release (0-127)
  1.0,    // 8: Filter Mode (0=Off, 1=LP, 2=BP, 3=HP)
  80.0,   // 9: Filter Cutoff (0-127)
  10.0,   // 10: Filter Resonance (0-127)
  50.0,   // 11: Filter Strength (0-127)
  10.0,   // 12: Filter Attack (0-127)
  30.0,   // 13: Filter Decay (0-127)
  80.0,   // 14: Filter Sustain (0-127)
  50.0,   // 15: Filter Release (0-127)
  80.0    // 16: Volume (0-127)
};

const char* braidsControlNames[NUM_BRAIDS_PARAMETERS] = {
  "Shape", "Timbre", "Color", "Coarse", 
  "Amp Attack", "Amp Decay", "Amp Sustain", "Amp Release",
  "Filter Mode", "Filter Cutoff", "Filter Res", "Filter Strength",
  "Filt Attack", "Filt Decay", "Filt Sustain", "Filt Release",
  "Volume"
};

// Braids filter envelope state (for dynamic filter sweeps like MicroDexed-touch)
float braidsFilterState[VOICES];        // Current filter frequency for each voice
float braidsFilterTarget[VOICES];       // Target filter frequency for each voice  
bool braidsFilterEnvActive[VOICES];     // Whether filter envelope is active for each voice
unsigned long braidsNoteOnTime[VOICES]; // Note-on time for envelope calculation

// Braids algorithm names (48 synthesis modes)
const char* braidsAlgorithmNames[48] = {
  "CSAW",  // 0: Crossfading sawtooth
  "MORPH", // 1: Variable waveform morphing  
  "SAW/SQ", // 2: Sawtooth to square morphing
  "FOLD",  // 3: Sine to triangle morphing
  "BUZZ",  // 4: Harmonically rich buzz
  "SUB SQ", // 5: Square with sub-oscillator
  "SUB SW", // 6: Sawtooth with sub-oscillator
  "SYN SQ", // 7: Hard sync square waves
  "SYN SW", // 8: Hard sync sawtooth waves
  "SAW x3", // 9: Three detuned sawtooth waves
  "SQ x3", // 10: Three detuned square waves
  "TRI x3", // 11: Three detuned triangle waves
  "SIN x3", // 12: Three detuned sine waves
  "RING",  // 13: Triple ring modulation
  "SWARM", // 14: Swarming sawtooth oscillators
  "COMB",  // 15: Comb-filtered sawtooth
  "TOY",   // 16: Toy-like digital synthesis
  "Z LPF", // 17: Low-pass filtered oscillator
  "Z PKF", // 18: Peak filtered oscillator  
  "Z BPF", // 19: Band-pass filtered oscillator
  "Z HPF", // 20: High-pass filtered oscillator
  "VOSIM", // 21: VOice SIMulation synthesis
  "VOWEL", // 22: Vowel formant synthesis
  "V FOF", // 23: FOrmant synthesis (FOF)
  "HARM",  // 24: Additive harmonic synthesis
  "FM",    // 25: Frequency modulation
  "FBFM",  // 26: FM with feedback
  "WTFM",  // 27: Wavetable FM / Chaotic FM
  "PLUCK", // 28: Plucked string model
  "BOWED", // 29: Bowed string model
  "BLOWN", // 30: Blown/wind instrument model
  "FLUTE", // 31: Flute-like synthesis
  "BELL",  // 32: Struck bell/metallic sounds
  "DRUM",  // 33: Drum synthesis
  "KICK",  // 34: Kick drum synthesis
  "CYMBAL", // 35: Cymbal synthesis
  "SNARE", // 36: Snare drum synthesis
  "NOISE", // 37: Filtered noise synthesis
  "TWINQ", // 38: Twin peaks filtered noise
  "CLKN",  // 39: Clocked noise
  "CLOUD", // 40: Granular synthesis cloud
  "PRTCL", // 41: Particle-based noise
  "QPSK",  // 42: Digital modulation
  "ALG43", // 43: Reserved algorithm
  "ALG44", // 44: Reserved algorithm
  "ALG45", // 45: Reserved algorithm
  "ALG46", // 46: Reserved algorithm
  "ALG47"  // 47: Reserved algorithm
};

// Define synth presets
// Parameter mapping: 0-2:Osc1-3Range, 
// 3-4:Osc2-3Fine,
// 5-7:Osc1-3Wave,
// 8-10:Osc1-3Vol,
// 11:Cutoff,
// 12:Resonance,
// 13-15:FilterADSR,
// 16:Noise,
// 17-19:AmpADSR,
// 20:Osc1Fine,
// 21:FilterStrength
const MiniTeensyPreset presets[] = {
  {"80s Brass", {0.417, 0.417, 0.417, 0.539, 0.445, 0.417, 0.417, 0.417, 1.000, 0.789, 0.594, 0.562, 0.023, 0.039, 0.160, 0.000, 0.000, 0.000, 1.000, 0.026, 0.500, 1.000, 0.250, 0.000, 0.000, 0.330, 0.330, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.500, 0.500, 0.500, 0.500, 0.000}},
  {"Saw Keys", {0.417, 0.417, 0.417, 0.453, 0.539, 0.417, 0.417, 0.250, 0.695, 0.789, 0.789, 0.633, 0.039, 0.000, 0.097, 0.469, 0.000, 0.000, 1.000, 0.039, 0.500, 1.000, 0.250, 0.000, 0.000, 0.330, 0.580, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.500, 0.500, 0.500, 0.500, 0.000}},
  {"Square Keys", {0.417, 0.250, 0.417, 0.453, 0.547, 0.583, 0.917, 0.750, 0.789, 0.789, 0.789, 0.633, 0.008, 0.000, 0.113, 0.000, 0.000, 0.000, 0.448, 0.039, 0.500, 1.000, 0.008, 0.023, 1.000, 0.040, 0.330, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.500, 0.500, 0.500, 0.500, 0.000}},
  {"8-Bit Square", {0.417, 0.417, 0.417, 0.500, 0.500, 0.583, 0.417, 0.417, 0.789, 0.000, 0.000, 0.852, 0.000, 0.000, 0.160, 1.000, 0.000, 0.000, 1.000, 0.000, 0.500, 1.000, 0.250, 0.000, 0.000, 0.330, 1.000, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.500, 0.500, 0.500, 0.500, 0.000}},
  {"Butter Supersaw", {0.417, 0.417, 0.417, 0.547, 0.453, 0.417, 0.417, 0.417, 0.594, 0.789, 0.789, 0.609, 0.016, 0.000, 0.238, 0.437, 0.020, 0.000, 1.000, 0.008, 0.500, 1.000, 0.008, 0.016, 1.000, 0.330, 0.330, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.500, 0.500, 0.500, 0.500, 0.000}},
  {"West Coast Lead", {0.250, 0.417, 0.417, 0.477, 0.523, 0.417, 0.417, 0.250, 0.000, 1.000, 1.000, 0.711, 0.000, 0.000, 0.238, 1.000, 0.000, 0.000, 1.000, 0.000, 0.500, 1.000, 0.250, 0.000, 0.000, 0.330, 0.930, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.500, 0.500, 0.500, 0.500, 0.000}},
  {"Teensy Lead", {0.250, 0.417, 0.417, 0.477, 0.523, 0.583, 0.583, 0.750, 0.000, 1.000, 1.000, 0.633, 0.000, 0.000, 0.238, 1.000, 0.000, 0.000, 1.000, 0.000, 0.500, 1.000, 0.250, 0.000, 1.000, 0.150, 0.930, 0.711, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.500, 0.500, 0.500, 0.500, 0.000}},
  {"Analog Bass", {0.417, 0.417, 0.250, 0.531, 0.484, 0.417, 0.417, 0.583, 1.000, 1.000, 1.000, 0.617, 0.047, 0.000, 0.019, 0.000, 0.000, 0.000, 1.000, 0.000, 0.500, 1.000, 0.016, 0.055, 1.000, 0.330, 0.280, 0.812, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.500, 0.500, 0.500, 0.500, 0.000}},
  {"Legato Bass", {0.250, 0.417, 0.417, 0.477, 0.523, 0.583, 0.417, 0.250, 0.953, 1.000, 1.000, 0.711, 0.023, 0.000, 0.160, 0.633, 0.000, 0.000, 1.000, 0.000, 0.500, 1.000, 0.250, 0.000, 0.000, 0.330, 1.000, 1.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.500, 0.500, 0.500, 0.500, 0.000}},
  {"Funk Bass", {0.250, 0.417, 0.417, 0.539, 0.461, 0.583, 0.417, 0.250, 1.000, 1.000, 1.000, 0.516, 0.172, 0.000, 0.066, 0.000, 0.000, 0.000, 0.828, 0.002, 0.500, 1.000, 0.250, 0.000, 0.000, 0.330, 0.230, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.500, 0.500, 0.500, 0.500, 0.000}},
  {"8-Bit Harp", {0.750, 0.417, 0.750, 0.523, 0.445, 0.583, 0.750, 0.417, 1.000, 1.000, 0.508, 0.664, 0.000, 0.000, 0.051, 0.000, 0.050, 0.000, 0.000, 0.253, 0.500, 1.000, 0.250, 0.000, 0.000, 0.330, 0.330, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.500, 0.500, 0.500, 0.500, 0.000}},
  {"Love Pad", {0.417, 0.417, 0.417, 0.414, 0.562, 0.417, 0.417, 0.417, 0.789, 0.789, 0.789, 0.609, 0.109, 0.148, 0.504, 0.000, 0.240, 0.070, 0.863, 0.206, 0.500, 0.250, 0.008, 0.023, 1.000, 0.040, 0.330, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.500, 0.500, 0.500, 0.500, 0.000}},
  {"Thoughtful Pad", {0.750, 0.417, 0.250, 0.484, 0.539, 0.583, 0.417, 0.083, 0.227, 0.797, 0.750, 0.445, 0.195, 0.508, 1.000, 0.000, 0.290, 0.141, 1.000, 0.227, 0.500, 1.000, 0.250, 0.000, 0.000, 0.330, 0.580, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.500, 0.500, 0.500, 0.500, 0.000}},
  {"Saw Pad", {0.417, 0.417, 0.417, 0.578, 0.453, 0.417, 0.417, 0.250, 0.789, 0.789, 0.789, 0.508, 0.023, 0.187, 0.371, 0.508, 0.100, 0.047, 0.820, 0.320, 0.500, 1.000, 0.250, 0.000, 0.000, 0.330, 0.330, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.500, 0.500, 0.500, 0.500, 0.000}},
  {"5th Pad", {0.500, 0.500, 0.500, 0.895, 0.536, 0.417, 0.417, 0.417, 0.750, 0.750, 0.780, 0.590, 0.060, 0.113, 0.270, 0.230, 0.000, 0.000, 0.800, 0.018, 0.500, 0.500, 0.250, 0.000, 0.000, 0.330, 0.330, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.500, 0.500, 0.500, 0.500, 0.000}},
  {"Classic Sweep", {0.417, 0.417, 0.417, 0.578, 0.453, 0.417, 0.417, 0.250, 0.789, 0.789, 0.789, 0.453, 0.312, 0.031, 1.000, 0.000, 0.490, 0.000, 1.000, 0.031, 0.500, 1.000, 0.250, 0.000, 0.000, 0.330, 0.330, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.500, 0.500, 0.500, 0.500, 0.000}},
  {"Haunted Organ", {0.417, 0.583, 0.750, 0.476, 0.523, 0.083, 0.083, 0.083, 0.766, 0.594, 0.578, 0.516, 0.000, 0.000, 0.019, 0.625, 0.110, 0.000, 0.687, 0.018, 0.500, 1.000, 0.016, 0.039, 1.000, 0.210, 0.330, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.500, 0.500, 0.500, 0.500, 0.000}},
  {"Synth Drum", {0.417, 0.583, 0.250, 0.891, 0.445, 0.083, 0.083, 0.583, 0.117, 0.047, 0.016, 0.422, 0.093, 0.000, 0.016, 0.000, 1.000, 0.000, 0.000, 0.026, 0.500, 1.000, 0.250, 0.000, 0.000, 0.330, 0.330, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.500, 0.500, 0.500, 0.500, 0.000}},
  {"Noise-scape", {0.417, 0.417, 0.417, 0.539, 0.445, 0.417, 0.417, 0.417, 0.000, 0.000, 0.000, 0.203, 0.273, 0.000, 0.555, 1.000, 1.000, 0.187, 1.000, 0.253, 0.500, 1.000, 0.250, 0.000, 0.000, 0.330, 0.330, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.500, 0.500, 0.500, 0.500, 0.000}},
  {"Init", {0.417, 0.417, 0.417, 0.500, 0.500, 0.417, 0.417, 0.417, 0.789, 0.789, 0.789, 1.000, 0.000, 0.000, 0.160, 1.000, 0.000, 0.000, 1.000, 0.016, 0.500, 1.000, 0.250, 0.000, 0.000, 0.330, 0.330, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.500, 0.500, 0.500, 0.500, 0.000}}
};

// NUM_PRESETS defined in config.h
int currentPreset = 0;

// J60 presets structure 
// JunoPreset struct now defined in MenuNavigation.h

// Classic J60 style presets (resonance reduced to prevent feedback)
const JunoPreset junoPresets[] = {
  // Classic Juno Bass - DCO sawtooth + sub-osc square, punchy filter (no chorus)
  {"Juno Bass", {0.417, 0.417, 0.417, 0.500, 0.500, 0.417, 0.583, 0.417, 0.900, 0.400, 0.000, 0.350, 0.000, 0.010, 0.080, 0.600, 0.000, 0.000, 0.900, 0.020, 0.500, 1.000, 0.250, 0.000, 0.000, 0.200, 0.400, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.000, 0.700, 0.650, 0.000, 0.000}},
  
  // String Ensemble - Pulse wave + sub, slow attack, effects bypassed
  {"Juno Strings", {0.417, 0.417, 0.417, 0.500, 0.500, 0.583, 0.583, 0.000, 0.600, 0.300, 0.000, 0.450, 0.000, 0.150, 0.350, 0.500, 0.000, 0.050, 0.700, 0.400, 0.500, 0.800, 0.200, 0.000, 0.000, 0.500, 0.600, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.000, 0.700, 0.650, 0.000, 0.000}},
  
  // Brass Stab - Sawtooth main + pulse, filter movement, punchy envelope (no chorus)
  {"Juno Brass", {0.417, 0.417, 0.417, 0.500, 0.500, 0.417, 0.583, 0.417, 0.800, 0.300, 0.200, 0.500, 0.015, 0.030, 0.150, 0.400, 0.000, 0.000, 0.900, 0.080, 0.500, 1.000, 0.250, 0.000, 0.000, 0.300, 0.500, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.000, 0.700, 0.650, 0.000, 0.000}},
  
  // Polysynthesizer - PWM pulse waves, classic 80s poly sound with sub-osc, effects bypassed
  {"Juno Poly", {0.417, 0.417, 0.417, 0.500, 0.500, 0.583, 0.583, 0.583, 0.700, 0.250, 0.100, 0.550, 0.008, 0.020, 0.200, 0.450, 0.000, 0.010, 0.750, 0.100, 0.500, 1.000, 0.250, 0.000, 0.000, 0.350, 0.450, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.000, 0.700, 0.650, 0.000, 0.000}},
  
  // Lead Synthesizer - Sawtooth + PWM pulse, filter sweep, sustain (no chorus)
  {"Juno Lead", {0.417, 0.417, 0.417, 0.500, 0.500, 0.417, 0.583, 0.000, 0.900, 0.200, 0.000, 0.650, 0.025, 0.010, 0.100, 0.700, 0.000, 0.000, 0.900, 0.020, 0.500, 1.000, 0.250, 0.000, 0.000, 0.200, 0.700, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.000, 0.700, 0.650, 0.000, 0.000}},
  
  // Warm Pad - PWM pulse waves only, slow everything, atmospheric, effects bypassed
  {"Juno Pad", {0.417, 0.417, 0.417, 0.500, 0.500, 0.583, 0.583, 0.000, 0.500, 0.350, 0.000, 0.400, 0.000, 0.200, 0.500, 0.600, 0.000, 0.100, 0.600, 0.300, 0.500, 0.600, 0.200, 0.000, 0.000, 0.600, 0.650, 0.000, 0.000, 0.000, 0.000, 1.000, 0.500, 0.500, 1.000, 0.500, 0.000, 0.700, 0.650, 0.000, 0.000}}
};

// NUM_JUNO_PRESETS defined in config.h
int currentJunoPreset = 0;

// Menu state enums moved to MenuNavigation.h

MenuState currentMenuState = PARAMETERS;
bool inPresetBrowse = false; // When browsing individual presets
int presetBrowseIndex = 0; // Which preset we're browsing
int junoPresetBrowseIndex = 0; // Which Juno preset we're browsing

// Menu parameter mapping moved to MenuNavigation.cpp

// Waveform names for display
const char* waveformNames[] = {
  "Triangle", "Shark Tooth", "Sawtooth", "Wide Pulse", "Med Pulse", "Narrow Pulse"
};

// Range names for display  
const char* rangeNames[] = {
  "32'", "16'", "8'", "4'", "2'", "LO"
};

void updateLFOModulation() {
  // Throttle LFO updates to reduce CPU load
  static unsigned long lastLFOUpdate = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastLFOUpdate < 2) return; // Very fast updates for responsive mod/pitch wheels
  lastLFOUpdate = currentTime;
  
  // Apply pitch wheel to all active voices first
  float pitchWheelMultiplier = pow(2.0, pitchWheelValue * 2.0 / 12.0);
  
  // Calculate LFO output - authentic analog synth behavior
  float lfoOut = 0.0;
  
  // Authentic vintage behavior: mod wheel always works, LFO is independent
  float phase = (currentTime * lfoRate * 2 * PI) / 1000.0;
  float lfoSignal = sin(phase);
  
  // Calculate total modulation depth
  float totalDepth = 0.0;
  
  // Internal LFO: only active when enabled AND has depth
  if (lfoEnabled && lfoDepth > 0.01) {
    totalDepth += lfoDepth;
  }
  
  // Mod wheel: always works when moved (authentic vintage behavior)
  if (modWheelValue > 0.01) {
    totalDepth += modWheelValue;
  }
  
  // Apply modulation if any source is active
  if (totalDepth > 0.01) {
    lfoOut = lfoSignal * totalDepth;
  }
  
  // Apply LFO based on target
  if (lfoTarget == 0) {
    // Pitch target - apply LFO to oscillators
    float pitchLFOMultiplier = 1.0 + (lfoOut * 0.1); // ±10% frequency change
    for (int v = 0; v < VOICES; v++) {
      if (voices[v].active) {
        // Use current glide frequency if gliding, otherwise calculate from note
        float baseFreq;
        if (gliding[v]) {
          baseFreq = currentFreq[v];
        } else {
          baseFreq = 440.0 * pow(2.0, (voices[v].note - 69) / 12.0);
        }
        float totalPitchMultiplier = pitchWheelMultiplier * pitchLFOMultiplier;
        setVoiceFrequencies(v, baseFreq, totalPitchMultiplier);
      }
    }
    // Keep filter at base cutoff
    for (int v = 0; v < VOICES; v++) {
      filter1[v].frequency(cutoff);
    }
  } 
  else if (lfoTarget == 1) {
    // Filter target - apply LFO to filter cutoff
    float filterModulation = lfoOut * 1000.0; // ±1000 Hz
    float modulatedCutoff = constrain(cutoff + filterModulation, 20.0, 20000.0);
    for (int v = 0; v < VOICES; v++) {
      filter1[v].frequency(modulatedCutoff);
    }
    // Keep oscillators at base pitch (with pitch wheel, using glide if active)
    for (int v = 0; v < VOICES; v++) {
      if (voices[v].active) {
        // Use current glide frequency if gliding, otherwise calculate from note
        float baseFreq;
        if (gliding[v]) {
          baseFreq = currentFreq[v];
        } else {
          baseFreq = 440.0 * pow(2.0, (voices[v].note - 69) / 12.0);
        }
        osc1[v].frequency(baseFreq * osc1Range * osc1Fine * pitchWheelMultiplier);
        osc2[v].frequency(baseFreq * osc2Range * osc2Fine * pitchWheelMultiplier);
        osc3[v].frequency(baseFreq * osc3Range * osc3Fine * pitchWheelMultiplier);
      }
    }
  }
  else if (lfoTarget == 2) {
    // Amplitude target - apply LFO to mixer gains
    float ampMultiplier = 1.0 + (lfoOut * 0.5); // ±50% amplitude
    for (int v = 0; v < VOICES; v++) {
      oscMix[v].gain(0, vol1 * ampMultiplier);
      oscMix[v].gain(1, vol2 * ampMultiplier);
      oscMix[v].gain(2, vol3 * ampMultiplier);
    }
    // Keep oscillators at base pitch (with pitch wheel, using glide if active)
    for (int v = 0; v < VOICES; v++) {
      if (voices[v].active) {
        // Use current glide frequency if gliding, otherwise calculate from note
        float baseFreq;
        if (gliding[v]) {
          baseFreq = currentFreq[v];
        } else {
          baseFreq = 440.0 * pow(2.0, (voices[v].note - 69) / 12.0);
        }
        osc1[v].frequency(baseFreq * osc1Range * osc1Fine * pitchWheelMultiplier);
        osc2[v].frequency(baseFreq * osc2Range * osc2Fine * pitchWheelMultiplier);
        osc3[v].frequency(baseFreq * osc3Range * osc3Fine * pitchWheelMultiplier);
      }
    }
    // Keep filter at base cutoff
    for (int v = 0; v < VOICES; v++) {
      filter1[v].frequency(cutoff);
    }
  }
}

// Helper function to set oscillator frequencies with engine-specific characteristics
void setVoiceFrequencies(int voiceNum, float baseFreq, float pitchWheelMultiplier) {
  if (currentEngine == ENGINE_JUNO) {
    // J60 style: Main DCO with PWM + sub oscillator (one octave down)
    osc1[voiceNum].frequency(baseFreq * pitchWheelMultiplier);        // Main DCO
    osc2[voiceNum].frequency(baseFreq * 0.5 * pitchWheelMultiplier);  // Sub oscillator (square wave)
    osc3[voiceNum].frequency(baseFreq * pitchWheelMultiplier);        // Additional oscillator
    
    // Apply PWM LFO to pulse waves (when pulse wave is selected)
    // PWM LFO modulates pulse width from 0.1 to 0.9
    // Calculate LFO manually since AudioSynthWaveformSine doesn't have read()
    static unsigned long lastPwmUpdate = 0;
    unsigned long currentTime = millis();
    float pwmValue = 0.5; // Default center position
    
    if (currentTime - lastPwmUpdate >= 10) { // Update every 10ms for smooth PWM
      float phase = (currentTime * 0.5 * 2 * PI) / 1000.0; // 0.5 Hz PWM rate
      float lfoOut = sin(phase);
      pwmValue = 0.5 + 0.4 * lfoOut; // Center around 0.5, +/- 0.4 range
      pwmValue = constrain(pwmValue, 0.1, 0.9);
      lastPwmUpdate = currentTime;
    }
    
    // Apply PWM to oscillators when they're set to pulse wave
    if (osc1Wave == WAVEFORM_PULSE || osc1Wave == WAVEFORM_BANDLIMIT_PULSE) {
      osc1[voiceNum].pulseWidth(pwmValue);
    }
    if (osc2Wave == WAVEFORM_PULSE || osc2Wave == WAVEFORM_BANDLIMIT_PULSE) {
      osc2[voiceNum].pulseWidth(pwmValue);
    }
    if (osc3Wave == WAVEFORM_PULSE || osc3Wave == WAVEFORM_BANDLIMIT_PULSE) {
      osc3[voiceNum].pulseWidth(pwmValue);
    }
  } else {
    // VA engine: Use existing parameter-controlled frequencies
    osc1[voiceNum].frequency(baseFreq * osc1Range * osc1Fine * pitchWheelMultiplier);
    osc2[voiceNum].frequency(baseFreq * osc2Range * osc2Fine * pitchWheelMultiplier);
    osc3[voiceNum].frequency(baseFreq * osc3Range * osc3Fine * pitchWheelMultiplier);
  }
}




// Helper function to release envelopes  
void releaseVoiceEnvelopes(int voiceNum) {
  ampEnv[voiceNum].noteOff();
  filtEnv[voiceNum].noteOff();
}

void updateGlide() {
  if (glideTime == 0.0) return; // Glide is off
  
  static unsigned long lastGlideUpdate = 0;
  unsigned long currentTime = millis();
  
  // Update glide every 5ms for smooth transitions
  if (currentTime - lastGlideUpdate < 3) return; // Very smooth glide transitions
  lastGlideUpdate = currentTime;
  
  float glideTimeMs = 50 + (glideTime * 950); // 50ms to 1000ms (1 second max)
  float glideRate = 10.0 / glideTimeMs; // Much more aggressive rate
  
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
        currentFreq[v] += freqDelta * glideRate * 50.0; // Much faster transition
      }
      
      // Apply the current frequency to oscillators with all multipliers
      // Only set frequency directly if LFO is not targeting pitch (LFO will handle it)
      if (lfoTarget != 0) {
        float pitchWheelMultiplier = pow(2.0, pitchWheelValue * 2.0 / 12.0);
        setVoiceFrequencies(v, currentFreq[v], pitchWheelMultiplier);
      }
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

// ===== ENGINE SWITCHING FUNCTIONS =====

void switchEngine(EngineType newEngine) {
  if (newEngine == currentEngine) return;
  
  // Stop all notes
  allNotesOff();
  
  // Switch engine 
  currentEngine = newEngine;
  
  // Update engine routing to shared effects chain and disable unused nodes for CPU savings
  switch(currentEngine) {
    case ENGINE_VA:
      // VA active, others disabled
      effectsInputL.gain(0, 1.0); effectsInputR.gain(0, 1.0);  // VA active
      effectsInputL.gain(1, 0.0); effectsInputR.gain(1, 0.0);  // Juno off
      effectsInputL.gain(2, 0.0); effectsInputR.gain(2, 0.0);  // DX7 off
      effectsInputL.gain(3, 0.0); effectsInputR.gain(3, 0.0);  // Braids off
      // Disable Braids audio processing to save CPU via mixer gains
      braidsMixer.gain(0, 0.0); braidsMixer.gain(1, 0.0); 
      braidsMixer.gain(2, 0.0); braidsMixer.gain(3, 0.0);
      braidsVoiceMixer45.gain(0, 0.0); braidsVoiceMixer45.gain(1, 0.0);
      braidsFinalMixer.gain(0, 0.0); braidsFinalMixer.gain(1, 0.0);
      // VA doesn't use effects by default
      finalMixL.gain(0, 0.0); finalMixR.gain(0, 0.0);  // Effects off
      break;
    case ENGINE_JUNO:
      // Juno active, others disabled  
      effectsInputL.gain(0, 0.0); effectsInputR.gain(0, 0.0);  // VA off
      effectsInputL.gain(1, 1.0); effectsInputR.gain(1, 1.0);  // Juno active
      effectsInputL.gain(2, 0.0); effectsInputR.gain(2, 0.0);  // DX7 off
      effectsInputL.gain(3, 0.0); effectsInputR.gain(3, 0.0);  // Braids off
      // Disable Braids audio processing to save CPU via mixer gains
      braidsMixer.gain(0, 0.0); braidsMixer.gain(1, 0.0); 
      braidsMixer.gain(2, 0.0); braidsMixer.gain(3, 0.0);
      braidsVoiceMixer45.gain(0, 0.0); braidsVoiceMixer45.gain(1, 0.0);
      braidsFinalMixer.gain(0, 0.0); braidsFinalMixer.gain(1, 0.0);
      // Effects mix controlled by preset (set in loadJunoPreset)
      Serial.println("Using Juno engine with shared effects chain");
      break;
    case ENGINE_DX7:
      // DX7 active, others disabled
      effectsInputL.gain(0, 0.0); effectsInputR.gain(0, 0.0);  // VA off
      effectsInputL.gain(1, 0.0); effectsInputR.gain(1, 0.0);  // Juno off
      effectsInputL.gain(2, 0.6); effectsInputR.gain(2, 0.6);  // DX7 active (reduced volume)
      effectsInputL.gain(3, 0.0); effectsInputR.gain(3, 0.0);  // Braids off
      // Disable Braids audio processing to save CPU via mixer gains
      braidsMixer.gain(0, 0.0); braidsMixer.gain(1, 0.0); 
      braidsMixer.gain(2, 0.0); braidsMixer.gain(3, 0.0);
      braidsVoiceMixer45.gain(0, 0.0); braidsVoiceMixer45.gain(1, 0.0);
      braidsFinalMixer.gain(0, 0.0); braidsFinalMixer.gain(1, 0.0);
      // DX7 doesn't use effects by default
      finalMixL.gain(0, 0.0); finalMixR.gain(0, 0.0);  // Effects off
      break;
    case ENGINE_BRAIDS:
      // Braids active, others disabled
      effectsInputL.gain(0, 0.0); effectsInputR.gain(0, 0.0);  // VA off
      effectsInputL.gain(1, 0.0); effectsInputR.gain(1, 0.0);  // Juno off
      effectsInputL.gain(2, 0.0); effectsInputR.gain(2, 0.0);  // DX7 off
      effectsInputL.gain(3, 0.8); effectsInputR.gain(3, 0.8);  // Braids active
      // Re-enable Braids audio processing via mixer gains
      braidsMixer.gain(0, 0.25); braidsMixer.gain(1, 0.25); 
      braidsMixer.gain(2, 0.25); braidsMixer.gain(3, 0.25);
      braidsVoiceMixer45.gain(0, 0.25); braidsVoiceMixer45.gain(1, 0.25);
      braidsFinalMixer.gain(0, 0.5); braidsFinalMixer.gain(1, 0.5);
      // Braids uses effects by default for modular sound
      finalMixL.gain(0, 0.3); finalMixR.gain(0, 0.3);  // Effects on (moderate level)
      Serial.println("Using Braids engine with shared effects chain");
      break;
  }
  
  // Load first preset for each engine
  switch(currentEngine) {
    case ENGINE_VA:
      loadPreset(0); // Load first VA preset
      break;
    case ENGINE_JUNO:
      loadJunoPreset(0); // Load first Juno preset
      // Apply J60 specific parameter constraints
      applyJunoParameterMapping();
      Serial.println("J60 engine configured with authentic parameters");
      break;
    case ENGINE_DX7:
      loadDX7Preset(0); // Load first DX7 preset
      break;
    case ENGINE_BRAIDS:
      loadBraidsPreset(0); // Load first Braids preset
      Serial.println("Braids engine configured with modular synthesis parameters");
      break;
  }
  
  Serial.print("Switched to engine: ");
  const char* engineNames[] = {"VA", "J60", "DX7", "Braids"};
  Serial.println(engineNames[currentEngine]);
}


// Preset array is defined at line ~393

// Preset loading functions
void loadPreset(int presetIndex) {
  presetIndex = constrain(presetIndex, 0, NUM_PRESETS - 1);
  
  Serial.print("Loading VA preset ");
  Serial.print(presetIndex);
  Serial.print(": ");
  Serial.println(presets[presetIndex].name);
  
  // Load all parameter values from preset (41 parameters unified structure)
  for (int i = 0; i < 41; i++) {
    allParameterValues[i] = presets[presetIndex].parameters[i];
    updateSynthParameter(i, allParameterValues[i]);
  }
  
  // CRITICAL: Force update all voices regardless of active state
  // This ensures waveforms and frequencies are properly initialized
  
  currentPreset = presetIndex;
}

void loadJunoPreset(int presetIndex) {
  presetIndex = constrain(presetIndex, 0, NUM_JUNO_PRESETS - 1);
  
  Serial.print("Loading Juno preset ");
  Serial.print(presetIndex);
  Serial.print(": ");
  Serial.println(junoPresets[presetIndex].name);
  
  // Load all parameter values from preset (41 parameters unified structure)
  for (int i = 0; i < 41; i++) {
    allParameterValues[i] = junoPresets[presetIndex].parameters[i];
    updateSynthParameter(i, allParameterValues[i]);
  }
  
  currentJunoPreset = presetIndex;
  applyJunoParameterMapping();
}

// DX7 preset loading
void loadDX7Preset(int presetNum) {
  presetNum = constrain(presetNum, 0, 31);
  currentDX7Preset = presetNum;
  dexed.loadVoiceParameters(progmem_bank[currentDX7Bank][presetNum]);
  
  Serial.print("Loaded DX7 preset ");
  Serial.print(presetNum);
  Serial.print(": ");
  
  // Extract patch name for display
  char voice_name[11];
  memset(voice_name, 0, 11);
  memcpy(voice_name, &progmem_bank[currentDX7Bank][presetNum][144], 10);
  Serial.println(voice_name);
  
}

const BraidsPreset braidsPresets[] = {
  {"CSaw Lead", 0, 64, 32, 5, 20, 100, 50, 3000, 30},       // Classic saw lead
  {"Sine Pad", 8, 20, 80, 40, 80, 120, 100, 1500, 10},      // Sine wave pad
  {"FM Bell", 16, 90, 60, 10, 60, 80, 80, 4000, 20},        // FM-style bell
  {"Noise Drum", 24, 127, 100, 1, 30, 0, 20, 2000, 50},     // Percussion
  {"Vocal Formant", 32, 80, 90, 15, 40, 90, 60, 2500, 40},  // Vocal-like
  {"Digital Harsh", 40, 120, 127, 5, 25, 70, 30, 5000, 80}, // Digital/harsh
  {"Analog Sync", 4, 100, 70, 8, 35, 85, 45, 3500, 60},     // Sync sweep
  {"Pluck Bass", 12, 60, 40, 3, 15, 60, 25, 800, 20},       // Bass pluck
};

void loadBraidsPreset(int presetNum) {
  presetNum = constrain(presetNum, 0, NUM_BRAIDS_PRESETS - 1);
  currentBraidsPreset = presetNum;
  
  const BraidsPreset& preset = braidsPresets[presetNum];
  
  // Set Braids oscillator parameters for all voices
  for (int v = 0; v < VOICES; v++) {
    braidsOsc[v].set_braids_shape(preset.shape);
    braidsOsc[v].set_braids_timbre((preset.timbre * 2) << 6); // Scale to Braids range (timbre * 128)
    braidsOsc[v].set_braids_color(preset.color << 8);   // Scale to Braids range (color * 256)
    
    // Set envelope parameters
    braidsEnvelope[v].attack(preset.attack * 4.0);
    braidsEnvelope[v].decay(preset.decay * 4.0);
    braidsEnvelope[v].sustain(preset.sustain / 127.0);
    braidsEnvelope[v].release(preset.release * 4.0);
    
    // Set filter parameters
    braidsFilter[v].setLowpass(0, preset.filterFreq, preset.filterRes / 127.0 * 5.0);
  }
  
  Serial.print("Loaded Braids preset ");
  Serial.print(presetNum);
  Serial.print(": ");
  Serial.println(preset.name);
}

// Braids parameter update function
void updateBraidsParameter(int paramIndex, float value) {
  if (paramIndex < 0 || paramIndex >= NUM_BRAIDS_PARAMETERS) return;
  
  braidsParameters[paramIndex] = value;
  
  // Apply parameter changes to all voices
  for (int v = 0; v < VOICES; v++) {
    switch(paramIndex) {
      case 0: // Shape (0-47)
        braidsOsc[v].set_braids_shape((int)value);
        break;
      case 1: // Timbre (0-127)
        braidsOsc[v].set_braids_timbre((int)(value * 2) << 6); // Scale to Braids range (value * 128)
        break;
      case 2: // Color (0-127) 
        braidsOsc[v].set_braids_color((int)(value) << 8); // Scale to Braids range (value * 256) for more dramatic effect
        break;
      case 3: // Coarse (transpose)
        // Will be applied during note events
        break;
      case 4: // Amp Attack (0-127)
        braidsEnvelope[v].attack(value * 4.0);
        break;
      case 5: // Amp Decay (0-127)
        braidsEnvelope[v].decay(value * 4.0);
        break;
      case 6: // Amp Sustain (0-127)
        braidsEnvelope[v].sustain(value / 127.0);
        break;
      case 7: // Amp Release (0-127)
        braidsEnvelope[v].release(value * 4.0);
        break;
      case 8: // Filter Mode (0-3)
        // Filter mode will be applied to filter setup
        break;
      case 9: // Filter Cutoff (0-127)
        {
          float freq = 50 + (value / 127.0) * (8000 - 50); // 50Hz to 8kHz
          int mode = (int)braidsParameters[8];
          float res = braidsParameters[10] / 127.0 * 5.0; // 0 to 5.0 resonance
          if (mode == 1) braidsFilter[v].setLowpass(0, freq, res);
          else if (mode == 2) braidsFilter[v].setBandpass(0, freq, res);
          else if (mode == 3) braidsFilter[v].setHighpass(0, freq, res);
        }
        break;
      case 10: // Filter Resonance (0-127)
        {
          float freq = 50 + (braidsParameters[9] / 127.0) * (8000 - 50);
          int mode = (int)braidsParameters[8];
          float res = value / 127.0 * 5.0; // 0 to 5.0 resonance
          if (mode == 1) braidsFilter[v].setLowpass(0, freq, res);
          else if (mode == 2) braidsFilter[v].setBandpass(0, freq, res);
          else if (mode == 3) braidsFilter[v].setHighpass(0, freq, res);
        }
        break;
      case 11: // Filter Strength (0-127)
        // Filter strength will be used for VA filter routing
        break;
      case 12: // Filter Attack (0-127)
        // Filter ADSR attack time - implemented in updateBraidsFilterEnvelope()
        break;
      case 13: // Filter Decay (0-127)
        // Filter ADSR decay time - implemented in updateBraidsFilterEnvelope()
        break;
      case 14: // Filter Sustain (0-127)
        // Filter ADSR sustain level - implemented in updateBraidsFilterEnvelope()
        break;
      case 15: // Filter Release (0-127)
        // Filter ADSR release time - implemented in updateBraidsFilterEnvelope()
        break;
      case 16: // Volume (0-127)
        {
          // Apply volume to final mixer output (0-127 -> 0.0-1.0)
          float volume = value / 127.0;
          braidsFinalMixer.gain(0, volume);  // Main mixer output
          braidsFinalMixer.gain(1, volume);  // Secondary mixer output
        }
        break;
    }
  }
}

// Braids filter envelope update (called regularly like MicroDexed-touch)
void updateBraidsFilterEnvelope() {
  if (currentEngine != ENGINE_BRAIDS) return;
  
  for (int v = 0; v < VOICES; v++) {
    if (!braidsFilterEnvActive[v] || !voices[v].active) continue;
    
    unsigned long elapsedTime = millis() - braidsNoteOnTime[v];
    float baseFreq = 50 + (braidsParameters[9] / 127.0) * (8000 - 50); // Base filter cutoff
    
    // Get filter ADSR parameters (12-15)
    float attackTime = (braidsParameters[12] / 127.0) * 2000.0; // 0-2000ms attack
    float decayTime = (braidsParameters[13] / 127.0) * 2000.0;  // 0-2000ms decay
    float sustainLevel = braidsParameters[14] / 127.0;          // 0-1.0 sustain level
    
    float envelopeValue = 0.0;
    
    if (voices[v].active) {
      // Note is on - calculate ADSR envelope
      if (elapsedTime < attackTime) {
        // Attack phase
        envelopeValue = elapsedTime / attackTime;
      } else if (elapsedTime < attackTime + decayTime) {
        // Decay phase
        float decayProgress = (elapsedTime - attackTime) / decayTime;
        envelopeValue = 1.0 - (decayProgress * (1.0 - sustainLevel));
      } else {
        // Sustain phase
        envelopeValue = sustainLevel;
      }
    } else {
      // Note is off - handle release phase
      // For now, use sustain level (release implementation would need note-off tracking)
      envelopeValue = sustainLevel;
    }
    
    // Apply envelope to filter frequency with strength multiplier
    float strength = braidsParameters[11] / 127.0; // Filter strength (0-1)
    float filterRange = braidsFilterTarget[v] - baseFreq;
    braidsFilterState[v] = baseFreq + (filterRange * envelopeValue * strength);
    
    // Apply current filter state
    int mode = (int)braidsParameters[8];
    float res = braidsParameters[10] / 127.0 * 5.0;
    if (mode == 1) braidsFilter[v].setLowpass(0, braidsFilterState[v], res);
    else if (mode == 2) braidsFilter[v].setBandpass(0, braidsFilterState[v], res);  
    else if (mode == 3) braidsFilter[v].setHighpass(0, braidsFilterState[v], res);
  }
}

// Note handling with engine routing
void noteOn(int note, int velocity) {
  if (currentEngine == ENGINE_DX7) {
    // Route to DX7 engine
    dexed.keydown(note, velocity);
    return;
  }
  
  if (currentEngine == ENGINE_BRAIDS) {
    // Route to Braids engine with polyphonic voice allocation
    int voiceNum = -1;
    
    if (playMode == 0) {
      // Mono mode - use note stack but retrigger envelope for every new note
      addToMonoStack(note);
      
      // Turn off other voices if they're somehow active
      for (int v = 1; v < VOICES; v++) {
        if (voices[v].active) {
          braidsEnvelope[v].noteOff();
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
        braidsEnvelope[0].noteOff();
      }
    } else {
      // Poly mode - find an available voice (includes voice stealing if needed)
      voiceNum = findAvailableVoice();
      // If voice stealing occurred, turn off the envelope for clean transition
      if (voices[voiceNum].active) {
        braidsEnvelope[voiceNum].noteOff();
      }
    }
    
    // Set up voice
    voices[voiceNum].active = true;
    voices[voiceNum].note = note;
    voices[voiceNum].noteOnTime = millis();
    
    // Set up filter envelope (like MicroDexed-touch)
    braidsFilterEnvActive[voiceNum] = true;
    braidsNoteOnTime[voiceNum] = millis();
    float baseFreq = 50 + (braidsParameters[9] / 127.0) * (8000 - 50);
    braidsFilterTarget[voiceNum] = baseFreq * 4.0; // Target higher frequency for attack
    braidsFilterState[voiceNum] = baseFreq; // Start at base frequency
    
    // Apply coarse transpose and set pitch
    int transposedNote = note + (int)braidsParameters[3]; 
    int pitch = transposedNote << 7; // Convert MIDI note to Braids pitch format
    braidsOsc[voiceNum].set_braids_pitch(pitch);
    braidsEnvelope[voiceNum].noteOn();
    
    return;
  }
  
  
  // VA and Juno engines use the same voice management - 
  // the voice processing will drive both engines simultaneously
  // Original VA note handling code follows
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
      setVoiceFrequencies(0, baseFreq, pitchWheelMultiplier);
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
      setVoiceFrequencies(0, baseFreq, pitchWheelMultiplier);
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
      setVoiceFrequencies(voiceNum, baseFreq, pitchWheelMultiplier);
    }
    
    // Always trigger envelopes in poly mode
    ampEnv[voiceNum].noteOn();
    filtEnv[voiceNum].noteOn();
  }
}

void noteOff(int note) {
  if (currentEngine == ENGINE_DX7) {
    // Route to DX7 engine
    dexed.keyup(note);
    return;
  }
  
  if (currentEngine == ENGINE_BRAIDS) {
    // Route to Braids engine with polyphonic voice management
    if (playMode == 0) {
      // Mono mode - use note stack and retrigger envelope for next note
      removeFromMonoStack(note);
      
      // If the released note was the currently playing note
      if (voices[0].active && voices[0].note == note) {
        // Stop current envelope
        braidsEnvelope[0].noteOff();
        
        int nextNote = getTopMonoNote();
        if (nextNote != -1) {
          // Play the next note in the stack WITH envelope retrigger (mono behavior)
          voices[0].note = nextNote;
          
          // Apply coarse transpose and set pitch for next note
          int transposedNote = nextNote + (int)braidsParameters[3]; 
          int pitch = transposedNote << 7;
          braidsOsc[0].set_braids_pitch(pitch);
          
          // Retrigger envelope for the next note (mono behavior)
          braidsEnvelope[0].noteOn();
        } else {
          // No more notes - voice stays off
          voices[0].active = false;
          braidsFilterEnvActive[0] = false;
        }
      }
    } else {
      // Poly mode - find the voice with this note and turn it off
      for (int v = 0; v < VOICES; v++) {
        if (voices[v].active && voices[v].note == note) {
          braidsEnvelope[v].noteOff();
          voices[v].active = false;
          braidsFilterEnvActive[v] = false;
          break;
        }
      }
    }
    return;
  }
  
  // VA and Juno engines use the same voice management
  // Original VA note off handling
  if (playMode == 0) {
    // Mono mode - use note stack and retrigger envelope for next note
    removeFromMonoStack(note);
    
    // If the released note was the currently playing note
    if (voices[0].active && voices[0].note == note) {
      // Stop current envelope
      releaseVoiceEnvelopes(0);
      
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
          setVoiceFrequencies(0, baseFreq, pitchWheelMultiplier);
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
          setVoiceFrequencies(0, baseFreq, pitchWheelMultiplier);
        }
      } else {
        // No more notes - turn off envelopes
        releaseVoiceEnvelopes(0);
        voices[0].active = false;
      }
    }
  } 
  else {
    // Poly mode - normal note off behavior
    int voiceNum = findVoiceForNote(note);
    if (voiceNum >= 0) {
      // Turn off envelopes
      releaseVoiceEnvelopes(voiceNum);
      voices[voiceNum].active = false;
    }
  }
}

void allNotesOff() {
  // Stop all voices using helper function
  for (int v = 0; v < VOICES; v++) {
    releaseVoiceEnvelopes(v);
    voices[v].active = false;
  }
  
  // Stop DX7 voices
  dexed.panic();
}

void setup() {
  Serial.begin(9600);
  AudioMemory(60); // 48 for lower latency

  // USB Device MIDI is automatically initialized  
#ifdef ENABLE_DIN_MIDI
  // Initialize DIN MIDI
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(OnNoteOn);
  MIDI.setHandleNoteOff(OnNoteOff);
  MIDI.setHandleControlChange(OnControlChange);
  MIDI.setHandlePitchBend(OnPitchBend);
#endif

#ifdef ENABLE_TEENSY_DAC
  sgtl5000_1.enable();
  sgtl5000_1.lineOutLevel(29);
  sgtl5000_1.dacVolumeRamp();
  sgtl5000_1.dacVolume(1.0);
  sgtl5000_1.unmuteHeadphone();
  sgtl5000_1.unmuteLineout();
  sgtl5000_1.volume(0.8, 0.8); // Headphone volume
#endif
  
  // Initialize Audio
  lfo.frequency(lfoRate);
  lfo.amplitude(1.0); // Full amplitude - we'll control depth in software
  pinMode(MENU_ENCODER_SW, INPUT_PULLUP);
  
  // Initialize encoder values
  for (int i = 0; i < 20; i++) {
    encoderValues[i] = 0;
    lastEncoderValues[i] = 0;
  }
  
  // Initialize all parameter values to their current states
  for (int i = 0; i < 41; i++) {
    updateSynthParameter(i, allParameterValues[i]);
  }
  
  // Initialize Display
#ifdef USE_LCD_DISPLAY
  lcd.init();
  lcd.backlight();
  displayText("Multi-Teensy", "6-Voice Poly");
#endif

#ifdef USE_OLED_DISPLAY
  display.begin();
  Serial.println("OLED initialized");
  
  display.clearBuffer();
  display.setFont(u8g2_font_8x13_tf);
  display.drawStr(3, 20, "Multi-Teensy");
  display.drawStr(3, 40, "6-Voice Poly");
  display.sendBuffer();
#endif
  
  // Initialize all voice arrays
  for (int v = 0; v < VOICES; v++) {
    // Initialize oscillators
    osc1[v].begin(WAVEFORM_BANDLIMIT_SAWTOOTH);
    osc2[v].begin(WAVEFORM_BANDLIMIT_SAWTOOTH);
    osc3[v].begin(WAVEFORM_BANDLIMIT_SAWTOOTH);
    
    // Set initial amplitudes - oscillators always on, controlled by mixer
    osc1[v].amplitude(0.8); // Reduced amplitude
    osc2[v].amplitude(0.8);
    osc3[v].amplitude(0.8);
    
    // Configure mixer gains - reduced to prevent clipping
    oscMix[v].gain(0, vol1 * 0.8); // Reduced gain
    oscMix[v].gain(1, vol2 * 0.8); 
    oscMix[v].gain(2, vol3 * 0.8); 
    oscMix[v].gain(3, 0.0);  // Noise (controlled by noiseVol)
    
    // Configure DC source for filter envelope
    dcFilter[v].amplitude(filterStrength);
    
    // Configure filter (ladder filter like working script)
    filter1[v].frequency(cutoff);
    filter1[v].resonance(0.0); // Start with no resonance
    filter1[v].octaveControl(3.0); // Back to 3.0 like working script
    
    // Configure envelopes
    ampEnv[v].attack(ampAttack);
    ampEnv[v].sustain(ampSustain);
    ampEnv[v].decay(ampDecay);
    ampEnv[v].release(ampDecay);
    
    filtEnv[v].attack(filtAttack);
    filtEnv[v].sustain(filtSustain);
    filtEnv[v].decay(filtDecay);
    filtEnv[v].release(filtDecay);
    
    // Initialize voice state
    voices[v].note = 0;
    voices[v].active = false;
    voices[v].noteOnTime = 0;
    
    // Initialize glide state
    targetFreq[v] = 0.0;
    currentFreq[v] = 0.0;
    gliding[v] = false;
  }
  
  // Configure noise (shared across all voices)
  noise1.amplitude(0.5); // Reduced noise amplitude
  noisePink.amplitude(0.5); // Pink noise amplitude
  // Initialize noise mixer - start with white noise
  noiseMix.gain(0, 1.0); // White noise
  noiseMix.gain(1, 0.0); // Pink noise off initially
  
  // Configure voice mixers with higher gain
  voiceMix1.gain(0, 1.0); // Maximum gain for better volume
  voiceMix1.gain(1, 1.0);
  voiceMix1.gain(2, 1.0);
  voiceMix1.gain(3, 0.0);
  
  voiceMix2.gain(0, 1.0);
  voiceMix2.gain(1, 1.0);
  voiceMix2.gain(2, 1.0);
  voiceMix2.gain(3, 0.0);
  
  // Configure final mono mixer with reduced master output
  finalMix.gain(0, 0.6); // Reduced master output gain
  finalMix.gain(1, 0.6); // Reduced master output gain
  finalMix.gain(2, 0.0);
  finalMix.gain(3, 0.0);

  // Initialize DX7 FM engine
  Serial.println("Initializing DX7 FM engine...");
  dexed.setEngineType(dx7EngineType); 
  dexed.loadInitVoice();
  dexed.setTranspose(12);
  dx7BankLoaded = true;
  loadDX7Preset(0); // Load first preset
  Serial.println("DX7 initialization complete");
  
  // Initialize Braids modular synth engine (polyphonic)
  Serial.println("Initializing Braids modular synth engine (6 voices)...");
  for (int v = 0; v < VOICES; v++) {
    braidsOsc[v].init_braids();
    braidsEnvelope[v].attack(10.0);
    braidsEnvelope[v].decay(100.0);  
    braidsEnvelope[v].sustain(0.7);
    braidsEnvelope[v].release(300.0);
    braidsFilter[v].setLowpass(0, 3000, 0.7);
    
    // Initialize filter envelope state
    braidsFilterState[v] = 1000.0;
    braidsFilterTarget[v] = 1000.0; 
    braidsFilterEnvActive[v] = false;
    braidsNoteOnTime[v] = 0;
  }
  
  // Set up voice mixing - each voice should have equal contribution
  // Each voice gets 1/6 of total volume for balanced 6-voice polyphony
  float voiceGain = 1.0 / 6.0;  // ~0.167 per voice
  
  // Voices 0-3 into main mixer (4 voices)
  for (int v = 0; v < 4; v++) {
    braidsMixer.gain(v, voiceGain);
  }
  
  // Voices 4-5 into secondary mixer (2 voices) 
  braidsVoiceMixer45.gain(0, voiceGain);  // Voice 4
  braidsVoiceMixer45.gain(1, voiceGain);  // Voice 5
  
  // Final mixer combines both submixers with proper scaling
  // Since braidsMixer has 4 voices and braidsVoiceMixer45 has 2 voices,
  // we need to scale them to maintain equal per-voice contribution
  braidsFinalMixer.gain(0, 1.0);  // Main mixer (voices 0-3)
  braidsFinalMixer.gain(1, 1.0);  // Voice mixer 4-5
  
  // Apply initial Braids parameter values
  Serial.println("Setting initial Braids parameters...");
  for (int i = 0; i < NUM_BRAIDS_PARAMETERS; i++) {
    updateBraidsParameter(i, braidsParameters[i]);
  }
  
  Serial.println("Braids initialization complete");
  
  // Initialize J60 specific components
  Serial.println("Initializing J60 components...");
  
  // PWM LFO initialization
  pwmLfo.frequency(0.5);      // Slow PWM sweep by default
  pwmLfo.amplitude(1.0);
  
  // High-pass filters for Juno voices (removes low-end mud, J60 characteristic)
  for (int v = 0; v < VOICES; v++) {
    junoHPF[v].setHighpass(0, 24, 0.7); // 24Hz high-pass with slight resonance
  }
  
  // Juno voice mixers
  junoMix.gain(0, 1.0);
  junoMix.gain(1, 1.0);
  junoMix.gain(2, 1.0);
  junoMix.gain(3, 1.0);
  
  junoMix2.gain(0, 1.0);
  junoMix2.gain(1, 1.0);
  junoMix2.gain(2, 0.0);
  junoMix2.gain(3, 0.0);
  
  junoFinalMix.gain(0, 0.6);  // Main voices
  junoFinalMix.gain(1, 0.6);  // Additional voices
  junoFinalMix.gain(2, 0.0);
  junoFinalMix.gain(3, 0.0);
  
  // Shared flange effect initialization
  int s_idx = FLANGE_DELAY_LENGTH/4;     // Index into delay line
  int s_depth = FLANGE_DELAY_LENGTH/4;   // Depth of effect
  double s_freq = chorusRate;           // LFO frequency - J60 chorus speed
  
  // Initialize shared flangers with J60 chorus settings
  if(!sharedFlangeL.begin(sharedFlangeDelayBufferL, FLANGE_DELAY_LENGTH, s_idx, s_depth, s_freq)) {
    Serial.println("Shared flange left initialization failed");
  } else {
    Serial.println("Shared flange left initialized");
  }
  
  if(!sharedFlangeR.begin(sharedFlangeDelayBufferR, FLANGE_DELAY_LENGTH, s_idx, s_depth, s_freq + 0.1)) {
    Serial.println("Shared flange right initialization failed");  
  } else {
    Serial.println("Shared flange right initialized");
  }
  
  // Start with flange OFF (passthru mode) - will be enabled per preset
  sharedFlangeL.voices(FLANGE_DELAY_PASSTHRU, 0, 0);
  sharedFlangeR.voices(FLANGE_DELAY_PASSTHRU, 0, 0);
  
  
  // Initialize shared plate reverb
  Serial.println("Shared plate reverb initialized");
  sharedPlateReverb.size(0.3);      // Medium reverb time
  sharedPlateReverb.hidamp(0.5);    // Medium high frequency damping
  sharedPlateReverb.lodamp(0.0);    // No low frequency damping
  sharedPlateReverb.lowpass(0.7);   // Some filtering
  sharedPlateReverb.diffusion(0.65); // Optimal diffusion
  
  
  // Effects input mixers - VA active by default, others disabled
  effectsInputL.gain(0, 1.0); effectsInputR.gain(0, 1.0);  // VA active (default)
  effectsInputL.gain(1, 0.0); effectsInputR.gain(1, 0.0);  // Juno off
  effectsInputL.gain(2, 0.0); effectsInputR.gain(2, 0.0);  // DX7 off
  effectsInputL.gain(3, 0.0); effectsInputR.gain(3, 0.0);  // Unused
  
  // Final stereo output mixers (effects + dry mix)
  finalMixL.gain(0, 0.7);  // Effects left (enabled with mix)
  finalMixL.gain(1, 0.3);  // Dry left (reduced for effects blend)
  finalMixL.gain(2, 0.0);  // Unused
  finalMixL.gain(3, 0.0);  // Unused
  
  finalMixR.gain(0, 0.7);  // Effects right (enabled with mix)
  finalMixR.gain(1, 0.3);  // Dry right (reduced for effects blend)
  finalMixR.gain(2, 0.0);  // Unused
  finalMixR.gain(3, 0.0);  // Unused
  
  Serial.println("J60 components initialized");
  
  // Set engine mixing levels - VA active by default
  engineMix.gain(0, 1.0);  // VA engine (active)
  engineMix.gain(1, 0.0);  // Juno engine (off)
  engineMix.gain(2, 0.0);  // DX7 engine (off)
  
  delay(2000);
  updateDisplay();
  Serial.println("Multi-Engine Synthesizer Ready!");

  loadPreset(0);
}


// Read all direct encoders
void readDirectEncoders() {
#ifndef MINIMAL_BUILD
  encoderValues[0] = enc1.read() / 4;     // Osc1 Range
  encoderValues[1] = enc2.read() / 4;     // Osc2 Range
  encoderValues[2] = enc3.read() / 4;     // Osc3 Range
  encoderValues[3] = enc4.read() / 4;     // Osc2 Fine
  encoderValues[4] = enc5.read() / 4;     // Osc3 Fine
  encoderValues[5] = enc6.read() / 4;     // Osc1 Wave
  encoderValues[6] = enc7.read() / 4;     // Osc2 Wave
  encoderValues[7] = enc8.read() / 4;     // Osc3 Wave
  encoderValues[8] = enc9.read() / 4;     // Volume 1
  encoderValues[9] = enc10.read() / 4;    // Volume 2
  encoderValues[10] = enc11.read() / 4;   // Volume 3
  // encoderValues[11] handled separately for cutoff (uses menuEncoder hardware)
  encoderValues[12] = enc13.read() / 4;   // Resonance
  encoderValues[13] = enc14.read() / 4;   // Filter Attack
  encoderValues[14] = enc15.read() / 4;   // Filter Decay/Release
  encoderValues[15] = enc16.read() / 4;   // Filter Sustain
  encoderValues[16] = enc17.read() / 4;   // Noise Volume
  encoderValues[17] = enc18.read() / 4;   // Amp Attack
  encoderValues[18] = enc19.read() / 4;   // Amp Sustain
  encoderValues[19] = enc20.read() / 4;   // Amp Decay
#endif
}


void readAllControls() {
  // Read all encoders
  readDirectEncoders();
  
#ifndef MINIMAL_BUILD
  // Check for encoder changes and update parameters
  for (int i = 0; i < 20; i++) {
    if (encoderValues[i] != lastEncoderValues[i]) {
      // If any physical knob is turned, exit menu mode
      if (inMenu) {
        inMenu = false;
      }
      
      int change = encoderValues[i] - lastEncoderValues[i];
      
      int targetParam = i;
      if (macroMode && i == 13) targetParam = 22;  // Filter Attack -> LFO Rate  
      if (macroMode && i == 14) targetParam = 23;  // Filter Decay -> LFO Depth
      if (macroMode && i == 15) targetParam = 25;  // Filter Release -> LFO Target
      
      updateEncoderParameter(targetParam, change);
      lastEncoderValues[i] = encoderValues[i];
    }
  }
#endif
}

// Core synthesis parameter update function
// Apply J60 specific parameter mapping
void applyJunoParameterMapping() {
  if (currentEngine != ENGINE_JUNO) return;
  
  // J60 oscillator configuration
  // DCO (Digital Controlled Oscillator) setup
  // Osc1 = Main DCO (PWM or Sawtooth)
  // Osc2 = Sub-oscillator (Square wave, -1 octave)  
  // Osc3 = Additional oscillator for noise/effects
  
  // Force J60 appropriate ranges
  osc1Range = 1.0;  // No range control on J60
  osc2Range = 0.5;  // Sub-oscillator is always -1 octave
  osc3Range = 1.0;  // Standard range
  
  // Force J60 fine tune off
  osc1Fine = 1.0;   // No fine tune on J60
  osc2Fine = 1.0;   // No fine tune on J60
  osc3Fine = 1.0;   // No fine tune on J60
  
  // J60 authentic DCO behavior:
  // DCO Section: PWM OR Sawtooth (not both)
  // Sub-oscillator: Square wave -1 octave (independent)
  
  // J60 DCO waveform selection logic
  // Use osc1 and osc2 volumes to determine which DCO waveform is active
  if (vol1 > 0.0 && vol2 > 0.0) {
    // Both volumes up - need to choose one DCO waveform
    // Use osc1Wave to determine DCO selection
    if (osc1Wave == WAVEFORM_SAWTOOTH || osc1Wave == WAVEFORM_BANDLIMIT_SAWTOOTH) {
      // Sawtooth selected - disable PWM, enable saw + sub
      osc1Wave = WAVEFORM_BANDLIMIT_SAWTOOTH;
      osc2Wave = WAVEFORM_SQUARE;  // Sub-oscillator
      vol1 = vol1;  // Keep sawtooth volume
      vol2 = vol2;  // Keep sub volume
    } else {
      // PWM selected - disable saw, enable PWM + sub  
      osc1Wave = WAVEFORM_BANDLIMIT_PULSE;
      osc2Wave = WAVEFORM_SQUARE;  // Sub-oscillator
      vol1 = vol1;  // Keep PWM volume
      vol2 = vol2;  // Keep sub volume
    }
  } else if (vol1 > 0.0) {
    // Only main DCO active
    if (osc1Wave == WAVEFORM_SAWTOOTH || osc1Wave == WAVEFORM_BANDLIMIT_SAWTOOTH) {
      osc1Wave = WAVEFORM_BANDLIMIT_SAWTOOTH;
    } else {
      osc1Wave = WAVEFORM_BANDLIMIT_PULSE;
    }
    osc2Wave = WAVEFORM_SQUARE;  // Sub always square when used
  } else if (vol2 > 0.0) {
    // Only sub-oscillator active
    osc1Wave = WAVEFORM_BANDLIMIT_PULSE;  // DCO off but need valid waveform
    osc2Wave = WAVEFORM_SQUARE;  // Sub-oscillator
  }
  
  // Force osc3 off for authentic 2-oscillator J60 behavior
  vol3 = 0.0;
  osc3Wave = WAVEFORM_SQUARE;
  
  // J60 envelope: Single ADSR shared by filter and amp
  // Use filter envelope parameters for both filter and amp
  ampAttack = filtAttack;
  ampDecay = filtDecay; 
  ampSustain = filtSustain;
  ampRelease = filtRelease;
  
  // Apply to all active voices
  updateOscillatorFrequencies();
  updateEnvelopes();
}

void updateSynthParameter(int paramIndex, float val) {
  // Convert to appropriate ranges and update parameters
  switch (paramIndex) {
    case 0: // Osc1 Range
      osc1Range = getOscillatorRange(val);
      updateOscillatorFrequencies();
      break;
    case 1: // Osc2 Range  
      osc2Range = getOscillatorRange(val);
      updateOscillatorFrequencies();
      break;
    case 2: // Osc3 Range
      osc3Range = getOscillatorRange(val);
      updateOscillatorFrequencies();
      break;
    case 3: {// Osc2 Fine
      // Extended fine tuning: ±25 cents, then ±12 semitones
      float totalCents;
      if (val <= 0.25) {
        // -12 to -1 semitones (left range)
        float semiRange = val / 0.25; // 0.0 to 1.0
        totalCents = -1200 + (semiRange * 1175); // -1200 to -25 cents
      } else if (val >= 0.75) {
        // +1 to +12 semitones (right range)
        float semiRange = (val - 0.75) / 0.25; // 0.0 to 1.0
        totalCents = 25 + (semiRange * 1175); // +25 to +1200 cents
      } else {
        // ±25 cents fine tuning (center range)
        totalCents = (val - 0.5) * 100.0; // -25 to +25 cents
      }
      osc2Fine = pow(2.0, totalCents / 1200.0);
      updateOscillatorFrequencies();
      break;}
    case 4: {// Osc3 Fine
      // Extended fine tuning: ±25 cents, then ±12 semitones
      float totalCents;
      if (val <= 0.25) {
        // -12 to -1 semitones (left range)
        float semiRange = val / 0.25; // 0.0 to 1.0
        totalCents = -1200 + (semiRange * 1175); // -1200 to -25 cents
      } else if (val >= 0.75) {
        // +1 to +12 semitones (right range)
        float semiRange = (val - 0.75) / 0.25; // 0.0 to 1.0
        totalCents = 25 + (semiRange * 1175); // +25 to +1200 cents
      } else {
        // ±25 cents fine tuning (center range)
        totalCents = (val - 0.5) * 100.0; // -25 to +25 cents
      }
      osc3Fine = pow(2.0, totalCents / 1200.0);
      updateOscillatorFrequencies();
      break;}
    case 5: // Osc1 Wave
      osc1Wave = getMiniTeensyWaveform(val, 1);
      updateWaveforms();
      break;
    case 6: // Osc2 Wave
      osc2Wave = getMiniTeensyWaveform(val, 2);
      updateWaveforms();
      break;
    case 7: // Osc3 Wave
      osc3Wave = getMiniTeensyWaveform(val, 3);
      updateWaveforms();
      break;
    case 8: // Volume 1
      vol1 = val * 0.8; // Increased gain from 0.4 to 0.8
      updateMixerLevels();
      break;
    case 9: // Volume 2
      vol2 = val * 0.8; // Increased gain from 0.4 to 0.8
      updateMixerLevels();
      break;
    case 10: // Volume 3
      vol3 = val * 0.8; // Increased gain from 0.4 to 0.8
      updateMixerLevels();
      break;
    case 11: // Cutoff
      // Logarithmic frequency response like analog synth (20Hz to 20kHz)
      cutoff = 20 * pow(1000.0, val); // 20Hz to 20kHz logarithmic
      for (int v = 0; v < VOICES; v++) {
        filter1[v].frequency(cutoff);
      }
      break;
    case 12: // Resonance
      resonance = val * 3.0;
      for (int v = 0; v < VOICES; v++) {
        filter1[v].resonance(resonance);
      }
      break;
    case 13: // Filter Attack
      filtAttack = 1 + val * 3000;
      updateEnvelopes();
      break;
    case 14: // Filter Decay/Release
      filtDecay = 10 + val * 5000;
      updateEnvelopes();
      break;
    case 15: // Filter Sustain
      filtSustain = val;
      updateEnvelopes();
      break;
    case 16: // Noise Volume (menu-only)
      noiseVol = val * 0.6; // Increased gain from 0.3 to 0.6
      updateNoiseLevel();
      break;
    case 17: // Amp Attack (menu-only)
      ampAttack = 1 + val * 3000;
      updateEnvelopes();
      break;
    case 18: // Amp Sustain (menu-only)
      ampSustain = val;
      updateEnvelopes();
      break;
    case 19: // Amp Decay (menu-only)
      ampDecay = 10 + val * 5000;
      updateEnvelopes();
      break;
    case 20: { // Osc1 Fine Tune (menu-only) - Extended fine tuning: ±25 cents, then ±12 semitones
      float totalCents;
      if (val <= 0.25) {
        // -12 to -1 semitones (left range)
        float semiRange = val / 0.25; // 0.0 to 1.0
        totalCents = -1200 + (semiRange * 1175); // -1200 to -25 cents
      } else if (val >= 0.75) {
        // +1 to +12 semitones (right range)
        float semiRange = (val - 0.75) / 0.25; // 0.0 to 1.0
        totalCents = 25 + (semiRange * 1175); // +25 to +1200 cents
      } else {
        // ±25 cents fine tuning (center range)
        totalCents = (val - 0.5) * 100.0; // -25 to +25 cents
      }
      osc1Fine = pow(2.0, totalCents / 1200.0);
      updateOscillatorFrequencies();
      break;
    }
    case 21: // Filter Strength (menu-only)
      filterStrength = val; // 0.0 to 1.0 envelope modulation amount
      // Update all voices immediately
      for (int v = 0; v < VOICES; v++) {
        dcFilter[v].amplitude(filterStrength);
      }
      break;
    case 22: // LFO Rate (menu-only)
      lfoRate = 0.1 + val * 19.9; // 0.1 to 20 Hz
      lfo.frequency(lfoRate);
      break;
    case 23: // LFO Depth (menu-only)
      lfoDepth = val; // 0.0 to 1.0
      break;
    case 24: // LFO Toggle (menu-only)
      lfoEnabled = (val > 0.5); // Toggle at 50%
      break;
    case 25: // LFO Target (menu-only)
      if (val < 0.33) lfoTarget = 0; // Pitch
      else if (val < 0.66) lfoTarget = 1; // Filter
      else lfoTarget = 2; // Amp
      break;
    case 26: // Play Mode (menu-only)
      if (val < 0.33) playMode = 0; // Mono
      else if (val < 0.66) playMode = 1; // Poly
      else playMode = 2; // Legato
      break;
    case 27: // Glide Time (menu-only)
      glideTime = val; // 0.0 to 1.0 (0 = off, 0.1-1.0 = 100ms to 10s)
      break;
    case 28: // Noise Type (menu-only)
      noiseType = (val > 0.5) ? 1 : 0; // 0 = White, 1 = Pink
      if (noiseType == 0) {
        noiseMix.gain(0, 1.0); // White noise on
        noiseMix.gain(1, 0.0); // Pink noise off
      } else {
        noiseMix.gain(0, 0.0); // White noise off
        noiseMix.gain(1, 1.0); // Pink noise on
      }
      break;
    case 29: // Macro Mode (menu-only)
      macroMode = (val > 0.5); // Toggle at 50%
      break;
    case 30: // MIDI Channel
      midiChannel = (int)(val * 16.0); // 0-16 (0 = omni, 1-16 = channels)
      break;
    case 31: // Chorus Bypass
      chorusBypass = (val > 0.5); // Toggle at 50%
      if (chorusBypass) {
        // Bypass: Set flanger to passthrough
        sharedFlangeL.voices(FLANGE_DELAY_PASSTHRU, 0, 0);
        sharedFlangeR.voices(FLANGE_DELAY_PASSTHRU, 0, 0);
      } else {
        // Active: Restore chorus settings
        int s_idx = FLANGE_DELAY_LENGTH/4;     
        int s_depth = (int)(chorusDepth * FLANGE_DELAY_LENGTH/4);
        sharedFlangeL.voices(s_idx, s_depth, chorusRate);
        sharedFlangeR.voices(s_idx, s_depth, chorusRate + 0.1); // Slight offset for stereo
      }
      updateEffectsMix(); // Update overall effects routing
      break;
    case 32: // Chorus Rate
      chorusRate = val; // 0-1
      if (!chorusBypass) {
        int s_idx = FLANGE_DELAY_LENGTH/4;     
        int s_depth = (int)(chorusDepth * FLANGE_DELAY_LENGTH/4);
        sharedFlangeL.voices(s_idx, s_depth, chorusRate);
        sharedFlangeR.voices(s_idx, s_depth, chorusRate + 0.1);
      }
      break;
    case 33: // Chorus Depth
      chorusDepth = val; // 0-1
      if (!chorusBypass) {
        int s_idx = FLANGE_DELAY_LENGTH/4;     
        int s_depth = (int)(chorusDepth * FLANGE_DELAY_LENGTH/4);
        sharedFlangeL.voices(s_idx, s_depth, chorusRate);
        sharedFlangeR.voices(s_idx, s_depth, chorusRate + 0.1);
      }
      break;
    case 34: // Reverb Bypass
      reverbBypass = (val > 0.5); // Toggle at 50%
      updateEffectsMix(); // Update overall effects routing
      break;
    case 35: // Reverb Size
      reverbSize = val; // 0-1
      sharedPlateReverb.size(val);
      break;
    case 36: // Reverb High Damping
      reverbHidamp = val; // 0-1
      sharedPlateReverb.hidamp(val);
      break;
    case 37: // Reverb Low Damping
      reverbLodamp = val; // 0-1
      sharedPlateReverb.lodamp(val);
      break;
    case 38: // Reverb Lowpass
      reverbLowpass = val; // 0-1
      sharedPlateReverb.lowpass(val);
      break;
    case 39: // Reverb Diffusion
      reverbDiffusion = val; // 0-1
      sharedPlateReverb.diffusion(val);
      break;
  }
}

// Update effects routing based on individual bypass states
void updateEffectsMix() {
  // Determine if any effects are active
  bool anyEffectsActive = !chorusBypass || !reverbBypass;
  
  if (anyEffectsActive) {
    // At least one effect is active - use effects chain
    // Increased gains to compensate for effects processing volume loss
    finalMixL.gain(0, 0.9); finalMixR.gain(0, 0.9);  // Effects mix (increased)
    finalMixL.gain(1, 0.4); finalMixR.gain(1, 0.4);  // Dry mix (increased)
  } else {
    // All effects bypassed - dry signal only
    finalMixL.gain(0, 0.0); finalMixR.gain(0, 0.0);  // Effects off
    finalMixL.gain(1, 1.0); finalMixR.gain(1, 1.0);  // Dry signal only
  }
}

// Menu-based parameter update function moved to MenuNavigation.cpp

// Encoder-based parameter update function moved to MenuNavigation.cpp

// Oscillator range and waveform utility functions moved to MenuNavigation.cpp

void updateOscillatorFrequencies() {
  // Update all active voices
  for (int v = 0; v < VOICES; v++) {
    if (voices[v].active) {
      float baseFreq = 440.0 * pow(2.0, (voices[v].note - 69) / 12.0);
      float pitchWheelMultiplier = pow(2.0, pitchWheelValue * 2.0 / 12.0);
      setVoiceFrequencies(v, baseFreq, pitchWheelMultiplier);
    }
  }
}


void updateMixerLevels() {
  // Update mixer levels for all voices
  for (int v = 0; v < VOICES; v++) {
    oscMix[v].gain(0, vol1 * 1.0); // Full volume levels
    oscMix[v].gain(1, vol2 * 1.0);
    oscMix[v].gain(2, vol3 * 1.0);
  }
}

void updateNoiseLevel() {
  // Update noise level for all voices
  for (int v = 0; v < VOICES; v++) {
    oscMix[v].gain(3, noiseVol);
  }
}

void updateWaveforms() {
  // Update waveforms for all voices
  for (int v = 0; v < VOICES; v++) {
    osc1[v].begin(osc1Wave);
    osc2[v].begin(osc2Wave);
    osc3[v].begin(osc3Wave);
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



// All menu navigation functions moved to MenuNavigation.cpp

void loop() {
  // Process ALL USB Device MIDI messages immediately for minimal latency
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
  
#ifdef ENABLE_DIN_MIDI
  // Process DIN MIDI messages
  MIDI.read();
#endif
  
  // Read controls and update synth
  readAllControls();
  
  // Handle encoder
  handleEncoder();
  
  // Update LFO filter modulation
  updateLFOModulation();
  
  // Update glide/portamento
  updateGlide();
  
  // Update Braids filter envelope (dynamic filter sweeps)
  updateBraidsFilterEnvelope();
  
  // Minimal serial input check for performance
  if (Serial.available()) {
    char input = Serial.read();
    if (input == 'r' || input == 'R') {
      resetEncoderBaselines();
    }
  }
  
  delay(5); // Reduced delay for better responsiveness
}