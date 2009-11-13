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

#include "manglerconfig.h"

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
        ManglerConfig       config;

        // Audio Player List Combo Box Setup
        Gtk::ComboBox       *audioPlayerComboBox;
        class audioPlayerModelColumns : public Gtk::TreeModel::ColumnRecord
        {
            public:
                audioPlayerModelColumns() { add(id); add(name); }
                Gtk::TreeModelColumn<uint32_t>      id;
                Gtk::TreeModelColumn<Glib::ustring> name;
        };
        audioPlayerModelColumns  audioPlayerColumns;
        Glib::RefPtr<Gtk::ListStore> audioPlayerTreeModel;

        // Input Device Combo Box Setup
        Gtk::ComboBox       *inputDeviceComboBox;
        class inputDeviceModelColumns : public Gtk::TreeModel::ColumnRecord
        {
            public:
                inputDeviceModelColumns() { add(id); add(name); add(description); }
                Gtk::TreeModelColumn<uint32_t>      id;
                Gtk::TreeModelColumn<Glib::ustring> name;
                Gtk::TreeModelColumn<Glib::ustring> description;
        };
        inputDeviceModelColumns  inputColumns;
        Glib::RefPtr<Gtk::ListStore> inputDeviceTreeModel;

        // Output Device Combo Box Setup
        Gtk::ComboBox       *outputDeviceComboBox;
        class outputDeviceModelColumns : public Gtk::TreeModel::ColumnRecord
        {
            public:
                outputDeviceModelColumns() { add(id); add(name); add(description); }
                Gtk::TreeModelColumn<uint32_t>      id;
                Gtk::TreeModelColumn<Glib::ustring> name;
                Gtk::TreeModelColumn<Glib::ustring> description;
        };
        outputDeviceModelColumns  outputColumns;
        Glib::RefPtr<Gtk::ListStore> outputDeviceTreeModel;

        // Notification Device Combo Box Setup
        Gtk::ComboBox       *notificationDeviceComboBox;
        class notificationDeviceModelColumns : public Gtk::TreeModel::ColumnRecord
        {
            public:
                notificationDeviceModelColumns() { add(id); add(name); add(description); }
                Gtk::TreeModelColumn<uint32_t>      id;
                Gtk::TreeModelColumn<Glib::ustring> name;
                Gtk::TreeModelColumn<Glib::ustring> description;
        };
        notificationDeviceModelColumns  notificationColumns;
        Glib::RefPtr<Gtk::ListStore> notificationDeviceTreeModel;

        // members functions
        void showSettingsWindow(void);
        bool settingsPTTKeyDetect(void);
        bool settingsPTTMouseDetect(void);
        void applySettings(void);
        void initSettings(void);

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
        void settingsEnableAudioIntegrationCheckButton_toggled_cb(void);

};

#endif
