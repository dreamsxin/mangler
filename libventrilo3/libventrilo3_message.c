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

#include "libventrilo3_message.h"
#include "ventrilo3.h"

#define V3_CLIENT_VERSION "3.0.5"
#define V3_PROTO_VERSION "3.0.0"

#define true    1
#define false   0

extern _v3_server v3_server;
extern _v3_luser v3_luser;
extern uint32_t _v3_hash_table[];

/*
 *  The following are utility functions used to convert data from a raw
 *  received packet into a reasonable value.  The string functions here
 *  will allocate memory when called, so any returned string must be
 *  free()'d later.
 */

// Functions to retreive a specific data type from a packet /*{{{*/
/*
 * Get a string from the packet whose length is identified by the first 2
 * bytes.  For example, a packet that contains:
 *
 *   04 62 6c 61 68             .blah
 *
 * Will allocate 5 bytes of memory (4 for the string + null) and copy the
 * next 4 bytes into that memory.
 *
 * Parameters:
 *     uint8_t *data            This is a pointer to the location of the
 *                           length (as a 2 byte int) of the packet.  The
 *                           actual string is assumed to be immediately
 *                           following.
 *     int *len              A pointer to an integer.  The number of bytes read
 *                           from the packet will be placed in this variable
 *
 * Returns:                  A pointer to a copy of the string.  The string
 *                           will be null terminated.
 */
char *
_v3_get_msg_string(void *offset, uint16_t *len) {/*{{{*/
    char *s;

    _v3_func_enter("_v3_get_msg_string");
    memcpy(len, offset, 2);
    *len = ntohs(*len);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "getting %d (0x%04X) byte string", *len, *len);
    s = malloc(*len+1);
    memset(s, 0, *len+1);
    memcpy(s, offset+2, *len);
    s[*len] = '\0';
    *len+=2;
    _v3_func_leave("_v3_get_msg_string");
    return s;
}/*}}}*/

int
_v3_put_msg_string(void *buffer, char *string) {/*{{{*/
    int len;
    
    _v3_func_enter("_v3_put_msg_string");
    len = htons((uint16_t)strlen(string));
    memcpy(buffer, &len, 2);
    memcpy(buffer+2, string, strlen(string));
    _v3_func_leave("_v3_put_msg_string");
    return strlen(string) + 2;
}/*}}}*/

int
_v3_get_msg_channel(void *offset, _v3_msg_channel *channel) {/*{{{*/
    uint16_t len;
    void *start_offset = offset;

    _v3_func_enter("_v3_get_msg_channel");
    // get the channel information
    memcpy(channel, offset, 48);
    offset+=48;

    channel->name = (char *)_v3_get_msg_string(offset, &len);
    offset+=len;
    channel->phonetic = (char *)_v3_get_msg_string(offset, &len);
    offset+=len;
    channel->comment = (char *)_v3_get_msg_string(offset, &len);
    offset+=len;
    _v3_func_leave("_v3_get_msg_channel");
    return(offset-start_offset);
}/*}}}*/

int
_v3_get_msg_user(void *offset, _v3_msg_user *user) {/*{{{*/
    uint16_t len;
    void *start_offset = offset;

    _v3_func_enter("_v3_get_msg_user");
    // get the user information
    memcpy(user, offset, 8);
    offset+=8;

    user->name = (char *)_v3_get_msg_string(offset, &len);
    offset+=len;
    user->phonetic = (char *)_v3_get_msg_string(offset, &len);
    offset+=len;
    user->comment = (char *)_v3_get_msg_string(offset, &len);
    offset+=len;
    user->integration_text = (char *)_v3_get_msg_string(offset, &len);
    offset+=len;
    user->url = (char *)_v3_get_msg_string(offset, &len);
    offset+=len;
    _v3_func_leave("_v3_get_msg_user");
    return(offset-start_offset);
}/*}}}*/

int
_v3_put_msg_user(void *buffer, v3_user *user) {/*{{{*/
    void *start_buffer = buffer;

    _v3_func_enter("_v3_put_msg_user");
    // put the user information
    _v3_debug(V3_DEBUG_PACKET_PARSE, "putting user id: %d", user->id);
    memcpy(buffer, user, 8);
    buffer+=8;

    // put the user strings
    buffer += _v3_put_msg_string(buffer, user->name);
    buffer += _v3_put_msg_string(buffer, user->phonetic);
    buffer += _v3_put_msg_string(buffer, user->comment);
    buffer += _v3_put_msg_string(buffer, user->integration_text);
    buffer += _v3_put_msg_string(buffer, user->url);

    _v3_func_leave("_v3_put_msg_user");
    return(buffer-start_buffer);
}/*}}}*/
/*}}}*/

/*
 * These functions parse or create the various message types.  For all
 * functions, the "data" member is an array of bytes either received from or
 * suitable to send to the server.  The "contents" member is the a properly
 * formatted structure of data.  For some packet types, both data and contents
 * will point to the same memory location (i.e. for static length messages).
 *
 * _v3_get_* functions accept a net message structure as a parameter and fill
 * in the contents member (allocating memory if needed).
 *
 * _v3_put_* functions accept the data needed to fill in the packet and return
 * a newly allocated net message structure with the data and contents members
 * filled in.
 *
 * _v3_destroy_* functions will free any memory allocated for a specific packet
 * type.  The message structure itself is *NOT* freed
 */


// Message 0x06 (6)  | AUTHENTICATION/LOGIN ERROR /*{{{*/
int
_v3_get_0x06(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x06 *m;

    _v3_func_enter("_v3_get_0x06");
    m = malloc(sizeof(_v3_msg_0x06));
    memcpy(m, msg->data, 12);
    if(m->subtype & 4) {
        m->encryption_key = malloc(msg->len - 12);
        memcpy(m->encryption_key, msg->data + 12, msg->len - 12);
    } else {
        m->unknown_2 = msg->data[12];
    }
    msg->contents = m;
    _v3_func_leave("_v3_get_0x06");
    return true;
}/*}}}*/

/*}}}*/
// Message 0x37 (55) | PING /*{{{*/
int
_v3_get_0x37(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x37 *m;

    _v3_func_enter("_v3_get_0x37");
    if (msg->len != sizeof(_v3_msg_0x37)) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "expected %d bytes, but message is %d bytes", sizeof(_v3_msg_0x4a), msg->len);
        _v3_func_leave("_v3_get_0x37");
        return false;
    }
    m = msg->contents = msg->data;
    _v3_debug(V3_DEBUG_PACKET_PARSE, "User Permissions:");
    _v3_debug(V3_DEBUG_PACKET_PARSE, "user_id.............: %d",   m->user_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "ping................: %d",   m->ping);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "sequence............: %d",   m->sequence);
    _v3_func_leave("_v3_get_0x37");
    return true;
}/*}}}*/

_v3_net_message *_v3_put_0x00() {
    _v3_net_message *m;
    _v3_msg_0x00 *mc;

    _v3_func_enter("_v3_put_0x00");
    // Build our message
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x00;
    m->len = sizeof(_v3_msg_0x00);

    // Build our message contents
    mc = malloc(sizeof(_v3_msg_0x00));
    memset(mc, 0, sizeof(_v3_msg_0x00));

    mc->type = 0x00;
    strncpy(mc->version, "3.0.0", 16);
    memset(mc->salt1, 0, 32);
    memset(mc->salt2, 0, 32);
    int ctr;
    for(ctr = 0; ctr < 31; ctr++) {
        mc->salt1[ctr] = rand() % 93 + 33;
        mc->salt2[ctr] = rand() % 93 + 33;
    }

    m->contents = mc;
    m->data = (char *)mc;
    _v3_func_leave("_v3_put_0x00");
    return m;
}

_v3_net_message *_v3_put_0x37(int sequence) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x37 *mc;

    _v3_func_enter("_v3_put_0x37");
    // Build our message
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x37;
    m->len = sizeof(_v3_msg_0x37);

    // Build our message contents
    mc = malloc(sizeof(_v3_msg_0x37));
    memset(mc, 0, sizeof(_v3_msg_0x37));

    mc->type = 0x37;
    mc->user_id = v3_luser.id;
    mc->sequence = sequence;
    mc->ping  = v3_luser.ping;
    m->contents = mc;
    m->data = (char *)mc;
    _v3_func_leave("_v3_put_0x37");
    return m;
}/*}}}*/
/*}}}*/
// Message 0x3b (59) | FORCE CHANNEL MOVE /*{{{*/
int
_v3_get_0x3b(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x3b *m;

    _v3_func_enter("_v3_get_0x3b");
    if (msg->len != sizeof(_v3_msg_0x3b)) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "expected %d bytes, but message is %d bytes", sizeof(_v3_msg_0x3b), msg->len);
        _v3_func_leave("_v3_get_0x3b");
        return false;
    }
    m = msg->contents = msg->data;
    _v3_debug(V3_DEBUG_PACKET_PARSE, "Force Channel Move:");
    _v3_debug(V3_DEBUG_PACKET_PARSE, "user id.............: %d",   m->user_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "channel id..........: %d",   m->channel_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "error id............: %d",   m->error_id);
    _v3_func_leave("_v3_get_0x3b");
    return true;
}/*}}}*/
/*}}}*/
// Message 0x3c (60) | CODEC INFORMATION /*{{{*/
int
_v3_get_0x3c(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x3c *m;

    _v3_func_enter("_v3_get_0x3c");
    if (msg->len != sizeof(_v3_msg_0x3c)) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "expected %d bytes, but message is %d bytes", sizeof(_v3_msg_0x3c), msg->len);
        _v3_func_leave("_v3_get_0x3c");
        return false;
    }
    m = msg->contents = msg->data;
    _v3_debug(V3_DEBUG_PACKET_PARSE, "Codec Information:");
    _v3_debug(V3_DEBUG_PACKET_PARSE, "codec...............: %d",   m->codec);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "format..............: %d",   m->codec_format);
    _v3_func_leave("_v3_get_0x3c");
    return true;
}/*}}}*/
/*}}}*/
// Message 0x4b (75) | TIMESTAMP /*{{{*/

_v3_net_message *_v3_put_0x4b(void) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x4b *mc;

    _v3_func_enter("_v3_put_0x4b");
    // Build our message
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x4b;
    m->len = sizeof(_v3_msg_0x4b);

    // Build our message contents
    mc = malloc(sizeof(_v3_msg_0x4b));
    memset(mc, 0, sizeof(_v3_msg_0x4b));

    mc->type = 0x4b;
    mc->timestamp = time(NULL);
    m->contents = mc;
    m->data = (char *)mc;
    _v3_func_leave("_v3_put_0x4b");
    return m;
}/*}}}*/
/*}}}*/

int 
_v3_get_0x42(_v3_net_message *msg) {
    _v3_msg_0x42 *m;
    _v3_func_enter("_v3_get_0x42");
    m = msg->contents = msg->data;
    switch(m->subtype) {
        case 2:
        case 3:
            m->msg = _v3_get_msg_string(msg->data + 12, &m->msglen);
            break;
    }
    _v3_func_leave("_v3_get_0x42");
    return true;
}

_v3_net_message *_v3_put_0x42(uint16_t subtype, uint16_t user_id, char* message) {
    _v3_net_message *m;
    _v3_msg_0x42 *mc;

    _v3_func_enter("_v3_put_0x42");
    // Build our message
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x42;
    
    // Build our message contents
    uint16_t base = sizeof(_v3_msg_0x42) - (sizeof(char *) + sizeof(uint16_t)); 
    uint16_t len  = base;
    mc = malloc(base);
    memset(mc, 0, base);
    mc->type = 0x42;
    mc->subtype = subtype;
    mc->user_id = user_id;
    
    if(message) {        
        len += strlen(message) + 2;
        mc = realloc(mc, len);
        _v3_put_msg_string((char *)mc + base, message);   
    }
    
    m->contents = mc;
    m->data = (char *)mc;
    m->len = len;
    _v3_func_leave("_v3_put_0x42");
    return m;
}

// Message 0x46 (70) | USER OPTIONS /*{{{*/
int
_v3_get_0x46(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x46 *m;

    _v3_func_enter("_v3_get_0x46");
    if (msg->len != sizeof(_v3_msg_0x46)) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "expected %d bytes, but message is %d bytes", sizeof(_v3_msg_0x4a), msg->len);
        _v3_func_leave("_v3_get_0x46");
        return false;
    }
    m = msg->contents = msg->data;
    _v3_debug(V3_DEBUG_PACKET_PARSE, "User Settings:");
    _v3_debug(V3_DEBUG_PACKET_PARSE, "user_id.............: %d",   m->user_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "setting.............: %d",   m->setting);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "value...............: %d",   m->value);
    _v3_func_leave("_v3_get_0x46");
    return true;
}/*}}}*/

_v3_net_message *_v3_put_0x46(uint16_t setting, uint16_t value) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x46 *mc;

    _v3_func_enter("_v3_put_0x46");
    // Build our message
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x46;
    m->len = sizeof(_v3_msg_0x46);

    // Build our message contents
    mc = malloc(sizeof(_v3_msg_0x46));
    memset(mc, 0, sizeof(_v3_msg_0x46));

    mc->type = 0x46;
    mc->user_id = v3_luser.id;
    mc->setting = setting;
    mc->value   = value;
    m->contents = mc;
    m->data = (char *)mc;
    _v3_func_leave("_v3_put_0x46");
    return m;
}/*}}}*/
/*}}}*/
// Message 0x48 (72) | LOGIN /*{{{*/
_v3_net_message *
_v3_put_0x48(void) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x48 *mc;

    _v3_func_enter("_v3_put_0x48");
    // Build our message
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x48;
    m->len = sizeof(_v3_msg_0x48);

    // Build our message contents
    mc = malloc(sizeof(_v3_msg_0x48));
    memset(mc, 0, sizeof(_v3_msg_0x48));

    mc->type = 0x48;
    mc->subtype = 2;
    mc->server_ip = v3_server.ip;
    mc->portmask = v3_server.port ^ 0xffff;
    mc->show_login_name = true;
    mc->auth_server_index = v3_server.auth_server_index;
    memcpy(mc->handshake, v3_server.handshake, 16);
    strncpy(mc->client_version, V3_CLIENT_VERSION, 16);
    strncpy(mc->proto_version,  V3_PROTO_VERSION, 16);
    if (strlen(v3_luser.password)) {
        _v3_hash_password((uint8_t *)v3_luser.password, (uint8_t *)mc->password_hash);
    }
    strncpy(mc->username, v3_luser.name, 32);
    strncpy(mc->phonetic, v3_luser.phonetic, 32);
    strncpy(mc->os, "WIN32", 64);

    m->contents = mc;
    m->data = (char *)mc;
    _v3_func_leave("_v3_put_0x48");
    return m;
}/*}}}*/
/*}}}*/
// Message 0x49 (73) | GET/REQUEST CHANNEL LIST MODIFICATION /*{{{*/
_v3_net_message *
_v3_put_0x49(uint16_t subtype, uint16_t user_id, char *channel_password, _v3_msg_channel *channel) {/*{{{*/
    _v3_net_message *msg;
    struct _v3_net_message_0x49 *msgdata;
    void *offset;

    _v3_func_enter("_v3_put_0x49");
    msg = malloc(sizeof(_v3_net_message));
    memset(msg, 0, sizeof(_v3_net_message));
    msg->type = 0x49;
    switch (subtype) {
        case V3_CHANGE_CHANNEL:
            // TODO: this is messy and should probably be cleaned up

            // this is a standard channel packet minus the pointer bytes for
            // the name, comment, and phonetic
            msg->len = sizeof(_v3_msg_0x49)-sizeof(void *)+sizeof(_v3_msg_channel) - sizeof(void *) * 4;
            _v3_debug(V3_DEBUG_PACKET_PARSE, "allocating %d bytes", msg->len);
            msgdata = malloc(sizeof(_v3_msg_0x49)-sizeof(void *)+sizeof(_v3_msg_channel));
            memset(msgdata, 0, sizeof(_v3_msg_0x49)-sizeof(void *)+sizeof(_v3_msg_channel));
            msgdata->type = msg->type;
            msgdata->subtype = V3_CHANGE_CHANNEL;
            msgdata->user_id = user_id;
            if (channel_password != NULL && strlen(channel_password) != 0) {
                _v3_hash_password((uint8_t *)channel_password, (uint8_t *)msgdata->hash_password);
            }
            offset = (void*)((char *)msgdata + sizeof(_v3_msg_0x49)-sizeof(void *));
            memcpy(offset, channel, sizeof(_v3_msg_channel));
            msg->data = (char *)msgdata;
            _v3_func_leave("_v3_put_0x49");
            return(msg);
        default:
            _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown channel message subtype: %02X", subtype);
            _v3_func_leave("_v3_put_0x49");
            return NULL;
    }
    return NULL;
}/*}}}*/

int
_v3_get_0x49(_v3_net_message *msg) {/*{{{*/
    /*
     * PACKET: message type: 0x49 (73)
     * PACKET: data length : 98
     * PACKET:     49 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00      I...............
     * PACKET:     00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00      ................
     * PACKET:     00 00 00 00 00 00 00 00 16 00 02 00 00 00 00 00      ................
     * PACKET:     01 00 01 00 01 00 01 00 01 00 01 00 00 00 00 00      ................
     * PACKET:     00 00 00 00 01 00 00 00 01 00 00 00 00 00 00 00      ................
     * PACKET:     00 00 00 00 00 00 00 00 00 04 74 65 73 74 00 00      ..........test..
     * PACKET:     00 00                                                ..
     */
    _v3_msg_0x49 *m;

    _v3_func_enter("_v3_get_0x49");
    m = malloc(sizeof(_v3_msg_0x49));
    memcpy(m, msg->data, sizeof(_v3_msg_0x49) - sizeof(void *));
    m->channel = malloc(sizeof(v3_channel));
    _v3_get_msg_channel(msg->data+sizeof(_v3_msg_0x49) - sizeof(void *), m->channel);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "got channel: id: %d | parent: %d | name: %s | phonetic: %s | comment: %s",
            m->channel->id,
            m->channel->parent,
            m->channel->name,
            m->channel->phonetic,
            m->channel->comment
            );
    msg->contents = m;
    _v3_func_leave("_v3_get_0x49");
    return true;
}/*}}}*/

/*}}}*/
// Message 0x4a (74) | USER PERMISSIONS /*{{{*/
int
_v3_get_0x4a(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x4a *m;

    _v3_func_enter("_v3_get_0x4a");
    if (msg->len != sizeof(_v3_msg_0x4a)) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "expected %d bytes, but message is %d bytes", sizeof(_v3_msg_0x4a), msg->len);
        _v3_func_leave("_v3_get_0x4a");
        return false;
    }
    m = msg->contents = msg->data;
    _v3_debug(V3_DEBUG_PACKET_PARSE, "User Permissions:");
    _v3_debug(V3_DEBUG_PACKET_PARSE, "lock_acct...........: %d",   m->lock_acct);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "dfl_chan............: %d",   m->dfl_chan);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "dupe_ip.............: %d",   m->dupe_ip);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "switch_chan.........: %d",   m->switch_chan);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "in_reserve_list.....: %d",   m->in_reserve_list);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_1......: %d",   m->unknown_perm_1);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_2......: %d",   m->unknown_perm_2);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_3......: %d",   m->unknown_perm_3);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "recv_bcast..........: %d",   m->recv_bcast);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "add_phantom.........: %d",   m->add_phantom);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "record..............: %d",   m->record);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "recv_complaint......: %d",   m->recv_complaint);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "send_complaint......: %d",   m->send_complaint);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "inactive_exempt.....: %d",   m->inactive_exempt);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_4......: %d",   m->unknown_perm_4);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_5......: %d",   m->unknown_perm_5);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "srv_admin...........: %d",   m->srv_admin);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "add_user............: %d",   m->add_user);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "del_user............: %d",   m->del_user);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "ban_user............: %d",   m->ban_user);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "kick_user...........: %d",   m->kick_user);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "move_user...........: %d",   m->move_user);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "assign_chan_admin...: %d",   m->assign_chan_admin);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "edit_rank...........: %d",   m->edit_rank);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "edit_motd...........: %d",   m->edit_motd);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "edit_guest_motd.....: %d",   m->edit_guest_motd);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "issue_rcon_cmd......: %d",   m->issue_rcon_cmd);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "edit_voice_target; .: %d",   m->edit_voice_target);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "edit_command_target.: %d",   m->edit_command_target);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "assign_rank.........: %d",   m->assign_rank);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "assign_reserved.....: %d",   m->assign_reserved);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_6......: %d",   m->unknown_perm_6);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_7......: %d",   m->unknown_perm_7);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_8......: %d",   m->unknown_perm_8);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_9......: %d",   m->unknown_perm_9);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_10.....: %d",   m->unknown_perm_10);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "bcast...............: %d",   m->bcast);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "bcast_lobby.........: %d",   m->bcast_lobby);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "bcast_user..........: %d",   m->bcast_user);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "bcast_x_chan........: %d",   m->bcast_x_chan);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "send_tts_bind.......: %d",   m->send_tts_bind);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "send_wav_bind.......: %d",   m->send_wav_bind);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "send_page...........: %d",   m->send_page);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "send_comment........: %d",   m->send_comment);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "set_phon_name.......: %d",   m->set_phon_name);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "gen_comment_snds....: %d",   m->gen_comment_snds);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "event_snds..........: %d",   m->event_snds);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "mute_glbl...........: %d",   m->mute_glbl);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "mute_other..........: %d",   m->mute_other);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "glbl_chat...........: %d",   m->glbl_chat);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "start_priv_chat.....: %d",   m->start_priv_chat);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_11.....: %d",   m->unknown_perm_11);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "eq_out..............: %d",   m->eq_out);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_12.....: %d",   m->unknown_perm_12);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_13.....: %d",   m->unknown_perm_13);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_14.....: %d",   m->unknown_perm_14);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "see_guest...........: %d",   m->see_guest);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "see_nonguest........: %d",   m->see_nonguest);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "see_motd............: %d",   m->see_motd);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "see_srv_comment; ...: %d",   m->see_srv_comment);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "see_chan_list.......: %d",   m->see_chan_list);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "see_chan_comment....: %d",   m->see_chan_comment);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "see_user_comment....: %d",   m->see_user_comment);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown_perm_15.....: %d",   m->unknown_perm_15);
    _v3_func_leave("_v3_get_0x4a");
    return true;
}/*}}}*/
/*}}}*/
// Message 0x50 (80) | MOTD /*{{{*/
int
_v3_get_0x50(_v3_net_message *msg) {/*{{{*/
    _v3_func_enter("_v3_get_0x50");
    msg->contents = msg->data;
    _v3_func_leave("_v3_get_0x50");
    return true;
}/*}}}*/
//* }}} */
// Message 0x52 (82) | SOUND DATA /*{{{*/
int
_v3_get_0x52(_v3_net_message *msg) {/*{{{*/
    int ctr, offset;
    _v3_msg_0x52 *m;

    _v3_func_enter("_v3_get_0x52");
    m = malloc(sizeof(_v3_msg_0x52));
    memcpy(m, msg->data, sizeof(_v3_msg_0x52));
    _v3_debug(V3_DEBUG_PACKET_PARSE, "subtype.......: %d", m->subtype);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "user_id.......: %d", m->user_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "codec info....: %d/%d", m->codec, m->codec_format);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "send_type.....: %d", m->send_type);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown 1.....: %d", m->unknown_1);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "data length...: %d", m->data_length);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "pcm length....: %d", m->pcm_length);
    switch (m->subtype) {
        case 0x00:
            {
                _v3_msg_0x52_0x00 *msub = (_v3_msg_0x52_0x00 *)m;
                msub = realloc(m, sizeof(_v3_msg_0x52_0x00));
                memcpy(msub, msg->data, sizeof(_v3_msg_0x52_0x00));
                _v3_debug(V3_DEBUG_PACKET_PARSE, "user %d started transmitting", msub->header.user_id);
                msg->contents = msub;
                _v3_func_leave("_v3_get_0x52_0x00");
                return true;
            }
        case 0x01:
            {
                _v3_msg_0x52_0x01_in *msub = (_v3_msg_0x52_0x01_in *)m;
                msub = realloc(m, sizeof(_v3_msg_0x52_0x01_in));
                memcpy(msub, msg->data, sizeof(_v3_msg_0x52_0x01_in));
                _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown 4.....: %d", msub->unknown_4);
                _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown 5.....: %d", msub->unknown_5);
                _v3_debug(V3_DEBUG_PACKET_PARSE, "allocating %d bytes for audio packet", sizeof(_v3_msg_0x52_0x01_in));
                _v3_debug(V3_DEBUG_PACKET_PARSE, "received an audio packet from user id %d", msub->header.user_id);
                switch (msub->header.codec) {
                    case 0x00: // GSM
                        if (msub->header.codec_format > 3) {
                            // this should always be <= than 3, otherwise, bail out
                            free(msub);
                            _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown gsm codec format 0x%02X", msub->header.codec_format);
                            _v3_func_leave("_v3_get_0x52 (0x01_in gsm)");
                            return false;
                        } else {
                            _v3_msg_0x52_gsmdata *data;
                            data = malloc(sizeof(_v3_msg_0x52_gsmdata));
                            _v3_debug(V3_DEBUG_PACKET_PARSE, "gsm length..........: %d (%d frames)", msub->header.data_length, msub->header.data_length/65);
                            _v3_debug(V3_DEBUG_PACKET_PARSE, "allocating %d bytes for pointers", msub->header.data_length / 65 * sizeof(uint8_t *));
                            data->frames = malloc(msub->header.data_length / 65 * sizeof(uint8_t *));
                            for (ctr = 0, offset = 28; ctr < msub->header.data_length / 65; ctr++, offset += 65) {
                                _v3_debug(V3_DEBUG_PACKET_PARSE, "allocating 65 bytes for frame");
                                data->frames[ctr] = malloc(65);
                                memcpy(data->frames[ctr], msg->data+offset, 65);
                            }
                            msub->data = data;
                            msg->contents = msub;
                            _v3_func_leave("_v3_get_0x52 (0x01_in gsm)");
                            return true;
                        }
                        break;
                    case 0x03: // speex
                        if (msub->header.codec_format > 32) {
                            // this should always be <= than 32, otherwise bail out
                            free(msub);
                            _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown speex codec format %02X", msub->header.codec_format);
                            _v3_func_leave("_v3_get_0x52 (0x01_in speex)");
                            return false;
                        } else {
                            _v3_msg_0x52_speexdata *data;
                            data = malloc(sizeof(_v3_msg_0x52_speexdata));
                            memcpy(data, msg->data+28, sizeof(uint16_t) * 2); // copy the audio count and frame size values
                            data->frame_count = ntohs(data->frame_count); // for some reason, these are big endian
                            data->sample_size  = ntohs(data->sample_size);  // convert them to host byte order
                            if (data->frame_count == 0) {
                                _v3_debug(V3_DEBUG_PACKET_PARSE, "received audio packet with zero frames");
                                return false;
                            }
                            _v3_debug(V3_DEBUG_PACKET_PARSE, "speex audio count: %d (%d byte frames)", data->frame_count, (msub->header.data_length - 4) / data->frame_count);
                            _v3_debug(V3_DEBUG_PACKET_PARSE, "allocating %d bytes for pointers", data->frame_count * sizeof(uint8_t *));
                            data->frames = malloc(data->frame_count * sizeof(uint8_t *));
                            for (ctr = 0, offset = 32; ctr < data->frame_count; ctr++, offset += (msub->header.data_length - 4) / data->frame_count) {
                                _v3_debug(V3_DEBUG_PACKET_PARSE, "allocating %d bytes for frame", (msub->header.data_length - 4) / data->frame_count);
                                data->frames[ctr] = malloc((msub->header.data_length - 4) / data->frame_count);
                                memcpy(data->frames[ctr], msg->data+offset, (msub->header.data_length - 4) / data->frame_count);
                            }

                            msub->data = data;
                            msg->contents = msub;
                            _v3_func_leave("_v3_get_0x52 (0x01_in speex)");
                            return true;
                        }
                        break;
                    default:
                        _v3_debug(V3_DEBUG_PACKET_PARSE, "unsupported codec type 0x%02X", (uint16_t)msub->header.codec);
                        _v3_func_leave("_v3_get_0x52 (0x01_in)");
                        return false;
                }
                return true;
            }
            break;
        case 0x02:
            {
                _v3_msg_0x52_0x02 *msub = (_v3_msg_0x52_0x02 *)m;
                msub = realloc(m, sizeof(_v3_msg_0x52_0x02));
                memcpy(msub, msg->data, sizeof(_v3_msg_0x52_0x02));
                _v3_debug(V3_DEBUG_PACKET_PARSE, "user %d stopped transmitting", msub->header.user_id);
                msg->contents = msub;
                _v3_func_leave("_v3_get_0x52");
            }
            return true;
        default:
            _v3_debug(V3_DEBUG_PACKET_PARSE, "unknown 0x52 subtype %02x", m->subtype);
            _v3_func_leave("_v3_get_0x52");
            return false;
    }
    _v3_func_leave("_v3_get_0x52");
}/*}}}*/

_v3_net_message *
_v3_put_0x52(uint8_t subtype, uint16_t codec, uint16_t codec_format, uint16_t send_type, uint32_t pcmlength, uint32_t length, void *data) {/*{{{*/
    _v3_net_message *msg;
    _v3_msg_0x52_0x01_out *msgdata;
    int ctr;

    _v3_func_enter("_v3_put_0x52");
    msg = malloc(sizeof(_v3_net_message));
    memset(msg, 0, sizeof(_v3_net_message));

    /*
     * First we go through and create our main message structure 
     */
    switch (subtype) {
        case V3_AUDIO_START:
            _v3_debug(V3_DEBUG_PACKET_PARSE, "sending 0x52 subtype 0x00 size %d", sizeof(_v3_msg_0x52_0x00));
            msgdata = malloc(sizeof(_v3_msg_0x52_0x00));
            memset(msgdata, 0, sizeof(_v3_msg_0x52_0x00));
            msgdata->header.subtype = V3_AUDIO_START;
            msgdata->header.data_length = 0;
            msgdata->header.user_id = v3_get_user_id();
            msgdata->header.send_type = send_type;
            msgdata->unknown_4 = htons(1);
            msgdata->unknown_5 = htons(2);
            msgdata->unknown_6 = htons(1);
            msg->len = sizeof(_v3_msg_0x52_0x00);
            break;
        case V3_AUDIO_DATA:
            _v3_debug(V3_DEBUG_PACKET_PARSE, "sending 0x52 subtype 0x01 header size %d data size %d", sizeof(_v3_msg_0x52_0x00), length);
            msgdata = malloc(sizeof(_v3_msg_0x52_0x01_out));
            memset(msgdata, 0, sizeof(_v3_msg_0x52_0x01_out));
            msg->len = sizeof(_v3_msg_0x52_0x01_out) - sizeof(void *) + length;
            if (codec == 0 || codec == 3) {
                _v3_debug(V3_DEBUG_PACKET_PARSE, "setting pcm length to %d", pcmlength);
                msgdata->header.pcm_length = pcmlength;
            }
            msgdata->header.data_length = length;
            // TODO: we really need to figure out what these values are
            msgdata->unknown_4 = htons(1);
            msgdata->unknown_5 = htons(2);
            msgdata->unknown_6 = htons(1);
            break;
        case V3_AUDIO_STOP:
            _v3_debug(V3_DEBUG_PACKET_PARSE, "sending 0x52 subtype 0x02 size %d", sizeof(_v3_msg_0x52_0x00));
            msgdata = malloc(sizeof(_v3_msg_0x52_0x02));
            memset(msgdata, 0, sizeof(_v3_msg_0x52_0x02));
            msg->len = sizeof(_v3_msg_0x52_0x02);
            msgdata->header.subtype = V3_AUDIO_STOP;
            msgdata->header.data_length = 0;
            msgdata->header.user_id = v3_get_user_id();
            msgdata->header.send_type = send_type;
            break;
        default:
            return NULL;
    }
    msgdata->header.type = msg->type = 0x52;
    msgdata->header.subtype = subtype;
    msgdata->header.codec = codec;
    msgdata->header.codec_format = codec_format;
    _v3_debug(V3_DEBUG_PACKET_PARSE, "user id is %d - send_type is %d (from %d)", msgdata->header.user_id, msgdata->header.send_type, send_type);

    /*
     * Now we allocate the actual msg->data for the network packet representation
     */
    _v3_debug(V3_DEBUG_MEMORY, "allocating %d bytes for data", sizeof(_v3_msg_0x52_0x01_out) - sizeof(void *) + length);
    msg->data = malloc(msg->len);
    memset(msg->data, 0, msg->len);

    /*
     * Next, copy all of the data into the newly allocated memory
     */
    switch (subtype) {
        case V3_AUDIO_START:
            memcpy(msg->data, msgdata, sizeof(_v3_msg_0x52_0x00));
            break;
        case V3_AUDIO_DATA:
            _v3_debug(V3_DEBUG_PACKET_PARSE, "send_type is %d", msgdata->header.send_type);
            memcpy(msg->data, msgdata, sizeof(_v3_msg_0x52_0x01_out));
            switch (codec) {
                case 0:
                    {
                        // Set our beginning offset into the packet
                        char *offset = msg->data + sizeof(_v3_msg_0x52_0x01_out) - sizeof(void *);
                        uint8_t **frames = data;
                        // copy the frames
                        for (ctr = 0; ctr < length / 65; ctr++) {
                            memcpy(offset, frames[ctr], 65);
                            offset += 65;
                        }
                    }
                    break;
                case 3:
                    {
                        // Set our beginning offset into the packet
                        char *offset = msg->data + sizeof(_v3_msg_0x52_0x01_out) - sizeof(void *);
                        uint16_t tmp, enc_frame_size;
                        _v3_msg_0x52_speexdata *speexdata = data;

                        tmp = htons(speexdata->frame_count);
                        memcpy(offset, &tmp, 2);
                        offset += 2;

                        tmp = htons(speexdata->sample_size);
                        memcpy(offset, &tmp, 2);
                        offset += 2;

                        // Figure out how long each frame is
                        enc_frame_size = ((length - 4) / speexdata->frame_count);

                        // Copy all frames in the array into the network packet
                        for (ctr = 0; ctr < speexdata->frame_count; ctr++) {
                            memcpy(offset, speexdata->frames[ctr], enc_frame_size);
                            free(speexdata->frames[ctr]);
                            offset += enc_frame_size;
                        }
                        // TODO: These, and the free in the loop above, are only temporary fixes.
                        // We should construct _v3_destroy_0x52 in such a way that it can free outgoing audio packages;
                        // or just write seperate functions for freeing incoming/outgoing audio.
                        free(speexdata->frames);
                        free(speexdata);
                    }
                    break;
            }
            break;
        case V3_AUDIO_STOP:
            memcpy(msg->data, msgdata, sizeof(_v3_msg_0x52_0x02));
            break;
    }
    free(msgdata);
    _v3_func_leave("_v3_put_0x52");
    return msg;

}/*}}}*/

int
_v3_destroy_0x52(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x52 *m;
    _v3_msg_0x52_0x01_in *msubin;
    // _v3_msg_0x52_0x01_out *msubout; // TODO: free output structures
    _v3_msg_0x52_gsmdata *gsmdata;
    _v3_msg_0x52_speexdata *speexdata;
    int ctr;

    _v3_func_enter("_v3_destroy_0x52");
    m = msg->contents;
    switch (m->subtype) {
        case 0x01:
            msubin = (_v3_msg_0x52_0x01_in *)m;
            switch (msubin->header.codec) {
                case 0x00:
                    gsmdata = msubin->data;
                    for (ctr = 0; ctr < msubin->header.data_length / 65; ctr++) {
                        _v3_debug(V3_DEBUG_PACKET_PARSE, "freeing 65 bytes for gsm frame %d", ctr);
                        free(gsmdata->frames[ctr]);
                    }
                    free(gsmdata->frames);
                    free(gsmdata);
                    break;
                case 0x03:
                    speexdata = msubin->data;
                    for (ctr = 0; ctr < speexdata->frame_count; ctr++) {
                        _v3_debug(V3_DEBUG_PACKET_PARSE, "freeing %d bytes for frame %d", msubin->header.data_length / speexdata->frame_count, ctr);
                        free(speexdata->frames[ctr]);
                    }
                    free(speexdata->frames);
                    free(speexdata);
                    break;
            }
            break;
        default:
            break;
    }
    _v3_func_leave("_v3_destroy_0x52");
    return true;
}/*}}}*/

/*}}}*/
// Message 0x53 (83) | CHANNEL CHANGE /*{{{*/
int
_v3_get_0x53(_v3_net_message *msg) {/*{{{*/
    _v3_func_enter("_v3_get_0x53");
    if (msg->len != sizeof(_v3_msg_0x53)) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "expected %d bytes, but message is %d bytes", sizeof(_v3_msg_0x53), msg->len);
        _v3_func_leave("_v3_get_0x53");
        return false;
    }
    msg->contents = msg->data;
    _v3_func_leave("_v3_get_0x53");
    return true;
}/*}}}*/
/*}}}*/
// Message 0x57 (87) | SERVER INFORMATION/*{{{*/
int
_v3_get_0x57(_v3_net_message *msg) {/*{{{*/
    _v3_func_enter("_v3_get_0x57");
    if (msg->len != sizeof(_v3_msg_0x57)) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "expected %d bytes, but message is %d bytes", sizeof(_v3_msg_0x57), msg->len);
        _v3_func_leave("_v3_get_0x57");
        return false;
    }
    msg->contents = msg->data;
    _v3_func_leave("_v3_get_0x57");
    return true;
}/*}}}*/
/*}}}*/
// Message 0x58 (88) | PHANTOM /*{{{*/
int
_v3_get_0x58(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x58 *m;

    _v3_func_enter("_v3_get_0x58");
    if (msg->len != sizeof(_v3_msg_0x58)) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "expected %d bytes, but message is %d bytes", sizeof(_v3_msg_0x58), msg->len);
        _v3_func_leave("_v3_get_0x58");
        return false;
    }

     m = msg->contents = msg->data;
    _v3_debug(V3_DEBUG_PACKET_PARSE, "Phantom:");
    _v3_debug(V3_DEBUG_PACKET_PARSE, "subtype.............: %d", m->subtype);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "error_id............: %d", m->error_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "real_user_id........: %d", m->real_user_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "phantom_user_id.....: %d", m->phantom_user_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "channel_id..........: %d", m->channel_id);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "log_error...........: %d", m->log_error);

    _v3_func_leave("_v3_get_0x58");
    return true;
}/*}}}*/
_v3_net_message *
_v3_put_0x58(uint16_t subtype, uint16_t channel, uint16_t phantom_user_id) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x58 *mc;

    _v3_func_enter("_v3_put_0x58");
    // Build our message
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x58;
    m->len = sizeof(_v3_msg_0x58);

    // Build our message contents
    mc = malloc(sizeof(_v3_msg_0x58));
    memset(mc, 0, sizeof(_v3_msg_0x58));
    mc->type = 0x58;
    mc->subtype = subtype;
    mc->real_user_id = v3_luser.id;

    switch(subtype) {
        case V3_PHANTOM_ADD:
            mc->channel_id = channel;
            break;
        case V3_PHANTOM_REMOVE:
            mc->phantom_user_id = phantom_user_id;
            break;
    }

    m->contents = mc;
    m->data = (char *)mc;
    _v3_func_leave("_v3_put_0x58");
    return m;
}/*}}}*/
/*}}}*/
// Message 0x59 (89) | ERROR MESSAGE /*{{{*/
int
_v3_get_0x59(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x59 *m;
    uint16_t plen;
    uint16_t len;

    _v3_func_enter("_v3_get_0x59");
    plen = sizeof(_v3_msg_0x59) - sizeof(void *);
    m = malloc(sizeof(_v3_msg_0x59));
    memset(m, 0, sizeof(_v3_msg_0x59));
    memcpy(m,  msg->data, plen);
    m->message = (char *)_v3_get_msg_string(msg->data+plen, &len);
    msg->contents = m;
    _v3_func_leave("_v3_get_0x59");
    return true;
}/*}}}*/
/*}}}*/
// Message 0x5c (92) | HASH TABLE SCRAMBLE  /*{{{*/
int
_v3_get_0x5c(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x5c *m;

    _v3_func_enter("_v3_get_0x5c");
    if (msg->len != sizeof(_v3_msg_0x5c)) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "expected %d bytes, but message is %d bytes", sizeof(_v3_msg_0x5c), msg->len);
        _v3_func_leave("_v3_get_0x5c");
        return false;
    }
    m = msg->contents = msg->data;
    _v3_debug(V3_DEBUG_PACKET_PARSE, "Hash Scramble:");
    _v3_debug(V3_DEBUG_PACKET_PARSE, "subtype.............: %d",   m->subtype);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "sum 1...............: %d",   m->sum_1);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "sum 2...............: %d",   m->sum_2);
    _v3_func_leave("_v3_get_0x5c");
    return true;
}/*}}}*/

_v3_net_message *
_v3_put_0x5c(uint8_t subtype) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x5c *mc;

    _v3_func_enter("_v3_put_0x5c");
    // Build our message
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x5c;
    m->len = sizeof(_v3_msg_0x5c);

    // Build our message contents
    mc = malloc(sizeof(_v3_msg_0x5c));
    memset(mc, 0, sizeof(_v3_msg_0x5c));
    mc->type = 0x5c;
    mc->subtype = subtype;

    // We may want to store rand() values for later reference.
    switch(subtype) {
        case 0:
            mc->sum_2 = _v3_msg5c_gensum(0xBAADF00D, 16);
            break;
        case 1:
            mc->sum_1 = (uint8_t)rand();
            break;
        case 2:
            mc->sum_2 = _v3_msg5c_gensum(0x0DBAADF0, 16);
            break;
        case 3:
            {
                uint8_t out[9];
                snprintf((char *)out, 8, "%08X", (uint32_t)rand());
                mc->sum_2 = _v3_msg5c_scramble(out);
            }
            break;
        case 4:
            mc->sum_2 = _v3_msg5c_gensum(0xBAADF00D, 32);
            break;
        case 5:
            {
                uint8_t out[9];
                snprintf((char *)out, 8, "%08x", (uint32_t)rand());
                mc->sum_2 = _v3_msg5c_scramble(out);
            }
            break;
        case 6:
            mc->sum_2 = _v3_msg5c_gensum(0xBAADF00D, 16);
            break;
        case 7:
            {
                uint8_t out[8];
                snprintf((char *)out, 8, "%08X", (uint32_t)rand());
                mc->sum_2 = _v3_msg5c_scramble(out);
            }
            break;
        case 8:
            mc->sum_2 = _v3_msg5c_gensum(0xBAADF00D, 32);
            break;
        case 9:
            mc->sum_2 = _v3_msg5c_gensum(0xBAADF00D, 16);
            break;
        case 10:
            mc->sum_2 = !rand();
            break;
    }

    m->contents = mc;
    m->data = (char *)mc;
    _v3_func_leave("_v3_put_0x5c");
    return m;
}/*}}}*/

    uint32_t
_v3_msg5c_scramble(uint8_t* in)/*{{{*/
{
    uint32_t i, out = 0;
    for(i = 0; i < 8; i++) {
        out = (out >> 8) ^ _v3_hash_table[(uint8_t)(in[i] ^ out)];
    }
    return out;
}/*}}}*/

uint32_t
_v3_msg5c_gensum(uint32_t seed, uint32_t iterations) {/*{{{*/
    uint32_t i, j, out = 0;
    uint32_t* ecx = (uint32_t*)malloc(sizeof(uint32_t) * iterations);
    for(i = 0; i < iterations; i++) {
        ecx[i] = seed;
    }
    for(i = 0; i < iterations; i++) {
        for(j = 0; j < 4; j++) {
            uint8_t offset = ((ecx[i] >> (j * 8)) ^ out) & 0xff;
            out = (out >> 8) ^ _v3_hash_table[offset];
        }
    }
    uint8_t formatted[9] = { 0 };
    snprintf((char *)formatted, 9, "%08x", out);
    free(ecx);
    return _v3_msg5c_scramble(formatted);
}/*}}}*/
/*}}}*/
// Message 0x5d (93) | USER LIST MODIFICATION /*{{{*/
int
_v3_get_0x5d(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x5d *m;
    int ctr;
    void *offset;

    _v3_func_enter("_v3_get_0x5d");
    m = malloc(sizeof(_v3_msg_0x5d));
    memcpy(m, msg->data, 8);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "packet contains %d users.  message subtype %02X", m->user_count, m->subtype);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "allocating %d bytes for userlist packet", sizeof(_v3_msg_0x5d));
    m = realloc(m, sizeof(_v3_msg_0x5d));
    _v3_debug(V3_DEBUG_PACKET_PARSE, "allocating %d bytes (%d users * %d bytes)", m->user_count*sizeof(_v3_msg_user), m->user_count, sizeof(_v3_msg_user));
    m->user_list = calloc(m->user_count, sizeof(_v3_msg_user));
    for (ctr = 0, offset = msg->data + 8; ctr < m->user_count; ctr++) {
        offset += _v3_get_msg_user(offset, &m->user_list[ctr]);
        _v3_debug(V3_DEBUG_PACKET_PARSE, "got user: id: %d | chan: %d | name: %s | phonetic: %s | comment: %s | int: %s | url: %s | guest %d",
                m->user_list[ctr].id,
                m->user_list[ctr].channel,
                m->user_list[ctr].name,
                m->user_list[ctr].phonetic,
                m->user_list[ctr].comment,
                m->user_list[ctr].integration_text,
                m->user_list[ctr].url
                );

    }
    m->lobby = &m->user_list[0];
    msg->contents = m;
    _v3_func_leave("_v3_get_0x5d");
    return true;
}/*}}}*/

_v3_net_message *
_v3_put_0x5d(uint16_t subtype, uint16_t count, v3_user *user) {/*{{{*/
    _v3_net_message *msg;
    _v3_msg_0x5d *msgdata;
    int len = 0;
    int ctr;

    _v3_func_enter("_v3_put_0x5d");
    msg = malloc(sizeof(_v3_net_message));
    memset(msg, 0, sizeof(_v3_net_message));

    msgdata = malloc(sizeof(_v3_msg_0x5d));
    memset(msgdata, 0, sizeof(_v3_msg_0x5d));
    msgdata->type = 0x5d;
    msgdata->subtype = subtype;
    msgdata->user_count = count;

    _v3_debug(V3_DEBUG_MEMORY, "allocating %d bytes for data", V3_MAX_USER_SIZE * count + sizeof(_v3_msg_0x5d));
    msg->data = malloc(V3_MAX_USER_SIZE * count + sizeof(_v3_msg_0x5d));
    memset(msg->data, 0, V3_MAX_USER_SIZE * count + sizeof(_v3_msg_0x5d));
    memcpy(msg->data, msgdata, 8); // only the first 8 bytes are sent, the rest is user structures
    len += 8;
    for (ctr = 0; ctr < count; ctr++) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "putting user %d: %d", ctr, user[ctr].id);
        len += _v3_put_msg_user((void *)(msg->data+len), &user[ctr]);
    }
    msg->len = len;
    free(msgdata);
    _v3_func_leave("_v3_put_0x5d");
    return msg;
}/*}}}*/
int
_v3_destroy_0x5d(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x5d *m;
    int ctr;

    m = msg->contents;
    _v3_func_enter("_v3_destroy_0x5d");
    for (ctr = 0; ctr < m->user_count; ctr++) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "freeing resources for user %d: %s", m->user_list[ctr].id, m->user_list[ctr].name);
        free(m->user_list[ctr].name);
        free(m->user_list[ctr].phonetic);
        free(m->user_list[ctr].comment);
        free(m->user_list[ctr].integration_text);
        free(m->user_list[ctr].url);
    }
    free(m->user_list);
    _v3_func_leave("_v3_destroy_0x5d");
    return true;
}/*}}}*/
/*}}}*/
// Message 0x60 (96) | CHANNEL LIST MODIFICATION /*{{{*/
int
_v3_get_0x60(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x60 *m;
    int ctr;
    void *offset;

    _v3_func_enter("_v3_get_0x60");
    m = malloc(sizeof(_v3_msg_0x60));
    memcpy(m, msg->data, 8);
    _v3_debug(V3_DEBUG_PACKET_PARSE, "server has %d channels, allocating %d bytes", m->channel_count, m->channel_count * sizeof(_v3_msg_channel));
    m->channel_list = calloc(m->channel_count, sizeof(_v3_msg_channel));
    for (ctr = 0, offset = msg->data + 8;ctr < m->channel_count; ctr++) {
        offset += _v3_get_msg_channel(offset, &m->channel_list[ctr]);
        _v3_debug(V3_DEBUG_PACKET_PARSE, "got channel: id: %d | name: %s | phonetic: %s | comment: %s",
                m->channel_list[ctr].id,
                m->channel_list[ctr].name,
                m->channel_list[ctr].phonetic,
                m->channel_list[ctr].comment);
    }
    msg->contents = m;
    _v3_func_leave("_v3_get_0x60");
    return true;
}/*}}}*/

int
_v3_destroy_0x60(_v3_net_message *msg) {/*{{{*/
    _v3_msg_0x60 *m;
    int ctr;

    m = msg->contents;
    _v3_func_enter("_v3_destroy_0x60");
    for (ctr = 0; ctr < m->channel_count; ctr++) {
        _v3_debug(V3_DEBUG_PACKET_PARSE, "freeing resources for channel %d: %s", m->channel_list[ctr].id, m->channel_list[ctr].name);
        free(m->channel_list[ctr].name);
        free(m->channel_list[ctr].phonetic);
        free(m->channel_list[ctr].comment);
    }
    free(m->channel_list);
    _v3_func_leave("_v3_destroy_0x60");
    return true;
}/*}}}*/
/*}}}*/
// Message 0x63 (99) | ADMIN /*{{{*/
_v3_net_message *
_v3_put_0x63(uint16_t subtype, uint16_t user_id, uint8_t *string) {/*{{{*/
    _v3_net_message *m;
    _v3_msg_0x63 *mc;

    _v3_func_enter("_v3_put_0x63");
    // Build our message
    m = malloc(sizeof(_v3_net_message));
    memset(m, 0, sizeof(_v3_net_message));
    m->type = 0x63;
    m->len = sizeof(_v3_msg_0x63);

    // Build our message contents
    mc = malloc(sizeof(_v3_msg_0x63));
    memset(mc, 0, sizeof(_v3_msg_0x63));

    mc->type = 0x63;
    mc->subtype = subtype;
    
    switch (subtype) {
        case V3_ADMIN_LOGIN:
            _v3_hash_password(string, mc->t.password_hash);
            break;
        case V3_ADMIN_LOGOUT:
            break;
        case V3_ADMIN_KICK:
        case V3_ADMIN_BAN:
        case V3_ADMIN_CHANNEL_BAN:
            mc->user_id = user_id;
            strncpy(mc->t.reason, string, sizeof(mc->t.reason));
            break;
    }

    m->contents = mc;
    m->data = (char *)mc;
    _v3_func_leave("_v3_put_0x63");
    return m;
}/*}}}*/
/*}}}*/
