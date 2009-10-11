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
		
#include <iostream>
#include <fstream>
#include "mangler.h"
#include "manglerconfig.h"


ManglerConfig::ManglerConfig() {/*{{{*/
    lv3_debuglevel              = 0;
    PushToTalkKeyEnabled        = false;
    PushToTalkKeyValue          = "";
    PushToTalkMouseEnabled      = false;
    PushToTalkMouseValue        = "";
    ManglerServerConfig         qc_lastserver;
    std::vector<ManglerServerConfig> serverlist;
    load();
}/*}}}*/
void ManglerConfig::save() {/*{{{*/
    mutex.lock();
    std::ofstream   f;
    std::string     cfgfilename = getenv("HOME");
    cfgfilename += "/.manglerrc";
    f.open((char *)cfgfilename.c_str());
    f << "PushToTalkKeyEnabled=" << PushToTalkKeyEnabled << std::endl;
    f << "PushToTalkKeyValue=" << PushToTalkKeyValue << std::endl;
    f << "PushToTalkMouseEnabled=" << PushToTalkMouseEnabled << std::endl;
    f << "PushToTalkMouseValue=" << PushToTalkMouseValue << std::endl;
    f << "qc_lastserver.name=" << qc_lastserver.name << std::endl;
    f << "qc_lastserver.port=" << qc_lastserver.port << std::endl;
    f << "qc_lastserver.username=" << qc_lastserver.username << std::endl;
    f << "qc_lastserver.password=" << qc_lastserver.password << std::endl;
    f << "qc_lastserver.phonetic=" << "" << std::endl;
    f << "qc_lastserver.comment=" << "" << std::endl;
    f << "lastConnectedServerId=" << "-1" << std::endl;
    f << "lv3_debuglevel=" << lv3_debuglevel << std::endl;
    // TODO: saveServerList();
    f.close();
    mutex.unlock();
}/*}}}*/
std::string ManglerConfig::get(std::string cfgname) {/*{{{*/
    mutex.lock();
    std::ifstream   f;
    f.exceptions ( std::ifstream::eofbit | std::ifstream::failbit | std::ifstream::badbit );
    std::string     cfgfilename = getenv("HOME");
    cfgfilename += "/.manglerrc";
    try {
        f.open((char *)cfgfilename.c_str());
    } catch (std::ifstream::failure e) {
        try {
            mutex.unlock();
            save();
            mutex.lock();
            f.open((char *)cfgfilename.c_str());
        } catch (std::ifstream::failure e) {
            fprintf(stderr, "could not create %s\n", (char *)cfgfilename.c_str());
            mutex.unlock();
            return "";
        };
    };
    while (! f.eof()) {
        std::string cfgline;
        try {
            f >> cfgline;
        } catch (std::ifstream::failure e) {
            mutex.unlock();
            return "";
        }
        std::string name = cfgline.substr(0, cfgline.find("="));
        if (name == cfgname) {
            std::string value = cfgline.substr(cfgline.find("=")+1);
            mutex.unlock();
            return value;
        }
    }
    mutex.unlock();
    return "";
}/*}}}*/
void ManglerConfig::load() {/*{{{*/
    PushToTalkKeyEnabled        = get("PushToTalkKeyEnabled") == "0" ? false : true;
    PushToTalkKeyValue          = get("PushToTalkKeyValue");
    PushToTalkMouseEnabled      = get("PushToTalkMouseEnabled") == "0" ? false : true;
    PushToTalkMouseValue        = get("PushToTalkMouseValue");
    qc_lastserver.name          = get("qc_lastserver.name");
    qc_lastserver.port          = get("qc_lastserver.port");
    qc_lastserver.username      = get("qc_lastserver.username");
    qc_lastserver.password      = get("qc_lastserver.password");
    qc_lastserver.phonetic      = get("qc_lastserver.phonetic");
    qc_lastserver.comment       = get("qc_lastserver.comment");
}/*}}}*/

