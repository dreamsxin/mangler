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

ManglerAudio::ManglerAudio() {
}

void
ManglerAudio::openStream(uint16_t userid, uint32_t rate) {
    fprintf(stderr, "starting audio stream for user %d at rate %d\n", userid, rate);

    /*
    pulse_samplespec[userid] = new pa_sample_spec;
    pulse_samplespec[userid]->format = PA_SAMPLE_S16LE;
    pulse_samplespec[userid]->rate = rate;
    pulse_samplespec[userid]->channels = 1;
    pulse_stop[userid] = false;
    if (!(pulse_stream[userid] = pa_simple_new(NULL, "mangler", PA_STREAM_PLAYBACK, NULL, "playback", pulse_samplespec[userid], NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        return;
    }
    pcm_queue[userid] = g_async_queue_new();
    Glib::Thread::create(sigc::bind(sigc::mem_fun(this, &ManglerAudio::play), userid), FALSE);
    */

}

void
ManglerAudio::closeStream(uint16_t userid) {
    fprintf(stderr, "closing audio stream for user %d\n", userid);
    pulse_stop[userid] = true;
}

void
ManglerAudio::queue(uint16_t userid, uint32_t length, uint8_t *sample) {
    fprintf(stderr, "queueing pcm data for userid %d (length %d)\n", userid, length);
    //g_async_queue_push(pcm_queue[userid], sample);
}

void
ManglerAudio::play(uint16_t userid) {
    void *samples;
    uint32_t length;
    int ret, error;

    /*
    while (! pulse_stop[userid]) {
        fprintf(stderr, "playing audio stream for user %d\n", userid);
        g_async_queue_lock(pcm_queue[userid]);
        length = g_async_queue_length(pcm_queue[userid]);
        samples = g_async_queue_pop(pcm_queue[userid]);
        if ((ret = pa_simple_write(pulse_stream[userid], samples, length, &error)) < 0) {
            fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
            g_async_queue_unlock(pcm_queue[userid]);
            exit(0);
        }
        fprintf(stderr, "done writing audio....\n");
        g_async_queue_unlock(pcm_queue[userid]);
    }
    fprintf(stderr, "exiting stream thread for %d\n", userid);
    g_async_queue_unref(pcm_queue[userid]);
    return;
    */
}

