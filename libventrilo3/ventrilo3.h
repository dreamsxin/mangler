/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2009-10-01 13:25:43 -0700 (Thu, 01 Oct 2009) $
 * $Revision: 504 $
 * $LastChangedBy: ekilfoil $
 * $URL: http://svn.manglerproject.net/svn/mangler/trunk/libventrilo3/ventrilo3.h $
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

#ifndef _VENTRILO3_H
#define _VENTRILO3_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#define V3_BLOCK                    1
#define V3_NONBLOCK                 0

#define V3_OK                       0
#define V3_MALFORMED                1
#define V3_NOTIMPL                  2

#define V3_ADD_CHANNEL              0x01
#define V3_REMOVE_CHANNEL           0x02
#define V3_CHANGE_CHANNEL           0x03
#define V3_MODIFY_CHANNEL           0x05

#define V3_REMOVE_USER              0x00
#define V3_ADD_USER                 0x01
#define V3_MODIFY_USER              0x02
#define V3_USER_LIST                0x04

#define V3_DEBUG_NONE               0
#define V3_DEBUG_STATUS             1
#define V3_DEBUG_ERROR              1 << 2
#define V3_DEBUG_STACK              1 << 3
#define V3_DEBUG_INTERNAL           1 << 4
#define V3_DEBUG_PACKET             1 << 5
#define V3_DEBUG_PACKET_PARSE       1 << 6
#define V3_DEBUG_PACKET_ENCRYPTED   1 << 7
#define V3_DEBUG_MEMORY             1 << 8
#define V3_DEBUG_SOCKET             1 << 9
#define V3_DEBUG_NOTICE             1 << 10
#define V3_DEBUG_INFO               1 << 11
#define V3_DEBUG_MUTEX              1 << 12
#define V3_DEBUG_ALL                65535

typedef struct __v3_net_message {/*{{{*/
    uint16_t len;
    uint16_t type;
    char *data;
    void *contents;
    int (* destroy)(struct __v3_net_message *msg);
    struct __v3_net_message *next;
} _v3_net_message;/*}}}*/
struct _v3_permissions {/*{{{*/
    uint8_t lock_acct;
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
};/*}}}*/

/*
   Define event types to be used by the caller's event handler
 */
enum _v3_events
{
    V3_EVENT_STATUS,
    V3_EVENT_PING,
    V3_EVENT_USER_LOGIN,
    V3_EVENT_USER_LOGOUT,
    V3_EVENT_USER_MODIFY,
    V3_EVENT_USER_TALK_START,
    V3_EVENT_USER_TALK_END,
    V3_EVENT_USER_PAGED,
    V3_EVENT_USER_CHAN_MOVE,
    V3_EVENT_LUSER_FORCE_CHAN_MOVE,
    V3_EVENT_CHAN_ADDED,
    V3_EVENT_CHAN_REMOVED,
    V3_EVENT_CHAN_MODIFIED,
    V3_EVENT_RECV_AUDIO,
};

typedef struct _v3_event v3_event;
struct _v3_event {
    uint16_t type;
    time_t timestamp;
    struct {
        uint8_t percent;
        char message[256];
    } status;
    uint16_t ping;
    struct {
        uint16_t id;
    } user;
    struct {
        uint16_t id;
    } channel;

    v3_event *next;
};


/*
 *  These structures are used in multiple message types.  (i.e. _v3_msg_channel
 *  is used in retreiveing the channel list as well as the channel modification
 *  message).  These are also the structures for the user list and channel list
 */
#define V3_USER_ACCEPT_PAGES        0x00
#define V3_USER_ACCEPT_U2U          0x01
#define V3_USER_ALLOW_RECORD        0x02
#define V3_USER_ACCEPT_CHAT         0x03

#define V3_MAX_USER_SIZE            512
#define V3_MAX_CHANNEL_SIZE         512
#define V3_MAX_STRING_SIZE          512
typedef struct __v3_msg_user {/*{{{*/
    uint16_t id;
    uint16_t channel;

    uint32_t bitfield;
    char *name;
    char *phonetic;
    char *comment;
    char *integration_text;
    char *url;

    /*
     * Put locally used user state and internal variables here
     */
    uint8_t  is_transmitting;
    uint8_t  accept_pages;
    uint8_t  accept_u2u;
    uint8_t  accept_chat;
    uint8_t  allow_recording;
    void     *next;
} _v3_msg_user;/*}}}*/
typedef struct __v3_msg_channel {/*{{{*/
    /*                           offset:   0  1  2  3  4  5  6  7  8  9  10
     *    no channel pw: PACKET:     65 6E 02 00 01 00 00 00 00 00 01 00 01 00 01 00      en..............
     *  with channel pw: PACKET:     65 6E 02 00 01 00 00 01 00 00 01 00 01 00 01 00      en..............
     */
                                                // byte from initial offset
    uint16_t id;                                // 0
    uint16_t parent;                            // 2
    uint8_t  unknown_1;                         // 4
    uint8_t  password_protected;                // 5
    uint16_t unknown_2;                         // 6
    uint16_t allow_recording;                   // 8
    uint16_t allow_cross_channel_transmit;      // 10
    uint16_t allow_paging;                      // 12
    uint16_t allow_wave_file_binds;             // 14
    uint16_t allow_tts_binds;                   // 16
    uint16_t allow_u2u_transmit;                // 18
    uint16_t disable_guest_transmit;            // 20
    uint16_t disable_sound_events;              // 22
    uint16_t voice_mode;                        // 24
    uint16_t transmit_time_limit;               // 26
    uint16_t allow_phantoms;                    // 28
    uint16_t max_clients;                       // 30
    uint16_t allow_guests;                      // 32
    uint16_t inactive_exempt;                   // 34
    uint16_t protect_mode;                      // 36
    uint16_t transmit_rank_level;               // 38
    uint16_t channel_codec;                     // 40
    uint16_t channel_format;                    // 42
    uint16_t allow_voice_target;                // 44
    uint16_t allow_command_target;              // 46
    char     *name;                             // length of name 48 - is variable length, so the rest of the
                                                // channel is variable offset
    char     *phonetic;
    char     *comment;

    /*
     * Put locally used channel state and internal variables here
     */
    void     *next;
} _v3_msg_channel;/*}}}*/
typedef _v3_msg_user     v3_user;
typedef _v3_msg_channel  v3_channel;

/*
   This structure defines the bit number of each permission setting, the
   internal name, and a name suitable for display to a user.
 */
struct _v3_perm_info {
    uint8_t bitnum;
    char *name;
    char *display_name;
};

typedef struct {
    uint8_t     key[256];
    uint32_t    pos;
    uint32_t    size;
#ifdef VENTRILO_ALGO_PROTOCOL
    int         proto;
#endif
} ventrilo_key_ctx;


typedef struct __v3_server {
    uint32_t ip;                      // The server's IP address
    uint16_t port;                    // The server's TCP port number
    uint16_t max_clients;             // The maximum number of clients allowed
    uint16_t connected_clients;       // The number of clients currently connected
    char *name;                       // The name of the server
    char *version;                    // The version of the server
    char *os;                         // The OS the server is running on
    char *handshake_key;              // Authentication information
    char *handshake;                  // Authentication information
    char *motd;                       // Message of the day
    char *guest_motd;                 // Guest message of the day
    int auth_server_index;            // The array index of the authentication server
    ventrilo_key_ctx server_key;      // The key used for decrypting messages from the server
    ventrilo_key_ctx client_key;      // The key used for encrypting messages to the server
    _v3_net_message *_queue;          // This queue (linked list) is used internally
    _v3_net_message *queue;           // This queue (linked list) stores messages that need to be processed by the client
    struct timeval last_timestamp;    // The time() of the last timestamp, a timestamp is sent every 10 seconds
} _v3_server;

/*
 * This structure stores the local user's information
 */
typedef struct __v3_luser {
    int  id;
    char *name;
    char *password;
    char *phonetic;
    char *comment;
    char *integration_text;
    char *url;
    int  ping;
    uint8_t  accept_pages;
    uint8_t  accept_u2u;
    uint8_t  accept_chat;
    uint8_t  allow_recording;
    struct _v3_permissions perms;
} _v3_luser;

/*
 *  Internal functions defined here to make life easier for coding the library.
 *  These should eventually be moved into libventrilo3.c or implemented as
 *  external functions if required and should not be used by programs linking
 *  the library.
 */
void    _v3_debug(uint32_t level, const char *format, ...);
char *  _v3_error(const char *format, ...);
void    _v3_func_enter(char *func);
void    _v3_func_leave(char *func);
_v3_net_message *       _v3_recv(int block);
void    _v3_hash_password(uint8_t* password, uint8_t* hash);
int     _v3_process_message(_v3_net_message *msg);
int     _v3_close_connection(void);
int     _v3_is_connected(void);
void    _v3_print_user_list(void);   // testing function -- will be deleted
void    _v3_print_channel_list(void);   // testing function -- will be deleted

/*
 * External functions that are used by a program linking to the library
 */
int         v3_login(char *server, char *username, char *password, char *phonetic);
int         v3_logout(void);
int         v3_change_channel(uint16_t channel_id, char *password);
int         v3_debuglevel(uint32_t level);
int         v3_is_loggedin(void);
uint16_t    v3_get_user_id(void);
int         v3_set_text(char *comment, char *url, char *integration_text, uint8_t silent);
int         v3_message_waiting(int block);
uint16_t    *v3_get_soundq(uint32_t *len);
uint32_t    v3_get_soundq_length(void);
v3_event    *v3_get_event(int block);
int         v3_get_max_clients(void);

// User list functions
int         v3_user_count(void);
void        v3_free_user(v3_user *user);
v3_user     *v3_get_user(uint16_t id);

// Channel list functions
int         v3_channel_count(void);
void        v3_free_channel(v3_channel *channel);
v3_channel  *v3_get_channel(uint16_t id);

// Message queues
extern uint32_t                _v3_soundq_length;
extern uint16_t                *_v3_soundq;



#endif // _VENTRILO3_H

