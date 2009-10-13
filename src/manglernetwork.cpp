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
#include "manglernetwork.h"

ManglerNetwork::ManglerNetwork(        Glib::RefPtr<Gtk::Builder>          builder) {
    this->builder = builder;
}

void
ManglerNetwork::connect(std::string hostname, std::string port, std::string username, std::string password) {/*{{{*/
    Gtk::MessageDialog *msgdialog;
    Gtk::Statusbar *statusbar;
    Gtk::ProgressBar *progressbar;
    v3_debuglevel(V3_DEBUG_ALL ^ (V3_DEBUG_PACKET_ENCRYPTED));
    std::string server = hostname + ":" + port;
    //std::string server = "tungsten.typefrag.com:29549"; std::string password = "";
    if (! v3_login((char *)server.c_str(), (char *)username.c_str(), (char *)password.c_str(), (char *)"")) {
        gdk_threads_enter();
        builder->get_widget("errorDialog", msgdialog);
        msgdialog->set_icon(mangler->icons["logo2"]);
        msgdialog->set_message(_v3_error(NULL));
        builder->get_widget("progressbar", progressbar);
        progressbar->set_fraction(0);
        progressbar->set_text("");
        builder->get_widget("statusbar", statusbar);
        statusbar->pop();
        statusbar->push("Not connected.");
        msgdialog->run();
        msgdialog->hide();
        gdk_threads_leave();
        return;
    }
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
