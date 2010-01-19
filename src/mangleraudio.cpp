/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate$
 * $Revision$
 * $LastChangedBy$
 * $URL$
 *
 * Copyright 2009-2010 Eric Kilfoil 
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
#include "mangler-sounds.h"
#include <sys/time.h>

ManglerAudio::ManglerAudio(Glib::ustring type) {/*{{{*/
    //fprintf(stderr, "creating audio object\n");
}/*}}}*/
ManglerAudio::~ManglerAudio() {/*{{{*/
}/*}}}*/

void
ManglerAudio::open(uint32_t rate, bool type, uint32_t pcm_framesize) {/*{{{*/
    outputStreamOpen = false;
    inputStreamOpen = false;
    //fprintf(stderr, "creating object: %d \n", type);
    this->rate = rate;
#ifdef HAVE_PULSE
    pulse_stream = NULL;
    pulse_samplespec.format = PA_SAMPLE_S16LE;
    pulse_samplespec.rate = rate;
    pulse_samplespec.channels = 1;
    buffer_attr.maxlength = -1;
    buffer_attr.tlength = -1;
    buffer_attr.prebuf = -1;
    buffer_attr.minreq = -1;
    buffer_attr.fragsize = pcm_framesize;
#endif
#ifdef HAVE_ALSA
    alsa_stream = NULL;
#endif
    if (type == AUDIO_OUTPUT) {
        //fprintf(stderr, "opening audio output\n");
        if (!openOutput(rate)) {
            return;
        }
        outputStreamOpen = true;
        inputStreamOpen = false;
        //fprintf(stderr, "starting output thread\n");
        pcm_queue = g_async_queue_new();
        Glib::Thread::create(sigc::mem_fun(*this, &ManglerAudio::output), FALSE);
    } else {
        this->pcm_framesize = pcm_framesize;
        if (this->pcm_framesize == 0) {
            fprintf(stderr, "framesize not specified on input stream open\n");
            return;
        }
        //fprintf(stderr, "starting input with rate %d and framesize %d\n", rate, pcm_framesize);
        if (!openInput(rate)) {
            return;
        }
        stop_input = false;
        outputStreamOpen = false;
        inputStreamOpen = true;
        //fprintf(stderr, "starting input thread\n");
        Glib::Thread::create(sigc::mem_fun(*this, &ManglerAudio::input), FALSE);
    }
}/*}}}*/

bool
ManglerAudio::openOutput(uint32_t rate) {/*{{{*/
    closeOutput(true); // close any existing output streams
#ifdef HAVE_PULSE
    if (mangler->settings->config.audioSubsystem == "pulse") {
        pulse_samplespec.rate = rate;
        //fprintf(stderr, "opening on pulse device %s\n", (char *)mangler->settings->config.outputDeviceName.c_str());
        if (!(pulse_stream = pa_simple_new(
                        NULL,
                        "Mangler",
                        PA_STREAM_PLAYBACK,
                        (mangler->settings->config.outputDeviceName == "Default" || 
                            mangler->settings->config.outputDeviceName == "" 
                            ? NULL 
                            : (char *)mangler->settings->config.outputDeviceName.c_str()),
                        "User Talking In Ventrilo Channel",
                        &pulse_samplespec,
                        NULL,
                        NULL,
                        &pulse_error))) {
            fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(pulse_error));
            pulse_stream = NULL;
            return false;
        }
    }
#endif
#ifdef HAVE_ALSA
    if (mangler->settings->config.audioSubsystem == "alsa") {
        if ((alsa_error = snd_pcm_open(&alsa_stream,
                        (mangler->settings->config.outputDeviceName == "Default" || 
                            mangler->settings->config.outputDeviceName == "" 
                            ? "default" 
                            : (char *)mangler->settings->config.outputDeviceName.c_str()),
                        SND_PCM_STREAM_PLAYBACK,
                        0)) < 0) {
            fprintf(stderr, "snd_pcm_open() failed: %s\n", snd_strerror(alsa_error));
            alsa_stream = NULL;
            return false;
        }
        if ((alsa_error = snd_pcm_set_params(alsa_stream, // pcm handle
                        SND_PCM_FORMAT_S16_LE,            // format
                        SND_PCM_ACCESS_RW_INTERLEAVED,    // access
                        1,                                // channels
                        rate,                             // rate
                        true,                             // soft_resample
                        150000)) < 0) {                   // latency in usec (0.15 sec)
            fprintf(stderr, "snd_pcm_set_params() failed: %s\n", snd_strerror(alsa_error));
            closeOutput(false);
            return false;
        }
        if ((alsa_error = snd_pcm_prepare(alsa_stream)) < 0) {
            fprintf(stderr, "snd_pcm_prepare() failed: %s\n", snd_strerror(alsa_error));
            closeOutput(false);
            return false;
        }
    }
#endif
    return true;
}/*}}}*/

void
ManglerAudio::closeOutput(bool drain) {/*{{{*/
#ifdef HAVE_PULSE
    if (pulse_stream) {
        if (drain && pa_simple_drain(pulse_stream, &pulse_error) < 0) {
            fprintf(stderr, __FILE__": pa_simple_drain() failed: %s\n", pa_strerror(pulse_error));
        }
        pa_simple_free(pulse_stream);
        pulse_stream = NULL;
    }
#endif
#ifdef HAVE_ALSA
    if (alsa_stream) {
        if (drain) {
            snd_pcm_drain(alsa_stream);
        }
        snd_pcm_close(alsa_stream);
        alsa_stream = NULL;
    }
#endif
}/*}}}*/

bool
ManglerAudio::openInput(uint32_t rate) {/*{{{*/
    closeInput(true); // close any existing input streams
#ifdef HAVE_PULSE
    if (mangler->settings->config.audioSubsystem == "pulse") {
        pulse_samplespec.rate = rate;
        //fprintf(stderr, "on pulse device %s\n", (char *)mangler->settings->config.outputDeviceName.c_str());
        if (!(pulse_stream = pa_simple_new(
                        NULL,
                        "Mangler",
                        PA_STREAM_RECORD,
                        (mangler->settings->config.inputDeviceName == "Default" || 
                            mangler->settings->config.inputDeviceName == "" 
                            ? NULL 
                            : (char *)mangler->settings->config.inputDeviceName.c_str()),
                        "Talking In Ventrilo Channel",
                        &pulse_samplespec,
                        NULL,
                        &buffer_attr,
                        &pulse_error))) {
            fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(pulse_error));
            pulse_stream = NULL;
            return false;
        }
    }
#endif
#ifdef HAVE_ALSA
    if (mangler->settings->config.audioSubsystem == "alsa") {
        if ((alsa_error = snd_pcm_open(&alsa_stream,
                        (mangler->settings->config.inputDeviceName == "Default" || 
                            mangler->settings->config.inputDeviceName == "" 
                            ? "default" 
                            : (char *)mangler->settings->config.inputDeviceName.c_str()),
                        SND_PCM_STREAM_CAPTURE,
                        0)) < 0) {
            fprintf(stderr, "snd_pcm_open() failed: %s\n", snd_strerror(alsa_error));
            alsa_stream = NULL;
            return false;
        }
        if ((alsa_error = snd_pcm_set_params(alsa_stream, // pcm handle
                        SND_PCM_FORMAT_S16_LE,            // format
                        SND_PCM_ACCESS_RW_INTERLEAVED,    // access
                        1,                                // channels
                        rate,                             // rate
                        true,                             // soft_resample
                        150000)) < 0) {                   // latency in usec (0.15 sec)
            fprintf(stderr, "snd_pcm_set_params() failed: %s\n", snd_strerror(alsa_error));
            closeInput(false);
            return false;
        }
        if ((alsa_error = snd_pcm_prepare(alsa_stream)) < 0) {
            fprintf(stderr, "snd_pcm_prepare() failed: %s\n", snd_strerror(alsa_error));
            closeInput(false);
            return false;
        }
        if ((alsa_error = snd_pcm_start(alsa_stream)) < 0) {
            fprintf(stderr, "snd_pcm_start() failed: %s\n", snd_strerror(alsa_error));
            closeInput(false);
            return false;
        }
    }
#endif
    return true;
}/*}}}*/

void
ManglerAudio::closeInput(bool drain) {/*{{{*/
#ifdef HAVE_PULSE
    if (pulse_stream) {
        pa_simple_free(pulse_stream);
        pulse_stream = NULL;
    }
#endif
#ifdef HAVE_ALSA
    if (alsa_stream) {
        if (drain) {
            snd_pcm_drain(alsa_stream);
        }
        snd_pcm_close(alsa_stream);
        alsa_stream = NULL;
    }
#endif
    mangler->inputvumeter->set_fraction(0);
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
ManglerAudio::input(void) {/*{{{*/
    int ret;
    uint8_t *buf = NULL;
    uint32_t ret_rate;
    float seconds = 0;
    struct timeval start, now, diff;
    uint16_t pcmmax;
    int ctr;
    bool drop;

    //fprintf(stderr, "getting input\n");
    for (;;) {
        //fprintf(stderr, "main input iteration\n");
        if (stop_input == true) {
            fprintf(stderr, "stopping input\n");
            closeInput(true);
            //throw Glib::Thread::Exit();
            return;
        }
        if (!inputStreamOpen) {
            fprintf(stderr, "input stream is not open\n");
            //throw Glib::Thread::Exit();
            return;
        }
        gettimeofday(&start, NULL);
        seconds = 0.0;
        ctr = 0;
        // As best as I can tell, we're supposed to send ~0.11 seconds of audio in each packet
        for (;;) {
            /*
            if (seconds >= 0.115) {
                //fprintf(stderr, "0.115 seconds of real time has elapsed\n", ctr);
                break;
            }
            */
            if (pcm_framesize * ctr > rate * 0.115 * 2) {
                //fprintf(stderr, "we have 0.115 seconds of audio in %d iterations\n", ctr);
                break;
            }
            //fprintf(stderr, "reallocating %d bytes of memory\n", pcm_framesize*(ctr+1));
            if ((pcm_framesize*(ctr+1)) > 16384) {
                fprintf(stderr, "audio frames are greater than buffer size.  dropping audio frames after %f seconds\n", seconds);
                drop = true;
                break;
            }
            drop = false;
            buf = (uint8_t *)realloc(buf, pcm_framesize*(ctr+1));
            //fprintf(stderr, "reading %d bytes of memory to %lu\n", pcm_framesize, (uint64_t) buf+(pcm_framesize*ctr));
#ifdef HAVE_PULSE
            if (mangler->settings->config.audioSubsystem == "pulse") {
                if (!pulse_stream && !openInput(rate)) { // reinitialize input stream for pulse
                    stop_input = true;
                    return;
                }
                //fprintf(stderr, "reading %d bytes from pulse source\n", pcm_framesize);
                if ((ret = pa_simple_read(pulse_stream, buf+(pcm_framesize*ctr), pcm_framesize, &pulse_error)) < 0) {
                    //fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(pulse_error));
                    closeInput(false);
                    stop_input = true;
                    //throw Glib::Thread::Exit();
                    return;
                }
            }
#endif
#ifdef HAVE_ALSA
            if (mangler->settings->config.audioSubsystem == "alsa") {
                if (!alsa_stream && !openInput(rate)) { // reinitialize input stream for alsa
                    stop_input = true;
                    return;
                }
                if ((alsa_frames = snd_pcm_readi(alsa_stream, buf+(pcm_framesize*ctr), pcm_framesize / sizeof(int16_t))) < 0) {
                    if (alsa_frames == -EPIPE) {
                        snd_pcm_prepare(alsa_stream);
                    } else if ((alsa_error = snd_pcm_recover(alsa_stream, alsa_frames, 0)) < 0) {
                        fprintf(stderr, "snd_pcm_readi() failed: %s\n", snd_strerror(alsa_error));
                        closeInput(false);
                        stop_input = true;
                        return;
                    }
                }
            }
#endif
            gettimeofday(&now, NULL);
            timeval_subtract(&diff, &now, &start);
            seconds = (float)diff.tv_sec + ((float)diff.tv_usec / (float)1000000);
            //fprintf(stderr, "iteration after %f seconds with %d bytes\n", seconds, pcm_framesize*ctr);
            pcmmax = 0;
            for (int16_t *pcmptr = (int16_t *)(buf+(pcm_framesize*ctr)); pcmptr < (int16_t *)(buf+pcm_framesize*(ctr+1)); pcmptr++) {
                pcmmax = abs(*pcmptr) > pcmmax ? abs(*pcmptr) : pcmmax;
            }
            if (pcmmax > 24576) {
                pcmmax = 24576;
            }
            mangler->inputvumeter->set_fraction(pcmmax/24576.0);
            ctr++;
        }
        if (! drop) {
            //fprintf(stderr, "sending audio %d bytes of audio\n", ctr * pcm_framesize);
            // TODO: hard coding user to channel for now, need to implement U2U
            if ((ret_rate = v3_send_audio(V3_AUDIO_SENDTYPE_U2CCUR, rate, buf, ctr * pcm_framesize)) != rate) {
                if (!ret_rate || !openInput((rate = ret_rate))) { // reinitialize input with the new sample rate
                    stop_input = true; // else we're logged out or we couldn't reinitialize
                }
            }
        }
        free(buf);
        buf = NULL;
        if (stop_input == true) {
            break;
        }
    }
    //fprintf(stderr, "done with input\n");
    v3_stop_audio();
    closeInput(true);
    outputStreamOpen = false;
    //throw Glib::Thread::Exit();
    return;
}/*}}}*/

void
ManglerAudio::output(void) {/*{{{*/
    int ret;
    ManglerPCM *queuedpcm;

    //fprintf(stderr, "playing audio\n");
    // If we don't have a pcm queue set up for us, something is very wrong
    if (!pcm_queue) {
        //throw Glib::Thread::Exit();
        return;
    }

    //short circuit if we are muted
    if(mangler->muteSound) {
        return;    
    }

    g_async_queue_ref(pcm_queue);
    usleep(500000); // buffer for 0.5 seconds
    for (;;) {
        stop_output = false;
        if (! v3_is_loggedin()) {
            // we were disconnected while playing the stream.  unref the queue
            // and flush the audio buffers
#ifdef HAVE_PULSE
            if (pulse_stream) {
                pa_simple_flush(pulse_stream, &pulse_error);
            }
#endif
            g_async_queue_unref(pcm_queue);
            break;
        }
        queuedpcm = (ManglerPCM *)g_async_queue_pop(pcm_queue);
        // finish() queues a 0 length packet to notify us that we're done
        if (queuedpcm->length == 0) {
            g_async_queue_unref(pcm_queue);
            delete queuedpcm;
            break;
        }
#ifdef HAVE_PULSE
        if (mangler->settings->config.audioSubsystem == "pulse") {
            if (!pulse_stream && !openOutput(rate)) { // reinitialize output stream for pulse
                g_async_queue_unref(pcm_queue);
                stop_output = true;
                return;
            }
            if ((ret = pa_simple_write(pulse_stream, queuedpcm->sample, queuedpcm->length, &pulse_error)) < 0) {
                fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(pulse_error));
                g_async_queue_unref(pcm_queue);
                closeOutput(false);
                stop_output = true;
                //throw Glib::Thread::Exit();
                return;
            }
        }
#endif
#ifdef HAVE_ALSA
        if (mangler->settings->config.audioSubsystem == "alsa") {
            if (!alsa_stream && !openOutput(rate)) { // reinitialize output stream for alsa
                g_async_queue_unref(pcm_queue);
                stop_output = true;
                return;
            }
            uint32_t buflen;
            uint32_t pcmlen = queuedpcm->length;
            uint8_t *pcmptr = queuedpcm->sample;
            while ((buflen = pcmlen >= ALSA_BUF ? ALSA_BUF : pcmlen)) {
                if ((alsa_frames = snd_pcm_writei(alsa_stream, pcmptr, buflen / sizeof(int16_t))) < 0) {
                    if (alsa_frames == -EPIPE) {
                        snd_pcm_prepare(alsa_stream);
                    } else if ((alsa_error = snd_pcm_recover(alsa_stream, alsa_frames, 0)) < 0) {
                        fprintf(stderr, "snd_pcm_writei() failed: %s\n", snd_strerror(alsa_error));
                        g_async_queue_unref(pcm_queue);
                        closeOutput(false);
                        stop_output = true;
                        return;
                    }
                }
                pcmlen -= buflen;
                pcmptr += buflen;
            }
        }
#endif
        delete queuedpcm;
    }
    closeOutput(true);
    stop_output = true;
    outputStreamOpen = true;
    //throw Glib::Thread::Exit();
    return;
}/*}}}*/

void
ManglerAudio::finish(void) {/*{{{*/
    if (outputStreamOpen) {
        stop_output = true;
        pcmdata = new ManglerPCM(0, NULL);
        g_async_queue_push(pcm_queue, pcmdata);
    }
    if (inputStreamOpen) {
        //fprintf(stderr, "stopping input stream\n");
        stop_input = true;
    }
}/*}}}*/

void
ManglerAudio::getDeviceList(Glib::ustring audioSubsystem) {/*{{{*/
    int ctr;

    inputDevices.clear();
    outputDevices.clear();

#ifdef HAVE_PULSE
    if (audioSubsystem == "pulse") {
        // This is where we'll store the input device list
        pa_devicelist_t pa_input_devicelist[16];

        // This is where we'll store the output device list
        pa_devicelist_t pa_output_devicelist[16];

        if (pa_get_devicelist(pa_input_devicelist, pa_output_devicelist) < 0) {
            fprintf(stderr, "pulseaudio: failed to get device list; make sure 'pulseaudio' is running\n");
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
    }
#endif
#ifdef HAVE_ALSA
    if (audioSubsystem == "alsa") {
        snd_pcm_stream_t stream[2] = { SND_PCM_STREAM_PLAYBACK, SND_PCM_STREAM_CAPTURE };
        
        for (ctr = 0; ctr < 2; ctr++) { // the rest is just copypasta, with bad code from alsa
            snd_ctl_t *handle;
            int card, err, dev, 
                idx_p = 0, idx_c = 0;
            snd_ctl_card_info_t *info;
            snd_pcm_info_t *pcminfo;
            snd_ctl_card_info_alloca(&info);
            snd_pcm_info_alloca(&pcminfo);
            
            card = -1;
            if (snd_card_next(&card) < 0 || card < 0) {
                fputs("alsa: no sound cards found...\n", stderr);
                return;
            }
            while (card >= 0) {
                char hw[256] = "";
                snprintf(hw, 255, "hw:%i", card);
                if ((err = snd_ctl_open(&handle, hw, 0)) < 0) {
                    fprintf(stderr, "alsa: control open (%i): %s\n", card, snd_strerror(err));
                    if (snd_card_next(&card) < 0) {
                        fprintf(stderr, "alsa: snd_ctl_open: snd_card_next\n");
                        break;
                    }
                    continue;
                }
                if ((err = snd_ctl_card_info(handle, info)) < 0) {
                    fprintf(stderr, "alsa: control hardware info (%i): %s\n", card, snd_strerror(err));
                    snd_ctl_close(handle);
                    if (snd_card_next(&card) < 0) {
                        fprintf(stderr, "alsa: snd_ctl_card_info: snd_card_next\n");
                        break;
                    }
                    continue;
                }
                dev = -1;
                for (;;) {
                    if (snd_ctl_pcm_next_device(handle, &dev) < 0)
                        fprintf(stderr, "alsa: snd_ctl_pcm_next_device\n");
                    if (dev < 0)
                        break;
                    snd_pcm_info_set_device(pcminfo, dev);
                    snd_pcm_info_set_subdevice(pcminfo, 0);
                    snd_pcm_info_set_stream(pcminfo, stream[ctr]);
                    if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) {
                        if (err != -ENOENT)
                            fprintf(stderr, "alsa: control digital audio info (%i): %s\n", card, snd_strerror(err));
                        continue;
                    }
                    char name[256] = "", desc[512] = "";
                    snprintf(name, 255, "hw:%i,%i", card, dev);
                    snprintf(desc, 511, "%s: %s (%s)",
                        snd_ctl_card_info_get_name(info),
                        snd_pcm_info_get_name(pcminfo),
                        name
                    );
                    switch (stream[ctr]) {
                      case SND_PCM_STREAM_PLAYBACK:
                        outputDevices.push_back(
                            new ManglerAudioDevice(
                                idx_p++,
                                name,
                                desc)
                        );
                        break;
                      case SND_PCM_STREAM_CAPTURE:
                        inputDevices.push_back(
                            new ManglerAudioDevice(
                                idx_c++,
                                name,
                                desc)
                        );
                        break;
                    }
                }
                snd_ctl_close(handle);
                if (snd_card_next(&card) < 0) {
                    fprintf(stderr, "alsa: snd_card_next\n");
                    break;
                }
            }
        }
    }
#endif
}/*}}}*/

void
ManglerAudio::playNotification(Glib::ustring name) {/*{{{*/
    if (mangler->muteSound) {
        return;
    }
    if ((name == "talkstart" || name == "talkend") && ! mangler->settings->config.notificationTransmitStartStop) {
        return;
    }
    if ((name == "channelenter" || name == "channelleave") && ! mangler->settings->config.notificationChannelEnterLeave) {
        return;
    }
    if ((name == "login" || name == "logout") && ! mangler->settings->config.notificationLoginLogout) {
        return;
    }
    if (sounds.empty()) {
        sounds["talkstart"]    = new ManglerPCM(sizeof(sound_talkstart),    sound_talkstart);
        sounds["talkend"]      = new ManglerPCM(sizeof(sound_talkend),      sound_talkend);
        sounds["channelenter"] = new ManglerPCM(sizeof(sound_channelenter), sound_channelenter);
        sounds["channelleave"] = new ManglerPCM(sizeof(sound_channelleave), sound_channelleave);
        sounds["login"]        = new ManglerPCM(sizeof(sound_login),        sound_login);
        sounds["logout"]       = new ManglerPCM(sizeof(sound_logout),       sound_logout);
    }
    Glib::Thread::create(sigc::bind(sigc::mem_fun(this, &ManglerAudio::playNotification_thread), name), FALSE);
}/*}}}*/

void
ManglerAudio::playNotification_thread(Glib::ustring name) {/*{{{*/
#ifdef HAVE_PULSE
    if (mangler->settings->config.audioSubsystem == "pulse") {
        int pulse_ret;
        pa_simple       *pulse_s;
        pulse_samplespec.format = PA_SAMPLE_S16LE;
        pulse_samplespec.rate = 44100;
        pulse_samplespec.channels = 1;
        pa_buffer_attr buffer_attr;
        buffer_attr.maxlength = -1;
        buffer_attr.tlength = -1;
        buffer_attr.prebuf = -1;
        buffer_attr.minreq = -1;
        buffer_attr.fragsize = pcm_framesize;
        if (!(pulse_s = pa_simple_new(
                        NULL,
                        "Mangler",
                        PA_STREAM_PLAYBACK,
                        (mangler->settings->config.notificationDeviceName == "Default" || 
                            mangler->settings->config.notificationDeviceName == "" 
                            ? NULL 
                            : (char *)mangler->settings->config.notificationDeviceName.c_str()),
                        "Notification Sound",
                        &pulse_samplespec,
                        NULL,
                        NULL,
                        &pulse_error))) {
            fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(pulse_error));
            //throw Glib::Thread::Exit();
            return;
        }
        if ((pulse_ret = pa_simple_write(pulse_s, sounds[name]->sample, sounds[name]->length, &pulse_error)) < 0) {
            fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(pulse_error));
            pa_simple_free(pulse_s);
            //throw Glib::Thread::Exit();
            return;
        }
        if (pa_simple_drain(pulse_s, &pulse_error) < 0) {
            fprintf(stderr, __FILE__": pa_simple_drain() failed: %s\n", pa_strerror(pulse_error));
        }

        pa_simple_free(pulse_s);
    }
#endif
#ifdef HAVE_ALSA
    if (mangler->settings->config.audioSubsystem == "alsa") {
        snd_pcm_sframes_t alsa_ret;
        snd_pcm_t         *alsa_s;
        if ((alsa_error = snd_pcm_open(&alsa_s,
                        (mangler->settings->config.notificationDeviceName == "Default" || 
                            mangler->settings->config.notificationDeviceName == "" 
                            ? "default" 
                            : (char *)mangler->settings->config.notificationDeviceName.c_str()),
                        SND_PCM_STREAM_PLAYBACK,
                        0)) < 0) {
            fprintf(stderr, "snd_pcm_open() failed: %s\n", snd_strerror(alsa_error));
            return;
        }
        if ((alsa_error = snd_pcm_set_params(alsa_s,   // pcm handle
                        SND_PCM_FORMAT_S16_LE,         // format
                        SND_PCM_ACCESS_RW_INTERLEAVED, // access
                        1,                             // channels
                        44100,                         // rate
                        true,                          // soft_resample
                        150000)) < 0) {                // latency in usec (0.15 sec)
            fprintf(stderr, "snd_pcm_set_params() failed: %s\n", snd_strerror(alsa_error));
            snd_pcm_close(alsa_s);
            return;
        }
        if ((alsa_error = snd_pcm_prepare(alsa_s)) < 0) {
            fprintf(stderr, "snd_pcm_prepare() failed: %s\n", snd_strerror(alsa_error));
            snd_pcm_close(alsa_s);
            return;
        }
        uint32_t buflen;
        uint32_t pcmlen = sounds[name]->length;
        uint8_t *pcmptr = sounds[name]->sample;
        while ((buflen = pcmlen >= ALSA_BUF ? ALSA_BUF : pcmlen)) {
            if ((alsa_ret = snd_pcm_writei(alsa_s, pcmptr, buflen / sizeof(int16_t))) < 0) {
                if (alsa_ret == -EPIPE) {
                    snd_pcm_prepare(alsa_s);
                } else if ((alsa_error = snd_pcm_recover(alsa_s, alsa_ret, 0)) < 0) {
                    fprintf(stderr, "snd_pcm_writei() failed: %s\n", snd_strerror(alsa_error));
                    snd_pcm_close(alsa_s);
                    return;
                }
            }
            pcmlen -= buflen;
            pcmptr += buflen;
        }
        snd_pcm_drain(alsa_s);
        snd_pcm_close(alsa_s);
    }
#endif
    //throw Glib::Thread::Exit();
    return;
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

int
timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y) {/*{{{*/
    /* Perform the carry for the later subtraction by updating y. */
    if (x->tv_usec < y->tv_usec) {
        int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
        y->tv_usec -= 1000000 * nsec;
        y->tv_sec += nsec;
    }
    if (x->tv_usec - y->tv_usec > 1000000) {
        int nsec = (x->tv_usec - y->tv_usec) / 1000000;
        y->tv_usec += 1000000 * nsec;
        y->tv_sec -= nsec;
    }

    /* Compute the time remaining to wait.
       tv_usec is certainly positive. */
    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_usec = x->tv_usec - y->tv_usec;

    /* Return 1 if result is negative. */
    return x->tv_sec < y->tv_sec;
}/*}}}*/
