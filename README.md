# Multi-Teensy Synth

A **multi-engine polyphonic synthesizer** built with the Teensy 4.1 microcontroller featuring three classic synthesis engines: **Virtual Analog** (Minimoog-inspired), **Juno-60** (Roland-inspired), and **DX7 FM** (Yamaha-inspired).

*"Three Classic Synths in One | A Teensy Powered Multi-Engine Tribute"*

## Key Features
- **Three synthesis engines** - switch between VA, Juno-60, and DX7 FM
- **6-voice polyphony** per engine
- **Virtual Analog Engine**: 3 oscillators, Moog-style filter, ADSR envelopes
- **Juno-60 Engine**: Roland character with chorus, high-pass filter, arpeggiator
- **DX7 FM Engine**: 6-operator FM synthesis, 32 algorithms, classic 80s sounds
- **Unified menu system** - single encoder controls all engines
- **USB Audio + MIDI** - single cable to computer
- **MIDI channel selection** - receive on specific channel or omni mode
- **20 hardware encoders** + LCD for real-time control
- **Multiple play modes** - Mono, Poly, Legato with glide

## Synthesis Engines

### Virtual Analog (Minimoog-inspired)
- **3 oscillators** with 6 waveforms each
- **24dB Moog-style ladder filter** 
- **Independent ADSR envelopes** for amp and filter
- **LFO** with pitch/filter/amp targets
- **Noise generator** (white/pink)

### Juno-60 (Roland-inspired) 
- **Modified VA engine** with Roland character
- **High-pass filter** for that classic Juno sound
- **Chorus effect** with variable depth
- **Softer filter response** compared to Moog
- **Arpeggiator** functionality

### DX7 FM (Yamaha-inspired)
- **6-operator FM synthesis**
- **32 classic algorithms** 
- **Operator-level control** 
- **LFO modulation**
- **Classic 80s electric pianos, brass, bells**

## Menu Navigation Structure

The synthesizer uses a **hierarchical menu system** accessible via the menu encoder and button. Press the menu button to enter the parent menu, then navigate to different sections:

### Parent Menu Structure
```
├── Engines
│   └── VA / Juno / DX7 (engine selection)
├── Presets  
│   └── Preset Browser (per engine)
├── Parameters
│   ├── Oscillator 1 (Range, Wave, Volume, Fine, Back)
│   ├── Oscillator 2 (Range, Wave, Volume, Fine, Back)  
│   ├── Oscillator 3 (Range, Wave, Volume, Fine, Back)
│   ├── Noise (Volume, Type, Back)
│   ├── Envelopes (Filter ADSR, Amp ADSR, Filter Strength, Back)
│   ├── Filter (Cutoff, Resonance, Back)
│   ├── LFO (Rate, Depth, Toggle, Target, Back)
│   └── Voice Mode (Play Mode, Glide Time, Back)
├── Effects
│   ├── Chorus Bypass (On/Off)
│   ├── Chorus Rate (0-1 Hz)
│   ├── Chorus Depth (0-100%)
│   ├── Reverb Bypass (On/Off) 
│   ├── Reverb Size (0-100%)
│   ├── Reverb Hi Damp (0-100%)
│   ├── Reverb Lo Damp (0-100%)
│   ├── Reverb Lowpass (0-100%)
│   └── Reverb Diffusion (0-100%)
└── Settings
    ├── Macro Knobs (Filter Env / LFO Controls)
    └── MIDI Channel (0=Omni, 1-16)
```

### Navigation Controls
- **Turn encoder**: Navigate through menu items
- **Press button**: Enter selected submenu or exit to parent level
- **Menu hierarchy**: Parent → Submenu → Parameter → Back to Submenu → Back to Parent
- **Individual effects bypass**: Chorus and reverb can be enabled/disabled independently
- **Real-time parameter updates**: All parameter changes are applied immediately

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

**Parts:**
- Teensy 4.1 (https://www.sparkfun.com/teensy-4-1-without-ethernet.html)
- Menu Encoder (https://www.amazon.com/Taiss-KY-040-Encoder-15×16-5-Arduino/dp/B07F26CT6B/ref=sr_1_3_pp)
- Other Encoders (https://www.aliexpress.us/item/3256801237549169.html)
- LCD 2X16 (https://www.amazon.com/Hosyond-Display-Module-Arduino-Raspberry/dp/B0BWTFN9WF/ref=sr_1_2)
- Knobs (https://www.amazon.com/Taiss-Silver-Rotary-Potentiometer-Diameter/dp/B07F25NMJ7/ref=sr_1_5)

## Wiring

### **Minimal Build (LCD + 1 Encoder)**
Perfect for testing or budget builds:
```
Menu Encoder: CLK→13, DT→14, SW→15
LCD (I2C):   SDA→18, SCL→19, VCC→3.3V, GND→GND
Power:       USB cable to computer
```
**That's it!** All synthesis parameters accessible through menu.

### **Full Build (20 Encoders)**
**Parameter Encoder Pins (configurable in code):**
```
Osc Ranges:  enc1(4,5), enc2(2,3), enc3(0,1)
Osc Fine:    enc4(8,9), enc5(6,7)
Osc Waves:   enc6(25,27), enc7(12,24), enc8(10,11)  
Volumes:     enc9(29,30), enc10(28,26), enc11(21,20)
Filter:      enc13(34,33), enc14(50,41), enc15(23,22), enc16(36,35)
Noise Vol:   enc17(31,32)
Amp Env:     enc18(17,16), enc19(38,37), enc20(40,39)
```

**Encoder Wiring:**
- **CLK/DT pins** to Teensy as shown above
- **All encoder GND pins** daisy-chained to Teensy GND
- **Menu encoder VCC pin** to Teensy 3.3V (if encoder has a breakout board)

### **Audio + MIDI Options**

**Option 1: Computer DAW (Current Setup)**
- Single USB cable provides audio output + MIDI input
- Plug-and-play with DAWs

**Option 2: Standalone with MIDI Keyboard**  
For standalone use without computer:
- Connect USB MIDI keyboard to **Teensy USB Host pins**
- Requires USB Host cable
- Audio output through computer USB (or modify code for I2S/line out)
- **Note:** Requires code modification for USB Host MIDI instead of USB Device

**Option 3: DIN MIDI Support (Hardware Modification)**
Add traditional 5-pin DIN MIDI input for hardware compatibility:

**Hardware Required:**
- 6N138 optocoupler IC  
- 220Ω resistor
- 5-pin DIN MIDI connector
- Standard MIDI interface circuit (see MIDI spec)

**Wiring:**
- Connect MIDI interface circuit output to **Teensy Serial1 (Pin 0)**
- MIDI input circuit connects to DIN connector pins 4,5
- **Important:** Pin 0 is currently used by enc3 (Osc3 Range). You'll need to move enc3's CLK wire from Pin 0 to one of the surface mount pins on the bottom of the Teensy 4.1 board (pins 42-47 are available)

**Code Changes Required:**
```cpp
// In Multi-Teensy-Synth.ino, uncomment this line:
#define ENABLE_DIN_MIDI

// That's it! The DIN MIDI support code is already included.
```

**Features:**
- Supports both USB MIDI and DIN MIDI simultaneously
- Selectable MIDI channel (1-16 or Omni) via Settings menu
- Standard MIDI implementation (Note On/Off, CC, Pitch Bend)

## Software Installation

1. **Install Arduino IDE** 2.0+ from [arduino.cc](https://www.arduino.cc/en/software)
2. **Add Teensy support:**
   - **Modern IDE:** Tools → Board Manager → Search "Teensy" → Install
   - **Fallback:** Download Teensyduino from [PJRC](https://www.pjrc.com/teensy/td_download.html)
3. **Install libraries:** Tools → Manage Libraries → Install "LiquidCrystal I2C" and "Encoder" 
   - **For DIN MIDI:** Also install "MIDI Library" by Francois Best 
4. **Download this project** and open `Multi-Teensy-Synth.ino`
5. **Configure:** Board→Teensy 4.1, USB Type→Audio+MIDI
6. **Upload!** Your computer will show "Teensy Audio" device

## Usage

**Engine Selection:** Main Menu → Engine Select → [VA | Juno-60 | DX7]

**Menu Navigation:** Click encoder to navigate, turn to adjust values. All parameters accessible via hierarchical menu system.

**MIDI Channel:** Settings → MIDI Channel sets which MIDI channel to receive (1-16, or Omni for all channels).

**Menu Structure:**
```
Engine Select → [Virtual Analog | Juno-60 | DX7 FM]
├── Engine Parameters (varies by selected engine)
│   ├── VA: OSC 1-3 | Noise | Envelopes | Filter | LFO
│   ├── Juno: OSC 1-3 | Chorus | HPF | Arp | Filter
│   └── DX7: Operators 1-6 | Algorithm | Feedback | LFO
└── Global Settings
    ├── MIDI Channel
    ├── Voice Mode
    └── Macro Knobs
```

## Development Status

**Current Version:** 1.0 Alpha
- ✅ Multi-engine framework implemented
- ✅ Virtual Analog engine (from Mini-Teensy-Synth)
- 🔄 Juno-60 engine (in development)
- 🔄 DX7 FM engine (in development) 
- 🔄 Menu system integration (in development)

**Planned Features:**
- Complete Juno-60 engine with chorus and arpeggiator
- DX7 FM synthesis with MicroDexed integration
- Preset system for all engines
- Cross-engine modulation capabilities

## Contributing
Fork, test on hardware, submit PR. Open source project!

## Thanks
- **PJRC** - Teensy platform and Audio Library
- **MicroDexed Team** - DX7 FM synthesis engine
- **Classic synth manufacturers** - Moog, Roland, Yamaha
- **Open source community**