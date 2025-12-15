/*
   MicroMDAEPiano

   MicroMDAEPiano is a port of the MDA-EPiano sound engine
   (https://sourceforge.net/projects/mda-vst/) for the Teensy-3.5/3.6 with audio shield.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#ifndef __mdaEPiano__
#define __mdaEPiano__

#include <Audio.h>
#include <Arduino.h>
#include <string.h>
#include "synth_mda_epiano.h"

#define NPARAMS 12       //number of parameters
#define NPROGS   8       //number of programs
#define NOUTS    2       //number of outputs
#define SUSTAIN 128
#define SILENCE 0.0001f  //voice choking
#define WAVELEN 422414   //wave data bytes

// MDAEPiano parameter mapping
#define MDA_EP_DECAY 0
#define MDA_EP_RELEASE 1
#define MDA_EP_HARDNESS 2
#define MDA_EP_TREBLE 3
#define MDA_EP_PAN_TREM 4
#define MDA_EP_LFO_RATE 5
#define MDA_EP_VELOCITY_SENSE 6
#define MDA_EP_STEREO 7
#define MDA_EP_MAX_POLY 8
#define MDA_EP_TUNE 9
#define MDA_EP_DETUNE 10
#define MDA_EP_OVERDRIVE 11

class mdaEPianoProgram
{
    friend class mdaEPiano;
  private:
    float param[NPARAMS];
    char  name[24];
};

struct VOICE  //voice state
{
  int32_t  delta;  //sample playback
  int32_t  frac;
  int32_t  pos;
  int32_t  end;
  int32_t  loop;

  float env;  //envelope
  float dec;

  float f0;   //first-order LPF
  float f1;
  float ff;

  float outl;
  float outr;
  int32_t  note; //remember what note triggered this
};


struct KGRP  //keygroup
{
  int32_t  root;  //MIDI root note
  int32_t  high;  //highest note
  int32_t  pos;
  int32_t  end;
  int32_t  loop;
};

class mdaEPiano
{
  public:
    mdaEPiano(uint8_t nvoices);
    ~mdaEPiano();

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

  protected:
    void process(int16_t *outputs_r, int16_t *outputs_l);
    void update();
    void fillpatch(int32_t p, char *name, float p0, float p1, float p2, float p3, float p4,
                   float p5, float p6, float p7, float p8, float p9, float p10, float p11);
    void setParameter(int32_t index, float value);
    float getParameter(int32_t index);

    mdaEPianoProgram* programs;
    float Fs, iFs;

    ///global internal variables
    uint8_t max_polyphony;
    KGRP  kgrp[34];
    VOICE* voice;
    int32_t activevoices;
    short *waves;
    float width;
    int32_t  size, sustain;
    float lfo0, lfo1, dlfo, lmod, rmod;
    float treb, tfrq, tl, tr;
    float tune, fine, random, stretch, overdrive;
    float muff, muffvel, sizevel, velsens, modwhl;
    float volume;
    float vol;
    uint8_t curProgram;
};

#endif
