/*
 * EPiano-Teensy Synth v1.0
 * A 16-voice polyphonic electric piano synthesizer built with the Teensy 4.1 microcontroller, 
 * featuring the MDA EPiano synthesis engine with real-time parameter control.
 * 
 * REQUIRED LIBRARIES (install via Arduino Library Manager):
 * - LiquidCrystal I2C (by Frank de Brabander) - for LCD display
 * - Adafruit SSD1306 (by Adafruit) - for OLED display  
 * - Adafruit GFX Library (by Adafruit) - for OLED display
 * - Encoder (by Paul Stoffregen)
 * - MIDI Library (by Francois Best) - only needed if enabling DIN MIDI
 */

#define NUM_PARAMETERS 12
#define VOICES 16

#include "config.h"
#include "MenuNavigation.h"

const char* PROJECT_NAME = "EPiano Synth";
const char* PROJECT_SUBTITLE = "16-Voice MDA";

#include <USBHost_t36.h>
#include <Audio.h>
#include <Wire.h>
#include <Encoder.h>
#include "src/synth_mda_epiano.h"

// ============================================================================
// Display Setup
// ============================================================================

#ifdef USE_LCD_DISPLAY
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
#endif

#ifdef USE_OLED_DISPLAY
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif

// ============================================================================
// Audio Setup
// ============================================================================

AudioSynthEPiano ep(VOICES);    // 16-voice EPiano
AudioOutputUSB usb1;            // USB audio output (stereo)
AudioConnection patchCord1(ep, 0, usb1, 0); // Left channel
AudioConnection patchCord2(ep, 1, usb1, 1); // Right channel

// ============================================================================
// Encoder Setup
// ============================================================================

Encoder menuEncoder(MENU_ENCODER_CLK, MENU_ENCODER_DT);

// Encoder arrays (21 elements to accommodate indices 0-20)
Encoder* encoders[21];
long encoderPositions[21] = {0};
bool encoderBaselines[21] = {false};

// ============================================================================
// MIDI Setup
// ============================================================================

#ifdef USE_MIDI_HOST
USBHost myusb;
USBHub hub1(myusb);
MIDIDevice midi1(myusb);
#endif

#ifdef ENABLE_DIN_MIDI
#include <MIDI.h>
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);
#endif

// ============================================================================
// Parameter Storage
// ============================================================================

float allParameterValues[NUM_PARAMETERS];
int currentPreset = 0;
int midiChannel = 0; // 0 = omni, 1-16 = specific channel

// Parameter names for display (declared in MenuNavigation.cpp)
extern const char* controlNames[];

// ============================================================================
// Setup Functions
// ============================================================================

void setup() {
  Serial.begin(115200);
  
  // Audio setup
  AudioMemory(50);
  
  // Set EPiano volume (not a parameter, just level)
  ep.setVolume(1.0);
  
  // Initialize parameter values to match EPiano defaults
  allParameterValues[0] = 0.5;   // Decay
  allParameterValues[1] = 0.5;   // Release
  allParameterValues[2] = 0.5;   // Hardness
  allParameterValues[3] = 0.5;   // Treble
  allParameterValues[4] = 0.5;   // Pan/Tremolo
  allParameterValues[5] = 0.65;  // LFO Rate
  allParameterValues[6] = 0.25;  // Velocity Sense
  allParameterValues[7] = 0.5;   // Stereo
  allParameterValues[8] = 1.0;   // Polyphony (16 voices)
  allParameterValues[9] = 0.5;   // Master Tune
  allParameterValues[10] = 0.146; // Detune
  allParameterValues[11] = 0.0;   // Overdrive
  
  // Apply all parameter values to EPiano engine
  for (int i = 0; i < NUM_PARAMETERS; i++) {
    updateParameterFromMenu(i, allParameterValues[i]);
  }
  
  // Setup encoders
  setupEncoders();
  
  // Setup menu encoder button
  pinMode(MENU_ENCODER_SW, INPUT_PULLUP);
  
#ifdef USE_LCD_DISPLAY
  Wire.begin();
  lcd.init();
  lcd.backlight();
  Serial.println("LCD initialized");
#endif

#ifdef USE_OLED_DISPLAY
  Wire.begin();
  delay(50);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  Serial.println("OLED initialized");
#endif

#ifdef USE_MIDI_HOST
  myusb.begin();
  Serial.println("USB Host MIDI initialized");
#endif

#ifdef ENABLE_DIN_MIDI
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(OnNoteOn);
  MIDI.setHandleNoteOff(OnNoteOff);
  MIDI.setHandleControlChange(OnControlChange);
  MIDI.setHandleProgramChange(OnProgramChange);
  Serial.println("DIN MIDI initialized");
#endif

  initializeMenu();
  
  Serial.println("EPiano-Teensy Synth initialized");
  Serial.println("Parameters: 12 EPiano parameters");
  Serial.println("Press 'r' to reset encoder baselines");
  
  // Show startup screen
  displayText(PROJECT_NAME, PROJECT_SUBTITLE);
  delay(2000);
  updateDisplay();
}

void setupEncoders() {
  // Initialize all encoder pointers to nullptr
  for (int i = 0; i <= 20; i++) {
    encoders[i] = nullptr;
  }
  
  // Only initialize encoders that are actually used (mapped to parameters >= 0)
  encoders[1] = new Encoder(ENC_1_CLK, ENC_1_DT);   // Decay
  encoders[2] = new Encoder(ENC_2_CLK, ENC_2_DT);   // Release  
  encoders[3] = new Encoder(ENC_3_CLK, ENC_3_DT);   // Hardness
  encoders[4] = new Encoder(ENC_4_CLK, ENC_4_DT);   // Treble
  encoders[5] = new Encoder(ENC_5_CLK, ENC_5_DT);   // Pan/Tremolo
  encoders[6] = new Encoder(ENC_6_CLK, ENC_6_DT);   // LFO Rate
  encoders[7] = new Encoder(ENC_7_CLK, ENC_7_DT);   // Velocity Sense
  encoders[8] = new Encoder(ENC_8_CLK, ENC_8_DT);   // Stereo
  encoders[9] = new Encoder(ENC_9_CLK, ENC_9_DT);   // Master Tune
  encoders[10] = new Encoder(ENC_10_CLK, ENC_10_DT); // Detune
  encoders[11] = new Encoder(ENC_11_CLK, ENC_11_DT); // Overdrive
  // Encoders 13-20 are disabled (mapped to -1), so we don't initialize them
}

// ============================================================================
// Parameter Update Function
// ============================================================================

void updateParameterFromMenu(int paramIndex, float val) {
  switch (paramIndex) {
    case 0: // Decay
      ep.setDecay(val);
      break;
    case 1: // Release
      ep.setRelease(val);
      break;
    case 2: // Hardness
      ep.setHardness(val);
      break;
    case 3: // Treble
      ep.setTreble(val);
      break;
    case 4: // Pan/Tremolo
      ep.setPanTremolo(val);
      break;
    case 5: // LFO Rate
      ep.setPanLFO(val);
      break;
    case 6: // Velocity Sense
      ep.setVelocitySense(val);
      break;
    case 7: // Stereo
      ep.setStereo(val);
      break;
    case 8: // Polyphony (read-only, fixed at 16)
      break;
    case 9: // Master Tune
      ep.setTune(val);
      break;
    case 10: // Detune
      ep.setDetune(val);
      break;
    case 11: // Overdrive
      ep.setOverdrive(val);
      break;
    case 12: // MIDI Channel
      midiChannel = (int)(val * 16); // 0-16 (0=omni, 1-16=channels)
      break;
  }
}

// ============================================================================
// MIDI Functions
// ============================================================================

void noteOn(int note, int velocity) {
  ep.noteOn(note, velocity);
}

void noteOff(int note) {
  ep.noteOff(note);
}

void handleControlChange(int cc, int value) {
  ep.processMidiController(cc, value);
}

void handleProgramChange(int program) {
  if (program >= 0 && program < 5) {
    loadPreset(program);
  }
}

#ifdef ENABLE_DIN_MIDI
void OnNoteOn(byte channel, byte note, byte velocity) {
  if (midiChannel != 0 && channel != midiChannel) return;
  noteOn(note, velocity);
}

void OnNoteOff(byte channel, byte note, byte velocity) {
  if (midiChannel != 0 && channel != midiChannel) return;
  noteOff(note);
}

void OnControlChange(byte channel, byte cc, byte value) {
  if (midiChannel != 0 && channel != midiChannel) return;
  handleControlChange(cc, value);
}

void OnProgramChange(byte channel, byte program) {
  if (midiChannel != 0 && channel != midiChannel) return;
  handleProgramChange(program);
}
#endif

// ============================================================================
// Encoder Functions
// ============================================================================

void readAllControls() {
  // Read all hardware encoders
  for (int i = 1; i <= 20; i++) {
    if (i == 12) continue; // Skip encoder 12 (doesn't exist)
    if (encoders[i] != nullptr) {
      long newPosition = encoders[i]->read();
      if (!encoderBaselines[i]) {
        encoderPositions[i] = newPosition;
        encoderBaselines[i] = true;
      } else {
        long change = newPosition - encoderPositions[i];
        if (abs(change) >= 4) { // Encoder sensitivity threshold
          // If any physical knob is turned, exit menu mode
          if (inMenu) {
            inMenu = false;
          }
          
          int direction = (change > 0) ? 1 : -1;  // Correct direction for physical encoders
          handleEncoderChange(i, direction);
          encoderPositions[i] = newPosition;
        }
      }
    }
  }
  
  // Handle menu encoder when not in menu
  if (!inMenu && MENU_ENCODER_PARAM >= 0) {
    static long lastMenuPosition = 0;
    long rawMenuPosition = menuEncoder.read();
    long menuChange = rawMenuPosition - lastMenuPosition;
    if (abs(menuChange) >= 4) {  // 4 ticks per parameter change
      int direction = (menuChange < 0) ? 1 : -1;  // Flipped direction
      updateEncoderParameter(MENU_ENCODER_PARAM, direction);
      lastMenuPosition = rawMenuPosition;
    }
  }
}

void handleEncoder() {
  static long lastMenuEncoderPosition = 0;
  static unsigned long lastMenuButtonPress = 0;
  static bool lastMenuButtonState = HIGH;
  
#ifdef USE_OLED_DISPLAY
  long newMenuEncoderValue = menuEncoder.read() / 4; // Less sensitive for OLED encoder
#else
  long newMenuEncoderValue = menuEncoder.read() / 2; // Standard sensitivity for separate encoder  
#endif
  bool menuButtonPressed = digitalRead(MENU_ENCODER_SW) == LOW;
  
  // Handle menu encoder rotation - EXACTLY like Mini-Teensy
  if (inMenu && newMenuEncoderValue != lastMenuEncoderPosition) {
    if (newMenuEncoderValue > lastMenuEncoderPosition) {
      decrementMenuIndex();  // Match Mini-Teensy direction
    } else {
      incrementMenuIndex();   // Match Mini-Teensy direction
    }
    updateDisplay();
    lastMenuEncoderPosition = newMenuEncoderValue;
  }
  
  // Handle menu button press
  if (menuButtonPressed && !lastMenuButtonState) {
    unsigned long currentTime = millis();
    if (currentTime - lastMenuButtonPress > 200) {
      if (inMenu) {
        selectMenuItem();
      } else {
        enterMenu();
      }
      lastMenuButtonPress = currentTime;
    }
  }
  lastMenuButtonState = menuButtonPressed;
}

void resetEncoderBaselines() {
  for (int i = 1; i <= 20; i++) {
    encoderBaselines[i] = false;
  }
  Serial.println("Encoder baselines reset");
}

// ============================================================================
// Main Loop
// ============================================================================

void loop() {
  // Handle USB Device MIDI
#ifdef USE_USB_DEVICE_MIDI
  while (usbMIDI.read()) {
    byte type = usbMIDI.getType();
    byte channel = usbMIDI.getChannel();
    byte data1 = usbMIDI.getData1();
    byte data2 = usbMIDI.getData2();
    
    if (midiChannel != 0 && channel != midiChannel) continue;
    
    switch (type) {
      case usbMIDI.NoteOn:
        if (data2 > 0) {
          noteOn(data1, data2);
        } else {
          noteOff(data1);
        }
        break;
      case usbMIDI.NoteOff:
        noteOff(data1);
        break;
      case usbMIDI.ControlChange:
        handleControlChange(data1, data2);
        break;
      case usbMIDI.ProgramChange:
        handleProgramChange(data1);
        break;
    }
  }
#endif

  // Handle USB Host MIDI
#ifdef USE_MIDI_HOST
  myusb.Task();
  if (midi1.read()) {
    byte type = midi1.getType();
    byte channel = midi1.getChannel();
    byte data1 = midi1.getData1();
    byte data2 = midi1.getData2();
    
    // if (midiChannel != 0 && channel != midiChannel) continue;
    
    switch (type) {
      case usbMIDI.NoteOn:
        if (data2 > 0) {
          noteOn(data1, data2);
        } else {
          noteOff(data1);
        }
        break;
      case usbMIDI.NoteOff:
        noteOff(data1);
        break;
      case usbMIDI.ControlChange:
        handleControlChange(data1, data2);
        break;
      case usbMIDI.ProgramChange:
        handleProgramChange(data1);
        break;
    }
  }
#endif

  // Handle DIN MIDI
#ifdef ENABLE_DIN_MIDI
  MIDI.read();
#endif
  
  readAllControls();
  handleEncoder();
  
  // Minimal serial input check for performance
  if (Serial.available()) {
    char input = Serial.read();
    if (input == 'r' || input == 'R') {
      resetEncoderBaselines();
    }
  }
  delay(5); // Reduced delay for better responsiveness
}