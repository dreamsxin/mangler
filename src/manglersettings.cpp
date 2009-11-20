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

ManglerSettings::ManglerSettings(Glib::RefPtr<Gtk::Builder> builder) {/*{{{*/

    this->builder = builder;

    // Connect our signals for this window
    builder->get_widget("settingsWindow", settingsWindow);
    /*
     * Set window properties that are not settable in builder
     */
    settingsWindow->set_keep_above(true);

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

    builder->get_widget("settingsEnableAudioIntegrationCheckButton", checkbutton);
    checkbutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerSettings::settingsEnableAudioIntegrationCheckButton_toggled_cb));
    builder->get_widget("settingsAudioIntegrationComboBox", audioPlayerComboBox);
    audioPlayerTreeModel = Gtk::ListStore::create(audioPlayerColumns);
    audioPlayerComboBox->set_model(audioPlayerTreeModel);
    // create a "none" row
    Gtk::TreeModel::Row audioPlayerNoneRow = *(audioPlayerTreeModel->append());
    audioPlayerNoneRow[audioPlayerColumns.id] = -1;
    audioPlayerNoneRow[audioPlayerColumns.name] = "None";
    audioPlayerComboBox->pack_start(audioPlayerColumns.name);
    audioPlayerComboBox->set_active(audioPlayerNoneRow);

    builder->get_widget("inputDeviceComboBox", inputDeviceComboBox);
    inputDeviceTreeModel = Gtk::ListStore::create(inputColumns);
    inputDeviceComboBox->set_model(inputDeviceTreeModel);
    inputDeviceComboBox->pack_start(inputColumns.description);

    builder->get_widget("outputDeviceComboBox", outputDeviceComboBox);
    outputDeviceTreeModel = Gtk::ListStore::create(outputColumns);
    outputDeviceComboBox->set_model(outputDeviceTreeModel);
    outputDeviceComboBox->pack_start(outputColumns.description);

    builder->get_widget("notificationDeviceComboBox", notificationDeviceComboBox);
    notificationDeviceTreeModel = Gtk::ListStore::create(notificationColumns);
    notificationDeviceComboBox->set_model(notificationDeviceTreeModel);
    notificationDeviceComboBox->pack_start(notificationColumns.description);

    mouseInputDevices = getInputDeviceList();
    builder->get_widget("settingsMouseDeviceComboBox", mouseDeviceComboBox);
    mouseDeviceTreeModel = Gtk::ListStore::create(mouseColumns);
    mouseDeviceComboBox->set_model(mouseDeviceTreeModel);
    mouseDeviceComboBox->pack_start(mouseColumns.name);
}/*}}}*/
void ManglerSettings::applySettings(void) {/*{{{*/
    Gtk::TreeModel::iterator iter;
    GdkWindow   *rootwin = gdk_get_default_root_window();


    // Push to Talk
    builder->get_widget("settingsEnablePTTKeyCheckButton", checkbutton);
    config.PushToTalkKeyEnabled = checkbutton->get_active() ? true : false;
    builder->get_widget("settingsPTTKeyValueLabel", label);
    config.PushToTalkKeyValue = label->get_text();
    config.parsePushToTalkValue(config.PushToTalkKeyValue);

    // Mouse to Talk
    builder->get_widget("settingsMouseDeviceComboBox", combobox);
    iter = combobox->get_active();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        config.mouseDeviceName = row[mouseColumns.name];
    }
    builder->get_widget("settingsEnablePTTMouseCheckButton", checkbutton);
    config.PushToTalkMouseEnabled = checkbutton->get_active() ? true : false;
    builder->get_widget("settingsPTTMouseValueLabel", label);
    config.PushToTalkMouseValue = label->get_text();
    if (config.PushToTalkMouseValue.length() > 6) {
        config.PushToTalkMouseValueInt = atoi(config.PushToTalkMouseValue.substr(6).c_str());
    }
    XUngrabButton(GDK_WINDOW_XDISPLAY(rootwin), AnyButton, AnyModifier, GDK_ROOT_WINDOW());
    XAllowEvents (GDK_WINDOW_XDISPLAY(rootwin), AsyncBoth, CurrentTime);
    /*
    if (checkbutton->get_active()) {
        XGrabButton(GDK_WINDOW_XDISPLAY(rootwin), config.PushToTalkMouseValueInt, AnyModifier, GDK_ROOT_WINDOW(), False, ButtonPressMask|ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None);
    }
    */

    // Audio Player Integration
    builder->get_widget("settingsEnableAudioIntegrationCheckButton", checkbutton);
    config.AudioIntegrationEnabled = checkbutton->get_active() ? true : false;
    iter = audioPlayerComboBox->get_active();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        config.AudioIntegrationPlayer = row[audioPlayerColumns.name];
    }

    // Audio Devices
    iter = inputDeviceComboBox->get_active();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        config.inputDeviceName = row[inputColumns.name];
    }
    iter = outputDeviceComboBox->get_active();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        config.outputDeviceName = row[outputColumns.name];
    }
    iter = notificationDeviceComboBox->get_active();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        config.notificationDeviceName = row[notificationColumns.name];
    }

    // Notification sounds
    builder->get_widget("notificationLoginLogoutCheckButton", checkbutton);
    config.notificationLoginLogout = checkbutton->get_active() ? true : false;

    builder->get_widget("notificationChannelEnterLeaveCheckButton", checkbutton);
    config.notificationChannelEnterLeave = checkbutton->get_active() ? true : false;

    builder->get_widget("notificationTalkStartEndCheckButton", checkbutton);
    config.notificationTransmitStartStop = checkbutton->get_active() ? true : false;

    // Debug level
    uint32_t debuglevel = 0;
    builder->get_widget("debugStatus", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_STATUS : 0;

    builder->get_widget("debugError", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_ERROR : 0;

    builder->get_widget("debugStack", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_STACK : 0;

    builder->get_widget("debugInternalNet", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_INTERNAL : 0;

    builder->get_widget("debugPacketDump", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_PACKET : 0;

    builder->get_widget("debugPacketParse", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_PACKET_PARSE : 0;

    builder->get_widget("debugEventQueue", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_EVENT : 0;

    builder->get_widget("debugSocket", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_SOCKET : 0;

    builder->get_widget("debugNotice", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_NOTICE : 0;

    builder->get_widget("debugInfo", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_INFO : 0;

    builder->get_widget("debugMutex", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_MUTEX : 0;

    builder->get_widget("debugMemory", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_MEMORY : 0;

    builder->get_widget("debugEncryptedPacket", checkbutton);
    debuglevel |= checkbutton->get_active() ? V3_DEBUG_PACKET_ENCRYPTED : 0;

    config.lv3_debuglevel = debuglevel;

    v3_debuglevel(debuglevel);
    config.save();
}/*}}}*/
void ManglerSettings::initSettings(void) {/*{{{*/
    config.load();

    // Push to Talk
    builder->get_widget("settingsEnablePTTKeyCheckButton", checkbutton);
    checkbutton->set_active(config.PushToTalkKeyEnabled ? true : false);
    builder->get_widget("settingsPTTKeyValueLabel", label);
    label->set_text(config.PushToTalkKeyValue);

    // Mouse to Talk
    builder->get_widget("settingsEnablePTTMouseCheckButton", checkbutton);
    checkbutton->set_active(config.PushToTalkMouseEnabled ? true : false);
    builder->get_widget("settingsPTTMouseValueLabel", label);
    label->set_text(config.PushToTalkMouseValue);

    // Audio Player Integration
    builder->get_widget("settingsEnableAudioIntegrationCheckButton", checkbutton);
    checkbutton->set_active(config.AudioIntegrationEnabled ? true : false);
    /*
       iterate through whatever is available based on what we can find and populate the store
       audioPlayerComboBox->set_active(iterOfSelectedinStore);
    */

    // Audio Devices
    // the proper devices are selected in the window->show() callback

    // Notification sounds
    builder->get_widget("notificationLoginLogoutCheckButton", checkbutton);
    checkbutton->set_active(config.notificationLoginLogout ? true : false);

    builder->get_widget("notificationChannelEnterLeaveCheckButton", checkbutton);
    checkbutton->set_active(config.notificationChannelEnterLeave ? true : false);

    builder->get_widget("notificationTalkStartEndCheckButton", checkbutton);
    checkbutton->set_active(config.notificationTransmitStartStop ? true : false);

    // Debug level
    builder->get_widget("debugStatus", checkbutton);
    checkbutton->set_active(config.lv3_debuglevel & V3_DEBUG_STATUS ? 1 : 0);

    builder->get_widget("debugError", checkbutton);
    checkbutton->set_active(config.lv3_debuglevel & V3_DEBUG_ERROR ? 1 : 0);

    builder->get_widget("debugStack", checkbutton);
    checkbutton->set_active(config.lv3_debuglevel & V3_DEBUG_STACK ? 1 : 0);

    builder->get_widget("debugInternalNet", checkbutton);
    checkbutton->set_active(config.lv3_debuglevel & V3_DEBUG_INTERNAL ? 1 : 0);

    builder->get_widget("debugPacketDump", checkbutton);
    checkbutton->set_active(config.lv3_debuglevel & V3_DEBUG_PACKET ? 1 : 0);

    builder->get_widget("debugPacketParse", checkbutton);
    checkbutton->set_active(config.lv3_debuglevel & V3_DEBUG_PACKET_PARSE ? 1 : 0);

    builder->get_widget("debugEventQueue", checkbutton);
    checkbutton->set_active(config.lv3_debuglevel & V3_DEBUG_EVENT ? 1 : 0);

    builder->get_widget("debugSocket", checkbutton);
    checkbutton->set_active(config.lv3_debuglevel & V3_DEBUG_SOCKET ? 1 : 0);

    builder->get_widget("debugNotice", checkbutton);
    checkbutton->set_active(config.lv3_debuglevel & (uint32_t)V3_DEBUG_NOTICE ? 1 : 0);

    builder->get_widget("debugInfo", checkbutton);
    checkbutton->set_active(config.lv3_debuglevel & V3_DEBUG_INFO ? 1 : 0);

    builder->get_widget("debugMutex", checkbutton);
    checkbutton->set_active(config.lv3_debuglevel & V3_DEBUG_MUTEX ? 1 : 0);

    builder->get_widget("debugMemory", checkbutton);
    checkbutton->set_active(config.lv3_debuglevel & V3_DEBUG_MEMORY ? 1 : 0);

    builder->get_widget("debugEncryptedPacket", checkbutton);
    checkbutton->set_active(config.lv3_debuglevel & V3_DEBUG_PACKET_ENCRYPTED ? 1 : 0);

}/*}}}*/

// Settings Window Callbacks
void ManglerSettings::showSettingsWindow(void) {/*{{{*/
    settingsWindow->show();
}/*}}}*/

void ManglerSettings::settingsWindow_show_cb(void) {/*{{{*/
    Gtk::TreeModel::Row row;
    isDetectingKey = false;
    isDetectingMouse = false;

    initSettings();
    // these callbacks initialize the state
    settingsEnablePTTKeyCheckButton_toggled_cb();
    settingsEnablePTTMouseCheckButton_toggled_cb();

    // Clear the input device store and rebuild it from the audioControl vector
    inputDeviceTreeModel->clear();
    row = *(inputDeviceTreeModel->append());
    row[inputColumns.id] = -1;
    row[inputColumns.name] = "Default";
    row[inputColumns.description] = "Default";
    int inputSelection = 0;
    int inputCtr = 1;
    for (
            std::vector<ManglerAudioDevice*>::iterator i = mangler->audioControl->inputDevices.begin();
            i <  mangler->audioControl->inputDevices.end();
            i++, inputCtr++) {
        Gtk::TreeModel::Row row = *(inputDeviceTreeModel->append());
        row[inputColumns.id] = (*i)->id;
        row[inputColumns.name] = (*i)->name;
        row[inputColumns.description] = (*i)->description;
        if (config.inputDeviceName == (*i)->name) {
            inputSelection = inputCtr;
        }
    }
    // TODO: get the currently selected item from settings object and select it
    inputDeviceComboBox->set_active(inputSelection);

    // Clear the output device store and rebuild it from the audioControl vector
    outputDeviceTreeModel->clear();
    row = *(outputDeviceTreeModel->append());
    row[outputColumns.id] = -1;
    row[outputColumns.name] = "Default";
    row[outputColumns.description] = "Default";
    int outputSelection = 0;
    int outputCtr = 1;
    for (
            std::vector<ManglerAudioDevice*>::iterator i = mangler->audioControl->outputDevices.begin();
            i <  mangler->audioControl->outputDevices.end();
            i++, outputCtr++) {
        Gtk::TreeModel::Row row = *(outputDeviceTreeModel->append());
        row[outputColumns.id] = (*i)->id;
        row[outputColumns.name] = (*i)->name;
        row[outputColumns.description] = (*i)->description;
        if (config.outputDeviceName == (*i)->name) {
            outputSelection = outputCtr;
        }
    }
    // TODO: get the currently selected item from settings object and select it
    outputDeviceComboBox->set_active(outputSelection);

    // Clear the notification device store and rebuild it from the audioControl vector
    notificationDeviceTreeModel->clear();
    row = *(notificationDeviceTreeModel->append());
    row[notificationColumns.id] = -1;
    row[notificationColumns.name] = "Default";
    row[notificationColumns.description] = "Default";
    int notificationSelection = 0;
    int notificationCtr = 1;
    for (
            std::vector<ManglerAudioDevice*>::iterator i = mangler->audioControl->outputDevices.begin();
            i <  mangler->audioControl->outputDevices.end();
            i++, notificationCtr++) {
        Gtk::TreeModel::Row row = *(notificationDeviceTreeModel->append());
        row[notificationColumns.id] = (*i)->id;
        row[notificationColumns.name] = (*i)->name;
        row[notificationColumns.description] = (*i)->description;
        if (config.notificationDeviceName == (*i)->name) {
            notificationSelection = notificationCtr;
        }
    }
    // TODO: get the currently selected item from settings object and select it
    notificationDeviceComboBox->set_active(notificationSelection);

    // Clear the mouse device store and rebuild it from the audioControl vector
    mouseDeviceTreeModel->clear();
    int mouseSelection = 0;
    int mouseCtr = 0;
    for (
            std::map<uint32_t, Glib::ustring>::iterator i = mouseInputDevices.begin();
            i != mouseInputDevices.end();
            i++, mouseCtr++) {
        Gtk::TreeModel::Row row = *(mouseDeviceTreeModel->append());
        row[mouseColumns.id] = (*i).first;
        row[mouseColumns.name] = (*i).second;
        if (config.mouseDeviceName == (*i).second) {
            mouseSelection = mouseCtr;
        }
    }
    if (!mouseSelection) {
        int mouseCtr = 0;
        for (
                std::map<uint32_t, Glib::ustring>::iterator i = mouseInputDevices.begin();
                i != mouseInputDevices.end();
                i++, mouseCtr++) {
            if ((*i).second.find("Mouse") != std::string::npos) {
                mouseSelection = mouseCtr;
            }
        }
    }
    // TODO: get the currently selected item from settings object and select it
    mouseDeviceComboBox->set_active(mouseSelection);
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
    applySettings();
}/*}}}*/
void ManglerSettings::settingsOkButton_clicked_cb(void) {/*{{{*/
    applySettings();
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
void ManglerSettings::settingsEnableAudioIntegrationCheckButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("settingsEnableAudioIntegrationCheckButton", checkbutton);
    if (checkbutton->get_active()) {
        audioPlayerComboBox->set_sensitive(true);
    } else {
        audioPlayerComboBox->set_sensitive(false);
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
        builder->get_widget("settingsMouseDeviceComboBox", combobox);
        combobox->set_sensitive(false);
        builder->get_widget("settingsPTTMouseButton", button);
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
        builder->get_widget("settingsMouseDeviceComboBox", combobox);
        combobox->set_sensitive(true);
        builder->get_widget("settingsPTTMouseButton", button);
        button->set_sensitive(true);
        builder->get_widget("settingsPTTKeyValueLabel", label);
        // if the text is as follows, the user pressed done without any keys
        // pressed down.  Reset it to the default text
        if (label->get_text() == "Hold your key combination and click done") {
            label->set_markup("<span weight='light'>&lt;press the set button to define a hotkey&gt;</span>");
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
        builder->get_widget("settingsMouseDeviceComboBox", combobox);
        combobox->set_sensitive(true);
    } else {
        // box was unchecked
        builder->get_widget("settingsPTTMouseLabel", label);
        label->set_sensitive(false);
        builder->get_widget("settingsPTTMouseValueLabel", label);
        label->set_sensitive(false);
        builder->get_widget("settingsPTTMouseButton", button);
        button->set_sensitive(false);
        builder->get_widget("settingsMouseDeviceComboBox", combobox);
        combobox->set_sensitive(false);
    }
}/*}}}*/
void ManglerSettings::settingsPTTMouseButton_clicked_cb(void) {/*{{{*/
    isDetectingMouse = true;
    builder->get_widget("settingsPTTMouseValueLabel", label);
    label->set_markup("<span color='red'>&lt;Click the mouse button you wish to use&gt;</span>");
    builder->get_widget("settingsPTTMouseButton", button);
    button->set_sensitive(false);
    builder->get_widget("settingsCancelButton", button);
    button->set_sensitive(false);
    builder->get_widget("settingsApplyButton", button);
    button->set_sensitive(false);
    builder->get_widget("settingsOkButton", button);
    button->set_sensitive(false);
    builder->get_widget("settingsPTTKeyButton", button);
    button->set_sensitive(false);
    builder->get_widget("settingsMouseDeviceComboBox", combobox);
    combobox->set_sensitive(false);
    Glib::signal_timeout().connect( sigc::mem_fun(*this, &ManglerSettings::settingsPTTMouseDetect), 100 );
}/*}}}*/
std::map<uint32_t, Glib::ustring> ManglerSettings::getInputDeviceList(void) {/*{{{*/
    GdkWindow   *rootwin = gdk_get_default_root_window();
    XDevice *dev;
    XDeviceInfo *xdev;
    XDeviceState *xds;
    XButtonState *xbs;
    int ndevices_return;
    std::map<uint32_t, Glib::ustring> devicelist;
    bool        ptt_on = false;

    xdev = XListInputDevices(GDK_WINDOW_XDISPLAY(rootwin), &ndevices_return);
    for (int ctr = 0; ctr < ndevices_return; ctr++) {
        devicelist[xdev[ctr].id] = xdev[ctr].name;
    }
    XFreeDeviceList(xdev);
    return(devicelist);
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

bool
ManglerSettings::settingsPTTMouseDetect(void) {/*{{{*/
    GdkWindow   *rootwin = gdk_get_default_root_window();
    char buttonname[32];
    static bool grabbed = false;
    int flag = 0;
    int x, y;
    int mousebutton;
    XEvent ev;


    // TODO: window close event needs to set isDetectingKey
    if (!isDetectingMouse) {
        return false;
    }
    if (! grabbed) {
        XUngrabPointer(GDK_WINDOW_XDISPLAY(rootwin), CurrentTime);
        XGrabPointer(GDK_WINDOW_XDISPLAY(rootwin), GDK_ROOT_WINDOW(), False, ButtonPress, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
        grabbed = true;
    }
    while (flag == 0) {
        while (!XPending(GDK_WINDOW_XDISPLAY(rootwin))) {
            usleep(100000);
        }
        XNextEvent(GDK_WINDOW_XDISPLAY(rootwin), &ev);
        switch (ev.type) {
            case ButtonPress:
                mousebutton = ev.xbutton.button;
                snprintf(buttonname, 31, "Button%d", ev.xbutton.button);
                config.PushToTalkMouseValue = buttonname;
                flag = 1;
                XUngrabPointer(GDK_WINDOW_XDISPLAY(rootwin), CurrentTime);
                grabbed = false;
                isDetectingMouse = false;
                x = ev.xbutton.x_root;
                y = ev.xbutton.y_root;
                break;
            case MotionNotify:
                break;
            default:
                break;
        }
        XAllowEvents (GDK_WINDOW_XDISPLAY(rootwin), AsyncBoth, CurrentTime);
    }
    isDetectingMouse = false;
    builder->get_widget("settingsPTTMouseValueLabel", label);
    label->set_markup(config.PushToTalkMouseValue);

    builder->get_widget("settingsCancelButton", button);
    button->set_sensitive(true);

    builder->get_widget("settingsApplyButton", button);
    button->set_sensitive(true);

    builder->get_widget("settingsOkButton", button);
    button->set_sensitive(true);

    builder->get_widget("settingsOkButton", button);
    button->set_sensitive(true);

    builder->get_widget("settingsPTTMouseButton", button);
    button->set_sensitive(true);

    builder->get_widget("settingsPTTKeyButton", button);
    button->set_sensitive(true);

    builder->get_widget("settingsMouseDeviceComboBox", combobox);
    combobox->set_sensitive(true);

    return(true);
}/*}}}*/


