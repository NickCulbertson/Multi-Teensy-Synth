# Synth_MDA_EPiano

EPiano synth (port of MDAEPiano) Teensy audio library

This library is the extraction of the MDA-EPiano sound engine from https://sourceforge.net/projects/mda-vst/

## API:

    void noteOn(int32_t note, int32_t velocity);
    void noteOff(int32_t note);
    bool processMidiController(uint8_t data1, uint8_t data2);
    void setProgram(uint8_t program);
    void resumeVoices();
    void resetVoices(void);
    void resetControllers(void);
    void stopVoices(void);
    void setDecay(float value);
    float getDecay(void);
    void setRelease(float value);
    float getRelease(void);
    void setHardness(float value);
    float getHardness(void);
    void setTreble(float value);
    float getTreble(void);
    void setPanTremolo(float value);
    float getPanTremolo(void);
    void setPanLFO(float value);
    float getPanLFO(void);
    void setVelocitySense(float value);
    float getVelocitySense(void);
    void setStereo(float value);
    float getStereo(void);
    void setPolyphony(uint8_t value);
    uint8_t getPolyphony(void);
    void setTune(float value);
    float getTune(void);
    void setDetune(float value);
    float getDetune(void);
    void setOverdrive(float value);
    float getOverdrive(void);
    void setVolume(float value);
    float getVolume(void);
    int32_t getActiveVoices(void);

