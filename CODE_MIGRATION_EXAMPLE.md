# Code Migration Example

This file shows exactly how to update your existing code to use the new unified configuration system.

## ✨ **THE UNIFIED APPROACH** - Use the same code in all projects!

The beauty of the new system is that you can use **identical code** across all projects. The configuration file automatically maps the generic names to the correct project-specific values.

### BEFORE (Old System):
```cpp
void handleControlChange(int cc, int value) {
  float paramValue = value / 127.0;
  
  if (cc == CC_MODWHEEL) {
    modWheelValue = paramValue;
    // ... mod wheel handling
    return;
  }
  
  int paramIndex = -1;
  
  if (cc == CC_OSC1_RANGE) paramIndex = 0;
  else if (cc == CC_OSC2_RANGE) paramIndex = 1;
  else if (cc == CC_OSC3_RANGE) paramIndex = 2;
  else if (cc == CC_OSC2_FINE) paramIndex = 3;
  else if (cc == CC_OSC3_FINE) paramIndex = 4;
  else if (cc == CC_OSC1_WAVE) paramIndex = 5;
  else if (cc == CC_OSC2_WAVE) paramIndex = 6;
  else if (cc == CC_OSC3_WAVE) paramIndex = 7;
  else if (cc == CC_OSC1_VOL) paramIndex = 8;
  else if (cc == CC_OSC2_VOL) paramIndex = 9;
  else if (cc == CC_OSC3_VOL) paramIndex = 10;
  else if (cc == CC_CUTOFF) paramIndex = 11;
  else if (cc == CC_RESONANCE) paramIndex = 12;
  // ... more parameters
  
  if (paramIndex >= 0) {
    updateParameterFromMenu(paramIndex, paramValue);
    if (!inMenu) {
      updateParameterDisplay(paramIndex, paramValue);
    }
  }
}
```

### AFTER (New Unified System) - **Same code works in ALL projects**:
```cpp
void handleControlChange(int cc, int value) {
  float paramValue = value / 127.0;
  
  // Standard MIDI CCs (shared across all projects) - NO CHANGE
  if (cc == CC_MODWHEEL) {
    modWheelValue = paramValue;
    // ... mod wheel handling  
    return;
  }
  
  int paramIndex = -1;
  
  // Unified MIDI CCs - SAME CODE FOR ALL PROJECTS!
  if (cc == CC_OSC1_RANGE) paramIndex = 0;
  else if (cc == CC_OSC2_RANGE) paramIndex = 1;
  else if (cc == CC_OSC3_RANGE) paramIndex = 2;
  else if (cc == CC_OSC2_FINE) paramIndex = 3;
  else if (cc == CC_OSC3_FINE) paramIndex = 4;
  else if (cc == CC_OSC1_WAVE) paramIndex = 5;
  else if (cc == CC_OSC2_WAVE) paramIndex = 6;
  else if (cc == CC_OSC3_WAVE) paramIndex = 7;
  else if (cc == CC_OSC1_VOL) paramIndex = 8;
  else if (cc == CC_OSC2_VOL) paramIndex = 9;
  else if (cc == CC_OSC3_VOL) paramIndex = 10;
  else if (cc == CC_CUTOFF) paramIndex = 11;           // CC 74 (shared)
  else if (cc == CC_RESONANCE) paramIndex = 12;        // CC 71 (shared)
  else if (cc == CC_FILTER_ATTACK) paramIndex = 13;
  else if (cc == CC_FILTER_DECAY) paramIndex = 14;
  else if (cc == CC_FILTER_SUSTAIN) paramIndex = 15;
  else if (cc == CC_NOISE_VOL) paramIndex = 16;
  else if (cc == CC_AMP_ATTACK) paramIndex = 17;
  else if (cc == CC_AMP_SUSTAIN) paramIndex = 18;
  else if (cc == CC_AMP_DECAY) paramIndex = 19;
  else if (cc == CC_OSC1_FINE) paramIndex = 20;
  else if (cc == CC_FILTER_STRENGTH) paramIndex = 21;
  else if (cc == CC_LFO_RATE) paramIndex = 22;
  else if (cc == CC_LFO_DEPTH) paramIndex = 23;
  
  if (paramIndex >= 0) {
    updateParameterFromMenu(paramIndex, paramValue);
    if (!inMenu) {
      updateParameterDisplay(paramIndex, paramValue);
    }
  }
}
```

**🎯 The magic:** `CC_OSC1_RANGE` automatically becomes:
- `CC_MINI_OSC1_RANGE` (CC 90) in Mini-Teensy-Synth  
- `CC_DCO_OSC1_RANGE` (CC 20) in DCO-Teensy-Synth
- And so on...

### **Same approach for different projects:**

### FM-Teensy-Synth (same code pattern):
```cpp
void handleControlChange(int cc, int value) {
  float paramValue = (float)value;
  
  // Standard MIDI CCs - SAME
  if (cc == CC_MODWHEEL) {
    modWheelValue = value / 127.0;
    return;
  }
  
  int paramIndex = -1;
  
  // FM-specific parameters - SAME NAMES
  if (cc == CC_ALGORITHM) paramIndex = 0;       // Auto-maps to CC_FM_ALGORITHM (CC 50)
  else if (cc == CC_FEEDBACK) paramIndex = 1;   // Auto-maps to CC_FM_FEEDBACK (CC 51)
  else if (cc == CC_LFO_SPEED) paramIndex = 2;  // Auto-maps to CC_FM_LFO_SPEED (CC 52)
  else if (cc == CC_MASTER_VOL) paramIndex = 3; // Auto-maps to CC_FM_MASTER_VOL (CC 53)
  else if (cc == CC_OP1_LEVEL) paramIndex = 4;  // Auto-maps to CC_FM_OP1_LEVEL (CC 54)
  else if (cc == CC_OP2_LEVEL) paramIndex = 5;  // Auto-maps to CC_FM_OP2_LEVEL (CC 55)
  // ... more FM parameters
  
  if (paramIndex >= 0) {
    updateParameterFromMenu(paramIndex, paramValue);
    if (!inMenu) {
      updateParameterDisplay(paramIndex, paramValue);
    }
  }
}
```

### MacroOscillator-Teensy-Synth (same code pattern):
```cpp
void handleControlChange(int cc, int value) {
  float paramValue = (float)value;
  
  // Standard MIDI CCs - SAME
  if (cc == CC_MODWHEEL) {
    modWheelValue = value / 127.0;
    return;
  }
  
  int paramIndex = -1;
  
  // Braids-specific parameters - SAME NAMES  
  if (cc == CC_SHAPE) paramIndex = 0;            // Auto-maps to CC_MACRO_SHAPE (CC 20)
  else if (cc == CC_TIMBRE) paramIndex = 1;      // Auto-maps to CC_MACRO_TIMBRE (CC 21)
  else if (cc == CC_COLOR) paramIndex = 2;       // Auto-maps to CC_MACRO_COLOR (CC 22)
  else if (cc == CC_COARSE) paramIndex = 3;      // Auto-maps to CC_MACRO_COARSE (CC 23)
  else if (cc == CC_AMP_ATTACK) paramIndex = 4;  // Auto-maps to CC_MACRO_AMP_ATTACK (CC 24)
  // ... more Braids parameters
  
  if (paramIndex >= 0) {
    updateParameterFromMenu(paramIndex, paramValue);
    if (!inMenu) {
      updateParameterDisplay(paramIndex, paramValue);
    }
  }
}
```

## DCO-Teensy-Synth handleControlChange() Function

### AFTER (New System):
```cpp
void handleControlChange(int cc, int value) {
  float paramValue = value / 127.0;
  
  // Standard MIDI CCs (shared) - NO CHANGE
  if (cc == CC_MODWHEEL) {
    modWheelValue = paramValue;
    return;
  }
  
  int paramIndex = -1;
  
  // DCO-specific MIDI CCs - USE CC_DCO_ PREFIX  
  if (cc == CC_DCO_OSC1_RANGE) paramIndex = 0;         // CC 20
  else if (cc == CC_DCO_OSC1_WAVE) paramIndex = 1;     // CC 21
  else if (cc == CC_DCO_OSC1_VOLUME) paramIndex = 2;   // CC 22
  else if (cc == CC_DCO_OSC2_RANGE) paramIndex = 3;    // CC 23
  else if (cc == CC_DCO_OSC2_WAVE) paramIndex = 4;     // CC 24
  else if (cc == CC_DCO_OSC2_VOLUME) paramIndex = 5;   // CC 25
  else if (cc == CC_DCO_OSC2_DETUNE) paramIndex = 6;   // CC 26
  else if (cc == CC_CUTOFF) paramIndex = 11;           // CC 74 (shared)
  else if (cc == CC_RESONANCE) paramIndex = 12;        // CC 71 (shared)
  // ... more DCO parameters
  
  if (paramIndex >= 0) {
    updateParameterFromMenu(paramIndex, paramValue);
    if (!inMenu) {
      updateParameterDisplay(paramIndex, paramValue);
    }
  }
}
```

## FM-Teensy-Synth handleControlChange() Function

### AFTER (New System):
```cpp
void handleControlChange(int cc, int value) {
  float paramValue = (float)value;
  
  // Standard MIDI CCs (shared) - NO CHANGE
  if (cc == CC_MODWHEEL) {
    modWheelValue = value / 127.0;
    return;
  }
  
  int paramIndex = -1;
  
  // FM-specific MIDI CCs - USE CC_FM_ PREFIX
  if (cc == CC_FM_ALGORITHM) paramIndex = 0;       // CC 50
  else if (cc == CC_FM_FEEDBACK) paramIndex = 1;   // CC 51
  else if (cc == CC_FM_LFO_SPEED) paramIndex = 2;  // CC 52
  else if (cc == CC_FM_MASTER_VOL) paramIndex = 3; // CC 53
  else if (cc == CC_FM_OP1_LEVEL) paramIndex = 4;  // CC 54
  else if (cc == CC_FM_OP2_LEVEL) paramIndex = 5;  // CC 55
  // ... more FM parameters
  
  if (paramIndex >= 0) {
    updateParameterFromMenu(paramIndex, paramValue);
    if (!inMenu) {
      updateParameterDisplay(paramIndex, paramValue);
    }
  }
}
```

## ⚙️ Encoder Mapping - Also Unified!

The encoder system works the same way. Use the generic names everywhere:

### BEFORE:
```cpp
// Different in each project
const int encoderMapping[19] = {
  ENC_1_PARAM_MINI, ENC_2_PARAM_MINI, ENC_3_PARAM_MINI, // etc...
};
```

### AFTER - **Same code in all projects:**
```cpp
// This works identically in all projects!
const int encoderMapping[19] = {
  ENC_1_PARAM, ENC_2_PARAM, ENC_3_PARAM, ENC_4_PARAM, ENC_5_PARAM,
  ENC_6_PARAM, ENC_7_PARAM, ENC_8_PARAM, ENC_9_PARAM, ENC_10_PARAM,
  ENC_11_PARAM, ENC_13_PARAM, ENC_14_PARAM, ENC_15_PARAM, ENC_16_PARAM,
  ENC_17_PARAM, ENC_18_PARAM, ENC_19_PARAM, ENC_20_PARAM
};

// Menu encoder also unified
#define MENU_ENCODER_PARAM  MENU_ENCODER_PARAM  // Auto-maps per project
```

## 🖨️ Update Serial Output Messages - Use Unified Names

### BEFORE:
```cpp
Serial.print("  Osc1 Range: CC"); Serial.println(CC_MINI_OSC1_RANGE); // Different for each project!
```

### AFTER - **Same code in all projects:**
```cpp
Serial.print("  Osc1 Range: CC"); Serial.println(CC_OSC1_RANGE);     // Auto-maps!
Serial.print("  Cutoff: CC"); Serial.println(CC_CUTOFF);             // Shared CC unchanged
Serial.print("  Mod Wheel: CC"); Serial.println(CC_MODWHEEL);        // Shared CC unchanged
```

**🎯 The magic:** `CC_OSC1_RANGE` prints the correct CC number for each project automatically:
- Mini: prints "90" 
- DCO: prints "20"
- FM: N/A (not applicable)
- Macro: N/A (uses CC_SHAPE instead)

## ✅ Key Benefits Summary

1. **Same code works everywhere** - Copy/paste between projects!
2. **Automatic CC mapping** - No more manual CC number tracking
3. **Shared standard CCs** - CC_MODWHEEL, CC_VOLUME, etc. work the same
4. **Unified encoder system** - ENC_1_PARAM works everywhere
5. **Easy maintenance** - Change config once, affects all projects
6. **No conflicts** - Each project gets its own CC range automatically

## 🚀 Migration Steps

1. **Deploy new configs:** `./deploy_config.sh`
2. **Replace CC names** with unified versions:
   - `CC_MINI_OSC1_RANGE` → `CC_OSC1_RANGE`
   - `CC_DCO_OSC1_RANGE` → `CC_OSC1_RANGE` 
   - `CC_FM_ALGORITHM` → `CC_ALGORITHM`
   - etc.
3. **Use unified encoder names:**
   - `ENC_1_PARAM_MINI` → `ENC_1_PARAM`
   - `MENU_ENCODER_PARAM_MINI` → `MENU_ENCODER_PARAM`
4. **Test each project** to ensure MIDI CC mappings work correctly

## 🎼 Unified Parameter Reference

| Generic Name | Mini (CC 90+) | DCO (CC 20+) | FM (CC 50+) | EPiano (CC 70+) | Macro (CC 20+) |
|--------------|---------------|--------------|-------------|-----------------|----------------|
| `CC_OSC1_RANGE` | CC 90 | CC 20 | N/A | N/A | N/A |
| `CC_OSC2_RANGE` | CC 91 | CC 23 | N/A | N/A | N/A |
| `CC_ALGORITHM` | N/A | N/A | CC 50 | N/A | N/A |
| `CC_SHAPE` | N/A | N/A | N/A | N/A | CC 20 |
| `CC_VELOCITY_SENS` | N/A | N/A | N/A | CC 70 | N/A |
| `CC_LFO_RATE` | CC 110 | CC 28 | N/A | N/A | CC 33 |

**Standard CCs (same everywhere):**
| Generic Name | All Projects |
|--------------|-------------|
| `CC_MODWHEEL` | CC 1 |
| `CC_VOLUME` | CC 7 |
| `CC_SUSTAIN` | CC 64 |
| `CC_CUTOFF` | CC 74 |
| `CC_RESONANCE` | CC 71 |

## MIDI CC Ranges Reference

| Project | Range | Examples |
|---------|-------|----------|
| **EPiano** | CC 70-89 | CC_EPIANO_VELOCITY_SENS (70), CC_EPIANO_CHORUS_RATE (74) |
| **DCO** | CC 20-49 | CC_DCO_OSC1_RANGE (20), CC_DCO_OSC2_RANGE (23) |  
| **FM** | CC 50-69 | CC_FM_ALGORITHM (50), CC_FM_OP1_LEVEL (54) |
| **Mini** | CC 90-119 | CC_MINI_OSC1_RANGE (90), CC_MINI_OSC2_RANGE (91) |
| **Macro** | CC 20-49 | CC_MACRO_SHAPE (20), CC_MACRO_TIMBRE (21) |
| **Shared** | Various | CC_MODWHEEL (1), CC_VOLUME (7), CC_CUTOFF (74), CC_RESONANCE (71) |

Note: DCO and Macro share the same CC range (20-49) but they're separate projects, so no conflicts occur in practice.