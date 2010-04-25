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

#ifndef _MANGLERAUDIO_H
#define _MANGLERAUDIO_H

#include "config.h"

#ifdef HAVE_PULSE
# include <pulse/pulseaudio.h>
# include <pulse/simple.h>
# include <pulse/error.h>
# include <pulse/gccmacro.h>
#endif
#ifdef HAVE_ALSA
# include <alsa/asoundlib.h>
# define ALSA_BUF 640
#endif
#ifdef HAVE_OSS
# include <fcntl.h>
# include <sys/ioctl.h>
# include <sys/soundcard.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define AUDIO_CONTROL   -2
#define AUDIO_INPUT     -1
#define AUDIO_OUTPUT    0
#define AUDIO_NOTIFY    1

class ManglerPCM
{
    public:
        ManglerPCM(uint32_t length, uint8_t *sample) {
            this->length = length;
            this->sample = (uint8_t *)malloc(length); // I'm a C programmer... sue me
            memcpy(this->sample, sample, length);
        }
        ~ManglerPCM() {
            free(this->sample);
        }
        uint32_t            length;
        uint8_t             *sample;
};
class ManglerAudioDevice
{
    public:
        ManglerAudioDevice(uint32_t id, Glib::ustring name, Glib::ustring description) {
            this->id            = id;
            this->name          = name;
            this->description   = description;
        }
        ~ManglerAudioDevice() {
        }
        uint32_t            id;
        Glib::ustring       name;
        Glib::ustring       description;
};
class ManglerAudio
{
    public:
        ManglerAudio(int type, uint32_t rate = 0, uint8_t channels = 1, uint32_t pcm_framesize = 0, uint8_t buffer = 4, bool check_loggedin = true);
        ~ManglerAudio();

        int             type;
        uint32_t        rate;
        uint32_t        pcm_framesize;
        uint8_t         channels;
        uint8_t         buffer;
        bool            check_loggedin;
        GAsyncQueue*    pcm_queue;
#ifdef HAVE_PULSE
        pa_sample_spec  pulse_samplespec;
        pa_buffer_attr  buffer_attr;
        pa_simple       *pulse_stream;
        int             pulse_error;
#endif
#ifdef HAVE_ALSA
        snd_pcm_t       *alsa_stream;
        snd_pcm_sframes_t alsa_frames;
        int             alsa_error;
#endif
#ifdef HAVE_OSS
        int             oss_fd;
#endif
        bool            outputStreamOpen;
        bool            inputStreamOpen;
        bool            stop_input;

        std::map<Glib::ustring, ManglerPCM *> sounds;

        std::vector<ManglerAudioDevice*> outputDevices;
        std::vector<ManglerAudioDevice*> inputDevices;

        bool            open(void);
        void            close(bool drain = false);
        void            queue(uint32_t length, uint8_t *sample);
        void            finish(void);
        void            output(void);
        void            input(void);
        void            getDeviceList(Glib::ustring audioSubsystem);
        void            playNotification(Glib::ustring name);
};

// this is easier in C
#ifdef HAVE_PULSE
typedef struct pa_devicelist {
    uint8_t initialized;
    char name[512];
    uint32_t index;
    char description[256];
} pa_devicelist_t;

void pa_state_cb(pa_context *c, void *userdata);
void pa_sinklist_cb(pa_context *c, const pa_sink_info *l, int eol, void *userdata);
void pa_sourcelist_cb(pa_context *c, const pa_source_info *l, int eol, void *userdata);
int pa_get_devicelist(pa_devicelist_t *input, pa_devicelist_t *output);
#endif

int timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y);

#endif

