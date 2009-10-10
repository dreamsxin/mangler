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

#ifndef _MANGLERSETTINGS_H
#define _MANGLERSETTINGS_H
#include <iostream>
#include <fstream>

class ManglerServerSettings/*{{{*/
{
    public:
        std::string         name;
        std::string         port;
        std::string         username;
        std::string         password;
        std::string         phonetic;
        std::string         comment;
        std::string         url;

        ManglerServerSettings() {
        }
};/*}}}*/
class ManglerSettings
{
    public:
        ManglerSettings(Glib::RefPtr<Gtk::Builder> builder);
        Glib::RefPtr<Gtk::Builder> builder;
        Gtk::Window         *settingsWindow;
        Gtk::Button         *button;
        Gtk::Label          *label;
        Gtk::Window         *window;
        Gtk::CheckButton    *checkbutton;
        bool                isDetectingKey;
        bool                isDetectingMouse;

        class _config {
            public:
                Glib::Mutex     mutex;
                uint32_t        lv3_debuglevel;
                bool            PushToTalkKeyEnabled;
                std::string     PushToTalkKeyValue;
                bool            PushToTalkMouseEnabled;
                std::string     PushToTalkMouseValue;
                ManglerServerSettings   qc_lastserver;
                std::vector<ManglerServerSettings> serverlist;

                _config() {
                    lv3_debuglevel              = 0;
                    PushToTalkKeyEnabled        = false;
                    PushToTalkKeyValue          = "";
                    PushToTalkMouseEnabled      = false;
                    PushToTalkMouseValue        = "";
                    ManglerServerSettings       qc_lastserver;
                    std::vector<ManglerServerSettings> serverlist;
                }
                void save() {
                    mutex.lock();
                    std::ofstream   f;
                    std::string     cfgfilename = getenv("HOME");
                    cfgfilename += "/.manglerrc";
                    std::cerr << "using config file " << cfgfilename << std::endl;
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
                    f << "lv3_debuglevel=" << lv3_debuglevel << std::endl;
                    f.close();
                    mutex.unlock();
                }
                std::string get(std::string cfgname) {
                    mutex.lock();
                    std::ifstream   f;
                    std::string     cfgfilename = getenv("HOME");
                    cfgfilename += "/.manglerrc";
                    f.open((char *)cfgfilename.c_str());
                    while (! f.eof()) {
                        std::string cfgline;
                        f >> cfgline;
                        std::string name = cfgline.substr(0, cfgline.find("="));
                        if (name == cfgname) {
                            std::string value = cfgline.substr(cfgline.find("=")+1);
                            mutex.unlock();
                            return value;
                        }
                    }
                    mutex.unlock();
                    return "";
                }
                void load() {
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
                }
        }   config;

        // members functions
        void showSettingsWindow(void);
        bool settingsPTTKeyDetect(void);
        bool settingsPTTMouseDetect(void);

        // callbacks
        void settingsWindow_show_cb(void);
        void settingsWindow_hide_cb(void);
        void settingsCancelButton_clicked_cb(void);
        void settingsApplyButton_clicked_cb(void);
        void settingsOkButton_clicked_cb(void);
        void settingsEnablePTTKeyCheckButton_toggled_cb(void);
        void settingsPTTKeyButton_clicked_cb(void);
        void settingsEnablePTTMouseCheckButton_toggled_cb(void);
        void settingsPTTMouseButton_clicked_cb(void);

};

#endif
