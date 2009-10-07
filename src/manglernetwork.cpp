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
    v3_debuglevel(V3_DEBUG_ALL ^ (V3_DEBUG_PACKET|V3_DEBUG_PACKET_ENCRYPTED));
    std::string server = "localhost:3784"; std::string password = "test";
    //std::string server = "tungsten.typefrag.com:29549"; std::string password = "";
    if (! v3_login((char *)server.c_str(), (char *)"eric", (char *)password.c_str(), (char *)"")) {
        return;
    }
    Glib::Thread::create(sigc::mem_fun(mangler->audio, &ManglerAudio::startOutputStream), FALSE);
    gdk_threads_enter();
    //mangler->channelTree->expand_all();
    builder->get_widget("connectButton", button);
    button->set_label("gtk-disconnect");
    builder->get_widget("serverTabLabel", label);
    label->set_label(server);
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
