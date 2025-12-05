#!/usr/bin/env python3
"""
Extract specific DX7 patches from ROM1A.syx into individual C arrays
"""
import sys

# From the sysex2c.py script
def unpack_packed_patch(p):
    # Input is a 128 byte thing from compact.bin
    # Output is a 156 byte thing that the synth knows about
    o = [0]*156
    for op in range(6):
        o[op*21:op*21 + 11] = p[op*17:op*17+11]
        leftrightcurves = p[op*17+11]
        o[op * 21 + 11] = leftrightcurves & 3
        o[op * 21 + 12] = (leftrightcurves >> 2) & 3
        detune_rs = p[op * 17 + 12]
        o[op * 21 + 13] = detune_rs & 7
        o[op * 21 + 20] = detune_rs >> 3
        kvs_ams = p[op * 17 + 13]
        o[op * 21 + 14] = kvs_ams & 3
        o[op * 21 + 15] = kvs_ams >> 2
        o[op * 21 + 16] = p[op * 17 + 14]
        fcoarse_mode = p[op * 17 + 15]
        o[op * 21 + 17] = fcoarse_mode & 1
        o[op * 21 + 18] = fcoarse_mode >> 1
        o[op * 21 + 19] = p[op * 17 + 16]
    
    o[126:126+9] = p[102:102+9]
    oks_fb = p[111]
    o[135] = oks_fb & 7
    o[136] = oks_fb >> 3
    o[137:137+4] = p[112:112+4]
    lpms_lfw_lks = p[116]
    o[141] = lpms_lfw_lks & 1
    o[142] = (lpms_lfw_lks >> 1) & 7
    o[143] = lpms_lfw_lks >> 4
    o[144:144+11] = p[117:117+11]
    o[155] = 0x3f

    # Clamp the unpacked patches to a known max. 
    maxes =  [
        99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, # osc6
        3, 3, 7, 3, 7, 99, 1, 31, 99, 14,
        99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, # osc5
        3, 3, 7, 3, 7, 99, 1, 31, 99, 14,
        99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, # osc4
        3, 3, 7, 3, 7, 99, 1, 31, 99, 14,
        99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, # osc3
        3, 3, 7, 3, 7, 99, 1, 31, 99, 14,
        99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, # osc2
        3, 3, 7, 3, 7, 99, 1, 31, 99, 14,
        99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, # osc1
        3, 3, 7, 3, 7, 99, 1, 31, 99, 14,
        99, 99, 99, 99, 99, 99, 99, 99, # pitch eg rate & level 
        31, 7, 1, 99, 99, 99, 99, 1, 5, 7, 48, # algorithm etc
        126, 126, 126, 126, 126, 126, 126, 126, 126, 126, # name
        127 # operator on/off
    ]
    for i in range(156):
        if(o[i] > maxes[i]): o[i] = maxes[i]
        if(o[i] < 0): o[i] = 0
    return o

def print_header_data(voice_data, indent="  "):
    print(f"{indent}{{", end="")
    for y in range(len(voice_data)):
        if y % 12 == 0:
            print(f"\n{indent}  ", end="")
        if y != len(voice_data) - 1:
            print(f"{voice_data[y]:3d}, ", end="")
        else:
            print(f"{voice_data[y]:3d}", end="")
    print(f"\n{indent}}}", end="")

def extract_patches(sysex_file, patch_numbers):
    """Extract specific patch numbers from sysex file"""
    
    print("// DX7 ROM1A Selected Patches")
    print("// Extracted from ROM1A.syx")
    print("")
    
    with open(sysex_file, "rb") as f:
        # Read and validate header
        header = f.read(6)
        if header[0] != 240:
            print("Error: Start of sysex not found")
            return
        if header[1] != 67:
            print("Error: Not a Yamaha sysex file")
            return
        if header[3] != 9:
            print("Error: Not a 32 voice sysex file")
            return
            
        byte_count = header[4] * 128 + header[5]
        if byte_count != 4096:
            print("Error: Byte count mismatch")
            return
            
        # Read patch data
        patch_data = f.read(4096)
        
        print(f"uint8_t dx7_selected_patches[{len(patch_numbers)}][156] = {{")
        
        for i, patch_num in enumerate(patch_numbers):
            if patch_num < 1 or patch_num > 32:
                print(f"Error: Patch number {patch_num} out of range (1-32)")
                continue
                
            # Extract 128-byte packed data for this patch
            start_offset = (patch_num - 1) * 128
            packed_data = patch_data[start_offset:start_offset + 128]
            
            # Get patch name (bytes 118-127 in packed data)
            patch_name = str(packed_data[118:128].decode('ascii')).strip()
            
            # Unpack to 156-byte format
            unpacked_data = unpack_packed_patch(packed_data)
            
            print(f"  // Patch {patch_num}: {patch_name}")
            print_header_data(unpacked_data)
            
            if i < len(patch_numbers) - 1:
                print(",")
            else:
                print("")
                
        print("};")
        print("")
        print("// Patch names for reference:")
        for i, patch_num in enumerate(patch_numbers):
            start_offset = (patch_num - 1) * 128
            packed_data = patch_data[start_offset:start_offset + 128]
            patch_name = str(packed_data[118:128].decode('ascii')).strip()
            print(f"// {i}: {patch_name}")

if __name__ == "__main__":
    # Extract some popular patches from ROM1A
    # Patch numbers are 1-32 (not 0-31)
    popular_patches = [1, 4, 5, 11, 12, 13, 14, 15, 16]  # E.PIANO 1, GUITAR 1, GUITAR 2, etc.
    
    extract_patches("ROM1A.syx", popular_patches)