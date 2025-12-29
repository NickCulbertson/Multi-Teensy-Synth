# EPiano-Teensy Synth

A **16-voice polyphonic electric piano synthesizer** built with the Teensy 4.1 microcontroller, featuring the MDA EPiano synthesis engine. Provides authentic electric piano sounds with real-time parameter control.

## Key Features
- **16-voice polyphony** with authentic electric piano sound modeling
- **8 classic electric piano programs** (Default, Bright, Mellow, Autopan, Tremolo)
- **Real-time parameter control** (decay, release, hardness, treble, pan/tremolo, etc.)
- **USB Audio + MIDI** - single cable to computer
- **20 hardware encoders** + LCD for real-time control
- **Configurable encoder mapping** for intuitive control
- **High-quality synthesis** based on MDA EPiano engine

## Hardware Requirements

**Full Build:**
- **Teensy 4.1** microcontroller  
- **19x Rotary Encoders** + **1x Menu Encoder** with push button
- **16x2 I2C LCD** display
- Enclosure, knob caps, hookup wire

**Minimal Build:**
- **Teensy 4.1** + **1x Menu Encoder** + **LCD**
- USB cable for audio/MIDI output
- All parameters accessible via menu system

## Software Installation

1. **Install Arduino IDE** 2.0+ from [arduino.cc](https://www.arduino.cc/en/software)
2. **Add Teensy support:**
   - **Modern IDE:** Tools → Board Manager → Search "Teensy" → Install
   - **Fallback:** Download Teensyduino from [PJRC](https://www.pjrc.com/teensy/td_download.html)
3. **Install libraries:** Tools → Manage Libraries → Install "LiquidCrystal I2C" and "Encoder" 
4. **Download this project** and open `EPiano-Teensy-Synth.ino`
5. **Configure:** Board→Teensy 4.1, USB Type→Audio+MIDI
6. **Upload!** Your computer will show "Teensy Audio" device

## Usage

**Menu Navigation:** Click encoder to navigate, turn to adjust values. All parameters accessible via hierarchical menu system.

**Parameters:**
- **Decay** - Note decay time
- **Release** - Note release time  
- **Hardness** - Velocity sensitivity and timbre
- **Treble** - High frequency boost
- **Pan/Tremolo** - Stereo panning and tremolo effects
- **LFO Rate** - Modulation speed
- **Velocity Sense** - How velocity affects sound
- **Stereo** - Stereo width control
- **Tune** - Master tuning
- **Detune** - Slight detuning for chorus effect
- **Overdrive** - Tube-style saturation

## Acknowledgements

This synthesizer uses the **MDA EPiano** synthesis engine, originally created by **Paul Kellett** and made open source. The Teensy port comes from **MicroDexed Touch** project.

- **Original MDA EPiano**: Paul Kellett (mda-vst) - MIT/GPL dual license
- **Teensy Implementation**: MicroDexed Touch project
- **Hardware Platform**: PJRC Teensy 4.1 and Audio Library

## License

This project uses multiple open-source licenses. The MDA EPiano engine is licensed under MIT/GPL dual license. See individual files for specific license information.