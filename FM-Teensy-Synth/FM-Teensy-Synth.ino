/*
 * FM-Teensy Synth v1.0
 * A polyphonic FM synthesizer built with the Teensy 4.1 microcontroller,
 * using the Dexed DX7-compatible sound engine with intuitive menu control.
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

#define NUM_PARAMETERS 10  // FM has 10 essential real-time parameters
#define NUM_PRESETS 32     // DX7 has 32 presets per bank
#define VOICES 16          // DX7 supports up to 16 voices

#include "config.h"
#include "MenuNavigation.h"

// Project strings
const char* PROJECT_NAME = "FM Synth";
const char* PROJECT_SUBTITLE = "DX7 Compatible";

#include <USBHost_t36.h>
#include <Audio.h>
#include <Wire.h>
#include <Encoder.h>

// DX7/Dexed FM Synthesis Engine
#define DX7_IMPLEMENTATION
#include "src/Synth_Dexed/synth_dexed.h"
#include "dx7_roms_unpacked.h"

#ifdef USE_LCD_DISPLAY
  #include <LiquidCrystal_I2C.h>
#endif

#ifdef USE_OLED_DISPLAY
  #include <U8g2lib.h>
  #define OLED_WIDTH 128
  #define OLED_HEIGHT 64
  #define OLED_RESET -1
#endif

// ============================================================================
// MIDI Setup
// ============================================================================

#ifdef USE_MIDI_HOST
USBHost myusb;
USBHub hub1(myusb);
MIDIDevice midi1(myusb);
#endif

#ifdef USE_DIN_MIDI
#include <MIDI.h>
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);
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

// DX7 bank/patch selection
int currentDX7Preset = 0;  // Currently loaded preset (0-31)
int currentDX7Bank = 0;    // Currently loaded bank (0-7)
int dx7BankIndex = 0;      // Bank browser index
int dx7PatchIndex = 0;     // Patch browser index

// Default parameter values for FM synthesis (0.0-1.0 normalized)
float allParameterValues[NUM_PARAMETERS] = {
  0.400, // Algorithm (0-31, normalized to 0.0-1.0)
  0.000, // Feedback (0-7)
  0.400, // LFO Speed (0-99)
  0.300, // Master Volume (0.0-1.0+, normalized gain) - 30% = 0.6 gain (matches Multi-Teensy)
  0.700, // OP1 Output Level (0-99)
  0.500, // OP2 Output Level (0-99)
  0.300, // OP3 Output Level (0-99)
  0.200, // OP4 Output Level (0-99)
  0.100, // OP5 Output Level (0-99)
  0.050  // OP6 Output Level (0-99)
};

// FM synthesis objects
AudioSynthDexed       dexed(VOICES, AUDIO_SAMPLE_RATE);  // DX7-compatible FM synthesis with 16 voices

#ifdef USE_USB_AUDIO
AudioOutputUSB        usb1;            // USB audio output (stereo)
#endif

#ifdef USE_TEENSY_DAC
AudioOutputI2S        i2s1;            // I2S DAC output (Teensy Audio Shield)
AudioControlSGTL5000  sgtl5000_1;
#endif

// Audio connections
#ifdef USE_USB_AUDIO
AudioConnection patchCord1(dexed, 0, usb1, 0); // Left channel
AudioConnection patchCord2(dexed, 0, usb1, 1); // Right channel (mono to stereo)
#endif

#ifdef USE_TEENSY_DAC
AudioConnection patchCord3(dexed, 0, i2s1, 0); // Left channel
AudioConnection patchCord4(dexed, 0, i2s1, 1); // Right channel (mono to stereo)
#endif

// Control parameter names for menu display
const char* controlNames[NUM_PARAMETERS] = {
  "Algorithm", "Feedback", "LFO Speed", "Master Vol", "OP1 Level", 
  "OP2 Level", "OP3 Level", "OP4 Level", "OP5 Level", "OP6 Level"
};

bool macroMode = false;

MenuState currentMenuState = PARENT_MENU;
bool inMenu = false;
int menuIndex = 0;

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

// Display update tracking for MIDI CC changes
int lastChangedParam = -1;
float lastChangedValue = 0.0;
String lastChangedName = "";
bool parameterChanged = false;



// ============================================================================
// Centralized MIDI Handler
// ============================================================================

void processMidiMessage(byte type, byte channel, byte data1, byte data2) {
  // Filter by MIDI channel (0 = omni, 1-16 = specific channel)
  if (midiChannel != 0 && channel != midiChannel) return;
  
  switch (type) {
    case 0x90: // Note On (or Note Off with velocity 0)
      if (data2 > 0) {
        dexed.keydown(data1, data2);
      } else {
        dexed.keyup(data1);
      }
      break;
      
    case 0x80: // Note Off
      dexed.keyup(data1);
      break;
      
    case 0xB0: // Control Change
      handleControlChange(data1, data2);
      break;
      
    case 0xC0: // Program Change
      handleProgramChange(data1);
      break;
      
    case 0xE0: // Pitch Bend
      {
        int pitchBendValue = (data2 << 7) | data1; // Combine MSB and LSB
        pitchWheelValue = (pitchBendValue - 8192) / 8192.0;
        dexed.setPitchbend((uint16_t)pitchBendValue);
      }
      break;
  }
}

void handleControlChange(int cc, int value) {
  // Convert MIDI value (0-127) to parameter value (0.0-1.0)
  float paramValue = value / 127.0;
  
  // Handle standard MIDI CCs first
  if (cc == CC_MODWHEEL) {
    modWheelValue = paramValue;
    dexed.setModWheel(value);
    
    // Track mod wheel change for display
    if (!inMenu) {
      lastChangedParam = -1;  // Special flag for non-parameter controls
      lastChangedName = "Mod Wheel";
      lastChangedValue = value;  // Use raw 0-127 value for display
      parameterChanged = true;
    }
    return;
  }
  
  // Handle FM parameter CCs
  int paramIndex = -1;
  
  // if (cc == CC_ALGORITHM) paramIndex = 0;
  // else if (cc == CC_FEEDBACK) paramIndex = 1;
  // else if (cc == CC_LFO_SPEED) paramIndex = 2;
  // else if (cc == CC_MASTER_VOL) paramIndex = 3;
  // else if (cc == CC_OP1_LEVEL) paramIndex = 4;
  // else if (cc == CC_OP2_LEVEL) paramIndex = 5;
  // else if (cc == CC_OP3_LEVEL) paramIndex = 6;
  // else if (cc == CC_OP4_LEVEL) paramIndex = 7;
  // else if (cc == CC_OP5_LEVEL) paramIndex = 8;
  // else if (cc == CC_OP6_LEVEL) paramIndex = 9;

  if (cc == CC_1_PARAM) paramIndex = ENC_1_PARAM;          // ALGORITHM
  else if (cc == CC_2_PARAM) paramIndex = ENC_2_PARAM;     // FEEDBACK
  else if (cc == CC_3_PARAM) paramIndex = ENC_3_PARAM;     // LFO_SPEED
  else if (cc == CC_4_PARAM) paramIndex = ENC_4_PARAM;     // MASTER_VOL
  else if (cc == CC_5_PARAM) paramIndex = ENC_5_PARAM;     // OP1_LEVEL
  else if (cc == CC_6_PARAM) paramIndex = ENC_6_PARAM;     // OP2_LEVEL
  else if (cc == CC_7_PARAM) paramIndex = ENC_7_PARAM;     // OP3_LEVEL
  else if (cc == CC_8_PARAM) paramIndex = ENC_8_PARAM;     // OP4_LEVEL
  else if (cc == CC_9_PARAM) paramIndex = ENC_9_PARAM;     // OP5_LEVEL 
  else if (cc == CC_10_PARAM) paramIndex = ENC_10_PARAM;   // OP6_LEVEL 
  
  // Update parameter if mapped
  if (paramIndex >= 0) {
    allParameterValues[paramIndex] = paramValue;
    updateParameterFromMenu(paramIndex, paramValue);
    
    // Track parameter change for display
    if (!inMenu) {
      lastChangedParam = paramIndex;
      lastChangedValue = paramValue;
      lastChangedName = controlNames[paramIndex];
      parameterChanged = true;
    }
  }
}

void handleProgramChange(int program) {
  // Map program change 0-127 to FM bank/patches
  int bankIndex = (program / 32) % 8;    // 8 banks available (0-7)
  int patchIndex = program % 32;         // 32 patches per bank (0-31)
  
  loadDX7Preset(program); 
  Serial.print("Program change to bank: ");
  Serial.print(bankIndex);
  Serial.print(" patch: ");
  Serial.println(patchIndex);
}

void setup() {
  Serial.begin(115200);
  
  // Audio setup
  AudioMemory(40);
  
#ifdef USE_TEENSY_DAC
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.8);
  Serial.println("Teensy Audio Shield initialized");
#endif

#ifdef USE_MIDI_HOST
  myusb.begin();
  Serial.println("USB Host MIDI initialized");
#endif

#ifdef USE_DIN_MIDI
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn([](byte channel, byte note, byte velocity) {
    processMidiMessage(0x90, channel, note, velocity);
  });
  MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity) {
    processMidiMessage(0x80, channel, note, velocity);
  });
  MIDI.setHandleControlChange([](byte channel, byte cc, byte value) {
    processMidiMessage(0xB0, channel, cc, value);
  });
  MIDI.setHandleProgramChange([](byte channel, byte program) {
    processMidiMessage(0xC0, channel, program, 0);
  });
  MIDI.setHandlePitchBend([](byte channel, int bend) {
    byte data1 = bend & 0x7F;       // LSB
    byte data2 = (bend >> 7) & 0x7F; // MSB
    processMidiMessage(0xE0, channel, data1, data2);
  });
  Serial.println("DIN MIDI initialized");
#endif
  
  pinMode(MENU_ENCODER_SW, INPUT_PULLUP);
  
  // Initialize encoder values
  for (int i = 0; i < 19; i++) {
    encoderValues[i] = 0;
    lastEncoderValues[i] = 0;
  }
  
  // Initialize DX7 FM synthesis engine
  Serial.println("Initializing DX7 FM synthesis engine...");
  dexed.setEngineType(0); // MSFA engine (warmest, no pops)
  dexed.loadInitVoice();
  dexed.setTranspose(12); // Center at middle C
  
  // Initialize FM parameters
  for (int i = 0; i < NUM_PARAMETERS; i++) {
    updateSynthParameter(i, allParameterValues[i]);
  }

  // Load first preset from ROM (bank 0, patch 0)
  currentDX7Bank = 0;
  currentDX7Preset = 0;
  loadDX7Preset(0);
  
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
  
  Serial.println("FM-Teensy Synth initialized");
  Serial.println("Parameters: 10 FM parameters");
  
#ifdef USE_USB_AUDIO
  Serial.println("Audio output: USB Audio");
#endif
#ifdef USE_TEENSY_DAC
  Serial.println("Audio output: Teensy Audio Shield (I2S)");
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
  
  // Handle menu encoder parameter control (if configured)
  // This is handled separately from the 19 main encoders
  static long lastMenuEncoderValue = 0;
  long currentMenuEncoderValue = 0;
  
#ifdef USE_OLED_DISPLAY
  currentMenuEncoderValue = menuEncoder.read() / 4; // Adjusted sensitivity for OLED encoder
#else
  currentMenuEncoderValue = menuEncoder.read() / 2; // Standard sensitivity for separate encoder
#endif
  
  if (!inMenu && MENU_ENCODER_PARAM >= 0 && MENU_ENCODER_PARAM < NUM_PARAMETERS && currentMenuEncoderValue != lastMenuEncoderValue) {
    int change = currentMenuEncoderValue - lastMenuEncoderValue;
    updateEncoderParameter(MENU_ENCODER_PARAM, change);
    lastMenuEncoderValue = currentMenuEncoderValue;
  }
}

void updateSynthParameter(int paramIndex, float val) {
  // Update FM synthesis parameters
  switch (paramIndex) {
    case 0: // Algorithm (0-31)
      dexed.setAlgorithm((uint8_t)(val * 31));
      break;
    case 1: // Feedback (0-7)
      dexed.setFeedback((uint8_t)(val * 7));
      break;
    case 2: // LFO Speed (0-99)
      dexed.setLFOSpeed((uint8_t)(val * 99));
      break;
    case 3: // Master Volume (0.0-2.0, allowing boost)
      dexed.setGain(val * 2.0f);
      break;
    case 4: // OP1 Output Level (0-99)
      dexed.setOPOutputLevel(0, (uint8_t)(val * 99));
      break;
    case 5: // OP2 Output Level (0-99)
      dexed.setOPOutputLevel(1, (uint8_t)(val * 99));
      break;
    case 6: // OP3 Output Level (0-99)
      dexed.setOPOutputLevel(2, (uint8_t)(val * 99));
      break;
    case 7: // OP4 Output Level (0-99)
      dexed.setOPOutputLevel(3, (uint8_t)(val * 99));
      break;
    case 8: // OP5 Output Level (0-99)
      dexed.setOPOutputLevel(4, (uint8_t)(val * 99));
      break;
    case 9: // OP6 Output Level (0-99)
      dexed.setOPOutputLevel(5, (uint8_t)(val * 99));
      break;
  }
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

void loop() {
  // Process USB Device MIDI messages (if enabled)
#ifdef USE_USB_DEVICE_MIDI
  while (usbMIDI.read()) {
    processMidiMessage(usbMIDI.getType(), usbMIDI.getChannel(), 
                      usbMIDI.getData1(), usbMIDI.getData2());
  }
#endif

#ifdef USE_MIDI_HOST
  myusb.Task();
  while (midi1.read()) {
    processMidiMessage(midi1.getType(), midi1.getChannel(), 
                      midi1.getData1(), midi1.getData2());
  }
#endif
  
#ifdef ENABLE_DIN_MIDI
  MIDI.read();
#endif
  
  readAllControls();
  handleEncoder();
  
  // Update display if parameter changed during this loop iteration
  if (parameterChanged && !inMenu) {
    String line2 = "";
    
    if (lastChangedParam >= 0) {
      // Special display formatting for certain parameters
      if (lastChangedParam == 0) { // Algorithm (0-31)
        int algNum = (int)(lastChangedValue * 31);
        line2 = "Alg " + String(algNum + 1);
      } else {
        int displayValue = (int)(lastChangedValue * 127); // 0-127 scale
        line2 = String(displayValue);
      }
    } else {
      // For mod wheel and other non-parameter controls
      line2 = String((int)lastChangedValue);
    }
    
    displayText(lastChangedName, line2);
    parameterChanged = false;
  }
  
  // Minimal serial input check for performance
  if (Serial.available()) {
    char input = Serial.read();
    if (input == 'r' || input == 'R') {
      resetEncoderBaselines();
    }
  }
  delay(5);
}