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

#ifndef _MANGLERAUDIO_H
#define _MANGLERAUDIO_H

#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>
#include <stdlib.h>
#include <string.h>


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
class ManglerAudio
{
    public:
        ManglerAudio(uint16_t userid, uint32_t rate);
        ~ManglerAudio();
        void            queue(uint32_t length, uint8_t *sample);
        void            play(void);

        GAsyncQueue*    pcm_queue;
        pa_sample_spec  pulse_samplespec;
        pa_simple       *pulse_stream;
        ManglerPCM      *pcmdata;

        uint16_t        userid;
        int             error;
        bool            playing;
};


#endif
