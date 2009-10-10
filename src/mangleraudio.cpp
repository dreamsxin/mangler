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
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>

ManglerAudio::ManglerAudio(uint16_t userid, uint32_t rate) {
    pulse_samplespec.format = PA_SAMPLE_S16LE;
    pulse_samplespec.rate = rate;
    pulse_samplespec.channels = 1;
    this->userid = userid;
    if (!(pulse_stream = pa_simple_new(NULL, "mangler", PA_STREAM_PLAYBACK, NULL, "playback", &pulse_samplespec, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        return;
    }
    pcm_queue = g_async_queue_new();
    Glib::Thread::create(sigc::mem_fun(*this, &ManglerAudio::play), FALSE);
}

ManglerAudio::~ManglerAudio() {
    pcmdata = new ManglerPCM(0, NULL);
    g_async_queue_push(pcm_queue, pcmdata);
    while (playing) {
        usleep(20);
    }
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
    for (;;) {
        playing = true;
        pcmdata = (ManglerPCM *)g_async_queue_pop(pcm_queue);
        if (pcmdata->length == 0) {
            break;
        }
        if ((ret = pa_simple_write(pulse_stream, pcmdata->sample, pcmdata->length, &error)) < 0) {
            fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
            g_async_queue_unref(pcm_queue);
            pa_simple_free(pulse_stream);
            playing = false;
            Glib::Thread::Exit();
        }
    }
    g_async_queue_push(pcm_queue, pcmdata);
    pa_simple_free(pulse_stream);
    playing = false;
    Glib::Thread::Exit();
}

