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

#ifndef _MANGLERCONFIG_H
#define _MANGLERCONFIG_H

class ManglerServerConfig/*{{{*/
{
    public:
        Glib::ustring         name;
        Glib::ustring         port;
        Glib::ustring         username;
        Glib::ustring         password;
        Glib::ustring         phonetic;
        Glib::ustring         comment;
        Glib::ustring         url;

        ManglerServerConfig() {
        }
};/*}}}*/
class ManglerConfig {
	public:
		Glib::Mutex     mutex;
		uint32_t        lv3_debuglevel;
		bool            PushToTalkKeyEnabled;
                Glib::ustring   PushToTalkKeyValue;
		bool            PushToTalkMouseEnabled;
                Glib::ustring   PushToTalkMouseValue;
                Glib::ustring   inputDeviceName;
                Glib::ustring   outputDeviceName;
		ManglerServerConfig   qc_lastserver;
		std::vector<ManglerServerConfig> serverlist;
                FILE            *cfgstream;

		ManglerConfig();
		bool save();
                Glib::ustring get(Glib::ustring cfgname);
                bool put(Glib::ustring name, bool value);
                bool put(Glib::ustring name, Glib::ustring value);
                bool put(Glib::ustring name, uint32_t value);
		void load();
};

#endif
