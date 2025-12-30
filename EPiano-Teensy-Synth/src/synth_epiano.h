#pragma once

#define USE_XFADE_DATA 1 // TODO

#include <Audio.h>
#include <AudioStream.h>
#include "mdaEPiano.h"

class AudioSynthEPiano : public AudioStream, public mdaEPiano {
  public:
    const uint16_t audio_block_time_us = 1000000 / (AUDIO_SAMPLE_RATE / AUDIO_BLOCK_SAMPLES);
    uint32_t xrun = 0;
    uint16_t render_time_max = 0;

    AudioSynthEPiano(uint8_t nvoices) : AudioStream(0, NULL), mdaEPiano(nvoices) { };

    void update(void)
    {
      if (in_update == true)
      {
        xrun++;
        return;
      }
      else
        in_update = true;

      elapsedMicros render_time;
      audio_block_t *lblock;
      audio_block_t *rblock;

      lblock = allocate();
      rblock = allocate();

      if (!lblock || !rblock)
      {
        in_update = false;
        return;
      }

      mdaEPiano::process(rblock->data, lblock->data);

      if (render_time > audio_block_time_us) // everything greater audio_block_time_us (2.9ms for buffer size of 128) is a buffer underrun!
        xrun++;

      if (render_time > render_time_max)
        render_time_max = render_time;

      transmit(lblock, 0);
      transmit(rblock, 1);
      release(lblock);
      release(rblock);

      in_update = false;
    };

  private:
    volatile bool in_update = false;
};
