#!/bin/bash

# Deploy Master Config Script
# This script copies the master config to all projects with the appropriate PROJECT_TYPE defined

echo "🚀 Deploying master config to all Multi-Teensy-Synth projects..."

# Base directory
BASE_DIR="/Users/nickculbertson/Documents/Multi-Teensy-Synth"

# Function to deploy config to a specific project
deploy_to_project() {
    local project_dir="$1"
    local project_define="$2"
    local project_name="$3"
    
    if [ -d "$BASE_DIR/$project_dir" ]; then
        echo "📁 Deploying to $project_name..."
        
        # Copy master config
        cp "$BASE_DIR/config_master.h" "$BASE_DIR/$project_dir/config.h"
        
        # Uncomment the appropriate PROJECT_TYPE define
        sed -i.bak "s|// #define $project_define|#define $project_define|g" "$BASE_DIR/$project_dir/config.h"
        
        # Remove the backup file created by sed
        rm "$BASE_DIR/$project_dir/config.h.bak"
        
        echo "✅ $project_name config updated with $project_define"
    else
        echo "⚠️  Directory $project_dir not found, skipping..."
    fi
}

# Deploy to each project
deploy_to_project "EPiano-Teensy-Synth" "PROJECT_EPIANO" "EPiano-Teensy-Synth"
deploy_to_project "DCO-Teensy-Synth" "PROJECT_DCO" "DCO-Teensy-Synth"  
deploy_to_project "FM-Teensy-Synth" "PROJECT_FM" "FM-Teensy-Synth"
deploy_to_project "Mini-Teensy-Synth" "PROJECT_MINI" "Mini-Teensy-Synth"
deploy_to_project "MacroOscillator-Teensy-Synth" "PROJECT_MACRO" "MacroOscillator-Teensy-Synth"

echo ""
echo "🎉 Configuration deployment complete!"
echo ""
echo "📋 MIDI CC Ranges assigned:"
echo "   • EPiano: CC 70-89"
echo "   • DCO:    CC 20-49"  
echo "   • FM:     CC 50-69"
echo "   • Mini:   CC 90-119"
echo "   • Macro:  CC 20-49 (separate project)"
echo ""
echo "📝 Next steps:"
echo "1. Update your .ino files to use the new CC defines (e.g., CC_MINI_OSC1_RANGE instead of CC_OSC1_RANGE)"
echo "2. Test each project to ensure MIDI CC mappings work correctly"
echo "3. Update any hardcoded CC numbers in your MIDI controller setup"