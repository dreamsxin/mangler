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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "mangler.h"
#include "manglerconfig.h"
#include <gdk/gdkx.h>
#include "config.h"

#include "manglerserverlist.h"

#include <cstdlib>
#include <sys/types.h>
#include <dirent.h>

// config directory, relative to $HOME
#define CONFIG_DIRECTORY ".mangler"

using namespace std;

class ManglerConfigDir {/*{{{*/
    friend class ManglerConfig;
    public:
    ManglerConfigDir(const string &configdir);
    private:
    static void ConvertOldConfig();
    static string filename(const string &name);
    static string confdir;
};/*}}}*/

string ManglerConfigDir::confdir;

ManglerConfigDir *ManglerConfig::confdir = new ManglerConfigDir( CONFIG_DIRECTORY );

ManglerConfigDir::ManglerConfigDir(const string &configdir) {/*{{{*/
    string homedir = getenv("HOME");
    if (! homedir.length() || homedir[homedir.length()-1] != '/') homedir += "/";
    confdir = homedir + configdir;
    if (confdir[confdir.length()-1] == '/') confdir.erase(confdir.length()-1, confdir.npos);
    DIR *confDIR = ::opendir(confdir.c_str());
    if (! confDIR) {
        if (::mkdir(confdir.c_str(), 0700)) {
            fprintf(stderr, "Unable to make directory '%s'\n", confdir.c_str());
            fprintf(stderr, "No configuration settings can be saved.\n");
        }
    }
    confdir += "/";
}/*}}}*/

void ManglerConfigDir::ConvertOldConfig() {/*{{{*/
    //char *convert_command = "[ -d ~/.mangler ] || mkdir ~/.mangler ; egrep ^serverlist ~/.manglerrc | sed -e 's/serverlist\.[0-9]*\.//' | sed -e 's/^name=\([A-Za-z_0-9-]*\)$/\n\[\1\]/' > ~/.mangler/servers.ini ; ( echo '[mangler]' ; egrep -v ^serverlist ~/.manglerrc ) > ~/.mangler/config.ini";
    //char *convert_command = "[ -d ~/.mangler ] || mkdir ~/.mangler ; egrep ^serverlist ~/.manglerrc | sed -e 's/serverlist\\.[0-9]*\\.//' | sed -e 's/^name=\\(.*\\)$/\\n\\[\\1\\]/' > ~/.mangler/servers.ini ; ( echo '[mangler]' ; egrep -v ^serverlist ~/.manglerrc ) > ~/.mangler/config.ini";
    const char *convert_command = "egrep ^serverlist ~/.manglerrc | sed -e 's/serverlist\\.[0-9]*\\.//' | sed -e 's/^name=\\(.*\\)$/\\n\\[\\1\\]/' > ~/.mangler/servers.ini ; ( echo '[mangler]' ; egrep -v ^serverlist ~/.manglerrc | sed -e 's/^window\\.\\(width\\|height\\)=\\(.*\\)$/Window\\1=\\2/' -e 's/^window\\.\\(buttonsHidden\\|serverInfoHidden\\|guestFlagHidden\\)=\\(.*\\)$/\\1=\\2/' -e 's/^notification\\.\\([A-Za-z]*\\)=\\(.*\\)$/Notification\\1=\\2/' -e '/^lastConnectedServerId=.*$/d'; echo -n 'LastConnectedServerName=' ; egrep \"^serverlist.`egrep '^lastConnectedServerId=.*' ~/.manglerrc | cut -d= -f2`.name=\" ~/.manglerrc | cut -d= -f2 ) > ~/.mangler/config.ini";
    ::system(convert_command);
}/*}}}*/

string ManglerConfigDir::filename(const string &name) {/*{{{*/
    return confdir + name;
}/*}}}*/

ManglerConfig::ManglerConfig() /*{{{*/
    : config( confdir->filename("config.ini") )
    , servers( confdir->filename("servers.ini") ) {
    // this is where we check to see if config.ini was loaded
    if (! config.contains("mangler")) {
        struct stat statbuf;
        string oldfile = getenv("HOME");
        if (oldfile[oldfile.length()-1] != '/') oldfile += "/.manglerrc";
        else oldfile += ".manglerrc";
        if (stat(oldfile.c_str(), &statbuf) == 0) ManglerConfigDir::ConvertOldConfig();
        // should have something now!!
        config.reload();
        servers.reload();
    }
    if (! config.contains("mangler")) {
        std::istringstream sin( DefaultConfiguration );
        config.load(sin);
    }
}/*}}}*/

ManglerConfig::~ManglerConfig() {/*{{{*/
    save(); // might as well :)
}/*}}}*/

void ManglerConfig::save() {/*{{{*/
    //mutex.lock();
    config.save();
    servers.save();
    //mutex.unlock();
}/*}}}*/

std::vector<int> ManglerConfig::PushToTalkXKeyCodes() const {/*{{{*/
    int begin = 0;
    uint32_t ctr;
    GdkWindow   *rootwin = gdk_get_default_root_window();
    vector<int> ret;
    if (! config.contains("mangler") || ! config.at("mangler").contains("PushToTalkKeyValue"))
        return ret;
    Glib::ustring pttString = config.at("mangler").at("PushToTalkKeyValue").toUString();
    Glib::ustring keyname;
    for (ctr = 0; ctr < pttString.length(); ctr++) {
        if (pttString[ctr] == '+') {
            keyname = pttString.substr(begin, ctr-begin);
            begin = ctr+1;
            if (keyname[0] == '<' && keyname[keyname.length()-1] == '>') {
                keyname = keyname.substr(1, keyname.length() - 2);
            }
            int keycode = XKeysymToKeycode(GDK_WINDOW_XDISPLAY(rootwin), XStringToKeysym(keyname.c_str()));
            ret.push_back(keycode);
        }
    }
    keyname = pttString.substr(begin, ctr-begin);
    if (keyname[0] == '<' && keyname[keyname.length()-1] == '>') {
        keyname = keyname.substr(1, keyname.length() - 2);
    }
    int keycode = XKeysymToKeycode(GDK_WINDOW_XDISPLAY(rootwin), XStringToKeysym(keyname.c_str()));
    ret.push_back(keycode);
    return ret;
}/*}}}*/

iniValue &ManglerConfig::operator[](const string &configVar) {/*{{{*/
    return config["mangler"][configVar];
}/*}}}*/

bool ManglerConfig::hasUserVolume(const string &server, const string &user) const {/*{{{*/
    string varname = string( "UserVolume[" ) + user + string( "]" );
    return servers.contains(server) && servers.at(server).contains(varname);
}/*}}}*/

iniValue &ManglerConfig::UserVolume(const string &server, const string &user) {/*{{{*/
    string varname = string( "UserVolume[" ) + user + string( "]" );
    return servers[server][varname];
}/*}}}*/

iniValue &ManglerConfig::ChannelPassword(const string &server, uint16_t channel) {/*{{{*/
    string varname = string( "ChannelPassword[" ) + iniVariant( channel ).toString() + string( "]" );
    return servers[server][varname];
}/*}}}*/

const char *ManglerConfig::DefaultConfiguration = "[mangler]\n"
"PushToTalkKeyEnabled=0\n"
"PushToTalkKeyValue=\n"
"PushToTalkMouseEnabled=0\n"
"PushToTalkMouseValue=\n"
"AudioIntegrationEnabled=0\n"
"AudioIntegrationPlayer=0\n"
"VoiceActivationEnabled=0\n"
"VoiceActivationSilenceDuration=2000\n"
"VoiceActivationSensitivity=25\n"
"InputDeviceName=\n"
"InputDeviceCustomName=\n"
"OutputDeviceName=\n"
"OutputDeviceCustomName=\n"
"NotificationDeviceName=\n"
"NotificationDeviceCustomName=\n"
"NotificationLoginLogout=1\n"
"NotificationChannelEnterLeave=1\n"
"NotificationTransmitStartStop=1\n"
"MouseDeviceName=\n"
#ifdef HAVE_PULSE
"AudioSubsystem=pulse\n"
#elif HAVE_ALSA
"AudioSubsystem=alsa\n"
#endif
"qc_lastserver.hostname=\n"
"qc_lastserver.port=\n"
"qc_lastserver.username=\n"
"qc_lastserver.password=\n"
"qc_lastserver.phonetic=1\n"
"qc_lastserver.comment=1\n"
"LastConnectedServerId=0\n"
"lv3_debuglevel=0\n"
"MasterVolumeLevel=79\n"
"WindowWidth=500\n"
"WindowHeight=400\n"
"ButtonsHidden=0\n"
"ServerInfoHidden=0\n"
"GuestFlagHidden=0\n"
"ChatTimestamps=0\n";

