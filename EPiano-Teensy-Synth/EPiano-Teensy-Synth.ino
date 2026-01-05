/*
 * EPiano-Teensy Synth v1.0
 * A 16-voice polyphonic electric piano synthesizer built with the Teensy 4.1 microcontroller, 
 * featuring the MDA EPiano synthesis engine with real-time parameter control.
 */

#define NUM_PARAMETERS 13
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


AudioSynthEPiano ep(VOICES);    // 16-voice EPiano

#ifdef USE_USB_AUDIO
AudioOutputUSB usb1;            // USB audio output (stereo)
AudioConnection patchCord1(ep, 0, usb1, 0); // Left channel
AudioConnection patchCord2(ep, 1, usb1, 1); // Right channel
#endif

#ifdef USE_TEENSY_DAC
AudioOutputI2S i2s1;            // I2S DAC output (Teensy Audio Shield)
AudioControlSGTL5000 sgtl5000_1;
AudioConnection patchCord3(ep, 0, i2s1, 0); // Left channel
AudioConnection patchCord4(ep, 1, i2s1, 1); // Right channel
#endif


Encoder menuEncoder(MENU_ENCODER_CLK, MENU_ENCODER_DT);

// Encoder arrays (21 elements to accommodate indices 0-20)
Encoder* encoders[21];
long encoderPositions[21] = {0};
bool encoderBaselines[21] = {false};


#ifdef USE_MIDI_HOST
USBHost myusb;
USBHub hub1(myusb);
MIDIDevice midi1(myusb);
#endif

#ifdef USE_DIN_MIDI
#include <MIDI.h>
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);
#endif


float allParameterValues[NUM_PARAMETERS];
int currentPreset = 0;
int midiChannel = 0; // 0 = omni, 1-16 = specific channel

// Display update tracking for MIDI CC changes
int lastChangedParam = -1;
float lastChangedValue = 0.0;
String lastChangedName = "";
bool parameterChanged = false;

extern const char* controlNames[];

extern const EPianoPreset epianoPresets[];


void setup() {
  Serial.begin(115200);
  
  // Audio setup
  AudioMemory(50);
  
#ifdef USE_TEENSY_DAC
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.8);
  Serial.println("Teensy Audio Shield initialized");
#endif

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
  Serial.println("DIN MIDI initialized");
#endif

  initializeMenu();
  
  Serial.println("EPiano-Teensy Synth initialized");
  Serial.println("Parameters: 12 EPiano parameters");
  
#ifdef USE_USB_AUDIO
  Serial.println("Audio output: USB Audio");
#endif
#ifdef USE_TEENSY_DAC
  Serial.println("Audio output: Teensy Audio Shield (I2S)");
#endif

  Serial.println("MIDI CC mapping (using unified CC_X_PARAM):");
  Serial.print("  Decay: CC"); Serial.println(CC_1_PARAM);
  Serial.print("  Release: CC"); Serial.println(CC_2_PARAM);
  Serial.print("  Hardness: CC"); Serial.println(CC_3_PARAM);
  Serial.print("  Treble: CC"); Serial.println(CC_4_PARAM);
  Serial.print("  Pan/Tremolo: CC"); Serial.println(CC_5_PARAM);
  Serial.print("  LFO Rate: CC"); Serial.println(CC_6_PARAM);
  Serial.print("  Velocity Sense: CC"); Serial.println(CC_7_PARAM);
  Serial.print("  Stereo: CC"); Serial.println(CC_8_PARAM);
  Serial.print("  Tune: CC"); Serial.println(CC_9_PARAM);
  Serial.print("  Detune: CC"); Serial.println(CC_10_PARAM);
  Serial.print("  Overdrive: CC"); Serial.println(CC_12_PARAM);
  Serial.print("  Volume: CC"); Serial.println(CC_VOLUME);
  Serial.println("Program changes: 0-4 for presets");
  
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
  encoders[9] = new Encoder(ENC_9_CLK, ENC_9_DT);   // Polyph0ny
  encoders[10] = new Encoder(ENC_10_CLK, ENC_10_DT); // Master Tune
  encoders[11] = new Encoder(ENC_11_CLK, ENC_11_DT); // Detune
  encoders[13] = new Encoder(ENC_17_CLK, ENC_17_DT); // Overdrive

  // Encoders 13-20 are disabled (mapped to -1), so we don't initialize them
}


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


void noteOn(int note, int velocity) {
  ep.noteOn(note, velocity);
}

void noteOff(int note) {
  ep.noteOff(note);
}


void processMidiMessage(byte type, byte channel, byte data1, byte data2) {
  // Filter by MIDI channel (0 = omni, 1-16 = specific channel)
  if (midiChannel != 0 && channel != midiChannel) return;
  
  switch (type) {
    case 0x90: // Note On (or Note Off with velocity 0)
      if (data2 > 0) {
        noteOn(data1, data2);
      } else {
        noteOff(data1);
      }
      break;
      
    case 0x80: // Note Off
      noteOff(data1);
      break;
      
    case 0xB0: // Control Change
      handleControlChange(data1, data2);
      break;
      
    case 0xC0: // Program Change
      handleProgramChange(data1);
      break;
  }
}

void handleControlChange(int cc, int value) {
  // Convert MIDI value (0-127) to parameter value (0.0-1.0)
  float paramValue = value / 127.0;
  
  // Handle standard MIDI CCs first
  if (cc == CC_MODWHEEL) {
    // EPiano doesn't have built-in mod wheel support, just store the value for potential future use
    // Track mod wheel change for display
    lastChangedParam = -1;  // Special flag for non-parameter controls
    lastChangedName = "Mod Wheel";
    lastChangedValue = value;  // Use raw 0-127 value for display
    parameterChanged = true;
    return;
  }
  
  if (cc == CC_VOLUME) {
    ep.setVolume(paramValue);
    
    // Track volume change for display
    lastChangedParam = -1;  // Special flag for non-parameter controls  
    lastChangedName = "Volume";
    lastChangedValue = value;  // Use raw 0-127 value for display
    parameterChanged = true;
    return;
  }
  
  int paramIndex = -1;
  
  if (cc == CC_1_PARAM) paramIndex = 0;        // Decay
  else if (cc == CC_2_PARAM) paramIndex = 1;   // Release
  else if (cc == CC_3_PARAM) paramIndex = 2;   // Hardness
  else if (cc == CC_4_PARAM) paramIndex = 3;   // Treble
  else if (cc == CC_5_PARAM) paramIndex = 4;   // Pan/Tremolo
  else if (cc == CC_6_PARAM) paramIndex = 5;   // LFO Rate
  else if (cc == CC_7_PARAM) paramIndex = 6;   // Velocity
  else if (cc == CC_8_PARAM) paramIndex = 7;   // Stereo
  else if (cc == CC_9_PARAM) paramIndex = 8;   // Polyphony
  else if (cc == CC_10_PARAM) paramIndex = 9;  // Master Tune
  else if (cc == CC_11_PARAM) paramIndex = 10; // Detune
  else if (cc == CC_12_PARAM) paramIndex = 11; // Overdrive
  
  // Update parameter if mapped
  if (paramIndex >= 0) {
    allParameterValues[paramIndex] = paramValue;
    updateParameterFromMenu(paramIndex, paramValue);
    
    // Track parameter change for display
    lastChangedParam = paramIndex;
    lastChangedValue = paramValue;
    lastChangedName = controlNames[paramIndex];
    parameterChanged = true;
  }
  // Also pass to EPiano engine for any internal handling
  ep.processMidiController(cc, value);
}

void handleProgramChange(int program) {
  if (program >= 0 && program < getNumPresets()) {
    loadPreset(program);
    Serial.print("Program change to preset: ");
    Serial.println(program);
    
    // Update display to show preset name
    String line1 = "Preset " + String(program + 1);
    String line2 = String(getPresetName(program));
    displayText(line1, line2);
  }
}



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



void loop() {
  // Handle USB Device MIDI
#ifdef USE_USB_DEVICE_MIDI
  while (usbMIDI.read()) {
    processMidiMessage(usbMIDI.getType(), usbMIDI.getChannel(), 
                      usbMIDI.getData1(), usbMIDI.getData2());
  }
#endif

  // Handle USB Host MIDI
#ifdef USE_MIDI_HOST
  myusb.Task();
  while (midi1.read()) {
    processMidiMessage(midi1.getType(), midi1.getChannel(), 
                      midi1.getData1(), midi1.getData2());
  }
#endif

  // Handle DIN MIDI
#ifdef USE_DIN_MIDI
  MIDI.read();
#endif
  
  readAllControls();
  handleEncoder();
  
  // Update display if parameter changed during this loop iteration
  if (parameterChanged) {
    // If we were in menu mode, exit menu to show MIDI parameter
    if (inMenu) {
      inMenu = false;
    }
    String line2 = "";
    
    if (lastChangedParam >= 0) {
      // Special display formatting for certain parameters
      if (lastChangedParam == 9) { // Master Tune
        float cents = (lastChangedValue - 0.5) * 100; // -50 to +50 cents
        line2 = (cents >= 0 ? "+" : "") + String((int)cents) + "c";
      } else if (lastChangedParam == 10) { // Detune  
        float detune = lastChangedValue * 20; // 0 to 20 cents
        line2 = String(detune, 1) + "c";
      } else {
        int displayValue = (int)(lastChangedValue * 127); // 0-127 scale
        line2 = String(displayValue);
      }
    } else {
      // For volume and other non-parameter controls
      line2 = String((int)lastChangedValue);
    }
    
    displayText(lastChangedName, line2);
    parameterChanged = false;
  }
  
  delay(5); // Reduced delay for better responsiveness
}