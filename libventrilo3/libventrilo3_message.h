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
 */

typedef struct _v3_net_message_0x00 {/*{{{*/
    /*
     *  Message Type 0x00 is only used during the login phase as a part of the
     *  key exchange.  The client sends 64 bytes of random data to the server.
     */
    uint16_t type;
    char version[16];
    char salt1[32];
    char salt2[32];
} _v3_msg_0x00;/*}}}*/

typedef struct _v3_net_message_0x06 {
    /*
     * Message Type 0x06 is used for many purposes, mostly during authing. 
     * Includes but not limited to: auth failure, encryption key exchange, server disabled messages.
     */
    uint32_t type;        	  // 0
    uint16_t unknown_1;     	  // 4
    uint16_t error_id;  	  // 6
    uint32_t subtype;   	  // 8
    
    uint8_t  unknown_2;		  // 12 - variable length starts here
    uint8_t* encryption_key;
} _v3_msg_0x06;
int _v3_get_0x06(_v3_net_message *msg);

typedef struct _v3_net_message_0x37 {/*{{{*/
    /*
     *  Message Type 0x37 is a ping to/from the server
     *  libventrilo3: 02:06:58:         ======= received TCP packet =====================================
     *  libventrilo3: 02:06:58:         PACKET: message type: 0x37 (55)
     *  libventrilo3: 02:06:58:         PACKET: data length : 12
     *  libventrilo3: 02:06:58:         PACKET:     37 00 00 00 9C 03 C0 44 FF FF 00 00                  7......D....
     */
    uint32_t type;        // 0
    uint16_t user_id;     // 4
    uint16_t sequence;    // 6
    uint16_t ping;        // 8
    uint16_t unknown_2;   // 10

} _v3_msg_0x37;
int _v3_get_0x37(_v3_net_message *msg);
_v3_net_message *_v3_put_0x37(int sequence);/*}}}*/
typedef struct _v3_net_message_0x3b {/*{{{*/
    /*
     * Message Type 0x3B is used to move a user from one channel to another.
     * Error codes can be found in _v3_move_errors.
     */
    uint32_t type;	// 0
    uint16_t user_id;	// 4
    uint16_t channel_id;// 6
    uint32_t error_id;	// 8
} _v3_msg_0x3b;/*}}}*/
typedef struct _v3_net_message_0x42 {/*{{{*/
    /*
     * Message Type 0x42 is used for both rcon and global chat.
     * In case of subtypes 2 and 3, a VentriloNetString is appended to the end of this packet.
     */
    uint32_t type;	// 0
    uint16_t user_id;	// 4
    uint32_t subtype;	// 6
    uint16_t unknown;	// 10
} _v3_msg_0x42;/*}}}*/
typedef struct _v3_net_message_0x46 {/*{{{*/
    /*
     * libventrilo3: 10:17:47:         PACKET: message type: 0x46 (70)
     * libventrilo3: 10:17:47:         PACKET: data length : 12
     * libventrilo3: 10:17:47:         PACKET:     46 00 00 00 18 00 01 00 01 00 00 00                  F...........
     */
    uint32_t type;        // 0
    uint16_t user_id;     // 4
    uint16_t setting;     // 6
    uint16_t value;       // 8
    uint16_t unknown_1;   // 10
} _v3_msg_0x46;
int _v3_get_0x46(_v3_net_message *msg);
_v3_net_message *_v3_put_0x46(uint16_t setting, uint16_t value);/*}}}*/
typedef struct _v3_net_message_0x4b {/*{{{*/
    uint32_t type;        // 0
    uint32_t timestamp;   // 4
    uint32_t unknown_1;   // 8
} _v3_msg_0x4b;
_v3_net_message *_v3_put_0x4b(void);/*}}}*/
typedef struct _v3_net_message_0x48 {/*{{{*/
    /*
     *  Message Type 0x48 is the the client sending login information to the
     *  server such as username, password, phonetic, etc.
     */
    uint32_t type;
    uint32_t subtype;
    uint32_t unknown_1;
    uint32_t server_ip;
    uint16_t portmask;
    uint16_t show_login_name;
    uint16_t unknown_2;
    uint16_t auth_server_index;
    char     handshake[16];
    char     client_version[16];
    uint8_t  unknown_3[48];
    char     proto_version[16];
    char     password_hash[32];
    char     username[32];
    char     phonetic[32];
    char     os[64];
} _v3_msg_0x48;
int _v3_get_0x48(_v3_net_message *msg);
_v3_net_message *_v3_put_0x48(void);/*}}}*/
typedef struct _v3_net_message_0x49 {/*{{{*/
    /*
     *  Message Type 0x49 is a channel message notification.  When received
     *  from the server, this means a channel has been added (0x01), removed (0x02), or
     *  modified (0x05).  When sent from the client, this is a channel
     *  change message (0x03)
     */
    uint32_t type;                    // 0
    uint16_t user_id;                 // 4
    uint16_t subtype;                 // 6
    uint8_t  hash_password[32];       // 8 - 39

    v3_channel *channel;
} _v3_msg_0x49;
int _v3_get_0x49(_v3_net_message *msg);
_v3_net_message *_v3_put_0x49(uint16_t subtype, uint16_t user_id, char *channel_password, _v3_msg_channel *channel);/*}}}*/
typedef struct _v3_net_message_0x4a {/*{{{*/
    uint32_t type;                    // 0
    uint32_t subtype;                 // 4
    uint8_t unknown_1[16];            // 8
    uint8_t hash_password[32];        // 24

    uint8_t unknown_2[4];             // 56

    uint8_t lock_acct;                // 60
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
typedef struct _v3_net_message_0x52_gsm {/*{{{*/
    uint32_t type;                    // 0
    uint16_t subtype;                 // 4
    uint16_t user_id;                 // 6
    uint16_t codec;                   // 8
    uint16_t codec_format;            // 10
    uint16_t unknown_1[2];            // 12
    uint16_t length;                  // 16
    uint16_t unknown_2;               // 18
    uint16_t sound_speed;             // 20
    uint16_t unknown_3[3];            // 22

    uint8_t  **frames;                // 28 to whatever
} _v3_msg_0x52_gsm;/*}}}*/
typedef struct _v3_net_message_0x52_speex {/*{{{*/
    uint32_t type;                    // 0
    uint16_t subtype;                 // 4
    uint16_t user_id;                 // 6
    uint16_t codec;                   // 8
    uint16_t codec_format;            // 10
    uint16_t unknown_1[2];            // 12
    uint16_t length;                  // 16
    uint16_t unknown_2;               // 18
    uint16_t sound_speed;             // 20
    uint16_t unknown_3[3];            // 22

    uint16_t audio_count;             // 28
    uint16_t frame_size;              // 30
    uint8_t  **frames;                // 32 to whatever
} _v3_msg_0x52_speex;/*}}}*/
typedef struct _v3_net_message_0x52 {/*{{{*/
    /*
     * speex:
     *      libventrilo3: 12:32:52:         ======= received TCP packet =====================================
     *      libventrilo3: 12:32:52:         PACKET: message type: 0x52 (82)
     *      libventrilo3: 12:32:52:         PACKET: data length : 704
     *      libventrilo3: 12:32:52:         PACKET:     52 00 00 00 01 00 FD 02 03 00 20 00 02 00 00 00      R...............
     *      libventrilo3: 12:32:52:         PACKET:     A4 02 00 00 80 20 00 00 00 00 00 00 00 06 02 80      ................
     *      libventrilo3: 12:32:52:         PACKET:     00 6E 3B 91 BC 7D A9 56 67 37 39 69 A4 D0 77 54      .n;..}.Vg79i..wT
     *      libventrilo3: 12:32:52:         PACKET:     5C 94 03 BA 69 1F 83 83 B0 32 C8 DE 7D E0 EE FD      \...i....2..}...
     *      libventrilo3: 12:32:52:         PACKET:     75 AF A0 9A A1 AA 36 C5 72 53 22 0C 58 23 42 A5      u.....6.rS".X#B.
     *      libventrilo3: 12:32:52:         PACKET:     7C 86 66 CD 9A 31 65 6C 80 D8 8F AD 1B A3 EF CC      |.f..1el........
     *
     *  gsm:
     *      libventrilo3: 18:18:32:     ======= received TCP packet =====================================
     *      libventrilo3: 18:18:32:     PACKET: message type: 0x52 (82)
     *      libventrilo3: 18:18:32:     PACKET: data length : 353
     *      libventrilo3: 18:18:32:     PACKET:     52 00 00 00 01 00 04 00 00 00 01 00 02 00 00 00      R...............
     *      libventrilo3: 18:18:32:     PACKET:     45 01 00 00 38 0D 00 00 00 00 00 00 26 F7 8E C5      E...8.......&...
     *      libventrilo3: 18:18:32:     PACKET:     82 02 60 DB B6 6D 1C 57 06 60 E4 48 92 E3 D8 03      ..`..m.W.`.H....
     *      libventrilo3: 18:18:32:     PACKET:     80 63 C9 8D E4 66 4C 60 DB 38 72 1B A7 82 15 5D      .c...fL`.8r....]
     *
     * start message (0x00):
     *      libventrilo3: 19:42:16:                 PACKET:     52 00 00 00 00 00 05 00 03 00 20 00 00 00 00 00      R...............
     *      libventrilo3: 19:42:16:                 PACKET:     00 00 00 00 00 00 00 00 00 01 00 02 00 01 00 00      ................

     */
    uint32_t type;                    // 0
    uint16_t subtype;                 // 4
    uint16_t user_id;                 // 6
    uint16_t codec;                   // 8
    uint16_t codec_format;            // 10
} _v3_msg_0x52;
int _v3_get_0x52(_v3_net_message *msg);
int _v3_destroy_0x52(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x52_0x01 {/*{{{*/
    uint16_t type;                    // 0
    uint16_t empty;                   // 2
    uint16_t subtype;                 // 4
    uint16_t user_id;                 // 6
    uint16_t codec;                   // 8
    uint16_t codec_format;            // 10
    uint16_t unknown_1[2];            // 12
    uint16_t length;                  // 16
    uint16_t unknown_2;               // 18
    uint16_t sound_speed;             // 20
    uint16_t unknown_3[3];            // 22
} _v3_msg_0x52_0x01;/*}}}*/
typedef struct _v3_net_message_0x53 {/*{{{*/
    /*
     *  Message Type 0x53 is a channel change notification
     * libventrilo3: 14:20:30:         ======= received TCP packet =====================================
     * libventrilo3: 14:20:30:         PACKET: message type: 0x53 (83)
     * libventrilo3: 14:20:30:         PACKET: data length : 8
     * libventrilo3: 14:20:30:         PACKET:     53 00 00 00 16 00 0D 00                              S.......
     */
    uint32_t type;
    uint16_t user_id;
    uint16_t channel_id;
} _v3_msg_0x53;
int _v3_get_0x53(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x57 {/*{{{*/
    /*
     *  Message Type 0x57 holds information about the server.  The server sends
     *  this message during the login phase.
     */
    uint32_t type;			// 0
    uint32_t unknown_1;			// 4
    uint16_t port;			// 8
    uint16_t max_clients;		// 10
    uint16_t connected_clients;		// 12
    uint8_t  unknown_2[14];		// 14
    char     name[32];			// 28
    char     version[16];		// 60
    uint8_t  unknown_3[32];		// 76

} _v3_msg_0x57;
int _v3_get_0x57(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x59 {/*{{{*/
    /*
     *  Note: this structure is out of alignment on 64bit machines
     *
     *  Message Type 0x59 is a server error message
     *  libventrilo3: 09:25:28:         PACKET:     59 00 00 00 25 00 00 00 00 00 01 00 00 00 00 00      Y...%...........
     *  libventrilo3: 09:25:28:         PACKET:     00 00 00 00 00 04 61 73 64 66                        ......asdf
     */
    uint32_t type;               // 0
    uint16_t error;              // 4
    uint16_t minutes_banned;     // 6
    uint16_t log_error;          // 8
    uint16_t close_connection;   // 10
    uint16_t unknown_[4];        // 12
    char *   message;            // 20

} __attribute__ ((packed)) _v3_msg_0x59;
int _v3_get_0x59(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x5c {/*{{{*/
    uint32_t type;
    uint16_t subtype;
    uint16_t sum_1;
    uint32_t sum_2;
} _v3_msg_0x5c;
int _v3_get_0x5c(_v3_net_message *msg);
_v3_net_message *_v3_put_0x5c(uint8_t subtype);
uint32_t _v3_msg5c_scramble(uint8_t* in);
uint32_t _v3_msg5c_gensum(uint32_t seed, uint32_t iterations);/*}}}*/
typedef struct _v3_net_message_0x5d {/*{{{*/
    /*
       User list update

       libventrilo3: 19:23:53:     ======= received TCP packet =====================================
       libventrilo3: 19:23:53:     PACKET: message type: 0x5D (93)
       libventrilo3: 19:23:53:     PACKET: data length : 72
       libventrilo3: 19:23:53:     PACKET:     5D 00 00 00 04 00 02 00 00 00 00 00 02 00 00 00      ]...............
       libventrilo3: 19:23:53:     PACKET:     00 08 53 65 72 76 65 72 20 31 00 08 53 65 72 76      ..Server.1..Serv
       libventrilo3: 19:23:53:     PACKET:     65 72 20 31 00 00 00 00 00 00 16 00 00 00 01 04      er.1............
       libventrilo3: 19:23:53:     PACKET:     00 00 00 04 65 72 69 63 00 08 70 68 6F 6E 65 74      ....eric..phonet
       libventrilo3: 19:23:53:     PACKET:     69 63 00 00 00 00 00 00                              ic......
     */
    uint32_t type;                    // 0
    uint16_t subtype;                 // 4
    uint16_t user_count;              // 6
    _v3_msg_user *lobby;              // 8 - variable length starts here
    _v3_msg_user *user_list;
} _v3_msg_0x5d;
_v3_net_message *_v3_put_0x5d(uint16_t subtype, uint16_t count, v3_user *user);
int _v3_get_0x5d(_v3_net_message *msg);
int _v3_destroy_0x5d(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x60 {/*{{{*/
    /*
     * Channel list update
     *
     * libventrilo3: 19:38:52:     ======= received TCP packet =====================================
     * libventrilo3: 19:38:52:     PACKET: message type: 0x60 (96)
     * libventrilo3: 19:38:52:     PACKET: data length : 216
     * libventrilo3: 19:38:52:     PACKET:     60 00 00 00 03 00 00 00 01 00 00 00 00 01 00 00      `...............
     * libventrilo3: 19:38:52:     PACKET:     01 00 01 00 01 00 01 00 01 00 01 00 00 00 00 00      ................
     * libventrilo3: 19:38:52:     PACKET:     00 00 00 00 01 00 00 00 01 00 00 00 00 00 00 00      ................
     * libventrilo3: 19:38:52:     PACKET:     FF FF FF FF 00 00 00 00 00 0C 74 65 73 74 5F 63      ..........test_c
     * libventrilo3: 19:38:52:     PACKET:     68 61 6E 6E 65 6C 00 0D 74 65 73 74 5F 70 68 6F      hannel..test_pho
     * libventrilo3: 19:38:52:     PACKET:     6E 65 74 69 63 00 0C 74 65 73 74 5F 63 6F 6D 6D      netic..test_comm
     * libventrilo3: 19:38:52:     PACKET:     65 6E 74 0E 00 01 00 00 00 00 00 01 00 01 00 01      ent.............
     * libventrilo3: 19:38:52:     PACKET:     00 01 00 01 00 01 00 00 00 00 00 00 00 00 00 01      ................
     * libventrilo3: 19:38:52:     PACKET:     00 00 00 01 00 00 00 00 00 00 00 FF FF FF FF 00      ................
     * libventrilo3: 19:38:52:     PACKET:     00 00 00 00 05 74 65 73 74 32 00 00 00 00 0F 00      .....test2......
     * libventrilo3: 19:38:52:     PACKET:     0E 00 00 00 00 00 01 00 01 00 01 00 01 00 01 00      ................
     * libventrilo3: 19:38:52:     PACKET:     01 00 00 00 00 00 00 00 00 00 01 00 00 00 01 00      ................
     * libventrilo3: 19:38:52:     PACKET:     00 00 00 00 00 00 FF FF FF FF 00 00 00 00 00 04      ................
     * libventrilo3: 19:38:52:     PACKET:     74 65 73 74 00 00 00 00                              test....
     */
    uint32_t type;                    	// 0
    uint32_t channel_count;		// 4
    _v3_msg_channel *channel_list;
} _v3_msg_0x60;
int _v3_get_0x60(_v3_net_message *msg);
int _v3_destroy_0x60(_v3_net_message *msg);/*}}}*/
typedef struct _v3_net_message_0x61 {/*{{{*/
    /*
     * Message Type 0x61 is used for adding, removing, and listing user bans.
     * Bitmasks can be found in _v3_bitmasks.
     */
    uint32_t type;        	// 0
    uint32_t subtype;     	// 4
    uint32_t bitmask_id;  	// 8
    uint32_t ip_address;   	// 12
    uint16_t ban_count;       	// 16
    uint16_t ban_id;       	// 18
    uint8_t  banned_user[32];   // 20
    uint8_t  banned_by[32];	// 52
    uint8_t  ban_msg[128];	// 84
} _v3_msg_0x61;/*}}}*/
typedef struct _v3_net_message_0x62 {/*{{{*/
    /*
     * Message Type 0x62 is used for sending and receiving pages.
     * Errors can be found in _v3_page_errors.
     */
    uint32_t type;		// 0
    uint16_t from;		// 4
    uint16_t to;		// 6
    uint32_t error_id;		// 8
} _v3_msg_0x62;/*}}}*/


char *   _v3_get_msg_string(void *offset, uint16_t *len);
int      _v3_get_msg_channel(void *offset, _v3_msg_channel *channel);
int      _v3_put_msg_channel(char *buf, _v3_msg_channel *channel);
int      _v3_get_msg_user(void *offset, _v3_msg_user *user);
int      _v3_put_msg_user(void *buf, _v3_msg_user *user);

