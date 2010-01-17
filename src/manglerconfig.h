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

#ifndef _MANGLERCONFIG_H
#define _MANGLERCONFIG_H

class ManglerServerConfig/*{{{*/
{
    public:
        uint32_t              id;
        Glib::ustring         name;
        Glib::ustring         hostname;
        Glib::ustring         port;
        Glib::ustring         username;
        Glib::ustring         password;
        Glib::ustring         phonetic;
        Glib::ustring         comment;
        Glib::ustring         url;
        Glib::ustring         charset;
        bool                  acceptU2U;
        bool                  acceptPages;
        bool                  acceptPrivateChat;
        bool                  allowRecording;
        bool                  persistentConnection;
        bool                  persistentComments;
        uint32_t              motdhash;
        std::map <Glib::ustring, uint8_t> uservolumes;
        std::map <uint16_t, Glib::ustring> channelpass;

        ManglerServerConfig() {
            name = "";
            hostname = "";
            port = "";
            username = "";
            password = "";
            phonetic = "";
            comment = "";
            url = "";
            charset = "";
            motdhash = 0;
            acceptU2U = true;
            acceptPages = true;
            acceptPrivateChat = true;
            allowRecording = true;
            persistentConnection = true;
            persistentComments = true;
        }
};/*}}}*/
class ManglerConfig {
	public:
		Glib::Mutex     mutex;
		uint32_t        lv3_debuglevel;
		uint8_t         masterVolumeLevel;
		bool            PushToTalkKeyEnabled;
                Glib::ustring   PushToTalkKeyValue;
                std::vector<int>     PushToTalkXKeyCodes;
		bool            PushToTalkMouseEnabled;
                Glib::ustring   PushToTalkMouseValue;
                int             PushToTalkMouseValueInt;
		bool            AudioIntegrationEnabled;
                Glib::ustring   AudioIntegrationPlayer;
                Glib::ustring   inputDeviceName;
                Glib::ustring   outputDeviceName;
                Glib::ustring   notificationDeviceName;
                Glib::ustring   mouseDeviceName;
                Glib::ustring   audioSubsystem;
                bool            notificationLoginLogout;
                bool            notificationChannelEnterLeave;
                bool            notificationTransmitStartStop;
                int32_t         lastConnectedServerId;
                uint32_t        windowWidth;
                uint32_t        windowHeight;
                bool            buttonsHidden;
                bool            serverInfoHidden;
                bool            guestFlagHidden;
                bool            chatTimestamps;

		ManglerServerConfig   qc_lastserver;
		std::vector<ManglerServerConfig *> serverlist;
                FILE            *cfgstream;

		ManglerConfig();
		bool save();
                Glib::ustring get(Glib::ustring cfgname);
                std::map <Glib::ustring, uint8_t> get_user_volumes(Glib::ustring serverbase);
                std::map <uint16_t, Glib::ustring> get_channel_passwords(Glib::ustring serverbase);
                bool put(Glib::ustring name, bool value);
                bool put(Glib::ustring name, Glib::ustring value);
                bool put(Glib::ustring name, uint32_t value);
                bool put(Glib::ustring name, int32_t value);
                bool put(uint16_t id, ManglerServerConfig server);
		void load();
                void parsePushToTalkValue(Glib::ustring pttString);
                uint32_t addserver(void);
                ManglerServerConfig *getserver(uint32_t id);
                void removeserver(uint32_t id);

};

#endif
