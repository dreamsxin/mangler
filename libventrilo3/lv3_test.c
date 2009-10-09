/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2009-09-30 12:58:47 -0700 (Wed, 30 Sep 2009) $
 * $Revision: 495 $
 * $LastChangedBy: ekilfoil $
 * $URL: http://svn.manglerproject.net/svn/mangler/trunk/libventrilo3/lv3_test.c $
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

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>
#include <speex/speex.h>
#include "ventrilo3.h"

extern _v3_luser v3_luser;

#define false   0
#define true    1

void
ctrl_c (int signum)
{
    printf("disconnecting... ");
    v3_logout();
    printf("done\n");
    exit(0);
}



int main(int argc, char *argv[]) {
    _v3_net_message *msg;
    char *username;
    int ctr = 1;
    int c, ret;
    v3_event *ev;

    // pulse
    pa_sample_spec PAss = {
        .format = PA_SAMPLE_S16LE,
        .rate = 0,
        .channels = 1
    };
    pa_simple *PAs = NULL;
    int PAerror;

    v3_debuglevel(
            //V3_DEBUG_NONE|
            //V3_DEBUG_STATUS|
            //V3_DEBUG_ERROR|
            V3_DEBUG_STACK|
            //V3_DEBUG_INTERNAL|
            V3_DEBUG_PACKET|
            V3_DEBUG_PACKET_PARSE|
            //V3_DEBUG_PACKET_ENCRYPTED|
            //V3_DEBUG_MEMORY|
            //V3_DEBUG_SOCKET|
            //V3_DEBUG_NOTICE|
            V3_DEBUG_INFO|
            V3_DEBUG_MUTEX|
            V3_DEBUG_EVENT
            );


    signal (SIGINT, ctrl_c);
    username = strdup(getenv("USER"));
    //if (! v3_login("evolve.typefrag.com:54174", username, "password", "phonetic")) {
    if (! v3_login("localhost:3784", username, "test", "phonetic")) {
    //if (! v3_login("tungsten.typefrag.com:29549", username, "mangler", "phonetic")) {
        printf("ERROR: %s\n", _v3_error(NULL));
        return 0;
    }
    _v3_print_channel_list();
    _v3_print_user_list();
    printf("# of channels: %d\n", v3_channel_count());
    printf("# of users: %d\n", v3_user_count());
    {
        v3_user *u;
        if ((u = v3_get_user(23))) {
            printf("%s\n", u->name);
            v3_free_user(u);
        } else {
            printf("couldn't find user\n");
        }
    }
    free(username);
    c = ctr = 0;
    v3_clear_events();
    do {
        ctr++;
        if (ctr > 20) {
            // should be after login
        }
        if ((msg = _v3_recv(V3_BLOCK)) == NULL) {
            printf("recv() failed: %s\n", _v3_error(NULL));
            return false;
        }

        switch (_v3_process_message(msg)) {
            case V3_MALFORMED:
                _v3_debug(V3_DEBUG_INFO, "received malformed packet");
                break;
            case V3_NOTIMPL:
                _v3_debug(V3_DEBUG_INFO, "packet type not implemented");
                break;
            case V3_OK:
                _v3_debug(V3_DEBUG_INFO, "packet processed");
                break;
        }
        if ((ev = v3_get_event(V3_NONBLOCK))) {
            _v3_debug(V3_DEBUG_INFO, "got new event type %d", ev->type);
            if (ev->type == V3_EVENT_PLAY_AUDIO) {
                if (ev->pcm.rate != PAss.rate) {
                    _v3_debug(V3_DEBUG_INFO, "reopening output stream at rate %d....", ev->pcm.rate);
                    PAss.rate = ev->pcm.rate;
                    if (PAs != NULL) {
                        pa_simple_free(PAs);
                    }
                    if (!(PAs = pa_simple_new(NULL, argv[0], PA_STREAM_PLAYBACK, NULL, "playback", &PAss, NULL, NULL, &PAerror))) {
                        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(PAerror));
                        exit(0);
                    }
                }
                _v3_debug(V3_DEBUG_INFO, "outputting sound");
                _v3_debug(V3_DEBUG_INFO, "writing %d bytes to PA", ev->pcm.length);
                if ((ret = pa_simple_write(PAs, ev->pcm.sample, (size_t) ev->pcm.length, &PAerror)) < 0) {
                    fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(PAerror));
                    exit(0);
                }
                _v3_debug(V3_DEBUG_INFO, "write complete with ret %d",ret);
            }
            free(ev);
        }
    } while (_v3_is_connected());
    return 0;
}

