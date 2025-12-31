# Multi-Teensy-Synth Unified Configuration System

This unified configuration system allows you to maintain consistent settings across all synthesizer projects while providing project-specific customization.

## 🚀 Quick Start

### 1. Deploy Configurations
Run the deployment script to copy the master config to all projects:
```bash
./deploy_config.sh
```

### 2. Update Your Code
Each project now needs to use project-specific CC defines. Here's what to change:

#### Before (Old System):
```cpp
if (cc == CC_OSC1_RANGE) paramIndex = 0;
else if (cc == CC_OSC2_RANGE) paramIndex = 1;
```

#### After (New System):
```cpp
// Mini-Teensy-Synth example:
if (cc == CC_MINI_OSC1_RANGE) paramIndex = 0;
else if (cc == CC_MINI_OSC2_RANGE) paramIndex = 1;

// DCO-Teensy-Synth example:
if (cc == CC_DCO_OSC1_RANGE) paramIndex = 0;
else if (cc == CC_DCO_OSC2_RANGE) paramIndex = 1;
```

## 📋 MIDI CC Assignments

Each project has its own CC range to avoid conflicts:

| Project | CC Range | Example Parameters |
|---------|----------|-------------------|
| **EPiano** | 70-89 | CC_EPIANO_VELOCITY_SENS (70), CC_EPIANO_CHORUS_RATE (74) |
| **DCO** | 20-49 | CC_DCO_OSC1_RANGE (20), CC_DCO_OSC2_RANGE (23) |
| **FM** | 50-69 | CC_FM_ALGORITHM (50), CC_FM_OP1_LEVEL (54) |
| **Mini** | 90-119 | CC_MINI_OSC1_RANGE (90), CC_MINI_OSC2_RANGE (91) |
| **Macro** | 20-49 | CC_MACRO_SHAPE (20), CC_MACRO_TIMBRE (21) |

**Standard CCs** (shared across all projects):
- CC 1: Mod Wheel
- CC 7: Volume  
- CC 64: Sustain Pedal
- CC 71: Filter Resonance
- CC 74: Filter Cutoff

## 🔧 Configuration Options

### Audio Output
```cpp
// Choose one:
#define USE_USB_AUDIO      // USB Audio (default)
// #define USE_TEENSY_DAC   // Teensy Audio Shield/I2S DAC
```

### MIDI Input
```cpp
#define USE_USB_DEVICE_MIDI // USB Device MIDI (default)
#define USE_MIDI_HOST       // USB Host MIDI  
// #define USE_DIN_MIDI     // DIN MIDI (requires hardware)
```

### Display Type
```cpp
// Choose one:
#define USE_LCD_DISPLAY     // 16x2 LCD (default)
// #define USE_OLED_DISPLAY // OLED display
```

## 📁 File Structure

```
Multi-Teensy-Synth/
├── config_master.h              # Master configuration (edit this)
├── deploy_config.sh             # Deployment script  
├── CONFIG_SYSTEM_README.md      # This file
├── EPiano-Teensy-Synth/
│   └── config.h                 # Auto-generated (PROJECT_EPIANO)
├── DCO-Teensy-Synth/
│   └── config.h                 # Auto-generated (PROJECT_DCO)
├── FM-Teensy-Synth/
│   └── config.h                 # Auto-generated (PROJECT_FM)
├── Mini-Teensy-Synth/
│   └── config.h                 # Auto-generated (PROJECT_MINI)
└── MacroOscillator-Teensy-Synth/
    └── config.h                 # Auto-generated (PROJECT_MACRO)
```

## ✏️ Making Changes

### To update all projects:
1. Edit `config_master.h` 
2. Run `./deploy_config.sh`
3. All project configs are updated automatically

### To update just one project:
1. Edit the specific project's `config.h` directly
2. **OR** edit `config_master.h` and redeploy

### Adding a new parameter:
1. Add the CC define in the appropriate project section of `config_master.h`:
```cpp
#ifdef PROJECT_MINI
#define CC_MINI_NEW_PARAM     (CC_BASE + 22)  // CC 112
#endif
```
2. Add the encoder mapping:
```cpp
#define ENC_21_PARAM_MINI   22  // NEW_PARAM
```
3. Run `./deploy_config.sh`

## 🎛️ Encoder Mapping

Each project defines which encoder controls which parameter:

```cpp
// Example for Mini-Teensy-Synth:
#define ENC_1_PARAM_MINI    0   // OSC1_RANGE (enc1 → parameter 0)
#define ENC_2_PARAM_MINI    1   // OSC2_RANGE (enc2 → parameter 1)  
#define ENC_3_PARAM_MINI    2   // OSC3_RANGE (enc3 → parameter 2)
// etc...
```

Use `-1` to disable an encoder:
```cpp
#define ENC_20_PARAM_MINI   -1  // Encoder 20 disabled
```

## 🔄 Migration Guide

### Step 1: Backup Your Current Configs
```bash
cp EPiano-Teensy-Synth/config.h EPiano-config-backup.h
cp DCO-Teensy-Synth/config.h DCO-config-backup.h
# etc...
```

### Step 2: Deploy New System
```bash
./deploy_config.sh
```

### Step 3: Update Your Code

For each project, update the `handleControlChange` function:

**Mini-Teensy-Synth Example:**
```cpp
// OLD:
if (cc == CC_OSC1_RANGE) paramIndex = 0;
else if (cc == CC_OSC2_RANGE) paramIndex = 1;
else if (cc == CC_CUTOFF) paramIndex = 11;

// NEW: 
if (cc == CC_MINI_OSC1_RANGE) paramIndex = 0;
else if (cc == CC_MINI_OSC2_RANGE) paramIndex = 1;
else if (cc == CC_CUTOFF) paramIndex = 11; // Shared CC stays the same
```

**DCO-Teensy-Synth Example:**
```cpp
// NEW:
if (cc == CC_DCO_OSC1_RANGE) paramIndex = 0;
else if (cc == CC_DCO_OSC2_RANGE) paramIndex = 3;
else if (cc == CC_CUTOFF) paramIndex = 11; // Shared CC
```

### Step 4: Update Serial Output Messages
```cpp
// OLD:
Serial.print("  Osc1 Range: CC"); Serial.println(CC_OSC1_RANGE);

// NEW (Mini example):
Serial.print("  Osc1 Range: CC"); Serial.println(CC_MINI_OSC1_RANGE);
```

## 🎵 MIDI Controller Setup

Update your MIDI controller mappings:

| Controller Knob | EPiano | DCO | FM | Mini | Macro |
|-----------------|--------|-----|----|----|-------|
| Knob 1 | CC 70 | CC 20 | CC 50 | CC 90 | CC 20 |
| Knob 2 | CC 71 | CC 21 | CC 51 | CC 91 | CC 21 |
| Knob 3 | CC 72 | CC 22 | CC 52 | CC 92 | CC 22 |
| Knob 4 | CC 73 | CC 23 | CC 53 | CC 93 | CC 23 |

## ⚡ Benefits

✅ **No CC conflicts** between projects  
✅ **Easy to maintain** - edit once, deploy everywhere  
✅ **Consistent pin assignments** across all projects  
✅ **Project-specific customization** when needed  
✅ **Clear documentation** of all parameters  
✅ **Scalable** for future projects  

## 🐛 Troubleshooting

**Problem:** MIDI CC not working after migration  
**Solution:** Check that you updated the CC define names in your `handleControlChange()` function

**Problem:** Encoder not working  
**Solution:** Verify the encoder mapping in your project's section of config.h

**Problem:** Compilation errors  
**Solution:** Ensure only one `PROJECT_*` define is uncommented in config.h

**Need help?** Check the individual project config.h files to see the auto-generated parameter mappings.