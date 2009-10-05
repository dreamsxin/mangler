/*
    * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
     */

#include "mangler.h"
#include "manglernetwork.h"

ManglerNetwork::ManglerNetwork(        Glib::RefPtr<Gtk::Builder>          builder) {
    this->builder = builder;
}

void
ManglerNetwork::connect(void) {/*{{{*/
    v3_debuglevel(V3_DEBUG_ALL);
    Gtk::ProgressBar *pb;
    //if (! v3_login((char *)"localhost:3784", (char *)"eric", (char *)"test", (char *)"")) {
    if (! v3_login((char *)"evolve.typefrag.com:54174", (char *)"eric", (char *)"mangler", (char *)"")) {
    //if (! v3_login((char *)"tungsten.typefrag.com:29549", (char *)"eric", (char *)"mangler", (char *)"")) {
        gdk_threads_enter();
        builder->get_widget("disconnectedDialog", msgdialog);
        msgdialog->set_message(_v3_error(NULL));
        v3_logout();
        msgdialog->run();
        gdk_threads_leave();
        gdk_threads_enter();
        msgdialog->hide();
        gdk_threads_leave();
        return;
    }
    v3_user *user = v3_get_user(0);
    mangler->channelTree->addChannel(user->id, 0, user->name, user->comment);
    for (int ctr = 0; ctr < 0xff; ctr++) {
        if (v3_channel *c = v3_get_channel(ctr)) {
            gdk_threads_enter();
            mangler->channelTree->addChannel(c->id, c->parent, c->name, c->comment, c->phonetic);
            gdk_threads_leave();
            v3_free_channel(c);
        }
    }
    for (int ctr = 1; ctr < 0xff; ctr++) {
        if (v3_user *u = v3_get_user(ctr)) {
            gdk_threads_enter();
            mangler->channelTree->addUser(u->id, u->channel, u->name, u->comment, u->phonetic, u->url, u->integration_text);
            gdk_threads_leave();
            v3_free_user(u);
        }
    }
    gdk_threads_enter();
    mangler->channelTree->expand_all();
    builder->get_widget("connectButton", button);
    button->set_label("gtk-disconnect");
    gdk_threads_leave();
    do {
        _v3_net_message *msg;

        if ((msg = _v3_recv(V3_BLOCK)) == NULL) {
            printf("recv() failed: %s\n", _v3_error(NULL));
            return;
        }
        switch (_v3_process_message(msg)) {
            case V3_MALFORMED:
                _v3_debug(V3_DEBUG_INFO, "received malformed packet");
                break;
            case V3_NOTIMPL:
                _v3_debug(V3_DEBUG_INFO, "packet type not implemented");
                break;
            case V3_OK:
                _v3_debug(V3_DEBUG_INFO, "packet processed");
                /*
                if (v3_queue_size() > 0) {
                    fprintf("stderr", "there's something to do\n");
                }
                */
                break;
        }
    } while(v3_is_loggedin());
    return;
}/*}}}*/

void
ManglerNetwork::disconnect(void) {
    v3_logout();
}
