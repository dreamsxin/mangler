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

#include <gtkmm.h>
#include <iostream>
#include "mangler.h"
#include "manglerui.h"
#include "mangler-icons.h"
#include <gdk/gdkx.h>


Glib::ustring c_to_ustring(char *input);
Glib::ustring iso_8859_1_to_utf8(char *input);

using namespace std;

Mangler *mangler;

Mangler::Mangler(struct _cli_options *options) {/*{{{*/
    // load all of our icons
    icons.insert(std::make_pair("black_circle",                 Gdk::Pixbuf::create_from_inline(-1, black_circle                )));
    icons.insert(std::make_pair("blue_circle",                  Gdk::Pixbuf::create_from_inline(-1, blue_circle                 )));
    icons.insert(std::make_pair("cyan_circle",                  Gdk::Pixbuf::create_from_inline(-1, cyan_circle                 )));
    icons.insert(std::make_pair("green_circle",                 Gdk::Pixbuf::create_from_inline(-1, green_circle                )));
    icons.insert(std::make_pair("grey_circle",                  Gdk::Pixbuf::create_from_inline(-1, grey_circle                 )));
    icons.insert(std::make_pair("purple_circle",                Gdk::Pixbuf::create_from_inline(-1, purple_circle               )));
    icons.insert(std::make_pair("red_circle",                   Gdk::Pixbuf::create_from_inline(-1, red_circle                  )));
    icons.insert(std::make_pair("yellow_circle",                Gdk::Pixbuf::create_from_inline(-1, yellow_circle               )));
    icons.insert(std::make_pair("logo1",                        Gdk::Pixbuf::create_from_inline(-1, logo1                       )));
    icons.insert(std::make_pair("logo2",                        Gdk::Pixbuf::create_from_inline(-1, logo2                       )));
    icons.insert(std::make_pair("logo3",                        Gdk::Pixbuf::create_from_inline(-1, logo3                       )));

    icons.insert(std::make_pair("tray_icon",                    Gdk::Pixbuf::create_from_inline(-1, tray_icon_purple            )));
    icons.insert(std::make_pair("tray_icon_blue",               Gdk::Pixbuf::create_from_inline(-1, tray_icon_blue              )));
    icons.insert(std::make_pair("tray_icon_red",                Gdk::Pixbuf::create_from_inline(-1, tray_icon_red               )));
    icons.insert(std::make_pair("tray_icon_green",              Gdk::Pixbuf::create_from_inline(-1, tray_icon_green             )));
    icons.insert(std::make_pair("tray_icon_yellow",             Gdk::Pixbuf::create_from_inline(-1, tray_icon_yellow            )));
    icons.insert(std::make_pair("tray_icon_grey",               Gdk::Pixbuf::create_from_inline(-1, tray_icon_grey              )));
    icons.insert(std::make_pair("tray_icon_purple",             Gdk::Pixbuf::create_from_inline(-1, tray_icon_purple            )));

    icons.insert(std::make_pair("user_icon_xmit",               Gdk::Pixbuf::create_from_inline(-1, user_icon_xmit              )));
    icons.insert(std::make_pair("user_icon_noxmit",             Gdk::Pixbuf::create_from_inline(-1, user_icon_noxmit            )));

    icons.insert(std::make_pair("user_icon_xmit_otherroom",     Gdk::Pixbuf::create_from_inline(-1, user_icon_xmit_otherroom    )));
    icons.insert(std::make_pair("user_icon_noxmit_otherroom",   Gdk::Pixbuf::create_from_inline(-1, user_icon_noxmit_otherroom  )));


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

    // Settings Button
    builder->get_widget("settingsButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::settingsButton_clicked_cb));

    // About Button
    builder->get_widget("aboutButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::aboutButton_clicked_cb));

    // Settings Button
    builder->get_widget("xmitButton", button);
    button->signal_pressed().connect(sigc::mem_fun(this, &Mangler::xmitButton_pressed_cb));
    button->signal_released().connect(sigc::mem_fun(this, &Mangler::xmitButton_released_cb));

    // Quick Connect Dialog Buttons
    builder->get_widget("qcConnectButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::qcConnectButton_clicked_cb));

    builder->get_widget("qcCancelButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::qcCancelButton_clicked_cb));

    // MOTD Window Buttons
    builder->get_widget("motdOkButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::motdOkButton_clicked_cb));

    // Set up our generic password dialog box
    builder->get_widget("passwordDialog", passwordDialog);
    builder->get_widget("passwordEntry", passwordEntry);
    builder->get_widget("passwordOkButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::passwordDialogOkButton_clicked_cb));
    builder->get_widget("passwordCancelButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &Mangler::passwordDialogCancelButton_clicked_cb));

    // Create Channel Tree
    channelTree = new ManglerChannelTree(builder);

    // Create Network Communication Object
    network = new ManglerNetwork(builder);

    // Create our audio control object for managing devices
    audioControl = new ManglerAudio("control");
    audioControl->getDeviceList();

    // Create settings object, load the configuration file, and apply.  If the
    // user has PTT key/mouse enabled, start a timer here
    settings = new ManglerSettings(builder);
    isTransmittingButton = 0;
    isTransmittingMouse = 0;
    isTransmittingKey = 0;
    isTransmitting = 0;
    Glib::signal_timeout().connect(sigc::mem_fun(this, &Mangler::checkPushToTalkKeys), 100);
    Glib::signal_timeout().connect(sigc::mem_fun(this, &Mangler::checkPushToTalkMouse), 100);

    // Statusbar Icon
    statusIcon = Gtk::StatusIcon::create(icons["tray_icon_grey"]);
    statusIcon->signal_activate().connect(sigc::mem_fun(this, &Mangler::statusIcon_activate_cb));
    iconified = false;

}/*}}}*/

/*
 * Signal handler callbacks
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
    dialog->run();
    dialog->hide();

}/*}}}*/
void Mangler::serverConfigButton_clicked_cb(void) {/*{{{*/
    builder->get_widget("serverListWindow", window);
    window->set_icon(icons["tray_icon"]);
    window->show();
}/*}}}*/
void Mangler::connectButton_clicked_cb(void) {/*{{{*/
    builder->get_widget("connectButton", button);
    channelTree->updateLobby("Connecting...");
    if (button->get_label() == "gtk-connect") {
        Glib::Thread::create(sigc::bind(sigc::mem_fun(this->network, &ManglerNetwork::connect), "localhost", "3784", "username", "password"), FALSE);
        // TODO: move this into a thread and use blocking waits
        Glib::signal_timeout().connect( sigc::mem_fun(*this, &Mangler::getNetworkEvent), 10 );
    } else {
        v3_logout();
    }
    return;
}/*}}}*/
void Mangler::commentButton_clicked_cb(void) {/*{{{*/
    //fprintf(stderr, "comment button clicked\n");
}/*}}}*/
void Mangler::chatButton_clicked_cb(void) {/*{{{*/
    //fprintf(stderr, "chat button clicked\n");
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
    //fprintf(stderr, "admin button clicked\n");
}/*}}}*/
void Mangler::settingsButton_clicked_cb(void) {/*{{{*/
    settings->settingsWindow->show();
}/*}}}*/
void Mangler::aboutButton_clicked_cb(void) {/*{{{*/
    builder->get_widget("aboutWindow", aboutdialog);
    aboutdialog->set_icon(icons["tray_icon"]);
    aboutdialog->set_logo(icons["tray_icon"]);
    aboutdialog->run();
    aboutdialog->hide();
}/*}}}*/
void Mangler::xmitButton_pressed_cb(void) {/*{{{*/
    //fprintf(stderr, "xmit clicked\n");
    isTransmittingButton = true;
    startTransmit();
}/*}}}*/
void Mangler::xmitButton_released_cb(void) {/*{{{*/
    isTransmittingButton = false;
    if (! isTransmittingKey && ! isTransmittingMouse) {
        stopTransmit();
    }
}/*}}}*/
void Mangler::statusIcon_activate_cb(void) {/*{{{*/
    if (iconified == true) {
        manglerWindow->deiconify();
        manglerWindow->set_skip_pager_hint(false);
        manglerWindow->set_skip_taskbar_hint(false);
        iconified = false;
    } else {
        manglerWindow->set_skip_pager_hint(true);
        manglerWindow->set_skip_taskbar_hint(true);
        manglerWindow->iconify();
        iconified = true;
    }
}/*}}}*/

void Mangler::startTransmit(void) {/*{{{*/
    const v3_codec *codec;
    v3_user  *user;

    if (! v3_is_loggedin()) {
        return;
    }
    user = v3_get_user(v3_get_user_id());
    if (! user) {
        return;
    }
    if (isTransmitting) {
        return;
    }
    audioControl->playNotification("talkstart");
    statusIcon->set(icons["tray_icon_green"]);
    isTransmitting = true;
    channelTree->userIsTalking(v3_get_user_id(), true);
    codec = v3_get_channel_codec(user->channel);
    //fprintf(stderr, "channel %d codec rate: %d at sample size %d\n", user->channel, codec->rate, codec->samplesize);
    v3_start_audio(V3_AUDIO_SENDTYPE_U2CCUR);
    v3_free_user(user);
    inputAudio = new ManglerAudio("input");
    inputAudio->open(codec->rate, AUDIO_INPUT, codec->samplesize);
}/*}}}*/
void Mangler::stopTransmit(void) {/*{{{*/
    if (!isTransmitting) {
        return;
    }
    audioControl->playNotification("talkend");
    statusIcon->set(icons["tray_icon_red"]);
    channelTree->userIsTalking(v3_get_user_id(), false);
    isTransmitting = false;
    if (inputAudio) {
        inputAudio->finish();
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
    fprintf(stderr, "connecting to: %s:%s\n", server.c_str(), port.c_str());
    settings->config.qc_lastserver.hostname = server;
    settings->config.qc_lastserver.port = port;
    settings->config.qc_lastserver.username = username;
    settings->config.qc_lastserver.password = password;
    settings->config.save();
    Glib::Thread::create(sigc::bind(sigc::mem_fun(this->network, &ManglerNetwork::connect), server, port, username, password), FALSE);
    // TODO: move this into a thread and use blocking waits
    Glib::signal_timeout().connect( sigc::mem_fun(*this, &Mangler::getNetworkEvent), 10 );
}/*}}}*/
void Mangler::qcCancelButton_clicked_cb(void) {/*{{{*/
}/*}}}*/

// MOTD Window Callbacks
void Mangler::motdOkButton_clicked_cb(void) {/*{{{*/
    builder->get_widget("motdWindow", window);
    window->hide();
}/*}}}*/

// Timeout Callbacks
bool Mangler::getNetworkEvent() {/*{{{*/
    v3_event *ev;

    while ((ev = v3_get_event(V3_NONBLOCK)) != NULL) {
        v3_user *u;
        v3_channel *c;
        gdk_threads_enter();
        switch (ev->type) {
            case V3_EVENT_PING:/*{{{*/
                char buf[16];
                builder->get_widget("pingLabel", label);
                if (ev->ping != 65535) {
                    snprintf(buf, 16, "%d", ev->ping);
                    label->set_text(buf);
                } else {
                    label->set_text("checking...");
                }
                builder->get_widget("userCountLabel", label);
                snprintf(buf, 16, "%d/%d", v3_user_count(), v3_get_max_clients());
                label->set_text(buf);
                break;/*}}}*/
            case V3_EVENT_STATUS:/*{{{*/
                builder->get_widget("progressbar", progressbar);
                builder->get_widget("statusbar", statusbar);
                progressbar->set_fraction(ev->status.percent/(float)100);
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
                //fprintf(stderr, "adding user id %d: %s to channel %d\n", ev->user.id, u->name, ev->channel.id);
                channelTree->addUser(
                        (uint32_t)u->id,
                        (uint32_t)ev->channel.id,
                        c_to_ustring(u->name),
                        c_to_ustring(u->comment),
                        u->phonetic,
                        u->url,
                        c_to_ustring(u->integration_text),
                        (bool)u->guest);
                v3_free_user(u);
                break;/*}}}*/
            case V3_EVENT_USER_MODIFY:/*{{{*/
                u = v3_get_user(ev->user.id);
                if (!u) {
                    fprintf(stderr, "couldn't retreive user id %d\n", ev->user.id);
                    break;
                }
                // we cannot remove the lobby user, so bail out when the server comment is updated. See ticket #30
                if (u->id == 0) {
                    break;
                }
                //fprintf(stderr, "updating user id %d: %s in channel %d\n", ev->user.id, u->name, ev->channel.id);
                channelTree->removeUser(ev->user.id);
                channelTree->addUser(
                        (uint32_t)u->id,
                        (uint32_t)ev->channel.id,
                        c_to_ustring(u->name),
                        c_to_ustring(u->comment),
                        u->phonetic,
                        u->url,
                        c_to_ustring(u->integration_text),
                        (bool)u->guest);
                v3_free_user(u);
                break;/*}}}*/
            case V3_EVENT_USER_LOGOUT:/*{{{*/
                // can't get any user info... it's already gone by this point
                //fprintf(stderr, "removing user id %d\n", ev->user.id);
                channelTree->removeUser(ev->user.id);
                break;/*}}}*/
            case V3_EVENT_CHAN_REMOVE:/*{{{*/
                // can't get any channel info... it's already gone by this point
                //fprintf(stderr, "removing channel id %d\n", ev->channel.id);
                channelTree->removeChannel(ev->channel.id);
                break;/*}}}*/
            case V3_EVENT_LOGIN_COMPLETE:/*{{{*/
                {
                    const v3_codec *codec_info;
                    codec_info = v3_get_channel_codec(0);
                    builder->get_widget("codecLabel", label);
                    label->set_text(codec_info->name);
                    channelTree->expand_all();
                    audioControl->playNotification("login");
                }
                break;/*}}}*/
            case V3_EVENT_USER_CHAN_MOVE:/*{{{*/
                u = v3_get_user(ev->user.id);
                if (! u) {
                    fprintf(stderr, "failed to retreive user information for user id %d", ev->user.id);
                    break;;
                }
                if (ev->user.id == v3_get_user_id()) {
                    // we're moving channels... update the codec label
                    const v3_codec *codec_info;
                    codec_info = v3_get_channel_codec(ev->channel.id);
                    builder->get_widget("codecLabel", label);
                    label->set_text(codec_info->name);
                } else {
                    if (ev->channel.id == v3_get_user_channel(v3_get_user_id())) {
                        // they're joining our channel
                        audioControl->playNotification("channelenter");
                    } else if (channelTree->getUserChannelId(ev->user.id) == v3_get_user_channel(v3_get_user_id())) {
                        // they're leaving our channel
                        audioControl->playNotification("channelleave");
                    }
                }
                //fprintf(stderr, "moving user id %d to channel id %d\n", ev->user.id, ev->channel.id);
                channelTree->removeUser((uint32_t)ev->user.id);
                channelTree->addUser(
                        (uint32_t)u->id,
                        (uint32_t)ev->channel.id,
                        c_to_ustring(u->name),
                        c_to_ustring(u->comment),
                        u->phonetic,
                        u->url,
                        c_to_ustring(u->integration_text),
                        (bool)u->guest);
                v3_free_user(u);
                break;/*}}}*/
            case V3_EVENT_CHAN_ADD:/*{{{*/
                c = v3_get_channel(ev->channel.id);
                if (! c) {
                    fprintf(stderr, "failed to retreive channel information for channel id %d", ev->channel.id);
                    break;;
                }
                channelTree->addChannel(
                        (uint8_t)c->protect_mode,
                        (uint32_t)c->id,
                        (uint32_t)c->parent,
                        c_to_ustring(c->name),
                        c_to_ustring(c->comment),
                        c->phonetic);
                v3_free_channel(c);
                break;/*}}}*/
            case V3_EVENT_ERROR_MSG:/*{{{*/
                builder->get_widget("errorDialog", msgdialog);
                msgdialog->set_icon(icons["tray_icon"]);
                msgdialog->set_message(ev->error.message);
                msgdialog->run();
                msgdialog->hide();
                break;/*}}}*/
            case V3_EVENT_USER_TALK_START:/*{{{*/
                {
                    v3_user *me, *user;
                    me = v3_get_user(v3_get_user_id());
                    user = v3_get_user(ev->user.id);
                    channelTree->userIsTalking(ev->user.id, true);
                    if (me && user && me->channel == user->channel) {
                        v3_free_user(me);
                        v3_free_user(user);
                    } else {
                        if (!me) {
                            fprintf(stderr, "couldn't find my own user info %d\n", v3_get_user_id());
                        }
                        if (!user) {
                            fprintf(stderr, "couldn't find user for for user id %d\n", ev->user.id);
                        }
                    }
                }
                break;/*}}}*/
            case V3_EVENT_USER_TALK_END:/*{{{*/
                //fprintf(stderr, "user %d stopped talking\n", ev->user.id);
                channelTree->userIsTalking(ev->user.id, false);
                // TODO: this is bad, there must be a flag in the last audio
                // packet saying that it's the last one.  Need to figure out
                // what that flag is and close it in V3_EVENT_PLAY_AUDIO
                if (outputAudio[ev->user.id]) {
                    outputAudio[ev->user.id]->finish();
                    outputAudio.erase(ev->user.id);
                }
                break;/*}}}*/
            case V3_EVENT_PLAY_AUDIO:/*{{{*/
                // Open a stream if we don't have one for this user
                channelTree->userIsTalking(ev->user.id, true);
                if (!outputAudio[ev->user.id]) {
                    outputAudio[ev->user.id] = new ManglerAudio("output");
                    outputAudio[ev->user.id]->open(ev->pcm.rate, AUDIO_OUTPUT);
                }
                // And queue the audio
                if (outputAudio[ev->user.id]) {
                    outputAudio[ev->user.id]->queue(ev->pcm.length, (uint8_t *)ev->data.sample);
                }
                break;/*}}}*/
            case V3_EVENT_DISPLAY_MOTD:/*{{{*/
                {
                    Glib::RefPtr< Gtk::TextBuffer > tb = Gtk::TextBuffer::create();
                    builder->get_widget("motdWindow", window);
                    window->set_title("Mangler - MOTD");
                    builder->get_widget("motdTextView", textview);
                    tb->set_text(c_to_ustring(ev->data.motd));
                    textview->set_buffer(tb);
                    window->show();
                }
                break;/*}}}*/
            case V3_EVENT_DISCONNECT:/*{{{*/
                {
                    channelTree->clear();
                    button->set_label("gtk-connect");
                    builder->get_widget("progressbar", progressbar);
                    builder->get_widget("statusbar", statusbar);
                    progressbar->set_text("");
                    progressbar->set_fraction(0);
                    statusbar->pop();
                    statusbar->push("Not connected");
                    builder->get_widget("serverTabLabel", label);
                    label->set_label("Not Connected");
                    builder->get_widget("pingLabel", label);
                    label->set_label("N/A");
                    builder->get_widget("userCountLabel", label);
                    label->set_label("N/A");
                    builder->get_widget("codecLabel", label);
                    label->set_label("N/A");
                    mangler->statusIcon->set(icons["tray_icon_grey"]);
                    audioControl->playNotification("logout");
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

    Window root_return, child_return;
    int root_x_return, root_y_return;
    int win_x_return, win_y_return;
    unsigned int mask_return;

    bool        ptt_on = false;

    if (! settings->config.PushToTalkMouseEnabled) {
        isTransmittingKey = false;
        return true;
    }
    XQueryPointer(GDK_WINDOW_XDISPLAY(rootwin), GDK_ROOT_WINDOW(), &root_return, &child_return, &root_x_return, &root_y_return, &win_x_return, &win_y_return, &mask_return);
    if (settings->config.PushToTalkMouseValue == "Button1" &&  mask_return & Button1Mask) {
        ptt_on = true;
    } else if (settings->config.PushToTalkMouseValue == "Button2" &&  mask_return & Button2Mask) {
        ptt_on = true;
    } else if (settings->config.PushToTalkMouseValue == "Button3" &&  mask_return & Button3Mask) {
        ptt_on = true;
    } else if (settings->config.PushToTalkMouseValue == "Button4" &&  mask_return & Button4Mask) {
        ptt_on = true;
    } else if (settings->config.PushToTalkMouseValue == "Button5" &&  mask_return & Button5Mask) {
        ptt_on = true;
    } else {
        ptt_on = false;
    }
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

Glib::ustring Mangler::getPasswordEntry(Glib::ustring title, Glib::ustring prompt) {/*{{{*/
    password = "";
    passwordEntry->set_text("");
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

Glib::ustring c_to_ustring(char *input) {/*{{{*/
    Glib::ustring converted, input_u = input;

    // check if input is already valid UTF-8
    if (input_u.validate())
        return input_u;

   // try to convert using the current locale
    try {
        converted = Glib::locale_to_utf8(input);
    } catch (Glib::ConvertError &e) {
        // locale conversion failed
        converted = iso_8859_1_to_utf8(input);
    }

    return converted;
}/*}}}*/

/* converts a CP1252/ISO-8859-1(5) hybrid to UTF-8                           */
/* Features: 1. It never fails, all 00-FF chars are converted to valid UTF-8 */
/*           2. Uses CP1252 in the range 80-9f because ISO doesn't have any- */
/*              thing useful in this range and it helps us receive from mIRC */
/*           3. The five undefined chars in CP1252 80-9f are replaced with   */
/*              ISO-8859-15 control codes.                                   */
/*           4. Handles 0xa4 as a Euro symbol ala ISO-8859-15.               */
/*           5. Uses ISO-8859-1 (which matches CP1252) for everything else.  */
/*           6. This routine measured 3x faster than g_convert :)            */
Glib::ustring iso_8859_1_to_utf8 (char *input) {/*{{{*/
    Glib::ustring output;
    unsigned char *text = (unsigned char *)input;
    int len = strlen(input);
    static const gunichar lowtable[] = /* 74 byte table for 80-a4 */
    {
    /* compressed utf-8 table */
        0x20ac, /* 80 Euro. CP1252 from here on... */
        0x81,   /* 81 NA */
        0x201a, /* 82 */
        0x192,  /* 83 */
        0x201e, /* 84 */
        0x2026, /* 85 */
        0x2020, /* 86 */
        0x2021, /* 87 */
        0x2c6,  /* 88 */
        0x2030, /* 89 */
        0x160,  /* 8a */
        0x2039, /* 8b */
        0x152,  /* 8c */
        0x8d,   /* 8d NA */
        0x17d,  /* 8e */
        0x8f,   /* 8f NA */
        0x90,   /* 90 NA */
        0x2018, /* 91 */
        0x2019, /* 92 */
        0x201c, /* 93 */
        0x201d, /* 94 */
        0x2022, /* 95 */
        0x2013, /* 96 */
        0x2014, /* 97 */
        0x2dc,  /* 98 */
        0x2122, /* 99 */
        0x161,  /* 9a */
        0x203a, /* 9b */
        0x153,  /* 9c */
        0x9d,   /* 9d NA */
        0x17e,  /* 9e */
        0x178,  /* 9f */
        0xa0,   /* a0 */
        0xa1,   /* a1 */
        0xa2,   /* a2 */
        0xa3,   /* a3 */
        0x20ac  /* a4 ISO-8859-15 Euro. */
    };

    while (len) {
        if (G_UNLIKELY(*text >= 0x80) && G_UNLIKELY(*text <= 0xa4)) {
            int idx = *text - 0x80;
            output += lowtable[idx];
        } else {
            output += (gunichar)*text;    /* ascii/iso88591 maps directly */
        }

        text++;
        len--;
    }
    return output;
}/*}}}*/
