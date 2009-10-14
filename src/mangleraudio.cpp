/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate$
 * $Revision$
 * $LastChangedBy$
 * $URL$
 *
 * Copyright 2009 Eric Kilfoil 
 *
 * This file is part of Mangler.
 *
 * Mangler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mangler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Mangler.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mangler.h"
#include "mangleraudio.h"

ManglerAudio::ManglerAudio(uint16_t userid, uint32_t rate) {
#ifdef HAVE_PULSE
    pulse_samplespec.format = PA_SAMPLE_S16LE;
    pulse_samplespec.rate = rate;
    pulse_samplespec.channels = 1;
    this->userid = userid;
    if (!(pulse_stream = pa_simple_new(NULL, "mangler", PA_STREAM_PLAYBACK, NULL, "playback", &pulse_samplespec, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        return;
    }
#elif HAVE_ALSA
    snd_pcm_t *pcm_handle;          
    snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
    snd_pcm_hw_params_t *hwparams;
    char pcm_name[] = "plughw:0,0";
    snd_pcm_hw_params_alloca(&hwparams);
    if (snd_pcm_open(&pcm_handle, pcm_name, stream, 0) < 0) {
        fprintf(stderr, "Error opening PCM device %s\n", pcm_name);
        return(-1);
    }
#endif
    pcm_queue = g_async_queue_new();
    Glib::Thread::create(sigc::mem_fun(*this, &ManglerAudio::play), FALSE);
}
ManglerAudio::~ManglerAudio() {
}

void
ManglerAudio::finish(void) {
    pcmdata = new ManglerPCM(0, NULL);
    g_async_queue_push(pcm_queue, pcmdata);
}

void
ManglerAudio::queue(uint32_t length, uint8_t *sample) {
    pcmdata = new ManglerPCM(length, sample);
    g_async_queue_push(pcm_queue, pcmdata);
}

void
ManglerAudio::play(void) {
    int ret, error;

    g_async_queue_ref(pcm_queue);
    usleep(500000); // buffer for a half second
    for (;;) {
        playing = true;
        pcmdata = (ManglerPCM *)g_async_queue_pop(pcm_queue);
        if (pcmdata->length == 0) {
            break;
        }
#ifdef HAVE_PULSE
        if ((ret = pa_simple_write(pulse_stream, pcmdata->sample, pcmdata->length, &error)) < 0) {
            fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
            g_async_queue_unref(pcm_queue);
            pa_simple_free(pulse_stream);
            playing = false;
            Glib::Thread::Exit();
        }
#endif
    }
#ifdef HAVE_PULSE
    if (pa_simple_drain(pulse_stream, &error) < 0) {
        fprintf(stderr, __FILE__": pa_simple_drain() failed: %s\n", pa_strerror(error));
    }
    pa_simple_free(pulse_stream);
#endif
    playing = false;
    Glib::Thread::Exit();
}

