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

#include "mangler.h"
#include "manglersettings.h"
#include <gdk/gdkx.h>

ManglerSettings::ManglerSettings(Glib::RefPtr<Gtk::Builder> builder) {

    this->builder = builder;

    // Connect our signals for this window
    builder->get_widget("settingsWindow", settingsWindow);
    settingsWindow->signal_show().connect(sigc::mem_fun(this, &ManglerSettings::settingsWindow_show_cb));
    settingsWindow->signal_hide().connect(sigc::mem_fun(this, &ManglerSettings::settingsWindow_hide_cb));

    builder->get_widget("settingsCancelButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerSettings::settingsCancelButton_clicked_cb));

    builder->get_widget("settingsApplyButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerSettings::settingsApplyButton_clicked_cb));

    builder->get_widget("settingsOkButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerSettings::settingsOkButton_clicked_cb));

    builder->get_widget("settingsEnablePTTKeyCheckButton", checkbutton);
    checkbutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerSettings::settingsEnablePTTKeyCheckButton_toggled_cb));

    builder->get_widget("settingsPTTKeyButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerSettings::settingsPTTKeyButton_clicked_cb));

    builder->get_widget("settingsEnablePTTMouseCheckButton", checkbutton);
    checkbutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerSettings::settingsEnablePTTMouseCheckButton_toggled_cb));

    builder->get_widget("settingsPTTMouseButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerSettings::settingsPTTMouseButton_clicked_cb));

    builder->get_widget("inputDeviceComboBox", inputDeviceComboBox);
    inputDeviceTreeModel = Gtk::ListStore::create(inputColumns);
    inputDeviceComboBox->set_model(inputDeviceTreeModel);
    inputDeviceComboBox->pack_start(inputColumns.description);

    builder->get_widget("outputDeviceComboBox", outputDeviceComboBox);
    outputDeviceTreeModel = Gtk::ListStore::create(outputColumns);
    outputDeviceComboBox->set_model(outputDeviceTreeModel);
    outputDeviceComboBox->pack_start(outputColumns.description);
}

// Settings Window Callbacks
void ManglerSettings::showSettingsWindow(void) {/*{{{*/
    settingsWindow->show();
}/*}}}*/

void ManglerSettings::settingsWindow_show_cb(void) {/*{{{*/
    Gtk::TreeModel::Row row;
    isDetectingKey = false;
    isDetectingMouse = false;
    static int initialized = false;

    // these callbacks initialize the state
    settingsEnablePTTKeyCheckButton_toggled_cb();
    settingsEnablePTTMouseCheckButton_toggled_cb();

    // Clear the input device store and rebuild it from the audioControl vector
    inputDeviceTreeModel->clear();
    row = *(inputDeviceTreeModel->append());
    row[inputColumns.id] = -1;
    row[inputColumns.name] = "Default";
    row[inputColumns.description] = "Default";
    for (
            std::vector<ManglerAudioDevice*>::iterator i = mangler->audioControl->inputDevices.begin();
            i <  mangler->audioControl->inputDevices.end();
            i++) {
        Gtk::TreeModel::Row row = *(inputDeviceTreeModel->append());
        row[inputColumns.id] = (*i)->id;
        row[inputColumns.name] = (*i)->name;
        row[inputColumns.description] = (*i)->description;
    }
    // TODO: get the currently selected item from settings object and select it
    inputDeviceComboBox->set_active(0);

    // Clear the output device store and rebuild it from the audioControl vector
    outputDeviceTreeModel->clear();
    row = *(outputDeviceTreeModel->append());
    row[outputColumns.id] = -1;
    row[outputColumns.name] = "Default";
    row[outputColumns.description] = "Default";
    for (
            std::vector<ManglerAudioDevice*>::iterator i = mangler->audioControl->outputDevices.begin();
            i <  mangler->audioControl->outputDevices.end();
            i++) {
        Gtk::TreeModel::Row row = *(outputDeviceTreeModel->append());
        row[outputColumns.id] = (*i)->id;
        row[outputColumns.name] = (*i)->name;
        row[outputColumns.description] = (*i)->description;
    }
    // TODO: get the currently selected item from settings object and select it
    outputDeviceComboBox->set_active(0);
}/*}}}*/
void ManglerSettings::settingsWindow_hide_cb(void) {/*{{{*/
    isDetectingKey = false;
    isDetectingMouse = false;
}/*}}}*/
void ManglerSettings::settingsCancelButton_clicked_cb(void) {/*{{{*/
    // additional cleanup should happen in ManglerSettings::settingsWindow_hide_cb()
    settingsWindow->hide();
}/*}}}*/
void ManglerSettings::settingsApplyButton_clicked_cb(void) {/*{{{*/
    fprintf(stderr, "settings window Apply button clicked\n");
    // TODO: save settings to file
}/*}}}*/
void ManglerSettings::settingsOkButton_clicked_cb(void) {/*{{{*/
    fprintf(stderr, "settings window OK button clicked\n");
    // TODO: save settings to file
    settingsWindow->hide();
}/*}}}*/
void ManglerSettings::settingsEnablePTTKeyCheckButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("settingsEnablePTTKeyCheckButton", checkbutton);
    if (checkbutton->get_active()) {
        // box was checked
        builder->get_widget("settingsPTTKeyLabel", label);
        label->set_sensitive(true);
        builder->get_widget("settingsPTTKeyValueLabel", label);
        label->set_sensitive(true);
        builder->get_widget("settingsPTTKeyButton", button);
        button->set_sensitive(true);
    } else {
        // box was unchecked
        builder->get_widget("settingsPTTKeyLabel", label);
        label->set_sensitive(false);
        builder->get_widget("settingsPTTKeyValueLabel", label);
        label->set_sensitive(false);
        builder->get_widget("settingsPTTKeyButton", button);
        button->set_sensitive(false);
    }
}/*}}}*/
/*
 * When this button is pressed, we need to disable all of the other items on
 * the page until the user clicks the button again.  This starts a timer to
 * check keyboard state and update the settingsPTTKeyLabel to the key
 * combination value.  Setting isDetectingKey to false will cause the timer
 * callback to stop running.
 * The timer callback is ManglerSettings::settingsPTTKeyDetect.
 */
void ManglerSettings::settingsPTTKeyButton_clicked_cb(void) {/*{{{*/
    builder->get_widget("settingsPTTKeyButton", button);
    if (button->get_label() == "Set") {
        isDetectingKey = true;
        button->set_label("Done");
        builder->get_widget("settingsCancelButton", button);
        button->set_sensitive(false);
        builder->get_widget("settingsApplyButton", button);
        button->set_sensitive(false);
        builder->get_widget("settingsOkButton", button);
        button->set_sensitive(false);
        Glib::signal_timeout().connect( sigc::mem_fun(*this, &ManglerSettings::settingsPTTKeyDetect), 100 );
    } else {
        isDetectingKey = false;
        button->set_label("Set");
        builder->get_widget("settingsCancelButton", button);
        button->set_sensitive(true);
        builder->get_widget("settingsApplyButton", button);
        button->set_sensitive(true);
        builder->get_widget("settingsOkButton", button);
        button->set_sensitive(true);
        builder->get_widget("settingsPTTKeyValueLabel", label);
        // if the text is as follows, the user pressed done without any keys
        // pressed down.  Reset it to the default text
        if (label->get_text() == "Hold your key combination and click done") {
            label->set_markup("<span weight='light'>&lt;press the set button to define a hokey&gt;</span>");
        }
    }
}/*}}}*/
void ManglerSettings::settingsEnablePTTMouseCheckButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("settingsEnablePTTMouseCheckButton", checkbutton);
    if (checkbutton->get_active()) {
        // box was checked
        builder->get_widget("settingsPTTMouseLabel", label);
        label->set_sensitive(true);
        builder->get_widget("settingsPTTMouseValueLabel", label);
        label->set_sensitive(true);
        builder->get_widget("settingsPTTMouseButton", button);
        button->set_sensitive(true);
    } else {
        // box was unchecked
        builder->get_widget("settingsPTTMouseLabel", label);
        label->set_sensitive(false);
        builder->get_widget("settingsPTTMouseValueLabel", label);
        label->set_sensitive(false);
        builder->get_widget("settingsPTTMouseButton", button);
        button->set_sensitive(false);
    }
}/*}}}*/
void ManglerSettings::settingsPTTMouseButton_clicked_cb(void) {/*{{{*/
    builder->get_widget("settingsPTTMouseButton", button);
    isDetectingMouse = true;
    button->set_label("Done");
    builder->get_widget("settingsCancelButton", button);
    button->set_sensitive(false);
    builder->get_widget("settingsApplyButton", button);
    button->set_sensitive(false);
    builder->get_widget("settingsOkButton", button);
    button->set_sensitive(false);
    // at this point, we need to grab the mouse and wait for a mouse button event
}/*}}}*/

bool
ManglerSettings::settingsPTTKeyDetect(void) {/*{{{*/
    char        pressed_keys[32];
    Glib::ustring ptt_keylist;
    GdkWindow   *rootwin = gdk_get_default_root_window();

    // TODO: window close event needs to set isDetectingKey
    if (!isDetectingKey) {
        return false;
    }
    /*
     * Query the X keymap and get a list of all the keys are pressed.  Convert
     * keycodes to keysyms to keynames and build a human readable string that
     * will be the actual value stored in the settings file
     */
    XQueryKeymap(GDK_WINDOW_XDISPLAY(rootwin), pressed_keys);
    for (int ctr = 0; ctr < 256; ctr++) {
        if ((pressed_keys[ctr >> 3] >> (ctr & 0x07)) & 0x01) {
            std::string keyname = XKeysymToString(XKeycodeToKeysym(GDK_WINDOW_XDISPLAY(rootwin), ctr, 0));
            if (keyname.length() > 1) {
                ptt_keylist.insert(0, "<" + keyname + ">" + (ptt_keylist.empty() ? "" : "+"));
            } else {
                keyname[0] = toupper(keyname[0]);
                ptt_keylist.append((ptt_keylist.empty() ? "" : "+") + keyname);
            }
        }
    }
    builder->get_widget("settingsPTTKeyValueLabel", label);
    if (ptt_keylist.empty()) {
        label->set_markup("<span color='red'>Hold your key combination and click done</span>");
    } else {
        label->set_text(ptt_keylist);
    }
    return(true);
}/*}}}*/

