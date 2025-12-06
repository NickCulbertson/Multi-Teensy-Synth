# DX7 SysEx Bank Manager

**Ultra-simple DX7 bank loading for Multi-Teensy-Synth**

## How to Add DX7 Banks (3 Easy Steps)

1. **Drop .syx files here** - Copy your DX7 bank files (.syx) into this `sysex/` directory
2. **Run the script** - Execute: `python3 sysex2c.py`  
3. **Done!** - Your synth now has all the banks with automatic menu navigation

That's it! The script automatically:
- ✅ Finds all .syx files in the directory
- ✅ Converts them to the correct format
- ✅ Generates bank names from filenames  
- ✅ Updates the main header file
- ✅ No manual code changes needed!

## Example
```bash
# Add your .syx files to this directory, then:
cd sysex
python3 sysex2c.py

# Output: Processing 8 files and writing to ../dx7_rom1a_unpacked.h
# Your synth now has 8 banks accessible via Presets → Banks menu
```

## Where to Get DX7 Banks
- **Classic Banks**: ROM1A.syx, ROM1B.syx (original factory sounds)
- **Community**: [dexed GitHub](https://github.com/asb2m10/dexed) 
- **Online**: Search "DX7 sysex download"

## Menu Navigation
After updating banks, your synth will have:
```
Presets → DX7 Banks → [Your Bank Names] → Patches 1-32
```

Each bank contains 32 patches with real DX7 patch names displayed.