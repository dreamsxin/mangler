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

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "ventrilo3.h"

/*
 * This file contains structures for each message type received by the Ventrilo3
 * protocol.  Each type is broken up into subtypes where applicable.  For
 * example:
 *
 * _v3_net_message_0x5d_0x04 is message type 0x5d subtype 0x04
 *
 * Where further breakdown is necessary (for instance user and channel
 * information messages), structures are defined specifically for that data.
 * That data (so far) is identical for any subtype.  Therefore, a user
 * definition in message type 0x5d subtype 0x04 is identical to a user
 * definition in type 0x5d subtype 0x01.
 *
 * To parse a packet, a smaller structure (i.e. a "type") will be recast as a
 * larger structure depending on the subtype.
 *
 * In cases where a particular value is known, it is marked by a name of
 * (unknown_#).  The number is arbitrary and subject to change.  If an unknown
 * value is identified, it should be pulled out of the unknown pool and named
 * appropriately.
 *
 * For questions or comments, join irc://irc.freenode.net channel #mangler
 * 
 * Better documentation on these messages can be found at our wiki:
 * http://www.mangler.org/trac/wiki/ProtocolDocumentation
 */

typedef struct _v3_net_message_0x00 {/*{{{*/
    uint32_t type;
    char version[16];
    char salt1[32];
    char salt2[32];
} _v3_msg_0x00;/*}}}*/
typedef struct _v3_net_message_0x06 {/*{{{*/
    uint32_t type;              // 0
    uint16_t unknown_1;         // 4
    uint16_t error_id;          // 6
    uint32_t subtype;           // 8
    
    uint8_t  unknown_2;         // 12 - variable length starts here
    uint8_t* encryption_key;
} _v3_msg_0x06;
int _v3_get_0x06(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x37 {/*{{{*/
    uint32_t type;              // 0
    uint16_t user_id;           // 4
    uint16_t sequence;          // 6
    uint16_t ping;              // 8
    uint16_t inactivity;        // 10
} _v3_msg_0x37;
int _v3_get_0x37(_v3_net_message *msg);
_v3_net_message *_v3_put_0x37(int sequence);/*}}}*/
typedef struct _v3_net_message_0x3a {/*{{{*/
    uint32_t type;              // 0
    uint32_t empty;             // 4
    uint16_t msglen;            // 6
    
    char *   msg;               // 8 - variable length starts here
} _v3_msg_0x3a;/*}}}*/
typedef struct _v3_net_message_0x3b {/*{{{*/
    uint32_t type;              // 0
    uint16_t user_id;           // 4
    uint16_t channel_id;        // 6
    uint32_t error_id;          // 8
} _v3_msg_0x3b;
int _v3_get_0x3b(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x3c {/*{{{*/
    uint32_t type;              // 0
    uint8_t  unknown1[4];
    uint16_t codec;
    uint16_t codec_format;
    uint8_t  unknown2[12];
} _v3_msg_0x3c;
int _v3_get_0x3c(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x3f {/*{{{*/
    uint32_t type;              // 0
    uint32_t empty;             // 4
    uint16_t pathlen;           // 6
    
    char *   filepath;          // 8 - variable length starts here
} _v3_msg_0x3f;/*}}}*/
typedef struct _v3_net_message_0x42 {/*{{{*/
    uint32_t type;              // 0
    uint16_t user_id;           // 4
    uint32_t subtype;           // 6
    uint16_t unknown;           // 10
} _v3_msg_0x42;/*}}}*/
typedef struct _v3_net_message_0x46 {/*{{{*/
    uint32_t type;              // 0
    uint16_t user_id;           // 4
    uint16_t setting;           // 6
    uint16_t value;             // 8
    uint16_t unknown_1;         // 10
} _v3_msg_0x46;
int _v3_get_0x46(_v3_net_message *msg);
_v3_net_message *_v3_put_0x46(uint16_t setting, uint16_t value);/*}}}*/
typedef struct _v3_net_message_0x48 {/*{{{*/
    uint32_t type;              // 0
    uint32_t subtype;           // 4
    uint32_t unknown_1;         // 8
    uint32_t server_ip;         // 12
    uint16_t portmask;          // 16
    uint16_t show_login_name;   // 18
    uint16_t unknown_2;         // 20
    uint16_t auth_server_index; // 22
    char     handshake[16];     // 24
    char     client_version[16];// 40
    uint8_t  unknown_3[48];     // 56
    char     proto_version[16]; // 104
    char     password_hash[32]; // 120
    char     username[32];      // 152
    char     phonetic[32];      // 184
    char     os[64];            // 216
} _v3_msg_0x48;
int _v3_get_0x48(_v3_net_message *msg);
_v3_net_message *_v3_put_0x48(void);/*}}}*/
typedef struct _v3_net_message_0x49 {/*{{{*/
    uint32_t type;              // 0
    uint16_t user_id;           // 4
    uint16_t subtype;           // 6
    uint8_t  hash_password[32]; // 8

    v3_channel *channel;        // 40 - variable lenghth starts here
} _v3_msg_0x49;
int _v3_get_0x49(_v3_net_message *msg);
_v3_net_message *_v3_put_0x49(uint16_t subtype, uint16_t user_id, char *channel_password, _v3_msg_channel *channel);/*}}}*/
typedef struct _v3_net_message_0x4a {/*{{{*/
    uint32_t type;              // 0
    uint32_t subtype;           // 4
    uint8_t unknown_1[16];      // 8
    uint8_t hash_password[32];  // 24
    uint32_t unknown_2;         // 56
    uint8_t lock_acct;          // 60
    uint8_t dfl_chan;
    uint8_t dupe_ip;
    uint8_t switch_chan;
    uint8_t in_reserve_list;
    uint8_t unknown_perm_1;
    uint8_t unknown_perm_2;
    uint8_t unknown_perm_3;
    uint8_t recv_bcast;
    uint8_t add_phantom;
    uint8_t record;
    uint8_t recv_complaint;
    uint8_t send_complaint;
    uint8_t inactive_exempt;
    uint8_t unknown_perm_4;
    uint8_t unknown_perm_5;
    uint8_t srv_admin;
    uint8_t add_user;
    uint8_t del_user;
    uint8_t ban_user;
    uint8_t kick_user;
    uint8_t move_user;
    uint8_t assign_chan_admin;
    uint8_t edit_rank;
    uint8_t edit_motd;
    uint8_t edit_guest_motd;
    uint8_t issue_rcon_cmd;
    uint8_t edit_voice_target;
    uint8_t edit_command_target;
    uint8_t assign_rank;
    uint8_t assign_reserved;
    uint8_t unknown_perm_6;
    uint8_t unknown_perm_7;
    uint8_t unknown_perm_8;
    uint8_t unknown_perm_9;
    uint8_t unknown_perm_10;
    uint8_t bcast;
    uint8_t bcast_lobby;
    uint8_t bcast_user;
    uint8_t bcast_x_chan;
    uint8_t send_tts_bind;
    uint8_t send_wav_bind;
    uint8_t send_page;
    uint8_t send_comment;
    uint8_t set_phon_name;
    uint8_t gen_comment_snds;
    uint8_t event_snds;
    uint8_t mute_glbl;
    uint8_t mute_other;
    uint8_t glbl_chat;
    uint8_t start_priv_chat;
    uint8_t unknown_perm_11;
    uint8_t eq_out;
    uint8_t unknown_perm_12;
    uint8_t unknown_perm_13;
    uint8_t unknown_perm_14;
    uint8_t see_guest;
    uint8_t see_nonguest;
    uint8_t see_motd;
    uint8_t see_srv_comment;
    uint8_t see_chan_list;
    uint8_t see_chan_comment;
    uint8_t see_user_comment;
    uint8_t unknown_perm_15;
} _v3_msg_0x4a;
int _v3_get_0x4a(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x4b {/*{{{*/
    uint32_t type;              // 0
    uint32_t timestamp;         // 4
    uint32_t empty;             // 8
} _v3_msg_0x4b;
_v3_net_message *_v3_put_0x4b(void);/*}}}*/
typedef struct _v3_net_message_0x50 {/*{{{*/
    uint32_t type;              // 0
    uint32_t timestamp;         // 4
    uint16_t guest_motd_flag;   // 8
    uint16_t message_num;       // 10
    uint16_t message_id;        // 12
    uint16_t message_size;      // 14
    uint8_t  message[256];      // 16
} _v3_msg_0x50;
int _v3_get_0x50(_v3_net_message *msg);/*}}}*/

typedef struct _v3_net_message_0x52 {/*{{{*/
    uint32_t type;              // 0
    uint16_t subtype;           // 4
    uint16_t user_id;           // 6
    uint16_t codec;             // 8
    uint16_t codec_format;      // 10
    uint16_t send_type;         // 12
    uint16_t unknown_1;         // 14
    uint32_t data_length;       // 16
    uint16_t unknown_2;         // 20
    uint16_t unknown_3;         // 22
} _v3_msg_0x52; /*}}}*/
typedef struct _v3_net_message_0x52_0x00 {/*{{{*/
    uint32_t type;              // 0
    uint16_t subtype;           // 4
    uint16_t user_id;           // 6
    uint16_t codec;             // 8
    uint16_t codec_format;      // 10
    uint16_t send_type;         // 12
    uint16_t unknown_1;         // 14
    uint32_t data_length;       // 16
    uint16_t unknown_2;         // 20
    uint16_t unknown_3;         // 22
    uint16_t unknown_4;         // 24
    uint16_t unknown_5;         // 26
    uint16_t unknown_6;         // 28
    uint16_t unknown_7;         // 30
} _v3_msg_0x52_0x00; /*}}}*/
typedef struct _v3_net_message_0x52_0x01_in {/*{{{*/
    uint16_t type;              // 0
    uint16_t empty;             // 2
    uint16_t subtype;           // 4
    uint16_t user_id;           // 6
    uint16_t codec;             // 8
    uint16_t codec_format;      // 10
    uint32_t send_type;         // 12
    uint32_t data_length;       // 16
    uint16_t unkonwn_2;         // 20
    uint16_t unknown_3;         // 22
    uint16_t unknown_4;         // 24
    uint16_t unknown_5;         // 26

    void     *data;             // 28 - either gsmdata* or speexdata*
} _v3_msg_0x52_0x01_in;/*}}}*/
typedef struct _v3_net_message_0x52_0x01_out {/*{{{*/
    uint16_t type;              // 0
    uint16_t empty;             // 2
    uint16_t subtype;           // 4
    uint16_t user_id;           // 6
    uint16_t codec;             // 8
    uint16_t codec_format;      // 10
    uint32_t send_type;         // 12
    uint32_t data_length;       // 16
    uint16_t unkonwn_2;         // 20
    uint16_t unknown_3;         // 22
    uint16_t unknown_4;         // 24
    uint16_t unknown_5;         // 26
    uint16_t unknown_6;         // 28
    uint16_t unknown_7;         // 30

    void     *data;             // 28 - either gsmdata* or speexdata*
} _v3_msg_0x52_0x01_out;/*}}}*/
typedef struct _v3_net_message_0x52_gsmdata {/*{{{*/
    uint8_t  **frames;
} _v3_msg_0x52_gsmdata;/*}}}*/
typedef struct _v3_net_message_0x52_speexdata {/*{{{*/
    uint16_t frame_count;
    uint16_t sample_size;
    uint8_t  **frames;
} _v3_msg_0x52_speexdata;/*}}}*/
typedef struct _v3_net_message_0x52_0x02 {/*{{{*/
    uint32_t type;              // 0
    uint16_t subtype;           // 4
    uint16_t user_id;           // 6
    uint16_t codec;             // 8
    uint16_t codec_format;      // 10
    uint16_t sendtype;          // 12
    uint16_t unknown_1;         // 14
    uint32_t data_length;       // 16
    uint16_t unknown_2;         // 20
    uint16_t unknown_3;         // 22
    uint16_t unknown_4;         // 24
    uint16_t unknown_5;         // 26
} _v3_msg_0x52_0x02; /*}}}*/
typedef struct _v3_net_message_0x52_0x03 {/*{{{*/
    uint32_t type;              // 0
    uint16_t subtype;           // 4
    uint16_t user_id;           // 6
    uint16_t codec;             // 8
    uint16_t codec_format;      // 10
    uint16_t sendtype;          // 12
    uint16_t unknown_1;         // 14
    uint32_t data_length;       // 16
    uint16_t unknown_2;         // 20
    uint16_t unknown_3;         // 22
} _v3_msg_0x52_0x03; /*}}}*/
int _v3_get_0x52(_v3_net_message *msg);
int _v3_destroy_0x52(_v3_net_message *msg);

typedef struct _v3_net_message_0x53 {/*{{{*/
    uint32_t type;              // 0
    uint16_t user_id;           // 2
    uint16_t channel_id;        // 4
} _v3_msg_0x53;
int _v3_get_0x53(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x57 {/*{{{*/
    uint32_t type;              // 0
    uint32_t unknown_1;         // 4
    uint16_t port;              // 8
    uint16_t max_clients;       // 10
    uint16_t connected_clients; // 12
    uint8_t  unknown_2[14];     // 14
    char     name[32];          // 28
    char     version[16];       // 60
    uint8_t  unknown_3[32];     // 76

} _v3_msg_0x57;
int _v3_get_0x57(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x59 {/*{{{*/
    uint32_t type;              // 0
    uint16_t error;             // 4
    uint16_t minutes_banned;    // 6
    uint16_t log_error;         // 8
    uint16_t close_connection;  // 10
    uint16_t unknown_[4];       // 12
    char *   message;           // 20

} __attribute__ ((packed)) _v3_msg_0x59;
int _v3_get_0x59(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x5c {/*{{{*/
    uint32_t type;              // 0
    uint16_t subtype;           // 4
    uint16_t sum_1;             // 6
    uint32_t sum_2;             // 8
} _v3_msg_0x5c;
int _v3_get_0x5c(_v3_net_message *msg);
_v3_net_message *_v3_put_0x5c(uint8_t subtype);
uint32_t _v3_msg5c_scramble(uint8_t* in);
uint32_t _v3_msg5c_gensum(uint32_t seed, uint32_t iterations);/*}}}*/
typedef struct _v3_net_message_0x5d {/*{{{*/
    uint32_t type;              // 0
    uint16_t subtype;           // 4
    uint16_t user_count;        // 6
    _v3_msg_user *lobby;        // 8 - variable length starts here
    _v3_msg_user *user_list;
} _v3_msg_0x5d;
_v3_net_message *_v3_put_0x5d(uint16_t subtype, uint16_t count, v3_user *user);
int _v3_get_0x5d(_v3_net_message *msg);
int _v3_destroy_0x5d(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x60 {/*{{{*/
    uint32_t type;              // 0
    uint32_t channel_count;     // 4
    _v3_msg_channel *channel_list; // 8 - variable length stats here
} _v3_msg_0x60;
int _v3_get_0x60(_v3_net_message *msg);
int _v3_destroy_0x60(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x61 {/*{{{*/
    uint32_t type;              // 0
    uint32_t subtype;           // 4
    uint32_t bitmask_id;        // 8
    uint32_t ip_address;        // 12
    uint16_t ban_count;         // 16
    uint16_t ban_id;            // 18
    uint8_t  banned_user[32];   // 20
    uint8_t  banned_by[32];     // 52
    uint8_t  ban_msg[128];      // 84
} _v3_msg_0x61;/*}}}*/
typedef struct _v3_net_message_0x62 {/*{{{*/
    uint32_t type;              // 0
    uint16_t from;              // 4
    uint16_t to;                // 6
    uint32_t error_id;          // 8
} _v3_msg_0x62;/*}}}*/

char *   _v3_get_msg_string(void *offset, uint16_t *len);
int      _v3_get_msg_channel(void *offset, _v3_msg_channel *channel);
int      _v3_put_msg_channel(char *buf, _v3_msg_channel *channel);
int      _v3_get_msg_user(void *offset, _v3_msg_user *user);
int      _v3_put_msg_user(void *buf, _v3_msg_user *user);

