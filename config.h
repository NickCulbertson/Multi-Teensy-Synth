#ifndef CONFIG_H
#define CONFIG_H

// Hardware Configuration
#define VOICES 6
#define MENU_ENCODER_CLK 13
#define MENU_ENCODER_DT 14
#define MENU_ENCODER_SW 15

// Audio Configuration
#define AUDIO_MEMORY_BLOCKS 48
#define FLANGE_DELAY_LENGTH (6*AUDIO_BLOCK_SAMPLES)

// Parameter Configuration
#define NUM_PARAMETERS 41
#define NUM_VA_PARAMETERS 39
#define NUM_JUNO_PARAMETERS 21  // Juno-specific parameters (individual volumes + LFO + filter + envelopes)
#define NUM_BRAIDS_PARAMETERS 17  // Braids-specific parameters (added filter ADSR + volume)

// Display Configuration (choose one)
// #define USE_LCD_DISPLAY
#define USE_OLED_DISPLAY

// Optional Features
// #define ENABLE_DIN_MIDI
// #define MINIMAL_BUILD

// Engine Types
typedef enum {
  ENGINE_VA = 0,
  ENGINE_DX7 = 1,
  ENGINE_JUNO = 2, 
  ENGINE_BRAIDS = 3
} EngineType;

// Preset Configuration
#define NUM_PRESETS 20
#define NUM_JUNO_PRESETS 7
#define NUM_BRAIDS_PRESETS 8

#endif // CONFIG_H