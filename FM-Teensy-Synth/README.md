# FM-Teensy-Synth v1.0

A polyphonic FM synthesizer built with the Teensy 4.1 microcontroller, using the Dexed DX7-compatible sound engine with intuitive menu control and configurable encoder mapping.

## Features

- **DX7-Compatible FM Synthesis** - Full Dexed implementation with authentic DX7 sound
- **32 Built-in Presets** - Classic DX7 ROM sounds from original cartridges
- **10 Real-time Parameters** - Essential FM controls mapped to encoders
- **Configurable Hardware** - Easily remap any encoder to any parameter
- **Menu System** - Browse presets and edit parameters via LCD/OLED display
- **USB/DIN MIDI** - Full MIDI support with mod wheel and pitch bend
- **16-Voice Polyphony** - Professional polyphonic performance

## Hardware Requirements

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

## Software Requirements

### Arduino Libraries (install via Library Manager)
- **LiquidCrystal I2C** (by Frank de Brabander) - for LCD display
- **Adafruit SSD1306** (by Adafruit) - for OLED display
- **Adafruit GFX Library** (by Adafruit) - for OLED display  
- **Encoder** (by Paul Stoffregen)
- **MIDI Library** (by Francois Best) - only needed if enabling DIN MIDI

### Built-in Teensy Libraries (no installation needed)
- Audio, Wire, USBHost_t36

## FM Parameters

The following 10 essential FM parameters are available for real-time control:

| Parameter | Range | Description |
|-----------|-------|-------------|
| Algorithm | 0-31 | Operator routing configuration |
| Feedback | 0-7 | Operator feedback amount |
| LFO Speed | 0-99 | Low frequency oscillator rate |
| Master Vol | 0-200% | Overall synth volume with boost |
| OP1 Level | 0-99 | Operator 1 output level |
| OP2 Level | 0-99 | Operator 2 output level |
| OP3 Level | 0-99 | Operator 3 output level |
| OP4 Level | 0-99 | Operator 4 output level |
| OP5 Level | 0-99 | Operator 5 output level |
| OP6 Level | 0-99 | Operator 6 output level |

## Default Encoder Mapping

| Encoder | Parameter | Description |
|---------|-----------|-------------|
| enc1 | Algorithm | Most important FM parameter |
| enc2 | Feedback | Classic FM feedback control |
| enc3 | LFO Speed | Vibrato/tremolo rate |
| enc4 | Master Vol | Overall volume control |
| enc5 | OP1 Level | Operator 1 volume |
| enc6 | OP2 Level | Operator 2 volume |
| enc7 | OP3 Level | Operator 3 volume |
| enc8 | OP4 Level | Operator 4 volume |
| enc9 | OP5 Level | Operator 5 volume |
| enc10 | OP6 Level | Operator 6 volume |
| enc11-20 | Disabled | Available for custom mapping |

## Configuration

### USB MIDI Mode Selection

Choose your MIDI connection type in `config.h`:

**Option 1: USB Device Mode (DEFAULT)**
```cpp
// #define USE_USB_MIDI_HOST   // Comment out this line
// #define USE_USB_MIDI_DEVICE // Comment out this line  
```
- **Use when:** Connecting Teensy to computer via USB
- **Setup:** Teensy → USB Cable → Computer
- **MIDI:** Computer sends MIDI to Teensy
- **Audio:** Computer receives USB audio from Teensy

**Option 2: USB Host Mode**
```cpp
#define USE_USB_MIDI_HOST     // Uncomment this line
// #define USE_USB_MIDI_DEVICE // Keep commented out
```
- **Use when:** Connecting USB MIDI controller directly to Teensy
- **Setup:** USB MIDI Controller → USB Cable → Teensy USB Host Port
- **MIDI:** Controller sends MIDI directly to Teensy
- **Audio:** Teensy outputs to headphones/speakers (no USB audio to computer)

### Encoder Remapping
Edit `config.h` to customize which encoders control which parameters:

```cpp
#define ENC_1_PARAM    0   // Algorithm
#define ENC_2_PARAM    1   // Feedback  
#define ENC_3_PARAM    2   // LFO Speed
// ... customize as needed
#define ENC_11_PARAM   -1  // Disabled
```

### Menu Encoder Options
Configure the menu encoder behavior:

```cpp
// Traditional mode - button press to enter menu
#define MENU_ENCODER_PARAM  0   // Controls Algorithm when not in menu

// Menu-only mode - turning encoder auto-enters menu
#define MENU_ENCODER_PARAM  -1  // No parameter control
```

### Display Selection
Choose your display type in `config.h`:

```cpp
#define USE_LCD_DISPLAY     // 16x2 I2C LCD
// #define USE_OLED_DISPLAY // 128x64 I2C OLED
```

## DIN MIDI Setup (Optional)

To add DIN MIDI input:

1. **Resolve pin conflict**: Move enc3 from pin 0 to pins 42-43 in `config.h`
2. **Build MIDI circuit**: 5-pin DIN → 6N138 optocoupler → 220Ω resistor → Pin 0
3. **Enable in code**: Uncomment `#define ENABLE_DIN_MIDI` in `config.h`
4. **Install library**: "MIDI Library" by Francois Best

## Usage

### Basic Operation
- **Play**: Connect USB MIDI controller or use DIN MIDI
- **Change presets**: Enter menu → Presets → Select Bank → Select Patch → Press to load
- **Edit parameters**: Turn any mapped encoder for real-time control
- **Menu navigation**: Press menu encoder to enter/exit menu system

### Preset Management
- **8 ROM banks** with 32 presets each from classic DX7 cartridges
- **Bank/patch browsing** - select bank first, then browse patches within that bank
- **Preset names** displayed from original ROM data
- **MIDI program change** (0-127) maps to presets 0-31 in current bank

### Parameter Control
- **Real-time editing**: Turn encoders while playing for immediate response
- **Menu editing**: Enter parameter menu for precise value adjustment
- **MIDI control**: Mod wheel and pitch bend automatically routed to FM engine

## Technical Notes

- **Audio**: 16-bit/44.1kHz USB audio output (mono to stereo)
- **Polyphony**: 16 voices maximum
- **Latency**: Low-latency real-time parameter updates
- **Memory**: Uses Teensy 4.1 PROGMEM for ROM storage
- **Compatibility**: Full DX7 SysEx compatibility via Dexed engine

## Troubleshooting

- **No audio**: Check USB connection and Audio Library memory allocation
- **MIDI issues**: Verify MIDI channel settings and USB MIDI driver
- **Display problems**: Check I2C connections and display type in config.h
- **DIN MIDI conflicts**: Ensure enc3 is moved away from pin 0

## Credits

- **Dexed FM Engine**: DX7 synthesis implementation
- **DX7 ROM Data**: Classic preset sounds
- **Mini-Teensy Hardware**: Encoder-based control surface
- **Teensy Audio Library**: High-performance audio processing