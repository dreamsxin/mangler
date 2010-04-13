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

#ifdef ANDROID
#include <android/log.h>
#else
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>

#if HAVE_SPEEX_DSP
# include <speex/speex_resampler.h>
#endif
#if HAVE_SPEEX
# include <speex/speex.h>
#endif
#if HAVE_CELT
# include <celt/celt.h>
#endif
#ifdef HAVE_GSM_H
# include <gsm.h>
#elif HAVE_GSM_GSM_H
# include <gsm/gsm.h>
#endif


// TODO: check how portable this is... (known good: ubuntu, arch)
#define __USE_UNIX98
#include <pthread.h>
#undef __USE_UNIX98

extern int h_errno;

#include "ventrilo3.h"
#include "libventrilo3.h"
#include "libventrilo3_message.h"

#define true  1
#define false 0

uint16_t stack_level = 0;

void
_v3_debug(uint32_t level, const char *format, ...) {/*{{{*/
    va_list args;
    char timestamp[200];
    time_t t;
    struct tm *tmp;
    char buf[1024] = "";
    int ctr;
    struct timeval tv;

    if (! (level & v3_debuglevel(-1))) {
        return;
    }
    va_start(args, format);
    char str[1024] = "";
    vsnprintf(str, 1023, format, args);
    va_end(args);

    for (ctr = 0; ctr < stack_level * 4; ctr++) {
        strncat(buf, " ", 1023);
    }
    strncat(buf, str, 1023);
    gettimeofday(&tv, NULL);
    t = tv.tv_sec;
    tmp = localtime(&t);
    if (tmp == NULL) {
#ifndef ANDROID
        fprintf(stderr, "libventrilo3: %s\n", buf); // print without timestamp
#else
        __android_log_write(ANDROID_LOG_VERBOSE, "libventrilo3", buf);
#endif
        return;
    }

    if (strftime(timestamp, sizeof(timestamp), "%T", tmp) == 0) {
#ifndef ANDROID
        fprintf(stderr, "libventrilo3: %s\n", buf); // print without timestamp
#else
        __android_log_write(ANDROID_LOG_VERBOSE, "libventrilo3", buf);
#endif
        return;
    }
    
#ifndef ANDROID
    fprintf(stderr, "libventrilo3: %s.%06d: %s\n", timestamp, (uint32_t)tv.tv_usec, buf); // print with timestamp
#else
    __android_log_print(ANDROID_LOG_VERBOSE, "libventrilo3", " %s.%06d: %s", timestamp, (uint32_t)tv.tv_usec, buf);
#endif
    return;
}/*}}}*/

void
_v3_func_enter(char *func) {/*{{{*/
    char buf[256] = "";

    if (! (V3_DEBUG_STACK & v3_debuglevel(-1))) {
        // maintain the stack level even if we're not debugging this
        stack_level++;
        return;
    }
    snprintf(buf, 255, "---> %s()", func);
    _v3_debug(V3_DEBUG_STACK, buf);
    stack_level++;
    return;
}/*}}}*/

void
_v3_func_leave(char *func) {/*{{{*/
    char buf[256] = "";

    // make sure we're at least one level deep to prevent underruns
    if (stack_level < 1) {
        stack_level = 1;
    }
    if (! (V3_DEBUG_STACK & v3_debuglevel(-1))) {
        // maintain the stack level even if we're not debugging this
        stack_level--;
        return;
    }
    stack_level--;
    snprintf(buf, 255, "<--- %s()", func);
    _v3_debug(V3_DEBUG_STACK, buf);
    return;
}/*}}}*/

char *
_v3_error(const char *format, ...) {/*{{{*/
    va_list args;

    if (format == NULL) {
        return _v3_error_text;
    }
    va_start(args, format);
    vsnprintf(_v3_error_text, sizeof(_v3_error_text), format, args);
    va_end(args);
    _v3_debug(V3_DEBUG_ERROR, _v3_error_text);
    return _v3_error_text;
}/*}}}*/

char *
_v3_status(uint8_t percent, const char *format, ...) {/*{{{*/
    va_list args;
    if (format == NULL) {
        return _v3_status_text;
    }
    va_start(args, format);
    vsnprintf(_v3_status_text, sizeof(_v3_status_text), format, args);
    va_end(args);
    v3_event *ev = _v3_create_event(V3_EVENT_STATUS);
    ev->status.percent = percent;
    strncpy(ev->status.message, _v3_status_text, 256);
    v3_queue_event(ev);
    _v3_debug(V3_DEBUG_STATUS, _v3_status_text);
    return _v3_status_text;
}/*}}}*/

void
_v3_net_message_dump_raw(char *data, int len) {/*{{{*/
    int ctr, ctr2;
    char buf[256], buf2[4];

    // specifically bail out of this one since there's a lot of wasted
    // processing if we're not debugging it
    if (! (V3_DEBUG_PACKET_ENCRYPTED & v3_debuglevel(-1))) {
        return;
    }
    for (ctr = 0; ctr < len; ctr+=16) {
        if (ctr+16 > len) {
            buf[0] = 0;
            for (ctr2 = ctr; ctr2 < len; ctr2++) {
                snprintf(buf2, 4, "%02X ", (uint8_t)data[ctr2]);
                strncat(buf, buf2, 255);
            }
            _v3_debug(V3_DEBUG_PACKET_ENCRYPTED, "PACKET:     %s", buf);
        } else {
            _v3_debug(V3_DEBUG_PACKET_ENCRYPTED, "PACKET:     %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
                    (uint8_t)data[ctr],
                    (uint8_t)data[ctr+1],
                    (uint8_t)data[ctr+2],
                    (uint8_t)data[ctr+3],
                    (uint8_t)data[ctr+4],
                    (uint8_t)data[ctr+5],
                    (uint8_t)data[ctr+6],
                    (uint8_t)data[ctr+7],
                    (uint8_t)data[ctr+8],
                    (uint8_t)data[ctr+9],
                    (uint8_t)data[ctr+10],
                    (uint8_t)data[ctr+11],
                    (uint8_t)data[ctr+12],
                    (uint8_t)data[ctr+13],
                    (uint8_t)data[ctr+14],
                    (uint8_t)data[ctr+15]
                    );
        }
    }
    return;
}/*}}}*/

void
_v3_hexdump(char *data, int len) {/*{{{*/
    int ctr, ctr2;
    char buf[256], buf2[4];

    for (ctr = 0; ctr < len; ctr+=16) {
        if (ctr+16 > len) {
            buf[0] = 0;
            for (ctr2 = ctr; ctr2 < len; ctr2++) {
                snprintf(buf2, 4, "%02X ", (uint8_t)data[ctr2]);
                strncat(buf, buf2, 255);
            }
            _v3_debug(V3_DEBUG_INFO, "%s", buf);
        } else {
            _v3_debug(V3_DEBUG_PACKET, "PACKET:     %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X      %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
                    (uint8_t)data[ctr],
                    (uint8_t)data[ctr+1],
                    (uint8_t)data[ctr+2],
                    (uint8_t)data[ctr+3],
                    (uint8_t)data[ctr+4],
                    (uint8_t)data[ctr+5],
                    (uint8_t)data[ctr+6],
                    (uint8_t)data[ctr+7],
                    (uint8_t)data[ctr+8],
                    (uint8_t)data[ctr+9],
                    (uint8_t)data[ctr+10],
                    (uint8_t)data[ctr+11],
                    (uint8_t)data[ctr+12],
                    (uint8_t)data[ctr+13],
                    (uint8_t)data[ctr+14],
                    (uint8_t)data[ctr+15],
                    (uint8_t)data[ctr]    > 32 && (uint8_t)data[ctr]    < 127 ? (uint8_t)data[ctr]     : '.',
                    (uint8_t)data[ctr+1]  > 32 && (uint8_t)data[ctr+1]  < 127 ? (uint8_t)data[ctr+1]   : '.',
                    (uint8_t)data[ctr+2]  > 32 && (uint8_t)data[ctr+2]  < 127 ? (uint8_t)data[ctr+2]   : '.',
                    (uint8_t)data[ctr+3]  > 32 && (uint8_t)data[ctr+3]  < 127 ? (uint8_t)data[ctr+3]   : '.',
                    (uint8_t)data[ctr+4]  > 32 && (uint8_t)data[ctr+4]  < 127 ? (uint8_t)data[ctr+4]   : '.',
                    (uint8_t)data[ctr+5]  > 32 && (uint8_t)data[ctr+5]  < 127 ? (uint8_t)data[ctr+5]   : '.',
                    (uint8_t)data[ctr+6]  > 32 && (uint8_t)data[ctr+6]  < 127 ? (uint8_t)data[ctr+6]   : '.',
                    (uint8_t)data[ctr+7]  > 32 && (uint8_t)data[ctr+7]  < 127 ? (uint8_t)data[ctr+7]   : '.',
                    (uint8_t)data[ctr+8]  > 32 && (uint8_t)data[ctr+8]  < 127 ? (uint8_t)data[ctr+8]   : '.',
                    (uint8_t)data[ctr+9]  > 32 && (uint8_t)data[ctr+9]  < 127 ? (uint8_t)data[ctr+9]   : '.',
                    (uint8_t)data[ctr+10] > 32 && (uint8_t)data[ctr+10] < 127 ? (uint8_t)data[ctr+10]  : '.',
                    (uint8_t)data[ctr+11] > 32 && (uint8_t)data[ctr+11] < 127 ? (uint8_t)data[ctr+11]  : '.',
                    (uint8_t)data[ctr+12] > 32 && (uint8_t)data[ctr+12] < 127 ? (uint8_t)data[ctr+12]  : '.',
                    (uint8_t)data[ctr+13] > 32 && (uint8_t)data[ctr+13] < 127 ? (uint8_t)data[ctr+13]  : '.',
                    (uint8_t)data[ctr+14] > 32 && (uint8_t)data[ctr+14] < 127 ? (uint8_t)data[ctr+14]  : '.',
                    (uint8_t)data[ctr+15] > 32 && (uint8_t)data[ctr+15] < 127 ? (uint8_t)data[ctr+15]  : '.'
                        );
        }
    }
    return;
}/*}}}*/

void
_v3_net_message_dump(_v3_net_message *msg) {/*{{{*/
    int ctr, ctr2;
    char buf[256] = "", buf2[8] = "";

    // specifically bail out of this one since there's a lot of wasted
    // processing if we're not debugging it
    if (! (V3_DEBUG_PACKET & v3_debuglevel(-1))) {
        return;
    }
    _v3_debug(V3_DEBUG_PACKET, "PACKET: message type: 0x%02X (%d)", (uint8_t)msg->type, (uint16_t)msg->type);
    _v3_debug(V3_DEBUG_PACKET, "PACKET: data length : %d", msg->len);
    for (ctr = 0; ctr < msg->len; ctr+=16) {
        if (ctr+16 > msg->len) {
            buf[0] = 0;
            for (ctr2 = ctr; ctr2 < msg->len; ctr2++) {
                snprintf(buf2, 4, "%02X ", (uint8_t)msg->data[ctr2]);
                strncat(buf, buf2, 255);
            }
            for ( ; ctr2 % 16 ; ctr2++) {
                snprintf(buf2, 4, "   ");
                strncat(buf, buf2, 255);
            }
            buf[strlen(buf)-1] = '\0';
            snprintf(buf2, 8, "      ");
            strncat(buf, buf2, 255);
            for (ctr2 = ctr; ctr2 < msg->len; ctr2++) {
                snprintf(buf2, 8, "%c", (uint8_t)msg->data[ctr2]    > 32 && (uint8_t)msg->data[ctr2]    < 127 ? (uint8_t)msg->data[ctr2]     : '.');
                strncat(buf, buf2, 255);
            }
            _v3_debug(V3_DEBUG_PACKET, "PACKET:     %s", buf);
        } else {
            _v3_debug(V3_DEBUG_PACKET, "PACKET:     %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X      %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
                    (uint8_t)msg->data[ctr],
                    (uint8_t)msg->data[ctr+1],
                    (uint8_t)msg->data[ctr+2],
                    (uint8_t)msg->data[ctr+3],
                    (uint8_t)msg->data[ctr+4],
                    (uint8_t)msg->data[ctr+5],
                    (uint8_t)msg->data[ctr+6],
                    (uint8_t)msg->data[ctr+7],
                    (uint8_t)msg->data[ctr+8],
                    (uint8_t)msg->data[ctr+9],
                    (uint8_t)msg->data[ctr+10],
                    (uint8_t)msg->data[ctr+11],
                    (uint8_t)msg->data[ctr+12],
                    (uint8_t)msg->data[ctr+13],
                    (uint8_t)msg->data[ctr+14],
                    (uint8_t)msg->data[ctr+15],
                    (uint8_t)msg->data[ctr]    > 32 && (uint8_t)msg->data[ctr]    < 127 ? (uint8_t)msg->data[ctr]     : '.',
                    (uint8_t)msg->data[ctr+1]  > 32 && (uint8_t)msg->data[ctr+1]  < 127 ? (uint8_t)msg->data[ctr+1]   : '.',
                    (uint8_t)msg->data[ctr+2]  > 32 && (uint8_t)msg->data[ctr+2]  < 127 ? (uint8_t)msg->data[ctr+2]   : '.',
                    (uint8_t)msg->data[ctr+3]  > 32 && (uint8_t)msg->data[ctr+3]  < 127 ? (uint8_t)msg->data[ctr+3]   : '.',
                    (uint8_t)msg->data[ctr+4]  > 32 && (uint8_t)msg->data[ctr+4]  < 127 ? (uint8_t)msg->data[ctr+4]   : '.',
                    (uint8_t)msg->data[ctr+5]  > 32 && (uint8_t)msg->data[ctr+5]  < 127 ? (uint8_t)msg->data[ctr+5]   : '.',
                    (uint8_t)msg->data[ctr+6]  > 32 && (uint8_t)msg->data[ctr+6]  < 127 ? (uint8_t)msg->data[ctr+6]   : '.',
                    (uint8_t)msg->data[ctr+7]  > 32 && (uint8_t)msg->data[ctr+7]  < 127 ? (uint8_t)msg->data[ctr+7]   : '.',
                    (uint8_t)msg->data[ctr+8]  > 32 && (uint8_t)msg->data[ctr+8]  < 127 ? (uint8_t)msg->data[ctr+8]   : '.',
                    (uint8_t)msg->data[ctr+9]  > 32 && (uint8_t)msg->data[ctr+9]  < 127 ? (uint8_t)msg->data[ctr+9]   : '.',
                    (uint8_t)msg->data[ctr+10] > 32 && (uint8_t)msg->data[ctr+10] < 127 ? (uint8_t)msg->data[ctr+10]  : '.',
                    (uint8_t)msg->data[ctr+11] > 32 && (uint8_t)msg->data[ctr+11] < 127 ? (uint8_t)msg->data[ctr+11]  : '.',
                    (uint8_t)msg->data[ctr+12] > 32 && (uint8_t)msg->data[ctr+12] < 127 ? (uint8_t)msg->data[ctr+12]  : '.',
                    (uint8_t)msg->data[ctr+13] > 32 && (uint8_t)msg->data[ctr+13] < 127 ? (uint8_t)msg->data[ctr+13]  : '.',
                    (uint8_t)msg->data[ctr+14] > 32 && (uint8_t)msg->data[ctr+14] < 127 ? (uint8_t)msg->data[ctr+14]  : '.',
                    (uint8_t)msg->data[ctr+15] > 32 && (uint8_t)msg->data[ctr+15] < 127 ? (uint8_t)msg->data[ctr+15]  : '.'
                        );
        }
    }
    return;
}/*}}}*/

void
_v3_print_permissions(v3_permissions *perms) {/*{{{*/
    _v3_debug(V3_DEBUG_PACKET_PARSE, "User Permissions:");
    _v3_debug(V3_DEBUG_PACKET_PARSE, "account_id..........: %d",   perms->account_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "replace_owner_id....: %d",   perms->replace_owner_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "rank_id.............: %d",   perms->rank_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_1......: %d",   perms->unknown_perm_1);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "lock_acct...........: %d",   perms->lock_acct);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "in_reserve_list.....: %d",   perms->in_reserve_list);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "dupe_ip.............: %d",   perms->dupe_ip);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "switch_chan.........: %d",   perms->switch_chan);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "dfl_chan............: %d",   perms->dfl_chan);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_2......: %d",   perms->unknown_perm_2);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_3......: %d",   perms->unknown_perm_3);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "recv_bcast..........: %d",   perms->recv_bcast);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "add_phantom.........: %d",   perms->add_phantom);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "record..............: %d",   perms->record);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "recv_complaint......: %d",   perms->recv_complaint);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "send_complaint......: %d",   perms->send_complaint);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "inactive_exempt.....: %d",   perms->inactive_exempt);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_4......: %d",   perms->unknown_perm_4);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_5......: %d",   perms->unknown_perm_5);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "srv_admin...........: %d",   perms->srv_admin);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "add_user............: %d",   perms->add_user);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "del_user............: %d",   perms->del_user);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "ban_user............: %d",   perms->ban_user);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "kick_user...........: %d",   perms->kick_user);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "move_user...........: %d",   perms->move_user);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "assign_chan_admin...: %d",   perms->assign_chan_admin);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "edit_rank...........: %d",   perms->edit_rank);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "edit_motd...........: %d",   perms->edit_motd);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "edit_guest_motd.....: %d",   perms->edit_guest_motd);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "issue_rcon_cmd......: %d",   perms->issue_rcon_cmd);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "edit_voice_target; .: %d",   perms->edit_voice_target);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "edit_command_target.: %d",   perms->edit_command_target);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "assign_rank.........: %d",   perms->assign_rank);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "assign_reserved.....: %d",   perms->assign_reserved);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_6......: %d",   perms->unknown_perm_6);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_7......: %d",   perms->unknown_perm_7);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_8......: %d",   perms->unknown_perm_8);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_9......: %d",   perms->unknown_perm_9);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_10.....: %d",   perms->unknown_perm_10);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "bcast...............: %d",   perms->bcast);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "bcast_lobby.........: %d",   perms->bcast_lobby);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "bcast_user..........: %d",   perms->bcast_user);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "bcast_x_chan........: %d",   perms->bcast_x_chan);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "send_tts_bind.......: %d",   perms->send_tts_bind);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "send_wav_bind.......: %d",   perms->send_wav_bind);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "send_page...........: %d",   perms->send_page);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "send_comment........: %d",   perms->send_comment);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "set_phon_name.......: %d",   perms->set_phon_name);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "gen_comment_snds....: %d",   perms->gen_comment_snds);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "event_snds..........: %d",   perms->event_snds);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "mute_glbl...........: %d",   perms->mute_glbl);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "mute_other..........: %d",   perms->mute_other);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "glbl_chat...........: %d",   perms->glbl_chat);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "start_priv_chat.....: %d",   perms->start_priv_chat);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_11.....: %d",   perms->unknown_perm_11);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "eq_out..............: %d",   perms->eq_out);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_12.....: %d",   perms->unknown_perm_12);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_13.....: %d",   perms->unknown_perm_13);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_14.....: %d",   perms->unknown_perm_14);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "see_guest...........: %d",   perms->see_guest);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "see_nonguest........: %d",   perms->see_nonguest);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "see_motd............: %d",   perms->see_motd);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "see_srv_comment; ...: %d",   perms->see_srv_comment);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "see_chan_list.......: %d",   perms->see_chan_list);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "see_chan_comment....: %d",   perms->see_chan_comment);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "see_user_comment....: %d",   perms->see_user_comment);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_15.....: %d",   perms->unknown_perm_15);
}/*}}}*/

int
_v3_destroy_packet(_v3_net_message *msg) {/*{{{*/
    _v3_func_enter("_v3_destroy_packet");
    if (msg->contents == msg->data) {
        _v3_debug(V3_DEBUG_MEMORY, "contents and data are same memory.  freeing contents");
        free(msg->contents);
        msg->contents = NULL;
        msg->data = NULL;
    }
    if (msg->contents != NULL) {
        _v3_debug(V3_DEBUG_MEMORY, "freeing contents");
        free(msg->contents);
        msg->contents = NULL;
    }
    if (msg->data != NULL) {
        _v3_debug(V3_DEBUG_MEMORY, "freeing data");
        free(msg->data);
        msg->data = NULL;
    }
    if (msg != NULL) {
        _v3_debug(V3_DEBUG_MEMORY, "freeing msg");
        free(msg);
        msg = NULL;
    }

    _v3_func_leave("_v3_destroy_packet");
    return true;
}/*}}}*/

int
_v3_next_timestamp(struct timeval *result, struct timeval *timestamp) {/*{{{*/
    struct timeval now, last;

    gettimeofday(&now, NULL);
    memcpy(&last, timestamp, sizeof(struct timeval));
    last.tv_sec += 10;
    if (last.tv_usec < now.tv_usec) {
        int nsec = (now.tv_usec - last.tv_usec) / 1000000 + 1;
        now.tv_usec -= 1000000 * nsec;
        now.tv_sec += nsec;
    }
    if (last.tv_usec - now.tv_usec > 1000000) {
        int nsec = (last.tv_usec - now.tv_usec) / 1000000;
        now.tv_usec += 1000000 * nsec;
        now.tv_sec -= nsec;
    }

    result->tv_sec = last.tv_sec - now.tv_sec;
    result->tv_usec = last.tv_usec - now.tv_usec;

    if (result->tv_sec < 0) {
        result->tv_sec = 0;
        result->tv_usec = 0;
    }
    // return 1 if negative
    return last.tv_sec < now.tv_sec;
}/*}}}*/

int
_v3_server_auth(struct in_addr *srvip, uint16_t srvport) {/*{{{*/
    int sd, len, hs_srv_num;
    char buf[1024];
    struct sockaddr_in sa;
    struct  timeval tout;
    uint8_t handshake_key[256];
    uint8_t handshake[16];
    fd_set  fd_read;

    _v3_func_enter("_v3_server_auth");
    memset(buf, 0, 1024);
    strncpy(buf+4, "UDCL", 5);
    buf[9] = 4;
    buf[11] = 200;
    buf[12] = 2;

    sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sd < 0) {
        _v3_error("could not authenticate server: failed to create socket");
        _v3_func_leave("_v3_server_auth");
        return false;
    }

    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = srvip->s_addr;
    sa.sin_port = htons(srvport);
    _v3_debug(V3_DEBUG_INFO, "checking version of %s:%d", inet_ntoa(*srvip), srvport);
    if (sendto(sd, buf, 200, 0, (struct sockaddr *)&sa,  sizeof(struct sockaddr_in)) == -1) {
        _v3_error("could not authenticate server: failed to send auth packet");
        shutdown(sd, SHUT_WR);
        close(sd);
        _v3_func_leave("_v3_server_auth");
        return false;
    }
    tout.tv_sec  = 4;
    tout.tv_usec = 0;
    FD_ZERO(&fd_read);
    FD_SET(sd, &fd_read);
    if(select(sd + 1, &fd_read, NULL, NULL, &tout) <= 0) {
        _v3_error("could not authenticate server: timed out waiting for auth server");
        _v3_func_leave("_v3_server_auth");
        return false;
    }
    len = recvfrom(sd, buf, sizeof(buf), 0, NULL, NULL);
    if (len < 0) {
        _v3_error("could not authenticate server: udp receive failed");
        _v3_func_leave("_v3_server_auth");
        return false;
    }
    if (buf[168] < '3') {
        _v3_error("could not authenticate server: server is not ventrilo version 3");
    } else {
        _v3_debug(V3_DEBUG_INFO, "Server Name   : %s", buf+72);
        v3_server.name = strdup(buf+72);
        _v3_debug(V3_DEBUG_INFO, "Server OS     : %s", buf+136);
        v3_server.os = strdup(buf+136);
        _v3_debug(V3_DEBUG_INFO, "Server Version: %s", buf+168);
        v3_server.version = strdup(buf+168);
    }
    ventrilo3_handshake(srvip->s_addr, srvport, handshake, (uint32_t *)&hs_srv_num, handshake_key);
    v3_server.handshake_key = malloc(256);
    memcpy(v3_server.handshake_key, handshake_key, 64);
    v3_server.handshake = malloc(16);
    memcpy(v3_server.handshake, handshake, 16);
    v3_server.auth_server_index = hs_srv_num;
    _v3_debug(V3_DEBUG_INFO, "authserver index: %d -> %d", hs_srv_num, v3_server.auth_server_index);
    _v3_func_leave("_v3_server_auth");
    return true;
}/*}}}*/

int
_v3_login_connect(struct in_addr *srvip, uint16_t srvport) {/*{{{*/
    struct linger ling = {1, 1};
    struct sockaddr_in sa;

    _v3_func_enter("_v3_login_connect");
    _v3_sockd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    setsockopt(_v3_sockd, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof(ling));

    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = srvip->s_addr;
    sa.sin_port = htons(srvport);
    if (connect(_v3_sockd, (struct sockaddr *)&sa, sizeof(sa)) == -1) {
        _v3_error("failed to connect: %s", strerror(errno));
        _v3_func_leave("_v3_login_connect");
        return false;
    }
    _v3_func_leave("_v3_login_connect");
    return true;
}/*}}}*/

int
_v3_send(_v3_net_message *message) {/*{{{*/

    _v3_func_enter("_v3_send");
    _v3_debug(V3_DEBUG_PACKET, "======= building TCP packet =====================================");
    _v3_net_message_dump(message);

    // Encrypt the packet
    ventrilo_enc(&v3_server.client_key, (uint8_t *)message->data, message->len);

    // Send it on...
    _v3_send_enc_msg(message->data, message->len);

    _v3_func_leave("_v3_send");
    return true;
}/*}}}*/

int
_v3_send_enc_msg(char *data, int len) {/*{{{*/
    uint16_t lenptr;
    uint8_t buf[len+2];

    _v3_func_enter("_v3_send_enc_msg");
    _v3_debug(V3_DEBUG_PACKET_ENCRYPTED, "======= sending encrypted TCP packet ============================");
    _v3_net_message_dump_raw(data, len);
    v3_server.sent_packet_count++;
    v3_server.sent_byte_count += len + 2;
    lenptr = htons(len);
    memcpy(buf, &lenptr, 2);
    memcpy(buf+2, data, len);
    if (send(_v3_sockd, buf, len+2, 0) != len+2) {
        _v3_error("failed to send packet data");
        _v3_func_leave("_v3_send_enc_msg");
        return false;
    }
    _v3_func_leave("_v3_send_enc_msg");
    return true;
}/*}}}*/

/*
 * The majority of outbound message processing happens here since this function
 * also reads from the event pipe from the client
 *
 * This is more like a mainloop than a receive function.
 */
_v3_net_message *
_v3_recv(int block) {/*{{{*/
    _v3_net_message *msg;
    char msgdata[0xffff];
    int waiting;

    _v3_func_enter("_v3_recv");
    if (! _v3_is_connected()) {
        _v3_func_leave("_v3_recv");
        return NULL;
    }
    while ((waiting = v3_message_waiting(block)) != 0) {
        _v3_debug(V3_DEBUG_EVENT, "message waiting: %d", waiting);
        if (waiting == V3_BOTH_WAITING || waiting == V3_EVENT_WAITING) {
            // receiving an event from the event pipe
            v3_event ev;
            memset(&ev, 0, sizeof(v3_event));
            _v3_debug(V3_DEBUG_EVENT, "event waiting to be processed and sent outbound");
            if (fread(&ev, sizeof(ev), 1, v3_server.evinstream) != 1) {
                _v3_error("failed to receive from outbound pipe");
            } else {
                switch (ev.type) {
                    case V3_EVENT_DISCONNECT:/*{{{*/
                        {
                            _v3_logout();
                        }
                        break;/*}}}*/
                    case V3_EVENT_CHANGE_CHANNEL:/*{{{*/
                        {
                            _v3_net_message *msg;
                            v3_channel channel;
                            memset(&channel, 0, sizeof(_v3_msg_channel));
                            channel.id = ev.channel.id;
                            _v3_debug(V3_DEBUG_INFO, "changing to channel id %d", channel.id);
                            msg = _v3_put_0x49(V3_CHANGE_CHANNEL, v3_get_user_id(), ev.text.password, &channel);
                            if (!_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send channel change message");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_CHAN_MODIFY:
                    case V3_EVENT_CHAN_ADD:/*{{{*/
                        {
                            _v3_net_message *msg;
                            v3_channel channel;
                            memset(&channel, 0, sizeof(_v3_msg_channel));
                            /* channel.id = ev.channel.id; */
                            memcpy(&channel, &ev.data->channel, 48);
                            channel.name = ev.text.name;
                            channel.phonetic = ev.text.phonetic;
                            channel.comment = ev.text.comment;
                            if (channel.id) {
                                _v3_debug(V3_DEBUG_INFO, "modify channel id %d", channel.id);
                            } else {
                                _v3_debug(V3_DEBUG_INFO, "add channel '%s'", channel.name);
                            }
                            msg = _v3_put_0x49(channel.id ? V3_MODIFY_CHANNEL : V3_ADD_CHANNEL, v3_get_user_id(), ev.text.password, &channel);
                            if (!_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send channel modify/add message");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_CHAN_REMOVE:/*{{{*/
                        {
                            _v3_net_message *msg;
                            v3_channel channel;
                            memset(&channel, 0, sizeof(_v3_msg_channel));
                            channel.id = ev.channel.id;
                            _v3_debug(V3_DEBUG_INFO, "removing channel id %d", channel.id);
                            msg = _v3_put_0x49(V3_REMOVE_CHANNEL, v3_get_user_id(), NULL, &channel);
                            if (!_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send channel remove message");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_RANKLIST_OPEN:/*{{{*/
                        {
                            _v3_net_message *msg;
                            msg = _v3_put_0x36(V3_OPEN_RANK, NULL);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent rank list open request to server");
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send rank list open request");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_RANKLIST_CLOSE:/*{{{*/
                        {
                            _v3_net_message *msg;
                            msg = _v3_put_0x36(V3_CLOSE_RANK, NULL);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent rank list close request to server");
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send rank list close request");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_RANK_REMOVE:/*{{{*/
                        {
                            _v3_net_message *msg;
                            v3_rank rank;
                            memset(&rank, 0, sizeof(v3_rank));
                            rank.id = ev.data->rank.id;
                            rank.level = ev.data->rank.level;
                            msg = _v3_put_0x36(V3_REMOVE_RANK, &rank);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent rank remove request to server");
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send rank remove request");
                            }
                            _v3_destroy_packet(msg);

                        }
                        break;/*}}}*/
                    case V3_EVENT_RANK_MODIFY:
                    case V3_EVENT_RANK_ADD:/*{{{*/
                        {
                            uint16_t subtype;
                            const char *subtype_str;
                            if (ev.type == V3_EVENT_RANK_MODIFY) {
                                subtype = V3_MODIFY_RANK;
                                subtype_str = "modify";
                            } else {
                                subtype = V3_ADD_RANK;
                                subtype_str = "add";
                            }
                            _v3_net_message *msg;
                            v3_rank rank;
                            memset(&rank, 0, sizeof(v3_rank));
                            rank.id = ev.data->rank.id;
                            rank.level = ev.data->rank.level;
                            rank.name = ev.text.name;
                            rank.description = ev.text.comment;
                            msg = _v3_put_0x36(subtype, &rank);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent rank %s request to server", subtype_str);
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send rank %s request", subtype_str);
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_USER_TALK_START:/*{{{*/
                        {
                            _v3_net_message *msg;
                            const v3_codec  *codec;

                            codec = v3_get_channel_codec(v3_get_user_channel(v3_get_user_id()));
                            _v3_debug(V3_DEBUG_INFO, "got outbound user talk start event");
                            msg = _v3_put_0x52(V3_AUDIO_START, codec->codec, codec->format, ev.pcm.send_type, 0, 0, NULL);
                            if (!_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send user talk start message");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_PLAY_AUDIO:/*{{{*/
                        {
                            _v3_net_message *msg;
                            const v3_codec *codec = v3_get_channel_codec(v3_get_user_channel(v3_get_user_id()));
                            uint8_t *data         = NULL;
                            uint16_t datalen      = 0;
                            uint16_t framecount   = 0;
                            uint8_t celtfragsize  = 0;

                            _v3_debug(V3_DEBUG_INFO, "got outbound play audio event");
                            if ((data = _v3_audio_encode(
                                            /* pcm input */
                                            (void *)&ev.data->sample,
                                            ev.pcm.length,
                                            /* encoded output */
                                            codec,
                                            &datalen,
                                            /* optional args */
                                            ev.pcm.channels,
                                            &framecount,
                                            &celtfragsize))) {
                                switch (codec->codec) {
                                  case 0x01: // CELT
                                  case 0x02:
                                  {
                                    uint8_t *celtdataptr = data;
                                    uint16_t pktlen      = (codec->codec == 0x01) ? 198 : 108; // max data length for packet
                                    uint8_t pktframes    = pktlen / celtfragsize; // max celt frames per packet
                                    pktlen               = pktframes * celtfragsize; // new length for frames to fill a packet

                                    while (framecount && pktframes) {
                                        if (framecount < pktframes) {
                                            pktframes = framecount;
                                            pktlen = pktframes * celtfragsize;
                                        }
                                        msg = _v3_put_0x52(
                                                V3_AUDIO_DATA,
                                                codec->codec,
                                                codec->format,
                                                ev.pcm.send_type,
                                                2000 + ev.pcm.channels, // max: <= 3000
                                                pktlen, // max: 0x01: < 200; 0x02: < 110
                                                celtdataptr);
                                        celtdataptr += pktlen;
                                        if (!_v3_send(msg)) {
                                            _v3_debug(V3_DEBUG_SOCKET, "failed to send audio message");
                                        }
                                        _v3_destroy_packet(msg);
                                        framecount -= pktframes;
                                    }
                                    break;
                                  }
                                  default:
                                    msg = _v3_put_0x52(
                                            V3_AUDIO_DATA,
                                            codec->codec,
                                            codec->format,
                                            ev.pcm.send_type,
                                            ev.pcm.length,
                                            datalen,
                                            data);
                                    if (!_v3_send(msg)) {
                                        _v3_debug(V3_DEBUG_SOCKET, "failed to send audio message");
                                    }
                                    _v3_destroy_packet(msg);
                                    break;
                                }
                                _v3_vrf_record_event(
                                        V3_VRF_EVENT_AUDIO_DATA,
                                        v3_get_user_id(),
                                        codec->codec,
                                        codec->format,
                                        ev.pcm.length,
                                        datalen,
                                        data);
                                free(data);
                                data = NULL;
                            }
                        }
                        break;/*}}}*/
                    case V3_EVENT_USER_TALK_END:/*{{{*/
                        {
                            _v3_net_message *msg;

                            _v3_debug(V3_DEBUG_INFO, "got outbound user talk end event");
                            msg = _v3_put_0x52(V3_AUDIO_STOP, -1, -1, 0, 0, 0, NULL);
                            if (!_v3_send(msg)) {
                                 _v3_debug(V3_DEBUG_SOCKET, "failed to send user talk end message");
                            }
                            _v3_destroy_packet(msg);
                            _v3_vrf_record_event(V3_VRF_EVENT_AUDIO_STOP, v3_get_user_id(), -1, -1, 0, 0, NULL);
                        }
                        break;/*}}}*/
                    case V3_EVENT_USER_MODIFY:/*{{{*/
                        {
                            _v3_net_message *msg;
                            v3_user *user;

                            user = v3_get_user(v3_luser.id);
                            free(user->name);
                            free(user->phonetic);
                            free(user->comment);
                            free(user->url);
                            free(user->integration_text);
                            if (ev.flags & 0x100) {
                                user->bitfield |= 0x100;
                            }
                            user->name = strdup("");
                            user->phonetic = strdup("");
                            user->comment = strdup(ev.text.comment);
                            user->url = strdup(ev.text.url);
                            user->integration_text = strdup(ev.text.integration_text);
                            _v3_debug(V3_DEBUG_INFO, "setting cm: %s, url: %s, integration: %s", ev.text.comment, ev.text.url, ev.text.integration_text);
                            msg = _v3_put_0x5d(V3_MODIFY_USER, 1, user);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent text string changes to server");
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send text string changes message");
                            }
                            v3_free_user(user);
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_CHAT_JOIN:/*{{{*/
                        {
                            _v3_net_message *msg = _v3_put_0x42(V3_JOIN_CHAT, v3_luser.id, NULL);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent join chat request to server");
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send join chat request message");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_CHAT_LEAVE:/*{{{*/
                        {
                            _v3_net_message *msg = _v3_put_0x42(V3_LEAVE_CHAT, v3_luser.id, NULL);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent leave chat request to server");
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send leave chat request message");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_CHAT_MESSAGE:/*{{{*/
                        {
                            _v3_net_message *msg = _v3_put_0x42(V3_TALK_CHAT, v3_luser.id, ev.data->chatmessage);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent chat message to server");
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send chat message");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_PRIVATE_CHAT_START:/*{{{*/
                        {
                            _v3_net_message *msg = _v3_put_0x5a(V3_START_PRIV_CHAT, ev.user.id, v3_luser.id, NULL);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent priv chat start request to server for user %d", ev.user.id);
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send priv chat start request message");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_PRIVATE_CHAT_END:/*{{{*/
                        {
                            _v3_net_message *msg = _v3_put_0x5a(V3_END_PRIV_CHAT, ev.user.id, v3_luser.id, NULL);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent priv chat end request to server for user %d", ev.user.id);
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send priv chat end request message");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_PRIVATE_CHAT_MESSAGE:/*{{{*/
                        {
                            _v3_net_message *msg = _v3_put_0x5a(V3_TALK_PRIV_CHAT, ev.user.id, v3_luser.id, ev.data->chatmessage);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent private chat message to server for user %d", ev.user.id);
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send private chat message");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_TEXT_TO_SPEECH_MESSAGE:/*{{{*/
                        {
                            _v3_net_message *msg = _v3_put_0x3a(ev.data->chatmessage);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent text to speech message to server");
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send text to speech message");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_PLAY_WAVE_FILE_MESSAGE:/*{{{*/
                        {
                            _v3_net_message *msg = _v3_put_0x3f(ev.data->chatmessage);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent play wave file message to server");
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send play wave file message");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_PHANTOM_ADD:/*{{{*/
                        {
                            _v3_net_message *msg = _v3_put_0x58(V3_PHANTOM_ADD, ev.channel.id, 0);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent phantom add request to server (channel %d)", ev.channel.id);
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send phantom add request");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_PHANTOM_REMOVE:/*{{{*/
                        {
                            _v3_net_message *msg = _v3_put_0x58(V3_PHANTOM_REMOVE, 0, ev.user.id);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent phantom remove request to server (userid %d)", ev.user.id);
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send phantom remove request");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_FORCE_CHAN_MOVE:/*{{{*/
                        {
                            _v3_net_message *msg = _v3_put_0x3b(ev.user.id, ev.channel.id);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent force channel move request to server (userid %d to chan %d)", ev.user.id, ev.channel.id);
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send force channel move request");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_ADMIN_LOGIN:/*{{{*/
                        {
                            _v3_net_message *msg = _v3_put_0x63(V3_ADMIN_LOGIN, 0, ev.text.password);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent admin login request to server");
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send admin login request");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_ADMIN_LOGOUT:/*{{{*/
                        {
                            _v3_net_message *msg = _v3_put_0x63(V3_ADMIN_LOGOUT, 0, NULL);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent admin logout request to server");
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send admin logout request");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_ADMIN_KICK:/*{{{*/
                        {
                            _v3_net_message *msg = _v3_put_0x63(V3_ADMIN_KICK, ev.user.id, ev.data->reason);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent admin kick request to server");
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send admin kick request");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_ADMIN_BAN:/*{{{*/
                        {
                            _v3_net_message *msg = _v3_put_0x63(V3_ADMIN_BAN, ev.user.id, ev.data->reason);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent admin ban request to server");
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send admin ban request");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_ADMIN_CHANNEL_BAN:/*{{{*/
                        {
                            _v3_net_message *msg = _v3_put_0x63(V3_ADMIN_CHANNEL_BAN, ev.user.id, ev.data->reason);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent admin channel ban request to server");
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send admin channel ban request");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_USERLIST_OPEN:/*{{{*/
                        {
                            _v3_net_message *msg = _v3_put_0x4a(V3_USERLIST_OPEN, NULL, NULL);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent userlist open request to server");
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send userlist open request");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_USERLIST_CLOSE:/*{{{*/
                        {
                            _v3_net_message *msg = _v3_put_0x4a(V3_USERLIST_CLOSE, NULL, NULL);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent userlist close request to server");
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send userlist close request");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_USERLIST_REMOVE:/*{{{*/
                        {
                            v3_account *a = v3_get_account(ev.account.id);
                            _v3_net_message *msg = _v3_put_0x4a(V3_USERLIST_REMOVE, a, NULL);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent userlist remove request to server");
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send userlist remove request");
                            }
                            v3_free_account(a);
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_USERLIST_ADD:
                    case V3_EVENT_USERLIST_MODIFY:/*{{{*/
                        {
                            v3_account a;
                            memset(&a, 0, sizeof(v3_account));
                            a.perms = ev.data->account.perms;
                            a.username = strdup(ev.data->account.username);
                            a.owner = strdup(ev.data->account.owner);
                            a.notes = strdup(ev.data->account.notes);
                            a.lock_reason = strdup(ev.data->account.lock_reason);
                            a.chan_admin_count = ev.data->account.chan_admin_count;
                            a.chan_admin = malloc(a.chan_admin_count * 2);
                            memcpy(a.chan_admin, ev.data->account.chan_admin, ev.data->account.chan_admin_count * 2);
                            a.chan_auth_count = ev.data->account.chan_auth_count;
                            a.chan_auth = malloc(a.chan_auth_count * 2);
                            memcpy(a.chan_auth, ev.data->account.chan_auth, ev.data->account.chan_auth_count * 2);
                            a.next = NULL;

                            _v3_net_message *msg = _v3_put_0x4a(ev.type == V3_EVENT_USERLIST_ADD ? V3_USERLIST_ADD : V3_USERLIST_MODIFY, &a, NULL);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent userlist add/modify request to server");
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send userlist add/modify request");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_USERLIST_CHANGE_OWNER:/*{{{*/
                        {
                            v3_account *a = v3_get_account(ev.account.id);
                            v3_account *b = v3_get_account(ev.account.id2);
                            _v3_net_message *msg = _v3_put_0x4a(V3_USERLIST_CHANGE_OWNER, a, b);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent userlist change owner request to server");
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send userlist change owner request");
                            }
                            v3_free_account(a);
                            v3_free_account(b);
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    case V3_EVENT_SRV_PROP_OPEN:/*{{{*/
                        {
                            _v3_net_message *msg = _v3_put_0x4c(V3_SERVER_RECV_SETTING, 0x00, 0x00, 0x42, NULL);
                            if (_v3_send(msg)) {
                                _v3_debug(V3_DEBUG_SOCKET, "sent server property open request to server");
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send server property open request");
                            }
                            _v3_destroy_packet(msg);
                        }
                        break;/*}}}*/
                    default:
                        _v3_debug(V3_DEBUG_EVENT, "received unknown event type %d from queue", ev.type);
                        break;
                }
                if (ev.data) {
                    free(ev.data);
                }
            }
            if (waiting != V3_BOTH_WAITING) {
                continue;
            }
        }
        if (waiting == V3_BOTH_WAITING || waiting == V3_MSG_WAITING) {
            // receiving a message from the network
            _v3_debug(V3_DEBUG_EVENT, "msg waiting to recv inbound");
            msg = malloc(sizeof(_v3_net_message));
            memset(msg, 0, sizeof(_v3_net_message));
            msg->len = 0;
            msg->type = -1;
            msg->data = NULL;
            msg->next = NULL;
            if ((msg->len = _v3_recv_enc_msg(msgdata)) <= 0) {
                if (msg->len == 0) {
                    _v3_debug(V3_DEBUG_SOCKET, "server closed connection");
                    _v3_logout();
                } else {
                    _v3_debug(V3_DEBUG_SOCKET, "receive failed");
                    free(msg);
                    _v3_logout();
                    _v3_func_leave("_v3_recv");
                    return NULL;
                }
                free(msg);
                _v3_func_leave("_v3_recv");
                return NULL;
            } else {
                v3_server.recv_packet_count++;
                v3_server.recv_byte_count += msg->len;
                _v3_debug(V3_DEBUG_SOCKET, "received %d bytes", msg->len);
            }

            if(v3_server.recv_packet_count == 1) {
                ventrilo_first_dec((uint8_t *)msgdata, msg->len);
            } else {
                ventrilo_dec(&v3_server.server_key, (uint8_t *)msgdata, msg->len);
            }
            _v3_debug(V3_DEBUG_INTERNAL, "decryption complete");
            memcpy(&msg->type, msgdata, 2);
            msg->data = malloc(msg->len);
            memcpy(msg->data, msgdata, msg->len);
            _v3_debug(V3_DEBUG_PACKET, "======= received TCP packet =====================================");
            _v3_net_message_dump(msg);
            block = V3_NONBLOCK;
            _v3_func_leave("_v3_recv");
            return msg;
        }
    }
    _v3_func_leave("nothing waiting and non-blocking requested");
    _v3_func_leave("_v3_recv");
    return NULL;
}/*}}}*/

int
_v3_recv_enc_msg(char *data) {/*{{{*/
    uint16_t len;
    int recvlen, packlen;
    char buff[0xffff];

    _v3_func_enter("_v3_recv_enc_msg");
    if (! _v3_is_connected()) {
        _v3_func_leave("_v3_recv_enc_msg");
        return 0;
    }
    _v3_debug(V3_DEBUG_PACKET, "======= receiving encrypted TCP packet ============================");
    if ((recvlen = recv(_v3_sockd, (uint8_t *)&len+1, 1, 0)) != 1) {
        if (recvlen == 0 || recvlen == -1) {
            _v3_error("server closed connection");
            _v3_close_connection();
            _v3_func_leave("_v3_recv_enc_msg");
            return 0;
        }
        _v3_error("socket error on packet length byte 1 recv (%d): %s", recvlen, strerror(errno));
        _v3_func_leave("_v3_recv_enc_msg");
        return -1;
    }
    if ((recvlen = recv(_v3_sockd, (uint8_t *)&len, 1, 0)) != 1) {
        if (recvlen == 0 || recvlen == -1) {
            _v3_error("server closed connection");
            _v3_close_connection();
            _v3_func_leave("_v3_recv_enc_msg");
            return 0;
        }
        _v3_error("socket error on packet length byte 1 recv (%d): %s", recvlen, strerror(errno));
        _v3_func_leave("_v3_recv_enc_msg");
        return -1;
    }
    _v3_debug(V3_DEBUG_SOCKET, "receiving packet length %d bytes", len);
    for (packlen = 0; packlen < len; packlen += recvlen) {
        recvlen = recv(_v3_sockd, buff+packlen, len - packlen, 0);
        if (recvlen == 0) {
            _v3_error("server has closed connected");
            _v3_close_connection();
            _v3_func_leave("_v3_recv_enc_msg");
            return 0;
        }
        if (recvlen == -1) {
            _v3_error("failed to recv packet: %s", recvlen, strerror(errno));
            _v3_func_leave("_v3_recv_enc_msg");
            return -1;
        }
        _v3_debug(V3_DEBUG_SOCKET, "received %d of %d bytes (total: %d)", recvlen, len, packlen+recvlen);
    }
    memcpy(data, buff, len);
    _v3_net_message_dump_raw(data, len);
    _v3_func_leave("_v3_recv_enc_msg");
    return len;
}/*}}}*/

int
v3_message_waiting(int block) {/*{{{*/
    fd_set rset;
    int ret;
    struct timeval tv, now;

    _v3_func_enter("v3_message_waiting");
    if (! _v3_is_connected()) {
        _v3_func_leave("v3_message_waiting");
        return false;
    }
    FD_ZERO(&rset);
    FD_SET(_v3_sockd, &rset);
    FD_SET(v3_server.evpipe[0], &rset);

    /*
     * Every 10 seconds, the client sends a timestamp message to the server.  I
     * don't know WHY the client does this, but it does... so we do too.  As
     * such, if we're blocking, we need to timeout in those 10 second intervals.
     */
    if (block) {
        gettimeofday(&now, NULL);
        _v3_next_timestamp(&tv, &v3_server.last_timestamp);
    } else {
        tv.tv_sec=0;
        tv.tv_usec=0;
    }
    _v3_debug(V3_DEBUG_INFO, "outbound timestamp pending in %d.%d seconds", tv.tv_sec, tv.tv_usec);

    while ((ret = select(_v3_sockd > v3_server.evpipe[0] ? _v3_sockd+1 : v3_server.evpipe[0]+1, &rset, NULL, NULL, &tv)) >= 0) {
        _v3_next_timestamp(&tv, &v3_server.last_timestamp);
        _v3_debug(V3_DEBUG_INFO, "outbound timestamp pending in %d.%d seconds", tv.tv_sec, tv.tv_usec);
        if (!_v3_is_connected()) {
            _v3_func_leave("v3_message_waiting");
            return false;
        }
        if (ret == 0 && block) {
            _v3_net_message *m;
            FD_ZERO(&rset);
            FD_SET(v3_server.evpipe[0], &rset);
            FD_SET(_v3_sockd, &rset);

            m = _v3_put_0x4b();
            _v3_send(m);
            _v3_destroy_packet(m);
            gettimeofday(&v3_server.last_timestamp, NULL);
            _v3_next_timestamp(&tv, &v3_server.last_timestamp);
            continue;
        }
        if (FD_ISSET(v3_server.evpipe[0], &rset) && FD_ISSET(_v3_sockd, &rset)) {
            _v3_debug(V3_DEBUG_SOCKET, "incoming event and outgoing msg waiting to be processed");
            _v3_func_leave("v3_message_waiting");
            return V3_BOTH_WAITING;
        } else if (FD_ISSET(v3_server.evpipe[0], &rset)) {
            _v3_debug(V3_DEBUG_SOCKET, "incoming event waiting to be processed");
            _v3_func_leave("v3_message_waiting");
            return V3_EVENT_WAITING;
        } else if (FD_ISSET(_v3_sockd, &rset)) {
            _v3_debug(V3_DEBUG_SOCKET, "incoming data waiting to be received");
            _v3_func_leave("v3_message_waiting");
            return V3_MSG_WAITING;
        } else {
            _v3_debug(V3_DEBUG_SOCKET, "no data waiting to be received");
            _v3_func_leave("v3_message_waiting");
            return false;
        }
    }
    _v3_error("select failed");
    return false;
}/*}}}*/

void
_v3_hash_password(uint8_t *password, uint8_t *hash) {/*{{{*/
    uint32_t crc, i, j, cnt, len;
    uint8_t  tmp[4] = { 0 };

    len = cnt = strlen((char *)password);
    for(i = 0; i < 32; i++, cnt++) {
        hash[i] = i < len ? password[i] : ((tmp[(cnt + 1) & 3] + hash[i-len]) - 0x3f) & 0x7f;
        for(j = 0, crc = 0; j < i + 1; j++) {
            crc = _v3_hash_table[hash[j] ^ (crc & 0xff)] ^ (crc >> 8);
        }
        *(uint32_t*)tmp = htonl(crc);
        cnt += hash[i];
        if(crc) {
            while(!tmp[cnt & 3]) cnt++;
        }
        hash[i] += tmp[cnt & 3];
    }
}/*}}}*/

int
_v3_close_connection(void) {/*{{{*/
    _v3_func_enter("_v3_close_connection");
    _v3_user_loggedin = 0;
    shutdown(_v3_sockd, SHUT_WR);
    close(_v3_sockd);
    _v3_sockd = -1;
    _v3_func_leave("_v3_close_connection");
    return _v3_sockd == -1 ? true : false;
}/*}}}*/

int
_v3_is_connected(void) {/*{{{*/
    _v3_func_enter("_v3_is_connected");
    if (_v3_sockd != -1) {
        _v3_debug(V3_DEBUG_SOCKET, "client is connected with socket descriptor: %d", _v3_sockd);
    } else {
        _v3_debug(V3_DEBUG_SOCKET, "client is not connected", _v3_sockd);
    }
    _v3_func_leave("_v3_is_connected");
    return _v3_sockd == -1 ? false : true;
}/*}}}*/

int
_v3_update_channel(v3_channel *channel) {/*{{{*/
    v3_channel *c, *last;

    _v3_func_enter("_v3_update_channel");
    _v3_lock_channellist();
    if (v3_channel_list == NULL) {
        c = malloc(sizeof(v3_channel));
        memset(c, 0, sizeof(v3_channel));
        memcpy(c, channel, sizeof(v3_channel));
        c->name           = strdup(channel->name);
        c->phonetic       = strdup(channel->phonetic);
        c->comment        = strdup(channel->comment);
        c->next           = NULL;
        v3_channel_list = c;
        _v3_debug(V3_DEBUG_INFO, "added first channel %s (codec: %d/%d)",  c->name, c->channel_codec, c->channel_format);
    } else {
        for (c = v3_channel_list; c != NULL; c = c->next) {
            if (c->id == channel->id) {
                void *tmp;
                _v3_debug(V3_DEBUG_INFO, "updating channel %s",  c->name);
                free(c->name);
                free(c->phonetic);
                free(c->comment);
                tmp = c->next;
                memcpy(c, channel, sizeof(v3_channel));
                c->name           = strdup(channel->name);
                c->phonetic       = strdup(channel->phonetic);
                c->comment        = strdup(channel->comment);
                c->next = tmp;
                _v3_debug(V3_DEBUG_INFO, "updated channel %s (codec %d/%d)",  c->name, c->channel_codec, c->channel_format);
                _v3_unlock_channellist();
                _v3_func_leave("_v3_update_channel");
                return true;
            }
            last = c;
        }
        c = last->next = malloc(sizeof(v3_channel));
        memset(c, 0, sizeof(v3_channel));
        memcpy(c, channel, sizeof(v3_channel));
        c->name           = strdup(channel->name);
        c->phonetic       = strdup(channel->phonetic);
        c->comment        = strdup(channel->comment);
        c->next           = NULL;
        _v3_debug(V3_DEBUG_INFO, "added channel %s (codec %d/%d)",  c->name, c->channel_codec, c->channel_format);
    }
    _v3_unlock_channellist();
    _v3_func_leave("_v3_update_channel");
    return true;
}/*}}}*/

int
_v3_update_user(v3_user *user) {/*{{{*/
    v3_user *u, *last;

    _v3_func_enter("_v3_update_user");
    _v3_lock_userlist();
    if (v3_user_list == NULL) { // no users in list... create the first
        u = malloc(sizeof(v3_user));
        memset(u, 0, sizeof(v3_user));
        memcpy(u, user, sizeof(v3_user));
        u->name             = strdup(user->name);
        u->comment          = strdup(user->comment);
        u->phonetic         = strdup(user->phonetic);
        u->integration_text = strdup(user->integration_text);
        u->url              = strdup(user->url);
        u->guest            = user->bitfield & 0x400 ? 1 : 0;
        u->next             = NULL;
        _v3_user_volumes[u->id] = 79;
        v3_user_list = u;
    } else {
        for (u = v3_user_list; u != NULL; u = u->next) { // search for existing users
            if (u->id == user->id) {
                v3_user tmpuser;
                memcpy(&tmpuser, u, sizeof(v3_user));
                // Users cannot change their name
                // free(u->name);
                free(u->phonetic);
                free(u->comment);
                free(u->integration_text);
                free(u->url);
                memcpy(u, user, sizeof(v3_user));
                // Users cannot change their name
                // u->name          = strdup(user->name);
                u->name             = tmpuser.name;
                u->comment          = strdup(user->comment);
                u->phonetic         = strdup(user->phonetic);
                u->integration_text = strdup(user->integration_text);
                u->url              = strdup(user->url);
                u->guest            = user->bitfield & 0x400 ? 1 : 0;
                u->accept_pages     = tmpuser.accept_pages;
                u->accept_u2u       = tmpuser.accept_u2u;
                u->accept_chat      = tmpuser.accept_chat;
                u->allow_recording  = tmpuser.allow_recording;
                u->global_mute      = tmpuser.global_mute;
                u->next             = tmpuser.next;
                _v3_debug(V3_DEBUG_INFO, "updated user %s",  u->name);
                _v3_unlock_userlist();
                _v3_func_leave("_v3_update_user");
                return true;
            }
            last = u;
        } // add a new user
        u = last->next = malloc(sizeof(v3_user));
        memset(u, 0, sizeof(v3_user));
        memcpy(u, user, sizeof(v3_user));
        u->name             = strdup(user->name);
        u->comment          = strdup(user->comment);
        u->phonetic         = strdup(user->phonetic);
        u->integration_text = strdup(user->integration_text);
        u->url              = strdup(user->url);
        u->guest            = user->bitfield & 0x400 ? 1 : 0;
        _v3_user_volumes[u->id] = 79;
        u->next             = NULL;
    }
    _v3_unlock_userlist();
    _v3_func_leave("_v3_update_user");
    return true;
}/*}}}*/

int
_v3_update_rank(v3_rank *rank) {/*{{{*/
    v3_rank *r, *last;

    _v3_func_enter("_v3_update_rank");
    _v3_lock_ranklist();
    if (v3_rank_list == NULL) { // no ranks in list... create the first
        r = malloc(sizeof(v3_rank));
        memset(r, 0, sizeof(v3_rank));
        memcpy(r, rank, sizeof(v3_rank));
        r->name             = strdup(rank->name);
        r->description      = strdup(rank->description);
        r->next             = NULL;
        v3_rank_list = r;
    } else {
        for (r = v3_rank_list; r != NULL; r = r->next) { // search for existing ranks
            if (r->id == rank->id) {
                void *tmp;
                free(r->name);
                free(r->description);
                tmp = r->next;
                memcpy(r, rank, sizeof(v3_rank));
                r->name             = strdup(rank->name);
                r->description      = strdup(rank->description);
                r->next             = tmp;
                _v3_debug(V3_DEBUG_INFO, "updated rank %s",  r->name);
                _v3_unlock_ranklist();
                _v3_func_leave("_v3_update_rank");
                return true;
            }
            last = r;
        } // add a new rank
        r = last->next = malloc(sizeof(v3_rank));
        memset(r, 0, sizeof(v3_rank));
        memcpy(r, rank, sizeof(v3_rank));
        r->name             = strdup(rank->name);
        r->description      = strdup(rank->description);
        r->next             = NULL;
    }
    _v3_unlock_ranklist();
    _v3_func_leave("_v3_update_rank");
    return true;
}/*}}}*/

void
_v3_print_channel_list(void) {/*{{{*/
    v3_channel *c;
    int ctr=0;

    for (c = v3_channel_list; c != NULL; c = c->next) {
        _v3_debug(V3_DEBUG_INFO, "=====[ channel %d ]====================================================================", ctr++);
        _v3_debug(V3_DEBUG_INFO, "channel id      : %d", c->id);
        _v3_debug(V3_DEBUG_INFO, "channel name    : %s", c->name);
        _v3_debug(V3_DEBUG_INFO, "channel comment : %s", c->comment);
        _v3_debug(V3_DEBUG_INFO, "channel phonetic: %s", c->phonetic);
    }
}/*}}}*/

void
_v3_destroy_channellist(void) {/*{{{*/
    v3_channel *c, *next;

    _v3_func_enter("_v3_destroy_channellist");
    _v3_lock_channellist();
    c = v3_channel_list;
    while (c != NULL) {
        free(c->name);
        free(c->comment);
        free(c->phonetic);
        next = c->next;
        free(c);
        c = next;
    }
    v3_channel_list = NULL;
    _v3_unlock_channellist();
    _v3_func_leave("_v3_destroy_channellist");
}/*}}}*/

int
_v3_remove_user(uint16_t id) {/*{{{*/
    v3_user *u, *last;

    _v3_lock_userlist();
    _v3_func_enter("_v3_remove_user");
    last = v3_user_list;
    for (u = v3_user_list; u != NULL; u = u->next) {
        if (u->id == id) {
            last->next = u->next;
            _v3_debug(V3_DEBUG_INFO, "removed user %s",  u->name);
            v3_free_user(u);
            _v3_func_leave("_v3_remove_user");
            _v3_unlock_userlist();
            return true;
        }
        last = u;
    }
    _v3_func_leave("_v3_remove_user");
    _v3_unlock_userlist();
    return false;
}/*}}}*/

int
_v3_remove_channel(uint16_t id) {/*{{{*/
    v3_channel *c, *last;

    _v3_lock_channellist();
    _v3_func_enter("_v3_remove_channel");
    last = v3_channel_list;
    for (c = v3_channel_list; c != NULL; c = c->next) {
        if (c->id == id) {
            last->next = c->next;
            _v3_debug(V3_DEBUG_INFO, "removed channel %s",  c->name);
            v3_free_channel(c);
            _v3_func_leave("_v3_remove_channel");
            _v3_unlock_channellist();
            return true;
        }
        last = c;
    }
    _v3_func_leave("_v3_remove_channel");
    _v3_unlock_channellist();
    return false;
}/*}}}*/

int
_v3_remove_rank(uint16_t id) {/*{{{*/
    v3_rank *r, *last;

    _v3_lock_ranklist();
    _v3_func_enter("_v3_remove_rank");
    last = v3_rank_list;
    for (r = v3_rank_list; r != NULL; r = r->next) {
        if (r->id == id) {
            last->next = r->next;
            _v3_debug(V3_DEBUG_INFO, "removed rank %s",  r->name);
            v3_free_rank(r);
            _v3_func_leave("_v3_remove_rank");
            _v3_unlock_ranklist();
            return true;
        }
        last = r;
    }
    _v3_func_leave("_v3_remove_rank");
    _v3_unlock_ranklist();
    return false;
}/*}}}*/

void
_v3_print_user_list(void) {/*{{{*/
    v3_user *u;
    int ctr=0;

    for (u = v3_user_list; u != NULL; u = u->next) {
        _v3_debug(V3_DEBUG_INFO, "=====[ user %d ]====================================================================", ctr++);
        _v3_debug(V3_DEBUG_INFO, "user id      : %d", u->id);
        _v3_debug(V3_DEBUG_INFO, "user name    : %s", u->name);
        _v3_debug(V3_DEBUG_INFO, "user comment : %s", u->comment);
        _v3_debug(V3_DEBUG_INFO, "user phonetic: %s", u->phonetic);
        _v3_debug(V3_DEBUG_INFO, "user integration_text: %s", u->integration_text);
        _v3_debug(V3_DEBUG_INFO, "user url: %s", u->url);
    }
}/*}}}*/

void
_v3_destroy_userlist(void) {/*{{{*/
    v3_user *u, *next;

    _v3_func_enter("_v3_destroy_userlist");
    u = v3_user_list;
    while (u != NULL) {
        free(u->name);
        free(u->comment);
        free(u->phonetic);
        free(u->integration_text);
        free(u->url);
        next = u->next;
        free(u);
        u = next;
    }
    v3_user_list = NULL;
    _v3_func_leave("_v3_destroy_userlist");
}/*}}}*/

void
_v3_copy_channel(v3_channel *dest, v3_channel *src) {/*{{{*/
    memcpy(dest, src, sizeof(v3_channel));
    dest->name              = strdup(src->name);
    dest->phonetic          = strdup(src->phonetic);
    dest->comment           = strdup(src->comment);
    dest->next              = NULL;
}/*}}}*/

void
_v3_copy_user(v3_user *dest, v3_user *src) {/*{{{*/
    memcpy(dest, src, sizeof(v3_user));
    dest->name              = strdup(src->name);
    dest->phonetic          = strdup(src->phonetic);
    dest->comment           = strdup(src->comment);
    dest->integration_text  = strdup(src->integration_text);
    dest->url               = strdup(src->url);
    dest->next              = NULL;
}/*}}}*/

/*
 * Wrapper functions to create locks (mutexes) on global lists and variables
 */
void
_v3_lock_userlist(void) {/*{{{*/
    // TODO: PTHREAD: check if threads are enabled, possibly use semaphores as a compile time option
    if (userlist_mutex == NULL) {
        pthread_mutexattr_t mta;
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);

        _v3_debug(V3_DEBUG_MUTEX, "initializing userlist mutex");
        userlist_mutex = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(userlist_mutex, &mta);
    }
    _v3_debug(V3_DEBUG_MUTEX, "locking userlist");
    pthread_mutex_lock(userlist_mutex);
}/*}}}*/

void
_v3_unlock_userlist(void) {/*{{{*/
    // TODO: PTHREAD: check if threads are enabled, possibly use semaphores as a compile time option
    if (userlist_mutex == NULL) {
        pthread_mutexattr_t mta;
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);

        _v3_debug(V3_DEBUG_MUTEX, "initializing userlist mutex");
        userlist_mutex = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(userlist_mutex, &mta);
    }
    _v3_debug(V3_DEBUG_MUTEX, "unlocking userlist");
    pthread_mutex_unlock(userlist_mutex);
}/*}}}*/

void
_v3_lock_channellist(void) {/*{{{*/
    // TODO: PTHREAD: check if threads are enabled, possibly use semaphores as a compile time option
    if (channellist_mutex == NULL) {
        pthread_mutexattr_t mta;
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);

        _v3_debug(V3_DEBUG_MUTEX, "initializing channellist mutex");
        channellist_mutex = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(channellist_mutex, &mta);
    }
    _v3_debug(V3_DEBUG_MUTEX, "locking channellist");
    pthread_mutex_lock(channellist_mutex);
}/*}}}*/

void
_v3_unlock_channellist(void) {/*{{{*/
    // TODO: PTHREAD: check if threads are enabled, possibly use semaphores as a compile time option
    if (channellist_mutex == NULL) {
        pthread_mutexattr_t mta;
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);

        _v3_debug(V3_DEBUG_MUTEX, "initializing channellist mutex");
        channellist_mutex = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(channellist_mutex, &mta);
    }
    _v3_debug(V3_DEBUG_MUTEX, "unlocking channellist");
    pthread_mutex_unlock(channellist_mutex);
}/*}}}*/

void
_v3_lock_ranklist(void) {/*{{{*/
    // TODO: PTHREAD: check if threads are enabled, possibly use semaphores as a compile time option
    if (ranklist_mutex == NULL) {
        pthread_mutexattr_t mta;
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);

        _v3_debug(V3_DEBUG_MUTEX, "initializing ranklist mutex");
        ranklist_mutex = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(ranklist_mutex, &mta);
    }
    _v3_debug(V3_DEBUG_MUTEX, "locking ranklist");
    pthread_mutex_lock(ranklist_mutex);
    pthread_mutex_lock(ranklist_mutex);
}/*}}}*/

void
_v3_unlock_ranklist(void) {/*{{{*/
    // TODO: PTHREAD: check if threads are enabled, possibly use semaphores as a compile time option
    if (ranklist_mutex == NULL) {
        pthread_mutexattr_t mta;
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);

        _v3_debug(V3_DEBUG_MUTEX, "initializing ranklist mutex");
        ranklist_mutex = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(ranklist_mutex, &mta);
    }
    _v3_debug(V3_DEBUG_MUTEX, "unlocking ranklist");
    pthread_mutex_unlock(ranklist_mutex);
}/*}}}*/

void
_v3_lock_sendq(void) {/*{{{*/
    // TODO: PTHREAD: check if threads are enabled, possibly use semaphores as a compile time option
    if (sendq_mutex == NULL) {
        pthread_mutexattr_t mta;
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);

        _v3_debug(V3_DEBUG_MUTEX, "initializing sendq mutex");
        sendq_mutex = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(sendq_mutex, &mta);
    }
    _v3_debug(V3_DEBUG_MUTEX, "locking sendq");
    pthread_mutex_lock(sendq_mutex);
}/*}}}*/

void
_v3_unlock_sendq(void) {/*{{{*/
    // TODO: PTHREAD: check if threads are enabled, possibly use semaphores as a compile time option
    if (sendq_mutex == NULL) {
        pthread_mutexattr_t mta;
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);

        _v3_debug(V3_DEBUG_MUTEX, "initializing sendq mutex");
        sendq_mutex = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(sendq_mutex, &mta);
    }
    _v3_debug(V3_DEBUG_MUTEX, "unlocking sendq");
    pthread_mutex_unlock(sendq_mutex);
}/*}}}*/

void
_v3_lock_luser(void) {/*{{{*/
    // TODO: PTHREAD: check if threads are enabled, possibly use semaphores as a compile time option
    if (luser_mutex == NULL) {
        pthread_mutexattr_t mta;
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);

        _v3_debug(V3_DEBUG_MUTEX, "initializing luser mutex");
        luser_mutex = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(luser_mutex, &mta);
    }
    _v3_debug(V3_DEBUG_MUTEX, "locking luser");
    pthread_mutex_lock(luser_mutex);
}/*}}}*/

void
_v3_unlock_luser(void) {/*{{{*/
    // TODO: PTHREAD: check if threads are enabled, possibly use semaphores as a compile time option
    if (luser_mutex == NULL) {
        pthread_mutexattr_t mta;
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);

        _v3_debug(V3_DEBUG_MUTEX, "initializing luser mutex");
        luser_mutex = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(luser_mutex, &mta);
    }
    _v3_debug(V3_DEBUG_MUTEX, "unlocking luser");
    pthread_mutex_unlock(luser_mutex);
}/*}}}*/

void
_v3_lock_server(void) {/*{{{*/
    // TODO: PTHREAD: check if threads are enabled, possibly use semaphores as a compile time option
    if (server_mutex == NULL) {
        pthread_mutexattr_t mta;
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);

        _v3_debug(V3_DEBUG_MUTEX, "initializing server mutex");
        server_mutex = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(server_mutex, &mta);
    }
    _v3_debug(V3_DEBUG_MUTEX, "locking server");
    pthread_mutex_lock(server_mutex);
}/*}}}*/

void
_v3_unlock_server(void) {/*{{{*/
    // TODO: PTHREAD: check if threads are enabled, possibly use semaphores as a compile time option
    if (server_mutex == NULL) {
        pthread_mutexattr_t mta;
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);

        _v3_debug(V3_DEBUG_MUTEX, "initializing server mutex");
        server_mutex = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(server_mutex, &mta);
    }
    _v3_debug(V3_DEBUG_MUTEX, "unlocking server");
    pthread_mutex_unlock(server_mutex);
}/*}}}*/

void
_v3_init_decoders(void) {/*{{{*/
    _v3_func_enter("_v3_init_decoders");

    memset(v3_decoders, 0, sizeof(v3_decoders));

    _v3_func_leave("_v3_init_decoders");
}/*}}}*/

void
_v3_destroy_decoder(_v3_decoders *decoder) {/*{{{*/
    //_v3_func_enter("_v3_destroy_decoder");

#if HAVE_GSM
    if (decoder->gsm) {
        gsm_destroy(decoder->gsm);
        decoder->gsm = NULL;
    }
#endif
#if HAVE_CELT
    if (decoder->celt) {
        celt_decoder_destroy(decoder->celt);
        decoder->celt = NULL;
    }
    if (decoder->celtmode) {
        celt_mode_destroy(decoder->celtmode);
        decoder->celtmode = NULL;
    }
    decoder->celtchans = 0;
#endif
#if HAVE_SPEEX
    if (decoder->speex) {
        speex_decoder_destroy(decoder->speex);
        decoder->speex = NULL;
    }
    decoder->speexrate = 0;
#endif

    //_v3_func_leave("_v3_destroy_decoder");
}/*}}}*/

void
_v3_destroy_decoders(void) {/*{{{*/
    _v3_func_enter("_v3_destroy_decoders");

    int ctr;
    for (ctr = 0; ctr < 65535; ctr++) {
        _v3_destroy_decoder(&v3_decoders[ctr]);
    }

    _v3_func_leave("_v3_destroy_decoders");
}/*}}}*/

uint8_t
_v3_parse_filter(v3_sp_filter *f, char *value) {/*{{{*/
    char *a, *i, *t, *tmp;

    _v3_func_enter("_v3_parse_filter");
    a = value;
    i = strchr(a, ',') + 1;
    // make sure strchr didn't return null (in which case it would be 1 since
    // we added 1)
    if (i == (void *)1) {
        _v3_func_leave("_v3_parse_filter");
        return 0;
    }
    tmp = i - 1;
    *tmp = 0;
    t = strchr(i, ',') + 1;
    if (t == (void *)1) {
        _v3_func_leave("_v3_parse_filter");
        return 0;
    }
    tmp = t - 1;
    *tmp = 0;
    f->action = atoi(a);
    f->interval = atoi(i);
    f->times = atoi(t);
    _v3_debug(V3_DEBUG_INFO, "parsed filter: %d, %d, %d\n", f->action, f->interval, f->times);
    _v3_func_leave("_v3_parse_filter");
    return 1;
}/*}}}*/

uint8_t *
_v3_audio_encode(
        /* pcm input */
        uint8_t *sample,
        uint32_t pcmlen,
        /* encoded output */
        const v3_codec *codec,
        uint16_t *datalen,
        /* optional args */
        uint8_t channels,
        uint16_t *framecount,
        uint8_t *celtfragsize) {/*{{{*/
    _v3_func_enter("_v3_audio_encode");

    if (!sample || !pcmlen || !codec || !datalen) {
        _v3_debug(V3_DEBUG_INFO, "argument missing for _v3_audio_encode");
        _v3_func_leave("_v3_audio_encode");
        return NULL;
    }
    int ctr;

    channels = (channels == 2) ? 2 : 1;
    switch (codec->codec) {
#if HAVE_GSM
      case 0x00: // GSM
      {
        static void *gsmenc  = NULL;
        uint16_t frame_count = pcmlen / 640;
        uint16_t gsmdatabuf  = frame_count * 65;
        uint8_t *gsmdata     = NULL;

        _v3_debug(V3_DEBUG_INFO, "encoding %d bytes of PCM to GSM @ %lu", pcmlen, codec->rate);
        if (channels > 1) {
            _v3_debug(V3_DEBUG_INFO, "mono only supported for gsm");
            break;
        }
        if (!gsmenc) {
            if (!(gsmenc = gsm_create())) {
                _v3_debug(V3_DEBUG_INFO, "failed to create gsm encoder");
                break;
            }
            int one = 1;
            gsm_option(gsmenc, GSM_OPT_WAV49, &one);
        }
        _v3_debug(V3_DEBUG_MEMORY, "allocating %lu bytes for %d gsm frames", gsmdatabuf, frame_count);
        gsmdata = malloc(gsmdatabuf);
        memset(gsmdata, 0, gsmdatabuf);
        for (ctr = 0; ctr < frame_count; ctr++) {
            _v3_debug(V3_DEBUG_INFO, "encoding gsm frame %d", ctr+1);
            gsm_encode(gsmenc, (void *)sample+(ctr*640), gsmdata+(ctr*65));
            gsm_encode(gsmenc, (void *)sample+(ctr*640)+320, gsmdata+(ctr*65)+32);
        }
        //gsm_destroy(gsmenc);
        //gsmenc = NULL;
        if (framecount) {
            *framecount = frame_count;
        }
        *datalen = gsmdatabuf;
        _v3_func_leave("_v3_audio_encode");
        return gsmdata;
      }
#endif
#if HAVE_CELT
      case 0x01: // CELT
      case 0x02:
      {
        static void *celtmode    = NULL;
        static void *celtenc     = NULL;
        static uint8_t last_chan = 0;
        const int prediction     = 2;
        const int complexity     = 10;
        uint16_t pcm_frame_size  = codec->pcmframesize * channels;
        uint16_t frame_count     = pcmlen / pcm_frame_size;
        uint8_t celt_frag_size   = (celtfragsize && *celtfragsize) ? *celtfragsize : ((codec->codec == 0x01) ? 60 : 54);
        uint8_t celt_frame_size  = celt_frag_size - 1;
        uint16_t celtdatabuf     = frame_count * celt_frag_size;
        uint8_t *celtdata        = NULL;
        uint8_t *celtdataptr     = NULL;

        _v3_debug(V3_DEBUG_INFO, "encoding %d bytes of PCM to CELT @ %lu", pcmlen, codec->rate);
        if (!celtmode || !celtenc || channels != last_chan) {
            if (celtenc) {
                celt_encoder_destroy(celtenc);
                celtenc = NULL;
            }
            if (celtmode) {
                celt_mode_destroy(celtmode);
                celtmode = NULL;
            }
            if (!(celtmode = celt_mode_create(44100, codec->pcmframesize / sizeof(int16_t), NULL)) ||
                !(celtenc = celt_encoder_create(celtmode, channels, NULL))) {
                _v3_debug(V3_DEBUG_INFO, "failed to create celt encoder");
                celtmode = NULL;
                celtenc = NULL;
                break;
            }
            if (celt_encoder_ctl(celtenc, CELT_SET_PREDICTION(prediction)) != CELT_OK) {
                _v3_debug(V3_DEBUG_INFO, "celt_encoder_ctl: prediction request failed");
                celt_encoder_destroy(celtenc);
                celt_mode_destroy(celtmode);
                celtmode = NULL;
                celtenc = NULL;
                break;
            }
            if (celt_encoder_ctl(celtenc, CELT_SET_COMPLEXITY(complexity)) != CELT_OK) {
                _v3_debug(V3_DEBUG_INFO, "celt_encoder_ctl: complexity 0 through 10 is only supported");
                celt_encoder_destroy(celtenc);
                celt_mode_destroy(celtmode);
                celtmode = NULL;
                celtenc = NULL;
                break;
            }
            last_chan = channels;
        }
        _v3_debug(V3_DEBUG_MEMORY, "allocating %lu bytes for %d celt frames", celtdatabuf, frame_count);
        celtdata = malloc(celtdatabuf);
        memset(celtdata, 0, celtdatabuf);
        celtdataptr = celtdata;
        for (ctr = 0; ctr < frame_count; ctr++) {
            _v3_debug(V3_DEBUG_INFO, "encoding celt frame %d", ctr+1);
            *celtdataptr++ = celt_frame_size;
            if (celt_encode(celtenc, (void *)sample+(ctr*pcm_frame_size), NULL, (void *)celtdataptr, celt_frame_size) < 0) {
                _v3_debug(V3_DEBUG_INFO, "failed to encode celt frame %d", ctr+1);
            }
            celtdataptr += celt_frame_size;
        }
        //celt_encoder_destroy(celtenc);
        //celt_mode_destroy(celtmode);
        //celtmode = NULL;
        //celtenc = NULL;
        if (framecount) {
            *framecount = frame_count;
        }
        if (celtfragsize) {
            *celtfragsize = celt_frag_size;
        }
        *datalen = celtdatabuf;
        _v3_func_leave("_v3_audio_encode");
        return celtdata;
      }
#endif
#if HAVE_SPEEX
      case 0x03: // SPEEX
      {
        static void *spxenc      = NULL;
        static uint32_t rate     = 0;
        static uint8_t format    = -1;
        int quality              = -1;
        uint16_t frame_count     = pcmlen / codec->pcmframesize;
        uint16_t pcm_frame_size  = codec->pcmframesize / sizeof(int16_t);
        uint16_t spx_frame_size;
        const uint16_t spxmaxbuf = 200;
        uint16_t spxdatabuf      = (frame_count + 1) * spxmaxbuf;
        uint8_t *spxdata         = NULL;
        uint16_t spxdatalen      = 0;
        uint16_t *spxhead        = NULL;
        SpeexBits bits;

        _v3_debug(V3_DEBUG_INFO, "encoding %d bytes of PCM to SPEEX @ %lu", pcmlen, codec->rate);
        if (channels > 1) {
            _v3_debug(V3_DEBUG_INFO, "mono only supported for speex");
            break;
        }
        if (!spxenc || codec->rate != rate || codec->format != format) {
            if (spxenc) {
                speex_encoder_destroy(spxenc);
                spxenc = NULL;
            }
            switch (codec->rate) {
              case 8000:
                _v3_debug(V3_DEBUG_INFO, "using narrow band mode");
                spxenc = speex_encoder_init(&speex_nb_mode);
                break;
              case 16000:
                _v3_debug(V3_DEBUG_INFO, "using wide band mode");
                spxenc = speex_encoder_init(&speex_wb_mode);
                break;
              case 32000:
                _v3_debug(V3_DEBUG_INFO, "using ultra-wide band mode");
                spxenc = speex_encoder_init(&speex_uwb_mode);
                break;
            }
            if (!spxenc) {
                _v3_debug(V3_DEBUG_INFO, "failed to create speex encoder");
                break;
            }
            rate = codec->rate;
            format = codec->format;
            quality = codec->quality;
            speex_encoder_ctl(spxenc, SPEEX_SET_QUALITY, &quality);
        }
        _v3_debug(V3_DEBUG_MEMORY, "allocating %lu bytes of data buffer for %d speex frames", spxdatabuf, frame_count);
        spxdata = malloc(spxdatabuf);
        memset(spxdata, 0, spxdatabuf);
        spxhead = (void *)spxdata;
        *spxhead++ = htons(frame_count);
        *spxhead++ = htons(pcm_frame_size);
        spxdatalen = (void *)spxhead - (void *)spxdata;
        speex_bits_init(&bits);
        for (ctr = 0; ctr < frame_count; ctr++) {
            speex_bits_reset(&bits);
            _v3_debug(V3_DEBUG_INFO, "encoding speex frame %d", ctr+1);
            speex_encode_int(spxenc, (void *)sample+(ctr*codec->pcmframesize), &bits);
            spx_frame_size = speex_bits_write(&bits, (void *)spxdata+(spxdatalen+sizeof(uint16_t)), spxmaxbuf);
            *((uint16_t *)(spxdata + spxdatalen)) = htons(spx_frame_size);
            spxdatalen += sizeof(uint16_t) + spx_frame_size;
        }
        speex_bits_destroy(&bits);
        _v3_debug(V3_DEBUG_MEMORY, "used %lu out of %lu bytes for %d speex frames", spxdatalen, spxdatabuf, frame_count);
        //speex_encoder_destroy(spxenc);
        //spxenc = NULL;
        if (framecount) {
            *framecount = frame_count;
        }
        *datalen = spxdatalen;
        _v3_func_leave("_v3_audio_encode");
        return spxdata;
      }
#endif
      default:
        _v3_debug(V3_DEBUG_INFO, "unsupported codec %d/%d", codec->codec, codec->format);
        break;
    }

    _v3_func_leave("_v3_audio_encode");
    return NULL;
}/*}}}*/

int
_v3_audio_decode(
        /* encoded input */
        const v3_codec *codec,
        _v3_decoders *decoder,
        uint8_t *data,
        uint16_t datalen,
        /* pcm output */
        uint8_t *sample,
        uint32_t *pcmlen,
        /* optional args */
        uint8_t channels) {/*{{{*/
    _v3_func_enter("_v3_audio_decode");

    if (!codec || !decoder || !data || !datalen || !sample || !pcmlen || (pcmlen && !*pcmlen)) {
        _v3_debug(V3_DEBUG_INFO, "argument missing for _v3_audio_decode");
        _v3_func_leave("_v3_audio_decode");
        return V3_FAILURE;
    }
    uint32_t pcmmaxlen = *pcmlen;
    int ctr;

    *pcmlen = 0;
    channels = (channels == 2) ? 2 : 1;
    switch (codec->codec) {
#if HAVE_GSM
      case 0x00: // GSM
      {
        if (!decoder->gsm) {
            if (!(decoder->gsm = gsm_create())) {
                _v3_debug(V3_DEBUG_INFO, "failed to create gsm decoder");
                break;
            }
            int one = 1;
            gsm_option(decoder->gsm, GSM_OPT_WAV49, &one);
        }
        for (ctr = 0; ctr < datalen / 65; ctr++) {
            if (gsm_decode(decoder->gsm, data+(ctr*65), (void *)sample+(ctr*640)) ||
                gsm_decode(decoder->gsm, data+(ctr*65)+33, (void *)sample+(ctr*640)+320)) {
                _v3_debug(V3_DEBUG_INFO, "failed to decode gsm frame %d", ctr+1);
            }
            *pcmlen += 640;
        }
        _v3_func_leave("_v3_audio_decode");
        return V3_OK;
      }
#endif
#if HAVE_CELT
      case 0x01: // CELT
      case 0x02:
      {
        uint16_t pcm_frame_size  = codec->pcmframesize * channels;
        uint8_t celt_frame_size;
        uint8_t *celtdataptr     = data;
        uint16_t celtdatalen     = datalen;

        if (!decoder->celtmode || !decoder->celt || channels != decoder->celtchans) {
            if (decoder->celt) {
                celt_decoder_destroy(decoder->celt);
                decoder->celt = NULL;
            }
            if (decoder->celtmode) {
                celt_mode_destroy(decoder->celtmode);
                decoder->celtmode = NULL;
            }
            if (!(decoder->celtmode = celt_mode_create(44100, codec->pcmframesize / sizeof(int16_t), NULL)) ||
                !(decoder->celt = celt_decoder_create(decoder->celtmode, channels, NULL))) {
                _v3_debug(V3_DEBUG_INFO, "failed to create celt decoder");
                decoder->celtmode = NULL;
                decoder->celt = NULL;
                break;
            }
            decoder->celtchans = channels;
        }
        while (celtdatalen) {
            celt_frame_size = *celtdataptr;
            celtdataptr++;
            if (!celt_frame_size || celtdatalen - celt_frame_size - 1 < 0 || *pcmlen + pcm_frame_size > pcmmaxlen) {
                _v3_debug(V3_DEBUG_INFO, "received a malformed celt packet");
                _v3_func_leave("_v3_audio_decode");
                return V3_MALFORMED;
            }
            celtdatalen--;
            if (celt_decode(decoder->celt, (void *)celtdataptr, celt_frame_size, (void *)sample + *pcmlen)) {
                _v3_debug(V3_DEBUG_INFO, "failed to decode celt frame");
            }
            celtdataptr += celt_frame_size;
            celtdatalen -= celt_frame_size;
            *pcmlen += pcm_frame_size;
        }
        _v3_func_leave("_v3_audio_decode");
        return V3_OK;
      }
#endif
#if HAVE_SPEEX
      case 0x03: // SPEEX
      {
        uint16_t pcm_frame_size  = codec->pcmframesize;
        uint16_t spx_frame_size;
        uint8_t *spxdataptr      = data;
        uint16_t spxdatalen      = datalen;
        SpeexBits bits;

        if (!decoder->speex || codec->rate != decoder->speexrate) {
            if (decoder->speex) {
                speex_decoder_destroy(decoder->speex);
                decoder->speex = NULL;
            }
            switch (codec->rate) {
              case 8000:
                _v3_debug(V3_DEBUG_INFO, "using narrow band mode");
                decoder->speex = speex_decoder_init(&speex_nb_mode);
                break;
              case 16000:
                _v3_debug(V3_DEBUG_INFO, "using wide band mode");
                decoder->speex = speex_decoder_init(&speex_wb_mode);
                break;
              case 32000:
                _v3_debug(V3_DEBUG_INFO, "using ultra-wide band mode");
                decoder->speex = speex_decoder_init(&speex_uwb_mode);
                break;
            }
            if (!decoder->speex) {
                _v3_debug(V3_DEBUG_INFO, "received unknown pcm rate for speex %d", codec->rate);
                break;
            }
            decoder->speexrate = codec->rate;
        }
        spxdataptr += sizeof(uint16_t) * 2;
        if (spxdatalen - sizeof(uint16_t) * 2 < 0) {
            _v3_debug(V3_DEBUG_INFO, "received a malformed speex packet");
            _v3_func_leave("_v3_audio_decode");
            return V3_MALFORMED;
        }
        spxdatalen -= sizeof(uint16_t) * 2;
        speex_bits_init(&bits);
        while (spxdatalen) {
            spx_frame_size = ntohs(*(uint16_t *)spxdataptr);
            spxdataptr += sizeof(uint16_t);
            if (!spx_frame_size || spxdatalen - spx_frame_size - sizeof(uint16_t) < 0 || *pcmlen + pcm_frame_size > pcmmaxlen) {
                _v3_debug(V3_DEBUG_INFO, "received a malformed speex packet");
                _v3_func_leave("_v3_audio_decode");
                return V3_MALFORMED;
            }
            spxdatalen -= sizeof(uint16_t);
            speex_bits_read_from(&bits, (void *)spxdataptr, spx_frame_size);
            speex_decode_int(decoder->speex, &bits, (void *)sample + *pcmlen);
            spxdataptr += spx_frame_size;
            spxdatalen -= spx_frame_size;
            *pcmlen += pcm_frame_size;
        }
        speex_bits_destroy(&bits);
        _v3_func_leave("_v3_audio_decode");
        return V3_OK;
      }
#endif
      default:
        _v3_debug(V3_DEBUG_INFO, "unsupported codec %d/%d", codec->codec, codec->format);
        break;
    }

    _v3_func_leave("_v3_audio_decode");
    return V3_FAILURE;
}/*}}}*/

/*
 * Recording API
 */
void *
v3_vrf_init(const char *filename) {/*{{{*/
    _v3_func_enter("v3_vrf_init");

    v3_vrf *vrfh = malloc(sizeof(v3_vrf));
    memset(vrfh, 0, sizeof(v3_vrf));
    _v3_debug(V3_DEBUG_MUTEX, "initializing vrf mutex");
    pthread_mutexattr_t mta;
    pthread_mutexattr_init(&mta);
    pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutex_init(&vrfh->mutex, &mta);
    if (filename && strlen(filename)) {
        if (!(vrfh->file = fopen(filename, "rb+"))) {
            _v3_error("%s: %s", filename, strerror(errno));
            free(vrfh);
            _v3_func_leave("v3_vrf_init");
            return NULL;
        }
        vrfh->filename = strdup(filename);
        _v3_debug(V3_DEBUG_INFO, "opened file for reading: %s", vrfh->filename);
        fseek(vrfh->file, 0, SEEK_END);
        vrfh->filelen = ftell(vrfh->file);
        rewind(vrfh->file);
        _v3_debug(V3_DEBUG_INFO, "file size: %u", vrfh->filelen);
        if (!vrfh->filelen) {
            _v3_error("%s: file is empty", filename);
            v3_vrf_destroy(vrfh);
            _v3_func_leave("v3_vrf_init");
            return NULL;
        }
        if (_v3_vrf_get_header(vrfh) != V3_OK) {
            v3_vrf_destroy(vrfh);
            _v3_func_leave("v3_vrf_init");
            return NULL;
        }
        if (!strncmp(vrfh->header.headid, V3_VRF_TEMPID, sizeof(vrfh->header.headid)) &&
            _v3_vrf_recover(vrfh) != V3_OK) {
            _v3_error("%s: failed to recover vrf segment table", filename);
            v3_vrf_destroy(vrfh);
            _v3_func_leave("v3_vrf_init");
            return NULL;
        }
    }

    _v3_func_leave("v3_vrf_init");
    return vrfh;
}/*}}}*/

void
v3_vrf_destroy(void *vrfh) {/*{{{*/
    _v3_func_enter("v3_vrf_destroy");

    v3_vrf *_vrfh = vrfh;
    if (!vrfh) {
        _v3_func_leave("v3_vrf_destroy");
        return;
    }
    if (_vrfh->table) {
        free(_vrfh->table);
        _vrfh->table = NULL;
    }
    if (_vrfh->filename) {
        free(_vrfh->filename);
        _vrfh->filename = NULL;
    }
    if (_vrfh->file) {
        fclose(_vrfh->file);
        _vrfh->file = NULL;
    }
    pthread_mutex_destroy(&_vrfh->mutex);
    free(vrfh);
    vrfh = NULL;

    _v3_func_leave("v3_vrf_destroy");
}/*}}}*/

void
v3_vrf_data_init(v3_vrf_data *vrfd) {/*{{{*/
    _v3_func_enter("v3_vrf_data_init");

    if (!vrfd) {
        _v3_func_leave("v3_vrf_data_init");
        return;
    }
    memset(vrfd, 0, sizeof(v3_vrf_data));

    _v3_func_leave("v3_vrf_data_init");
}/*}}}*/

void
v3_vrf_data_destroy(v3_vrf_data *vrfd) {/*{{{*/
    _v3_func_enter("v3_vrf_data_destroy");

    if (!vrfd) {
        _v3_func_leave("v3_vrf_data_destroy");
        return;
    }
    if (vrfd->text) {
        free(vrfd->text);
        vrfd->text = NULL;
    }
    if (vrfd->data) {
        free(vrfd->data);
        vrfd->data = NULL;
    }
    if (vrfd->_decoder) {
        _v3_destroy_decoder(vrfd->_decoder);
        free(vrfd->_decoder);
        vrfd->_decoder = NULL;
    }
    if (vrfd->_audio) {
        free(vrfd->_audio);
        vrfd->_audio = NULL;
    }
    memset(vrfd, 0, sizeof(v3_vrf_data));

    _v3_func_leave("v3_vrf_data_destroy");
}/*}}}*/

void
_v3_vrf_lock(v3_vrf *vrfh) {/*{{{*/
    _v3_func_enter("_v3_vrf_lock");

    _v3_debug(V3_DEBUG_MUTEX, "locking vrf");
    pthread_mutex_lock(&vrfh->mutex);

    _v3_func_leave("_v3_vrf_lock");
}/*}}}*/

void
_v3_vrf_unlock(v3_vrf *vrfh) {/*{{{*/
    _v3_func_enter("_v3_vrf_unlock");

    _v3_debug(V3_DEBUG_MUTEX, "unlocking vrf");
    pthread_mutex_unlock(&vrfh->mutex);

    _v3_func_leave("_v3_vrf_unlock");
}/*}}}*/

void
_v3_vrf_print_header(const v3_vrf_header *header) {/*{{{*/
    _v3_func_enter("_v3_vrf_print_header");

    _v3_debug(V3_DEBUG_INFO, "headid.....: %.*s", sizeof(header->headid), header->headid);
    _v3_debug(V3_DEBUG_INFO, "size.......: %u", header->size);
    _v3_debug(V3_DEBUG_INFO, "headlen....: %u", header->headlen);
    _v3_debug(V3_DEBUG_INFO, "unknown1...: %u", header->unknown1);
    _v3_debug(V3_DEBUG_INFO, "segtable...: 0x%08x", header->segtable);
    _v3_debug(V3_DEBUG_INFO, "segcount...: %u", header->segcount);
    _v3_debug(V3_DEBUG_INFO, "vrfversion.: 0x%02x", header->vrfversion);
    _v3_debug(V3_DEBUG_INFO, "unknown2...: %u", header->unknown2);
    _v3_debug(V3_DEBUG_INFO, "unknown3...: %u", header->unknown3);
    _v3_debug(V3_DEBUG_INFO, "infolen....: %u", header->infolen);
    _v3_debug(V3_DEBUG_INFO, "codec......: 0x%02x", header->codec);
    _v3_debug(V3_DEBUG_INFO, "codecformat: 0x%02x", header->codecformat);
    _v3_debug(V3_DEBUG_INFO, "unknown4...: %u", header->unknown4);
    _v3_debug(V3_DEBUG_INFO, "platform...: %.*s", sizeof(header->platform),  header->platform);
    _v3_debug(V3_DEBUG_INFO, "version....: %.*s", sizeof(header->version),   header->version);
    _v3_debug(V3_DEBUG_INFO, "username...: %.*s", sizeof(header->username),  header->username);
    _v3_debug(V3_DEBUG_INFO, "comment....: %.*s", sizeof(header->comment),   header->comment);
    _v3_debug(V3_DEBUG_INFO, "url........: %.*s", sizeof(header->url),       header->url);
    _v3_debug(V3_DEBUG_INFO, "copyright..: %.*s", sizeof(header->copyright), header->copyright);

    _v3_func_leave("_v3_vrf_print_header");
}/*}}}*/

void
_v3_vrf_print_info(const v3_vrf_header *header) {/*{{{*/
    _v3_func_enter("_v3_vrf_print_info");

    _v3_debug(V3_DEBUG_INFO, "size.......: %u", header->size);
    _v3_debug(V3_DEBUG_INFO, "codec......: 0x%02x", header->codec);
    _v3_debug(V3_DEBUG_INFO, "codecformat: 0x%02x", header->codecformat);
    _v3_debug(V3_DEBUG_INFO, "platform...: %.*s", sizeof(header->platform),  header->platform);
    _v3_debug(V3_DEBUG_INFO, "version....: %.*s", sizeof(header->version),   header->version);
    _v3_debug(V3_DEBUG_INFO, "username...: %.*s", sizeof(header->username),  header->username);
    _v3_debug(V3_DEBUG_INFO, "comment....: %.*s", sizeof(header->comment),   header->comment);
    _v3_debug(V3_DEBUG_INFO, "url........: %.*s", sizeof(header->url),       header->url);
    _v3_debug(V3_DEBUG_INFO, "copyright..: %.*s", sizeof(header->copyright), header->copyright);

    _v3_func_leave("_v3_vrf_print_info");
}/*}}}*/

void
_v3_vrf_print_segment(uint32_t id, const v3_vrf_segment *segment) {/*{{{*/
    _v3_func_enter("_v3_vrf_print_segment");

    _v3_debug(V3_DEBUG_INFO, "--- segment %i ---", id);
    _v3_debug(V3_DEBUG_INFO, "headlen.: %u", segment->headlen);
    _v3_debug(V3_DEBUG_INFO, "type....: 0x%02x", segment->type);
    _v3_debug(V3_DEBUG_INFO, "valid...: %u", segment->valid);
    _v3_debug(V3_DEBUG_INFO, "offset..: 0x%08x", segment->offset);
    _v3_debug(V3_DEBUG_INFO, "time....: %u", segment->time);
    _v3_debug(V3_DEBUG_INFO, "duration: %u", segment->duration);
    _v3_debug(V3_DEBUG_INFO, "unknown1: %u", segment->unknown1);
    _v3_debug(V3_DEBUG_INFO, "unknown2: %u", segment->unknown2);
    _v3_debug(V3_DEBUG_INFO, "username: %.*s", sizeof(segment->username), segment->username);

    _v3_func_leave("_v3_vrf_print_segment");
}/*}}}*/

void
_v3_vrf_print_audio(const v3_vrf_audio *audio) {/*{{{*/
    _v3_func_enter("_v3_vrf_print_audio");

    _v3_debug(V3_DEBUG_INFO, "headlen..: %u", audio->headlen);
    _v3_debug(V3_DEBUG_INFO, "type.....: 0x%02x", audio->type);
    _v3_debug(V3_DEBUG_INFO, "unknown1.: %u", audio->unknown1);
    _v3_debug(V3_DEBUG_INFO, "index....: %u", audio->index);
    _v3_debug(V3_DEBUG_INFO, "fragcount: %u", audio->fragcount);
    _v3_debug(V3_DEBUG_INFO, "unknown2.: %u", audio->unknown2);
    _v3_debug(V3_DEBUG_INFO, "unknown3.: %u", audio->unknown3);
    _v3_debug(V3_DEBUG_INFO, "unknown4.: %u", audio->offset);

    _v3_func_leave("_v3_vrf_print_audio");
}/*}}}*/

void
_v3_vrf_print_fragment(uint32_t type, const v3_vrf_fragment *fragment) {/*{{{*/
    _v3_func_enter("_v3_vrf_print_fragment");

    _v3_debug(V3_DEBUG_INFO, "headlen....: %u", fragment->headlen);
    _v3_debug(V3_DEBUG_INFO, "fraglen....: %u", fragment->fraglen);
    if (type != V3_VRF_TYPE_TEXT) {
        _v3_debug(V3_DEBUG_INFO, "pcmlen.....: %u", fragment->audio.pcmlen);
        _v3_debug(V3_DEBUG_INFO, "unknown1...: %u", fragment->audio.unknown1);
    }
    if (type == V3_VRF_TYPE_EXT) {
        _v3_debug(V3_DEBUG_INFO, "codec......: 0x%02x", fragment->audio.ext.codec);
        _v3_debug(V3_DEBUG_INFO, "codecformat: 0x%02x", fragment->audio.ext.codecformat);
        _v3_debug(V3_DEBUG_INFO, "unknown2...: %u", fragment->audio.ext.unknown2);
    }

    _v3_func_leave("_v3_vrf_print_fragment");
}/*}}}*/

int
_v3_vrf_get_header(v3_vrf *vrfh) {/*{{{*/
    _v3_func_enter("_v3_vrf_get_header");

    if (!vrfh) {
        _v3_func_leave("_v3_vrf_get_header");
        return V3_FAILURE;
    }
    _v3_vrf_lock(vrfh);
    rewind(vrfh->file);
    if (!fread(&vrfh->header, sizeof(v3_vrf_header), 1, vrfh->file)) {
        _v3_error("%s: failed to get vrf header: %s", vrfh->filename, strerror(errno));
        _v3_vrf_unlock(vrfh);
        _v3_func_leave("_v3_vrf_get_header");
        return V3_FAILURE;
    }
    v3_vrf_header *header = &vrfh->header;
    header->size        = ntohl(header->size);
    header->headlen     = ntohl(header->headlen);
    header->unknown1    = ntohl(header->unknown1);
    header->segtable    = ntohl(header->segtable);
    header->segcount    = ntohl(header->segcount);
    header->vrfversion  = ntohl(header->vrfversion);
    header->unknown2    = ntohl(header->unknown2);
    header->unknown3    = ntohl(header->unknown3);
    header->infolen     = ntohl(header->infolen);
    header->codec       = ntohl(header->codec);
    header->codecformat = ntohl(header->codecformat);
    header->unknown4    = ntohl(header->unknown4);
    _v3_vrf_print_header(header);
    if (strncmp(header->headid, V3_VRF_HEADID, sizeof(header->headid)) &&
        strncmp(header->headid, V3_VRF_TEMPID, sizeof(header->headid))) {
        _v3_error("%s: unrecognized file header id", vrfh->filename);
        _v3_vrf_unlock(vrfh);
        _v3_func_leave("_v3_vrf_get_header");
        return V3_MALFORMED;
    }
    _v3_vrf_unlock(vrfh);

    _v3_func_leave("_v3_vrf_get_header");
    return V3_OK;
}/*}}}*/

uint32_t
v3_vrf_get_count(void *vrfh) {/*{{{*/
    _v3_func_enter("v3_vrf_get_count");

    if (!vrfh) {
        _v3_func_leave("v3_vrf_get_count");
        return 0;
    }
    v3_vrf *_vrfh = vrfh;
    _v3_debug(V3_DEBUG_INFO, "segcount: %u", _vrfh->header.segcount);

    _v3_func_leave("v3_vrf_get_count");
    return _vrfh->header.segcount;
}/*}}}*/

int
v3_vrf_get_info(void *vrfh, v3_vrf_data *vrfd) {/*{{{*/
    _v3_func_enter("v3_vrf_get_info");

    if (!vrfh) {
        _v3_func_leave("v3_vrf_get_info");
        return V3_FAILURE;
    }
    v3_vrf *_vrfh = vrfh;
    v3_vrf_header *header = &_vrfh->header;
    _v3_vrf_print_info(header);
    if (vrfd) {
        v3_vrf_data_init(vrfd);
        vrfd->size = header->size;
        vrfd->codec = header->codec;
        vrfd->codecformat = header->codecformat;
        strncpy(vrfd->platform,  header->platform,  sizeof(vrfd->platform) - 1);
        strncpy(vrfd->version,   header->version,   sizeof(vrfd->version) - 1);
        strncpy(vrfd->username,  header->username,  sizeof(vrfd->username) - 1);
        strncpy(vrfd->comment,   header->comment,   sizeof(vrfd->comment) - 1);
        strncpy(vrfd->url,       header->url,       sizeof(vrfd->url) - 1);
        strncpy(vrfd->copyright, header->copyright, sizeof(vrfd->copyright) - 1);
    }

    _v3_func_leave("v3_vrf_get_info");
    return V3_OK;
}/*}}}*/

int
_v3_vrf_get_table(v3_vrf *vrfh) {/*{{{*/
    _v3_func_enter("_v3_vrf_get_table");

    if (!vrfh) {
        _v3_func_leave("_v3_vrf_get_table");
        return V3_FAILURE;
    }
    _v3_vrf_lock(vrfh);
    if (vrfh->table) {
        vrfh->tablesize = 0;
        free(vrfh->table);
        vrfh->table = NULL;
    }
    vrfh->tablesize = sizeof(v3_vrf_segment) * vrfh->header.segcount;
    if (!vrfh->header.segtable || vrfh->tablesize + vrfh->header.segtable > vrfh->filelen) {
        _v3_error("%s: vrf is corrupted", vrfh->filename);
        _v3_vrf_unlock(vrfh);
        _v3_func_leave("_v3_vrf_get_table");
        return V3_MALFORMED;
    }
    vrfh->table = malloc(vrfh->tablesize);
    memset(vrfh->table, 0, vrfh->tablesize);
    fseek(vrfh->file, vrfh->header.segtable, SEEK_SET);
    if (!fread(vrfh->table, vrfh->tablesize, 1, vrfh->file)) {
        _v3_error("%s: failed to get vrf segment table: %s", vrfh->filename, strerror(errno));
        vrfh->tablesize = 0;
        free(vrfh->table);
        vrfh->table = NULL;
        _v3_vrf_unlock(vrfh);
        _v3_func_leave("_v3_vrf_get_table");
        return V3_FAILURE;
    }
    uint32_t ctr;
    for (ctr = 0; ctr < vrfh->header.segcount; ctr++) {
        v3_vrf_segment *segment = &vrfh->table[ctr];
        segment->headlen  = ntohl(segment->headlen);
        segment->type     = ntohl(segment->type);
        segment->valid    = ntohl(segment->valid);
        segment->offset   = ntohl(segment->offset);
        segment->time     = ntohl(segment->time);
        segment->duration = ntohl(segment->duration);
        segment->unknown1 = ntohl(segment->unknown1);
        segment->unknown2 = ntohl(segment->unknown2);
    }
    _v3_vrf_unlock(vrfh);

    _v3_func_leave("_v3_vrf_get_table");
    return V3_OK;
}/*}}}*/

int
_v3_vrf_check_table(v3_vrf *vrfh) {/*{{{*/
    _v3_func_enter("_v3_vrf_check_table");

    if (!vrfh) {
        _v3_func_leave("_v3_vrf_check_table");
        return V3_FAILURE;
    }
    if (!vrfh->table && _v3_vrf_get_table(vrfh) != V3_OK) {
        _v3_func_leave("_v3_vrf_check_table");
        return V3_FAILURE;
    }
    if (!vrfh->header.segcount || vrfh->header.segtable == vrfh->filelen) {
        _v3_error("%s: no segments", vrfh->filename);
        _v3_func_leave("_v3_vrf_check_table");
        return V3_FAILURE;
    }

    _v3_func_leave("_v3_vrf_check_table");
    return V3_OK;
}/*}}}*/

v3_vrf_segment *
_v3_vrf_get_segment(v3_vrf *vrfh, uint32_t id) {/*{{{*/
    _v3_func_enter("_v3_vrf_get_segment");

    if (!vrfh) {
        _v3_func_leave("_v3_vrf_get_segment");
        return NULL;
    }
    if (!vrfh->file || !vrfh->filename) {
        _v3_error("%p: no file opened", vrfh);
        _v3_func_leave("_v3_vrf_get_segment");
        return NULL;
    }
    if (id > vrfh->header.segcount) {
        _v3_error("%s: requested id greater than segment count", vrfh->filename);
        _v3_func_leave("_v3_vrf_get_segment");
        return NULL;
    }
    if (_v3_vrf_check_table(vrfh) != V3_OK) {
        _v3_func_leave("_v3_vrf_get_segment");
        return NULL;
    }
    v3_vrf *_vrfh = vrfh;
    v3_vrf_segment *segment = &_vrfh->table[id];
    _v3_vrf_print_segment(id, segment);

    _v3_func_leave("_v3_vrf_get_segment");
    return segment;
}/*}}}*/

int
v3_vrf_get_segment(void *vrfh, uint32_t id, v3_vrf_data *vrfd) {/*{{{*/
    _v3_func_enter("v3_vrf_get_segment");

    if (!vrfh) {
        _v3_func_leave("v3_vrf_get_segment");
        return V3_FAILURE;
    }
    v3_vrf_segment *segment;
    if (!(segment = _v3_vrf_get_segment(vrfh, id))) {
        _v3_func_leave("v3_vrf_get_segment");
        return V3_FAILURE;
    }
    if (vrfd) {
        v3_vrf_data_init(vrfd);
        vrfd->id = id;
        vrfd->time = segment->time;
        vrfd->duration = segment->duration;
        strncpy(vrfd->username, segment->username, sizeof(vrfd->username) - 1);
    }

    _v3_func_leave("v3_vrf_get_segment");
    return V3_OK;
}/*}}}*/

int
_v3_vrf_get_audio(v3_vrf *vrfh, uint32_t offset, v3_vrf_audio *audio) {/*{{{*/
    _v3_func_enter("_v3_vrf_get_audio");

    if (!vrfh || !audio) {
        _v3_func_leave("_v3_vrf_get_audio");
        return V3_FAILURE;
    }
    _v3_vrf_lock(vrfh);
    fseek(vrfh->file, offset, SEEK_SET);
    if (!fread(audio, sizeof(v3_vrf_audio), 1, vrfh->file)) {
        _v3_error("%s: failed to get vrf audio segment: %s", vrfh->filename, strerror(errno));
        _v3_vrf_unlock(vrfh);
        _v3_func_leave("_v3_vrf_get_audio");
        return V3_FAILURE;
    }
    audio->headlen   = ntohl(audio->headlen);
    audio->type      = ntohl(audio->type);
    audio->unknown1  = ntohl(audio->unknown1);
    audio->index     = ntohl(audio->index);
    audio->fragcount = ntohl(audio->fragcount);
    audio->unknown2  = ntohl(audio->unknown2);
    audio->unknown3  = ntohl(audio->unknown3);
    audio->offset    = ntohl(audio->offset);
    _v3_vrf_print_audio(audio);
    _v3_vrf_unlock(vrfh);

    _v3_func_leave("_v3_vrf_get_audio");
    return V3_OK;
}/*}}}*/

int
_v3_vrf_get_fragment(v3_vrf *vrfh, uint32_t type, uint32_t *offset, v3_vrf_fragment *fragment, uint32_t *fraglen, void **fragdata) {/*{{{*/
    _v3_func_enter("_v3_vrf_get_fragment");

    if (!vrfh || !offset || !fragment) {
        _v3_func_leave("_v3_vrf_get_fragment");
        return V3_FAILURE;
    }
    uint32_t fragread;
    switch (type) {
      case V3_VRF_TYPE_AUDIO:
        fragread = sizeof(v3_vrf_fragment) - sizeof(fragment->audio.ext);
        break;
      case V3_VRF_TYPE_TEXT:
        fragread = sizeof(v3_vrf_fragment) - sizeof(fragment->audio);
        break;
      case V3_VRF_TYPE_EXT:
        fragread = sizeof(v3_vrf_fragment);
        break;
      default:
        _v3_error("%s: unknown audio type: 0x%02x", vrfh->filename, type);
        _v3_func_leave("_v3_vrf_get_fragment");
        return V3_FAILURE;
    }
    _v3_vrf_lock(vrfh);
    fseek(vrfh->file, *offset, SEEK_SET);
    if (!fread(fragment, fragread, 1, vrfh->file)) {
        _v3_error("%s: failed to get vrf audio fragment: %s", vrfh->filename, strerror(errno));
        _v3_vrf_unlock(vrfh);
        _v3_func_leave("_v3_vrf_get_fragment");
        return V3_FAILURE;
    }
    fragment->headlen = ntohl(fragment->headlen);
    fragment->fraglen = ntohl(fragment->fraglen);
    if (type != V3_VRF_TYPE_TEXT) {
        fragment->audio.pcmlen   = ntohl(fragment->audio.pcmlen);
        fragment->audio.unknown1 = ntohl(fragment->audio.unknown1);
    }
    if (type == V3_VRF_TYPE_EXT) {
        fragment->audio.ext.codec       = ntohs(fragment->audio.ext.codec);
        fragment->audio.ext.codecformat = ntohs(fragment->audio.ext.codecformat);
        fragment->audio.ext.unknown2    = ntohl(fragment->audio.ext.unknown2);
    }
    _v3_vrf_print_fragment(type, fragment);
    uint32_t _fraglen;
    switch (type) {
      case V3_VRF_TYPE_AUDIO:
      case V3_VRF_TYPE_EXT:
        _fraglen = fragment->fraglen;
        break;
      case V3_VRF_TYPE_TEXT:
        _fraglen = fragment->headlen - fragread;
        break;
    }
    if (fraglen && fragdata) {
        if (!fragment->headlen || _fraglen > V3_VRF_MAX_FRAGLEN) {
            _v3_error("%s: vrf fragment is corrupted", vrfh->filename);
            _v3_vrf_unlock(vrfh);
            _v3_func_leave("_v3_vrf_get_fragment");
            return V3_MALFORMED;
        }
        *fragdata = malloc(_fraglen);
        memset(*fragdata, 0, _fraglen);
        if (!fread(*fragdata, _fraglen, 1, vrfh->file)) {
            _v3_error("%s: failed to get vrf audio fragment data: %s", vrfh->filename, strerror(errno));
            free(*fragdata);
            *fragdata = NULL;
            _v3_vrf_unlock(vrfh);
            _v3_func_leave("_v3_vrf_get_fragment");
            return V3_FAILURE;
        }
        *fraglen = _fraglen;
    }
    *offset += fragread + _fraglen;
    _v3_vrf_unlock(vrfh);

    _v3_func_leave("_v3_vrf_get_fragment");
    return V3_OK;
}/*}}}*/

int
v3_vrf_get_audio(void *vrfh, uint32_t id, v3_vrf_data *vrfd) {/*{{{*/
    _v3_func_enter("v3_vrf_get_audio");

    if (!vrfh || !vrfd) {
        _v3_func_leave("v3_vrf_get_audio");
        return V3_FAILURE;
    }
    v3_vrf *_vrfh = vrfh;
    vrfd->type = V3_VRF_DATA_NULL;
    v3_vrf_audio *audio = vrfd->_audio;
    if (!audio) {
        v3_vrf_segment *segment;
        if (!(segment = _v3_vrf_get_segment(vrfh, id))) {
            _v3_func_leave("v3_vrf_get_audio");
            return V3_FAILURE;
        }
        if (!segment->offset || segment->offset >= _vrfh->header.segtable || segment->offset >= _vrfh->filelen) {
            _v3_error("%s: vrf segment %i is corrupted", _vrfh->filename, id);
            _v3_func_leave("v3_vrf_get_audio");
            return V3_MALFORMED;
        }
        audio = malloc(sizeof(v3_vrf_audio));
        memset(audio, 0, sizeof(v3_vrf_audio));
        if (_v3_vrf_get_audio(vrfh, segment->offset, audio) != V3_OK) {
            free(audio);
            _v3_vrf_unlock(vrfh);
            _v3_func_leave("v3_vrf_get_audio");
            return V3_FAILURE;
        }
        audio->offset = segment->offset + audio->headlen;
        vrfd->_audio = audio;
    }
    if (audio->fragcount --> 0) {
        v3_vrf_fragment fragment;
        uint32_t fraglen;
        void *fragdata;
        if (_v3_vrf_get_fragment(vrfh, audio->type, &audio->offset, &fragment, &fraglen, &fragdata) != V3_OK) {
            _v3_func_leave("v3_vrf_get_audio");
            return V3_FAILURE;
        }
        switch (audio->type) {
          case V3_VRF_TYPE_AUDIO:
            fragment.audio.ext.codec       = _vrfh->header.codec;
            fragment.audio.ext.codecformat = _vrfh->header.codecformat;
          case V3_VRF_TYPE_EXT:
            vrfd->type        = V3_VRF_DATA_AUDIO;
            vrfd->codec       = fragment.audio.ext.codec;
            vrfd->codecformat = fragment.audio.ext.codecformat;
            vrfd->rate        = v3_get_codec_rate(vrfd->codec, vrfd->codecformat);
            vrfd->channels    = (fragment.audio.pcmlen - 2000 == 2) ? 2 : 1;
            if (!vrfd->_decoder) {
                vrfd->_decoder = malloc(sizeof(_v3_decoders));
                memset(vrfd->_decoder, 0, sizeof(_v3_decoders));
            }
            v3_event *ev;
            vrfd->length = sizeof(ev->data->sample);
            if (!vrfd->data) {
                vrfd->data = malloc(vrfd->length);
            }
            memset(vrfd->data, 0, vrfd->length);
            int ret;
            if ((ret = _v3_audio_decode(
                            /* encoded input */
                            v3_get_codec(vrfd->codec, vrfd->codecformat),
                            vrfd->_decoder,
                            fragdata,
                            fraglen,
                            /* pcm output */
                            vrfd->data,
                            &vrfd->length,
                            /* optional args */
                            vrfd->channels)) != V3_OK) {
                free(fragdata);
                _v3_func_leave("v3_vrf_get_audio");
                return ret;
            }
            free(fragdata);
            break;
          case V3_VRF_TYPE_TEXT:
            if (vrfd->text) {
                free(vrfd->text);
                vrfd->text = NULL;
            }
            vrfd->type   = V3_VRF_DATA_TEXT;
            vrfd->text   = fragdata;
            vrfd->length = fraglen;
            break;
        }
        _v3_func_leave("v3_vrf_get_audio");
        return V3_OK;
    }
    audio->fragcount = 0;

    _v3_func_leave("v3_vrf_get_audio");
    return V3_OK;
}/*}}}*/

int
_v3_vrf_put_header(v3_vrf *vrfh) {/*{{{*/
    _v3_func_enter("_v3_vrf_put_header");

    if (!vrfh) {
        _v3_func_leave("_v3_vrf_put_header");
        return V3_FAILURE;
    }
    v3_vrf_header _header;
    memcpy(&_header, &vrfh->header, sizeof(v3_vrf_header));
    _v3_vrf_print_header(&_header);
    _header.size        = htonl(_header.size);
    _header.headlen     = htonl(_header.headlen);
    _header.unknown1    = htonl(_header.unknown1);
    _header.segtable    = htonl(_header.segtable);
    _header.segcount    = htonl(_header.segcount);
    _header.vrfversion  = htonl(_header.vrfversion);
    _header.unknown2    = htonl(_header.unknown2);
    _header.unknown3    = htonl(_header.unknown3);
    _header.infolen     = htonl(_header.infolen);
    _header.codec       = htonl(_header.codec);
    _header.codecformat = htonl(_header.codecformat);
    _header.unknown4    = htonl(_header.unknown4);
    _v3_vrf_lock(vrfh);
    rewind(vrfh->file);
    if (!fwrite(&_header, sizeof(v3_vrf_header), 1, vrfh->file)) {
        _v3_error("%s: failed to put vrf header: %s", vrfh->filename, strerror(errno));
        _v3_vrf_unlock(vrfh);
        _v3_func_leave("_v3_vrf_put_header");
        return V3_FAILURE;
    }
    fflush(vrfh->file);
    _v3_vrf_unlock(vrfh);

    _v3_func_leave("_v3_vrf_put_header");
    return V3_OK;
}/*}}}*/

int
v3_vrf_put_info(void *vrfh, const v3_vrf_data *vrfd) {/*{{{*/
    _v3_func_enter("v3_vrf_put_info");

    if (!vrfh || !vrfd) {
        _v3_func_leave("v3_vrf_put_info");
        return V3_FAILURE;
    }
    v3_vrf *_vrfh = vrfh;
    if (!_vrfh->file || !_vrfh->filename) {
        _v3_error("%p: no file opened", vrfh);
        _v3_func_leave("v3_vrf_put_info");
        return V3_FAILURE;
    }
    v3_vrf_header *header = &_vrfh->header;
    strncpy(header->username,  vrfd->username,  sizeof(vrfd->username) - 1);
    strncpy(header->comment,   vrfd->comment,   sizeof(vrfd->comment) - 1);
    strncpy(header->url,       vrfd->url,       sizeof(vrfd->url) - 1);
    strncpy(header->copyright, vrfd->copyright, sizeof(vrfd->copyright) - 1);
    _v3_vrf_print_info(header);
    if (_v3_vrf_put_header(vrfh) != V3_OK) {
        _v3_func_leave("v3_vrf_put_info");
        return V3_FAILURE;
    }

    _v3_func_leave("v3_vrf_put_info");
    return V3_OK;
}/*}}}*/

uint32_t
_v3_vrf_put_segment(uint32_t id, const v3_vrf_segment *segment, void *offset) {/*{{{*/
    _v3_func_enter("_v3_vrf_put_segment");

    if (!segment || !offset) {
        _v3_func_leave("_v3_vrf_put_segment");
        return 0;
    }
    v3_vrf_segment _segment;
    memcpy(&_segment, segment, sizeof(v3_vrf_segment));
    _v3_vrf_print_segment(id, &_segment);
    _segment.headlen  = htonl(_segment.headlen);
    _segment.type     = htonl(_segment.type);
    _segment.valid    = htonl(_segment.valid);
    _segment.offset   = htonl(_segment.offset);
    _segment.time     = htonl(_segment.time);
    _segment.duration = htonl(_segment.duration);
    _segment.unknown1 = htonl(_segment.unknown1);
    _segment.unknown2 = htonl(_segment.unknown2);
    memcpy(offset, &_segment, sizeof(v3_vrf_segment));

    _v3_func_leave("_v3_vrf_put_segment");
    return sizeof(v3_vrf_segment);
}/*}}}*/

uint32_t
_v3_vrf_put_audio(const v3_vrf_audio *audio, void *offset) {/*{{{*/
    _v3_func_enter("_v3_vrf_put_audio");

    if (!audio || !offset) {
        _v3_func_leave("_v3_vrf_put_audio");
        return 0;
    }
    v3_vrf_audio _audio;
    memcpy(&_audio, audio, sizeof(v3_vrf_audio));
    _v3_vrf_print_audio(&_audio);
    _audio.headlen   = htonl(_audio.headlen);
    _audio.type      = htonl(_audio.type);
    _audio.unknown1  = htonl(_audio.unknown1);
    _audio.index     = htonl(_audio.index);
    _audio.fragcount = htonl(_audio.fragcount);
    _audio.unknown2  = htonl(_audio.unknown2);
    _audio.unknown3  = htonl(_audio.unknown3);
    _audio.offset    = htonl(_audio.offset);
    memcpy(offset, &_audio, sizeof(v3_vrf_audio));

    _v3_func_leave("_v3_vrf_put_audio");
    return sizeof(v3_vrf_audio);
}/*}}}*/

uint32_t
_v3_vrf_put_fragment(uint32_t type, const v3_vrf_fragment *fragment, void *offset) {/*{{{*/
    _v3_func_enter("_v3_vrf_put_fragment");

    if (!fragment || !offset) {
        _v3_func_leave("_v3_vrf_put_fragment");
        return 0;
    }
    uint32_t fragwrite;
    v3_vrf_fragment _fragment;
    memcpy(&_fragment, fragment, sizeof(v3_vrf_fragment));
    _v3_vrf_print_fragment(type, &_fragment);
    fragwrite = sizeof(v3_vrf_fragment) - sizeof(_fragment.audio);
    _fragment.headlen = htonl(_fragment.headlen);
    _fragment.fraglen = htonl(_fragment.fraglen);
    if (type != V3_VRF_TYPE_TEXT) {
        fragwrite = sizeof(v3_vrf_fragment) - sizeof(_fragment.audio.ext);
        _fragment.audio.pcmlen   = htonl(_fragment.audio.pcmlen);
        _fragment.audio.unknown1 = htonl(_fragment.audio.unknown1);
    }
    if (type == V3_VRF_TYPE_EXT) {
        fragwrite = sizeof(v3_vrf_fragment);
        _fragment.audio.ext.codec       = htons(_fragment.audio.ext.codec);
        _fragment.audio.ext.codecformat = htons(_fragment.audio.ext.codecformat);
        _fragment.audio.ext.unknown2    = htonl(_fragment.audio.ext.unknown2);
    }
    memcpy(offset, &_fragment, fragwrite);

    _v3_func_leave("_v3_vrf_put_fragment");
    return fragwrite;
}/*}}}*/

void
_v3_vrf_put_record(uint32_t user_id, uint32_t index, uint32_t type, const char *username, v3_vrf_rec *rec) {/*{{{*/
    _v3_func_enter("_v3_vrf_put_record");

    if (!rec) {
        _v3_func_leave("_v3_vrf_put_record");
        return;
    }
    v3_vrf_segment *segment = &rec->segment;
    rec->user_id  = user_id;
    rec->index    = index;
    segment->type = type;
    if (username) {
        strncpy(segment->username, username, sizeof(segment->username) - 1);
    }
    v3_vrf_audio audio;
    memset(&audio, 0, sizeof(v3_vrf_audio));
    audio.headlen = sizeof(v3_vrf_audio) + sizeof(segment->username);
    audio.type    = type;
    audio.index   = index;
    rec->data = malloc(audio.headlen);
    memset(rec->data, 0, audio.headlen);
    rec->datalen = _v3_vrf_put_audio(&audio, rec->data);
    if (username) {
        strncpy(rec->data + rec->datalen, username, sizeof(segment->username) - 1);
    }
    rec->datalen = audio.headlen;

    _v3_func_leave("_v3_vrf_put_record");
}/*}}}*/

void
_v3_vrf_record_event(
        int type,
        uint16_t user_id,
        uint16_t codec,
        uint16_t codecformat,
        uint32_t pcmlen,
        uint32_t datalen,
        void *data) {/*{{{*/
    _v3_func_enter("_v3_vrf_record_event");

    if (!_v3_vrfh) {
        _v3_func_leave("_v3_vrf_record_event");
        return;
    }
    v3_user *u;
    if (!(u = _v3_get_user(user_id))) {
        type = V3_VRF_EVENT_DATA_FLUSH;
    }
    uint32_t time;
    struct timeval now;
    gettimeofday(&now, NULL);
    time = now.tv_sec * 1000 + now.tv_usec / 1000 - _v3_vrfh->start.tv_sec * 1000 + _v3_vrfh->start.tv_usec / 1000;
    _v3_vrf_lock(_v3_vrfh);
    v3_vrf_rec *queue = &_v3_vrfh->queue;
    v3_vrf_rec *rec = NULL;
    v3_event *ev = _v3_create_event(V3_EVENT_RECORD_UPDATE);
    switch (type) {
      case V3_VRF_EVENT_AUDIO_DATA:
      case V3_VRF_EVENT_TEXT_DATA:
        if (!data || !u->allow_recording) {
            break;
        }
        for (rec = queue; rec; rec = rec->next) {
            if (rec->user_id == user_id && !rec->stopped) {
                break;
            }
            if (!rec->user_id || !rec->next) {
                if (rec->user_id && !rec->next) {
                    rec->next = malloc(sizeof(v3_vrf_rec));
                    memset(rec->next, 0, sizeof(v3_vrf_rec));
                    rec = rec->next;
                }
                _v3_vrfh->header.segcount++;
                break;
            }
        }
    }
    switch (type) {
      case V3_VRF_EVENT_AUDIO_DATA:
      {
        if (!rec) {
            v3_free_event(ev);
            ev = NULL;
            break;
        }
        if (!rec->user_id) {
            _v3_vrf_put_record(user_id, _v3_vrfh->header.segcount - 1, V3_VRF_TYPE_EXT, u->name, rec);
        }
        v3_vrf_fragment fragment;
        memset(&fragment, 0, sizeof(v3_vrf_fragment));
        fragment.headlen = sizeof(v3_vrf_fragment);
        fragment.fraglen = datalen;
        fragment.audio.pcmlen   = pcmlen;
        fragment.audio.unknown1 = time;
        fragment.audio.ext.codec       = codec;
        fragment.audio.ext.codecformat = codecformat;
        rec->data = realloc(rec->data, rec->datalen + fragment.headlen + datalen);
        rec->datalen += _v3_vrf_put_fragment(V3_VRF_TYPE_EXT, &fragment, rec->data + rec->datalen);
        memcpy(rec->data + rec->datalen, data, datalen);
        rec->datalen += datalen;
        if (!rec->segment.time) {
            rec->segment.time = time;
        }
        rec->segment.duration = time;
        rec->fragcount++;
        strncpy(ev->text.name, rec->segment.username, sizeof(ev->text.name) - 1);
        ev->record.index = rec->index;
        ev->record.time = time;
        break;
      }
      case V3_VRF_EVENT_TEXT_DATA:
      {
        if (!rec) {
            v3_free_event(ev);
            ev = NULL;
            break;
        }
        if (!rec->user_id) {
            _v3_vrf_put_record(user_id, _v3_vrfh->header.segcount - 1, V3_VRF_TYPE_TEXT, u->name, rec);
        }
        v3_vrf_fragment fragment;
        memset(&fragment, 0, sizeof(v3_vrf_fragment));
        fragment.headlen = sizeof(v3_vrf_fragment) + V3_VRF_TEXTLEN - sizeof(fragment.audio);
        fragment.fraglen = time;
        rec->data = realloc(rec->data, rec->datalen + fragment.headlen);
        memset(rec->data + rec->datalen, 0, fragment.headlen);
        rec->datalen += _v3_vrf_put_fragment(V3_VRF_TYPE_TEXT, &fragment, rec->data + rec->datalen);
        strncpy(rec->data + rec->datalen, data, V3_VRF_TEXTLEN);
        rec->datalen += V3_VRF_TEXTLEN;
        rec->segment.time = time;
        rec->segment.duration = time;
        rec->fragcount++;
        rec->stopped = true;
        strncpy(ev->status.message, data, sizeof(ev->status.message) - 1);
        strncpy(ev->text.name, rec->segment.username, sizeof(ev->text.name) - 1);
        ev->record.index = rec->index;
        ev->record.time = time;
        ev->record.stopped = true;
        break;
      }
      case V3_VRF_EVENT_AUDIO_STOP:
        for (rec = queue; rec; rec = rec->next) {
            if (rec->user_id == user_id && !rec->stopped) {
                rec->stopped = true;
                ev->record.index = rec->index;
                ev->record.stopped = true;
                break;
            }
        }
        if (!rec) {
            v3_free_event(ev);
            ev = NULL;
        }
        break;
      case V3_VRF_EVENT_DATA_FLUSH:
      default:
        for (rec = queue; rec; rec = rec->next) {
            if (rec->user_id) {
                rec->stopped = true;
            }
        }
        ev->record.flushed = true;
        break;
    }
    for (rec = queue; rec; rec = rec->next) {
        if (rec->data && (rec == queue || rec->stopped)) {
            fseek(_v3_vrfh->file, 0, SEEK_END);
            if (!rec->segment.offset) {
                rec->segment.offset = ftell(_v3_vrfh->file);
            }
            if (!fwrite(rec->data, rec->datalen, 1, _v3_vrfh->file)) {
                _v3_error("%s: FATAL: failed to put vrf data: %s", _v3_vrfh->filename, strerror(errno));
            }
            rec->datalen = 0;
            free(rec->data);
            rec->data = NULL;
        }
        if (!rec->stopped) {
            break;
        }
    }
    while (queue->stopped) {
        v3_vrf_segment *segment = &queue->segment;
        segment->headlen   = sizeof(v3_vrf_segment);
        segment->valid     = true;
        segment->duration -= segment->time;
        _v3_vrfh->table = realloc(_v3_vrfh->table, _v3_vrfh->tablesize + segment->headlen);
        _v3_vrfh->tablesize += _v3_vrf_put_segment(queue->index, segment, (void *)_v3_vrfh->table + _v3_vrfh->tablesize);
        v3_vrf_audio audio;
        memset(&audio, 0, sizeof(v3_vrf_audio));
        if (_v3_vrf_get_audio(_v3_vrfh, segment->offset, &audio) == V3_OK) {
            audio.fragcount = queue->fragcount;
            _v3_vrf_put_audio(&audio, &audio);
            fseek(_v3_vrfh->file, segment->offset, SEEK_SET);
            fwrite(&audio, sizeof(v3_vrf_audio), 1, _v3_vrfh->file);
            fflush(_v3_vrfh->file);
        }
        v3_vrf_rec *next;
        if ((next = queue->next)) {
            memcpy(queue, next, sizeof(v3_vrf_rec));
            free(next);
        } else {
            memset(queue, 0, sizeof(v3_vrf_rec));
        }
    }
    if (ev) {
        v3_queue_event(ev);
    }
    _v3_vrf_unlock(_v3_vrfh);

    _v3_func_leave("_v3_vrf_record_event");
}/*}}}*/

void
_v3_vrf_record_finish(v3_vrf *vrfh, uint32_t segtable) {/*{{{*/
    _v3_func_enter("_v3_vrf_record_finish");

    if (!vrfh) {
        _v3_func_leave("_v3_vrf_record_finish");
        return;
    }
    if (vrfh->table) {
        fseek(vrfh->file, segtable, SEEK_SET);
        if (!fwrite(vrfh->table, vrfh->tablesize, 1, vrfh->file)) {
            _v3_error("%s: FATAL: failed to put vrf segment table: %s", vrfh->filename, strerror(errno));
        }
        fflush(vrfh->file);
        vrfh->tablesize = 0;
        free(vrfh->table);
        vrfh->table = NULL;
    }
    vrfh->header.segtable = segtable;
    strncpy(vrfh->header.headid, V3_VRF_HEADID, sizeof(vrfh->header.headid));
    fseek(vrfh->file, 0, SEEK_END);
    vrfh->filelen = ftell(vrfh->file);
    vrfh->header.size = vrfh->filelen;
    _v3_vrf_put_header(vrfh);

    _v3_func_leave("_v3_vrf_record_finish");
}/*}}}*/

int
v3_vrf_record_start(const char *filename) {/*{{{*/
    _v3_func_enter("v3_vrf_record_start");

    if (_v3_vrfh) {
        _v3_error("vrf is recording: %s", _v3_vrfh->filename);
        _v3_func_leave("v3_vrf_record_start");
        return V3_FAILURE;
    }
    if (!filename || !strlen(filename)) {
        _v3_error("no filename specified");
        _v3_func_leave("v3_vrf_record_start");
        return V3_FAILURE;
    }
    v3_vrf *vrfh;
    if (!(vrfh = v3_vrf_init(NULL))) {
        _v3_func_leave("v3_vrf_record_start");
        return V3_FAILURE;
    }
    if (!(vrfh->file = fopen(filename, "wb+"))) {
        _v3_error("%s: %s", filename, strerror(errno));
        v3_vrf_destroy(vrfh);
        _v3_func_leave("v3_vrf_record_start");
        return V3_FAILURE;
    }
    vrfh->filename = strdup(filename);
    _v3_debug(V3_DEBUG_INFO, "opened file for writing: %s", vrfh->filename);
    gettimeofday(&vrfh->start, NULL);
    v3_vrf_header *header = &vrfh->header;
    strncpy(header->headid, V3_VRF_TEMPID, sizeof(header->headid));
    header->headlen     = V3_VRF_HEADLEN;
    header->vrfversion  = V3_VRF_VERSION;
    header->infolen     = V3_VRF_INFOLEN;
    header->codec       = v3_server.codec;
    header->codecformat = v3_server.codec_format;
    strncpy(header->platform, "Linux", sizeof(header->platform) - 1);
    strncpy(header->version, V3_CLIENT_VERSION, sizeof(header->version) - 1);
    strncpy(header->username, v3_luser.name, sizeof(header->username) - 1);
    if (_v3_vrf_put_header(vrfh) != V3_OK) {
        v3_vrf_destroy(vrfh);
        _v3_func_leave("v3_vrf_record_start");
        return V3_FAILURE;
    }
    _v3_vrfh = vrfh;

    _v3_func_leave("v3_vrf_record_start");
    return V3_OK;
}/*}}}*/

void
v3_vrf_record_stop(void) {/*{{{*/
    _v3_func_enter("v3_vrf_record_stop");

    if (!_v3_vrfh) {
        _v3_func_leave("v3_vrf_record_stop");
        return;
    }
    v3_vrf *vrfh = _v3_vrfh;
    _v3_vrf_lock(vrfh);
    _v3_vrf_record_event(V3_VRF_EVENT_DATA_FLUSH, v3_get_user_id(), -1, -1, 0, 0, NULL);
    _v3_vrfh = NULL;
    _v3_vrf_unlock(vrfh);
    fseek(vrfh->file, 0, SEEK_END);
    _v3_vrf_record_finish(vrfh, ftell(vrfh->file));
    v3_vrf_destroy(vrfh);

    _v3_func_leave("v3_vrf_record_stop");
}/*}}}*/

int
_v3_vrf_recover(v3_vrf *vrfh) {/*{{{*/
    _v3_func_enter("_v3_vrf_recover");

    if (!vrfh || !vrfh->filelen || vrfh->table || vrfh->header.segcount) {
        _v3_func_leave("_v3_vrf_recover");
        return V3_FAILURE;
    }
    if (strncmp(vrfh->header.headid, V3_VRF_TEMPID, sizeof(vrfh->header.headid))) {
        _v3_func_leave("_v3_vrf_recover");
        return V3_FAILURE;
    }
    uint32_t offset = sizeof(v3_vrf_header);
    if (fseek(vrfh->file, offset, SEEK_SET)) {
        _v3_func_leave("_v3_vrf_recover");
        return V3_FAILURE;
    }
    uint32_t headlen;
    v3_vrf_audio audio;
    v3_vrf_fragment fragment;
    v3_vrf_segment *segment;
    while (offset + sizeof(headlen) < vrfh->filelen) {
        if (fseek(vrfh->file, offset, SEEK_SET)) {
            break;
        }
        if (!fread(&headlen, sizeof(headlen), 1, vrfh->file)) {
            break;
        }
        switch (ntohl(headlen)) {
          case (sizeof(v3_vrf_audio) + sizeof(segment->username)):
            memset(&audio, 0, sizeof(v3_vrf_audio));
            if (_v3_vrf_get_audio(vrfh, offset, &audio) != V3_OK) {
                _v3_func_leave("_v3_vrf_recover");
                return V3_FAILURE;
            }
            vrfh->table = realloc(vrfh->table, vrfh->tablesize + sizeof(v3_vrf_segment));
            vrfh->header.segcount++;
            segment = &vrfh->table[vrfh->header.segcount - 1];
            memset(segment, 0, sizeof(v3_vrf_segment));
            segment->headlen = sizeof(v3_vrf_segment);
            segment->type    = audio.type;
            segment->offset  = offset;
            fread(segment->username, sizeof(segment->username) - 1, 1, vrfh->file);
            vrfh->tablesize += segment->headlen;
            offset += audio.headlen;
            break;
          case (sizeof(v3_vrf_fragment)):
          case (V3_VRF_TEXTLEN + (sizeof(v3_vrf_fragment) - sizeof(fragment.audio))):
            if (!vrfh->header.segcount) {
                _v3_func_leave("_v3_vrf_recover");
                return V3_FAILURE;
            }
            segment = &vrfh->table[vrfh->header.segcount - 1];
            segment->valid = true;
            segment->unknown1++;
            memset(&fragment, 0, sizeof(v3_vrf_fragment));
            if (_v3_vrf_get_fragment(vrfh, segment->type, &offset, &fragment, NULL, NULL) != V3_OK) {
                _v3_func_leave("_v3_vrf_recover");
                return V3_FAILURE;
            }
            switch (segment->type) {
              case V3_VRF_TYPE_AUDIO:
              case V3_VRF_TYPE_EXT:
                if (!segment->time) {
                    segment->time = fragment.audio.unknown1;
                } else {
                    segment->duration = fragment.audio.unknown1 - segment->time;
                }
                break;
              case V3_VRF_TYPE_TEXT:
                segment->time = fragment.fraglen;
                break;
            }
            break;
          default:
            _v3_func_leave("_v3_vrf_recover");
            return V3_FAILURE;
        }
    }
    if (vrfh->header.segcount) {
        uint32_t ctr;
        for (ctr = 0; ctr < vrfh->header.segcount; ctr++) {
            segment = &vrfh->table[ctr];
            memset(&audio, 0, sizeof(v3_vrf_audio));
            if (_v3_vrf_get_audio(vrfh, segment->offset, &audio) == V3_OK) {
                audio.fragcount = segment->unknown1;
                _v3_vrf_put_audio(&audio, &audio);
                fseek(vrfh->file, segment->offset, SEEK_SET);
                fwrite(&audio, sizeof(v3_vrf_audio), 1, vrfh->file);
            }
            segment->unknown1 = 0;
            _v3_vrf_put_segment(ctr, segment, segment);
        }
    }
    fseek(vrfh->file, 0, SEEK_END);
    _v3_vrf_record_finish(vrfh, ftell(vrfh->file));

    _v3_func_leave("_v3_vrf_recover");
    return V3_OK;
}/*}}}*/

/*
 *  Pretty much all packet processing happens in this function.
 *
 *  Pass in a raw message structure with the type, length, and data members set
 *  and it updates all internal structures and queues any events that need to
 *  be passed on to a calling program.
 *
 *  Returns an integer:
 *
 *     V3_NOTIMPL......: The packet type is not implemented
 *     V3_MALFORMED....: The packet type is corrupt and/or the subtype is not implemented
 *     V3_OK...........: The packet was processed successfully
 *     V3_FAILURE......: The packet was processed, but a critical error occurred
 */
int
_v3_process_message(_v3_net_message *msg) {/*{{{*/
    _v3_func_enter("_v3_process_message");
    _v3_debug(V3_DEBUG_INTERNAL, "beginning packet processing on msg type '0x%02X' (%d)", msg->type, (uint16_t)msg->type);
    switch (msg->type) {
        case 0x06:/*{{{*/
            if(!_v3_get_0x06(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_msg_0x06 *m = msg->contents;
                char buf[512] = "";
                int error = false;
                int disconnected = false;

                _v3_lock_server();
                if(m->subtype & 0x01) {
                    error = true;
                    disconnected = true;
                    strncat(buf, "You have been disconnected from the server.\n", 511);
                    _v3_logout();
                }
                if(m->subtype & 0x02) {
                    v3_event *ev = _v3_create_event(V3_EVENT_ADMIN_AUTH);
                    v3_queue_event(ev);
                }
                if(m->subtype & 0x04) {
                    if (ventrilo_read_keys(&v3_server.client_key, &v3_server.server_key, m->encryption_key, msg->len - 12) < 0) {
                        _v3_error("could not parse keys from the server");
                        free(m->encryption_key);
                        _v3_func_leave("_v3_process_message");
                        return V3_FAILURE;
                    }
                    free(m->encryption_key);
                }
                if(m->subtype & 0x10) {
                    _v3_debug(V3_DEBUG_INTERNAL, "FIXME: Unknown subtype, please report a packetdump.");
                }
                if(m->subtype & 0x20) {
                    _v3_debug(V3_DEBUG_INTERNAL, "FIXME: Unknown subtype, please report a packetdump.");
                }
                if(m->subtype & 0x40) {
                    error = true;
                    strncat(buf, "The supplied password is incorrect.\n", 511);
                }
                if(m->subtype & 0x100) {
                    _v3_debug(V3_DEBUG_INTERNAL, "FIXME: Unknown subtype, please report a packetdump.");
                }
                if(m->subtype & 0x200) {
                    error = true;
                    strncat(buf, _v3_server_disabled_errors[m->error_id-1], 511);
                }
                if(m->subtype & 0x400) {
                    _v3_debug(V3_DEBUG_INTERNAL, "FIXME: Unknown subtype, please report a packetdump.");
                }

                _v3_destroy_packet(msg);
                _v3_unlock_server();

                if (error) {
                    _v3_error("%s", buf);

                    if (v3_is_loggedin()) {
                        v3_event *ev = _v3_create_event(V3_EVENT_ERROR_MSG);
                        ev->error.disconnected = disconnected;
                        strncpy(ev->error.message, buf, 512);
                        v3_queue_event(ev);
                    }

                    _v3_func_leave("_v3_process_message");
                    return V3_FAILURE;
                } else {
                    // it's an informational message free it for now, may want
                    // to queue it as something else later
                    _v3_func_leave("_v3_process_message");
                    return V3_OK;
                }
            }/*}}}*/
        case 0x33:/*{{{*/
            if (!_v3_get_0x33(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                int ctr;
                _v3_msg_0x33 *m = msg->contents;
                memset(v3_luser.channel_admin, 0, 65535);
                for (ctr = 0; ctr < m->channel_id_count; ctr++) {
                    v3_luser.channel_admin[m->channel_ids[ctr]] = 1;
                }
                v3_event *ev = _v3_create_event(V3_EVENT_CHAN_ADMIN_UPDATED);
                v3_queue_event(ev);
            }
            _v3_destroy_0x33(msg);
            _v3_destroy_packet(msg);
            _v3_func_leave("_v3_process_message");
            return V3_OK;/*}}}*/
        case 0x34:/*{{{*/
            _v3_lock_server();
            _v3_debug(V3_DEBUG_INTERNAL, "scrambling client encryption keys");
            ventrilo3_algo_scramble(&v3_server.client_key, (uint8_t *)v3_server.handshake_key);
            _v3_debug(V3_DEBUG_INTERNAL, "scrambling server encryption keys");
            ventrilo3_algo_scramble(&v3_server.server_key, (uint8_t *)v3_server.handshake_key);
            _v3_destroy_packet(msg);
            _v3_user_loggedin = 1;
            _v3_unlock_server();
            _v3_func_leave("_v3_process_message");
            return V3_OK;/*}}}*/
        case 0x36:/*{{{*/
            if (!_v3_get_0x36(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_msg_0x36 *m = msg->contents;
                v3_rank *rl;
                int ctr;

                _v3_lock_ranklist();

                if (m->error_id) {
                    if (m->error_id != 5) {
                        char *error;

                        if (m->error_id > (sizeof(_v3_ranks_errors) / sizeof(_v3_ranks_errors[0])))
                            error = "Unknown";
                        else
                            error = _v3_ranks_errors[m->error_id - 1];

                        v3_event *ev = _v3_create_event(V3_EVENT_ERROR_MSG);
                        strncpy(ev->error.message, "Rank editor error:\n", sizeof(ev->error.message));
                        strncat(ev->error.message, error, sizeof(ev->error.message));
                        v3_queue_event(ev);
                    }

                    _v3_destroy_0x36(msg);
                    _v3_destroy_packet(msg);
                    _v3_func_leave("_v3_process_message");
                    return V3_OK;
                }

                rl = calloc(m->rank_count, sizeof(v3_user));
                switch (m->subtype) {
                    case V3_REMOVE_RANK:
                        _v3_debug(V3_DEBUG_INFO, "removing %d ranks from user list",  m->rank_count);
                        for (ctr = 0; ctr < m->rank_count; ctr++) {
                            // do we need to queue an event?
                            v3_event *ev = _v3_create_event(V3_EVENT_RANK_REMOVE);
                            ev->data->rank.id = m->rank_list[ctr].id;
                            _v3_debug(V3_DEBUG_INFO, "queuing event type %d for user %d", ev->type, ev->user.id);
                            _v3_remove_rank(m->rank_list[ctr].id);
                            v3_queue_event(ev);
                        }
                        break;
                    case V3_MODIFY_RANK:
                    case V3_ADD_RANK:
                    case V3_RANK_LIST:
                        _v3_debug(V3_DEBUG_INFO, "adding %d ranks on rank list",  m->rank_count);
                        for (ctr = 0; ctr < m->rank_count; ctr++) {
                            _v3_update_rank(&m->rank_list[ctr]);
                            v3_event *ev = _v3_create_event(m->subtype == V3_ADD_RANK ? V3_EVENT_RANK_ADD : V3_EVENT_RANK_MODIFY);
                            ev->data->rank.id = m->rank_list[ctr].id;
                            ev->data->rank.level = m->rank_list[ctr].level;
                            strncpy(ev->text.name, m->rank_list[ctr].name, 31);
                            strncpy(ev->text.comment, m->rank_list[ctr].description, 127);
                            _v3_debug(V3_DEBUG_INFO, "queuing event type %d for user %d", ev->type, ev->user.id);
                            v3_queue_event(ev);
                        }
                        break;
#if 0
                    case V3_RANK_LIST:
                        // <strke>user 1 is always ourself in this subtype</strike>
                        // TODO: this is a bad assumption... the userlist can span multiple 0x5d packets
                        _v3_debug(V3_DEBUG_INFO, "adding %d ranks to rank list",  m->rank_count);
                        for (ctr = 0; ctr < m->rank_count; ctr++) {
                            _v3_update_rank(&m->rank_list[ctr]);
                            //v3_event *ev = _v3_create_event(V3_EVENT_RANK_LOGIN);
                            //ev->rank.id = m->rank_list[ctr].id;
                            //ev->channel.id = m->rank_list[ctr].channel;
                            //_v3_debug(V3_DEBUG_INFO, "queuing event type %d for rank %d", ev->type, ev->rank.id);
                            //v3_queue_event(ev);
                        }
                        break;
#endif
                }
                free(rl);
                _v3_unlock_ranklist();
            }
            _v3_destroy_0x36(msg);
            _v3_destroy_packet(msg);
            _v3_func_leave("_v3_process_message");
            return V3_OK;/*}}}*/
        case 0x37:/*{{{*/
            if (!_v3_get_0x37(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_msg_0x37 *m = msg->contents;
                v3_event *ev = _v3_create_event(V3_EVENT_PING);
                ev->ping = m->ping;
                v3_queue_event(ev);
                _v3_net_message *response;
                v3_luser.id = m->user_id;
                v3_luser.ping = m->ping;
                response = _v3_put_0x37(m->sequence);
                _v3_send(response);
                _v3_destroy_packet(response);
            }
            _v3_destroy_packet(msg);
            _v3_func_leave("_v3_process_message");
            return V3_OK;/*}}}*/
        case 0x3a:/*{{{*/
            /*
             *  This is almost identical to 0x3f, so whatever you do here probably
             *  needs to be done there, too.
             */
            if (!_v3_get_0x3a(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_msg_0x3a *m = msg->contents;
                v3_event *ev = _v3_create_event(V3_EVENT_TEXT_TO_SPEECH_MESSAGE);
                ev->user.id = m->user_id;
                strncpy(ev->data->chatmessage, m->msg, sizeof(ev->data->chatmessage) - 1);
                v3_queue_event(ev);
                _v3_vrf_record_event(V3_VRF_EVENT_TEXT_DATA, m->user_id, -1, -1, 0, 0, m->msg);
                free(m->msg);
            }
            _v3_destroy_packet(msg);
            _v3_func_leave("_v3_process_message");
            return V3_OK;/*}}}*/
        case 0x3b:/*{{{*/
            /*
             *  This is almost identical to 0x53, so whatever you do here probably
             *  needs to be done there, too.
             */
            if (!_v3_get_0x3b(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_msg_0x3b *m = msg->contents;
                v3_user *user;
                if (!m->error_id) {
                    _v3_debug(V3_DEBUG_INFO, "user %d force moved to channel %d", m->user_id, m->channel_id);
                    user = v3_get_user(m->user_id);
                    if (user) {
                        user->channel = m->channel_id;
                        _v3_update_user(user);
                        v3_free_user(user);
                        v3_event *ev = _v3_create_event(V3_EVENT_USER_CHAN_MOVE);
                        ev->user.id = m->user_id;
                        ev->channel.id = m->channel_id;
                        v3_queue_event(ev);
                        _v3_vrf_record_event(
                                (m->user_id == v3_get_user_id())
                                    ? V3_VRF_EVENT_DATA_FLUSH
                                    : V3_VRF_EVENT_AUDIO_STOP,
                                m->user_id, -1, -1, 0, 0, NULL);
                    }
                }
            }
            _v3_destroy_packet(msg);
            _v3_func_leave("_v3_process_message");
            return V3_OK;/*}}}*/
        case 0x3c:/*{{{*/
            if (!_v3_get_0x3c(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_msg_0x3c *m = msg->contents;
                v3_server.codec = m->codec;
                v3_server.codec_format = m->codec_format;
            }
            _v3_destroy_packet(msg);
            _v3_func_leave("_v3_process_message");
            return V3_OK;/*}}}*/
        case 0x3f:/*{{{*/
            /*
             *  This is almost identical to 0x3a, so whatever you do here probably
             *  needs to be done there, too.
             */
            if (!_v3_get_0x3f(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_msg_0x3f *m = msg->contents;
                v3_event *ev = _v3_create_event(V3_EVENT_PLAY_WAVE_FILE_MESSAGE);
                ev->user.id = m->user_id;
                strncpy(ev->data->chatmessage, m->msg, sizeof(ev->data->chatmessage) - 1);
                v3_queue_event(ev);
                free(m->msg);
            }
            _v3_destroy_packet(msg);
            _v3_func_leave("_v3_process_message");
            return V3_OK;/*}}}*/
        case 0x42:/*{{{*/
            if(!_v3_get_0x42(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_msg_0x42 *m = msg->contents;
                switch(m->subtype) {
                    case V3_JOIN_CHAT:
                        {
                            v3_event *ev = _v3_create_event(V3_EVENT_CHAT_JOIN);
                            ev->user.id = m->user_id;
                            v3_queue_event(ev);
                        }
                        break;
                    case V3_LEAVE_CHAT:
                        {
                            v3_event *ev = _v3_create_event(V3_EVENT_CHAT_LEAVE);
                            ev->user.id = m->user_id;
                            v3_queue_event(ev);
                        }
                        break;
                    case V3_TALK_CHAT:
                        {
                            v3_event *ev = _v3_create_event(V3_EVENT_CHAT_MESSAGE);
                            ev->user.id = m->user_id;
                            strncpy(ev->data->chatmessage, m->msg, sizeof(ev->data->chatmessage) - 1);
                            free(m->msg);
                            v3_queue_event(ev);
                        }
                        break;
                    case V3_RCON_CHAT:
                        {
                            v3_event *ev = _v3_create_event(V3_EVENT_CHAT_MESSAGE);
                            ev->user.id = 0;
                            strncpy(ev->data->chatmessage, m->msg, sizeof(ev->data->chatmessage) - 1);
                            free(m->msg);
                            v3_queue_event(ev);
                        }
                        break;
                    case V3_JOINFAIL_CHAT:
                        {
                            v3_event *ev = _v3_create_event(V3_EVENT_ERROR_MSG);
                            strncpy(ev->error.message, "You do not have sufficient access rights to use the global chat window.", 511);
                            v3_queue_event(ev);
                        }
                        break;
                }
                _v3_destroy_packet(msg);
            }
            _v3_func_leave("_v3_process_message");
            return V3_OK;/*}}}*/
        case 0x46:/*{{{*/
            if (!_v3_get_0x46(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                v3_user *u;
                _v3_lock_userlist();
                _v3_msg_0x46 *m = msg->contents;
                if ((u = _v3_get_user(m->user_id)) != NULL) {
                    switch (m->setting) {
                        case V3_USER_ACCEPT_PAGES:
                            _v3_debug(V3_DEBUG_INFO, "setting user %d as accepting pages = %d", m->user_id, m->value);
                            u->accept_pages = m->value;
                            break;
                        case V3_USER_ACCEPT_U2U:
                            _v3_debug(V3_DEBUG_INFO, "setting user %d as accepting u2u = %d", m->user_id, m->value);
                            u->accept_u2u = m->value;
                            break;
                        case V3_USER_ALLOW_RECORD:
                            _v3_debug(V3_DEBUG_INFO, "setting user %d as allowing recording = %d", m->user_id, m->value);
                            u->allow_recording = m->value;
                            break;
                        case V3_USER_ACCEPT_CHAT:
                            _v3_debug(V3_DEBUG_INFO, "setting user %d as accepting priv chat = %d", m->user_id, m->value);
                            u->accept_chat = m->value;
                            break;
                        case V3_USER_GLOBAL_MUTE:
                            {
                                _v3_debug(V3_DEBUG_INFO, "setting user %d as globally muted = %d", m->user_id, m->value);
                                u->global_mute = m->value;
                                v3_event *ev = _v3_create_event(V3_EVENT_USER_GLOBAL_MUTE_CHANGED);
                                ev->user.id = u->id;
                                _v3_debug(V3_DEBUG_INFO, "queuing event type %d for user %d", ev->type, ev->user.id);
                                v3_queue_event(ev);
                            }
                            break;
                        case V3_USER_CHANNEL_MUTE:
                            {
                                _v3_debug(V3_DEBUG_INFO, "setting user %d as channel muted = %d", m->user_id, m->value);
                                u->channel_mute = m->value;
                                v3_event *ev = _v3_create_event(V3_EVENT_USER_CHANNEL_MUTE_CHANGED);
                                ev->user.id = u->id;
                                _v3_debug(V3_DEBUG_INFO, "queuing event type %d for user %d", ev->type, ev->user.id);
                                v3_queue_event(ev);
                            }
                            break;
                        default:
                            _v3_debug(V3_DEBUG_INFO, "unknown setting for user %d setting: %d = %d", m->user_id, m->setting, m->value);
                            break;
                    }
                }
                _v3_unlock_userlist();
            }
            _v3_destroy_packet(msg);
            _v3_func_leave("_v3_process_message");
            return V3_OK;/*}}}*/
        case 0x49:/*{{{*/
            if (!_v3_get_0x49(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_msg_0x49 *m = msg->contents;

                _v3_lock_channellist();
                switch (m->subtype) {
                    case V3_REMOVE_CHANNEL:
                        {
                            _v3_debug(V3_DEBUG_INFO, "removing channel %d from list",  m->channel->id);
                            v3_event *ev = _v3_create_event(V3_EVENT_CHAN_REMOVE);
                            ev->channel.id = m->channel->id;
                            _v3_debug(V3_DEBUG_INFO, "queuing event type %d for channel %d", ev->type, ev->channel.id);
                            _v3_remove_channel(m->channel->id);
                            v3_queue_event(ev);
                        }
                        break;
                    case V3_MODIFY_CHANNEL:
                    case V3_ADD_CHANNEL:
                        {
                            _v3_debug(V3_DEBUG_INFO, "adding channel %d to list",  m->channel->id);
                            _v3_update_channel(m->channel);
                            v3_event *ev = _v3_create_event((m->subtype == V3_MODIFY_CHANNEL) ? V3_EVENT_CHAN_MODIFY : V3_EVENT_CHAN_ADD);
                            ev->channel.id = m->channel->id;
                            _v3_debug(V3_DEBUG_INFO, "queuing event type %d for channel %d", ev->type, ev->channel.id);
                            v3_queue_event(ev);
                        }
                        break;
                    case V3_AUTHFAIL_CHANNEL:
                        {
                            v3_event *ev = _v3_create_event(V3_EVENT_CHAN_BADPASS);
                            ev->channel.id = m->channel->id;
                            strncpy(ev->error.message, "Error switching to channel.  The password you entered is wrong or you do not have permission", 511);
                            v3_queue_event(ev);
                        }
                        break;
                }
                _v3_unlock_channellist();
            }
            // TODO: _v3_destroy_0x49(msg);
            _v3_destroy_packet(msg);
            _v3_func_leave("_v3_process_message");
            return V3_OK;/*}}}*/
        case 0x4a:/*{{{*/
            if (!_v3_get_0x4a(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_msg_0x4a *m = msg->contents;

                if (m->error_id) {
                    if (m->error_id != 10) {
                        char *error;

                        if (m->error_id > (sizeof(_v3_permissions_errors) / sizeof(_v3_permissions_errors[0])))
                            error = "Unknown";
                        else
                            error = _v3_permissions_errors[m->error_id - 1];

                        v3_event *ev = _v3_create_event(V3_EVENT_ERROR_MSG);
                        strncpy(ev->error.message, "User editor error:\n", sizeof(ev->error.message));
                        strncat(ev->error.message, error, sizeof(ev->error.message));
                        v3_queue_event(ev);
                    }

                    _v3_destroy_packet(msg);
                    _v3_func_leave("_v3_process_message");
                    return V3_OK;
                }

                switch (m->subtype) {
                    case V3_USERLIST_OPEN:
                    case V3_USERLIST_MODIFY:
                    case V3_USERLIST_ADD:
                        {
                        int i;
                        _v3_msg_0x4a_account *msub = msg->contents;
                        _v3_debug(V3_DEBUG_INFO, "received %d user accounts", msub->acct_list_count);
                        for (i = 0; i < msub->acct_list_count; i++) {
                            _v3_update_account(msub->acct_list[i]);
                            v3_event *ev = _v3_create_event((msub->header.subtype == V3_USERLIST_MODIFY) ? V3_EVENT_USERLIST_MODIFY : V3_EVENT_USERLIST_ADD);
                            ev->account.id = msub->acct_list[i]->perms.account_id;
                            _v3_debug(V3_DEBUG_INFO, "queuing event type %d for account id %d", ev->type, ev->account.id);
                            v3_queue_event(ev);
                        }
                        }
                        break;
                    case V3_USERLIST_REMOVE:
                        {
                        _v3_msg_0x4a_perms *msub = msg->contents;
                        _v3_debug(V3_DEBUG_INFO, "received remove account for %d", msub->perms.account_id);
                        _v3_remove_account(msub->perms.account_id);

                        v3_event *ev = _v3_create_event(V3_EVENT_USERLIST_REMOVE);
                        ev->account.id = msub->perms.account_id;
                        _v3_debug(V3_DEBUG_INFO, "queuing event type %d for account id %d", ev->type, ev->account.id);
                        v3_queue_event(ev);
                        }
                        break;
                    case V3_USERLIST_LUSER:
                        {
                        _v3_msg_0x4a_perms *msub = msg->contents;
                        _v3_debug(V3_DEBUG_INFO, "received local user permissions");
                        _v3_print_permissions(&msub->perms);

                        _v3_lock_luser();
                        v3_luser.perms = msub->perms;
                        _v3_unlock_luser();
                        v3_event *ev = _v3_create_event(V3_EVENT_PERMS_UPDATED);
                        v3_queue_event(ev);
                        }
                        break;
                    case V3_USERLIST_CHANGE_OWNER:
                        {
                        v3_account *a;
                        char *old_owner, *new_owner = NULL;
                        _v3_msg_0x4a_perms *msub = msg->contents;

                        _v3_debug(V3_DEBUG_INFO, "received change owner (%d => %d)", msub->perms.account_id, msub->perms.replace_owner_id);

                        a = v3_get_account(msub->perms.account_id);
                        if (a == NULL) {
                            _v3_debug(V3_DEBUG_INFO, "can't find account id %d", msub->perms.account_id);
                            break;
                        }
                        old_owner = strdup(a->username);
                        v3_free_account(a);

                        a = v3_get_account(msub->perms.replace_owner_id);
                        if (new_owner == NULL) {
                            _v3_debug(V3_DEBUG_INFO, "can't find account id %d", msub->perms.replace_owner_id);
                            free(old_owner);
                            break;
                        }
                        new_owner = strdup(a->username);
                        v3_free_account(a);

                        _v3_lock_accountlist();
                        for (a = v3_account_list; a != NULL; a = a->next) {
                            if (strcmp(a->owner, old_owner))
                                continue;

                            free(a->owner);
                            a->owner = strdup(new_owner);

                            v3_event *ev = _v3_create_event(V3_EVENT_USERLIST_MODIFY);
                            ev->account.id = a->perms.account_id;
                            _v3_debug(V3_DEBUG_INFO, "queuing event type %d for account id %d", ev->type, ev->account.id);
                            v3_queue_event(ev);
                        }
                        _v3_unlock_accountlist();

                        free(old_owner);
                        free(new_owner);
                        }
                        break;
                    default:
                        break;
                }
            }
            _v3_destroy_0x4a(msg);
            _v3_destroy_packet(msg);
            _v3_func_leave("_v3_process_message");
            return V3_OK;/*}}}*/
        case 0x4c:/*{{{*/
            if (!_v3_get_0x4c(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_msg_0x4c *m = msg->contents;
                _v3_net_message *response;
                static v3_server_prop prop;
                _v3_lock_server();
                switch (m->subtype) {
                    case V3_SERVER_RECV_SETTING:
                        switch (m->property) {
                            case V3_SRV_PROP_RECV_INIT:
                                memset(&prop, 0, sizeof(v3_server_prop));
                                break;
                            case V3_SRV_PROP_RECV_START:
                                break;
                            case V3_SRV_PROP_CHAT_FILTER:
                                prop.chat_filter = atoi(m->value);
                                break;
                            case V3_SRV_PROP_CHAN_SORT:
                                prop.channel_order = atoi(m->value);
                                break;
                            case V3_SRV_PROP_MOTD_ALWAYS:
                                prop.motd_display = atoi(m->value);
                                break;
                            case V3_SRV_PROP_CHAT_SPAM_FILT:
                                _v3_parse_filter(&prop.chat_spam_filter, m->value);
                                break;
                            case V3_SRV_PROP_COMMENT_SPAM_FILT:
                                _v3_parse_filter(&prop.comment_spam_filter, m->value);
                                break;
                            case V3_SRV_PROP_WAVE_SPAM_FILT:
                                _v3_parse_filter(&prop.wave_spam_filter, m->value);
                                break;
                            case V3_SRV_PROP_TTS_SPAM_FILT:
                                _v3_parse_filter(&prop.tts_spam_filter, m->value);
                                break;
                            case V3_SRV_PROP_INACTIVE_TIMEO:
                                prop.inactivity_timeout = atoi(m->value);;
                                break;
                            case V3_SRV_PROP_INACTIVE_ACTION:
                                prop.inactivity_action = atoi(m->value);;
                                break;
                            case V3_SRV_PROP_INACTIVE_CHAN:
                                strncpy(prop.inactivity_channel, m->value, 1023);
                                break;
                            case V3_SRV_PROP_REM_SRV_COMMENT:
                                prop.rem_srv_comment = atoi(m->value);
                                break;
                            case V3_SRV_PROP_REM_CHAN_NAMES:
                                prop.rem_chan_names = atoi(m->value);
                                break;
                            case V3_SRV_PROP_REM_CHAN_COMMENTS:
                                prop.rem_chan_comments = atoi(m->value);
                                break;
                            case V3_SRV_PROP_REM_USER_NAMES:
                                prop.rem_user_names = atoi(m->value);
                                break;
                            case V3_SRV_PROP_REM_USER_COMMENTS:
                                prop.rem_user_comments = atoi(m->value);
                                break;
                            case V3_SRV_PROP_CHKPOINT:
                                break;
                            case V3_SRV_PROP_WAVE_BIND_FILT:
                                prop.wave_bind_filter = atoi(m->value);
                                break;
                            case V3_SRV_PROP_TTS_BIND_FILT:
                                prop.tts_bind_filter = atoi(m->value);
                                break;
                            case V3_SRV_PROP_CHAN_SPAM_FILT:
                                _v3_parse_filter(&prop.channel_spam_filter, m->value);
                                break;
                            case V3_SRV_PROP_REM_SHOW_LOGIN:
                                prop.rem_show_login_names = atoi(m->value);
                                break;
                            case V3_SRV_PROP_MAX_GUEST_LOGIN:
                                prop.max_guest = atoi(m->value);
                                break;
                            case V3_SRV_PROP_AUTOKICK_TIME:
                                prop.autokick_len = atoi(m->value);
                                break;
                            case V3_SRV_PROP_AUTOBAN_TIME:
                                prop.autoban_len = atoi(m->value);
                                break;
                            case V3_SRV_PROP_RECV_DONE:
                                break;
                        }
                        if (m->property != V3_SRV_PROP_RECV_DONE) {
                            _v3_debug(V3_DEBUG_INFO, "settings recv 0x%02X: %s", m->property, m->value);
                            // store the setting somewhere

                            if  (m->property == 0x03 || m->property == 0x05 || m->property == 0x16) {
                                m->property++;
                            }
                            response = _v3_put_0x4c(V3_SERVER_RECV_SETTING, m->property + 1, 0x00, 0x42, NULL);
                            _v3_send(response);
                            _v3_destroy_packet(response);
                        } else {
                            _v3_debug(V3_DEBUG_INFO, "chat_filter...................: %d", prop.chat_filter);
                            _v3_debug(V3_DEBUG_INFO, "channel_order.................: %d", prop.channel_order);
                            _v3_debug(V3_DEBUG_INFO, "motd_display..................: %d", prop.motd_display);
                            _v3_debug(V3_DEBUG_INFO, "chat spam filt................: %d, %d, %d", prop.chat_spam_filter.action, prop.chat_spam_filter.interval, prop.chat_spam_filter.times);
                            _v3_debug(V3_DEBUG_INFO, "comment spam filt.............: %d, %d, %d", prop.comment_spam_filter.action, prop.comment_spam_filter.interval, prop.comment_spam_filter.times);
                            _v3_debug(V3_DEBUG_INFO, "wave spam filt................: %d, %d, %d", prop.wave_spam_filter.action, prop.wave_spam_filter.interval, prop.wave_spam_filter.times);
                            _v3_debug(V3_DEBUG_INFO, "tts spam filt.................: %d, %d, %d", prop.tts_spam_filter.action, prop.tts_spam_filter.interval, prop.tts_spam_filter.times);
                            _v3_debug(V3_DEBUG_INFO, "inactive timeout..............: %d",  prop.inactivity_timeout);
                            _v3_debug(V3_DEBUG_INFO, "inactive action...............: %d", prop.inactivity_action);
                            _v3_debug(V3_DEBUG_INFO, "inactive chan.................: %d", prop.inactivity_channel[1024]); // TODO: this sucks, we should probably resolve the channel path in lv3
                            _v3_debug(V3_DEBUG_INFO, "rem srv comments..............: %d", prop.rem_srv_comment);
                            _v3_debug(V3_DEBUG_INFO, "rem chan names................: %d", prop.rem_chan_names);
                            _v3_debug(V3_DEBUG_INFO, "rem chan comments.............: %d", prop.rem_chan_comments);
                            _v3_debug(V3_DEBUG_INFO, "rem user names................: %d", prop.rem_user_names);
                            _v3_debug(V3_DEBUG_INFO, "rem user comments.............: %d", prop.rem_user_comments);
                            _v3_debug(V3_DEBUG_INFO, "wave bind filt................: %d", prop.wave_bind_filter);
                            _v3_debug(V3_DEBUG_INFO, "tts bind filt.................: %d", prop.tts_bind_filter);
                            _v3_debug(V3_DEBUG_INFO, "channel spam filt.............: %d, %d, %d", prop.channel_spam_filter.action, prop.channel_spam_filter.interval, prop.channel_spam_filter.times);
                            _v3_debug(V3_DEBUG_INFO, "rem show login names..........: %d", prop.rem_show_login_names);
                            _v3_debug(V3_DEBUG_INFO, "max guest.....................: %d", prop.max_guest);
                            _v3_debug(V3_DEBUG_INFO, "autokick len..................: %d",  prop.autokick_len);
                            _v3_debug(V3_DEBUG_INFO, "autoban len...................: %d",  prop.autoban_len);

                            v3_event *ev = _v3_create_event(V3_EVENT_RECV_SRV_PROP);
                            memcpy(&ev->data->srvprop, &prop, sizeof(v3_server_prop));
                            v3_queue_event(ev);
                        }
                        break;
                    case V3_SERVER_CLIENT_SET:
                        switch (m->property) {
                            case V3_SRV_PROP_CHAT_FILTER:
                                v3_server.per_channel_chat = atoi(m->value);
                                break;
                            case V3_SRV_PROP_CHAN_SORT:
                                v3_server.channel_manual_sort = atoi(m->value);
                                break;
                            case V3_SRV_PROP_MOTD_ALWAYS:
                                v3_server.motd_always = atoi(m->value);
                                break;
                        }
                        v3_event *ev = _v3_create_event(V3_EVENT_SERVER_PROPERTY_UPDATED);
                        ev->serverproperty.property = m->property;
                        ev->serverproperty.value = atoi(m->value);
                        _v3_debug(V3_DEBUG_INFO, "queuing event type %d for property %d and value %d", ev->type, ev->serverproperty.property, ev->serverproperty.value);
                        v3_queue_event(ev);
                        break;
                }

            }

            _v3_func_leave("_v3_process_message");
            _v3_unlock_server();
            _v3_destroy_packet(msg);
            return V3_OK;/*}}}*/
        case 0x50:/*{{{*/
            if (!_v3_get_0x50(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_msg_0x50 *m = msg->contents;
                v3_user *u;
                uint8_t guest = 0;
                char **motd;
                int size = 0;

                _v3_lock_server();
                if(m->message_id + 1 > m->message_num) {
                    _v3_debug(V3_DEBUG_PACKET_PARSE, "received %d packet but max packets is %d", m->message_id, m->message_num);
                    _v3_func_leave("_v3_get_0x50");
                    _v3_unlock_server();
                    return false;
                }
                if (m->guest_motd_flag == 1 && &v3_server.guest_motd != NULL) {
                    motd = &v3_server.guest_motd;
                } else {
                    motd = &v3_server.motd;
                }
                if(m->message_id == 0) {
                    if (*motd != NULL) {
                        free(*motd);
                    }
                    *motd = malloc(m->message_size + 1);
                    memset(*motd, 0, m->message_size + 1);
                    memcpy(*motd, m->message, m->message_size);
                } else {
                    size = strlen(*motd);
                    *motd = realloc(*motd, size + m->message_size + 1);
                    memset(*motd + size, 0, m->message_size + 1);
                    memcpy(*motd + size, m->message, m->message_size);
                }
                if ((u = v3_get_user(v3_get_user_id()))) {
                    guest = u->guest;
                } else {
                    // this should never happen... but just in case....
                    guest = false;
                }
                if ((m->message_id+1) == m->message_num) {
                    // At this point we have our motd, may want to notify the user here :)
                    v3_event *ev = _v3_create_event(V3_EVENT_DISPLAY_MOTD);
                    strncpy(ev->data->motd, *motd, 2047);
                    v3_queue_event(ev);
                }
                _v3_destroy_packet(msg);
                _v3_unlock_server();
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            }
            return V3_OK;/*}}}*/
        case 0x52:/*{{{*/
            if (!_v3_get_0x52(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_msg_0x52_0x01_in *m = (_v3_msg_0x52_0x01_in *)msg->contents;
                v3_event *ev = _v3_create_event(0);
                switch (m->header.subtype) {
                    case V3_AUDIO_START:
                    case 0x04:
                        {
                            ev->type = V3_EVENT_USER_TALK_START;
                            ev->user.id = m->header.user_id;
                            ev->pcm.send_type = m->header.send_type;
                            ev->pcm.rate = v3_get_codec_rate(m->header.codec, m->header.codec_format);
                            _v3_lock_userlist();
                            _v3_get_user(m->header.user_id)->is_transmitting = true;
                            _v3_unlock_userlist();
                        }
                        break;
                    case V3_AUDIO_STOP:
                        {
                            ev->type = V3_EVENT_USER_TALK_END;
                            ev->user.id = m->header.user_id;
                            _v3_lock_userlist();
                            _v3_get_user(m->header.user_id)->is_transmitting = false;
                            _v3_unlock_userlist();
                            _v3_vrf_record_event(V3_VRF_EVENT_AUDIO_STOP, m->header.user_id, -1, -1, 0, 0, NULL);
                        }
                        break;
                    case V3_AUDIO_DATA:
                        {
                            const v3_codec *codec = v3_get_codec(m->header.codec, m->header.codec_format);
                            ev->type = V3_EVENT_PLAY_AUDIO;
                            ev->user.id = m->header.user_id;
                            ev->pcm.send_type = m->header.send_type;
                            ev->pcm.rate = codec->rate;
                            ev->pcm.channels = (m->header.pcm_length - 2000 == 2) ? 2 : 1;
                            ev->pcm.length = sizeof(ev->data->sample);
                            int ret;

                            _v3_vrf_record_event(
                                    V3_VRF_EVENT_AUDIO_DATA,
                                    m->header.user_id,
                                    m->header.codec,
                                    m->header.codec_format,
                                    m->header.pcm_length,
                                    m->header.data_length,
                                    m->data);
                            if ((ret = _v3_audio_decode(
                                            /* encoded input */
                                            codec,
                                            &v3_decoders[m->header.user_id],
                                            m->data,
                                            m->header.data_length,
                                            /* pcm output */
                                            (void *)&ev->data->sample,
                                            &ev->pcm.length,
                                            /* optional args */
                                            ev->pcm.channels)) != V3_OK) {
                                _v3_destroy_0x52(msg);
                                _v3_destroy_packet(msg);
                                free(ev);
                                _v3_func_leave("_v3_process_message");
                                return ret;
                            }
                            // don't waste resources if we don't need to deal with it
                            static const int16_t maxsample = 0x7fff;
                            static const int16_t minsample = 0x7fff + 1;
                            register float tmpsample = 0;
                            int ctr;

                            if (_v3_master_volume != 79) {
                                float multiplier = tan(_v3_master_volume/100.0);
                                _v3_debug(V3_DEBUG_INFO, "master: amplifying to level %d (%3.10f multiplier)", _v3_master_volume, multiplier);
                                for (ctr = 0; ctr < ev->pcm.length / 2; ctr++) {
                                    tmpsample = ev->data->sample16[ctr];
                                    tmpsample *= multiplier;
                                    ev->data->sample16[ctr] = (tmpsample > maxsample)
                                        ? maxsample
                                        : ((tmpsample < minsample) ? minsample : tmpsample);
                                }
                            }
                            if (_v3_user_volumes[ev->user.id] != 79) {
                                float multiplier = tan(_v3_user_volumes[ev->user.id]/100.0);
                                _v3_debug(V3_DEBUG_INFO, "user: amplifying to level %d (%3.10f multiplier)", _v3_user_volumes[ev->user.id], multiplier);
                                for (ctr = 0; ctr < ev->pcm.length / 2; ctr++) {
                                    tmpsample = ev->data->sample16[ctr];
                                    tmpsample *= multiplier;
                                    ev->data->sample16[ctr] = (tmpsample > maxsample)
                                        ? maxsample
                                        : ((tmpsample < minsample) ? minsample : tmpsample);
                                }
                            }
                        }
                        break;
                }
                _v3_debug(V3_DEBUG_EVENT, "queueing pcm msg length %d", ev->pcm.length);
                v3_queue_event(ev);
                _v3_destroy_0x52(msg);
            }
            _v3_destroy_packet(msg);
            _v3_func_leave("_v3_process_message");
            return V3_OK;/*}}}*/
        case 0x53:/*{{{*/
            /*
             *  This is almost identical to 0x3b, so whatever you do here probably
             *  needs to be done there, too.
             */
            if (!_v3_get_0x53(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_msg_0x53 *m = (_v3_msg_0x53 *)msg->contents;

                v3_user *user;
                _v3_debug(V3_DEBUG_INFO, "user %d moved to channel %d", m->user_id, m->channel_id);
                user = v3_get_user(m->user_id);
                if (user) {
                    user->channel = m->channel_id;
                    _v3_update_user(user);
                    v3_free_user(user);
                    v3_event *ev = _v3_create_event(V3_EVENT_USER_CHAN_MOVE);
                    ev->user.id = m->user_id;
                    ev->channel.id = m->channel_id;
                    v3_queue_event(ev);
                    _v3_vrf_record_event(
                            (m->user_id == v3_get_user_id())
                                ? V3_VRF_EVENT_DATA_FLUSH
                                : V3_VRF_EVENT_AUDIO_STOP,
                            m->user_id, -1, -1, 0, 0, NULL);
                }
            }
            _v3_destroy_packet(msg);
            _v3_func_leave("_v3_process_message");
            return V3_OK;/*}}}*/
        case 0x57:/*{{{*/
            if (! _v3_get_0x57(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_lock_server();
                _v3_msg_0x57 *m = msg->contents;
                free(v3_server.name);
                free(v3_server.version);
                v3_server.name = strdup(m->name);
                v3_server.version = strdup(m->version);
                v3_server.max_clients = m->max_clients;
                v3_server.is_licensed = m->is_licensed;
                v3_server.connected_clients = m->connected_clients;
                v3_server.port = m->port;
                _v3_unlock_server();
            }
            _v3_destroy_packet(msg);
            _v3_func_leave("_v3_process_message");
            return V3_OK;/*}}}*/
        case 0x58:/*{{{*/
            if (!_v3_get_0x58(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_msg_0x58 *m = msg->contents;

                if (m->error_id) {
                    char *error;

                    if (m->error_id > (sizeof(_v3_phantom_errors) / sizeof(_v3_phantom_errors[0])))
                        error = "Unknown";
                    else
                        error = _v3_phantom_errors[m->error_id - 1];

                    v3_event *ev = _v3_create_event(V3_EVENT_ERROR_MSG);
                    strncpy(ev->error.message, "Phantom error:\n", sizeof(ev->error.message));
                    strncat(ev->error.message, error, sizeof(ev->error.message));
                    v3_queue_event(ev);
                } else {
                    v3_user new_phantom_user = {0};
                    v3_event *ev;

                    switch (m->subtype) {
                        case V3_PHANTOM_ADD:
                            new_phantom_user.id = m->phantom_user_id;
                            new_phantom_user.channel = m->channel_id;
                            new_phantom_user.name = v3_get_user(m->real_user_id)->name;
                            new_phantom_user.comment = "";
                            new_phantom_user.phonetic = "";
                            new_phantom_user.integration_text = "";
                            new_phantom_user.url = "";
                            new_phantom_user.real_user_id = m->real_user_id;

                            _v3_lock_userlist();
                            _v3_update_user(&new_phantom_user);
                            _v3_unlock_userlist();

                            ev = _v3_create_event(V3_EVENT_USER_LOGIN);
                            ev->user.id = new_phantom_user.id;
                            ev->channel.id = new_phantom_user.channel;
                            _v3_debug(V3_DEBUG_INFO, "queuing event type %d for user %d", ev->type, ev->user.id);
                            v3_queue_event(ev);
                            break;

                        case V3_PHANTOM_REMOVE:
                            _v3_lock_userlist();
                            _v3_remove_user(m->phantom_user_id);
                            _v3_unlock_userlist();

                            ev = _v3_create_event(V3_EVENT_USER_LOGOUT);
                            ev->user.id = m->phantom_user_id;
                            ev->channel.id = m->channel_id;
                            _v3_debug(V3_DEBUG_INFO, "queuing event type %d for user %d", ev->type, ev->user.id);

                            v3_queue_event(ev);
                            break;

                        default:
                            _v3_error("bad phantom subtype");
                            _v3_destroy_packet(msg);
                            _v3_func_leave("_v3_process_message");
                            return V3_MALFORMED;
                    }

                }
            }
            _v3_destroy_packet(msg);
            _v3_func_leave("_v3_process_message");
            return V3_OK;/*}}}*/
        case 0x59:/*{{{*/
            if (!_v3_get_0x59(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                char buf[1024];
                _v3_msg_0x59 *m = msg->contents;
                snprintf(buf, 1024, "%s%s", _v3_errors[m->error], m->message);

                if (m->minutes_banned) {
                    char buf2[1024];
                    snprintf(buf2, 1024, " You can connect again in %d minutes", m->minutes_banned);
                    strncat(buf, buf2, 1023);
                }

                _v3_error("%s", buf);

                if (v3_is_loggedin()) {
                    v3_event *ev = _v3_create_event(V3_EVENT_ERROR_MSG);
                    ev->error.disconnected = m->close_connection;
                    strncpy(ev->error.message, buf, 512);
                    v3_queue_event(ev);
                }

                if (m->close_connection) {
                    _v3_debug(V3_DEBUG_INTERNAL, "disconnecting from server");
                    _v3_logout();
                }
            }
            _v3_destroy_packet(msg);
            _v3_func_leave("_v3_process_message");
            return V3_OK;/*}}}*/
        case 0x5a:/*{{{*/
            if(!_v3_get_0x5a(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_msg_0x5a *m = msg->contents;
                switch(m->subtype) {
                    case V3_START_PRIV_CHAT:
                        {
                            // for some reason, the server responds to us to tell us that we started chat?
                            // this condition checks to make sure a remote user has requested chat with us
                            if (m->user2 != v3_get_user_id()) {
                                v3_event *ev = _v3_create_event(V3_EVENT_PRIVATE_CHAT_START);
                                ev->user.privchat_user1 = m->user1;
                                ev->user.privchat_user2 = m->user2;
                                _v3_debug(V3_DEBUG_INFO, "received chat start from user id %d", m->user2);
                                v3_queue_event(ev);
                            }
                        }
                        break;
                    case V3_END_PRIV_CHAT:
                        {
                            v3_event *ev = _v3_create_event(V3_EVENT_PRIVATE_CHAT_END);
                            ev->user.privchat_user1 = m->user1;
                            ev->user.privchat_user2 = m->user2;
                            _v3_debug(V3_DEBUG_INFO, "received chat end from user id %d", m->user2);
                            v3_queue_event(ev);
                        }
                        break;
                    case V3_TALK_PRIV_CHAT:
                        {
                            v3_event *ev = _v3_create_event(V3_EVENT_PRIVATE_CHAT_MESSAGE);
                            ev->user.privchat_user1 = m->user1;
                            ev->user.privchat_user2 = m->user2;
                            ev->flags = m->error;
                            strncpy(ev->data->chatmessage, m->msg, sizeof(ev->data->chatmessage) - 1);
                            _v3_debug(V3_DEBUG_INFO, "received chat message from user id %d: %s", m->user2, m->msg);
                            free(m->msg);
                            v3_queue_event(ev);
                        }
                        break;
                    case V3_BACK_PRIV_CHAT:
                        {
                            v3_event *ev = _v3_create_event(V3_EVENT_PRIVATE_CHAT_BACK);
                            ev->user.privchat_user1 = m->user1;
                            ev->user.privchat_user2 = m->user2;
                            _v3_debug(V3_DEBUG_INFO, "received chat back from user id %d");
                            v3_queue_event(ev);
                        }
                        break;
                    case V3_AWAY_PRIV_CHAT:
                        {
                            v3_event *ev = _v3_create_event(V3_EVENT_PRIVATE_CHAT_AWAY);
                            ev->user.privchat_user1 = m->user1;
                            ev->user.privchat_user2 = m->user2;
                            _v3_debug(V3_DEBUG_INFO, "received chat away from user id %d");
                            v3_queue_event(ev);
                        }
                        break;
                    default:
                        _v3_debug(V3_DEBUG_INFO, "received unknown subtype %d", m->subtype);
                        break;
                }
                _v3_destroy_packet(msg);
            }
            _v3_func_leave("_v3_process_message");
            return V3_OK;/*}}}*/
        case 0x5c:/*{{{*/
            if (!_v3_get_0x5c(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                if (v3_is_loggedin()) {
                    _v3_msg_0x5c *m = msg->contents;
                    _v3_net_message *response;
                    response = _v3_put_0x5c(m->subtype);
                    _v3_send(response);
                    _v3_destroy_packet(response);
                }
            }
            _v3_destroy_packet(msg);
            _v3_func_leave("_v3_process_message");
            return V3_OK;/*}}}*/
        case 0x5d:/*{{{*/
            if (!_v3_get_0x5d(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_msg_0x5d *m = msg->contents;
                v3_user *ul, *u;
                int ctr;

                _v3_lock_luser();
                _v3_lock_userlist();

                ul = calloc(m->user_count, sizeof(v3_user));
                switch (m->subtype) {
                    case V3_REMOVE_USER:
                        _v3_debug(V3_DEBUG_INFO, "removing %d users from user list",  m->user_count);
                        for (ctr = 0; ctr < m->user_count; ctr++) {
                            if (_v3_get_user(m->user_list[ctr].id)->is_transmitting) {
                                _v3_vrf_record_event(V3_VRF_EVENT_AUDIO_STOP, m->user_list[ctr].id, -1, -1, 0, 0, NULL);
                            }
                            v3_event *ev = _v3_create_event(V3_EVENT_USER_LOGOUT);
                            ev->user.id = m->user_list[ctr].id;
                            ev->channel.id = m->user_list[ctr].channel;
                            _v3_debug(V3_DEBUG_INFO, "queuing event type %d for user %d", ev->type, ev->user.id);
                            _v3_remove_user(m->user_list[ctr].id);
                            v3_queue_event(ev);

                            // disconnect all user's phantoms
                            for (u = v3_user_list; u != NULL; u = u->next) {
                                if (u->real_user_id == m->user_list[ctr].id) {
                                    v3_event *ev = _v3_create_event(V3_EVENT_USER_LOGOUT);
                                    ev->user.id = u->id;
                                    ev->channel.id = u->channel;
                                    _v3_debug(V3_DEBUG_INFO, "queuing event type %d for user %d", ev->type, ev->user.id);
                                    _v3_remove_user(u->id);
                                    v3_queue_event(ev);
                                }
                            }
                        }
                        break;
                    case V3_MODIFY_USER:
                    case V3_ADD_USER:
                        _v3_debug(V3_DEBUG_INFO, "adding %d users on user list",  m->user_count);
                        for (ctr = 0; ctr < m->user_count; ctr++) {
                            _v3_update_user(&m->user_list[ctr]);
                            v3_event *ev = _v3_create_event(m->subtype == V3_ADD_USER ? V3_EVENT_USER_LOGIN : V3_EVENT_USER_MODIFY);
                            ev->user.id = m->user_list[ctr].id;
                            ev->channel.id = m->user_list[ctr].channel;
                            _v3_debug(V3_DEBUG_INFO, "queuing event type %d for user %d", ev->type, ev->user.id);
                            v3_queue_event(ev);
                        }
                        break;
                    case V3_USER_LIST:
                        // <strke>user 1 is always ourself in this subtype</strike>
                        // TODO: this is a bad assumption... the userlist can span multiple 0x5d packets
                        if (v3_luser.id == -1) {
                            v3_luser.id = m->user_list[1].id;
                            if (v3_luser.name) {
                                free(v3_luser.name);
                            }
                            v3_luser.name = strdup(m->user_list[1].name);
                            if (v3_luser.phonetic) {
                                free(v3_luser.phonetic);
                            }
                            v3_luser.phonetic = strdup(m->user_list[1].phonetic);
                            if (v3_luser.comment) {
                                free(v3_luser.comment);
                            }
                            v3_luser.comment = strdup(m->user_list[1].comment);
                            if (v3_luser.integration_text) {
                                free(v3_luser.integration_text);
                            }
                            v3_luser.integration_text = strdup(m->user_list[1].integration_text);
                            if (v3_luser.url) {
                                free(v3_luser.url);
                            }
                            v3_luser.url = strdup(m->user_list[1].url);
                            _v3_debug(V3_DEBUG_INTERNAL, "found myself: id: %d | chan: %d | name: %s | phonetic: %s | comment: %s | int: %s | url: %s",
                                    m->user_list[1].id,
                                    m->user_list[1].channel,
                                    m->user_list[1].name,
                                    m->user_list[1].phonetic,
                                    m->user_list[1].comment,
                                    m->user_list[1].integration_text,
                                    m->user_list[1].url
                                    );
                        }
                        _v3_debug(V3_DEBUG_INFO, "adding %d users to user list",  m->user_count);
                        for (ctr = 0; ctr < m->user_count; ctr++) {
                            _v3_update_user(&m->user_list[ctr]);
                            v3_event *ev = _v3_create_event(V3_EVENT_USER_LOGIN);
                            ev->user.id = m->user_list[ctr].id;
                            ev->channel.id = m->user_list[ctr].channel;
                            ev->flags |= V3_LOGIN_FLAGS_EXISTING;
                            _v3_debug(V3_DEBUG_INFO, "queuing event type %d for user %d to channel %d", ev->type, ev->user.id, ev->channel.id);
                            v3_queue_event(ev);
                        }
                        break;
                    case V3_RANKCHANGE_USER:
                        _v3_debug(V3_DEBUG_INFO, "changing rank for %d users on user list",  m->user_count);
                        {
                            v3_user *u;
                            for (ctr = 0; ctr < m->user_count; ctr++) {
                                if ((u = _v3_get_user(m->user_list[ctr].id))) {
                                    u->rank_id = m->user_list[ctr].rank_id;
                                }
                                v3_event *ev = _v3_create_event(V3_EVENT_USER_RANK_CHANGE);
                                ev->user.id = m->user_list[ctr].id;
                                _v3_debug(V3_DEBUG_INFO, "queuing event type %d for user %d", ev->type, ev->user.id);
                                v3_queue_event(ev);
                            }
                        }
                        break;
                }
                free(ul);
                _v3_unlock_userlist();
                _v3_unlock_luser();
                // TODO: send user list updated event
                _v3_debug(V3_DEBUG_INTERNAL, "do something with this message");
            }
            _v3_destroy_0x5d(msg);
            _v3_destroy_packet(msg);
            _v3_func_leave("_v3_process_message");
            return V3_OK;/*}}}*/
        case 0x60:/*{{{*/
            if (!_v3_get_0x60(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_msg_0x60 *m = (_v3_msg_0x60 *)msg->contents;
                v3_channel *cl;
                int ctr;

                _v3_lock_channellist();
                _v3_debug(V3_DEBUG_INFO, "adding %d channels from channel list",  m->channel_count);
                cl = calloc(m->channel_count, sizeof(v3_channel));
                for (ctr = 0; ctr < m->channel_count; ctr++) {
                    _v3_update_channel(&m->channel_list[ctr]);
                    v3_event *ev = _v3_create_event(V3_EVENT_CHAN_ADD);
                    ev->channel.id = m->channel_list[ctr].id;
                    _v3_debug(V3_DEBUG_INFO, "queuing event type %d for channel %d", ev->type, ev->channel.id);
                    v3_queue_event(ev);
                }
                free(cl);
                _v3_unlock_channellist();
                _v3_destroy_0x60(msg);
            }
            _v3_destroy_packet(msg);
            _v3_func_leave("_v3_process_message");
            return V3_OK;/*}}}*/
        default:
            _v3_debug(V3_DEBUG_NOTICE, "warning: unimplemented packet type: 0x%02X", msg->type);
            _v3_destroy_packet(msg);
            _v3_func_leave("_v3_process_message");
            return V3_NOTIMPL;
    }
}/*}}}*/

/*
 * These are functions that are available via the API for applications to use.
 */
int
v3_login(char *server, char *username, char *password, char *phonetic) {/*{{{*/
    char *srvname = NULL;
    char *srvport = NULL;
    char *sep = NULL;
    struct in_addr srvip;
    struct timeval tv;
    int type;
    _v3_net_message *msg;

    _v3_func_enter("v3_login");
    if (v3_server.handshake_key != NULL) {
        _v3_debug(V3_DEBUG_INTERNAL, "freeing up resources from previous login");
        v3_server.port = 0;
        v3_server.max_clients = 0;
        v3_server.recv_byte_count = 0;
        v3_server.recv_packet_count = 0;
        v3_server.sent_byte_count = 0;
        v3_server.sent_packet_count = 0;
        v3_server.connected_clients = 0;
        free(v3_server.name);
        free(v3_server.version);
        free(v3_server.os);
        free(v3_server.handshake_key);
        free(v3_server.handshake);
        free(v3_server.motd);
        free(v3_server.guest_motd);
        v3_server.name          = NULL;
        v3_server.version       = NULL;
        v3_server.os            = NULL;
        v3_server.handshake_key = NULL;
        v3_server.handshake     = NULL;
        v3_server.motd          = NULL;
        v3_server.guest_motd    = NULL;
    }
    gettimeofday(&tv, NULL);
    srand(tv.tv_usec);

    if (strlen(username) >= 32) {
        _v3_error("Login Failed: Username is too long (max is 32 characters).");
        _v3_func_leave("v3_login");
        return false;
    }
    if (strlen(phonetic) >= 32) {
        _v3_error("Login Failed: Phonetic is too long (max is 32 characters).");
        _v3_func_leave("v3_login");
        return false;
    }
    srvname = strdup(server);
    if ((sep = strchr(srvname, ':')) == NULL) {
        _v3_error("v3_login: invalid server name format.  expected: 'name:port'");
        _v3_func_leave("v3_login");
        return false;
    }
    *sep = '\0';
    srvport = sep+1;
    _v3_debug(V3_DEBUG_INTERNAL, "parsed server name: %s", srvname);
    _v3_debug(V3_DEBUG_INTERNAL, "parsed server port: %s", srvport);
    /*
       If the supplied server name  is not an IP address, perform a DNS lookup
     */
    if (! inet_aton(srvname, &srvip)) {
        struct hostent *hp;
        int res = 0;
#ifdef HAVE_GETHOSTBYNAME_R
        struct hostent hostbuf;
        size_t hstbuflen;
        char *tmphstbuf;
        int herr;

        _v3_status(5, "Looking up hostname for %s", srvname);
        hstbuflen = 1024;
        tmphstbuf = malloc (hstbuflen);

        while ((res = gethostbyname_r (srvname, &hostbuf, tmphstbuf, hstbuflen, &hp, &herr)) == ERANGE) {
            /* Enlarge the buffer.  */
            hstbuflen *= 2;
            tmphstbuf = realloc (tmphstbuf, hstbuflen);
        }
        free(tmphstbuf);
#else
        // if gethostbyname_r does not exist, assume that the gethostbyname is re-entrant
        hp = gethostbyname (srvname);
#endif
        if (res || hp == NULL || hp->h_length < 1) {
            _v3_error("Hostname lookup failed.");
            _v3_func_leave("v3_login");
            return false;
        }
        memcpy(&srvip.s_addr, hp->h_addr_list[0], sizeof(srvip.s_addr));
        _v3_debug(V3_DEBUG_INTERNAL, "found host: %s", inet_ntoa(srvip));
    }
    /*
     * Intialize the server and user structures;
     */
    v3_server.ip = srvip.s_addr;
    v3_server.port = atoi(srvport);
    v3_luser.name = strdup(username);
    v3_luser.password = strdup(password);
    v3_luser.phonetic = strdup(phonetic);
    _v3_init_decoders();
    if (pipe(v3_server.evpipe)) {
        _v3_error("could not create outbound event queue");
        _v3_func_leave("v3_login");
        return false;
    }
    v3_server.evinstream = fdopen(v3_server.evpipe[0], "r");
    v3_server.evoutstream = fdopen(v3_server.evpipe[1], "w");
    free(srvname);
    /*
       Call home and verify the server's license
     */
    _v3_status(10, "Checking server license.");
    if (! _v3_server_auth(&srvip, (uint16_t) v3_server.port)) {
        _v3_status(0, "Not connected.");
        _v3_func_leave("v3_login");
        return false;
    }

    /*
       Create a TCP connection to the server
     */
    if (!_v3_login_connect(&srvip, (uint16_t) v3_server.port)) {
        _v3_status(0, "Not connected.");
        _v3_func_leave("v3_login");
        return false;
    }
    _v3_status(20, "Connected to %s... exchanging encryption keys", inet_ntoa(srvip));
    gettimeofday(&v3_server.last_timestamp, NULL);

    /*
       Initiate the encryption handshake by handing the server our randomly generated key.
     */
    msg = _v3_put_0x00();
    ventrilo_first_enc((uint8_t *)msg->data, msg->len);
    _v3_send_enc_msg(msg->data, msg->len);
    _v3_destroy_packet(msg);

    /*
       Grab server information.
     */
    msg = _v3_recv(V3_BLOCK);
    if (!msg) {
        _v3_func_leave("v3_login");
        return false;
    }
    type = msg->type;
    _v3_process_message(msg);

    /*
       User was banned, get to the choppah!
     */
    if(type == 0x59) {
        _v3_func_leave("v3_login");
        return false;
    }

    /*
       Send authentication info.
     */
    msg = _v3_put_0x48();
    _v3_send(msg);
    _v3_destroy_packet(msg);

    /*
       Process several messages until we're logged in.
     */
    do {
        msg = _v3_recv(V3_BLOCK);
        if(!msg) {
            _v3_func_leave("v3_login");
            return false;
        }
        type = msg->type;
        switch (type) {
            case 0x4a:
                _v3_status(40, "Receiving user permissions.");
                break;
            case 0x5d:
                _v3_status(50, "Receiving user list.");
                break;
            case 0x60:
                _v3_status(60, "Receiving channel list.");
                break;
            case 0x46:
                _v3_status(70, "Receiving user settings.");
                break;
            case 0x50:
                _v3_status(80, "Receiving MOTD.");
                break;
        }
        switch (_v3_process_message(msg)) {
            case V3_MALFORMED:
                _v3_debug(V3_DEBUG_INTERNAL, "received malformed packet");
                break;
            case V3_NOTIMPL:
                _v3_debug(V3_DEBUG_INTERNAL, "packet type not implemented");
                break;
            case V3_OK:
                _v3_debug(V3_DEBUG_INTERNAL, "packet processed");
                break;
            case V3_FAILURE:
                _v3_func_leave("v3_login");
                return false;
                break;
        }
    } while(type != 0x34);

    if (v3_is_loggedin()) {
        _v3_net_message *response;

        /*
         * Start off the 0x5C sequence to verify we're a valid client.
         */
        _v3_status(90, "Scrambling encryption table.");
        response = _v3_put_0x5c(0);
        _v3_send(response);
        _v3_destroy_packet(response);

        /*
         * Tell server some specific client settings.
         */
        _v3_status(95, "Sending user settings.");
        response = _v3_put_0x46(V3_USER_ACCEPT_PAGES, v3_luser.accept_pages);
        _v3_send(response);
        _v3_destroy_packet(response);

        response = _v3_put_0x46(V3_USER_ACCEPT_U2U, v3_luser.accept_u2u);
        _v3_send(response);
        _v3_destroy_packet(response);

        response = _v3_put_0x46(V3_USER_ACCEPT_CHAT, v3_luser.accept_chat);
        _v3_send(response);
        _v3_destroy_packet(response);

        response = _v3_put_0x46(V3_USER_ALLOW_RECORD, v3_luser.allow_recording);
        _v3_send(response);
        _v3_destroy_packet(response);

        /*
         * Let GUI know that login completed.
         */
        _v3_status(100, "Login complete.");
        {
            v3_event *ev = _v3_create_event(V3_EVENT_LOGIN_COMPLETE);
            v3_queue_event(ev);
        }
        _v3_func_leave("v3_login");
        return true;
    } else {
        _v3_func_leave("v3_login");
        return false;
    }
}/*}}}*/

void
v3_join_chat(void) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_join_chat");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_join_chat");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.type = V3_EVENT_CHAT_JOIN;
    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
        _v3_func_leave("v3_join_chat");
        return;
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_join_chat");
    return;
}/*}}}*/

void
v3_leave_chat(void) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_leave_chat");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_leave_chat");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.type = V3_EVENT_CHAT_LEAVE;
    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
        _v3_func_leave("v3_leave_chat");
        return;
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_leave_chat");
    return;
}/*}}}*/

void
v3_send_chat_message(char *message) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_send_chat_message");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_send_chat_message");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.data = malloc(sizeof(v3_event_data));
    memset(ev.data, 0, sizeof(v3_event_data));
    ev.type = V3_EVENT_CHAT_MESSAGE;
    strncpy(ev.data->chatmessage, message, sizeof(ev.data->chatmessage)-1);
    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
        _v3_func_leave("v3_send_chat_message");
        return;
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_send_chat_message");
    return;
}/*}}}*/

void
v3_start_privchat(uint16_t userid) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_start_privchat");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_start_privchat");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.type = V3_EVENT_PRIVATE_CHAT_START;
    ev.user.id = userid;
    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
        _v3_func_leave("v3_start_privchat");
        return;
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_start_privchat");
    return;
}/*}}}*/

void
v3_end_privchat(uint16_t userid) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_end_privchat");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_end_privchat");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.type = V3_EVENT_PRIVATE_CHAT_END;
    ev.user.id = userid;
    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
        _v3_func_leave("v3_end_privchat");
        return;
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_end_privchat");
    return;
}/*}}}*/

void
v3_send_privchat_message(uint16_t userid, char *message) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_send_privchat_message");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_send_privchat_message");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.data = malloc(sizeof(v3_event_data));
    memset(ev.data, 0, sizeof(v3_event_data));
    ev.type = V3_EVENT_PRIVATE_CHAT_MESSAGE;
    ev.user.id = userid;
    strncpy(ev.data->chatmessage, message, sizeof(ev.data->chatmessage)-1);
    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
        _v3_func_leave("v3_send_privchat_message");
        return;
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_send_privchat_message");
    return;
}/*}}}*/

void
v3_send_privchat_away(uint16_t userid) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_send_privchat_away");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_send_privchat_away");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.type = V3_EVENT_PRIVATE_CHAT_AWAY;
    ev.user.id = userid;
    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
        _v3_func_leave("v3_send_privchat_away");
        return;
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_send_privchat_away");
    return;
}/*}}}*/

void
v3_send_privchat_back(uint16_t userid) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_send_privchat_back");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_send_privchat_back");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.type = V3_EVENT_PRIVATE_CHAT_BACK;
    ev.user.id = userid;
    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
        _v3_func_leave("v3_send_privchat_back");
        return;
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_send_privchat_back");
    return;
}/*}}}*/

void
v3_send_tts_message(char *message) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_send_tts_message");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_send_tts_message");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.data = malloc(sizeof(v3_event_data));
    memset(ev.data, 0, sizeof(v3_event_data));
    ev.type = V3_EVENT_TEXT_TO_SPEECH_MESSAGE;
    strncpy(ev.data->chatmessage, message, sizeof(ev.data->chatmessage) - 1);
    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
        _v3_func_leave("v3_send_tts_message");
        return;
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_send_tts_message");
    return;
}/*}}}*/

void
v3_send_play_wave_message(char *message) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_send_play_wave_message");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_send_play_wave_message");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.data = malloc(sizeof(v3_event_data));
    memset(ev.data, 0, sizeof(v3_event_data));
    ev.type = V3_EVENT_PLAY_WAVE_FILE_MESSAGE;
    strncpy(ev.data->chatmessage, message, sizeof(ev.data->chatmessage) - 1);
    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
        _v3_func_leave("v3_send_play_wave_message");
        return;
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_send_play_wave_message");
    return;
}/*}}}*/

int
_v3_logout(void) {/*{{{*/
    _v3_func_enter("_v3_logout");

    // notify the client that they are disconnected
    v3_event *ev = _v3_create_event(V3_EVENT_DISCONNECT);
    v3_queue_event(ev);

    _v3_close_connection();
    free(v3_luser.name);
    free(v3_luser.password);
    free(v3_luser.phonetic);
    /*
     * freeing these causes a hang on logout...
     free(userlist_mutex);
     free(channellist_mutex);
     free(server_mutex);
     free(luser_mutex);
     free(sendq_mutex);
     */
    _v3_destroy_decoders();
    _v3_destroy_channellist();
    _v3_destroy_userlist();
    _v3_destroy_ranklist();
    _v3_destroy_accountlist();
    memset(v3_luser.channel_admin, 0, 65535);
    v3_luser.id = -1;
    fclose(v3_server.evinstream);
    fclose(v3_server.evoutstream);
    close(v3_server.evpipe[0]);
    close(v3_server.evpipe[1]);
    v3_server.evpipe[0] = -1;
    v3_server.evpipe[1] = -1;
    _v3_func_leave("_v3_logout");
    return true;
}/*}}}*/

int
v3_is_loggedin(void) {/*{{{*/
    return _v3_user_loggedin;
}/*}}}*/

uint16_t
v3_get_user_id(void) {/*{{{*/
    return v3_luser.id;
}/*}}}*/

int
v3_debuglevel(uint32_t level) {/*{{{*/
    int oldlevel;

    oldlevel = _v3_debuglevel;
    if (level != -1) {
        _v3_debuglevel = level;
    }
    return oldlevel;
}/*}}}*/

int
v3_queue_size() {/*{{{*/
    _v3_net_message *msg;
    int ctr;

    if (v3_server.queue == NULL) {
        return 0;
    }
    if (v3_server.queue->next == NULL) {
        return 1;
    }
    for (ctr = 0; v3_server.queue->next == NULL; ctr++) {
        msg = v3_server.queue->next;
    }
    return ctr;
}/*}}}*/

void
v3_phantom_remove(uint16_t channel_id) {/*{{{*/
    v3_event ev = {0};
    v3_user *u;

   _v3_func_enter("v3_phantom_remove");

    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_phantom_remove");
        return;
    }

    _v3_debug(V3_DEBUG_EVENT, "attempting to remove phantom from channel %d", channel_id);

    _v3_lock_userlist();
    for (u = v3_user_list; u != NULL; u = u->next) {
        if (u->channel == channel_id && u->real_user_id == v3_luser.id) {
            break;
        }
    }
    _v3_unlock_userlist();

    if (u == NULL) {
        _v3_error("can't find a luser phantom in channel %d", channel_id);
    } else {
        ev.type = V3_EVENT_PHANTOM_REMOVE;
        ev.user.id = u->id;

        _v3_lock_sendq();
        _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
        if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
            _v3_error("could not write to event pipe");
        }

        fflush(v3_server.evoutstream);
        _v3_unlock_sendq();
    }

    _v3_func_leave("v3_phantom_remove");
}/*}}}*/

void
v3_phantom_add(uint16_t channel_id) {/*{{{*/
    v3_event ev = {0};

   _v3_func_enter("v3_phantom_add");

    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_phantom_add");
        return;
    }

    _v3_debug(V3_DEBUG_EVENT, "attempting to add phantom to channel %d", channel_id);

    ev.type = V3_EVENT_PHANTOM_ADD;
    ev.channel.id = channel_id;

    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }

    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_phantom_add");
}/*}}}*/

void
v3_force_channel_move(uint16_t user_id, uint16_t channel_id) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_force_channel_move");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_force_channel_move");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.type = V3_EVENT_FORCE_CHAN_MOVE;
    ev.channel.id = channel_id;
    ev.user.id = user_id;
    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_force_channel_move");
    return;
}/*}}}*/

void
v3_change_channel(uint16_t channel_id, char *password) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_change_channel");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_change_channel");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.type = V3_EVENT_CHANGE_CHANNEL;
    if (password == NULL) {
        strncpy(ev.text.password, "", 31);
    } else {
        strncpy(ev.text.password, password, 31);
    }
    ev.channel.id = channel_id;
    ev.user.id = v3_get_user_id();
    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_change_channel");
    return;
}/*}}}*/

void
v3_admin_login(char *password) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_admin_login");
    if (!v3_is_loggedin() || password == NULL || !password[0]) {
        _v3_func_leave("v3_admin_login");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.type = V3_EVENT_ADMIN_LOGIN;
    strncpy(ev.text.password, password, sizeof(ev.text.password));

    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_admin_login");
    return;
}/*}}}*/

void
v3_admin_logout(void) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_admin_logout");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_admin_logout");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.type = V3_EVENT_ADMIN_LOGOUT;

    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_admin_logout");
    return;
}/*}}}*/

void
v3_admin_boot(enum _v3_boot_types type, uint16_t user_id, char *reason) {/*{{{*/
    v3_event ev = {0};

    _v3_func_enter("v3_admin_boot");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_admin_boot");
        return;
    }

    ev.data = malloc(sizeof(v3_event_data));
    memset(ev.data, 0, sizeof(v3_event_data));
    ev.user.id = user_id;
    strncpy(ev.data->reason, reason ? reason : "", sizeof(ev.data->reason));

    switch (type) {
        case V3_BOOT_KICK:
            ev.type = V3_EVENT_ADMIN_KICK;
            break;
        case V3_BOOT_BAN:
            ev.type = V3_EVENT_ADMIN_BAN;
            break;
        case V3_BOOT_CHANNEL_BAN:
            ev.type = V3_EVENT_ADMIN_CHANNEL_BAN;
            break;
    }

    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_admin_boot");
    return;
}/*}}}*/

void
v3_userlist_open(void) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_userlist_open");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_userlist_open");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.type = V3_EVENT_USERLIST_OPEN;

    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_userlist_open");
    return;
}/*}}}*/

void
v3_userlist_close(void) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_userlist_close");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_userlist_close");
        return;
    }

    _v3_destroy_accountlist();

    memset(&ev, 0, sizeof(v3_event));
    ev.type = V3_EVENT_USERLIST_CLOSE;

    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_userlist_close");
    return;
}/*}}}*/

void
v3_userlist_remove(uint16_t account_id) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_userlist_remove");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_userlist_remove");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.type = V3_EVENT_USERLIST_REMOVE;
    ev.account.id = account_id;

    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_userlist_remove");
    return;
}/*}}}*/

void
v3_userlist_update(v3_account *account) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_userlist_update");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_userlist_update");
        return;
    }

    memset(&ev, 0, sizeof(v3_event));
    ev.data = malloc(sizeof(v3_event_data));
    memset(ev.data, 0, sizeof(v3_event_data));

    if (account->perms.account_id) {
        ev.type = V3_EVENT_USERLIST_MODIFY;
    } else {
        ev.type = V3_EVENT_USERLIST_ADD;
    }

    ev.data->account.perms = account->perms;
    strncpy(ev.data->account.username, account->username, sizeof(ev.data->account.username) - 1);
    strncpy(ev.data->account.owner, account->owner, sizeof(ev.data->account.owner) - 1);
    strncpy(ev.data->account.notes, account->notes, sizeof(ev.data->account.notes) - 1);
    strncpy(ev.data->account.lock_reason, account->lock_reason, sizeof(ev.data->account.lock_reason) - 1);
    ev.data->account.chan_admin_count = account->chan_admin_count;
    memcpy(ev.data->account.chan_admin, account->chan_admin, ev.data->account.chan_admin_count * 2);
    ev.data->account.chan_auth_count = account->chan_auth_count;
    memcpy(ev.data->account.chan_auth, account->chan_auth, ev.data->account.chan_auth_count * 2);

    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_userlist_update");
    return;
}/*}}}*/

void
v3_userlist_change_owner(uint16_t old_owner_id, uint16_t new_owner_id) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_userlist_change_owner");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_userlist_change_owner");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.type = V3_EVENT_USERLIST_CHANGE_OWNER;
    ev.account.id = old_owner_id;
    ev.account.id2 = new_owner_id;

    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_userlist_change_owner");
    return;
}/*}}}*/

void
v3_serverprop_open(void) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_serverprop_open");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_serverprop_open");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.type = V3_EVENT_SRV_PROP_OPEN;

    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_serverprop_open");
    return;
}/*}}}*/

void
v3_set_text(char *comment, char *url, char *integration_text, uint8_t silent) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_set_text");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_set_text");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.type = V3_EVENT_USER_MODIFY;
    ev.user.id = v3_get_user_id();
    if (silent) {
        ev.flags |= 0x100;
    }
    strncpy(ev.text.comment, comment, 127);
    strncpy(ev.text.url, url, 127);
    strncpy(ev.text.integration_text, integration_text, 127);
    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_set_text");
    return;
}/*}}}*/

int
v3_channel_count(void) {/*{{{*/
    v3_channel *c;
    int ctr=0;

    for (c = v3_channel_list; c != NULL; c = c->next, ctr++);
    return ctr;

}/*}}}*/

int
v3_user_count(void) {/*{{{*/
    v3_user *c;
    int ctr=0;

    _v3_lock_userlist();
    for (c = v3_user_list; c != NULL; c = c->next, ctr++);
    _v3_unlock_userlist();
    return ctr-1;

}/*}}}*/

/*
 * This function returns a COPY of the user structure
 */
v3_user *
v3_get_user(uint16_t id) {/*{{{*/
    v3_user *u, *ret_user = NULL;

    _v3_lock_userlist();
    if ((u = _v3_get_user(id))) {
        ret_user = malloc(sizeof(v3_user));
        _v3_copy_user(ret_user, u);
        return ret_user;
    }
    _v3_unlock_userlist();
    return ret_user;
}/*}}}*/

/*
 * This function provides a pointer to the user in the INTERNAL linked list.
 * This does not return copy of the structure.  The caller is responsible for
 * locking this structure if they will be making updates.
 */
v3_user *
_v3_get_user(uint16_t id) {/*{{{*/
    v3_user *u;

    _v3_lock_userlist();
    for (u = v3_user_list; u != NULL; u = u->next) {
        if (u->id == id) {
            _v3_unlock_userlist();
            return u;
        }
    }
    _v3_unlock_userlist();
    return NULL;
}/*}}}*/


uint16_t
v3_get_user_channel(uint16_t id) {/*{{{*/
    v3_user *u;
    uint16_t channel;

    _v3_lock_userlist();
    for (u = v3_user_list; u != NULL; u = u->next) {
        if (u->id == id) {
            channel = u->channel;
            _v3_unlock_userlist();
            return channel;
        }
    }
    _v3_unlock_userlist();
    return -1;
}/*}}}*/

void
v3_free_user(v3_user *user) {/*{{{*/
    free(user->name);
    free(user->phonetic);
    free(user->comment);
    free(user->integration_text);
    free(user->url);
    free(user);
}/*}}}*/

v3_channel *
v3_get_channel(uint16_t id) {/*{{{*/
    v3_channel *c, *ret_channel;

    _v3_func_enter("v3_get_channel");
    _v3_lock_channellist();
    ret_channel = malloc(sizeof(v3_channel));
    for (c = v3_channel_list; c != NULL; c = c->next) {
        if (c->id == id) {
            _v3_copy_channel(ret_channel, c);
            _v3_unlock_channellist();
            _v3_func_leave("v3_get_channel");
            return ret_channel;
        }
    }
    _v3_unlock_channellist();
    _v3_func_leave("v3_get_channel");
    return NULL;
}/*}}}*/

void
v3_free_channel(v3_channel *channel) {/*{{{*/
    free(channel->name);
    free(channel->phonetic);
    free(channel->comment);
    free(channel);
}/*}}}*/

void
v3_channel_update(v3_channel *channel, const char *password) {/*{{{*/
    /* update or create channel */
    v3_event ev;

    _v3_func_enter("v3_channel_update");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_channel_update");
        return;
    }

    memset(&ev, 0, sizeof(v3_event));
    ev.data = malloc(sizeof(v3_event_data));
    memset(ev.data, 0, sizeof(v3_event_data));

    if (channel->id) {
        ev.type = V3_EVENT_CHAN_MODIFY;
    } else {
        ev.type = V3_EVENT_CHAN_ADD;
    }
    
    memcpy(&(ev.data->channel), channel, sizeof(v3_channel) - sizeof(void*) * 4);
    if (password) {
        strncpy(ev.text.password, password, 31);
    }
    strncpy(ev.text.name, channel->name, 31);
    strncpy(ev.text.phonetic, channel->phonetic, 31);
    strncpy(ev.text.comment, channel->comment, 127);

    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_channel_update");
    return;
}/*}}}*/

void
v3_channel_remove(uint16_t channel_id) {/*{{{*/
    /* remove channel */
    v3_event ev;

    _v3_func_enter("v3_channel_remove");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_channel_remove");
        return;
    }

    memset(&ev, 0, sizeof(v3_event));

    ev.type = V3_EVENT_CHAN_REMOVE;
    ev.channel.id = channel_id;
    ev.user.id = v3_get_user_id();
    
    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_channel_remove");
    return;
}/*}}}*/

v3_rank *
v3_get_rank(uint16_t id) {/*{{{*/
    v3_rank *rank, *ret_rank;

    _v3_lock_ranklist();
    for (rank = v3_rank_list; rank != NULL; rank = rank->next) {
        if (rank->id == id) {
            ret_rank = malloc(sizeof(v3_rank));
            _v3_copy_rank(ret_rank, rank);
            _v3_unlock_ranklist();
            return ret_rank;
        }
    }
    _v3_unlock_ranklist();
    return NULL;
}/*}}}*/

void
v3_ranklist_open(void) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_ranklist_open");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_ranklist_open");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.type = V3_EVENT_RANKLIST_OPEN;

    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_ranklist_open");
    return;
}/*}}}*/

void
v3_ranklist_close(void) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_ranklist_close");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_ranklist_close");
        return;
    }

    memset(&ev, 0, sizeof(v3_event));
    ev.type = V3_EVENT_RANKLIST_CLOSE;

    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_ranklist_close");
    return;
}/*}}}*/

void
v3_rank_update(v3_rank *rank) {/*{{{*/
    /* update or create rank */
    v3_event ev;

    _v3_func_enter("v3_rank_update");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_rank_update");
        return;
    }
    
    memset(&ev, 0, sizeof(v3_event));
    ev.data = malloc(sizeof(v3_event_data));
    memset(ev.data, 0, sizeof(v3_event_data));
    
    if (rank->id) ev.type = V3_EVENT_RANK_MODIFY;
    else ev.type = V3_EVENT_RANK_ADD;
    ev.data->rank.id = rank->id;
    ev.data->rank.level = rank->level;
    strncpy(ev.text.name, rank->name, 31);
    strncpy(ev.text.comment, rank->description, 127);

    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_rank_update");
    return;
    
}/*}}}*/

void
v3_rank_remove(uint16_t rankid) {/*{{{*/
    /* remove rank */
    v3_event ev;

    _v3_func_enter("v3_rank_remove");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_rank_remove");
        return;
    }
    
    memset(&ev, 0, sizeof(v3_event));
    ev.data = malloc(sizeof(v3_event_data));
    memset(ev.data, 0, sizeof(v3_event_data));
    ev.type = V3_EVENT_RANK_REMOVE;
    v3_rank *rank = v3_get_rank(rankid);
    if (! rank) {
        _v3_func_leave("v3_rank_remove");
        return;
    }
    ev.data->rank.id = rankid;
    ev.data->rank.level = rank->level;
    strncpy(ev.text.name, rank->name, 31);
    strncpy(ev.text.comment, rank->description, 127);

    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_rank_remove");
    return;
    
}/*}}}*/

void
v3_free_rank(v3_rank *rank) {/*{{{*/
    free(rank->name);
    free(rank->description);
    free(rank);
}/*}}}*/

void
_v3_copy_rank(v3_rank *dest, v3_rank *src) {/*{{{*/
    memcpy(dest, src, sizeof(v3_rank));
    dest->name              = strdup(src->name);
    dest->description       = strdup(src->description);
    dest->next              = NULL;
}/*}}}*/

void
_v3_destroy_ranklist(void) {/*{{{*/
    v3_rank *rank, *next;

    _v3_func_enter("_v3_destroy_ranklist");
    rank = v3_rank_list;
    while (rank != NULL) {
        free(rank->name);
        free(rank->description);
        next = rank->next;
        free(rank);
        rank = next;
    }
    v3_rank_list = NULL;
    _v3_func_leave("_v3_destroy_ranklist");
}/*}}}*/

int
_v3_update_account(v3_account *account) {/*{{{*/
    v3_account *a, *last;

    _v3_func_enter("_v3_update_account");
    _v3_lock_accountlist();
    if (v3_account_list == NULL) {
        a = malloc(sizeof(v3_account));
        memset(a, 0, sizeof(v3_account));
        memcpy(a, account, sizeof(v3_account));

        a->username     = strdup(account->username);
        a->owner        = strdup(account->owner);
        a->notes        = strdup(account->notes);
        a->lock_reason  = strdup(account->lock_reason);
        a->chan_admin   = malloc(account->chan_admin_count * 2);
        memcpy(a->chan_admin, account->chan_admin, account->chan_admin_count * 2);
        a->chan_auth    = malloc(account->chan_auth_count * 2);
        memcpy(a->chan_auth, account->chan_auth, account->chan_auth_count * 2);

        a->next         = NULL;
        v3_account_list = a;
        _v3_debug(V3_DEBUG_INFO, "added first account %s (id %d)",  a->username, a->perms.account_id);
    } else {
        for (a = v3_account_list; a != NULL; a = a->next) {
            if (a->perms.account_id == account->perms.account_id) {
                void *tmp;
                _v3_debug(V3_DEBUG_INFO, "updating account %s",  a->username);
                free(a->username);
                free(a->owner);
                free(a->notes);
                free(a->lock_reason);
                free(a->chan_admin);
                free(a->chan_auth);
                tmp = a->next;

                memcpy(a, account, sizeof(v3_account));
                a->username     = strdup(account->username);
                a->owner        = strdup(account->owner);
                a->notes        = strdup(account->notes);
                a->lock_reason  = strdup(account->lock_reason);
                a->chan_admin   = malloc(account->chan_admin_count * 2);
                memcpy(a->chan_admin, account->chan_admin, account->chan_admin_count * 2);
                a->chan_auth    = malloc(account->chan_auth_count * 2);
                memcpy(a->chan_auth, account->chan_auth, account->chan_auth_count * 2);

                a->next = tmp;
                _v3_debug(V3_DEBUG_INFO, "updated account %s (id %d)",  a->username, a->perms.account_id);
                _v3_unlock_accountlist();
                _v3_func_leave("_v3_update_account");
                return true;
            }
            last = a;
        }
        a = last->next = malloc(sizeof(v3_account));
        memset(a, 0, sizeof(v3_account));
        memcpy(a, account, sizeof(v3_account));

        a->username     = strdup(account->username);
        a->owner        = strdup(account->owner);
        a->notes        = strdup(account->notes);
        a->lock_reason  = strdup(account->lock_reason);
        a->chan_admin   = malloc(account->chan_admin_count * 2);
        memcpy(a->chan_admin, account->chan_admin, account->chan_admin_count * 2);
        a->chan_auth    = malloc(account->chan_auth_count * 2);
        memcpy(a->chan_auth, account->chan_auth, account->chan_auth_count * 2);
        a->next         = NULL;

        _v3_debug(V3_DEBUG_INFO, "added account %s (id %d)",  a->username, a->perms.account_id);
    }
    _v3_unlock_accountlist();
    _v3_func_leave("_v3_update_account");
    return true;
}/*}}}*/


void
_v3_print_account_list(void) {/*{{{*/
    v3_account *c;
    int ctr=0;

    _v3_lock_accountlist();
    for (c = v3_account_list; c != NULL; c = c->next) {
        _v3_debug(V3_DEBUG_INFO, "=====[ account %d ]====================================================================", ctr++);
        _v3_debug(V3_DEBUG_INFO, "account id      : %d", c->perms.account_id);
        _v3_debug(V3_DEBUG_INFO, "account name    : %s", c->username);
        _v3_debug(V3_DEBUG_INFO, "account owner   : %s", c->owner);
        _v3_debug(V3_DEBUG_INFO, "account notes   : %s", c->notes);
    }
    _v3_unlock_accountlist();
}/*}}}*/

void
v3_free_account(v3_account *account) {/*{{{*/
    free(account->username);
    free(account->owner);
    free(account->notes);
    free(account->lock_reason);
    free(account->chan_admin);
    free(account->chan_auth);
    free(account);
}/*}}}*/

void
_v3_destroy_accountlist(void) {/*{{{*/
    v3_account *a, *next;

    _v3_func_enter("_v3_destroy_accountlist");
    _v3_lock_accountlist();
    a = v3_account_list;
    while (a != NULL) {
        free(a->username);
        free(a->owner);
        free(a->notes);
        free(a->lock_reason);
        free(a->chan_admin);
        free(a->chan_auth);
        next = a->next;
        free(a);
        a = next;
    }
    v3_account_list = NULL;
    _v3_unlock_accountlist();
    _v3_func_leave("_v3_destroy_accountlist");
}/*}}}*/

int
_v3_remove_account(uint16_t id) {/*{{{*/
    v3_account *a, *last;

    _v3_lock_accountlist();
    _v3_func_enter("_v3_remove_account");
    last = v3_account_list;
    for (a = v3_account_list; a != NULL; a = a->next) {
        if (a->perms.account_id == id) {
            last->next = a->next;
            _v3_debug(V3_DEBUG_INFO, "removed account %s", a->username);
            v3_free_account(a);
            _v3_func_leave("_v3_remove_account");
            _v3_unlock_accountlist();
            return true;
        }
        last = a;
    }
    _v3_func_leave("_v3_remove_account");
    _v3_unlock_accountlist();
    return false;
}/*}}}*/

int
v3_account_count(void) {/*{{{*/
    v3_account *a;
    int ctr=0;

    _v3_lock_accountlist();
    for (a = v3_account_list; a != NULL; a = a->next, ctr++);
    _v3_unlock_accountlist();

    return ctr;

}/*}}}*/

v3_account *
v3_get_account(uint16_t id) {/*{{{*/
    v3_account *a, *ret_account;

    _v3_func_enter("v3_get_account");
    _v3_lock_accountlist();
    ret_account = malloc(sizeof(v3_account));
    for (a = v3_account_list; a != NULL; a = a->next) {
        if (a->perms.account_id == id) {
            _v3_copy_account(ret_account, a);
            _v3_unlock_accountlist();
            _v3_func_leave("v3_get_account");
            return ret_account;
        }
    }
    _v3_unlock_accountlist();
    _v3_func_leave("v3_get_account");
    return NULL;
}/*}}}*/

void
_v3_copy_account(v3_account *dest, v3_account *src) {/*{{{*/
    memcpy(dest, src, sizeof(v3_account));
    dest->username     = strdup(src->username);
    dest->owner        = strdup(src->owner);
    dest->notes        = strdup(src->notes);
    dest->lock_reason  = strdup(src->lock_reason);
    dest->chan_admin   = malloc(src->chan_admin_count * 2);
    memcpy(dest->chan_admin, src->chan_admin, src->chan_admin_count * 2);
    dest->chan_auth    = malloc(src->chan_auth_count * 2);
    memcpy(dest->chan_auth, src->chan_auth, src->chan_auth_count * 2);
    dest->next         = NULL;
}/*}}}*/

void
_v3_lock_accountlist(void) {/*{{{*/
    // TODO: PTHREAD: check if threads are enabled, possibly use semaphores as a compile time option
    if (accountlist_mutex == NULL) {
        pthread_mutexattr_t mta;
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);

        _v3_debug(V3_DEBUG_MUTEX, "initializing accountlist mutex");
        accountlist_mutex = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(accountlist_mutex, &mta);
    }
    _v3_debug(V3_DEBUG_MUTEX, "locking accountlist");
    pthread_mutex_lock(accountlist_mutex);
}/*}}}*/

void
_v3_unlock_accountlist(void) {/*{{{*/
    // TODO: PTHREAD: check if threads are enabled, possibly use semaphores as a compile time option
    if (accountlist_mutex == NULL) {
        pthread_mutexattr_t mta;
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);

        _v3_debug(V3_DEBUG_MUTEX, "initializing accountlist mutex");
        accountlist_mutex = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(accountlist_mutex, &mta);
    }
    _v3_debug(V3_DEBUG_MUTEX, "unlocking accountlist");
    pthread_mutex_unlock(accountlist_mutex);
}/*}}}*/

int
v3_queue_event(v3_event *ev) {/*{{{*/
    v3_event *last;
    int len = 0;

    _v3_func_enter("v3_queue_event");
    if (eventq_mutex == NULL) {
        // do not queue events if the mutex hasn't been initialized.  This
        // means the client isn't ready (or doesn't want) to receive events
        free(ev);
        _v3_debug(V3_DEBUG_EVENT, "client does not appear to be listening yet... not queueing");
        _v3_func_leave("v3_queue_event");
        return true;
    }
    pthread_mutex_lock(eventq_mutex);
    // if we're not allowed to see channels, gui should think any channel is the lobby
    if(!v3_luser.perms.see_chan_list || !v3_channel_count()) {
        ev->channel.id = 0;
    }
    ev->next = NULL;
    ev->timestamp = time(NULL);
    // if this returns null, there's no events in the queue
    if ((last = _v3_get_last_event(&len)) == NULL) {
        _v3_eventq = ev;
        _v3_debug(V3_DEBUG_EVENT, "queued event type %d.  now have 1 event in queue", ev->type);
        pthread_cond_signal(eventq_cond);
        pthread_mutex_unlock(eventq_mutex);
        _v3_func_leave("v3_queue_event");
        return true;
    }
    // otherwise, tack it on to the end
    last->next = ev;
    _v3_debug(V3_DEBUG_EVENT, "queued event type %d.  now have %d events in queue", ev->type, len);
    pthread_mutex_unlock(eventq_mutex);
    _v3_func_leave("v3_queue_event");
    return true;
}/*}}}*/

v3_event *
v3_get_event(int block) {/*{{{*/
    v3_event *ev;

    if (eventq_mutex == NULL) {
        pthread_mutexattr_t mta;
        pthread_mutexattr_init(&mta);
        pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);

        _v3_debug(V3_DEBUG_MUTEX, "initializing _v3_eventq mutex");
        eventq_mutex = malloc(sizeof(pthread_mutex_t));
        eventq_cond = malloc(sizeof(pthread_cond_t));
        pthread_mutex_init(eventq_mutex, &mta);
        pthread_cond_init(eventq_cond, (pthread_condattr_t *) &mta);
    }
    // if we're not blocking and ev is NULL, just return NULL;
    if (block == V3_NONBLOCK && _v3_eventq == NULL) {
        return NULL;
    }
    pthread_mutex_lock(eventq_mutex);
    if (_v3_eventq == NULL) {
        _v3_debug(V3_DEBUG_MUTEX, "waiting for an event...");
        pthread_cond_wait(eventq_cond, eventq_mutex);
    }
    ev = _v3_eventq;
    _v3_eventq = ev->next;
    pthread_mutex_unlock(eventq_mutex);
    return ev;
}/*}}}*/

v3_event *
_v3_create_event(uint16_t event) {/*{{{*/
    v3_event *ev;
    ev = malloc(sizeof(v3_event));
    memset(ev, 0, sizeof(v3_event));
    ev->data = malloc(sizeof(v3_event_data));
    memset(ev->data, 0, sizeof(v3_event_data));
    if(event) {
        ev->type = event;
    }
    return ev;
}/*}}}*/

v3_event *
_v3_get_last_event(int *len) {/*{{{*/
    v3_event *ev;
    int ctr;
    if (_v3_eventq == NULL) {
        return NULL;
    }
    for (ctr = 0, ev = _v3_eventq; ev->next != NULL; ctr++, ev = ev->next);
    if (len != NULL) {
        *len = ctr;
    }
    return ev;
}/*}}}*/

void
v3_free_event(v3_event *ev) {/*{{{*/
    if (ev && ev->data) {
        free(ev->data);
        ev->data = NULL;
    }
    if (ev) {
        free(ev);
        ev = NULL;
    }
}/*}}}*/

void
v3_clear_events(void) {/*{{{*/
    v3_event *ev;
    if (_v3_eventq == NULL) {
        return;
    }
    while (_v3_eventq != NULL) {
        ev = _v3_eventq->next;
        v3_free_event(_v3_eventq);
        _v3_eventq = ev;
    }
    return;
}/*}}}*/

int
v3_get_max_clients(void) {/*{{{*/
    // - 1 for the lobby user
    return v3_server.max_clients;
}/*}}}*/

int
v3_is_licensed(void) {/*{{{*/
    return v3_server.is_licensed;
}/*}}}*/

uint32_t
v3_get_bytes_recv(void) {/*{{{*/
    return v3_server.recv_byte_count;
}/*}}}*/

uint32_t
v3_get_bytes_sent(void) {/*{{{*/
    return v3_server.sent_byte_count;
}/*}}}*/

uint32_t
v3_get_packets_recv(void) {/*{{{*/
    return v3_server.recv_packet_count;
}/*}}}*/

uint32_t
v3_get_packets_sent(void) {/*{{{*/
    return v3_server.sent_packet_count;
}/*}}}*/

uint32_t
v3_get_codec_rate(uint16_t codec, uint16_t format) {/*{{{*/
    int ctr = 0;

    while (v3_codecs[ctr].codec != (uint8_t)-1) {
        if (v3_codecs[ctr].codec == codec && v3_codecs[ctr].format == format) {
            return v3_codecs[ctr].rate;
        }
        ctr++;
    }

    return 0;
}/*}}}*/

const v3_codec*
v3_get_codec(uint16_t codec, uint16_t format) {/*{{{*/
    int ctr = 0;

    while (v3_codecs[ctr].codec != (uint8_t)-1) {
        if (v3_codecs[ctr].codec == codec && v3_codecs[ctr].format == format) {
            return &v3_codecs[ctr];
        }
        ctr++;
    }

    return &v3_codecs[ctr];
}/*}}}*/

const v3_codec*
v3_get_channel_codec(uint16_t channel_id) {/*{{{*/
    v3_channel *c;
    const v3_codec *codec_info;

    _v3_func_enter("v3_get_channel_codec");
    if (channel_id == 0 || (c = v3_get_channel(channel_id)) == NULL) { // the lobby is always the default codec
        _v3_func_leave("v3_get_channel_codec");
        return v3_get_codec(v3_server.codec, v3_server.codec_format);
    }
    _v3_debug(V3_DEBUG_INFO, "getting codec for %d/%d", c->channel_codec, c->channel_format);
    if (c->channel_codec == 65535 || c->channel_format == 65535) {
        _v3_debug(V3_DEBUG_INFO, "getting server default codec");
        codec_info = v3_get_codec(v3_server.codec, v3_server.codec_format);
    } else {
        _v3_debug(V3_DEBUG_INFO, "getting channel codec");
        codec_info = v3_get_codec(c->channel_codec, c->channel_format);
    }
    v3_free_channel(c);
    if (codec_info) {
        _v3_debug(V3_DEBUG_INFO, "channel codec is %d/%d %s", codec_info->codec, codec_info->format, codec_info->name);
    } else {
        _v3_debug(V3_DEBUG_INFO, "unknown codec for channel %d", channel_id);
    }
    _v3_func_leave("v3_get_channel_codec");
    return codec_info;
}/*}}}*/

// Return the channel id of the channel that requires the password (possibly a
// parent channel)
uint16_t
v3_channel_requires_password(uint16_t channel_id) {/*{{{*/
    uint16_t parent;
    v3_channel *c;

    _v3_func_enter("v3_channel_requires_password");
    if (channel_id == 0) {
        _v3_func_leave("v3_channel_requires_password");
        return false;
    }
    c = v3_get_channel(channel_id);
    if (c->protect_mode == 1) {
        v3_free_channel(c);
        _v3_func_leave("v3_channel_requires_password");
        return channel_id;
    }
    parent = c->parent;
    v3_free_channel(c);
    _v3_func_leave("v3_channel_requires_password");
    return v3_channel_requires_password(parent);
}/*}}}*/

/*
 * Do not confuse v3_logout() with _v3_logout().  This is the external API
 * function that requests a disconnect via the pipe.  _v3_logout() responds by
 * queueing a V3_EVENT_DISCONNECT event
 */
void
v3_logout(void) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_logout");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_logout");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.type = V3_EVENT_DISCONNECT;
    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe", sizeof(v3_event));
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
        _v3_func_leave("v3_logout");
        return;
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_logout");
    return;
}/*}}}*/

void
v3_start_audio(uint16_t send_type) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_start_audio");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_start_audio");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.type = V3_EVENT_USER_TALK_START;
    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe for event type %d", sizeof(v3_event), ev.type);
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_start_audio");
    return;
}/*}}}*/

uint32_t
v3_pcmlength_for_rate(uint32_t rate) {/*{{{*/
    const v3_codec *codec;

    _v3_func_enter("v3_pcmlength_for_rate");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_pcmlength_for_rate");
        return 0;
    }
    codec = v3_get_channel_codec(v3_get_user_channel(v3_get_user_id()));
    if (codec) {
        uint32_t bytestosend = codec->pcmframesize;
        switch (codec->codec) {
          case 0:
            switch (codec->format) {
              case 1:
                bytestosend *= 4;
                break;
              case 2:
                bytestosend *= 7;
                break;
              case 3:
                bytestosend *= 15;
                break;
            }
            break;
          case 1:
            bytestosend *= 15;
            break;
          case 2:
            bytestosend *= 7;
            break;
          case 3:
            bytestosend *= 6;
            break;
        }
        bytestosend *= ((float)rate / (float)codec->rate);
        _v3_func_leave("v3_pcmlength_for_rate");
        return bytestosend + bytestosend % 2;
    }
    _v3_func_leave("v3_pcmlength_for_rate");
    return 0;
}/*}}}*/

uint32_t
v3_send_audio(uint16_t send_type, uint32_t rate, uint8_t *pcm, uint32_t length, uint8_t stereo) {/*{{{*/
    v3_event ev;
    const v3_codec *codec;

    _v3_func_enter("v3_send_audio");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_send_audio");
        return 0;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.data = malloc(sizeof(v3_event_data));
    memset(ev.data, 0, sizeof(v3_event_data));
    ev.type = V3_EVENT_PLAY_AUDIO;
    ev.pcm.send_type = send_type;
    ev.pcm.rate = rate;
    ev.pcm.length = length;
    ev.pcm.channels = stereo ? 2 : 1;

    codec = v3_get_channel_codec(v3_get_user_channel(v3_get_user_id()));
    if (send_type == V3_AUDIO_SENDTYPE_U2CCUR && codec->rate != rate) {
#if HAVE_SPEEX_DSP
        static void *resampler = NULL;
        static uint32_t in_rate = 0;
        static uint32_t out_rate = 0;
        static int err = 0;
        uint8_t channels = stereo ? 2 : 1;
        uint32_t insamples = length;
        uint32_t outsamples = v3_pcmlength_for_rate(codec->rate);

        if (!resampler || rate != in_rate || codec->rate != out_rate) {
            if (resampler) {
                speex_resampler_destroy(resampler);
                resampler = NULL;
            }
            in_rate = rate;
            out_rate = codec->rate;
            resampler = speex_resampler_init(channels, in_rate, out_rate, 10, &err);
        }
        if (length > v3_pcmlength_for_rate(rate)) {
            _v3_error("sample size is %d but a sample of %d size was supplied.", outsamples, insamples);
            _v3_func_leave("v3_send_audio");
            return 0;
        }
        if (err) {
            _v3_error("resampler initialization error: %d: %s\n", err, speex_resampler_strerror(err));
            _v3_func_leave("v3_send_audio");
            return 0;
        }
        insamples  /= sizeof(int16_t) * channels;
        outsamples /= sizeof(int16_t) * channels;
        err = speex_resampler_process_interleaved_int(resampler, (void *)pcm, &insamples, (void *)ev.data->sample, &outsamples);
        if (err) {
            _v3_error("resampling error: %d: %s\n", err, speex_resampler_strerror(err));
            _v3_func_leave("v3_send_audio");
            return 0;
        }
        //speex_resampler_destroy(resampler);
        ev.pcm.length = outsamples * sizeof(int16_t) * channels;
#else
        //_v3_error("sample rate (%d) did not match codec rate (%d) and speex dsp was not found.", rate, codec->rate);
        _v3_func_leave("v3_send_audio");
        return codec->rate; // this is still needed for mangleraudio
#endif
    } else {
        memcpy(ev.data->sample, pcm, length);
    }
    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe for event type %d (pcm length %d)", sizeof(v3_event), ev.type, length);
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_send_audio");
    return rate;
}/*}}}*/

void
v3_stop_audio(void) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_start_audio");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_start_audio");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.type = V3_EVENT_USER_TALK_END;
    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe for event type %d", sizeof(v3_event), ev.type);
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_start_audio");
    return;
}/*}}}*/

void
v3_set_server_opts(uint8_t type, uint8_t value) {/*{{{*/
    switch (type) {
        case V3_USER_ACCEPT_PAGES:
            v3_luser.accept_pages = value;
            break;
        case V3_USER_ACCEPT_U2U:
            v3_luser.accept_u2u = value;
            break;
        case V3_USER_ALLOW_RECORD:
            v3_luser.allow_recording = value;
            break;
        case V3_USER_ACCEPT_CHAT:
            v3_luser.accept_chat = value;
            break;
    }
}/*}}}*/

const v3_permissions *
v3_get_permissions(void) {/*{{{*/
    return &v3_luser.perms;
}/*}}}*/

uint8_t
v3_is_channel_admin(uint16_t channel_id) {/*{{{*/
    v3_channel *c;
    if (v3_luser.channel_admin[channel_id]) {
        return 1;
    }
    if (! channel_id) {
        return 0;
    }
    c = v3_get_channel(channel_id);
    channel_id = c->parent;
    v3_free_channel(c);
    return v3_is_channel_admin(channel_id);
}/*}}}*/

/*
 * Using these functions may chew up CPU since they perform mathematical
 * operations on every 16 bit pcm sample.
 */
void
v3_set_volume_master(int level) {/*{{{*/
    if (level < 0 || level > 148) {
        return;
    }
    _v3_master_volume = level;
}/*}}}*/

void
v3_set_volume_user(uint16_t id, int level) {/*{{{*/
    if (level < 0 || level > 148) {
        return;
    }
    _v3_user_volumes[id] = level;
}/*}}}*/

void
v3_set_volume_luser(int level) {/*{{{*/
    v3_set_volume_user(v3_get_user_id(), level);
}/*}}}*/

uint8_t
v3_get_volume_user(uint16_t id) {/*{{{*/
    return _v3_user_volumes[id];
}/*}}}*/

uint8_t
v3_get_volume_luser(void) {/*{{{*/
    return v3_get_volume_user(v3_get_user_id());
}/*}}}*/

