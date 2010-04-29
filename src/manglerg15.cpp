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
#include "manglerg15.h"

#ifdef HAVE_G15

ManglerG15::ManglerG15()
{
    if ((fd = new_g15_screen(G15_G15RBUF)) > 0) {
        memset(&canvas, 0, sizeof(g15canvas));
        memcpy(&canvas.buffer, g15_blank, G15_BUFFER_LEN);
        g15_send(fd, (char *)&canvas.buffer, G15_BUFFER_LEN);
    }
}

void
ManglerG15::addevent(Glib::ustring text) {
    vector<Glib::ustring>::iterator it;
    int ctr;

    if (events.size() > 5) {
        events.erase(events.begin());
    }
    events.push_back(text);
    memset(&canvas, 0, sizeof(g15canvas));
    memcpy(&canvas.buffer, g15_blank, G15_BUFFER_LEN);
    for (ctr = 1, it = events.begin(); it < events.end(); it++, ctr+=7) {
        g15r_renderString(&canvas, (unsigned char *)((Glib::ustring)*it).c_str(), 0, G15_TEXT_SMALL, 43, ctr);
        //fprintf(stderr, "%s\n", ((Glib::ustring)*it).c_str() );
    }
    g15_send(fd, (char *)&canvas.buffer, G15_BUFFER_LEN);
}


#endif

