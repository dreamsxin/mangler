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

ManglerAudio::ManglerAudio(uint16_t userid, uint32_t rate) {/*{{{*/
    outputStreamOpen = false;
#ifdef HAVE_PULSE
    pulse_samplespec.format = PA_SAMPLE_S16LE;
    pulse_samplespec.rate = rate;
    pulse_samplespec.channels = 1;
    this->userid = userid;
    if (!(pulse_stream = pa_simple_new(
                    NULL,
                    "mangler",
                    PA_STREAM_PLAYBACK,
                    (mangler->settings->config.outputDeviceName == "Default" ? NULL : (char *)mangler->settings->config.outputDeviceName.c_str()),
                    "playback",
                    &pulse_samplespec,
                    NULL,
                    NULL,
                    &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        return;
    }
    outputStreamOpen = true;
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
}/*}}}*/
ManglerAudio::ManglerAudio() {/*{{{*/
    this->getDeviceList();
}/*}}}*/
ManglerAudio::~ManglerAudio() {/*{{{*/
}/*}}}*/

void
ManglerAudio::getDeviceList(void) {/*{{{*/
#ifdef HAVE_PULSE
    int ctr;

    // This is where we'll store the input device list
    pa_devicelist_t pa_input_devicelist[16];

    // This is where we'll store the output device list
    pa_devicelist_t pa_output_devicelist[16];

    if (pa_get_devicelist(pa_input_devicelist, pa_output_devicelist) < 0) {
        fprintf(stderr, "failed to get device list\n");
        return;
    }

    for (ctr = 0; ctr < 16; ctr++) {
        if (! pa_output_devicelist[ctr].initialized) {
            break;
        }
        outputDevices.push_back(
                new ManglerAudioDevice(
                    pa_output_devicelist[ctr].index,
                    pa_output_devicelist[ctr].name,
                    pa_output_devicelist[ctr].description)
                );
    }

    for (ctr = 0; ctr < 16; ctr++) {
        if (! pa_input_devicelist[ctr].initialized) {
            break;
        }
        inputDevices.push_back(
                new ManglerAudioDevice(
                    pa_input_devicelist[ctr].index,
                    pa_input_devicelist[ctr].name,
                    pa_input_devicelist[ctr].description)
                );
    }
    return;
#endif
}/*}}}*/

void
ManglerAudio::finish(void) {/*{{{*/
    if (!outputStreamOpen) {
        return;
    }
    pcmdata = new ManglerPCM(0, NULL);
    g_async_queue_push(pcm_queue, pcmdata);
}/*}}}*/

void
ManglerAudio::queue(uint32_t length, uint8_t *sample) {/*{{{*/
    if (!outputStreamOpen) {
        return;
    }
    pcmdata = new ManglerPCM(length, sample);
    g_async_queue_push(pcm_queue, pcmdata);
}/*}}}*/

void
ManglerAudio::play(void) {/*{{{*/
    int ret, error;

    // Make sure we have a stream open, or just die out
    if (!outputStreamOpen) {
        Glib::Thread::Exit();
    }
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
    outputStreamOpen = true;
    Glib::Thread::Exit();
}/*}}}*/

#ifdef HAVE_PULSE
// Pulse Audio Device List Retrieval (in all of it's C glory)/*{{{*/
int pa_get_devicelist(pa_devicelist_t *input, pa_devicelist_t *output) {
    // Define our pulse audio loop and connection variables
    pa_mainloop *pa_ml;
    pa_mainloop_api *pa_mlapi;
    pa_operation *pa_op;
    pa_context *pa_ctx;

    // We'll need these state variables to keep track of our requests
    int state = 0;
    int pa_ready = 0;

    // Initialize our device lists
    memset(input, 0, sizeof(pa_devicelist_t) * 16);
    memset(output, 0, sizeof(pa_devicelist_t) * 16);

    // Create a mainloop API and connection to the default server
    pa_ml = pa_mainloop_new();
    pa_mlapi = pa_mainloop_get_api(pa_ml);
    pa_ctx = pa_context_new(pa_mlapi, "test");

    // This function connects to the pulse server
    pa_context_connect(pa_ctx, NULL, (pa_context_flags_t)0, NULL);

    // This function defines a callback so the server will tell us it's state.
    // Our callback will wait for the state to be ready.  The callback will
    // modify the variable to 1 so we know when we have a connection and it's
    // ready.
    // If there's an error, the callback will set pa_ready to 2
    pa_context_set_state_callback(pa_ctx, pa_state_cb, &pa_ready);

    // Now we'll enter into an infinite loop until we get the data we receive
    // or if there's an error
    for (;;) {
        // We can't do anything until PA is ready, so just iterate the mainloop
        // and continue
        if (pa_ready == 0) {
            pa_mainloop_iterate(pa_ml, 1, NULL);
            continue;
        }
        // We couldn't get a connection to the server, so exit out
        if (pa_ready == 2) {
            pa_context_disconnect(pa_ctx);
            pa_context_unref(pa_ctx);
            pa_mainloop_free(pa_ml);
            return -1;
        }
        // At this point, we're connected to the server and ready to make
        // requests
        switch (state) {
            // State 0: we haven't done anything yet
            case 0:
                // This sends an operation to the server.  pa_sinklist_info is
                // our callback function and a pointer to our devicelist will
                // be passed to the callback The operation ID is stored in the
                // pa_op variable
                pa_op = pa_context_get_sink_info_list(pa_ctx,
                        pa_sinklist_cb,
                        output
                        );

                // Update state for next iteration through the loop
                state++;
                break;
            case 1:
                // Now we wait for our operation to complete.  When it's
                // complete our pa_output_devicelist is filled out, and we move
                // along to the next state
                if (pa_operation_get_state(pa_op) == PA_OPERATION_DONE) {
                    pa_operation_unref(pa_op);

                    // Now we perform another operation to get the source
                    // (input device) list just like before.  This time we pass
                    // a pointer to our input structure
                    pa_op = pa_context_get_source_info_list(pa_ctx,
                            pa_sourcelist_cb,
                            input
                            );
                    // Update the state so we know what to do next
                    state++;
                }
                break;
            case 2:
                if (pa_operation_get_state(pa_op) == PA_OPERATION_DONE) {
                    // Now we're done, clean up and disconnect and return
                    pa_operation_unref(pa_op);
                    pa_context_disconnect(pa_ctx);
                    pa_context_unref(pa_ctx);
                    pa_mainloop_free(pa_ml);
                    return 0;
                }
                break;
            default:
                // We should never see this state
                fprintf(stderr, "in state %d\n", state);
                return -1;
        }
        // Iterate the main loop and go again.  The second argument is whether
        // or not the iteration should block until something is ready to be
        // done.  Set it to zero for non-blocking.
        pa_mainloop_iterate(pa_ml, 1, NULL);
    }
}

// This callback gets called when our context changes state.  We really only
// care about when it's ready or if it has failed
void pa_state_cb(pa_context *c, void *userdata) {
    pa_context_state_t state;
    int *pa_ready = (int *)userdata;

    state = pa_context_get_state(c);
    switch  (state) {
        // There are just here for reference
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
        default:
            break;
        case PA_CONTEXT_FAILED:
        case PA_CONTEXT_TERMINATED:
            *pa_ready = 2;
            break;
        case PA_CONTEXT_READY:
            *pa_ready = 1;
            break;
    }
}

// pa_mainloop will call this function when it's ready to tell us about a sink.
// Since we're not threading, there's no need for mutexes on the devicelist
// structure
void pa_sinklist_cb(pa_context *c, const pa_sink_info *l, int eol, void *userdata) {
    pa_devicelist_t *pa_devicelist = (pa_devicelist_t *)userdata;
    int ctr = 0;

    // If eol is set to a positive number, you're at the end of the list
    if (eol > 0) {
        return;
    }

    // We know we've allocated 16 slots to hold devices.  Loop through our
    // structure and find the first one that's "uninitialized."  Copy the
    // contents into it and we're done.  If we receive more than 16 devices,
    // they're going to get dropped.  You could make this dynamically allocate
    // space for the device list, but this is a simple example.
    for (ctr = 0; ctr < 16; ctr++) {
        if (! pa_devicelist[ctr].initialized) {
            strncpy(pa_devicelist[ctr].name, l->name, 511);
            strncpy(pa_devicelist[ctr].description, l->description, 255);
            pa_devicelist[ctr].index = l->index;
            pa_devicelist[ctr].initialized = 1;
            break;
        }
    }
}

// See above.  This callback is pretty much identical to the previous
void pa_sourcelist_cb(pa_context *c, const pa_source_info *l, int eol, void *userdata) {
    pa_devicelist_t *pa_devicelist = (pa_devicelist_t *)userdata;
    int ctr = 0;

    if (eol > 0) {
        return;
    }

    for (ctr = 0; ctr < 16; ctr++) {
        if (! pa_devicelist[ctr].initialized) {
            strncpy(pa_devicelist[ctr].name, l->name, 511);
            strncpy(pa_devicelist[ctr].description, l->description, 255);
            pa_devicelist[ctr].index = l->index;
            pa_devicelist[ctr].initialized = 1;
            break;
        }
    }
}/*}}}*/
#endif
