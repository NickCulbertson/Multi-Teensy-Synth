# Multi-Teensy-Synth Collection (WIP)

**🚧 WORK IN PROGRESS 🚧**  
This project is very much under construction. More engines and proper attributions are being included. This is just a preview.

A collection of **standalone polyphonic synthesizers** built with the Teensy 4.1 microcontroller, each featuring different synthesis engines and optimized for the Mini-Teensy hardware platform.

**Built on open-source synthesis engines:** [MicroDexed Touch](https://codeberg.org/positionhigh/MicroDexed-touch), [Mutable Instruments Braids](https://github.com/pichenettes/eurorack), [MDA EPiano](https://sourceforge.net/projects/mda-vst/), and [MicroDexed](https://codeberg.org/dcoredump/MicroDexed). *See acknowledgements below for full attribution.*

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
- Authentic electric piano sound modeling
- 8 classic electric piano programs
- Real-time parameter control (decay, release, hardness, treble, etc.)
- Configurable encoder mapping for intuitive control
- High-quality 16-bit audio output

### 3. FM-Teensy-Synth
**16-Voice FM Synthesizer** (DX7-compatible via Dexed engine)
- Full DX7 compatibility with 6-operator FM synthesis
- 8 ROM banks with 32 presets each (256 total classic DX7 sounds)
- Real-time control of all 6 operator levels + algorithm, feedback, LFO
- Bank/patch browsing system like original DX7
- Authentic DX7 sound engine with multiple algorithms

### 4. DCO-Teensy-Synth
**6-Voice DCO Synthesizer** (Juno-60 inspired)
- PWM + Sawtooth oscillators with sub-oscillator
- 24dB ladder filter + 12dB high-pass filter
- Authentic BBD Chorus with stereo imaging
- Independent LFO with multiple modulation targets
- 6 classic Juno-style presets

### 5. MacroOscillator-Teensy-Synth
**6-Voice Digital Synthesizer** (Braids engine)
- 40+ synthesis algorithms and models
- Macro oscillator concepts with timbre control
- Digital synthesis techniques from Eurorack
- Real-time parameter morphing
- Advanced synthesis algorithms

## 🛠 Hardware Requirements

### Required Components
- **Teensy 4.1** microcontroller
- **Mini-Teensy hardware** (19 rotary encoders + menu encoder)
- **16x2 LCD** or **128x64 OLED** display
- **USB connection** for MIDI and audio

### Optional Components
- **DIN MIDI Input** circuit (6N138 optocoupler + 220Ω resistor + 5-pin DIN)
- **USB MIDI Host** setup:
  - USB OTG adapter cable or USB Host pins on Teensy 4.1
  - USB MIDI controller (keyboard, pad controller, etc.)
  - External 5V power supply if using high-power USB devices

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

1. **Choose a synthesizer** from the three available options
2. **Install required libraries** via Arduino Library Manager
3. **Configure hardware** in `config.h`:
   - Set display type (LCD or OLED)
   - Configure MIDI sources
   - Map encoders to desired parameters
4. **Upload to Teensy 4.1**
5. **Connect MIDI controller** and start playing!

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
│   ├── mdaEPiano.h/.cpp        # MDA EPiano sound engine
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

## 🎯 Features

- **Professional Audio Quality** - 16-bit/44.1kHz USB audio output
- **Low Latency** - Optimized for real-time performance
- **Polyphonic** - Multiple voices per synthesizer (6-16 depending on engine)
- **MIDI Compatible** - Full MIDI support with multiple input options
- **User Friendly** - Intuitive menu systems and configurable controls
- **Standalone Operation** - No computer required with USB Host MIDI
- **Expandable** - Easy to add new synthesis engines or modify existing ones

## 🔊 Audio Output

All synthesizers output high-quality audio via:
- **USB Audio** - Connect to computer for recording/monitoring
- **I2S Audio** - Connect to external DAC for standalone operation (hardware modification required)

## 🎵 Use Cases

- **Studio Recording** - Professional-quality synthesis for music production
- **Live Performance** - Reliable standalone operation with direct MIDI control
- **Sound Design** - Real-time parameter tweaking for creative sound exploration
- **Education** - Learn synthesis concepts with hands-on control
- **Prototyping** - Platform for developing new synthesis algorithms

## 🤝 Contributing

Feel free to contribute new synthesis engines, improvements, or bug fixes. Each synthesizer is standalone, making it easy to add new engines or enhance existing ones.

## 🏆 Acknowledgements - Standing on the Shoulders of Giants

This project stands proudly on the shoulders of giants. The synthesizer ecosystem thrives because of the incredible work done by passionate developers, companies, and communities who have shared their knowledge and code with the world. Without these foundational projects, this Multi-Teensy synthesis collection would not exist.

### Core Platform
- **[PJRC](https://www.pjrc.com/)** - Teensy 4.1 microcontroller platform and the incredible Teensy Audio Library that makes real-time audio synthesis possible on embedded hardware
- **[Roland Corporation](https://www.roland.com/)** - For creating legendary synthesizers like the Juno-60 (1982) and DX7 that inspired these project's sound designs and architectures

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

#### **Additional Community Resources**
- **[teensy-braids](https://github.com/modlfo/teensy-braids)** - Community Teensy port of Braids
- **[burns.ca/eurorack](https://burns.ca/eurorack.html)** - Excellent Eurorack analysis and documentation  
- **LV2 MDA ports** by [nphilipp](https://github.com/nphilipp/lv2-mda) and community contributors

### Hardware Innovation
- **[MiniDexed Touch Hardware](https://www.synthtopia.com/content/2022/09/04/microdexed-touch-is-an-open-source-fm-groovebox/)** - Hardware interface design and user experience inspiration
- **Community hardware builders** - Countless makers who have shared build guides, troubleshooting, and improvements

### Development Philosophy

The open-source synthesizer community embodies a unique spirit of collaboration, where:
- **Code is shared freely** - Allowing others to learn, modify, and improve
- **Hardware designs are open** - Enabling anyone to build and modify instruments
- **Knowledge is documented** - Creating resources for future generations of builders
- **Attribution is respected** - Honoring the work of original creators

### Our Commitment

We are committed to:
- **Respecting all licenses** - Ensuring proper attribution and compliance
- **Contributing back** - Sharing improvements and bug fixes with upstream projects
- **Educating makers** - Providing clear documentation and build guides
- **Supporting the community** - Helping others learn and build their own instruments

### A Living Tribute

This project is not just a collection of synthesizers—it's a living tribute to the incredible work done by synthesizer pioneers, both in hardware and software. Every note played is a reminder of the collective genius that makes these sounds possible.

**Thank you to everyone who has contributed to the open-source synthesizer ecosystem. Your work lives on in every beat, every melody, and every sonic exploration made possible by this project.**

---

## 📄 License

This project is open source with multiple licenses depending on the synthesis engines used. Please see individual project folders and the acknowledgements above for specific license information. All original code contributions are licensed under GPL v3.0 unless otherwise specified.
