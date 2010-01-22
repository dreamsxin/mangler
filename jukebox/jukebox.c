/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2010-01-14 20:30:10 -0800 (Thu, 14 Jan 2010) $
 * $Revision: 513 $
 * $LastChangedBy: ekilfoil $
 * $URL: http://svn.mangler.org/mangler/trunk/libventrilo3/lv3_test.c $
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

#define _GNU_SOURCE


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <strings.h>
#include <mpg123.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <getopt.h>
#include <pthread.h>

#include "../libventrilo3/ventrilo3.h"

// data types
struct _conninfo {
    char *server;
    char *username;
    char *password;
    char *channelid;
    char *path;
};

struct _musicfile {
    char *filename;
    int rate;
    int channels;
    int invalid;
    char *path;
    char *title;
    char *artist;
    char *album;
    char *genre;
};

typedef struct _musicfile musicfile;

// global vars
int debug = 0;
int should_exit = 0;
musicfile **musiclist;
int musicfile_count = 0;


// prototypes
void usage(char *argv[]);
void ctrl_c(int signum);
void *jukebox_connection(void *connptr);
void *jukebox_player(void *connptr);
mpg123_handle *open_mp3(char *filename);
void close_mp3(mpg123_handle *mh);
int get_mp3_frame(mpg123_handle *mh, int channels, int16_t *buf);
int get_id3_info(musicfile *musicfile);
char *id3strdup(mpg123_string *inlines);
int get_random_number(int min, int max);
void send_now_playing(int filenum);


void ctrl_c (int signum) {
    printf("disconnecting... ");
    v3_logout();
    printf("done\n");
    exit(0);
}

void usage(char *argv[]) {
    fprintf(stderr, "usage: %s -s hostname:port -u username -c channelid /path/to/music\n", argv[0]);
    exit(1);
}

void *jukebox_connection(void *connptr) {
    struct _conninfo *conninfo;
    _v3_net_message *msg;
    conninfo = connptr;
    if (debug >= 2) {
        v3_debuglevel(V3_DEBUG_ALL);
    }
    if (! v3_login(conninfo->server, conninfo->username, conninfo->password, "")) {
        fprintf(stderr, "could not log in to server: %s\n", _v3_error(NULL));
    }
    while ((msg = _v3_recv(V3_BLOCK)) != NULL) {
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
    }
    should_exit = 1;
    pthread_exit(NULL);
}

void *jukebox_player(void *connptr) {
    struct _conninfo *conninfo;
    mpg123_handle *mh;
    int connected = 0;
    int playing = 0;
    int filenum = -1;
    int16_t sendbuf[16384];
    v3_event *ev;

    conninfo = connptr;
    for (;;) {
        if ((ev = v3_get_event(V3_NONBLOCK))) {
            if (debug) {
                fprintf(stderr, "jukebox: got event type %d\n", ev->type);
            }
            switch (ev->type) {
                case V3_EVENT_DISCONNECT:
                    should_exit = 1;
                    free(ev);
                    pthread_exit(NULL);
                    break;
                case V3_EVENT_LOGIN_COMPLETE:
                    if (debug) {
                        fprintf(stderr, "login complete...");
                    }
                    v3_change_channel(atoi(conninfo->channelid), "");
                    v3_join_chat();
                    connected = 1;
                    break;
                case V3_EVENT_CHAT_MESSAGE:
                    if (strcmp(ev->data.chatmessage, "play worst band in the world") == 0) {
                        v3_send_chat_message("we don't have an Creed songs...");
                    } else if (strncmp(ev->data.chatmessage, "play ", 5) == 0) {
                        char *searchspec;
                        int ctr;
                        int found = 0;
                        searchspec = ev->data.chatmessage + 5;
                        for (ctr = 0; ctr < musicfile_count; ctr++) {
                            // make sure we have at least 1 thing that matches
                            // so  we don't end up in an endless loop
                            if (strcasestr(musiclist[ctr]->path, searchspec)) { 
                                found = 1;
                                break;
                            }
                        }
                        if (! found) {
                            v3_send_chat_message("no songs matched your request");
                        } else {
                            int attempts = 0;
                            v3_stop_audio();
                            close_mp3(mh);
                            mh = NULL;
                            // we have SOMETHING in the filelist that matches, but no guarantee that it's a song... try 10
                            // different matches before giving up
                            for (attempts = 0; attempts < 20; attempts++) {
                                filenum = get_random_number(0, musicfile_count-1);
                                if (debug) {
                                    fprintf(stderr, "checking for %s: %s\n", searchspec, musiclist[filenum]->path);
                                }
                                if (strcasestr(musiclist[filenum]->path, searchspec) == 0) {
                                    // this file didn't match, so just get a new random file and don't count this
                                    // attempt
                                    attempts--;
                                    continue;
                                }
                                if (debug) {
                                    fprintf(stderr, "found %s in %s\n", searchspec, musiclist[filenum]->path);
                                }
                                if (!(mh = open_mp3(musiclist[filenum]->path))) {
                                    if (debug) {
                                        fprintf(stderr, "could not open: %s\n", musiclist[filenum]->path);
                                    }
                                    continue;
                                }
                            }
                            if (attempts == 10) {
                                // give up and just pick a random song
                                v3_send_chat_message("Apparently something matched, but it doesn't appear to be a song... so I fail.  Here's something else");
                                playing = 0;
                            } else {
                                send_now_playing(filenum);
                                playing = 1;
                                v3_start_audio(V3_AUDIO_SENDTYPE_U2CCUR);
                            }
                        }
                    } else if (strcmp(ev->data.chatmessage, "next track") == 0) {
                        v3_stop_audio();
                        close_mp3(mh);
                        mh = NULL;
                        playing = 0;
                    }
                    break;
            }
            free(ev);
        }
        if (connected) {
            if (! playing) {
                while (! mh) {
                    filenum = get_random_number(0, musicfile_count-1);
                    if (!(mh = open_mp3(musiclist[filenum]->path))) {
                        if (debug) {
                            fprintf(stderr, "could not open: %s\n", musiclist[filenum]->path);
                        }
                    }
                }
                if (debug) {
                    fprintf(stderr, "playing: %s\n", musiclist[filenum]->path);
                }
                send_now_playing(filenum);
                v3_start_audio(V3_AUDIO_SENDTYPE_U2CCUR);
                playing = 1;
            }
            if (! mh) {
                fprintf(stderr, "mh is NULL?  unpossible!\n");
                exit(1);
            }
            if (get_mp3_frame(mh, 2, sendbuf)) { // TODO: fix channel count!!!
                v3_send_audio(V3_AUDIO_SENDTYPE_U2CCUR, 44100, (uint8_t *)sendbuf, 640*15);
                usleep(108000);
            } else {
                v3_stop_audio();
                fprintf(stderr, "no more frames or some error\n");
                close_mp3(mh);
                mh = NULL;
                playing = 0;
            }
        }
    }
    pthread_exit(NULL);
}

void send_now_playing(int filenum) {
    char msgbuf[255];

    if (!get_id3_info(musiclist[filenum])) {
        if (debug) {
            fprintf(stderr, "no valid id3 tag: %s\n", musiclist[filenum]->path);
        }
    }
    snprintf(msgbuf, 254, "playing: ");
    if (musiclist[filenum]->artist && strlen(musiclist[filenum]->artist)) {
        strncat(msgbuf, musiclist[filenum]->artist, 254);
    }
    if (musiclist[filenum]->title && strlen(musiclist[filenum]->title)) {
        strncat(msgbuf, " - \"", 254);
        strncat(msgbuf, musiclist[filenum]->title, 254);
        strncat(msgbuf, "\"", 254);
    }
    if (musiclist[filenum]->album && strlen(musiclist[filenum]->album)) {
        strncat(msgbuf, " from ", 254);
        strncat(msgbuf, musiclist[filenum]->album, 254);
    }
    if (strcmp(msgbuf, "playing: ") == 0) {
        strncat(msgbuf, musiclist[filenum]->path, 254);
    }
    v3_send_chat_message(msgbuf);
}

void scan_media_path(char *path) {
    DIR *dir;
    struct dirent *ent;
    char namebuf[2048];
    char *cptr;
    struct stat s;

    if (! (dir = opendir(path))) {
        fprintf(stderr, "could not open diretory: %s\n", path);
        exit(1);
    }
    while ((ent = readdir(dir))) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }
        snprintf(namebuf, 2047, "%s/%s", path, ent->d_name);
        if (stat(namebuf, &s) != 0) {
            continue;
        }
        if (S_ISDIR(s.st_mode)) {
            if (debug) {
                fprintf(stderr, "found directory: %s\n", namebuf);
            }
            scan_media_path(namebuf);
        } else {
            cptr = namebuf + strlen(namebuf)-4;
            if (strcasecmp(cptr, ".mp3") != 0) {
                continue;
            }
            musiclist = realloc(musiclist, (musicfile_count+1) * sizeof(musicfile *));
            musiclist[musicfile_count] = malloc(sizeof(musicfile));
            memset(musiclist[musicfile_count], 0, sizeof(musicfile));
            musiclist[musicfile_count]->path = strdup(namebuf);
            /*
            if (!get_id3_info(musiclist[musicfile_count])) {
                free(musiclist[musicfile_count]);
                continue;
            }
            */
            musicfile_count++;
            if (debug) {
                fprintf(stderr, "found file #%d: %s\n", musicfile_count, namebuf);
            }
        }
    }
}

int get_id3_info(musicfile *musicfile) {
    mpg123_handle *mh;
    mpg123_id3v1 *v1;
    mpg123_id3v2 *v2;
    int meta;

    mpg123_init();
    if (debug) {
        fprintf(stderr, "scanning file %s\n", musicfile->path);
    }
    mh = mpg123_new(NULL, NULL);
    if(mpg123_open(mh, musicfile->path) != MPG123_OK) {
        if (debug) {
            fprintf(stderr, "cannot open %s\n", musicfile->path);
        }
        mpg123_exit();
        return 0;
    }
    if (mpg123_scan(mh) != MPG123_OK) {
        if (debug) {
            fprintf(stderr, "cannot scan %s\n", musicfile->path);
        }
        mpg123_close(mh);
        mpg123_delete(mh);
        mpg123_exit();
        return 0;
    }
    meta = mpg123_meta_check(mh);
    if (meta & MPG123_ID3 && mpg123_id3(mh, &v1, &v2) == MPG123_OK) {
        if(v2 != NULL) {
            musicfile->title  = id3strdup(v2->title);
            musicfile->artist = id3strdup(v2->artist);
            musicfile->album  = id3strdup(v2->album);
            musicfile->genre  = id3strdup(v2->genre);
            if (debug) {
                fprintf(stderr, "found an id3 tag on %s\n", musicfile->path);
            }
            /*
            fprintf(stderr, "title : %s\n", musicfile->title);
            fprintf(stderr, "artist: %s\n", musicfile->artist);
            fprintf(stderr, "album : %s\n", musicfile->album);
            fprintf(stderr, "genre : %s\n", musicfile->genre);
            */
        } else {
            if (debug) {
                fprintf(stderr, "no id3 tag on %s\n", musicfile->path);
            }
        }
    }
    mpg123_exit();
    return 1;
}

char *id3strdup(mpg123_string *inlines) {
    size_t i;
    int hadcr = 0, hadlf = 0;
    char *lines = NULL;
    char *line  = NULL;
    size_t len = 0;
    char *ret;

    if(inlines != NULL && inlines->fill)
    {
        lines = inlines->p;
        len   = inlines->fill;
    } else {
        return NULL;
    }

    line = lines;
    for(i=0; i<len; ++i) {
        if(lines[i] == '\n' || lines[i] == '\r' || lines[i] == 0) {
            char save = lines[i]; /* saving, changing, restoring a byte in the data */
            if(save == '\n') ++hadlf;
            if(save == '\r') ++hadcr;
            if((hadcr || hadlf) && hadlf % 2 == 0 && hadcr % 2 == 0) line = "";

            if(line) {
                lines[i] = 0;
                ret = strdup(line);
                return ret;
                // whatever else it's doing, we don't care...
                line = NULL;
                lines[i] = save;
            }
        } else {
            hadlf = hadcr = 0;
            if(line == NULL) line = lines+i;
        }
    }
    return NULL;
}

mpg123_handle *open_mp3(char *filename) {
    mpg123_handle *mh = NULL;
    int err = MPG123_OK;
    long int rate = 0;
    int channels = 0;
    int encoding = 0;

    err = mpg123_init();
    if ( err != MPG123_OK) {
        fprintf( stderr, "error: trouble with mpg123: %s\n", mh==NULL ? mpg123_plain_strerror(err) : mpg123_strerror(mh) );
        close_mp3(mh);
        return NULL;
    }
    if ((mh = mpg123_new(NULL, &err)) == NULL) {
        fprintf( stderr, "error: could not create mpg123 handle\n");
        close_mp3(mh);
        return NULL;
    }
    if (mpg123_open(mh, filename) != MPG123_OK) {
        fprintf( stderr, "error: could not open %s\n", filename);
        close_mp3(mh);
        return NULL;
    }
    if (mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK ) {
        fprintf( stderr, "error: could not get format (?)\n");
        close_mp3(mh);
        return NULL;
    }
    if (rate != 44100) {
        fprintf( stderr, "error: sample rate %lu not supported\n", rate);
        close_mp3(mh);
        return NULL;
    }
    if(encoding != MPG123_ENC_SIGNED_16) {
        close_mp3(mh);
        fprintf(stderr, "error: unknown encoding: 0x%x!\n", encoding);
        return NULL;
    }
    mpg123_format_none(mh);
    mpg123_format(mh, rate, channels, encoding);
    return mh;
}

int get_mp3_frame(mpg123_handle *mh, int channels, int16_t *buf) {
    unsigned char readbuffer[640*2*15]; // 320 samples, 2 channels, 15 frames
    int16_t *readptr;
    size_t numdecoded;
    int err;

    if ((err = mpg123_read(mh, readbuffer, channels*640*15, &numdecoded)) != MPG123_DONE) {
        if (err != MPG123_OK) {
            fprintf(stderr, "got error : %d!\n", err);
            return 0;
        }
        readptr = (int16_t *)readbuffer;
        if (channels == 2) {
            int ctr;
            for (ctr = 0; ctr < 640*15/2; ctr++) {
                buf[ctr] = (readptr[ctr*2] + readptr[ctr*2+1]) / 2;
            }
        }
    } else {
        return 0;
    }
    return 1;
}

void close_mp3(mpg123_handle *mh) {
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
}

int get_random_number(int min, int max) {
    max = max - min + 1;
    return min + (int)( ((float)max) * rand() / ( RAND_MAX + 1.0 ) );
}


int main(int argc, char *argv[]) {
    int opt;
    int rc;
    pthread_t network;
    pthread_t player;
    struct _conninfo conninfo;

    while ((opt = getopt(argc, argv, "dh:p:u:c:")) != -1) {
        switch (opt) {
            case 'd':
                debug++;
                break;
            case 'h':
                conninfo.server = strdup(optarg);
                break;
            case 'u':
                conninfo.username = strdup(optarg);
                break;
            case 'c':
                conninfo.channelid = strdup(optarg);
                break;
            case 'p':
                conninfo.password = strdup(optarg);
                break;
        }
    }
    if (! conninfo.server)  {
        fprintf(stderr, "error: server hostname (-h) was not specified\n");
        usage(argv);
    }
    if (! conninfo.username)  {
        fprintf(stderr, "error: username (-u) was not specified\n");
        usage(argv);
    }
    if (! conninfo.channelid) {
        fprintf(stderr, "error: channel id (-c) was not specified\n");
        usage(argv);
    }
    if (! conninfo.password) {
        conninfo.password = "";
    }
    if (optind >= argc) {
        fprintf(stderr, "error: path to music library not specified\n");
        usage(argv);
    }
    conninfo.path = strdup(argv[argc-1]);
    fprintf(stderr, "server: %s\nusername: %s\nmedia path: %s\n", conninfo.server, conninfo.username, conninfo.path);
    scan_media_path(conninfo.path);
    fprintf(stderr, "found %d files in music path\n", musicfile_count);
    rc = pthread_create(&network, NULL, jukebox_connection, (void *)&conninfo);
    rc = pthread_create(&player, NULL, jukebox_player, (void *)&conninfo);
    signal (SIGINT, ctrl_c);
    while (! should_exit) {
        sleep(1);
    }
    return(0);
}

