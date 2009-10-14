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

Glib::ustring c_to_ustring(char *input);

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


    try {
        if (options->uifromfile) {
            builder = Gtk::Builder::create_from_file(options->uifilename);
        } else {
            builder = Gtk::Builder::create_from_string(ManglerUI);
        }
        builder->get_widget("manglerWindow", manglerWindow);
        manglerWindow->set_icon(icons["logo2"]);
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

    // Create settings object and load the configuration file
    settings = new ManglerSettings(builder);
    settings->config.load();

    // Statusbar Icon
    statusIcon = Gtk::StatusIcon::create(icons["logo2"]);
}/*}}}*/

/*
 * Signal handler callbacks
 */
void Mangler::quickConnectButton_clicked_cb(void) {/*{{{*/
    Gtk::Dialog *dialog;
    Gtk::Entry *textbox;

    builder->get_widget("quickConnectDialog", dialog);
    dialog->set_icon(icons["logo2"]);

    builder->get_widget("qcServerName", textbox);
    textbox->set_text(settings->config.qc_lastserver.name);

    builder->get_widget("qcPort", textbox);
    textbox->set_text(settings->config.qc_lastserver.port);

    builder->get_widget("qcUsername", textbox);
    textbox->set_text(settings->config.qc_lastserver.username);

    builder->get_widget("qcPassword", textbox);
    textbox->set_text(settings->config.qc_lastserver.password);

    dialog->run();
    dialog->hide();

}/*}}}*/
void Mangler::serverConfigButton_clicked_cb(void) {/*{{{*/
    builder->get_widget("serverListWindow", window);
    window->set_icon(icons["logo2"]);
    window->show();
}/*}}}*/
void Mangler::connectButton_clicked_cb(void) {/*{{{*/
    builder->get_widget("connectButton", button);
    channelTree->updateLobby("Connecting...");
    if (button->get_label() == "gtk-connect") {
        Glib::Thread::create(sigc::bind(sigc::mem_fun(this->network, &ManglerNetwork::connect), "localhost", "3784", "username", "password"), FALSE);
        Glib::signal_timeout().connect( sigc::mem_fun(*this, &Mangler::getNetworkEvent), 50 );
    } else {
        channelTree->clear();
        button->set_label("gtk-connect");
        builder->get_widget("progressbar", progressbar);
        builder->get_widget("statusbar", statusbar);
        progressbar->set_text("");
        progressbar->set_fraction(0);
        statusbar->pop();
        statusbar->push("Not connected");
        v3_logout();
    }
    return;
}/*}}}*/
void Mangler::commentButton_clicked_cb(void) {/*{{{*/
    fprintf(stderr, "comment button clicked\n");
}/*}}}*/
void Mangler::chatButton_clicked_cb(void) {/*{{{*/
    fprintf(stderr, "chat button clicked\n");
}/*}}}*/
void Mangler::settingsButton_clicked_cb(void) {/*{{{*/
    settings->settingsWindow->show();
}/*}}}*/
void Mangler::aboutButton_clicked_cb(void) {/*{{{*/
    builder->get_widget("aboutWindow", aboutdialog);
    aboutdialog->set_icon(icons["logo2"]);
    aboutdialog->set_logo(icons["logo2"]);
    aboutdialog->run();
    aboutdialog->hide();
}/*}}}*/
void Mangler::xmitButton_pressed_cb(void) {/*{{{*/
    fprintf(stderr, "xmit button pressed\n");
}/*}}}*/
void Mangler::xmitButton_released_cb(void) {/*{{{*/
    fprintf(stderr, "xmit button released\n");
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
    settings->config.qc_lastserver.name = server;
    settings->config.qc_lastserver.port = port;
    settings->config.qc_lastserver.username = username;
    settings->config.qc_lastserver.password = password;
    settings->config.save();
    Glib::Thread::create(sigc::bind(sigc::mem_fun(this->network, &ManglerNetwork::connect), server, port, username, password), FALSE);
    Glib::signal_timeout().connect( sigc::mem_fun(*this, &Mangler::getNetworkEvent), 50 );
}/*}}}*/
void Mangler::qcCancelButton_clicked_cb(void) {/*{{{*/
}/*}}}*/

// MOTD Window Callbacks
void Mangler::motdOkButton_clicked_cb(void) {/*{{{*/
    builder->get_widget("motdWindow", window);
    window->hide();
}/*}}}*/

bool
Mangler::getNetworkEvent() {/*{{{*/
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
                if (ev->status.percent == 100) {
                    progressbar->set_text("");
                } else {
                    progressbar->set_text("Logging in...");
                }
                statusbar->pop();
                statusbar->push(ev->status.message);
                fprintf(stderr, "got event type %d: %d %s\n", ev->type, ev->status.percent, ev->status.message);
                break;/*}}}*/
            case V3_EVENT_USER_LOGIN:/*{{{*/
                u = v3_get_user(ev->user.id);
                fprintf(stderr, "adding user id %d: %s\n", ev->user.id, u->name);
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
            case V3_EVENT_CHAN_REMOVE:/*{{{*/
                // can't get any channel info... it's already gone by this point
                fprintf(stderr, "removing channel id %d\n", ev->channel.id);
                channelTree->removeChannel(ev->channel.id);
                break;/*}}}*/
            case V3_EVENT_USER_LOGOUT:/*{{{*/
                // can't get any user info... it's already gone by this point
                fprintf(stderr, "removing user id %d\n", ev->user.id);
                channelTree->removeUser(ev->user.id);
                break;/*}}}*/
            case V3_EVENT_LOGIN_COMPLETE:/*{{{*/
                {
                    const v3_codec *codec_info;
                    codec_info = v3_get_channel_codec(0);
                    builder->get_widget("codecLabel", label);
                    label->set_text(codec_info->name);
                    channelTree->expand_all();
                }
                break;/*}}}*/
            case V3_EVENT_USER_CHAN_MOVE:/*{{{*/
                u = v3_get_user(ev->user.id);
                if (! u) {
                    fprintf(stderr, "failed to retreive user information for user id %d", ev->user.id);
                    break;;
                }
                if (ev->user.id == v3_get_user_id()) {
                    // we're moving channels...
                    // update the codec label
                    const v3_codec *codec_info;
                    codec_info = v3_get_channel_codec(ev->channel.id);
                    builder->get_widget("codecLabel", label);
                    label->set_text(codec_info->name);
                }
                fprintf(stderr, "moving user id %d to channel id %d\n", ev->user.id, ev->channel.id);
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
                msgdialog->set_icon(icons["logo2"]);
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
                        if (!audio[ev->user.id]) {
                            audio[ev->user.id] = new ManglerAudio(ev->user.id, ev->pcm.rate);
                        }
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
                fprintf(stderr, "user %d stopped talking\n", ev->user.id);
                channelTree->userIsTalking(ev->user.id, false);
                if (audio[ev->user.id]) {
                    audio[ev->user.id]->finish();
                    audio.erase(ev->user.id);
                }
                break;/*}}}*/
            case V3_EVENT_PLAY_AUDIO:/*{{{*/
                if (audio[ev->user.id]) {
                    audio[ev->user.id]->queue(ev->pcm.length, (uint8_t *)ev->data.sample);
                }
                break;/*}}}*/
            case V3_EVENT_DISPLAY_MOTD:/*{{{*/
                {
                    Glib::RefPtr< Gtk::TextBuffer > tb = Gtk::TextBuffer::create();
                    builder->get_widget("motdWindow", window);
                    window->set_title("Mangler - MOTD");
                    builder->get_widget("motdTextView", textview);
                    tb->set_text(ev->data.motd);
                    textview->set_buffer(tb);
                    window->show();
                }
                break;/*}}}*/
            default:
                fprintf(stderr, "******************************************************** got unknown event type %d\n", ev->type);
        }
        channelTree->expand_all();
        gdk_threads_leave();
        free(ev);
    }
    return true;
}/*}}}*/

Glib::ustring
Mangler::getPasswordEntry(Glib::ustring title, Glib::ustring prompt) {
    password = "";
    passwordEntry->set_text("");
    passwordDialog->set_title(title);
    passwordDialog->run();
    passwordDialog->hide();
    return(password);
}

void
Mangler::passwordDialogOkButton_clicked_cb(void) {
    password = passwordEntry->get_text();
}

void
Mangler::passwordDialogCancelButton_clicked_cb(void) {
    password = "";
}



ManglerError::ManglerError(uint32_t code, Glib::ustring message, Glib::ustring module) {
    this->code = code;
    this->message = message;
    this->module = module;
}

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
    if (locale = setlocale (LC_ALL, "")) {
              fprintf(stderr, "initialized locale: %s\n", locale);
    } else {
              fprintf(stderr, "Can't set the specified locale! " "Check LANG, LC_CTYPE, LC_ALL.\n");
              return 1;
    }
    Glib::thread_init();
    if(!Glib::thread_supported()) {
        fprintf(stderr, "error: could not intialize Mangler: thread initialization failed\n");
        exit(0);
    }
    gdk_threads_init();
    mangler = new Mangler(&options);
    gdk_threads_enter();
    Gtk::Main::run(*mangler->manglerWindow);
    gdk_threads_leave();
}

Glib::ustring c_to_ustring(char *input) {
    int ctr;
    Glib::ustring u = "";
    Glib::ustring e = "";

    for (ctr = 0; ctr < strlen(input); ctr++) {
        u += (gunichar)((uint8_t)input[ctr]);
    }
    try {
        e = Glib::locale_to_utf8(u);
    } catch (const Glib::Error& e) {
        return u;
    }
    return e;
}

/*
Glib::ustring c_to_ustring(char *input) {
    int ctr;
    Glib::ustring u = "";
    Glib::ustring e = "";

    printf("UTF8: trying conversion\n");
    try {
        e = Glib::locale_to_utf8(input);
    } catch (const Glib::Error& error) {
        printf("UTF8: initial failed... converting manually\n");
        for (ctr = 0; ctr < strlen(input); ctr++) {
            u += (gunichar)((uint8_t)input[ctr]);
        }
        try {
            printf("UTF8: trying conversion again\n");
            e = Glib::locale_to_utf8(u);
        } catch (const Glib::Error& error) {
            printf("UTF8: second conversion failed too\n");
            return Glib::ustring(input);
        }
        return e;
    }
    return e;
}
*/
