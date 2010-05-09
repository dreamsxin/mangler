/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate$
 * $Revision$
 * $LastChangedBy$
 * $URL$
 *
 * Copyright 2009-2010 Eric Kilfoil
 * Copyright 2010 Roman Tetelman
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

#ifndef _MANGLER_BACKEND_H
#define _MANGLER_BACKEND_H

// circular dependencies in C(++) are fun!
class ManglerAudioDevice;

class ManglerBackend {
public:
    virtual bool            open(int type, Glib::ustring device, int rate, int channels) = 0;
    virtual void            close(bool drain = false) = 0;
    virtual bool            write(uint8_t* sample, uint32_t length, int channels) = 0;
    virtual bool            read(uint8_t* buf) = 0;
    virtual Glib::ustring   getAudioSubsystem(void) = 0;
    static ManglerBackend*  getBackend(Glib::ustring audioSubsystem, uint32_t rate, uint8_t channels, uint32_t pcm_framesize);
    virtual ~ManglerBackend();

    static void             getDeviceList(Glib::ustring audioSubsystem, std::vector<ManglerAudioDevice*>& input, std::vector<ManglerAudioDevice*>& output);
};

#endif

