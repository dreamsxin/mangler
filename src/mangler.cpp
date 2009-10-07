/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 */

#include <gtkmm.h>
#include <iostream>
#include "mangler.h"
#include "manglerui.h"
#include "mangler-icons.h"

using namespace std;

Mangler *mangler;

Mangler::Mangler(struct _cli_options *options) {/*{{{*/
    // load all of our icons
    icons.insert(std::make_pair("mangler_headset_blue",         Gdk::Pixbuf::create_from_inline(-1, mangler_headset_blue)));
    icons.insert(std::make_pair("mangler_headset_blue_red",     Gdk::Pixbuf::create_from_inline(-1, mangler_headset_blue_red    )));
    icons.insert(std::make_pair("mangler_headset_green",        Gdk::Pixbuf::create_from_inline(-1, mangler_headset_green       )));
    icons.insert(std::make_pair("mangler_headset_green_red",    Gdk::Pixbuf::create_from_inline(-1, mangler_headset_green_red   )));
    icons.insert(std::make_pair("mangler_headset_grey",         Gdk::Pixbuf::create_from_inline(-1, mangler_headset_grey        )));
    icons.insert(std::make_pair("mangler_headset_grey_red",     Gdk::Pixbuf::create_from_inline(-1, mangler_headset_grey_red    )));
    icons.insert(std::make_pair("mangler_headset_red",          Gdk::Pixbuf::create_from_inline(-1, mangler_headset_red         )));
    icons.insert(std::make_pair("mangler_headset_red_red",      Gdk::Pixbuf::create_from_inline(-1, mangler_headset_red_red     )));
    icons.insert(std::make_pair("user_silent_muted",            Gdk::Pixbuf::create_from_inline(-1, user_silent_muted           )));
    icons.insert(std::make_pair("user_silent",                  Gdk::Pixbuf::create_from_inline(-1, user_silent                 )));
    icons.insert(std::make_pair("user_xmit_elsewhere_muted",    Gdk::Pixbuf::create_from_inline(-1, user_xmit_elsewhere_muted   )));
    icons.insert(std::make_pair("user_xmit_elsewhere",          Gdk::Pixbuf::create_from_inline(-1, user_xmit_elsewhere         )));
    icons.insert(std::make_pair("user_xmit_muted",              Gdk::Pixbuf::create_from_inline(-1, user_xmit_muted             )));
    icons.insert(std::make_pair("user_xmit",                    Gdk::Pixbuf::create_from_inline(-1, user_xmit                   )));

    icons.insert(std::make_pair("black_circle",                 Gdk::Pixbuf::create_from_inline(-1, black_circle                )));
    icons.insert(std::make_pair("blue_circle",                  Gdk::Pixbuf::create_from_inline(-1, blue_circle                 )));
    icons.insert(std::make_pair("blue_circle_small",            Gdk::Pixbuf::create_from_inline(-1, blue_circle_small           )));
    icons.insert(std::make_pair("cyan_circle",                  Gdk::Pixbuf::create_from_inline(-1, cyan_circle                 )));
    icons.insert(std::make_pair("green_circle",                 Gdk::Pixbuf::create_from_inline(-1, green_circle                )));
    icons.insert(std::make_pair("grey_circle",                  Gdk::Pixbuf::create_from_inline(-1, grey_circle                 )));
    icons.insert(std::make_pair("purple_circle",                Gdk::Pixbuf::create_from_inline(-1, purple_circle               )));
    icons.insert(std::make_pair("red_circle",                   Gdk::Pixbuf::create_from_inline(-1, red_circle                  )));
    icons.insert(std::make_pair("yellow_circle",                Gdk::Pixbuf::create_from_inline(-1, yellow_circle               )));


    try {
        if (options->uifromfile) {
            builder = Gtk::Builder::create_from_file(options->uifilename);
        } else {
            builder = Gtk::Builder::create_from_string(ManglerUI);
        }
        builder->get_widget("manglerWindow", manglerWindow);
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

    // Create Channel Tree
    channelTree = new ManglerChannelTree(builder);

    // Create Network Communication Object
    network = new ManglerNetwork(builder);

    // Create audio object
    audio = new ManglerAudio();

    // Statusbar Icon
    statusIcon = Gtk::StatusIcon::create(icons["blue_circle"]);
}/*}}}*/

/*
 * Signal handler callbacks
 */
void Mangler::quickConnectButton_clicked_cb(void) {/*{{{*/
    fprintf(stderr, "quick connect button clicked\n");
    Gtk::Dialog *dialog;
    builder->get_widget("quickConnectDialog", dialog);
    dialog->run();
    dialog->hide();

}/*}}}*/
void Mangler::serverConfigButton_clicked_cb(void) {/*{{{*/
    fprintf(stderr, "server button clicked\n");
    builder->get_widget("serverListWindow", window);
    window->show();
}/*}}}*/
void Mangler::connectButton_clicked_cb(void) {/*{{{*/
    builder->get_widget("connectButton", button);
    channelTree->updateLobby("Connecting...");
    if (button->get_label() == "gtk-connect") {
        Glib::Thread::create(sigc::bind(sigc::mem_fun(this->network, &ManglerNetwork::connect), "localhost", "3784", "username", "password"), FALSE);
        Glib::signal_timeout().connect( sigc::mem_fun(*this, &Mangler::getNetworkEvent), 50 );
    } else {
        v3_logout();
        button->set_label("gtk-connect");
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
    fprintf(stderr, "settings button clicked\n");
    builder->get_widget("settingsWindow", window);
    window->show();
}/*}}}*/
void Mangler::aboutButton_clicked_cb(void) {/*{{{*/
    builder->get_widget("aboutWindow", dialog);
    dialog->run();
    dialog->hide();
}/*}}}*/
void Mangler::xmitButton_pressed_cb(void) {/*{{{*/
    fprintf(stderr, "xmit button pressed\n");
}/*}}}*/
void Mangler::xmitButton_released_cb(void) {/*{{{*/
    fprintf(stderr, "xmit button released\n");
}/*}}}*/

// Quick Connect callbacks
void Mangler::qcConnectButton_clicked_cb(void) {/*{{{*/
    fprintf(stderr, "qc connect button clicked\n");
    Gtk::Entry *textbox;

    builder->get_widget("qcServerName", textbox);
    std::string server = textbox->get_text();
    builder->get_widget("qcPort", textbox);
    std::string port = textbox->get_text();
    builder->get_widget("qcUsername", textbox);
    std::string username = textbox->get_text();
    builder->get_widget("qcPassword", textbox);
    std::string password = textbox->get_text();
    fprintf(stderr, "connecting to: %s:%s\n", server.c_str(), port.c_str());
    Glib::Thread::create(sigc::bind(sigc::mem_fun(this->network, &ManglerNetwork::connect), server, port, username, password), FALSE);
    Glib::signal_timeout().connect( sigc::mem_fun(*this, &Mangler::getNetworkEvent), 50 );
}/*}}}*/
void Mangler::qcCancelButton_clicked_cb(void) {/*{{{*/
    fprintf(stderr, "qc cancel button clicked\n");
}/*}}}*/

bool
Mangler::getNetworkEvent() {/*{{{*/
    v3_event *ev;

    while ((ev = v3_get_event(V3_NONBLOCK)) != NULL) {
        v3_user *u;
        v3_channel *c;
        gdk_threads_enter();
        switch (ev->type) {
            case V3_EVENT_PING:
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
                break;
            case V3_EVENT_STATUS:
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
                break;
            case V3_EVENT_USER_LOGIN:
                u = v3_get_user(ev->user.id);
                fprintf(stderr, "adding user id %d: %s\n", ev->user.id, u->name);
                channelTree->addUser((uint32_t)u->id, (uint32_t)u->channel, u->name, u->comment, u->phonetic, u->url, u->integration_text);
                v3_free_user(u);
                break;
            case V3_EVENT_CHAN_REMOVE:
                // can't get any channel info... it's already gone by this point
                fprintf(stderr, "removing channel id %d\n", ev->channel.id);
                channelTree->removeChannel(ev->channel.id);
                break;
            case V3_EVENT_USER_LOGOUT:
                // can't get any user info... it's already gone by this point
                fprintf(stderr, "removing user id %d\n", ev->user.id);
                channelTree->removeUser(ev->user.id);
                break;
            case V3_EVENT_LOGIN_COMPLETE:
                channelTree->expand_all();
                break;
            case V3_EVENT_USER_CHAN_MOVE:
                u = v3_get_user(ev->user.id);
                fprintf(stderr, "moving user id %d to channel id %d\n", ev->user.id, ev->channel.id);
                channelTree->removeUser((uint32_t)ev->user.id);
                channelTree->addUser((uint32_t)u->id, (uint32_t)ev->channel.id, u->name, u->comment, u->phonetic, u->url, u->integration_text);
                v3_free_user(u);
                break;
            case V3_EVENT_CHAN_ADD:
                c = v3_get_channel(ev->channel.id);
                fprintf(stderr, "adding channel id %d: %s\n", ev->channel.id, c->name);
                channelTree->addChannel((uint32_t)c->id, (uint32_t)c->parent, c->name, c->comment, c->phonetic);
                v3_free_channel(c);
                break;
            case V3_EVENT_ERROR_MSG:
                builder->get_widget("errorDialog", msgdialog);
                msgdialog->set_message(ev->error.message);
                msgdialog->run();
                msgdialog->hide();
                break;
            default:
                fprintf(stderr, "******************************************************** got unknown event type %d\n", ev->type);
        }
        channelTree->expand_all();
        gdk_threads_leave();
        free(ev);
    }
    return true;
}/*}}}*/


ManglerError::ManglerError(uint32_t code, std::string message, std::string module) {
    this->code = code;
    this->message = message;
    this->module = module;
}

int
main (int argc, char *argv[])
{
    Gtk::Main kit(argc, argv);
    struct _cli_options options;

    // TODO: use getopt()
    if (argc > 1) {
        options.uifilename = argv[1];
        options.uifromfile = true;
        fprintf(stderr, "using ui file: %s\n", options.uifilename.c_str());
    } else {
        options.uifromfile = false;
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

