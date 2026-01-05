#!/bin/bash

# Deploy Master Config Script
# This script copies the master config to all projects with the appropriate PROJECT_TYPE defined

echo "🚀 Deploying master config to all Multi-Teensy-Synth projects..."

# Base directory - automatically detect script location
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Check if config_master.h exists
if [ ! -f "$BASE_DIR/config_master.h" ]; then
    echo "❌ Error: config_master.h not found in $BASE_DIR"
    echo "Make sure you're running this script from the Multi-Teensy-Synth root directory."
    exit 1
fi

echo "📂 Working directory: $BASE_DIR"

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
deploy_to_project "MacroOSC-Teensy-Synth" "PROJECT_MACRO" "MacroOSC-Teensy-Synth"

echo ""
echo "🎉 Configuration deployment complete!"
