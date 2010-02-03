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
#define V3_AUTHFAIL_CHANNEL         0x07

#define V3_JOIN_CHAT                0x00
#define V3_LEAVE_CHAT               0x01
#define V3_TALK_CHAT                0x02
#define V3_RCON_CHAT                0x03
#define V3_JOINFAIL_CHAT            0x04

#define V3_START_PRIV_CHAT          0x00
#define V3_END_PRIV_CHAT            0x01
#define V3_TALK_PRIV_CHAT           0x02
#define V3_BACK_PRIV_CHAT           0x03
#define V3_AWAY_PRIV_CHAT           0x04

#define V3_RANK_LIST                0x00
#define V3_ADD_RANK                 0x03
#define V3_REMOVE_RANK              0x04
#define V3_MODIFY_RANK              0x05

#define V3_REMOVE_USER              0x00
#define V3_ADD_USER                 0x01
#define V3_MODIFY_USER              0x02
#define V3_USER_LIST                0x04

#define V3_AUDIO_START              0x00
#define V3_AUDIO_DATA               0x01
#define V3_AUDIO_STOP               0x02
#define V3_AUDIO_UNK                0x03

#define V3_PHANTOM_ADD              0x00
#define V3_PHANTOM_REMOVE           0x01

#define V3_ADMIN_LOGIN              0x00
#define V3_ADMIN_KICK               0x01
#define V3_ADMIN_BAN                0x03
#define V3_ADMIN_LOGOUT             0x04
#define V3_ADMIN_CHANNEL_BAN        0x05

#define V3_USERLIST_OPEN            0x00
#define V3_USERLIST_ADD             0x01
#define V3_USERLIST_REMOVE          0x02
#define V3_USERLIST_MODIFY          0x03
#define V3_USERLIST_CLOSE           0x04
#define V3_USERLIST_LUSER           0x05
#define V3_USERLIST_CHANGE_OWNER    0x06

#define V3_SERVER_CHAT_FILTER       0x02
#define V3_SERVER_ALPHABETIC        0x03
#define V3_SERVER_MOTD_ALWAYS       0x05

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
#define V3_DEBUG_EVENT              1 << 13
#define V3_DEBUG_ALL                65535

typedef struct __v3_net_message {/*{{{*/
    uint16_t len;
    uint16_t type;
    char *data;
    void *contents;
    int (* destroy)(struct __v3_net_message *msg);
    struct __v3_net_message *next;
} _v3_net_message;/*}}}*/
#pragma pack(push)
#pragma pack(1)
struct _v3_permissions {/*{{{*/
    uint16_t account_id;
    uint16_t replace_owner_id;
    uint8_t hash_password[32];
    uint16_t rank_id;
    uint16_t unknown_perm_1;
    uint8_t lock_acct;
    uint8_t in_reserve_list;
    uint8_t dupe_ip;
    uint8_t switch_chan;
    uint16_t dfl_chan;
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
#pragma pack(pop)
typedef struct _v3_permissions v3_permissions;

/*
   Define event types to be used by the caller's event handler
 */
enum _v3_events
{
    // inbound or outbound event types
    V3_EVENT_STATUS = 1,
    V3_EVENT_PING,
    V3_EVENT_USER_LOGIN,
    V3_EVENT_USER_LOGOUT,
    V3_EVENT_LOGIN_COMPLETE,
    V3_EVENT_LOGIN_FAIL,
    V3_EVENT_USER_CHAN_MOVE,
    V3_EVENT_CHAN_ADD,
    V3_EVENT_CHAN_MODIFY,
    V3_EVENT_CHAN_REMOVE,
    V3_EVENT_CHAN_BADPASS,
    V3_EVENT_ERROR_MSG,
    V3_EVENT_USER_TALK_START,
    V3_EVENT_USER_TALK_END,
    V3_EVENT_PLAY_AUDIO,
    V3_EVENT_DISPLAY_MOTD,
    V3_EVENT_DISCONNECT,
    V3_EVENT_USER_MODIFY,
    V3_EVENT_CHAT_JOIN,
    V3_EVENT_CHAT_LEAVE,
    V3_EVENT_CHAT_MESSAGE,
    V3_EVENT_ADMIN_AUTH,
    V3_EVENT_CHAN_ADMIN_UPDATED,
    V3_EVENT_PRIVATE_CHAT_MESSAGE,
    V3_EVENT_PRIVATE_CHAT_START,
    V3_EVENT_PRIVATE_CHAT_END,
    V3_EVENT_PRIVATE_CHAT_AWAY,
    V3_EVENT_PRIVATE_CHAT_BACK,
    V3_EVENT_USERLIST_ADD,
    V3_EVENT_USERLIST_MODIFY,
    V3_EVENT_USERLIST_REMOVE,
    V3_EVENT_USERLIST_CHANGE_OWNER,
    V3_EVENT_USER_GLOBAL_MUTE_CHANGED,

    // outbound specific event types
    V3_EVENT_CHANGE_CHANNEL,
    V3_EVENT_PHANTOM_ADD,
    V3_EVENT_PHANTOM_REMOVE,
    V3_EVENT_ADMIN_LOGIN,
    V3_EVENT_ADMIN_LOGOUT,
    V3_EVENT_ADMIN_KICK,
    V3_EVENT_ADMIN_BAN,
    V3_EVENT_ADMIN_CHANNEL_BAN,
    V3_EVENT_FORCE_CHAN_MOVE,
    V3_EVENT_USERLIST_OPEN,
    V3_EVENT_USERLIST_CLOSE,

    // not implemented
    V3_EVENT_USER_PAGED,
    V3_EVENT_CHAN_REMOVED,
    V3_EVENT_CHAN_MODIFIED,
    V3_EVENT_RECV_AUDIO,
    V3_EVENT_SERVER_PROPERTY_UPDATED,
};


// different boot types for api function v3_admin_boot
enum _v3_boot_types {
    V3_BOOT_KICK,
    V3_BOOT_BAN,
    V3_BOOT_CHANNEL_BAN,
};

#define V3_AUDIO_SENDTYPE_UNK0   0x00  // possibly broadcast?
#define V3_AUDIO_SENDTYPE_UNK1   0x01  // possibly broadcast to lobby?
#define V3_AUDIO_SENDTYPE_U2CCUR 0x02  // user to current channel
#define V3_AUDIO_SENDTYPE_U2C    0x03  // user to specific channel
#define V3_AUDIO_SENDTYPE_U2CSUB 0x04  // user to channel and all subchannels
#define V3_AUDIO_SENDTYPE_U2U    0x05  // user to user
#define V3_AUDIO_SENDTYPE_U2TARG 0x06  // user to voice target

// v3_event.flags values for V3_EVENT_USER_LOGIN
#define V3_LOGIN_FLAGS_EXISTING (1 << 0)    // user was added from userlist sent at login (existing user)

typedef struct _v3_event v3_event;
struct _v3_event {
    uint16_t type;
    time_t timestamp;
    struct {
        uint8_t percent;
        char message[256];
    } status;
    struct {
        uint16_t code;
        uint8_t  disconnected;
        char message[512];
    } error;
    uint16_t ping;
    struct {
        uint16_t id;
        uint16_t privchat_user1;
        uint16_t privchat_user2;
    } user;
    struct {
        uint16_t id;
    } channel;
    struct {
        uint16_t id;
        uint16_t id2;
    } account;
    struct {
        char name[32];
        char password[32];
        char phonetic[32];
        char comment[128];
        char url[128];
        char integration_text[128];
    } text;
    uint32_t flags;
    struct {
        uint32_t length;
        uint16_t send_type;
        uint32_t rate;
        uint8_t  channels;
    } pcm;
    struct {
        uint16_t property;
        uint8_t  value;
    } serverproperty;
    union {
        struct {
            v3_permissions perms;
            char username[32];
            char owner[32];
            char notes[256];
            char lock_reason[128];
            int chan_admin_count;
            uint16_t chan_admin[32];
            int chan_auth_count;
            uint16_t chan_auth[32];
        } account;
        int16_t sample16[16384];
        uint8_t sample[32768];
        char    motd[2048];
        char    chatmessage[256];
        char    reason[128];
    } data;
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
#define V3_USER_GLOBAL_MUTE         0x04

#define V3_MAX_USER_SIZE            512
#define V3_MAX_CHANNEL_SIZE         512
#define V3_MAX_STRING_SIZE          512
typedef struct __v3_msg_user {/*{{{*/
    uint16_t id;
    uint16_t channel;

    uint16_t bitfield;
    uint16_t rank_id;
    char *name;
    char *phonetic;
    char *comment;
    char *url;
    char *integration_text;

    /*
     * Put locally used user state and internal variables here
     */
    uint8_t  is_transmitting;
    uint8_t  accept_pages;
    uint8_t  accept_u2u;
    uint8_t  accept_chat;
    uint8_t  allow_recording;
    uint8_t  global_mute;
    uint8_t  guest;
    void     *next;
    uint16_t real_user_id; // used for phantom users
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
typedef struct __v3_msg_rank {/*{{{*/
    uint16_t id;
    uint16_t level;
    char *name;
    char *description;

    /*
     * Put internal variables here
     */
    void     *next;
} _v3_msg_rank;/*}}}*/
typedef struct __v3_msg_account {/*{{{*/
    v3_permissions perms;
    char *username;
    char *owner;
    char *notes;
    char *lock_reason;
    int chan_admin_count;
    uint16_t *chan_admin;
    int chan_auth_count;
    uint16_t *chan_auth;

    /*
     * Put internal variables here
     */
    void     *next;
} _v3_msg_account;/*}}}*/
typedef _v3_msg_account  v3_account;
typedef _v3_msg_user     v3_user;
typedef _v3_msg_channel  v3_channel;
typedef _v3_msg_rank     v3_rank;

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

typedef struct __v3_codec {
    uint8_t  codec;
    uint8_t  format;
    uint16_t pcmframesize;
    uint32_t rate;
    uint8_t  quality;
    char     name[128];
} v3_codec;
extern v3_codec v3_codecs[];

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
    int evpipe[2];                    // This is a pipe that libventrilo3 listens on for outbound events
    FILE *evinstream;                 // The inbound stream for the event queue pipe
    FILE *evoutstream;                // The outbound stream for the event queue pipe
    uint16_t codec;                   // The server's default codec
    uint16_t codec_format;            // The server's default codec format
    ventrilo_key_ctx server_key;      // The key used for decrypting messages from the server
    ventrilo_key_ctx client_key;      // The key used for encrypting messages to the server
    _v3_net_message *_queue;          // This queue (linked list) is used internally
    _v3_net_message *queue;           // This queue (linked list) stores messages that need to be processed by the client
    struct timeval last_timestamp;    // The time() of the last timestamp, a timestamp is sent every 10 seconds
    uint32_t recv_packet_count;       // Total amount of packets received from server.
    uint32_t recv_byte_count;         // Total amount of bytes received from server.
    uint32_t sent_packet_count;       // Total amount of packets received from server.
    uint32_t sent_byte_count;         // Total amount of bytes received from server.
    uint8_t motd_always;              // Always display MOTD
    uint8_t global_chat_filter;       // Global or Per Channel chat filter
    uint8_t channels_alphabetical;    // Display the channels alphabetical or manual
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
    v3_permissions perms;
    uint16_t channel_admin[65535];
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
void    _v3_print_permissions(v3_permissions *perms);

/*
 * External functions that are used by a program linking to the library
 */
int         v3_login(char *server, char *username, char *password, char *phonetic);
void        v3_join_chat(void);
void        v3_leave_chat(void);
void        v3_send_chat_message(char* message);
void        v3_start_privchat(uint16_t userid);
void        v3_end_privchat(uint16_t userid);
void        v3_send_privchat_message(uint16_t userid, char* message);
void        v3_send_privchat_away(uint16_t userid);
void        v3_send_privchat_back(uint16_t userid);
void        v3_logout(void);
void        v3_change_channel(uint16_t channel_id, char *password);
void        v3_admin_login(char *password);
void        v3_admin_logout(void);
void        v3_admin_boot(enum _v3_boot_types type, uint16_t user_id, char *reason);
void        v3_phantom_add(uint16_t channel_id);
void        v3_phantom_remove(uint16_t channel_id);
void        v3_force_channel_move(uint16_t user_id, uint16_t channel_id);
void        v3_userlist_open(void);
void        v3_userlist_close(void);
void        v3_userlist_remove(uint16_t account_id);
void        v3_userlist_update(v3_account *account);
void        v3_userlist_change_owner(uint16_t old_owner_id, uint16_t new_owner_id);
int         v3_debuglevel(uint32_t level);
int         v3_is_loggedin(void);
uint16_t    v3_get_user_id(void);
void        v3_set_text(char *comment, char *url, char *integration_text, uint8_t silent);
int         v3_message_waiting(int block);
uint16_t    *v3_get_soundq(uint32_t *len);
uint32_t    v3_get_soundq_length(void);
v3_event    *v3_get_event(int block);
int         v3_get_max_clients(void);
uint32_t    v3_get_bytes_recv(void);
uint32_t    v3_get_bytes_sent(void);
uint32_t    v3_get_packets_recv(void);
uint32_t    v3_get_packets_sent(void);
void        v3_clear_events(void);
uint32_t    v3_get_codec_rate(uint16_t codec, uint16_t format);
const v3_codec *v3_get_codec(uint16_t codec, uint16_t format);
const v3_codec *v3_get_channel_codec(uint16_t channel_id);
uint16_t    v3_get_user_channel(uint16_t id);
uint16_t    v3_channel_requires_password(uint16_t channel_id);
void        v3_start_audio(uint16_t send_type);
uint32_t    v3_send_audio(uint16_t send_type, uint32_t rate, uint8_t *pcm, uint32_t length, uint8_t celt_stereo);
void        v3_stop_audio(void);
void        v3_set_server_opts(uint8_t type, uint8_t value);
const v3_permissions *v3_get_permissions(void);
uint8_t     v3_is_channel_admin(uint16_t channel_id);


// User list functions
int         v3_user_count(void);
void        v3_free_user(v3_user *user);
v3_user     *v3_get_user(uint16_t id);

// Channel list functions
int         v3_channel_count(void);
void        v3_free_channel(v3_channel *channel);
v3_channel  *v3_get_channel(uint16_t id);

// Rank list functions
v3_rank     *v3_get_rank(uint16_t id);
void        v3_free_rank(v3_rank *rank);

// Account list functions
v3_account *v3_get_account(uint16_t id);
int         v3_account_count(void);
void        v3_free_account(v3_account *account);

// audio effects
void v3_set_volume_master(int level);
void v3_set_volume_user(uint16_t id, int level);
void v3_set_volume_luser(int level);
uint8_t v3_get_volume_user(uint16_t id);
uint8_t v3_get_volume_luser(void);


#endif // _VENTRILO3_H

