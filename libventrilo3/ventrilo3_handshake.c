/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate$
 * $Revision$
 * $LastChangedBy$
 * $URL$
 */

/*
    Copyright 2008 Luigi Auriemma

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

    http://www.gnu.org/licenses/gpl-2.0.txt
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "ventrilo_algo.h"

#ifdef WIN32
    #include <winsock.h>

    #define close   closesocket
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <netdb.h>
#endif


#ifndef _UINTS_
#define _UINTS_
typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
#endif

void    _v3_debug(uint32_t level, const char *format, ...);


#define V3HVER      "0.2"
#define V3HBUFFSZ   0x200

#ifdef V3HPROXY
    #define V3HPROXY_PARS   , "", "", 0
#else
    #define V3HPROXY_PARS
#endif



typedef struct {
    u32     key;
    u8      *host;
    u16     port;
#ifdef V3HPROXY
    u8      handshake_key[64];
    u8      handshake[16];
    int     ok;
#endif
} ventrilo3_auth_t;

static ventrilo3_auth_t ventrilo3_auth[] = {
    { 0x48332e1f, (uint8_t *)"72.51.46.31",   6100 V3HPROXY_PARS },
    { 0x4022b2b2, (uint8_t *)"64.34.178.178", 6100 V3HPROXY_PARS },
    { 0x3dc24a36, (uint8_t *)"74.54.61.194",  6100 V3HPROXY_PARS },
    { 0x46556ef2, (uint8_t *)"70.85.110.242", 6100 V3HPROXY_PARS },
    { 0,          (uint8_t *)NULL,            0    V3HPROXY_PARS }
};



int ventrilo3_hdr_udp(int type, u8 *buff, u8 *pck);
int ventrilo3_send_udp(int sd, u32 key, u32 ip, u16 port, u8 *data, int len);
int ventrilo3_recv_udp(int sd, ventrilo3_auth_t *vauth, u8 *data, int maxsz, int *handshake_num);
int getbe(u8 *data, u32 *ret, int bits);
int putbe(u8 *data, u32 num, int bits);
int v3timeout(int sock, int secs);



void ventrilo3_algo_scramble(ventrilo_key_ctx *ctx, u8 *v3key) {
    int     i,
            keylen;
    u8      *key;

    _v3_func_enter("ventrilo3_algo_scramble");
    key = ctx->key;
    keylen = ctx->size;
    for(i = 64; i < keylen; i++) {
        v3key[i] = i + keylen;
    }
    for(i = 0; i < keylen; i++) {
        key[i] += v3key[i];
        if(!key[i]) key[i] = i + 36;
    }
    ctx->pos = 0;
    _v3_func_leave("ventrilo3_algo_scramble");
}



int ventrilo3_handshake(u32 ip, u16 port, u8 *handshake, int *handshake_num, u8 *handshake_key) {
    struct  linger  ling = {1,1};
    int     sd;
    int     i,
            len;
    u8      sbuff[V3HBUFFSZ],
            rbuff[V3HBUFFSZ];

    _v3_debug(V3_DEBUG_STACK, " -> ventrilo3_handshake(%d, %d, NULL, tmp, NULL)", ip, port);
    sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sd < 0) return(-1);
    setsockopt(sd, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof(ling));

    memset(rbuff, 0, 200);
    ventrilo3_hdr_udp(4, sbuff, rbuff);
    putbe(sbuff + 0xc,  2,         8);
    putbe(sbuff + 0x10, 1,         16); // useless
    putbe(sbuff + 0x12, time(NULL),16); // rand useless number

    ventrilo3_send_udp(sd, 0, ip, port, sbuff, 200);
    len = ventrilo3_recv_udp(sd, NULL, rbuff, V3HBUFFSZ, handshake_num);
    if(len < 0) goto quit;

    for(i = 0; ventrilo3_auth[i].key; i++) {
        _v3_debug(V3_DEBUG_INFO, "sending auth packet to %s:%d", ventrilo3_auth[i].host, ventrilo3_auth[i].port);
        len = ventrilo3_hdr_udp(5, sbuff, rbuff);
        ventrilo3_send_udp(sd, ventrilo3_auth[i].key, inet_addr((char *)ventrilo3_auth[i].host), ventrilo3_auth[i].port, sbuff, len);
    }

    for(;;) {
        len = ventrilo3_recv_udp(sd, (void *)&ventrilo3_auth, rbuff, V3HBUFFSZ, handshake_num);
        _v3_debug(V3_DEBUG_INFO, "received auth response from %s:%d", ventrilo3_auth[*handshake_num].host, ventrilo3_auth[*handshake_num].port);
        if(len < 0) break;
        if(!len) continue;
        if(len < (0x5c + 16)) continue;
#ifdef V3HPROXY
        memcpy(ventrilo3_auth[*handshake_num].handshake_key, rbuff + 0x1c, 64);
        memcpy(ventrilo3_auth[*handshake_num].handshake,     rbuff + 0x5c, 16);
        ventrilo3_auth[*handshake_num].ok = 1;
        _v3_debug(V3_DEBUG_STACK, " <- ventrilo3_handshake() (proxy)");
#else
        memcpy(handshake_key, rbuff + 0x1c, 64);
        memcpy(handshake,     rbuff + 0x5c, 16);
        close(sd);
        _v3_debug(V3_DEBUG_STACK, " <- ventrilo3_handshake(%d, %d, handshake, %d, handshake_key)", ip, port, *handshake_num);
        return(0);
#endif
    }

quit:
    _v3_debug(V3_DEBUG_STACK, " <- ventrilo3_handshake()");
    close(sd);
#ifdef V3HPROXY
    return(0);
#else
    return(-1);
#endif
}



int ventrilo3_hdr_udp(int type, u8 *buff, u8 *pck) {
    u8      ecx;

    memset(buff, 0, 0x200); // no, I'm not mad, this is EXACTLY what Ventrilo does

    switch(type - 1) {
        case 0: ecx = 0xb4; break;
        case 1: ecx = 0x70; break;
        case 2: ecx = 0x24; break;
        case 3: ecx = 0xb8; break;
        case 4: ecx = 0x74; break;
        case 5: ecx = 0x5c; break;
        case 6: ecx = 0xd0; break;
        case 7: ecx = 0x08; break;
        case 8: ecx = 0x50; break;
        default: ecx = 0;   break;
    }
    ecx += 0x10;

    putbe(buff + 8,  type,   16);
    putbe(buff + 10, ecx,    16);
    buff[4] = 'U';
    buff[5] = 'D';
    buff[6] = 'C';
    buff[7] = 'L';
    buff[0xc] = 1;
    putbe(buff + 0x10, 0xb401, 32);         // x[seq], recheck
    putbe(buff + 0x14, getbe(pck + 0x14, NULL, 32), 32);
    putbe(buff + 0x18, getbe(pck + 0x18, NULL, 32), 32);
    putbe(buff + 0x1c, getbe(pck + 0x1c, NULL, 32), 32);
    putbe(buff + 0x20, getbe(pck + 0x20, NULL, 32), 32);
    putbe(buff + 0x24, getbe(pck + 0x28, NULL, 32), 32);
    buff[0x28] = pck[0x30];
    buff[0x29] = 0;
    buff[0x2a] = 0;
    buff[0x2b] = 0;
    putbe(buff + 0x2c, getbe(pck + 0x24, NULL, 16), 16);
    putbe(buff + 0x2e, 0, 16);
    putbe(buff + 0x30, getbe(pck + 0x14, NULL, 16), 16);
    memcpy(buff + 0x34, pck + 0x38, 16);    // hash
    memcpy(buff + 0x44, pck + 0x88, 32);    // WIN32
    buff[0x63] = 0;
    memcpy(buff + 0x64, pck + 0xa8, 32);    // version
    buff[0x83] = 0;
    return(132);
}



int ventrilo3_send_udp(int sd, u32 key, u32 ip, u16 port, u8 *data, int len) {
    struct  sockaddr_in peer;
    int     i;
    u8      tmp[4];

    tmp[0] = key >> 24;
    tmp[1] = key >> 16;
    tmp[2] = key >> 8;
    tmp[3] = key;
    for(i = 16; i < len; i++) {
        data[i] += tmp[i & 3];
    }

    peer.sin_addr.s_addr = ip;
    peer.sin_port        = htons(port);
    peer.sin_family      = AF_INET;

    //printf(". %s:%hu\n", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
    sendto(sd, data, len, 0, (struct sockaddr *)&peer, sizeof(struct sockaddr_in));
    return(0);
}



int ventrilo3_recv_udp(int sd, ventrilo3_auth_t *vauth, u8 *data, int maxsz, int *handshake_num) {
    struct  sockaddr_in peer;
    u32     key;
    int     len,
            i,
            psz;
    u8      tmp[4];

    if(v3timeout(sd, 2) < 0) return(-1);
    psz = sizeof(struct sockaddr_in);
    len = recvfrom(sd, data, maxsz, 0, (struct sockaddr *)&peer, (socklen_t *)&psz);
    if(len < 0) return(-1);
    if(!vauth) return(len);

    key = 0;
    for(i = 0; vauth[i].key; i++) {
        if(inet_addr((char *)vauth[i].host) == peer.sin_addr.s_addr) {
            key = vauth[i].key;
            break;
        }
    }
    if(!key) return(0);

    *handshake_num = i;
    tmp[0] = key >> 24;
    tmp[1] = key >> 16;
    tmp[2] = key >> 8;
    tmp[3] = key;
    for(i = 16; i < len; i++) {
        data[i] -= tmp[i & 3];
    }
    return(len);
}



int getbe(u8 *data, u32 *ret, int bits) {
    u32     num;
    int     i,
            bytes;

    bytes = bits >> 3;
    for(num = i = 0; i < bytes; i++) {
        num |= (data[i] << ((bytes - 1 - i) << 3));
    }
    if(!ret) return(num);
    *ret = num;
    return(bytes);
}



int putbe(u8 *data, u32 num, int bits) {
    int     i,
            bytes;

    bytes = bits >> 3;
    for(i = 0; i < bytes; i++) {
        data[i] = (num >> ((bytes - 1 - i) << 3)) & 0xff;
    }
    return(bytes);
}



int v3timeout(int sock, int secs) {
    struct  timeval tout;
    fd_set  fd_read;

    tout.tv_sec  = secs;
    tout.tv_usec = 0;
    FD_ZERO(&fd_read);
    FD_SET(sock, &fd_read);
    if(select(sock + 1, &fd_read, NULL, NULL, &tout)
      <= 0) return(-1);
    return(0);
}

