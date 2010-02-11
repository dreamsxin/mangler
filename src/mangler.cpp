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

#include <gtkmm.h>
#include <iostream>
#include <stdio.h>
#include "mangler.h"
#include "manglerui.h"
#include "mangler-icons.h"
#include <gdk/gdkx.h>
#include <X11/extensions/XInput.h>

using namespace std;

Mangler *mangler;

Mangler::Mangler(struct _cli_options *options) {/*{{{*/
    // load all of our icons
    icons.insert(std::make_pair("black_circle",                 Gdk::Pixbuf::create_from_inline(-1, black_circle                )));
    icons.insert(std::make_pair("blue_circle",                  Gdk::Pixbuf::create_from_inline(-1, blue_circle                 )));
    icons.insert(std::make_pair("cyan_circle",                  Gdk::Pixbuf::create_from_inline(-1, cyan_circle                 )));
    icons.insert(std::make_pair("green_circle",                 Gdk::Pixbuf::create_from_inline(-1, green_circle                )));
    icons.insert(std::make_pair("purple_circle",                Gdk::Pixbuf::create_from_inline(-1, purple_circle               )));
    icons.insert(std::make_pair("red_circle",                   Gdk::Pixbuf::create_from_inline(-1, red_circle                  )));
    icons.insert(std::make_pair("yellow_circle",                Gdk::Pixbuf::create_from_inline(-1, yellow_circle               )));
    icons.insert(std::make_pair("mangler_logo",                 Gdk::Pixbuf::create_from_inline(-1, mangler_logo                )));

    icons.insert(std::make_pair("tray_icon",                    Gdk::Pixbuf::create_from_inline(-1, tray_icon_purple            )));
    icons.insert(std::make_pair("tray_icon_blue",               Gdk::Pixbuf::create_from_inline(-1, tray_icon_blue              )));
    icons.insert(std::make_pair("tray_icon_red",                Gdk::Pixbuf::create_from_inline(-1, tray_icon_red               )));
    icons.insert(std::make_pair("tray_icon_green",              Gdk::Pixbuf::create_from_inline(-1, tray_icon_green             )));
    icons.insert(std::make_pair("tray_icon_yellow",             Gdk::Pixbuf::create_from_inline(-1, tray_icon_yellow            )));
    icons.insert(std::make_pair("tray_icon_grey",               Gdk::Pixbuf::create_from_inline(-1, tray_icon_grey              )));
    icons.insert(std::make_pair("tray_icon_purple",             Gdk::Pixbuf::create_from_inline(-1, tray_icon_purple            )));

    icons.insert(std::make_pair("user_icon_red",                Gdk::Pixbuf::create_from_inline(-1, user_icon_red               )));
    icons.insert(std::make_pair("user_icon_yellow",             Gdk::Pixbuf::create_from_inline(-1, user_icon_yellow            )));
    icons.insert(std::make_pair("user_icon_green",              Gdk::Pixbuf::create_from_inline(-1, user_icon_green             )));
    icons.insert(std::make_pair("user_icon_orange",             Gdk::Pixbuf::create_from_inline(-1, user_icon_orange            )));


    try {
        if (options->uifromfile) {
            builder = Gtk::Builder::create_from_file(options->uifilename);
        } else {
            builder = Gtk::Builder::create_from_string(ManglerUI);
        }
        builder->get_widget("manglerWindow", manglerWindow);
        manglerWindow->set_icon(icons["tray_icon"]);
    } catch(const Glib::Error& e) {
        std::cerr << e.what() << std::endl;
        exit(0);
    }
    //manglerWindow->signal_hide().connect(sigc::mem_fun(this, &Mangler::manglerWindow_hide_cb));
    Gtk::Main::signal_quit().connect(sigc::mem_fun(this, &Mangler::mangler_quit_cb));

    /*
     * Retreive all buttons from builder and set their singal handler callbacks
     */
    // Quick Connect Button
    builder->get_widget("quickConnectButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::quickConnectButton_clicked_cb));

    // Server Button
    builder->get_widget("serverConfigButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::serverConfigButton_clicked_cb));

    // Connect Button
    builder->get_widget("connectButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::connectButton_clicked_cb));

    // Comment Button
    builder->get_widget("commentButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::commentButton_clicked_cb));

    // Chat Button
    builder->get_widget("chatButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::chatButton_clicked_cb));

    // Bindings Button
    builder->get_widget("bindingsButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::bindingsButton_clicked_cb));

    // Admin Button
    builder->get_widget("adminButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::adminButton_clicked_cb));
    isAdmin = false;
    isChanAdmin = false;

    // Settings Button
    builder->get_widget("settingsButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::settingsButton_clicked_cb));

    // About Button
    builder->get_widget("aboutButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::aboutButton_clicked_cb));

    // Settings Button
    builder->get_widget("xmitButton", togglebutton);
    togglebutton->signal_toggled().connect(sigc::mem_fun(this, &Mangler::xmitButton_toggled_cb));

    // Quick Connect Dialog Buttons
    builder->get_widget("qcConnectButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::qcConnectButton_clicked_cb));

    builder->get_widget("qcCancelButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::qcCancelButton_clicked_cb));

    // MOTD Window Buttons
    builder->get_widget("motdOkButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::motdOkButton_clicked_cb));

    // Input VU Meter
    builder->get_widget("inputVUMeterProgressBar", inputvumeter);

    // Quick mute options
    muteMic   = false;
    muteSound = false;
    builder->get_widget("muteMicCheckButton", checkbutton);
    checkbutton->signal_toggled().connect(sigc::mem_fun(this, &Mangler::muteMicCheckButton_toggled_cb));
    builder->get_widget("muteSoundCheckButton", checkbutton);
    checkbutton->signal_toggled().connect(sigc::mem_fun(this, &Mangler::muteSoundCheckButton_toggled_cb));

    // Autoreconnect feature implementation
    wantDisconnect = false;

    // Feature implementation
    motdAlways = false;

    /*
     * Retreive all menu bar items from builder and set their singal handler
     * callbacks.  Most of these can use the same callback as their
     * corresponding button
     */
    builder->get_widget("buttonMenuItem", checkmenuitem);
    checkmenuitem->signal_toggled().connect(sigc::mem_fun(this, &Mangler::buttonMenuItem_toggled_cb));

    builder->get_widget("hideServerInfoMenuItem", checkmenuitem);
    checkmenuitem->signal_toggled().connect(sigc::mem_fun(this, &Mangler::hideServerInfoMenuItem_toggled_cb));

    builder->get_widget("hideGuestFlagMenuItem", checkmenuitem);
    checkmenuitem->signal_toggled().connect(sigc::mem_fun(this, &Mangler::hideGuestFlagMenuItem_toggled_cb));

    builder->get_widget("quickConnectMenuItem", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &Mangler::quickConnectButton_clicked_cb));

    builder->get_widget("serverListMenuItem", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &Mangler::serverConfigButton_clicked_cb));

    builder->get_widget("adminSeparatorMenuItem", menuitem);

    builder->get_widget("adminLoginMenuItem", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &Mangler::adminButton_clicked_cb));

    builder->get_widget("adminWindowMenuItem", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &Mangler::adminWindowMenuItem_activated_cb));

    builder->get_widget("settingsMenuItem", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &Mangler::settingsButton_clicked_cb));

    builder->get_widget("chatMenuItem", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &Mangler::chatButton_clicked_cb));

    builder->get_widget("commentMenuItem", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &Mangler::commentButton_clicked_cb));

    builder->get_widget("quitMenuItem", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &Mangler::quitMenuItem_activate_cb));

    builder->get_widget("aboutMenuItem", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &Mangler::aboutButton_clicked_cb));

    // connect the signal for our error dialog button
    builder->get_widget("errorOKButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::errorOKButton_clicked_cb));

    // Set up our generic password dialog box
    builder->get_widget("passwordDialog", passwordDialog);
    builder->get_widget("passwordEntry", passwordEntry);
    builder->get_widget("passwordOkButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::passwordDialogOkButton_clicked_cb));
    builder->get_widget("passwordCancelButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::passwordDialogCancelButton_clicked_cb));

    // Set up our kick/ban reason entry dialog box
    builder->get_widget("reasonDialog", reasonDialog);
    builder->get_widget("reasonEntry", reasonEntry);
    builder->get_widget("reasonOkButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::reasonDialogOkButton_clicked_cb));
    builder->get_widget("reasonCancelButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::reasonDialogCancelButton_clicked_cb));

    // Set up the text string change dialog box
    builder->get_widget("textStringChangeDialog", textStringChangeDialog);
    builder->get_widget("textStringChangeCommentEntry", textStringChangeCommentEntry);
    builder->get_widget("textStringChangeURLEntry", textStringChangeURLEntry);
    builder->get_widget("textStringChangeIntegrationEntry", textStringChangeIntegrationEntry);
    builder->get_widget("textStringOkButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::textStringChangeDialogOkButton_clicked_cb));
    builder->get_widget("textStringCancelButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::textStringChangeDialogCancelButton_clicked_cb));

    // Create Channel Tree
    channelTree = new ManglerChannelTree(builder);

    // Create Network Communication Object
    network = new ManglerNetwork(builder);

    // Create settings object, load the configuration file, and apply.  If the
    // user has PTT key/mouse enabled, start a timer here
    settings = new ManglerSettings(builder);
    isTransmittingButton = 0;
    isTransmittingMouse = 0;
    isTransmittingKey = 0;
    isTransmitting = 0;
    Glib::signal_timeout().connect(sigc::mem_fun(this, &Mangler::checkPushToTalkKeys), 100);
    Glib::signal_timeout().connect(sigc::mem_fun(this, &Mangler::checkPushToTalkMouse), 100);

    // Create our audio control object for managing devices
    audioControl = new ManglerAudio("control");
    audioControl->getDeviceList(settings->config.audioSubsystem);

    // set the default window size from the settings
    if (settings->config.windowWidth > 0 && settings->config.windowHeight > 0) {
        manglerWindow->set_default_size(settings->config.windowWidth, settings->config.windowHeight);
    }

    // set the master volume
    v3_set_volume_master(settings->config.masterVolumeLevel);

    builder->get_widget("buttonMenuItem", checkmenuitem);
    checkmenuitem->set_active(settings->config.buttonsHidden);

    builder->get_widget("hideServerInfoMenuItem", checkmenuitem);
    checkmenuitem->set_active(settings->config.serverInfoHidden);

    builder->get_widget("hideGuestFlagMenuItem", checkmenuitem);
    checkmenuitem->set_active(settings->config.guestFlagHidden);

    // Create Server List Window
    serverList = new ManglerServerList(builder);

    // Create Chat Window
    chat = new ManglerChat(builder);

    // Add our servers to the main window drop down
    builder->get_widget("serverSelectComboBox", combobox);
    combobox->set_model(serverList->serverListTreeModel);
    combobox->pack_start(serverList->serverListColumns.name);
    int serverSelection = 0;
    for (int32_t ctr = 0; ctr < (int32_t)settings->config.serverlist.size(); ctr++) {
        ManglerServerConfig *server = settings->config.serverlist[ctr];
        Gtk::TreeRow row = *(serverList->serverListTreeModel->append());
        row[serverList->serverListColumns.id] = ctr;
        row[serverList->serverListColumns.name] = server->name;
        row[serverList->serverListColumns.hostname] = server->hostname;
        row[serverList->serverListColumns.port] = server->port;
        row[serverList->serverListColumns.username] = server->username;
        if (ctr == settings->config.lastConnectedServerId) {
            serverSelection = ctr;
        }
    }
    //  Select the last one used (or the first if unknown)
    combobox->set_active(serverSelection);

    // Statusbar Icon
    statusIcon = Gtk::StatusIcon::create(icons["tray_icon_grey"]);
    statusIcon->signal_activate().connect(sigc::mem_fun(this, &Mangler::statusIcon_activate_cb));
    iconified = false;

    // Music (Now playing)
    integration = new ManglerIntegration();
    integration->setClient((MusicClient)settings->config.AudioIntegrationPlayer);
    integration->update(true);
    Glib::signal_timeout().connect(sigc::mem_fun(this, &Mangler::updateIntegration), 1000);


    Glib::signal_timeout().connect(sigc::mem_fun(*this, &Mangler::updateXferAmounts), 500);
    Glib::signal_timeout().connect(sigc::mem_fun(*this, &Mangler::getNetworkEvent), 10);
    admin = new ManglerAdmin(builder);
}/*}}}*/

/*
 * Button signal handler callbacks
 */
void Mangler::quickConnectButton_clicked_cb(void) {/*{{{*/
    Gtk::Dialog *dialog;
    Gtk::Entry *textbox;

    builder->get_widget("quickConnectDialog", dialog);
    dialog->set_icon(icons["tray_icon"]);

    builder->get_widget("qcServerName", textbox);
    textbox->set_text(settings->config.qc_lastserver.hostname);

    builder->get_widget("qcPort", textbox);
    textbox->set_text(settings->config.qc_lastserver.port);

    builder->get_widget("qcUsername", textbox);
    textbox->set_text(settings->config.qc_lastserver.username);

    builder->get_widget("qcPassword", textbox);
    textbox->set_text(settings->config.qc_lastserver.password);

    if (v3_is_loggedin()) {
        builder->get_widget("qcConnectButton", button);
        button->set_sensitive(false);
    } else {
        builder->get_widget("qcConnectButton", button);
        button->set_sensitive(true);
    }
    dialog->set_keep_above(true);
    dialog->run();
    dialog->hide();

}/*}}}*/
void Mangler::serverConfigButton_clicked_cb(void) {/*{{{*/
    builder->get_widget("serverListWindow", window);
    window->set_icon(icons["tray_icon"]);
    window->show();
}/*}}}*/
void Mangler::connectButton_clicked_cb(void) {/*{{{*/
    Gtk::Button *connectbutton;
    Gtk::TreeModel::iterator iter;

    builder->get_widget("connectButton", connectbutton);
    if (connectbutton->get_label() == "Cancel Reconnect") {
        wantDisconnect = true;
        onDisconnectHandler();
        builder->get_widget("statusbar", statusbar);
        statusbar->pop();
        statusbar->push("Disconnected");
    } else if (connectbutton->get_label() == "gtk-connect") {
        channelTree->updateLobby("Connecting...");
        wantDisconnect = false;
        connectbutton->set_sensitive(false);
        builder->get_widget("serverSelectComboBox", combobox);
        iter = combobox->get_active();
        if (iter) {
            Gtk::TreeModel::Row row = *iter;
            ManglerServerConfig *server;

            uint32_t server_id = row[serverList->serverListColumns.id];
            connectedServerId = server_id;
            server = settings->config.getserver(server_id);
            Glib::ustring hostname = server->hostname;
            Glib::ustring port     = server->port;
            Glib::ustring username = server->username;
            Glib::ustring password = server->password;
            Glib::ustring phonetic = server->phonetic;
            if (!server || server->hostname.empty() || server->port.empty() || server->username.empty()) {
                channelTree->updateLobby("Not connected");
                if (server->hostname.empty()) {
                    errorDialog("You have not specified a hostname for this server.");
                    return;
                }
                if (server->port.empty()) {
                    errorDialog("You have not specified a port for this server.");
                    return;
                }
                if (server->username.empty()) {
                    errorDialog("You have not specified a username for this server.");
                    return;
                }
                return;
            }
            set_charset(server->charset);

            settings->config.lastConnectedServerId = server_id;
            settings->config.save();
            v3_set_server_opts(V3_USER_ACCEPT_PAGES, server->acceptPages);
            v3_set_server_opts(V3_USER_ACCEPT_U2U, server->acceptU2U);
            v3_set_server_opts(V3_USER_ACCEPT_CHAT, server->acceptPrivateChat);
            v3_set_server_opts(V3_USER_ALLOW_RECORD, server->allowRecording);
            isAdmin = false;
            isChanAdmin = false;
            Glib::Thread::create(sigc::bind(sigc::mem_fun(this->network, &ManglerNetwork::connect), hostname, port, username, password, phonetic), FALSE);
        }
    } else {
        wantDisconnect = true;
        v3_logout();
    }
    return;
}/*}}}*/
void Mangler::commentButton_clicked_cb(void) {/*{{{*/
    if (v3_is_loggedin()) {
        textStringChangeCommentEntry->set_text(comment);
        textStringChangeURLEntry->set_text(url);
        textStringChangeIntegrationEntry->set_text(integration_text);
        textStringChangeDialog->run();
        textStringChangeDialog->hide();
    }
}/*}}}*/
void Mangler::chatButton_clicked_cb(void) {/*{{{*/
    if (v3_is_loggedin()) {
        if(!chat->isOpen) {
            chat->chatWindow->set_icon(icons["tray_icon"]);
            chat->chatWindow->show();
        } else {
            chat->chatWindow->present();
        }
    }
}/*}}}*/
void Mangler::bindingsButton_clicked_cb(void) {/*{{{*/
    //fprintf(stderr, "bindings button clicked\n");
    static Glib::ustring color = "red";
    if (color == "red") {
        color = "green";
        statusIcon->set(icons["tray_icon_green"]);
    } else if (color == "green") {
        color = "blue";
        statusIcon->set(icons["tray_icon_blue"]);
    } else if (color == "blue") {
        color = "yellow";
        statusIcon->set(icons["tray_icon_yellow"]);
    } else if (color == "yellow") {
        color = "red";
        statusIcon->set(icons["tray_icon_red"]);
    }
}/*}}}*/
void Mangler::adminButton_clicked_cb(void) {/*{{{*/
    Glib::ustring password;
    if (! isAdmin) {
        password = mangler->getPasswordEntry("Admin Password");
        if (password.length()) {
            v3_admin_login((char *)password.c_str());
            // if we tried sending a password, the only options are either
            // success or get booted from the server
        }
    } else {
        admin->adminWindow->show();
    }
}/*}}}*/
void Mangler::adminWindowMenuItem_activated_cb(void) {/*{{{*/
    admin->adminWindow->show();
}/*}}}*/
void Mangler::settingsButton_clicked_cb(void) {/*{{{*/
    settings->settingsWindow->show();
}/*}}}*/
void Mangler::aboutButton_clicked_cb(void) {/*{{{*/
    builder->get_widget("aboutWindow", aboutdialog);
    aboutdialog->set_keep_above(true);
    aboutdialog->set_logo(icons["mangler_logo"]);
    aboutdialog->run();
    aboutdialog->hide();
}/*}}}*/
void Mangler::xmitButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("xmitButton", togglebutton);
    if (togglebutton->get_active()) {
        isTransmittingButton = true;
        startTransmit();
    } else {
        stopTransmit();
        isTransmittingButton = false;
    }
}/*}}}*/

/*
 * Menu bar signal handler callbacks
 */
void Mangler::buttonMenuItem_toggled_cb(void) {/*{{{*/
    builder->get_widget("buttonMenuItem", checkmenuitem);
    builder->get_widget("mainWindowButtonVBox", vbox);
    if (checkmenuitem->get_active()) {
        vbox->hide();
        settings->config.buttonsHidden = true;
    } else {
        settings->config.buttonsHidden = false;
        vbox->show();
    }
    settings->config.save();
}/*}}}*/
void Mangler::hideServerInfoMenuItem_toggled_cb(void) {/*{{{*/
    builder->get_widget("hideServerInfoMenuItem", checkmenuitem);
    builder->get_widget("serverTable", table);
    if (checkmenuitem->get_active()) {
        table->hide();
        settings->config.serverInfoHidden = true;
    } else {
        settings->config.serverInfoHidden = false;
        table->show();
    }
    settings->config.save();
}/*}}}*/
void Mangler::hideGuestFlagMenuItem_toggled_cb(void) {/*{{{*/
    builder->get_widget("hideGuestFlagMenuItem", checkmenuitem);
    if (checkmenuitem->get_active()) {
        settings->config.guestFlagHidden = true;
    } else {
        settings->config.guestFlagHidden = false;
    }
    channelTree->refreshAllUsers();
    settings->config.save();
}/*}}}*/
void Mangler::quitMenuItem_activate_cb(void) {/*{{{*/
    Gtk::Main::quit();
}/*}}}*/

/*
 * Other signal handler callbacks
 */
void Mangler::statusIcon_activate_cb(void) {/*{{{*/
    if (iconified == true) {
        manglerWindow->deiconify();
        manglerWindow->present();
        manglerWindow->set_skip_pager_hint(false);
        manglerWindow->set_skip_taskbar_hint(false);
        iconified = false;
    } else {
        manglerWindow->iconify();
        manglerWindow->set_skip_pager_hint(true);
        manglerWindow->set_skip_taskbar_hint(true);
        iconified = true;
    }
}/*}}}*/
bool Mangler::mangler_quit_cb(void) {/*{{{*/
    int w, h;
    manglerWindow->get_size(w, h);
    settings->config.windowWidth = w;
    settings->config.windowHeight = h;
    settings->config.save();
    return true;
}/*}}}*/

void Mangler::startTransmit(void) {/*{{{*/
    const v3_codec *codec;
    v3_user  *user;

    if (! v3_is_loggedin()) {
        return;
    }
    if (muteMic) {
        return;
    }
    user = v3_get_user(v3_get_user_id());
    if (! user) {
        return;
    }
    if (isTransmitting) {
        v3_free_user(user);
        return;
    }
    isTransmitting = true;
    if ((codec = v3_get_channel_codec(user->channel))) {
        //fprintf(stderr, "channel %d codec rate: %d at sample size %d\n", user->channel, codec->rate, codec->pcmframesize);
        v3_free_user(user);
        channelTree->setUserIcon(v3_get_user_id(), "orange");
        statusIcon->set(icons["tray_icon_yellow"]);
        inputAudio = new ManglerAudio("input");
        inputAudio->open(codec->rate, AUDIO_INPUT, codec->pcmframesize);
    }
}/*}}}*/
void Mangler::stopTransmit(void) {/*{{{*/
    if (!isTransmitting) {
        return;
    }
    isTransmitting = false;
    if (v3_is_loggedin()) {
        channelTree->setUserIcon(v3_get_user_id(), "red");
    }
    statusIcon->set(icons["tray_icon_red"]);
    if (inputAudio) {
        inputAudio->finish();
    }
}/*}}}*/

// Quick Sound Mute
void Mangler::muteSoundCheckButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("muteSoundCheckButton", checkbutton);
    muteSound = checkbutton->get_active();
}/*}}}*/

// Quick Mic Mute
void Mangler::muteMicCheckButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("muteMicCheckButton", checkbutton);
    muteMic = checkbutton->get_active();
    if (muteMic && isTransmitting) {
        stopTransmit();
    } else if (!muteMic && (isTransmittingMouse || isTransmittingKey || isTransmittingButton)) {
        startTransmit();
    }
}/*}}}*/

// Quick Connect callbacks
void Mangler::qcConnectButton_clicked_cb(void) {/*{{{*/
    Gtk::Entry *textbox;

    channelTree->updateLobby("Connecting...");
    builder->get_widget("qcServerName", textbox);
    Glib::ustring server = textbox->get_text();
    builder->get_widget("qcPort", textbox);
    Glib::ustring port = textbox->get_text();
    builder->get_widget("qcUsername", textbox);
    Glib::ustring username = textbox->get_text();
    builder->get_widget("qcPassword", textbox);
    Glib::ustring password = textbox->get_text();
    //fprintf(stderr, "connecting to: %s:%s\n", server.c_str(), port.c_str());
    settings->config.qc_lastserver.hostname = server;
    settings->config.qc_lastserver.port = port;
    settings->config.qc_lastserver.username = username;
    settings->config.qc_lastserver.password = password;
    settings->config.save();
    set_charset("");
    connectedServerId = -1;
    isAdmin = false;
    isChanAdmin = false;
    v3_set_server_opts(V3_USER_ACCEPT_PAGES, 1);
    v3_set_server_opts(V3_USER_ACCEPT_U2U, 1);
    v3_set_server_opts(V3_USER_ACCEPT_CHAT, 1);
    v3_set_server_opts(V3_USER_ALLOW_RECORD, 1);
    Glib::Thread::create(sigc::bind(sigc::mem_fun(this->network, &ManglerNetwork::connect), server, port, username, password, ""), FALSE);
    // TODO: move this into a thread and use blocking waits
}/*}}}*/
void Mangler::qcCancelButton_clicked_cb(void) {/*{{{*/
}/*}}}*/

// MOTD Window Callbacks
void Mangler::motdOkButton_clicked_cb(void) {/*{{{*/
    builder->get_widget("motdWindow", window);
    window->hide();
}/*}}}*/

/*
 * Auto reconnect handling
 */
bool Mangler::reconnectStatusHandler(void) {/*{{{*/
    Gtk::Button *connectbutton;
    char buf[64] = "";
    int reconnectTimer = (15 - (time(NULL) - lastAttempt));

    builder->get_widget("connectButton", connectbutton);
    if (connectbutton->get_label() == "gtk-disconnect" || wantDisconnect) {
        return false;
    }
    builder->get_widget("statusbar", statusbar);
    snprintf(buf, 63, "Attempting reconnect in %d seconds", reconnectTimer);
    statusbar->pop();
    statusbar->push(buf);
    if (!reconnectTimer) {
        lastAttempt = time(NULL);
        connectbutton->set_label("gtk-connect");
        Mangler::connectButton_clicked_cb();
    }

    return true;
}/*}}}*/

void Mangler::onDisconnectHandler(void) {/*{{{*/
    Gtk::Button *connectbutton;
    ManglerServerConfig *server;

    builder->get_widget("connectButton", connectbutton);
    if (connectbutton->get_label() == "gtk-disconnect") {
        builder->get_widget("adminButton", button);
        button->set_sensitive(false);
        builder->get_widget("adminLoginMenuItem", menuitem);
        menuitem->set_sensitive(false);
        builder->get_widget("adminWindowMenuItem", menuitem);
        menuitem->set_sensitive(false);
        builder->get_widget("chatButton", button);
        button->set_sensitive(false);
        builder->get_widget("chatMenuItem", menuitem);
        menuitem->set_sensitive(false);
        builder->get_widget("commentButton", button);
        button->set_sensitive(false);
        builder->get_widget("commentMenuItem", menuitem);
        menuitem->set_sensitive(false);

        connectbutton->set_sensitive(true);
        channelTree->clear();
        admin->clearChannels();
        builder->get_widget("xmitButton", togglebutton);
        togglebutton->set_active(false);
        builder->get_widget("progressbar", progressbar);
        progressbar->set_text("");
        progressbar->set_fraction(0);
        builder->get_widget("statusbar", statusbar);
        statusbar->pop();
        statusbar->push("Not connected");
        //builder->get_widget("serverTabLabel", label);
        //label->set_label("Not Connected");
        builder->get_widget("pingLabel", label);
        label->set_label("N/A");
        builder->get_widget("userCountLabel", label);
        label->set_label("N/A");
        builder->get_widget("codecLabel", label);
        label->set_label("N/A");
        mangler->statusIcon->set(icons["tray_icon_grey"]);
        audioControl->playNotification("logout");
        isAdmin = false;
        isChanAdmin = false;
        builder->get_widget("adminSeparatorMenuItem", menuitem);
        menuitem->hide();
        builder->get_widget("adminLoginMenuItem", menuitem);
        menuitem->hide();
        builder->get_widget("adminWindowMenuItem", menuitem);
        menuitem->hide();
        chat->clear();
        if (connectedServerId != -1) {
            server = settings->config.getserver(connectedServerId);
            connectedServerId = -1;
            if (!wantDisconnect && server->persistentConnection) {
                connectbutton->set_label("Cancel Reconnect");
                lastAttempt = time(NULL);
                Glib::signal_timeout().connect_seconds(sigc::mem_fun(*this, &Mangler::reconnectStatusHandler), 1);
                return;
            }
        }
    }
    connectbutton->set_label("gtk-connect");
    builder->get_widget("serverSelectComboBox", combobox);
    combobox->set_sensitive(true);
}/*}}}*/

// Timeout Callbacks
/*
 * Inbound event processing happens here
 */
bool Mangler::getNetworkEvent() {/*{{{*/
    v3_event *ev;

    while ((ev = v3_get_event(V3_NONBLOCK)) != NULL) {
        v3_user *u;
        v3_channel *c;
        Glib::ustring rank = "";
        gdk_threads_enter();
        // if we're not logged in, just ignore whatever messages we receive
        // *unless* it's a disconnect message.  This prevents old messages in
        // the queue from attempting to interact with the GUI after a
        // disconnection
        switch (ev->type) {
            case V3_EVENT_PING:/*{{{*/
                if (v3_is_loggedin()) {
                    char buf[32];
                    builder->get_widget("pingLabel", label);
                    if (ev->ping != 65535) {
                        builder->get_widget("statusbar", statusbar);
                        snprintf(buf, 31, "Ping: %dms", ev->ping);
                        statusbar->pop();
                        statusbar->push(buf);
                        snprintf(buf, 16, "%d", ev->ping);
                        label->set_text(buf);
                    } else {
                        label->set_text("checking...");
                        statusbar->pop();
                        statusbar->push("Ping: checking...");
                    }
                    builder->get_widget("userCountLabel", label);
                    snprintf(buf, 16, "%d/%d", v3_user_count(), v3_get_max_clients());
                    label->set_text(buf);
                }
                break;/*}}}*/
            case V3_EVENT_STATUS:/*{{{*/
                builder->get_widget("progressbar", progressbar);
                builder->get_widget("statusbar", statusbar);
                if (ev->status.percent == 100) {
                    progressbar->hide();
                } else {
                    progressbar->show();
                    progressbar->set_fraction(ev->status.percent/(float)100);
                }
                statusbar->pop();
                statusbar->push(ev->status.message);
                //fprintf(stderr, "got event type %d: %d %s\n", ev->type, ev->status.percent, ev->status.message);
                break;/*}}}*/
            case V3_EVENT_USER_LOGIN:/*{{{*/
                u = v3_get_user(ev->user.id);
                if (!u) {
                    fprintf(stderr, "couldn't retreive user id %d\n", ev->user.id);
                    break;
                }
                if (!(ev->flags & V3_LOGIN_FLAGS_EXISTING) && (v3_get_user_channel(v3_get_user_id()) == ev->channel.id)) {
                    audioControl->playNotification("channelenter");
                }
                //fprintf(stderr, "adding user id %d: %s to channel %d\n", ev->user.id, u->name, ev->channel.id);
                if (u->rank_id) {
                    v3_rank *r;
                    if ((r = v3_get_rank(u->rank_id))) {
                        rank = c_to_ustring(r->name);
                        v3_free_rank(r);
                    }
                }
                channelTree->addUser(
                        (uint32_t)u->id,
                        (uint32_t)ev->channel.id,
                        c_to_ustring(u->name),
                        c_to_ustring(u->comment),
                        u->phonetic,
                        u->url,
                        c_to_ustring(u->integration_text),
                        (bool)u->guest,
                        (bool)u->real_user_id,
                        rank);
                // If we have a per user volume set for this user name, set it now
                if (connectedServerId != -1) {
                    ManglerServerConfig *server;
                    server = settings->config.getserver(connectedServerId);
                    std::map<Glib::ustring, uint8_t>::iterator it;
                    it = server->uservolumes.find(u->name);
                    if (it != server->uservolumes.end()) {
                        v3_set_volume_user(u->id, server->uservolumes[u->name]);
                    }
                }
                v3_free_user(u);
                break;/*}}}*/
            case V3_EVENT_USER_MODIFY:/*{{{*/
                if (v3_is_loggedin()) {
                    u = v3_get_user(ev->user.id);
                    if (!u) {
                        fprintf(stderr, "couldn't retreive user id %d\n", ev->user.id);
                        break;
                    }
                    if (u->id == 0) {
                        channelTree->updateLobby(c_to_ustring(u->name), c_to_ustring(u->comment), u->phonetic);
                    } else {
                        if (u->rank_id) {
                            v3_rank *r;
                            if ((r = v3_get_rank(u->rank_id))) {
                                rank = c_to_ustring(r->name);
                                v3_free_rank(r);
                            }
                        }
                        channelTree->updateUser(
                                (uint32_t)u->id,
                                (uint32_t)ev->channel.id,
                                c_to_ustring(u->name),
                                c_to_ustring(u->comment),
                                u->phonetic,
                                u->url,
                                c_to_ustring(u->integration_text),
                                (bool)u->guest,
                                (bool)u->real_user_id,
                                rank);
                    }
                    v3_free_user(u);
                }
                break;/*}}}*/
            case V3_EVENT_CHAN_MODIFY:/*{{{*/
                if (v3_is_loggedin()) {
                    const v3_codec *codec_info;
                    c = v3_get_channel(ev->channel.id);
                    if (!c) {
                        fprintf(stderr, "couldn't retreive channel id %d\n", ev->user.id);
                        break;
                    }
                    channelTree->updateChannel(
                            (uint8_t)c->protect_mode,
                            (uint32_t)c->id,
                            (uint32_t)c->parent,
                            c_to_ustring(c->name),
                            c_to_ustring(c->comment),
                            c->phonetic);
                    if (! isAdmin && ! isChanAdmin && v3_is_channel_admin(c->id)) {
                        isChanAdmin = true;
                        builder->get_widget("adminSeparatorMenuItem", menuitem);
                        menuitem->show();
                        builder->get_widget("adminWindowMenuItem", menuitem);
                        menuitem->show();
                    }
                    admin->channelUpdated(c);
                    if (ev->channel.id == v3_get_user_channel(v3_get_user_id())) {
                        builder->get_widget("codecLabel", label);
                        if ((codec_info = v3_get_channel_codec(ev->channel.id))) {
                            label->set_text(codec_info->name);
                        } else {
                            label->set_text("Unsupported Codec");
                        }
                    }
                    v3_free_channel(c);
                }
                break;/*}}}*/
            case V3_EVENT_USER_LOGOUT:/*{{{*/
                if (v3_is_loggedin()) {
                    if (outputAudio[ev->user.id]) {
                        outputAudio[ev->user.id]->finish();
                        outputAudio.erase(ev->user.id);
                    }
                    if (v3_get_user_channel(v3_get_user_id()) == ev->channel.id) {
                        audioControl->playNotification("channelleave");
                    }
                    // can't get any user info... it's already gone by this point
                    //fprintf(stderr, "removing user id %d\n", ev->user.id);
                    channelTree->removeUser(ev->user.id);
                    chat->removeUser(ev->user.id);
                }
                break;/*}}}*/
            case V3_EVENT_CHAN_REMOVE:/*{{{*/
                if (v3_is_loggedin()) {
                    // can't get any channel info... it's already gone by this point
                    //fprintf(stderr, "removing channel id %d\n", ev->channel.id);
                    channelTree->removeChannel(ev->channel.id);
                    admin->channelRemoved(ev->channel.id);
                }
                break;/*}}}*/
            case V3_EVENT_LOGIN_COMPLETE:/*{{{*/
                if (v3_is_loggedin()) {
                    const v3_codec *codec_info;

                    builder->get_widget("adminButton", button);
                    button->set_sensitive(true);
                    builder->get_widget("adminLoginMenuItem", menuitem);
                    menuitem->set_sensitive(true);
                    builder->get_widget("adminWindowMenuItem", menuitem);
                    menuitem->set_sensitive(true);
                    builder->get_widget("chatButton", button);
                    button->set_sensitive(true);
                    builder->get_widget("chatMenuItem", menuitem);
                    menuitem->set_sensitive(true);
                    builder->get_widget("commentButton", button);
                    button->set_sensitive(true);
                    builder->get_widget("commentMenuItem", menuitem);
                    menuitem->set_sensitive(true);

                    builder->get_widget("codecLabel", label);
                    if ((codec_info = v3_get_channel_codec(0))) {
                        label->set_text(codec_info->name);
                    } else {
                        label->set_text("Unsupported Codec");
                    }
                    channelTree->expand_all();
                    audioControl->playNotification("login");
                    if (connectedServerId != -1) {
                        ManglerServerConfig *server;
                        server = settings->config.getserver(connectedServerId);
                        if (server->persistentComments) {
                            comment = server->comment;
                            url = server->url;
                            v3_set_text((char *) ustring_to_c(server->comment).c_str(), (char *) ustring_to_c(server->url).c_str(), (char *) ustring_to_c(integration_text).c_str(), true);
                        }

                        //Default Channel
                        uint32_t defaultChannelId = server->defaultchannelid;
                        if (defaultChannelId) {
                            //For now, only handling right click menu set
                            channelTree->channelView_switchChannel2Default(defaultChannelId);
                        }
                    }
                    builder->get_widget("chatWindow", window);
                    if(chat->isOpen) {
                        v3_join_chat();
                    }
                    myID = v3_get_user_id();
                    builder->get_widget("adminSeparatorMenuItem", menuitem);
                    menuitem->show();
                    builder->get_widget("adminLoginMenuItem", menuitem);
                    menuitem->show();
                }
                break;/*}}}*/
            case V3_EVENT_USER_CHAN_MOVE:/*{{{*/
                {
                    u = v3_get_user(ev->user.id);
                    if (! u) {
                        fprintf(stderr, "failed to retreive user information for user id %d\n", ev->user.id);
                        break;
                    }
                    if (ev->user.id == v3_get_user_id()) {
                        // we're moving channels... update the codec label
                        const v3_codec *codec_info;
                        builder->get_widget("codecLabel", label);
                        if ((codec_info = v3_get_channel_codec(ev->channel.id))) {
                            label->set_text(codec_info->name);
                        } else {
                            label->set_text("Unsupported Codec");
                        }
                    } else {
                        if (ev->channel.id == v3_get_user_channel(v3_get_user_id())) {
                            // they're joining our channel
                            audioControl->playNotification("channelenter");
                        } else if (channelTree->getUserChannelId(ev->user.id) == v3_get_user_channel(v3_get_user_id())) {
                            // they're leaving our channel
                            audioControl->playNotification("channelleave");
                        }
                    }
                    if (u->rank_id) {
                        v3_rank *r;
                        if ((r = v3_get_rank(u->rank_id))) {
                            rank = c_to_ustring(r->name);
                            v3_free_rank(r);
                        }
                    }
                    //fprintf(stderr, "moving user id %d to channel id %d\n", ev->user.id, ev->channel.id);
                    Glib::ustring last_transmit = channelTree->getLastTransmit((uint32_t)ev->user.id);
                    channelTree->removeUser((uint32_t)ev->user.id);
                    channelTree->addUser(
                            (uint32_t)u->id,
                            (uint32_t)ev->channel.id,
                            c_to_ustring(u->name),
                            c_to_ustring(u->comment),
                            u->phonetic,
                            u->url,
                            c_to_ustring(u->integration_text),
                            (bool)u->guest,
                            (bool)u->real_user_id,
                            rank);
                    channelTree->setLastTransmit(ev->user.id, last_transmit);
                    // if there was an audio stream open for this user, close it
                    if (outputAudio[ev->user.id]) {
                        outputAudio[ev->user.id]->finish();
                        outputAudio.erase(ev->user.id);
                    }
                    channelTree->refreshAllUsers();
                    chat->chatUserTreeModelFilter->refilter();
                    v3_free_user(u);
                }
                break;/*}}}*/
            case V3_EVENT_CHAN_ADD:/*{{{*/
                c = v3_get_channel(ev->channel.id);
                if (! c) {
                    fprintf(stderr, "failed to retreive channel information for channel id %d\n", ev->channel.id);
                    break;;
                }
                channelTree->addChannel(
                        (uint8_t)c->protect_mode,
                        (uint32_t)c->id,
                        (uint32_t)c->parent,
                        c_to_ustring(c->name),
                        c_to_ustring(c->comment),
                        c->phonetic);
                if (! isAdmin && ! isChanAdmin && v3_is_channel_admin(c->id)) {
                    isChanAdmin = true;
                    builder->get_widget("adminSeparatorMenuItem", menuitem);
                    menuitem->show();
                    builder->get_widget("adminWindowMenuItem", menuitem);
                    menuitem->show();
                }
                admin->channelAdded(c);
                v3_free_channel(c);
                break;/*}}}*/
            case V3_EVENT_CHAN_BADPASS:/*{{{*/
                channelTree->forgetChannelSavedPassword(ev->channel.id);
                errorDialog(c_to_ustring(ev->error.message));
                break;/*}}}*/
            case V3_EVENT_ERROR_MSG:/*{{{*/
                errorDialog(c_to_ustring(ev->error.message));
                break;/*}}}*/
            case V3_EVENT_USER_TALK_START:/*{{{*/
                v3_user *me, *user;
                me = v3_get_user(v3_get_user_id());
                user = v3_get_user(ev->user.id);
                channelTree->refreshUser(ev->user.id);
                if (me && user && me->channel == user->channel) {
                    v3_free_user(me);
                    v3_free_user(user);
                } else {
                    if (!me) {
                        fprintf(stderr, "couldn't find my own user info %d\n", v3_get_user_id());
                    } else {
                        v3_free_user(me);
                    }
                    if (!user) {
                        fprintf(stderr, "couldn't find user for for user id %d\n", ev->user.id);
                    } else {
                        v3_free_user(user);
                    }
                }
                break;/*}}}*/
            case V3_EVENT_USER_TALK_END:/*{{{*/
                if (v3_is_loggedin()) {
                    //fprintf(stderr, "user %d stopped talking\n", ev->user.id);
                    channelTree->refreshUser(ev->user.id);
                    // TODO: this is bad, there must be a flag in the last audio
                    // packet saying that it's the last one.  Need to figure out
                    // what that flag is and close it in V3_EVENT_PLAY_AUDIO
                    if (outputAudio[ev->user.id]) {
                        outputAudio[ev->user.id]->finish();
                        outputAudio.erase(ev->user.id);
                    }
                }
                break;/*}}}*/
            case V3_EVENT_PLAY_AUDIO:/*{{{*/
                if (v3_is_loggedin()) {
                    channelTree->setUserIcon(ev->user.id, "green", true);
                    if (!channelTree->isMuted(ev->user.id) && !muteSound) {
                        // Open a stream if we don't have one for this user
                        if (!outputAudio[ev->user.id]) {
                            outputAudio[ev->user.id] = new ManglerAudio("output");
                            outputAudio[ev->user.id]->open(ev->pcm.rate, AUDIO_OUTPUT, 0, ev->pcm.channels);
                        }
                        // And queue the audio
                        if (outputAudio[ev->user.id]) {
                            outputAudio[ev->user.id]->queue(ev->pcm.length, (uint8_t *)ev->data.sample);
                        }
                    }
                }
                break;/*}}}*/
            case V3_EVENT_DISPLAY_MOTD:/*{{{*/
                {
                    uint32_t motdhash = 0;
                    ManglerServerConfig *server = NULL;

                    if (connectedServerId != -1) {
                        server = settings->config.getserver(connectedServerId);
                        // we're not launching a space shuttle here, no need for
                        // anything super complex
                        for (uint32_t ctr = 0; ctr < strlen(ev->data.motd); ctr++) {
                            motdhash += ev->data.motd[ctr] + ctr;
                        }
                    }
                    if (connectedServerId == -1 || motdAlways || (server && (motdhash != server->motdhash))) {
                        Glib::RefPtr< Gtk::TextBuffer > tb = Gtk::TextBuffer::create();
                        builder->get_widget("motdWindow", window);
                        window->set_title("Mangler - MOTD");
                        builder->get_widget("motdTextView", textview);
                        tb->set_text(c_to_ustring(ev->data.motd));
                        textview->set_buffer(tb);
                        window->show();
                        if (server) {
                            server->motdhash = motdhash;
                            settings->config.save();
                        }
                    }
                }
                break;/*}}}*/
            case V3_EVENT_DISCONNECT:/*{{{*/
                onDisconnectHandler();
                break;/*}}}*/
            case V3_EVENT_CHAT_JOIN:/*{{{*/
                {
                    chat->addUser(ev->user.id);
                    u = v3_get_user(ev->user.id);
                    if (!u) {
                        fprintf(stderr, "couldn't retreive user id %d\n", ev->user.id);
                        break;
                    }
                    if (u->id != 0) {
                        if (u->rank_id) {
                            v3_rank *r;
                            if ((r = v3_get_rank(u->rank_id))) {
                                rank = c_to_ustring(r->name);
                                v3_free_rank(r);
                            }
                        }
                        channelTree->updateUser(
                                (uint32_t)u->id,
                                (uint32_t)u->channel,
                                c_to_ustring(u->name),
                                c_to_ustring(u->comment),
                                u->phonetic,
                                u->url,
                                c_to_ustring(u->integration_text),
                                (bool)u->guest,
                                (bool)u->real_user_id,
                                rank);
                    }
                    v3_free_user(u);
                }
                break;/*}}}*/
            case V3_EVENT_CHAT_LEAVE:/*{{{*/
                {
                    chat->removeUser(ev->user.id);
                    u = v3_get_user(ev->user.id);
                    if (!u) {
                        fprintf(stderr, "couldn't retreive user id %d\n", ev->user.id);
                        break;
                    }
                    if (u->id != 0) {
                        if (u->rank_id) {
                            v3_rank *r;
                            if ((r = v3_get_rank(u->rank_id))) {
                                rank = c_to_ustring(r->name);
                                v3_free_rank(r);
                            }
                        }
                        channelTree->updateUser(
                                (uint32_t)u->id,
                                (uint32_t)u->channel,
                                c_to_ustring(u->name),
                                c_to_ustring(u->comment),
                                u->phonetic,
                                u->url,
                                c_to_ustring(u->integration_text),
                                (bool)u->guest,
                                (bool)u->real_user_id,
                                rank);
                    }
                    v3_free_user(u);
                }
                break;/*}}}*/
            case V3_EVENT_CHAT_MESSAGE:/*{{{*/
                if (v3_is_loggedin()) {
                    if (ev->user.id == 0)
                        chat->addRconMessage(c_to_ustring(ev->data.chatmessage));
                    else
                        chat->addChatMessage(ev->user.id, c_to_ustring(ev->data.chatmessage));
                }
                break;/*}}}*/
            case V3_EVENT_PRIVATE_CHAT_START:/*{{{*/
                {
                    uint16_t remote;
                    if (ev->user.privchat_user1 == v3_get_user_id()) {
                        remote = ev->user.privchat_user2;
                    } else {
                        remote = ev->user.privchat_user1;
                    }
                    if (privateChatWindows[remote]) {
                        privateChatWindows[remote]->remoteReopened();
                    } else {
                        v3_user *u;
                        Glib::ustring name = "unknown";
                        if ((u = v3_get_user(remote)) != NULL) {
                            name = c_to_ustring(u->name);
                            v3_free_user(u);
                        }
                        privateChatWindows[remote] = new ManglerPrivChat(remote);
                        privateChatWindows[remote]->addMessage("*** opened private chat with " + name);
                    }
                }
                break;/*}}}*/
            case V3_EVENT_PRIVATE_CHAT_END:/*{{{*/
                {
                    if (privateChatWindows[ev->user.privchat_user2]) {
                        privateChatWindows[ev->user.privchat_user2]->remoteClosed();
                    }
                }
                break;/*}}}*/
            case V3_EVENT_PRIVATE_CHAT_AWAY:/*{{{*/
                {
                    if (privateChatWindows[ev->user.privchat_user2]) {
                        privateChatWindows[ev->user.privchat_user2]->remoteAway();
                    }
                }
                break;/*}}}*/
            case V3_EVENT_PRIVATE_CHAT_BACK:/*{{{*/
                {
                    if (privateChatWindows[ev->user.privchat_user2]) {
                        privateChatWindows[ev->user.privchat_user2]->remoteBack();
                    }
                }
                break;/*}}}*/
            case V3_EVENT_PRIVATE_CHAT_MESSAGE:/*{{{*/
                {
                    uint16_t remote;
                    if (ev->user.privchat_user1 == v3_get_user_id()) {
                        remote = ev->user.privchat_user2;
                    } else {
                        remote = ev->user.privchat_user1;
                    }
                    if (privateChatWindows[remote]) {
                        if (!ev->flags) { // set to true on error
                            privateChatWindows[remote]->addChatMessage(ev->user.privchat_user2, c_to_ustring(ev->data.chatmessage));
                        } else {
                            privateChatWindows[remote]->addMessage("*** error sending message to remote user");
                        }
                    }
                }
                break;/*}}}*/
            case V3_EVENT_ADMIN_AUTH:/*{{{*/
                {
                    const v3_permissions *perms = v3_get_permissions();
                    if (perms->srv_admin) {
                        isAdmin = true;
                        builder->get_widget("adminLoginMenuItem", menuitem);
                        menuitem->hide();
                        builder->get_widget("adminWindowMenuItem", menuitem);
                        menuitem->show();
                    } else {
                        isAdmin = false;
                        builder->get_widget("adminLoginMenuItem", menuitem);
                        menuitem->show();
                        builder->get_widget("adminWindowMenuItem", menuitem);
                        if (isChanAdmin) {
                            menuitem->show();
                        } else {
                            menuitem->hide();
                        }
                    }
                    v3_user *lobby = v3_get_user(0);
                    if (lobby) {
                        channelTree->updateLobby(c_to_ustring(lobby->name), c_to_ustring(lobby->comment), lobby->phonetic);
                        v3_free_user(lobby);
                    }
                }
                break;/*}}}*/
            case V3_EVENT_CHAN_ADMIN_UPDATED:/*{{{*/
                channelTree->refreshAllChannels();
                break;/*}}}*/
            case V3_EVENT_USER_GLOBAL_MUTE_CHANGED:/*{{{*/
                channelTree->refreshUser(ev->user.id);
                break;/*}}}*/
            case V3_EVENT_SERVER_PROPERTY_UPDATED:/*{{{*/
                switch (ev->serverproperty.property) {
                    case V3_SERVER_CHAT_FILTER:
                        chat->isGlobal = ev->serverproperty.value;
                        chat->chatUserTreeModelFilter->refilter();
                        break;
                    case V3_SERVER_ALPHABETIC:
                        channelTree->sortAlphanumeric = ev->serverproperty.value;
                        channelTree->refreshAllChannels();
                        admin->channelSort(ev->serverproperty.value);
                        break;
                    case V3_SERVER_MOTD_ALWAYS:
                        motdAlways = ev->serverproperty.value;
                        break;
                }
                break;/*}}}*/
            default:
                fprintf(stderr, "******************************************************** got unknown event type %d\n", ev->type);
        }
        channelTree->expand_all();
        free(ev);
        gdk_threads_leave();
    }
    return true;
}/*}}}*/
bool Mangler::checkPushToTalkKeys(void) {/*{{{*/
    char        pressed_keys[32];
    GdkWindow   *rootwin = gdk_get_default_root_window();
    vector<int>::iterator i;
    bool        ptt_on = true;;

    if (! settings->config.PushToTalkKeyEnabled) {
        isTransmittingKey = false;
        return true;
    }
    XQueryKeymap(GDK_WINDOW_XDISPLAY(rootwin), pressed_keys);
    for (   i = settings->config.PushToTalkXKeyCodes.begin();
            i < settings->config.PushToTalkXKeyCodes.end();
            i++) {
        if (!((pressed_keys[*i >> 3] >> (*i & 0x07)) & 0x01)) {
            ptt_on = false;
            break;
        }
    }
    if (ptt_on) {
        isTransmittingKey = true;
        startTransmit();
    } else {
        isTransmittingKey = false;
        if (! isTransmittingButton && ! isTransmittingMouse) {
            stopTransmit();
        }
    }
    return(true);

}/*}}}*/
bool Mangler::checkPushToTalkMouse(void) {/*{{{*/
    GdkWindow   *rootwin = gdk_get_default_root_window();
    XDevice *dev = NULL;
    XDeviceInfo *xdev;
    XDeviceState *xds;
    XButtonState *xbs;
    int ctr;
    int ndevices_return;
    int state = 1;
    int byte = settings->config.PushToTalkMouseValueInt / 8;
    int bit = settings->config.PushToTalkMouseValueInt % 8;
    bool        ptt_on = false;

    if (! settings->config.PushToTalkMouseEnabled) {
        isTransmittingMouse = false;
        return true;
    }
    if (settings->config.mouseDeviceName.empty()) {
        return true;
    }

    xdev = XListInputDevices(GDK_WINDOW_XDISPLAY(rootwin), &ndevices_return);
    for (ctr = 0; ctr < ndevices_return; ctr++) {
        Glib::ustring name = xdev[ctr].name;
        if (name == settings->config.mouseDeviceName && xdev[ctr].use == IsXExtensionPointer) {
            break;
        }
    }
    if (ctr == ndevices_return) {
        return true;
    }

    dev = XOpenDevice(GDK_WINDOW_XDISPLAY(rootwin), xdev[ctr].id);
    if (! dev) {
        return true;
    }
    xds = (XDeviceState *)XQueryDeviceState(GDK_WINDOW_XDISPLAY(rootwin), dev);
    xbs = (XButtonState*) xds->data;
    state = state << bit;
    /* debug mouse state buttons
    for (int ctr = 1; ctr < 10; ctr++) {
        int byte = ctr / 8;
        int bit = ctr % 8;
        state = 1 << bit;
        fprintf(stderr, "(%d/%d)", byte, bit);
        fprintf(stderr, "b%d: %d -- ", ctr, (xbs->buttons[byte] & state) >> bit);
    }
    fprintf(stderr, "\n");
    */
    if ((xbs->buttons[byte] & state) >> bit) {
        ptt_on = true;
    }
    XFreeDeviceState(xds);
    XFreeDeviceList(xdev);
    XCloseDevice(GDK_WINDOW_XDISPLAY(rootwin), dev);

    if (ptt_on) {
        isTransmittingMouse = true;
        startTransmit();
    } else {
        isTransmittingMouse = false;
        if (! isTransmittingButton && ! isTransmittingKey) {
            stopTransmit();
        }
    }
    return(true);
}/*}}}*/
bool Mangler::updateXferAmounts(void) {/*{{{*/
    uint32_t packets, bytes;
    char buf[1024];

    builder->get_widget("sentLabel", label);
    bytes = v3_get_bytes_sent();
    packets = v3_get_packets_sent();
    if (bytes > 1024*1024) {
        snprintf(buf, 1023, "%2.2f megabytes / %d packets", (float)bytes/1024/1024, packets);
    } else if (bytes > 1024) {
        snprintf(buf, 1023, "%2.2f kilobytes / %d packets", (float)bytes/1024, packets);
    } else if (bytes) {
        snprintf(buf, 1023, "%d bytes / %d packets", bytes, packets);
    } else {
        snprintf(buf, 1023, "N/A");
    }
    label->set_text(buf);

    builder->get_widget("recvLabel", label);
    bytes = v3_get_bytes_recv();
    packets = v3_get_packets_recv();
    if (bytes > 1024*1024) {
        snprintf(buf, 1023, "%2.2f megabytes / %d packets", (float)bytes/1024/1024, packets);
    } else if (bytes > 1024) {
        snprintf(buf, 1023, "%2.2f kilobytes / %d packets", (float)bytes/1024, packets);
    } else if (bytes) {
        snprintf(buf, 1023, "%d bytes / %d packets", bytes, packets);
    } else {
        snprintf(buf, 1023, "N/A");
    }
    label->set_text(buf);

    return(true);
}/*}}}*/
/* {{{ GdkFilterReturn ptt_filter(GdkXEvent *gdk_xevent, GdkEvent *event, gpointer data) {
    GdkWindow   *rootwin = gdk_get_default_root_window();
    XEvent *xevent = (XEvent *)gdk_xevent;

    if (! mangler) {
        return GDK_FILTER_CONTINUE;
    }
    if (! mangler->settings->config.PushToTalkMouseEnabled) {
        mangler->isTransmittingKey = false;
        return GDK_FILTER_CONTINUE;
    }
    if (xevent->type == ButtonPress && !mangler->isTransmittingMouse && xevent->xbutton.button == mangler->settings->config.PushToTalkMouseValueInt) {
        fprintf(stderr, "press\n");
        mangler->startTransmit();
        mangler->isTransmittingMouse = true;
        fprintf(stderr, "allow\n");
        XAllowEvents(GDK_WINDOW_XDISPLAY(rootwin), AsyncPointer, CurrentTime);
        fprintf(stderr, "ungrab pointer\n");
        XUngrabPointer(GDK_WINDOW_XDISPLAY(rootwin), CurrentTime);
        fprintf(stderr, "ungrab button\n");
        XUngrabButton(GDK_WINDOW_XDISPLAY(rootwin), mangler->settings->config.PushToTalkMouseValueInt, AnyModifier, GDK_ROOT_WINDOW());
        fprintf(stderr, "grab button\n");
        XGrabButton(GDK_WINDOW_XDISPLAY(rootwin),   mangler->settings->config.PushToTalkMouseValueInt, AnyModifier, GDK_ROOT_WINDOW(), True, ButtonPressMask|ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None);
        return GDK_FILTER_CONTINUE;
    } else if ((xevent->type == ButtonRelease) && (xevent->xbutton.button == mangler->settings->config.PushToTalkMouseValueInt)) {
        fprintf(stderr, "release\n");
        mangler->stopTransmit();
        mangler->isTransmittingMouse = false;
        fprintf(stderr, "allow\n");
        XAllowEvents(GDK_WINDOW_XDISPLAY(rootwin), AsyncPointer, CurrentTime);
        fprintf(stderr, "ungrab pointer\n");
        XUngrabPointer(GDK_WINDOW_XDISPLAY(rootwin), CurrentTime);
        fprintf(stderr, "ungrab button\n");
        XUngrabButton(GDK_WINDOW_XDISPLAY(rootwin), mangler->settings->config.PushToTalkMouseValueInt, AnyModifier, GDK_ROOT_WINDOW());
        fprintf(stderr, "grab button\n");
        XGrabButton(GDK_WINDOW_XDISPLAY(rootwin), mangler->settings->config.PushToTalkMouseValueInt, AnyModifier, GDK_ROOT_WINDOW(), True, ButtonPressMask|ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None);
        return GDK_FILTER_CONTINUE;
    }
    return GDK_FILTER_CONTINUE;
} }}} */
bool Mangler::updateIntegration(void) {/*{{{*/
    if (v3_is_loggedin()) {
        if (!settings->config.AudioIntegrationEnabled || integration->client == MusicClient_None) {
            if (integration_text != "") {
                    v3_set_text((char *) ustring_to_c(comment).c_str(), (char *) ustring_to_c(url).c_str(), (char *)"", false);
            }
            return true;
        }
        Glib::ustring formatted_text = integration->format();
        switch (integration->get_mode()) {
            // Polling (mpd)
            case 0:
                if ( ((integration->update(false) || !integration->first()) ) || integration_text != formatted_text ) {
                    integration_text =  integration->format();
                    v3_set_text((char *) ustring_to_c(comment).c_str(), (char *) ustring_to_c(url).c_str(), (char *) ustring_to_c(integration_text).c_str(), false);
                }
                break;

                // Listening / callbacks (dbus ones)
            case 1:
                if (integration_text != formatted_text) {
                    integration_text = formatted_text;
                    v3_set_text((char *) ustring_to_c(comment).c_str(), (char *) ustring_to_c(url).c_str(), (char *) ustring_to_c(integration_text).c_str(), false);
                }
                break;
        }
    }
    return true;
}/*}}}*/

Glib::ustring Mangler::getPasswordEntry(Glib::ustring title, Glib::ustring prompt) {/*{{{*/
    password = "";
    passwordEntry->set_text("");
    passwordDialog->set_keep_above(true);
    passwordDialog->set_title(title);
    passwordDialog->run();
    passwordDialog->hide();
    return(password);
}/*}}}*/
void Mangler::passwordDialogOkButton_clicked_cb(void) {/*{{{*/
    password = passwordEntry->get_text();
}/*}}}*/
void Mangler::passwordDialogCancelButton_clicked_cb(void) {/*{{{*/
    password = "";
}/*}}}*/

bool Mangler::getReasonEntry(Glib::ustring title, Glib::ustring prompt) {/*{{{*/
    reason = "";
    reasonValid = false;
    reasonEntry->set_text("");
    reasonDialog->set_keep_above(true);
    reasonDialog->set_title(title);
    reasonDialog->run();
    reasonDialog->hide();
    return(reasonValid);
}/*}}}*/
void Mangler::reasonDialogOkButton_clicked_cb(void) {/*{{{*/
    reason = reasonEntry->get_text();
    reasonValid = true;
}/*}}}*/
void Mangler::reasonDialogCancelButton_clicked_cb(void) {/*{{{*/
    reasonValid = false;
}/*}}}*/

void Mangler::textStringChangeDialogOkButton_clicked_cb(void) {/*{{{*/
    comment          = textStringChangeCommentEntry->get_text();
    url              = textStringChangeURLEntry->get_text();
    integration_text = textStringChangeIntegrationEntry->get_text();
    if (connectedServerId != -1) {
        ManglerServerConfig *server;
        server = settings->config.getserver(connectedServerId);
        if (server->persistentComments) {
            server->comment = comment;
            settings->config.save();
        }
    }
    v3_set_text((char *)ustring_to_c(comment).c_str(), (char *)ustring_to_c(url).c_str(), (char *)ustring_to_c(integration_text).c_str(), true);
}/*}}}*/
void Mangler::textStringChangeDialogCancelButton_clicked_cb(void) {/*{{{*/
    textStringChangeCommentEntry->set_text(comment);
    textStringChangeURLEntry->set_text(url);
    textStringChangeIntegrationEntry->set_text(integration_text);
}/*}}}*/


// Misc Functions
uint32_t Mangler::getActiveServer(void) {/*{{{*/
    builder->get_widget("serverSelectComboBox", combobox);
    return combobox->get_active_row_number();
}/*}}}*/
void Mangler::setActiveServer(uint32_t row_number) {/*{{{*/
    builder->get_widget("serverSelectComboBox", combobox);
    combobox->set_active(row_number);
}/*}}}*/
void Mangler::errorDialog(Glib::ustring message) {/*{{{*/
    builder->get_widget("errorWindow", window);
    window->set_keep_above(true);
    window->set_icon(icons["tray_icon"]);
    builder->get_widget("errorMessageLabel", label);
    label->set_text(message);
    window->show();
    /*
    builder->get_widget("errorDialog", msgdialog);
    msgdialog->set_icon(icons["tray_icon"]);
    msgdialog->set_keep_above(true);
    msgdialog->set_message(message);
    msgdialog->run();
    msgdialog->hide();
    */
}/*}}}*/
void Mangler::errorOKButton_clicked_cb(void) {/*{{{*/
    builder->get_widget("errorWindow", window);
    window->hide();
}/*}}}*/

ManglerError::ManglerError(uint32_t code, Glib::ustring message, Glib::ustring module) {/*{{{*/
    this->code = code;
    this->message = message;
    this->module = module;
}/*}}}*/

int
main (int argc, char *argv[])
{
    Gtk::Main kit(argc, argv);
    struct _cli_options options;
    char *locale;

    // TODO: use getopt()
    if (argc > 1) {
        options.uifilename = argv[1];
        options.uifromfile = true;
        fprintf(stderr, "using ui file: %s\n", options.uifilename.c_str());
    } else {
        options.uifromfile = false;
    }
    if ((locale = setlocale (LC_ALL, ""))) {
              //fprintf(stderr, "initialized locale: %s\n", locale);
    } else {
              fprintf(stderr, "Can't set the specified locale! " "Check LANG, LC_CTYPE, LC_ALL.\n");
              return 1;
    }
    if(!Glib::thread_supported()) {
        Glib::thread_init();
    }
    gdk_threads_init();
    mangler = new Mangler(&options);
    gdk_threads_enter();
    Gtk::Main::run(*mangler->manglerWindow);
    gdk_threads_leave();
}

