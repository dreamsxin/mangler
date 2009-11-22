/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2009-10-10 12:38:51 -0700 (Sat, 10 Oct 2009) $
 * $Revision: 63 $
 * $LastChangedBy: ekilfoil $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglersettings.h $
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "mangler.h"
#include "manglerconfig.h"
#include <gdk/gdkx.h>


ManglerConfig::ManglerConfig() {/*{{{*/
    lv3_debuglevel                  = 0;
    windowWidth = 0;
    windowHeight = 0;
    buttonsHidden = false;
    PushToTalkKeyEnabled            = false;
    PushToTalkKeyValue              = "";
    PushToTalkMouseEnabled          = false;
    PushToTalkMouseValue            = "";
    AudioIntegrationEnabled         = false;
    AudioIntegrationPlayer          = "";
    notificationLoginLogout         = true;
    notificationChannelEnterLeave   = true;
    notificationTransmitStartStop   = true;
    mouseDeviceName                 = "";
    ManglerServerConfig             qc_lastserver;
    std::vector<ManglerServerConfig> serverlist;
    load();
}/*}}}*/
bool ManglerConfig::save() {/*{{{*/
    // We're going to do this in one in C because it's easier for me...
    mutex.lock();
    Glib::ustring     cfgfilename = getenv("HOME");
    cfgfilename += "/.manglerrc";
    if (! (this->cfgstream = fopen((char *)cfgfilename.c_str(), "w"))) {
        fprintf(stderr, "could not open settings file for writing: %s\n", (char *)cfgfilename.c_str());
        mutex.unlock();
        return false;
    }
    put("PushToTalkKeyEnabled", PushToTalkKeyEnabled);
    put("PushToTalkKeyValue", PushToTalkKeyValue);
    put("PushToTalkMouseEnabled", PushToTalkMouseEnabled);
    put("PushToTalkMouseValue", PushToTalkMouseValue);
    put("AudioIntegrationEnabled", AudioIntegrationEnabled);
    put("AudioIntegrationPlayer", AudioIntegrationPlayer);
    put("inputDeviceName", inputDeviceName);
    put("outputDeviceName", outputDeviceName);
    put("notificationDeviceName", notificationDeviceName);
    put("notification.loginLogout", notificationLoginLogout);
    put("notification.channelEnterLeave", notificationChannelEnterLeave);
    put("notification.transmitStartStop", notificationTransmitStartStop);
    put("mouseDeviceName", mouseDeviceName);
    put("qc_lastserver.hostname", qc_lastserver.hostname);
    put("qc_lastserver.port", qc_lastserver.port);
    put("qc_lastserver.username", qc_lastserver.username);
    put("qc_lastserver.password", qc_lastserver.password);
    put("qc_lastserver.phonetic", "");
    put("qc_lastserver.comment", "");
    put("lastConnectedServerId", lastConnectedServerId);
    put("lv3_debuglevel", lv3_debuglevel);
    put("window.width", windowWidth);
    put("window.height", windowHeight);
    put("window.buttonsHidden", buttonsHidden);

    for (uint32_t ctr = 0; ctr < serverlist.size(); ctr++) {
        put(ctr, *serverlist[ctr]);
    }
    fclose(this->cfgstream);
    mutex.unlock();
    return true;
}/*}}}*/
bool ManglerConfig::put(Glib::ustring name, Glib::ustring value) {/*{{{*/
    char buf[1024];
    snprintf(buf, 1023, "%s=%s\n", (char *)name.c_str(), (char *)value.c_str());
    if (!fputs(buf, this->cfgstream)) {
        return false;
    }
    return true;
}/*}}}*/
bool ManglerConfig::put(Glib::ustring name, uint32_t value) {/*{{{*/
    char buf[1024];
    snprintf(buf, 1023, "%s=%d\n", (char *)name.c_str(), value);
    if (!fputs(buf, this->cfgstream)) {
        return false;
    }
    return true;
}/*}}}*/
bool ManglerConfig::put(Glib::ustring name, int32_t value) {/*{{{*/
    char buf[1024];
    snprintf(buf, 1023, "%s=%d\n", (char *)name.c_str(), value);
    if (!fputs(buf, this->cfgstream)) {
        return false;
    }
    return true;
}/*}}}*/
bool ManglerConfig::put(Glib::ustring name, bool value) {/*{{{*/
    char buf[1024];
    snprintf(buf, 1023, "%s=%d\n", (char *)name.c_str(), value ? 1 : 0);
    if (!fputs(buf, this->cfgstream)) {
        return false;
    }
    return true;
}/*}}}*/
bool ManglerConfig::put(uint16_t id, ManglerServerConfig server) {/*{{{*/
    char name[1024];

    // Please excuse the formatting, this is easier to maintain
    snprintf(name, 1023, "serverlist.%d.name",                 id); if (!put(name, server.name                 ))  return false;
    snprintf(name, 1023, "serverlist.%d.hostname",             id); if (!put(name, server.hostname             ))  return false;
    snprintf(name, 1023, "serverlist.%d.port",                 id); if (!put(name, server.port                 ))  return false;
    snprintf(name, 1023, "serverlist.%d.username",             id); if (!put(name, server.username             ))  return false;
    snprintf(name, 1023, "serverlist.%d.password",             id); if (!put(name, server.password             ))  return false;
    snprintf(name, 1023, "serverlist.%d.phonetic",             id); if (!put(name, server.phonetic             ))  return false;
    snprintf(name, 1023, "serverlist.%d.comment",              id); if (!put(name, server.comment              ))  return false;
    snprintf(name, 1023, "serverlist.%d.url",                  id); if (!put(name, server.url                  ))  return false;
    snprintf(name, 1023, "serverlist.%d.charset",              id); if (!put(name, server.charset              ))  return false;
    snprintf(name, 1023, "serverlist.%d.accept_u2u",           id); if (!put(name, server.acceptU2U            ))  return false;
    snprintf(name, 1023, "serverlist.%d.accept_pages",         id); if (!put(name, server.acceptPages          ))  return false;
    snprintf(name, 1023, "serverlist.%d.accept_privchat",      id); if (!put(name, server.acceptPrivateChat    ))  return false;
    snprintf(name, 1023, "serverlist.%d.allow_recording",      id); if (!put(name, server.allowRecording       ))  return false;
    snprintf(name, 1023, "serverlist.%d.persistent_comments",  id); if (!put(name, server.persistentComments   ))  return false;
    snprintf(name, 1023, "serverlist.%d.motdhash",             id); if (!put(name, server.motdhash             ))  return false;
    return true;
}/*}}}*/
Glib::ustring ManglerConfig::get(Glib::ustring cfgname) {/*{{{*/
    // We're going to do this in one in C too...
    struct stat cfgstat;
    static bool notified = false;
    char buf[1024];
    int ctr = 0;

    mutex.lock();
    Glib::ustring     cfgfilename = getenv("HOME");
    cfgfilename += "/.manglerrc";
    // check to see if the file exists
    if (stat(cfgfilename.c_str(), &cfgstat)) {
        // if not create it
        if (! (this->cfgstream = fopen(cfgfilename.c_str(), "w"))) {
            // if creation fails, print error
            if (!notified) {
                fprintf(stderr, "could not create settings file: %s\n", (char *)cfgfilename.c_str());
                notified = true;
            }
            mutex.unlock();
            return "";
        } else {
            fclose(this->cfgstream);
        }
    }
    if (! (this->cfgstream = fopen(cfgfilename.c_str(), "r"))) {
        if (!notified) {
            fprintf(stderr, "could not open settings file for reading: %s\n", (char *)cfgfilename.c_str());
            notified = true;
        }
        mutex.unlock();
    }

    while (! feof(this->cfgstream)) {
        char *name, *value;
        if (! fgets(buf, 1024, this->cfgstream)) {
            fclose(this->cfgstream);
            mutex.unlock();
            return "";
        }
        ctr++;
        if (buf[strlen(buf)-1] != '\n') {
            fprintf(stderr, "error in settings file: line %d is longer than 1024 characters\n", ctr);
            fclose(this->cfgstream);
            mutex.unlock();
            return "";
        }
        buf[strlen(buf)-1] = '\0';
        name = buf;
        if ((value = strchr(buf, '=')) == NULL) {
            continue;
        }
        *value = '\0';
        value++;
        if (strcmp((char *)cfgname.c_str(), name) == 0) {
            Glib::ustring uvalue = value;
            fclose(this->cfgstream);
            mutex.unlock();
            return uvalue;
        }
    }
    fclose(this->cfgstream);
    mutex.unlock();
    return "";
}/*}}}*/
void ManglerConfig::load() {/*{{{*/
    PushToTalkKeyEnabled          = get("PushToTalkKeyEnabled") == "1" ? true : false; // default false
    PushToTalkKeyValue            = get("PushToTalkKeyValue");
    parsePushToTalkValue(PushToTalkKeyValue);
    PushToTalkMouseEnabled        = get("PushToTalkMouseEnabled") == "1" ? true : false; // default false
    PushToTalkMouseValue          = get("PushToTalkMouseValue");
    if (PushToTalkMouseValue.length() > 6) {
        PushToTalkMouseValueInt = atoi(PushToTalkMouseValue.substr(6).c_str());
    }
    AudioIntegrationEnabled       = get("AudioIntegrationEnabled") == "1" ? true : false; // default false
    AudioIntegrationPlayer        = get("AudioIntegrationPlayer");
    inputDeviceName               = get("inputDeviceName");
    outputDeviceName              = get("outputDeviceName");
    notificationDeviceName        = get("notificationDeviceName");
    notificationLoginLogout       = get("notification.loginLogout") == "0" ? false : true; // default true
    notificationChannelEnterLeave = get("notification.channelEnterLeave") == "0" ? false : true; // default true
    notificationTransmitStartStop = get("notification.transmitStartStop") == "0" ? false : true; // default true
    mouseDeviceName               = get("mouseDeviceName");
    qc_lastserver.hostname        = get("qc_lastserver.hostname");
    qc_lastserver.port            = get("qc_lastserver.port");
    qc_lastserver.username        = get("qc_lastserver.username");
    qc_lastserver.password        = get("qc_lastserver.password");
    qc_lastserver.phonetic        = get("qc_lastserver.phonetic");
    qc_lastserver.comment         = get("qc_lastserver.comment");
    lv3_debuglevel                = atoi(get("lv3_debuglevel").c_str());
    windowWidth                   = atoi(get("window.width").c_str());
    windowHeight                  = atoi(get("window.height").c_str());
    buttonsHidden                 = get("window.buttonsHidden") == "1" ? true : false; // default false
    lastConnectedServerId         = atoi(get("lastConnectedServerId").c_str());
    for (uint32_t ctr = 0; ctr < serverlist.size(); ctr++) {
        delete serverlist[ctr];
    }
    serverlist.clear();
    for (int ctr = 0; ctr < 65535; ctr++) {
        char buf[1024];
        Glib::ustring base;
        snprintf(buf, 1023, "serverlist.%d.", ctr);
        base = buf;
        if (get(base + "name") == "") {
            break;
        } else {
            ManglerServerConfig *server = new ManglerServerConfig();
            server->name = get(base + "name");
            server->hostname = get(base + "hostname");
            server->port = get(base + "port");
            server->username = get(base + "username");
            server->password = get(base + "password");
            server->phonetic = get(base + "phonetic");
            server->comment = get(base + "comment");
            server->url = get(base + "url");
            server->charset = get(base + "charset");
            server->acceptU2U = get(base + "accept_u2u") == "0" ? false : true;
            server->acceptPages = get(base + "accept_pages") == "0" ? false : true;
            server->acceptPrivateChat = get(base + "accept_privchat") == "0" ? false : true;
            server->allowRecording = get(base + "allow_recording") == "0" ? false : true;
            server->persistentComments = get(base + "persistent_comments") == "0" ? false : true;
            server->motdhash = atoi(get(base + "motdhash").c_str());
            serverlist.push_back(server);
        }
    }
}/*}}}*/
void ManglerConfig::parsePushToTalkValue(Glib::ustring pttString) {/*{{{*/
    int begin = 0;
    uint32_t ctr;
    GdkWindow   *rootwin = gdk_get_default_root_window();

    Glib::ustring keyname;
    PushToTalkXKeyCodes.clear();
    for (ctr = 0; ctr < pttString.length(); ctr++) {
        if (pttString[ctr] == '+') {
            keyname = pttString.substr(begin, ctr-begin);
            begin = ctr+1;
            if (keyname[0] == '<' && keyname[keyname.length()-1] == '>') {
                keyname = keyname.substr(1, keyname.length() - 2);
            }
            int keycode = XKeysymToKeycode(GDK_WINDOW_XDISPLAY(rootwin), XStringToKeysym(keyname.c_str()));
            PushToTalkXKeyCodes.push_back(keycode);
        }
    }
    keyname = pttString.substr(begin, ctr-begin);
    if (keyname[0] == '<' && keyname[keyname.length()-1] == '>') {
        keyname = keyname.substr(1, keyname.length() - 2);
    }
    int keycode = XKeysymToKeycode(GDK_WINDOW_XDISPLAY(rootwin), XStringToKeysym(keyname.c_str()));
    PushToTalkXKeyCodes.push_back(keycode);
}/*}}}*/
uint32_t ManglerConfig::addserver(void) {/*{{{*/
    serverlist.push_back(new ManglerServerConfig);
    return serverlist.size() - 1;
}/*}}}*/
void ManglerConfig::removeserver(uint32_t id) {/*{{{*/
    uint32_t active_server;
    active_server = mangler->getActiveServer();
    delete serverlist[id];
    serverlist.erase(serverlist.begin() + id);
    // we need to rebuild the serverlist liststore since the ids are changed now
    mangler->serverList->serverListTreeModel->clear();
    for (uint32_t ctr = 0; ctr < serverlist.size(); ctr++) {
        ManglerServerConfig *server = serverlist[ctr];
        Gtk::TreeRow row = *(mangler->serverList->serverListTreeModel->append());
        row[mangler->serverList->serverListColumns.id] = ctr;
        row[mangler->serverList->serverListColumns.name] = server->name;
        row[mangler->serverList->serverListColumns.hostname] = server->hostname;
        row[mangler->serverList->serverListColumns.port] = server->port;
        row[mangler->serverList->serverListColumns.username] = server->username;
    }
    mangler->setActiveServer(active_server);
    return;
}/*}}}*/
ManglerServerConfig *ManglerConfig::getserver(uint32_t id) {/*{{{*/
    return serverlist.at(id);
}/*}}}*/
