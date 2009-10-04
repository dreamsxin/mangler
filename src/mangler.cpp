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

Mangler::Mangler() {/*{{{*/
    // load all of our icons
    icons.insert(std::make_pair("mangler_headset_blue",         Gdk::Pixbuf::create_from_inline(-1, mangler_headset_blue)));
    icons.insert(std::make_pair("mangler_headset_blue_red",     Gdk::Pixbuf::create_from_inline(-1, mangler_headset_blue_red    )));
    icons.insert(std::make_pair("mangler_headset_green",        Gdk::Pixbuf::create_from_inline(-1, mangler_headset_green       )));
    icons.insert(std::make_pair("mangler_headset_green_red",    Gdk::Pixbuf::create_from_inline(-1, mangler_headset_green_red   )));
    icons.insert(std::make_pair("mangler_headset_grey",         Gdk::Pixbuf::create_from_inline(-1, mangler_headset_grey        )));
    icons.insert(std::make_pair("mangler_headset_grey_red",     Gdk::Pixbuf::create_from_inline(-1, mangler_headset_grey_red    )));
    icons.insert(std::make_pair("mangler_headset_red",          Gdk::Pixbuf::create_from_inline(-1, mangler_headset_red         )));
    icons.insert(std::make_pair("mangler_headset_red_red",      Gdk::Pixbuf::create_from_inline(-1, mangler_headset_red_red     )));
    icons.insert(std::make_pair("user_silent_muted",         Gdk::Pixbuf::create_from_inline(-1, user_silent_muted        )));
    icons.insert(std::make_pair("user_silent",               Gdk::Pixbuf::create_from_inline(-1, user_silent              )));
    icons.insert(std::make_pair("user_xmit_elsewhere_muted", Gdk::Pixbuf::create_from_inline(-1, user_xmit_elsewhere_muted)));
    icons.insert(std::make_pair("user_xmit_elsewhere",       Gdk::Pixbuf::create_from_inline(-1, user_xmit_elsewhere      )));
    icons.insert(std::make_pair("user_xmit_muted",           Gdk::Pixbuf::create_from_inline(-1, user_xmit_muted          )));
    icons.insert(std::make_pair("user_xmit",                 Gdk::Pixbuf::create_from_inline(-1, user_xmit                )));

    try {
        builder = Gtk::Builder::create_from_file("mangler.ui");
        //builder = Gtk::Builder::create_from_string(ManglerUI);
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
    network = new ManglerNetwork();

    // Statusbar Icon
    statusIcon = Gtk::StatusIcon::create(icons["mangler_headset_blue"]);
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
}/*}}}*/
void Mangler::connectButton_clicked_cb(void) {/*{{{*/
    fprintf(stderr, "connect button clicked\n");
    //try {
        Glib::Thread::create(sigc::mem_fun(this->network, &ManglerNetwork::connect), FALSE);
    //} catch (const Glib::Thread::Error& e) {
     //   fprintf(stderr, "thread error: %s\n", (char *)e->error);
    //}
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
    Gtk::MessageDialog *dialog;
    int ctr;

    builder->get_widget("qcServerName", textbox);
    std::string server = textbox->get_text();
    builder->get_widget("qcPort", textbox);
    std::string port = textbox->get_text();
    builder->get_widget("qcUsername", textbox);
    std::string username = textbox->get_text();
    builder->get_widget("qcPassword", textbox);
    std::string password = textbox->get_text();
    fprintf(stderr, "connecting to: %s:%s\n", server.c_str(), port.c_str());
    v3_debuglevel(V3_DEBUG_ALL);
    std::string connectserver = server + ":" + port;
    if (! v3_login((char *)connectserver.c_str(), (char *)username.c_str(), (char *)password.c_str(), (char *)"")) {
        builder->get_widget("disconnectedDialog", dialog);
        dialog->set_message(_v3_error(NULL));
        v3_logout();
        dialog->run();
        dialog->hide();
        return;
    }
    v3_debuglevel(V3_DEBUG_NONE);

    v3_user *user = v3_get_user(0);
    channelTree->addChannel(user->id, 0, user->name, user->comment);
    for (ctr = 0; ctr < 0xff; ctr++) {
        if (v3_channel *c = v3_get_channel(ctr)) {
            channelTree->addChannel(c->id, c->parent, c->name, c->comment);
            v3_free_channel(c);
        }
    }
    channelTree->expand_all();
}/*}}}*/
void Mangler::qcCancelButton_clicked_cb(void) {/*{{{*/
    fprintf(stderr, "qc cancel button clicked\n");
}/*}}}*/

void Mangler::disconnect(void) {/*{{{*/
    v3_logout();
}/*}}}*/


int
main (int argc, char *argv[])
{
    Gtk::Main kit(argc, argv);

    Glib::thread_init();
    if(!Glib::thread_supported()) {
        fprintf(stderr, "error: could not intialize Mangler: thread initialization failed\n");
        exit(0);
    }
    mangler = new Mangler();
    Gtk::Main::run(*mangler->manglerWindow);
}

