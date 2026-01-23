# Multi-Teensy-Synth Collection

A collection of **5 standalone polyphonic synthesizers** built with the Teensy 4.1 microcontroller, each featuring different synthesis engines.

**Built on legendary open-source synthesis engines:** [MicroDexed Touch](https://codeberg.org/positionhigh/MicroDexed-touch), [Mutable Instruments Braids](https://github.com/pichenettes/eurorack), [MDA EPiano](https://sourceforge.net/projects/mda-vst/), and [MicroDexed](https://codeberg.org/dcoredump/MicroDexed). *See full acknowledgements below.*

## 🎹 Available Synthesizers

### 1. Mini-Teensy-Synth
**6-Voice Virtual Analog Synthesizer** (Minimoog-inspired)
- 3 oscillators per voice with multiple waveforms
- Ladder filter with resonance and envelope
- LFO with multiple targets (pitch, filter, amplitude)
- Comprehensive ADSR envelopes
- Real-time parameter control via 19 encoders
- 20 built-in presets + configurable encoder mapping

### 2. EPiano-Teensy-Synth
**16-Voice Electric Piano Synthesizer** (MDA EPiano engine)
- Authentic electric piano sound modeling (Rhodes/Wurlitzer-style)
- 5 built-in presets: Default, Bright, Mellow, Autopan, Tremolo
- Real-time control: decay, release, hardness, treble, stereo width, LFO, overdrive
- Stereo output with authentic panning and modulation effects
- 11 mapped encoders for immediate hands-on control

### 3. FM-Teensy-Synth
**16-Voice FM Synthesizer** (DX7-compatible via Dexed engine)
- Full DX7 compatibility with 6-operator FM synthesis
- 8 ROM banks with 32 presets each (256 total classic DX7 sounds)
- Real-time control of all 6 operator levels + algorithm, feedback, LFO
- Bank/patch browsing system like original DX7
- Authentic DX7 sound engine with multiple algorithms

### 4. DCO-Teensy-Synth
**6-Voice DCO Synthesizer** (Juno inspired)
- Dual oscillators: PWM + Sawtooth with sub-oscillator
- Authentic 24dB ladder filter with resonance and envelope modulation
- Classic BBD-style Chorus with I, II, and I+II modes for vintage stereo imaging
- Independent LFO targeting filter, oscillators, or pulse width
- 6 classic Juno-inspired presets with direct chorus mode control

### 5. MacroOscillator-Teensy-Synth
**6-Voice Digital Synthesizer** (Mutable Instruments Braids)
- 40+ synthesis algorithms: analog modeling, FM, physical modeling, digital noise
- Macro oscillator approach with unified timbre and color controls
- Advanced algorithms: vowel formants, particle noise, resonant comb filtering
- Real-time morphing between synthesis methods
- Eurorack-quality digital synthesis in standalone format

## 🛠 Hardware Requirements

**NOTE: ENABLE YOUR HARDWARE SETUP IN CONFIG.H** - You can build this with no additional components if you use USB audio and USB MIDI. Params are changed with MIDICC and you can change preset with Program Changes.

**Full Build:**
### Required Components
- **Teensy 4.1** microcontroller

### Optional Components
- **19x Rotary Encoders** + **1x Menu Encoder** with push button
- **16x2 LCD** (I2C) or **128x64 OLED** display
- **USB connection** for MIDI input and audio output
- **DIN MIDI Input** circuit (6N138 optocoupler + 220Ω resistor + 5-pin DIN)
- **USB MIDI Host** setup:
  - USB OTG adapter cable or USB Host pins on Teensy 4.1
  - USB MIDI controller (keyboard, pad controller, etc.)
  - External 5V power supply if using high-power USB devices
- **Enclosure, knob caps, hookup wire**

**Minimal Build:**
- **Teensy 4.1** + **1x Menu Encoder** + **LCD**
- USB cable for audio/MIDI output
- All parameters accessible via menu system

**Parts:**
- Teensy 4.1 (https://www.sparkfun.com/teensy-4-1-without-ethernet.html)
- Menu Encoder (https://www.amazon.com/Taiss-KY-040-Encoder-15×16-5-Arduino/dp/B07F26CT6B/ref=sr_1_3_pp)
- Other Encoders (https://www.aliexpress.us/item/3256801237549169.html)
- LCD 2X16 (https://www.amazon.com/Hosyond-Display-Module-Arduino-Raspberry/dp/B0BWTFN9WF/ref=sr_1_2)
- Knobs (https://www.amazon.com/Taiss-Silver-Rotary-Potentiometer-Diameter/dp/B07F25NMJ7/ref=sr_1_5)

## Detailed Wiring & Pinout

This project is compatible with the MiniTeensy Synth box. See those full build details here: https://github.com/NickCulbertson/Mini-Teensy-Synth

### **Teensy 4.1 Complete Pinout Assignment**

```
                    Teensy 4.1 Pinout Map
    ┌─────┐                     ┌─────┐
    │  0  │ enc3 CLK         5V │ VIN │ LCD Power
    │  1  │ enc3 DT             │ GND │ All encoders daisy-chained to ground + LCD and Menu
    │  2  │ enc2 CLK            │ 3V  │ Menu Encoder Power
    │  3  │ enc2 DT             │ 23  │ enc15 CLK 
    │  4  │ enc1 CLK            │ 22  │ enc15 DT  
    │  5  │ enc1 DT             │ 21  │ enc11 CLK 
    │  6  │ enc5 CLK            │ 20  │ enc11 DT  
    │  7  │ enc5 DT             │ 19  │ SCL (LCD I2C) 
    │  8  │ enc4 CLK            │ 18  │ SDA (LCD I2C) 
    │  9  │ enc4 DT             │ 17  │ enc18 CLK 
    │ 10  │ enc8 CLK            │ 16  │ enc18 DT 
    │ 11  │ enc8 DT             │ 15  │ Menu Encoder DT 
    │ 12  │ enc7 CLK            │ 14  │ Menu Encoder CLK 
    └─────┘                     │ 13  │ Menu Encoder SW 
                                └─────┘
    │ 24  │ enc7 DT             │ 41  │ enc14 DT  
    │ 25  │ enc6 CLK            │ 40  │ enc20 CLK 
    │ 26  │ enc10 DT            │ 39  │ enc20 DT  
    │ 27  │ enc6 DT             │ 38  │ enc19 CLK 
    │ 28  │ enc10 CLK           │ 37  │ enc19 DT 
    │ 29  │ enc9 CLK            │ 36  │ enc16 CLK 
    │ 30  │ enc9 DT             │ 35  │ enc16 DT 
    │ 31  │ enc17 CLK           │ 34  │ enc13 CLK
    │ 32  │ enc17 DT            │ 33  │ enc13 DT  
    └─────┘                     │     │
                                │ 50  │ enc14 CLK (Under the Teensy)
                                │ 51  │
                                │ 52  │
                                └─────┘
```

### **Minimal Build (LCD + 1 Encoder)**
Perfect for testing or easier builds:

**Menu Encoder Connections:**
```
Menu Encoder Pin    →  Teensy 4.1 Pin    │  Function
─────────────────────────────────────────┼─────────────
SW (Push Button)    →  13                │  Menu Select
CLK                 →  14                │  Rotary Clock
DT                  →  15                │  Rotary Data  
VCC                 →  3.3V              │  Power (3.3V)
GND                 →  GND               │  Ground
```

**LCD I2C Connections:**
```
LCD Pin    →  Teensy 4.1 Pin    │  Function
────────────────────────────────┼─────────────────
VCC        →  5V (VIN)          │  Power
GND        →  GND               │  Ground
SDA        →  18                │  I2C Data
SCL        →  19                │  I2C Clock
```

## 📡 MIDI Configuration

All projects support flexible MIDI input options:

```cpp
// MIDI Configuration - Enable the MIDI sources you want to use
#define USE_USB_DEVICE_MIDI // USB Device MIDI for DAW/computer connection (default)
// #define USE_MIDI_HOST    // USB Host MIDI for external controllers connected to Teensy
// #define ENABLE_DIN_MIDI  // Hardware DIN MIDI input (requires circuit)
```

**Possible combinations:**
1. **USB Device only** - Connect to computer/DAW via USB
2. **USB Host only** - Connect MIDI controller directly to Teensy (standalone operation)  
3. **Both USB modes** - Maximum flexibility, both DAW and controller simultaneously
4. **DIN MIDI only** - Hardware MIDI input via 5-pin DIN connector
5. **Any combination** - Mix and match as needed

## 🎛 Encoder Mapping

Each project features **configurable encoder mapping** via `config.h`:

- **19 physical encoders** can be mapped to any synthesis parameter
- **Menu encoder** can control a parameter or be menu-only
- **Easy customization** - just change parameter numbers in config file
- **Disabled encoders** - set to -1 to disable unused encoders

Example configuration:
```cpp
#define ENC_1_PARAM    0   // Map encoder 1 to parameter 0
#define ENC_2_PARAM    1   // Map encoder 2 to parameter 1  
#define ENC_3_PARAM   -1   // Disable encoder 3
```

## 💾 Software Requirements

### Arduino Libraries (install via Library Manager)
- **LiquidCrystal I2C** (by Frank de Brabander) - for LCD display
- **Adafruit SSD1306** (by Adafruit) - for OLED display
- **Adafruit GFX Library** (by Adafruit) - for OLED display  
- **Encoder** (by Paul Stoffregen)
- **MIDI Library** (by Francois Best) - only needed if enabling DIN MIDI

### Built-in Teensy Libraries (no installation needed)
- Audio, Wire, USBHost_t36

## 🚀 Getting Started

1. **Choose a synthesizer** from the 5 available options
2. **Install required libraries** via Arduino Library Manager
3. **Configure hardware** in `config.h`:
   - Set display type (LCD or OLED)
   - Configure MIDI sources (USB Device/Host, DIN MIDI)
   - Map encoders to desired parameters
4. **Upload to Teensy 4.1**
5. **Connect MIDI controller** and start playing!

### Quick Feature Overview
- **Plug-and-play** - Each synth works immediately after upload
- **Menu system** - Navigate parameters, presets, and settings with menu encoder
- **Real-time control** - Turn any encoder to immediately adjust that parameter
- **Auto-exit menus** - Physical encoders automatically exit menus for instant control

## 📋 Project Structure

```
Multi-Teensy-Synth/
├── Mini-Teensy-Synth/          # Virtual Analog Synthesizer
│   ├── Mini-Teensy-Synth.ino   # Main sketch
│   ├── config.h                # Hardware & parameter mapping
│   ├── MenuNavigation.cpp      # Menu system implementation
│   └── README.md               # Detailed documentation
├── EPiano-Teensy-Synth/        # Electric Piano Synthesizer  
│   ├── EPiano-Teensy-Synth.ino # Main sketch
│   ├── config.h                # Hardware & parameter mapping
│   ├── MenuNavigation.cpp      # Menu system implementation
│   ├── src/                    # MDA EPiano sound engine
│   └── README.md               # Detailed documentation
├── FM-Teensy-Synth/            # FM Synthesizer
│   ├── FM-Teensy-Synth.ino     # Main sketch
│   ├── config.h                # Hardware & parameter mapping
│   ├── MenuNavigation.cpp      # Menu system implementation
│   ├── src/Synth_Dexed/        # DX7-compatible FM engine
│   ├── dx7_roms_unpacked.h     # DX7 ROM preset data
│   └── README.md               # Detailed documentation
├── DCO-Teensy-Synth/           # DCO Synthesizer (Juno-inspired)
│   ├── DCO-Teensy-Synth.ino    # Main sketch
│   ├── config.h                # Hardware & parameter mapping
│   ├── MenuNavigation.cpp      # Menu system implementation
│   └── README.md               # Detailed documentation
└── MacroOscillator-Teensy-Synth/ # Digital Macro Oscillator
    ├── MacroOscillator-Teensy-Synth.ino # Main sketch
    ├── config.h                # Hardware & parameter mapping
    ├── MenuNavigation.cpp      # Menu system implementation
    ├── src/                    # Braids engine source
    └── README.md               # Detailed documentation
```

## 🎯 Key Features

- **5 Complete Synthesis Engines** - Virtual analog, FM, electric piano, DCO, and digital macro oscillator
- **Professional Audio Quality** - 16-bit/44.1kHz stereo USB audio output
- **Real-time Performance** - Optimized for low-latency live playing and recording
- **6-16 Voice Polyphony** - Depending on synthesis complexity
- **Flexible MIDI Input** - USB Device, USB Host, and DIN MIDI support
- **Intuitive Control** - Menu systems with immediate encoder override
- **Configurable Mapping** - Assign any encoder to any parameter via config files
- **Standalone Ready** - No computer required with USB Host MIDI setup

## 🔊 Audio Output

All synthesizers output high-quality audio via:
- **USB Audio** - Connect to computer for recording/monitoring
- **I2S Audio** - Connect to external DAC for standalone operation (hardware modification required)

## 🤝 Contributing

Feel free to contribute new synthesis engines, improvements, or bug fixes. Each synthesizer is standalone, making it easy to add new engines or enhance existing ones.

## 🏆 Acknowledgements

This project stands proudly on the shoulders of giants. The synthesizer ecosystem thrives because of the incredible work done by indie developers, companies, and communities who have shared their knowledge and code with the world.

### Core Platform
- **[PJRC](https://www.pjrc.com/)** - Teensy 4.1 microcontroller platform and the incredible Teensy Audio Library that makes real-time audio synthesis possible on embedded hardware

### Synthesis Engine Foundations

This Multi-Teensy project incorporates synthesis engines and techniques from several groundbreaking open-source projects:

#### **Mutable Instruments Braids**
- **Original Author**: Émilie Gillet ([pichenettes](https://github.com/pichenettes))
- **License**: MIT License (STM32F projects)
- **Repository**: [github.com/pichenettes/eurorack](https://github.com/pichenettes/eurorack)
- **Contribution**: Macro oscillator concepts, digital synthesis algorithms, and sophisticated signal processing techniques
- **About**: Revolutionary Eurorack macro oscillator module that introduced advanced synthesis techniques to the modular world

#### **MicroDexed (FM Synthesis)**
- **Original Author**: H. Wirtz (wirtz@parasitstudio.de)
- **License**: GPL v3.0
- **Repository**: [codeberg.org/dcoredump/MicroDexed](https://codeberg.org/dcoredump/MicroDexed) (moved from GitHub)
- **Based on**: Dexed FM synthesizer by [asb2m10](https://github.com/asb2m10/dexed) and Google's [music-synthesizer-for-android](https://github.com/google/music-synthesizer-for-android)
- **Contribution**: 6-operator FM synthesis engine, DX7 compatibility, and advanced FM algorithms ported to Teensy
- **About**: A complete DX7-compatible FM synthesizer running on Teensy hardware

#### **MDA EPiano**
- **Original Author**: Paul Kellett (mda-vst)
- **License**: MIT/GPL (dual license)
- **Repository**: [sourceforge.net/projects/mda-vst](https://sourceforge.net/projects/mda-vst/)
- **Contribution**: Electric piano synthesis engine, sample-based synthesis techniques, and vintage electric piano emulation
- **About**: Classic electric piano VST plugin made open source, providing authentic electric piano sounds

#### **Synth_Dexed**
- **Author**: H. Wirtz
- **License**: GPL v3.0 / Apache 2.0 (dual license for compatibility)
- **Repository**: [codeberg.org/dcoredump/Synth_Dexed](https://codeberg.org/dcoredump/Synth_Dexed)
- **Contribution**: Teensy Audio Library integration for FM synthesis, efficient microcontroller implementation
- **About**: Six-operator FM synthesizer object specifically designed for microcontrollers

### Community Ports and Adaptations

This project directly incorporates Teensy implementations from community-driven ports:

#### **MicroDexed Touch - Teensy Synthesis Engines**
- **Project**: [MicroDexed Touch](https://www.synthtopia.com/content/2022/09/04/microdexed-touch-is-an-open-source-fm-groovebox/)
- **Direct Usage**: This project uses MicroDexed Touch's Teensy implementations of:
  - **MDA EPiano** - Electric piano synthesis engine ported to Teensy Audio Library
  - **Braids** - Macro oscillator synthesis engine adapted for Teensy hardware
  - **MicroDexed** - FM synthesis engine optimized for Teensy microcontrollers
- **Contribution**: Production-ready Teensy Audio Library integration, hardware-optimized synthesis code, and real-time performance enhancements
- **About**: Complete Teensy-based synthesizer platform that provided the foundation for our multi-engine approach

---

## 📄 License

This project is open source with multiple licenses depending on the synthesis engines used. Please see individual project folders and the acknowledgements above for specific license information. All original code contributions are licensed under GPL v3.0 unless otherwise specified.

---
*This project was developed with assistance from [Claude Code](https://code.claude.com/docs/en/overview).*
