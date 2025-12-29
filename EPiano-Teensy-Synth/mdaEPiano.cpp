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

#include "synth_mda_epiano.h"
#ifdef USE_XFADE_DATA
#include "mdaEPianoDataXfade.h"
#else
#include "mdaEPianoData.h"
#endif
#include "mdaEPiano.h"
#include <stdio.h>
#include <math.h>

mdaEPiano::mdaEPiano(uint8_t nvoices) // mdaEPiano::mdaEPiano(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, NPROGS, NPARAMS)
{
  Fs = AUDIO_SAMPLE_RATE;  iFs = 1.0f / Fs; //just in case...

  max_polyphony=nvoices;
  voice=new VOICE[max_polyphony];

  uint8_t i=0;
  programs = new mdaEPianoProgram[NPROGS];
    fillpatch(i++, (char *) "Default", 0.500f, 0.500f, 0.500f, 0.500f, 0.500f, 0.650f, 0.250f, 0.500f, 1.0f, 0.500f, 0.146f, 0.000f);
    fillpatch(i++, (char *) "Bright", 0.500f, 0.500f, 1.000f, 0.800f, 0.500f, 0.650f, 0.250f, 0.500f, 1.0f, 0.500f, 0.146f, 0.500f);
    fillpatch(i++, (char *) "Mellow", 0.500f, 0.500f, 0.000f, 0.000f, 0.500f, 0.650f, 0.250f, 0.500f, 1.0f, 0.500f, 0.246f, 0.000f);
    fillpatch(i++, (char *) "Autopan", 0.500f, 0.500f, 0.500f, 0.500f, 0.250f, 0.650f, 0.250f, 0.500f, 1.0f, 0.500f, 0.246f, 0.000f);
    fillpatch(i++, (char *) "Tremolo", 0.500f, 0.500f, 0.500f, 0.500f, 0.750f, 0.650f, 0.250f, 0.500f, 1.0f, 0.500f, 0.246f, 0.000f);
    fillpatch(i++, (char *) "(default)", 0.500f, 0.500f, 0.500f, 0.500f, 0.500f, 0.650f, 0.250f, 0.500f, 1.0f, 0.500f, 0.146f, 0.000f);
    fillpatch(i++, (char *) "(default)", 0.500f, 0.500f, 0.500f, 0.500f, 0.500f, 0.650f, 0.250f, 0.500f, 1.0f, 0.500f, 0.146f, 0.000f);
    fillpatch(i++, (char *) "(default)", 0.500f, 0.500f, 0.500f, 0.500f, 0.500f, 0.650f, 0.250f, 0.500f, 1.0f, 0.500f, 0.146f, 0.000f);

  setDecay(0.500f);
  setRelease(0.500f);
  setHardness(0.500f);
  setTreble(0.500f);
  setPanTremolo(0.500f);
  setPanLFO(0.650f);
  setVelocitySense(0.250f);
  setStereo(0.500f);
  setPolyphony(nvoices);
  setTune(0.500f);
  setDetune(0.146f);
  setOverdrive(0.000f);
  setVolume(0.64616f);

  waves = (short*)epianoDataXfade;

  //Waveform data and keymapping
  kgrp[ 0].root = 36;  kgrp[ 0].high = 39; //C1
  kgrp[ 3].root = 43;  kgrp[ 3].high = 45; //G1
  kgrp[ 6].root = 48;  kgrp[ 6].high = 51; //C2
  kgrp[ 9].root = 55;  kgrp[ 9].high = 57; //G2
  kgrp[12].root = 60;  kgrp[12].high = 63; //C3
  kgrp[15].root = 67;  kgrp[15].high = 69; //G3
  kgrp[18].root = 72;  kgrp[18].high = 75; //C4
  kgrp[21].root = 79;  kgrp[21].high = 81; //G4
  kgrp[24].root = 84;  kgrp[24].high = 87; //C5
  kgrp[27].root = 91;  kgrp[27].high = 93; //G5
  kgrp[30].root = 96;  kgrp[30].high = 999; //C6

  kgrp[0].pos = 0;        kgrp[0].end = 8476;     kgrp[0].loop = 4400;
  kgrp[1].pos = 8477;     kgrp[1].end = 16248;    kgrp[1].loop = 4903;
  kgrp[2].pos = 16249;    kgrp[2].end = 34565;    kgrp[2].loop = 6398;
  kgrp[3].pos = 34566;    kgrp[3].end = 41384;    kgrp[3].loop = 3938;
  kgrp[4].pos = 41385;    kgrp[4].end = 45760;    kgrp[4].loop = 1633; //was 1636;
  kgrp[5].pos = 45761;    kgrp[5].end = 65211;    kgrp[5].loop = 5245;
  kgrp[6].pos = 65212;    kgrp[6].end = 72897;    kgrp[6].loop = 2937;
  kgrp[7].pos = 72898;    kgrp[7].end = 78626;    kgrp[7].loop = 2203; //was 2204;
  kgrp[8].pos = 78627;    kgrp[8].end = 100387;   kgrp[8].loop = 6368;
  kgrp[9].pos = 100388;   kgrp[9].end = 116297;   kgrp[9].loop = 10452;
  kgrp[10].pos = 116298;  kgrp[10].end = 127661;  kgrp[10].loop = 5217; //was 5220;
  kgrp[11].pos = 127662;  kgrp[11].end = 144113;  kgrp[11].loop = 3099;
  kgrp[12].pos = 144114;  kgrp[12].end = 152863;  kgrp[12].loop = 4284;
  kgrp[13].pos = 152864;  kgrp[13].end = 173107;  kgrp[13].loop = 3916;
  kgrp[14].pos = 173108;  kgrp[14].end = 192734;  kgrp[14].loop = 2937;
  kgrp[15].pos = 192735;  kgrp[15].end = 204598;  kgrp[15].loop = 4732;
  kgrp[16].pos = 204599;  kgrp[16].end = 218995;  kgrp[16].loop = 4733;
  kgrp[17].pos = 218996;  kgrp[17].end = 233801;  kgrp[17].loop = 2285;
  kgrp[18].pos = 233802;  kgrp[18].end = 248011;  kgrp[18].loop = 4098;
  kgrp[19].pos = 248012;  kgrp[19].end = 265287;  kgrp[19].loop = 4099;
  kgrp[20].pos = 265288;  kgrp[20].end = 282255;  kgrp[20].loop = 3609;
  kgrp[21].pos = 282256;  kgrp[21].end = 293776;  kgrp[21].loop = 2446;
  kgrp[22].pos = 293777;  kgrp[22].end = 312566;  kgrp[22].loop = 6278;
  kgrp[23].pos = 312567;  kgrp[23].end = 330200;  kgrp[23].loop = 2283;
  kgrp[24].pos = 330201;  kgrp[24].end = 348889;  kgrp[24].loop = 2689;
  kgrp[25].pos = 348890;  kgrp[25].end = 365675;  kgrp[25].loop = 4370;
  kgrp[26].pos = 365676;  kgrp[26].end = 383661;  kgrp[26].loop = 5225;
  kgrp[27].pos = 383662;  kgrp[27].end = 393372;  kgrp[27].loop = 2811;
  kgrp[28].pos = 383662;  kgrp[28].end = 393372;  kgrp[28].loop = 2811; //ghost
  kgrp[29].pos = 393373;  kgrp[29].end = 406045;  kgrp[29].loop = 4522;
  kgrp[30].pos = 406046;  kgrp[30].end = 414486;  kgrp[30].loop = 2306;
  kgrp[31].pos = 406046;  kgrp[31].end = 414486;  kgrp[31].loop = 2306; //ghost
  kgrp[32].pos = 414487;  kgrp[32].end = 422408;  kgrp[32].loop = 2169;

  //initialise...
  resetVoices();
}

FLASHMEM void mdaEPiano::fillpatch(int32_t p, char *name, float p0, float p1, float p2, float p3, float p4, float p5, float p6, float p7, float p8, float p9, float p10,float p11)
{
  strcpy(programs[p].name, name);
  programs[p].param[0] = p0;  programs[p].param[1] = p1;
  programs[p].param[2] = p2;  programs[p].param[3] = p3;
  programs[p].param[4] = p4;  programs[p].param[5] = p5;
  programs[p].param[6] = p6;  programs[p].param[7] = p7;
  programs[p].param[8] = p8;  programs[p].param[9] = p9;
  programs[p].param[10]= p10; programs[p].param[11] = p11;
}

FLASHMEM void mdaEPiano::resetVoices(void)  // reset all voices
{
  for (int32_t v = 0; v < max_polyphony; v++)
  {
    voice[v].env = 0.0f;
    voice[v].dec = 0.99f; //all notes off
  }
  //volume = // 0.00002f * 127; // Fixing this level and using CC#7 as master_volume
  muff = 160.0f;
  sustain = activevoices = 0;
  tl = tr = lfo0 = dlfo = 0.0f;
  lfo1 = 1.0f;

  update();
}

FLASHMEM void mdaEPiano::resetControllers(void)  // reset controllers
{
  tl = tr = lfo0 = dlfo = 0.0f;
  lfo1 = 1.0f;

  update();
}

FLASHMEM void mdaEPiano::stopVoices(void)  // all keys off, but no reset for sustain
{
  for (int32_t v = 0; v < max_polyphony; v++)
  {
    voice[v].env = 0.0f;
  }
  muff = 160.0f;
  activevoices = 0;

  update();
}

FLASHMEM void mdaEPiano::update()  //parameter change
{
  float * param = programs[curProgram].param;
  size = (int32_t)(12.0f * param[2] - 6.0f);

  treb = 4.0f * param[3] * param[3] - 1.0f; //treble gain
  if (param[3] > 0.5f) tfrq = 14000.0f; else tfrq = 5000.0f; //treble freq
  tfrq = 1.0f - (float)exp(-iFs * tfrq);

  rmod = lmod = param[4] + param[4] - 1.0f; //lfo depth
  if (param[4] < 0.5f) rmod = -rmod;

  dlfo = 6.283f * iFs * (float)exp(6.22f * param[5] - 2.61f); //lfo rate

  velsens = 1.0f + param[6] + param[6];
  if (param[6] < 0.25f) velsens -= 0.75f - 3.0f * param[6];

  width = 0.03f * param[7];
  fine = param[9] - 0.5f;
  random = 0.077f * param[10] * param[10];
  stretch = 0.0f; //0.000434f * (param[11] - 0.5f); parameter re-used for overdrive!
  overdrive = 1.8f * param[11];
}


FLASHMEM void mdaEPiano::resumeVoices()
{
  Fs = AUDIO_SAMPLE_RATE;
  iFs = 1.0f / Fs;
  dlfo = 6.283f * iFs * (float)exp(6.22f * programs[ 0].param[5] - 2.61f); //lfo rate
}


mdaEPiano::~mdaEPiano ()  //destroy any buffers...
{
  if (programs) delete [] programs;
}

FLASHMEM void mdaEPiano::setDecay(float value)
{
  setParameter(MDA_EP_DECAY, value);
}

float mdaEPiano::getDecay(void)
{
  return(getParameter(MDA_EP_DECAY));
}

FLASHMEM void mdaEPiano::setRelease(float value)
{
  setParameter(MDA_EP_RELEASE, value);
}

float mdaEPiano::getRelease(void)
{
  return(getParameter(MDA_EP_RELEASE));
}

FLASHMEM void mdaEPiano::setHardness(float value)
{
  setParameter(MDA_EP_HARDNESS, value);
}

float mdaEPiano::getHardness(void)
{
  return(getParameter(MDA_EP_HARDNESS));
}

FLASHMEM void mdaEPiano::setTreble(float value)
{
  setParameter(MDA_EP_TREBLE, value);
}

float mdaEPiano::getTreble(void)
{
  return(getParameter(MDA_EP_TREBLE));
}

FLASHMEM void mdaEPiano::setPanTremolo(float value)
{
  setParameter(MDA_EP_PAN_TREM, value);
}

float mdaEPiano::getPanTremolo(void)
{
  return(getParameter(MDA_EP_PAN_TREM));
}

FLASHMEM void mdaEPiano::setPanLFO(float value)
{
  setParameter(MDA_EP_LFO_RATE, value);
}

float mdaEPiano::getPanLFO(void)
{
  return(getParameter(MDA_EP_LFO_RATE));
}

FLASHMEM void mdaEPiano::setVelocitySense(float value)
{
  setParameter(MDA_EP_VELOCITY_SENSE, value);
}

float mdaEPiano::getVelocitySense(void)
{
  return(getParameter(MDA_EP_VELOCITY_SENSE));
}

FLASHMEM void mdaEPiano::setStereo(float value)
{
  setParameter(MDA_EP_STEREO, value);
}

float mdaEPiano::getStereo(void)
{
  return(getParameter(MDA_EP_STEREO));
}

FLASHMEM void mdaEPiano::setPolyphony(uint8_t value)
{
  if (value <= 0)
    value = 1;

  resetVoices();

  if(voice)
    delete(voice);

  voice=new VOICE[value];
  resetVoices();

  max_polyphony = value;
}

uint8_t mdaEPiano::getPolyphony(void)
{
  return(max_polyphony);
}

FLASHMEM void mdaEPiano::setTune(float value)
{
  setParameter(MDA_EP_TUNE, value);
}

float mdaEPiano::getTune(void)
{
  return(getParameter(MDA_EP_TUNE));
}

FLASHMEM void mdaEPiano::setDetune(float value)
{
  setParameter(MDA_EP_DETUNE, value);
}

float mdaEPiano::getDetune(void)
{
  return(getParameter(MDA_EP_DETUNE));
}

FLASHMEM void mdaEPiano::setOverdrive(float value)
{
  setParameter(MDA_EP_OVERDRIVE, value);
}

float mdaEPiano::getOverdrive(void)
{
  return(getParameter(MDA_EP_OVERDRIVE));
}

FLASHMEM void mdaEPiano::setVolume(float value)
{
  //volume = value * 0.32258; // 0.00002 * 127^2
  volume = value * 0.16f;
}

float mdaEPiano::getVolume(void)
{
  return(volume/0.16f);
}

FLASHMEM void mdaEPiano::setParameter(int32_t index, float value)
{
  programs[0].param[index] = value;
  update();
}

float mdaEPiano::getParameter(int32_t index)
{
  return(programs[0].param[index]);
}

int32_t mdaEPiano::getActiveVoices(void)
{
  return (activevoices);
}

FLASHMEM void mdaEPiano::process(int16_t* outputs_r, int16_t* outputs_l)
{
  int16_t v;
  float x, l, r, od = overdrive;
  int32_t i;
  int16_t frame;

  for (frame = 0; frame < AUDIO_BLOCK_SAMPLES; frame++)
  {
    VOICE *V = voice;
    l = r = 0.0f;
    for (v = 0; v < activevoices; v++)
    {
      V->frac += V->delta;  //integer-based linear interpolation
      V->pos += V->frac >> 16;
      V->frac &= 0xFFFF;
      if (V->pos > V->end)
        V->pos -= V->loop;
      i = waves[V->pos] + ((V->frac * (waves[V->pos + 1] - waves[V->pos])) >> 16);
      x = V->env * static_cast<float>(i) / 32768.0f;

      V->env = V->env * V->dec;  //envelope
      if (x > 0.0f)
      {
        x -= od * x * x;   //overdrive
        if (x < -V->env) x = -V->env;
      }
      l += V->outl * x;
      r += V->outr * x;

      V++;
    }

    tl += tfrq * (l - tl);  //treble boost
    tr += tfrq * (r - tr);
    r  += treb * (r - tr);
    l  += treb * (l - tl);

    lfo0 += dlfo * lfo1;  //LFO for tremolo and autopan
    lfo1 -= dlfo * lfo0;
    l += l * lmod * lfo1;
    r += r * rmod * lfo1;  //worth making all these local variables?

    r *= 0.5;
    l *= 0.5;

    if (r > 1.0)
      r = 1.0;
    else if (r < -1.0)
      r = -1.0;
    if (l > 1.0)
      l = 1.0;
    else if (l < -1.0)
      l = -1.0;
    outputs_l[frame] = static_cast<int16_t>(l * 0x7fff);
    outputs_r[frame] = static_cast<int16_t>(r * 0x7fff);
  }

  if (fabs(tl) < 1.0e-10) tl = 0.0f; //anti-denormal
  if (fabs(tr) < 1.0e-10) tr = 0.0f;

  for (v = 0; v < activevoices; v++)
    if (voice[v].env < SILENCE)
      voice[v] = voice[--activevoices];
}

FLASHMEM void mdaEPiano::noteOn(int32_t note, int32_t velocity)
{
  float * param = programs[ 0].param;
  float l = 99.0f;
  int32_t  v, vl = 0, k, s;

  if (velocity > 0)
  {
    if (activevoices < max_polyphony) //add a note
    {
      vl = activevoices;
      activevoices++;
      voice[vl].f0 = voice[vl].f1 = 0.0f;
    }
    else //steal a note
    {
      for (v = 0; v <  max_polyphony; v++) //find quietest voice
      {
        if (voice[v].env < l) {
          l = voice[v].env;
          vl = v;
        }
      }
    }

    k = (note - 60) * (note - 60);
    l = fine + random * ((float)(k % 13) - 6.5f);  //random & fine tune
    if (note > 60) l += stretch * (float)k; //stretch

    s = size;
    //if(velocity > 40) s += (int32_t)(sizevel * (float)(velocity - 40));  - no velocity to hardness in ePiano

    k = 0;
    while (note > (kgrp[k].high + s)) k += 3; //find keygroup
    l += (float)(note - kgrp[k].root); //pitch
    l = 32000.0f * iFs * (float)exp(0.05776226505 * l);
    voice[vl].delta = (int32_t)(65536.0f * l);
    voice[vl].frac = 0;

    if (velocity > 48) k++; //mid velocity sample
    if (velocity > 80) k++; //high velocity sample
    voice[vl].pos = kgrp[k].pos;
    voice[vl].end = kgrp[k].end - 1;
    voice[vl].loop = kgrp[k].loop;

    voice[vl].env = (3.0f + 2.0f * velsens) * (float)pow(0.0078f * velocity, velsens); //velocity

    if (note > 60) voice[vl].env *= (float)exp(0.01f * (float)(60 - note)); //new! high notes quieter

    l = 50.0f + param[4] * param[4] * muff + muffvel * (float)(velocity - 64); //muffle
    if (l < (55.0f + 0.4f * (float)note)) l = 55.0f + 0.4f * (float)note;
    if (l > 210.0f) l = 210.0f;
    voice[vl].ff = l * l * iFs;

    voice[vl].note = note; //note->pan
    if (note <  12) note = 12;
    if (note > 108) note = 108;
    l = volume;
    voice[vl].outr = l + l * width * (float)(note - 60);
    voice[vl].outl = l + l - voice[vl].outr;

    if (note < 44) note = 44; //limit max decay length
    voice[vl].dec = (float)exp(-iFs * exp(-1.0 + 0.03 * (double)note - 2.0f * param[0]));
  }
  else //note off
  {
    for (v = 0; v <  max_polyphony; v++) if (voice[v].note == note) //any voices playing that note?
      {
        if (sustain == 0)
        {
          voice[v].dec = (float)exp(-iFs * exp(6.0 + 0.01 * (double)note - 5.0 * param[1]));
        }
        else voice[v].note = SUSTAIN;
      }
  }
}

FLASHMEM void mdaEPiano::noteOff(int32_t note)
{
	noteOn(note,0);
}

FLASHMEM bool mdaEPiano::processMidiController(uint8_t data1, uint8_t data2)
{
  float* param = programs[ 0].param;

  switch (data1)
  {
    case 0x01:  //mod wheel
      modwhl = 0.0078f * (float)(data2);
      if (modwhl > 0.05f) //over-ride pan/trem depth
      {
        rmod = lmod = modwhl; //lfo depth
        if (param[4] < 0.5f) rmod = -rmod;
      }
      break;

    case 0x07:  //volume
      //set_master_volume(map(data2, 0, 127, ENC_MASTER_VOLUME_MIN, ENC_MASTER_VOLUME_MAX));
      break;

    case 0x40:  //sustain pedal
    case 0x42:  //sustenuto pedal
      sustain = data2 & 0x40;
      if (sustain == 0)
      {
        noteOn(SUSTAIN, 0); //end all sustained notes
      }
      break;

    case 0x78: // All Sound Off: mutes all sounding notes. It does so regardless of release time or sustain. (See MIDI CC 123)
      resetVoices();
      break;
    case 0x79: // Reset All Controllers: it will reset all controllers to their default.
      resetControllers();
      break;
    case 0x7b: // All Notes Off: mutes all sounding notes. Release time will still be maintained, and notes held by sustain will not turn off until sustain pedal is depressed.
      stopVoices();
      break;
    case 0x7e: // Mono Mode: sets device mode to Monophonic.
      setPolyphony(1);
      break;
    case 0x7f: // Poly Mode: sets device mode to Polyphonic.
      setPolyphony(max_polyphony);
      break;
  }
  return (true);
}

FLASHMEM void mdaEPiano::setProgram(uint8_t program)
{
    curProgram = program;
    update();
}

