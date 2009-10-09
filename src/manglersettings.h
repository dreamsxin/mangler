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
