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

#include "config.h"
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
#include <speex/speex.h>
#ifdef HAVE_GSM_H
#include <gsm.h>
#else
#include <gsm/gsm.h>
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
        fprintf(stderr, "libventrilo3: %s\n",buf); // print without timestamp
        return;
    }

    if (strftime(timestamp, sizeof(timestamp), "%T", tmp) == 0) {
        fprintf(stderr, "libventrilo3: %s\n",buf); // print without timestamp
        return;
    }

    fprintf(stderr, "libventrilo3: %s.%06d: %s\n", timestamp, tv.tv_usec, buf); //print with timestamp
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
    v3_event *ev;

    if (format == NULL) {
        return _v3_status_text;
    }
    va_start(args, format);
    vsnprintf(_v3_status_text, sizeof(_v3_status_text), format, args);
    va_end(args);
    ev = malloc(sizeof(v3_event));
    memset(ev, 0, sizeof(v3_event));
    ev->type = V3_EVENT_STATUS;
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
            _v3_debug(V3_DEBUG_INFO, "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
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
_v3_net_message_dump(_v3_net_message *msg) {/*{{{*/
    int ctr, ctr2;
    char buf[256] = "", buf2[8] = "";

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

/*
   Perform the initial key exchange with the server
 */
int
_v3_server_key_exchange(void) {/*{{{*/
    _v3_net_message msg;
    _v3_msg_0x00 msg0;
    char buf[32];
    char *msgdata;
    int ctr, len;

    _v3_func_enter("_v3_server_key_exchange");

    // Build a net message of type zero with random data
    memset(&msg0, 0, sizeof(_v3_msg_0x00));
    strncpy(msg0.version, "3.0.0", 16);
    
    for(ctr = 0; ctr < 31; ctr++) {
        buf[ctr] = rand() % 93 + 33;
    }
    buf[ctr] = '\0';
    memcpy(msg0.salt1, buf, 32);
    
    for(ctr = 0; ctr < 31; ctr++) {
        buf[ctr] = rand() % 93 + 33;
    }
    buf[ctr] = '\0';
    memcpy(msg0.salt2, buf, 32);
    
    msg.type = 0x00;
    msg.len  = sizeof(_v3_msg_0x00);
    msg.data = (char *)&msg0;
    
    _v3_net_message_dump(&msg);

    // Encrypt the message and send it to the server
    ventrilo_first_enc((uint8_t *)msg.data, msg.len);
    _v3_send_enc_msg(msg.data, msg.len);

    msgdata = malloc(0xffff);
    len = _v3_recv_enc_msg(msgdata);
    ventrilo_first_dec((uint8_t *)msgdata, len);
    _v3_debug(V3_DEBUG_INFO, "Received following packet with keys:");
    _v3_net_message_dump_raw(msgdata, len);

    if (ventrilo_read_keys(&v3_server.client_key, &v3_server.server_key, (uint8_t *)msgdata+12, len-12) < 0) {
        _v3_error("could not parse keys from the server");
    }
    free(msgdata);

    _v3_func_leave("_v3_server_key_exchange");
    return true;
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
        return false;
    }
    tout.tv_sec  = 4;
    tout.tv_usec = 0;
    FD_ZERO(&fd_read);
    FD_SET(sd, &fd_read);
    if(select(sd + 1, &fd_read, NULL, NULL, &tout) <= 0) {
        _v3_error("could not authenticate server: timed out waiting for auth server");
        return false;
    }
    len = recvfrom(sd, buf, sizeof(buf), 0, NULL, NULL);
    if (len < 0) {
        _v3_error("could not authenticate server: udp receive failed");
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

    _v3_func_enter("_v3_send_enc_msg");
    _v3_debug(V3_DEBUG_PACKET, "======= sending encrypted TCP packet ============================");
    _v3_net_message_dump_raw(data, len);
    lenptr = htons(len);
    if (send(_v3_sockd, &lenptr, 2, 0) != 2) {
        _v3_error("failed to send packet length");
        _v3_func_leave("_v3_send_enc_msg");
        return false;
    }
    if (send(_v3_sockd, data, len, 0) != len) {
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
            _v3_debug(V3_DEBUG_EVENT, "event waiting to processed and sent outbound");
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
                            if (_v3_send(msg)) {
                                _v3_destroy_packet(msg);
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send channel change message");
                                _v3_destroy_packet(msg);
                            }
                        }
                        break;/*}}}*/
                    case V3_EVENT_PLAY_AUDIO:/*{{{*/
                        {
                            _v3_net_message *msg;
                            uint16_t        send_type;
                            const v3_codec  *codec;
                            void            *data;

                            _v3_debug(V3_DEBUG_INFO, "got outbound audio event", codec->rate);
                            codec = v3_get_channel_codec(v3_get_user_channel(v3_get_user_id()));
                            // TODO: this is too messy to do here, make it a function
                            switch (codec->codec) {
                                case 0:
                                    _v3_debug(V3_DEBUG_INFO, "encoding PCM to GSM @ %lu", codec->rate);
                                    break;
                                case 3:
                                    _v3_debug(V3_DEBUG_INFO, "encoding PCM to GSM @ %lu", codec->rate);
                                    break;
                            }
                            /*
                            msg = _v3_put_0x52(send_type, codec, codec_format, data);
                            if (_v3_send(msg)) {
                                _v3_destroy_packet(msg);
                            } else {
                                _v3_debug(V3_DEBUG_SOCKET, "failed to send channel change message");
                                _v3_destroy_packet(msg);
                            }
                            */
                        }
                        break;/*}}}*/
                    default:
                        _v3_debug(V3_DEBUG_EVENT, "received unknown event type %d from queue", ev.type);
                        break;
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
                _v3_debug(V3_DEBUG_SOCKET, "received %d bytes", msg->len);
            }
            ventrilo_dec(&v3_server.server_key, (uint8_t *)msgdata, msg->len);
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
        if (recvlen == 0) {
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
        if (recvlen == 0) {
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
    gettimeofday(&now, NULL);
    _v3_next_timestamp(&tv, &v3_server.last_timestamp);
    _v3_debug(V3_DEBUG_INFO, "outbound timestamp pending in %d.%d seconds", tv.tv_sec, tv.tv_usec);

    while ((ret = select(_v3_sockd > v3_server.evpipe[0] ? _v3_sockd+1 : v3_server.evpipe[0]+1, &rset, NULL, NULL, &tv)) >= 0) {
        _v3_next_timestamp(&tv, &v3_server.last_timestamp);
        _v3_debug(V3_DEBUG_INFO, "outbound timestamp pending in %d.%d seconds", tv.tv_sec, tv.tv_usec);
        if (!_v3_is_connected()) {
            return false;
        }
        if (ret == 0) {
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
_v3_hash_password(uint8_t* password, uint8_t* hash) {/*{{{*/
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
        _v3_debug(V3_DEBUG_INFO, "added first channel %s",  c->name);
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
                _v3_debug(V3_DEBUG_INFO, "updated channel %s",  c->name);
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
        _v3_debug(V3_DEBUG_INFO, "added channel %s",  c->name);
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
        v3_user_list = u;
    } else {
        for (u = v3_user_list; u != NULL; u = u->next) { // search for existing users
            if (u->id == user->id) {
                void *tmp;
                free(u->name);
                free(u->phonetic);
                free(u->comment);
                free(u->integration_text);
                free(u->url);
                tmp = u->next;
                memcpy(u, user, sizeof(v3_user));
                u->name             = strdup(user->name);
                u->comment          = strdup(user->comment);
                u->phonetic         = strdup(user->phonetic);
                u->integration_text = strdup(user->integration_text);
                u->url              = strdup(user->url);
                u->guest            = user->bitfield & 0x400 ? 1 : 0;
                u->next             = tmp;
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
        u->next             = NULL;
    }
    _v3_unlock_userlist();
    _v3_func_leave("_v3_update_user");
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

    _v3_func_enter("_v3_destroy_channel");
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
    _v3_func_leave("_v3_destroy_channel");
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
                v3_event *ev = malloc(sizeof(v3_event));
                char buf[512] = "";
                int error = false;

                memset(ev, 0, sizeof(v3_event));
                // This lock will only be needed when we start calling ventrilo_read_keys() from here.
                _v3_lock_server();
                if(m->subtype & 0x01) {
                    error = true;
                    ev->error.disconnected = true;
                    strncat(buf, "You have been disconnected from the server.\n", 511);
                    _v3_logout();
                }
                if(m->subtype & 0x02) {
                    _v3_debug(V3_DEBUG_INTERNAL, "FIXME: Authenticated as administrator.");
                }
                if(m->subtype & 0x04) {
                    /*
                     * TODO: 
                     * We should pass m->encryption_key to ventrilo_read_keys() and respond with our own key.
                     * For now, this is done using the _v3_server_key_exchange() function.
                     */
                    _v3_debug(V3_DEBUG_INTERNAL, "FIXME: ventrilo_read_keys() should be called from 0x06 processing.");
                    /*
                       if (ventrilo_read_keys(&v3_server.client_key, &v3_server.server_key, m->encryption_key, m->len - 12) < 0) {
                       _v3_error("could not parse keys from the server");
                       }*/
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
                    _v3_debug(V3_DEBUG_INTERNAL, "FIXME: Server has been disabled: %s", _v3_server_disabled_errors[m->error_id-1]);
                }
                if(m->subtype & 0x400) {
                    _v3_debug(V3_DEBUG_INTERNAL, "FIXME: Unknown subtype, please report a packetdump.");
                }
                if (error) {
                    ev->type = V3_EVENT_ERROR_MSG;
                    strncpy(ev->error.message, buf, 512);
                    v3_queue_event(ev);
                } else {
                    // it's an informational message free it for now, may want
                    // to queue it as something else later
                    free(ev);
                }
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                _v3_unlock_server();
            }
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
                  //libventrilo3: 15:35:41:         ======= received TCP packet =====================================
                  //libventrilo3: 15:35:41:         PACKET: message type: 0x3B (59)
                  //libventrilo3: 15:35:41:         PACKET: data length : 12
                  //libventrilo3: 15:35:41:         PACKET:     3B 00 00 00 05 00 03 00 00 00 00 00                  ;...........

                _v3_msg_0x3b *m = msg->contents;

                v3_event *ev;
                v3_user *user;
                _v3_debug(V3_DEBUG_INFO, "user %d moved to channel %d", m->user_id, m->channel_id);
                user = v3_get_user(m->user_id);
                user->channel = m->channel_id;
                _v3_update_user(user);
                v3_free_user(user);
                ev = malloc(sizeof(v3_event));
                memset(ev, 0, sizeof(v3_event));
                ev->type = V3_EVENT_USER_CHAN_MOVE;
                ev->user.id = m->user_id;
                ev->channel.id = m->channel_id;
                v3_queue_event(ev);
            }
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
            _v3_func_leave("_v3_process_message");
            _v3_unlock_server();
            return V3_OK;/*}}}*/
        case 0x37:/*{{{*/
            if (!_v3_get_0x37(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                v3_event *ev;
                _v3_msg_0x37 *m = msg->contents;
                ev = malloc(sizeof(v3_event));
                memset(ev, 0, sizeof(v3_event));
                ev->type = V3_EVENT_PING;
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
        case 0x46:/*{{{*/
            if (!_v3_get_0x46(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_lock_userlist();
                //_v3_msg_0x46 *m = msg->contents;
                // TODO: update the user list
                _v3_unlock_userlist();
            }
            _v3_destroy_packet(msg);
            _v3_func_leave("_v3_process_message");
            return V3_OK;/*}}}*/
        case 0x4a:/*{{{*/
            if (!_v3_get_0x4a(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_lock_luser();
                _v3_msg_0x4a *m = msg->contents;
                v3_luser.perms.lock_acct             = m->lock_acct;
                v3_luser.perms.dfl_chan              = m->dfl_chan;
                v3_luser.perms.dupe_ip               = m->dupe_ip;
                v3_luser.perms.switch_chan           = m->switch_chan;
                v3_luser.perms.in_reserve_list       = m->in_reserve_list;
                v3_luser.perms.unknown_perm_1        = m->unknown_perm_1;
                v3_luser.perms.unknown_perm_2        = m->unknown_perm_2;
                v3_luser.perms.unknown_perm_3        = m->unknown_perm_3;
                v3_luser.perms.recv_bcast            = m->recv_bcast;
                v3_luser.perms.add_phantom           = m->add_phantom;
                v3_luser.perms.record                = m->record;
                v3_luser.perms.recv_complaint        = m->recv_complaint;
                v3_luser.perms.send_complaint        = m->send_complaint;
                v3_luser.perms.inactive_exempt       = m->inactive_exempt;
                v3_luser.perms.unknown_perm_4        = m->unknown_perm_4;
                v3_luser.perms.unknown_perm_5        = m->unknown_perm_5;
                v3_luser.perms.srv_admin             = m->srv_admin;
                v3_luser.perms.add_user              = m->add_user;
                v3_luser.perms.del_user              = m->del_user;
                v3_luser.perms.ban_user              = m->ban_user;
                v3_luser.perms.kick_user             = m->kick_user;
                v3_luser.perms.move_user             = m->move_user;
                v3_luser.perms.assign_chan_admin     = m->assign_chan_admin;
                v3_luser.perms.edit_rank             = m->edit_rank;
                v3_luser.perms.edit_motd             = m->edit_motd;
                v3_luser.perms.edit_guest_motd       = m->edit_guest_motd;
                v3_luser.perms.issue_rcon_cmd        = m->issue_rcon_cmd;
                v3_luser.perms.edit_voice_target     = m->edit_voice_target;
                v3_luser.perms.edit_command_target   = m->edit_command_target;
                v3_luser.perms.assign_rank           = m->assign_rank;
                v3_luser.perms.assign_reserved       = m->assign_reserved;
                v3_luser.perms.unknown_perm_6        = m->unknown_perm_6;
                v3_luser.perms.unknown_perm_7        = m->unknown_perm_7;
                v3_luser.perms.unknown_perm_8        = m->unknown_perm_8;
                v3_luser.perms.unknown_perm_9        = m->unknown_perm_9;
                v3_luser.perms.unknown_perm_10       = m->unknown_perm_10;
                v3_luser.perms.bcast                 = m->bcast;
                v3_luser.perms.bcast_lobby           = m->bcast_lobby;
                v3_luser.perms.bcast_user            = m->bcast_user;
                v3_luser.perms.bcast_x_chan          = m->bcast_x_chan;
                v3_luser.perms.send_tts_bind         = m->send_tts_bind;
                v3_luser.perms.send_wav_bind         = m->send_wav_bind;
                v3_luser.perms.send_page             = m->send_page;
                v3_luser.perms.send_comment          = m->send_comment;
                v3_luser.perms.set_phon_name         = m->set_phon_name;
                v3_luser.perms.gen_comment_snds      = m->gen_comment_snds;
                v3_luser.perms.event_snds            = m->event_snds;
                v3_luser.perms.mute_glbl             = m->mute_glbl;
                v3_luser.perms.mute_other            = m->mute_other;
                v3_luser.perms.glbl_chat             = m->glbl_chat;
                v3_luser.perms.start_priv_chat       = m->start_priv_chat;
                v3_luser.perms.unknown_perm_11       = m->unknown_perm_11;
                v3_luser.perms.eq_out                = m->eq_out;
                v3_luser.perms.unknown_perm_12       = m->unknown_perm_12;
                v3_luser.perms.unknown_perm_13       = m->unknown_perm_13;
                v3_luser.perms.unknown_perm_14       = m->unknown_perm_14;
                v3_luser.perms.see_guest             = m->see_guest;
                v3_luser.perms.see_nonguest          = m->see_nonguest;
                v3_luser.perms.see_motd              = m->see_motd;
                v3_luser.perms.see_srv_comment       = m->see_srv_comment;
                v3_luser.perms.see_chan_list         = m->see_chan_list;
                v3_luser.perms.see_chan_comment      = m->see_chan_comment;
                v3_luser.perms.see_user_comment      = m->see_user_comment;
                v3_luser.perms.unknown_perm_15       = m->unknown_perm_15;
                _v3_unlock_luser();
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
                            v3_event *ev;
                            ev = malloc(sizeof(v3_event));
                            memset(ev, 0, sizeof(v3_event));
                            ev->type = V3_EVENT_CHAN_REMOVE;
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
                            v3_event *ev;
                            _v3_update_channel(m->channel);
                            ev = malloc(sizeof(v3_event));
                            memset(ev, 0, sizeof(v3_event));
                            ev->type = (m->subtype == V3_MODIFY_CHANNEL) ? V3_EVENT_CHAN_MODIFY : V3_EVENT_CHAN_ADD;
                            ev->channel.id = m->channel->id;
                            _v3_debug(V3_DEBUG_INFO, "queuing event type %d for channel %d", ev->type, ev->channel.id);
                            v3_queue_event(ev);
                        }
                        break;
                    case V3_AUTHFAIL_CHANNEL:
                        {
                            v3_event *ev;
                            ev = malloc(sizeof(v3_event));
                            memset(ev, 0, sizeof(v3_event));
                            ev->type = V3_EVENT_ERROR_MSG;
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
        case 0x50:/*{{{*/
            if (!_v3_get_0x50(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_msg_0x50 *m = msg->contents;
                char **motd;
                int size = 0;

                _v3_lock_server();
                if(m->message_id + 1 > m->message_num) {
                    _v3_debug(V3_DEBUG_PACKET_PARSE, "Received %d packet but max packets is %d", m->message_id, m->message_num);
                    _v3_func_leave("_v3_get_0x50");
                    _v3_unlock_server();
                    return false;
                }
                motd = m->guest_motd_flag == 1 ? &v3_server.guest_motd : &v3_server.motd;
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
                if(m->message_id +1 == m->message_num) {
                    // At this point we have our motd, may want to notify the user here :)
                    v3_event *ev;
                    ev = malloc(sizeof(v3_event));
                    memset(ev, 0, sizeof(v3_event));
                    ev->type = V3_EVENT_DISPLAY_MOTD;
                    strncpy(ev->data.motd, *motd, 2047);
                    v3_queue_event(ev);
                }
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                _v3_unlock_server();
                return V3_MALFORMED;
            }
            return V3_OK;/*}}}*/
        case 0x52:/*{{{*/
            if (!_v3_get_0x52(msg)) {
                _v3_destroy_packet(msg);
                _v3_func_leave("_v3_process_message");
                return V3_MALFORMED;
            } else {
                _v3_msg_0x52_0x01_in     *m = (_v3_msg_0x52_0x01_in *)msg->contents;

                v3_event *ev = malloc(sizeof(v3_event));
                memset(ev, 0, sizeof(v3_event));

                switch (m->subtype) {
                    case 0x00:
                        {
                            ev->type = V3_EVENT_USER_TALK_START;
                            ev->user.id = m->user_id;
                            ev->pcm.send_type = m->send_type;
                            ev->pcm.rate = v3_get_codec_rate(m->codec, m->codec_format);
                        }
                        break;
                    case 0x02:
                        {
                            ev->type = V3_EVENT_USER_TALK_END;
                            ev->user.id = m->user_id;
                        }
                        break;
                    case 0x01:
                        {   
                            _v3_msg_0x52_0x01_in *msub = (_v3_msg_0x52_0x01_in *)msg->contents;
                            ev->type = V3_EVENT_PLAY_AUDIO;
                            ev->user.id = m->user_id;
                            ev->pcm.send_type = m->send_type;
                            ev->pcm.rate = v3_get_codec_rate(msub->codec, msub->codec_format);

                            // TODO: it's too messy to have this here.  Write a function that decodes
                            switch (msub->codec) {
                                case 0: // GSM
                                    {
                                        _v3_msg_0x52_gsmdata *gsmdata =  msub->data;
                                        gsm handle;
                                        uint8_t buf[65];
                                        int8_t sample[640];
                                        int ctr;
                                        int one = 1; // used for codec settings

                                        if (!(handle = gsm_create())) {
                                            _v3_error("couldn't create gsm handle");
                                            _v3_destroy_0x52(msg);
                                            _v3_destroy_packet(msg);
                                            _v3_func_leave("_v3_process_message");
                                            free(ev);
                                            return V3_MALFORMED; // it's not really a malformed packet...
                                        }
                                        memset(sample, 0, 640);
                                        gsm_option(handle, GSM_OPT_WAV49, &one);
                                        for (ctr = 0; ctr < msub->data_length / 65; ctr++) {
                                            memcpy(buf, gsmdata->frames[ctr], 65);
                                            if (gsm_decode(handle, buf, (int16_t *)sample) || gsm_decode(handle, buf+33, ((int16_t *)sample)+160)) {
                                                _v3_debug(V3_DEBUG_INFO, "failed to decode gsm frame %d", ctr);
                                                continue;
                                            }
                                            _v3_debug(V3_DEBUG_INFO, "copying 640 bytes of frame %d from %lu to %lu", ctr, sample, ev->data.sample+(ctr*640));
                                            memcpy(ev->data.sample+(ctr*640), sample, 640);
                                        }
                                        ev->pcm.length = msub->data_length/65*640;
                                        _v3_debug(V3_DEBUG_EVENT, "queueing pcm msg length %d", ev->pcm.length);
                                    }
                                    break;
                                case 3: // SPEEX
                                    {
                                        char  cbits[200];
                                        void *state;
                                        SpeexBits bits;
                                        int frame_size;
                                        int ctr;

                                        _v3_msg_0x52_speexdata *speexdata = msub->data;
                                        // The frame size as a uint16_t is prepended to
                                        // every frame, but they're always the same.
                                        // The length of the data divided by the count
                                        // is the actual frame size in the packet.  Then
                                        // subtract two for extra int16 specifying length
                                        frame_size = msub->data_length / speexdata->frame_count - 2;
                                        switch (ev->pcm.rate) {
                                            case 8000:
                                                state = speex_decoder_init(&speex_nb_mode);
                                                break;
                                            case 16000:
                                                state = speex_decoder_init(&speex_wb_mode);
                                                break;
                                            case 32000:
                                                state = speex_decoder_init(&speex_uwb_mode);
                                                break;
                                            default:
                                                _v3_debug(V3_DEBUG_INFO, "received unknown speex pcm rate %d", ev->pcm.rate);
                                                _v3_destroy_0x52(msg);
                                                _v3_destroy_packet(msg);
                                                free(ev);
                                                return V3_MALFORMED;
                                        }
                                        speex_bits_init(&bits);
                                        for (ctr = 0; ctr < speexdata->frame_count; ctr++) {
                                            memcpy(cbits, speexdata->frames[ctr] + 2, frame_size);
                                            speex_bits_read_from(&bits, cbits, frame_size);
                                            speex_decode_int(state, &bits, ((int16_t *)ev->data.sample)+ctr*speexdata->sample_size);
                                        }
                                        ev->pcm.length  = speexdata->frame_count * speexdata->sample_size * sizeof(int16_t);
                                        _v3_debug(V3_DEBUG_EVENT, "queueing pcm msg length %d", ev->pcm.length);
                                        speex_decoder_destroy(state);
                                        speex_bits_destroy(&bits);
                                    }
                                    break;
                            }
                        }
                        break;
                }
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
                v3_event *ev;
                v3_user *user;
                _v3_debug(V3_DEBUG_INFO, "user %d moved to channel %d", m->user_id, m->channel_id);
                user = v3_get_user(m->user_id);
                user->channel = m->channel_id;
                _v3_update_user(user);
                v3_free_user(user);
                ev = malloc(sizeof(v3_event));
                memset(ev, 0, sizeof(v3_event));
                ev->type = V3_EVENT_USER_CHAN_MOVE;
                ev->user.id = m->user_id;
                ev->channel.id = m->channel_id;
                v3_queue_event(ev);
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
                v3_server.connected_clients = m->connected_clients;
                v3_server.port = m->port;
                _v3_unlock_server();
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
                v3_event *ev;
                char buf[1024];
                _v3_msg_0x59 *m = msg->contents;
                snprintf(buf, 1024, "%s%s", _v3_errors[m->error], m->message);
                if (m->close_connection) {
                    if (m->minutes_banned) {
                        char buf2[1024];
                        snprintf(buf2, 1024, " You can connect again in %d minutes", m->minutes_banned);
                        strncat(buf, buf2, 1023);
                    }
                    _v3_debug(V3_DEBUG_INTERNAL, "disconnecting from server");
                    _v3_close_connection();
                }
                ev = malloc(sizeof(v3_event));
                memset(ev, 0, sizeof(v3_event));
                ev->type = V3_EVENT_ERROR_MSG;
                ev->error.disconnected = m->close_connection;
                strncpy(ev->error.message, buf, 512);
                v3_queue_event(ev);
                _v3_error(buf);
            }
            _v3_destroy_packet(msg);
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
                v3_user *ul;
                int ctr;

                _v3_lock_luser();
                _v3_lock_userlist();

                ul = calloc(m->user_count, sizeof(v3_user));
                switch (m->subtype) {
                    case V3_REMOVE_USER:
                        _v3_debug(V3_DEBUG_INFO, "removing %d users from user list",  m->user_count);
                        for (ctr = 0; ctr < m->user_count; ctr++) {
                            v3_event *ev;
                            ev = malloc(sizeof(v3_event));
                            memset(ev, 0, sizeof(v3_event));
                            ev->type = V3_EVENT_USER_LOGOUT;
                            ev->user.id = m->user_list[ctr].id;
                            _v3_debug(V3_DEBUG_INFO, "queuing event type %d for user %d", ev->type, ev->user.id);
                            _v3_remove_user(m->user_list[ctr].id);
                            v3_queue_event(ev);
                        }
                        break;
                    case V3_MODIFY_USER:
                    case V3_ADD_USER:
                        _v3_debug(V3_DEBUG_INFO, "adding %d users on user list",  m->user_count);
                        for (ctr = 0; ctr < m->user_count; ctr++) {
                            v3_event *ev;
                            _v3_update_user(&m->user_list[ctr]);
                            ev = malloc(sizeof(v3_event));
                            memset(ev, 0, sizeof(v3_event));
                            ev->type = V3_ADD_USER ? V3_EVENT_USER_LOGIN : V3_EVENT_USER_MODIFY;
                            ev->user.id = m->user_list[ctr].id;
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
                            v3_event *ev;
                            _v3_update_user(&m->user_list[ctr]);
                            ev = malloc(sizeof(v3_event));
                            memset(ev, 0, sizeof(v3_event));
                            ev->type = V3_EVENT_USER_LOGIN;
                            ev->user.id = m->user_list[ctr].id;
                            ev->channel.id = m->user_list[ctr].channel;
                            _v3_debug(V3_DEBUG_INFO, "queuing event type %d for user %d to channel %d", ev->type, ev->user.id, ev->channel.id);
                            v3_queue_event(ev);
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
                    v3_event *ev;
                    _v3_update_channel(&m->channel_list[ctr]);
                    ev = malloc(sizeof(v3_event));
                    memset(ev, 0, sizeof(v3_event));
                    ev->type = V3_EVENT_CHAN_ADD;
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
 * These are functions that are availble via the API for applications to use
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
        return false;
    }
    if (strlen(phonetic) >= 32) {
        _v3_error("Login Failed: Phonetic is too long (max is 32 characters).");
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
        struct hostent hostbuf, *hp;
        size_t hstbuflen;
        char *tmphstbuf;
        int res;
        int herr;

        _v3_status(5, "Looking up hostname for %s", srvname);
        hstbuflen = 1024;
        tmphstbuf = malloc (hstbuflen);

        while ((res = gethostbyname_r (srvname, &hostbuf, tmphstbuf, hstbuflen, &hp, &herr)) == ERANGE) {
            /* Enlarge the buffer.  */
            hstbuflen *= 2;
            tmphstbuf = realloc (tmphstbuf, hstbuflen);
        }
        if (res || hp == NULL || hp->h_length < 1) {
            _v3_error("Hostname lookup failed.");
            _v3_func_leave("v3_login");
            return false;
        }
        memcpy(&srvip.s_addr, hp->h_addr_list[0], sizeof(srvip.s_addr));
        free(tmphstbuf);
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
    if (pipe(v3_server.evpipe)) {
        _v3_error("could not create outbound event queue");
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
        return false;
    }

    /*
       Create a TCP connection to the server
     */
    if (!_v3_login_connect(&srvip, (uint16_t) v3_server.port)) {
        _v3_status(0, "Not connected.");
        return false;
    }
    _v3_status(20, "Connected to %s... exchanging encryption keys", inet_ntoa(srvip));
    gettimeofday(&v3_server.last_timestamp, NULL);
    /*
       At this point, we have a TCP connection to the server.  It's time to
       start the authentication process, first start with the key exchange
     */
    _v3_server_key_exchange();

    // Wait for the info message to come back
    msg = _v3_recv(V3_BLOCK);
    if (msg->type != 0x57) {
        _v3_debug(V3_DEBUG_INTERNAL, "expected message type 0x57, received type %02X", msg->type);
        return false;
    }
    switch (_v3_process_message(msg)) {
        case V3_MALFORMED:
            _v3_debug(V3_DEBUG_INTERNAL, "received malformed packet or unimplemented subtype");
            break;
        case V3_NOTIMPL:
            _v3_debug(V3_DEBUG_INTERNAL, "packet type not implemented -- if we ignore it, maybe it'll go away");
            break;
        case V3_OK:
            _v3_debug(V3_DEBUG_INTERNAL, "packet processed");
            break;
    }

    /*
     * Message type 0x48 is the actual login message.  This sends the username,
     * password, phonetic, etc.  Process all responses up to (and including)
     * 0x34, which is the key scramble
     */
    _v3_status(30, "Connected to %s... logging in", inet_ntoa(srvip));
    msg = _v3_put_0x48();
    _v3_send(msg);
    _v3_destroy_packet(msg);
    do {
        if ((msg = _v3_recv(V3_BLOCK)) == NULL) {
            _v3_error("recv() failed");
            return false;
        }
        type = msg->type;
        switch (type) {
            case 0x4a:
                _v3_status(40, "Receiving User Permissions.");
                break;
            case 0x5d:
                _v3_status(50, "Receiving User List.");
                break;
            case 0x60:
                _v3_status(60, "Receiving Channel List.");
                break;
            case 0x46:
                _v3_status(70, "Receiving User Settings.");
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
        }
        // 0x59 is a error message that will be received if you are banned
        // 0x06 is a specific authentication error message
        if (type == 0x59 || type == 0x06) {
            return false;
        }
    } while (type != 0x34);
    /*
     * After we recive the key scramble packet, if we're still connected, we
     * should be logged in.  We need t respond to the 0x5c message we
     * previously ignored and send our user settings to the server.  At that
     * point, normal packet processing can begin.
     */
    if (v3_is_loggedin()) {
        _v3_net_message *response;

        _v3_status(90, "Scraming Encryption Table.");
        response = _v3_put_0x5c(0);
        _v3_send(response);
        _v3_destroy_packet(response);

        _v3_status(95, "Sending User Settings.");
        response = _v3_put_0x46(V3_USER_ACCEPT_PAGES, v3_luser.accept_pages);
        _v3_send(response);
        _v3_destroy_packet(response);

        //response = _v3_put_0x46(V3_USER_ACCEPT_U2U, v3_luser.accept_u2u);
        response = _v3_put_0x46(V3_USER_ACCEPT_U2U, 1);
        _v3_send(response);
        _v3_destroy_packet(response);

        response = _v3_put_0x46(V3_USER_ACCEPT_CHAT, v3_luser.accept_chat);
        _v3_send(response);
        _v3_destroy_packet(response);

        response = _v3_put_0x46(V3_USER_ALLOW_RECORD, v3_luser.allow_recording);
        _v3_send(response);
        _v3_destroy_packet(response);

        _v3_status(100, "Login Complete.");
        {
            v3_event *ev;
            ev = malloc(sizeof(v3_event));
            memset(ev, 0, sizeof(v3_event));
            ev->type = V3_EVENT_LOGIN_COMPLETE;
            v3_queue_event(ev);
        }

        return true;
    }
    return false;
}/*}}}*/

int
_v3_logout(void) {/*{{{*/
    v3_event *ev;

    _v3_func_enter("_v3_logout");

    // notify the client that they are disconnected
    ev = malloc(sizeof(v3_event));
    memset(ev, 0, sizeof(v3_event));
    ev->type = V3_EVENT_DISCONNECT;
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
    _v3_destroy_channellist();
    _v3_destroy_userlist();
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

int
v3_set_text(char *comment, char *url, char *integration_text, uint8_t silent) {/*{{{*/
    _v3_net_message *msg;
    v3_user *user;

    _v3_func_enter("v3_set_text");

    // TODO: this needs to be implemented using the queue
    user = v3_get_user(v3_luser.id);
    free(user->name);
    free(user->phonetic);
    free(user->comment);
    free(user->url);
    free(user->integration_text);
    user->name = strdup("");
    user->phonetic = strdup("");
    user->comment = strdup(comment);
    user->url = strdup(url);
    user->integration_text = strdup(integration_text);
    _v3_debug(V3_DEBUG_INFO, "setting cm: %s, url: %s, integration: %s", comment, url, integration_text);
    msg = _v3_put_0x5d(V3_MODIFY_USER, 1, user);
    if (_v3_send(msg)) {
        v3_free_user(user);
        _v3_destroy_packet(msg);
        _v3_func_leave("v3_set_text");
        return true;
    } else {
        v3_free_user(user);
        _v3_debug(V3_DEBUG_INFO, "send failed: ", _v3_error(NULL));
        _v3_func_leave("v3_set_text");
        _v3_destroy_packet(msg);
        return false;
    }
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

v3_user *
v3_get_user(uint16_t id) {/*{{{*/
    v3_user *u, *ret_user;

    _v3_lock_userlist();
    for (u = v3_user_list; u != NULL; u = u->next) {
        if (u->id == id) {
            ret_user = malloc(sizeof(v3_user));
            _v3_copy_user(ret_user, u);
            _v3_unlock_userlist();
            return ret_user;
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

int
v3_queue_event(v3_event *ev) {/*{{{*/
    v3_event *last;
    int len = 0;

    _v3_func_enter("v3_queue_event");
    if (eventq_mutex == NULL) {
        // do not queue events if the mutex hasn't been initialized.  This
        // means the client isn't ready (or doesn't want) to receive events
        free(ev);
        _v3_debug(V3_DEBUG_EVENT, "client does appear to be listening yet... not queueing");
        _v3_func_leave("v3_queue_event");
        return true;
    }
    pthread_mutex_lock(eventq_mutex);
    ev->next = NULL;
    ev->timestamp = time(NULL);
    // if this returns null, there's no events in the queue
    if ((last = _v3_get_last_event(&len)) == NULL) {
        _v3_eventq = ev;
        pthread_cond_signal(eventq_cond);
        pthread_mutex_unlock(eventq_mutex);
        _v3_debug(V3_DEBUG_EVENT, "queued event type %d.  now have 1 event in queue", ev->type);
        _v3_func_leave("v3_queue_event");
        return true;
    }
    // otherwise, tack it on to the end
    last->next = ev;
    pthread_mutex_unlock(eventq_mutex);
    _v3_debug(V3_DEBUG_EVENT, "queued event type %d.  now have %d events in queue", ev->type, len);
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
v3_clear_events(void) {/*{{{*/
    v3_event *ev;
    if (_v3_eventq == NULL) {
        return;
    }
    while (_v3_eventq != NULL) {
        ev = _v3_eventq->next;
        free(_v3_eventq);
        _v3_eventq = ev;
    }
    return;
}/*}}}*/

int
v3_get_max_clients(void) {/*{{{*/
    // - 1 for the lobby user
    return v3_server.max_clients;
}/*}}}*/

uint32_t
v3_get_codec_rate(uint16_t codec, uint16_t format) {/*{{{*/
    int ctr;

    for (ctr = 0; v3_codecs[ctr].codec != -1; ctr++) {
        if (v3_codecs[ctr].codec == codec && v3_codecs[ctr].format == format) {
            return v3_codecs[ctr].rate;
        }
    }
}/*}}}*/

const v3_codec*
v3_get_codec(uint16_t codec, uint16_t format) {/*{{{*/
    int ctr;

    for (ctr = 0; v3_codecs[ctr].codec != -1; ctr++) {
        if (v3_codecs[ctr].codec == codec && v3_codecs[ctr].format == format) {
            return &v3_codecs[ctr];
        }
    }
    return NULL;
}/*}}}*/

const v3_codec*
v3_get_channel_codec(uint16_t channel_id) {/*{{{*/
    v3_channel *c;
    const v3_codec *codec_info;

    if (channel_id == 0) { // the lobby is always the default codec
        return v3_get_codec(v3_server.codec, v3_server.codec_format);
    }
    c = v3_get_channel(channel_id);
    if (c->channel_codec == -1 || c->channel_format == -1) {
        codec_info = v3_get_codec(c->channel_codec, c->channel_format);
    } else {
        codec_info = v3_get_codec(v3_server.codec, v3_server.codec_format);
    }
    v3_free_channel(c);
    return codec_info;
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
v3_send_audio(uint16_t send_type, uint32_t rate, uint8_t *pcm, uint32_t length) {/*{{{*/
    v3_event ev;

    _v3_func_enter("v3_send_audio");
    if (!v3_is_loggedin()) {
        _v3_func_leave("v3_send_audio");
        return;
    }
    memset(&ev, 0, sizeof(v3_event));
    ev.type = V3_EVENT_PLAY_AUDIO;
    ev.pcm.rate = rate;
    ev.pcm.length = length;
    memcpy(ev.data.sample, pcm, length);
    _v3_lock_sendq();
    _v3_debug(V3_DEBUG_EVENT, "sending %lu bytes to event pipe for event type %d", sizeof(v3_event), ev.type);
    if (fwrite(&ev, sizeof(struct _v3_event), 1, v3_server.evoutstream) != 1) {
        _v3_error("could not write to event pipe");
    }
    fflush(v3_server.evoutstream);
    _v3_unlock_sendq();
    _v3_func_leave("v3_send_audio");
    return;
}/*}}}*/

/*
   struct  v3_channel **v3_channel_list(void);
   struct  v3_channel *v3_channel_info_by_name(char *channel_name);
   struct  v3_channel *v3_channel_info_by_id(int channel_id);
   int     v3_channel_count(void);

   struct  v3_user **v3_user_list();
   struct  v3_user *v3_user_info_by_name(char *user_name);
   struct  v3_user *v3_user_info_by_id(int user_id);
   int     v3_user_count(void);

   int     v3_send_audio(void);

   int     v3_register_event_handler(uint16_t event_type, void *event_handler);
 */
