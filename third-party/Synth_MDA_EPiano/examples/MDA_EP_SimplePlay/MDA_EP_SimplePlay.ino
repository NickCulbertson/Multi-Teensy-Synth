#include <Audio.h>
#include "synth_mda_epiano.h"
// Teensy core already provides usbMIDI

// --- Audio objects ---
AudioSynthEPiano     ep(16);    // 16 voices
AudioOutputUSB       usb1;      // USB audio out (stereo)

// Audio connections: EPiano → USB L/R
AudioConnection      patchCord1(ep, 0, usb1, 0);
AudioConnection      patchCord2(ep, 0, usb1, 1);

void setup() {
  AudioMemory(40);
  // No SGTL5000 / I2S / DAC setup needed for USB audio
}

// Handle incoming USB MIDI and play EPiano
void loop() {
  // Process all incoming MIDI messages
  while (usbMIDI.read()) {
    byte type    = usbMIDI.getType();
    byte channel = usbMIDI.getChannel();  // 1–16 (if you want to filter)
    byte data1   = usbMIDI.getData1();    // note / CC / program #
    byte data2   = usbMIDI.getData2();    // velocity / CC value

    switch (type) {
      case usbMIDI.NoteOn:
        if (data2 > 0) {
          // Note on
          ep.noteOn(data1, data2);
        } else {
          // Note on with vel 0 = note off
          ep.noteOff(data1);
        }
        break;

      case usbMIDI.NoteOff:
        ep.noteOff(data1);
        break;

      case usbMIDI.ProgramChange:
        // Map program change 0–127 to one of your EPiano programs
        // (you said random(7) before, so let's assume 0–6 are valid)
        ep.setProgram(data1 % 7);
        break;

      case usbMIDI.ControlChange:
        // Example: sustain pedal (CC64)
        if (data1 == 64) {
          // data2 >= 64 = pedal down, < 64 = pedal up
          // EPiano doesn’t have built-in sustain, but you could add logic
          // here if you later wrap it in a more complex synth.
        }
        break;

      default:
        // Ignore everything else for now
        break;
    }
  }

  // No delays here – let audio & MIDI run smoothly
}
