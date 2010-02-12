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

#include "mangler.h"
#include "manglerprivchat.h"

#include "manglercharset.h"

extern const char ManglerUI[];

ManglerPrivChat::ManglerPrivChat(uint16_t userid) {
    // We instantiate a new builder object here to get a completely new window (hopefully)
    builder = Gtk::Builder::create_from_string(ManglerUI, "privChatWindow");
    builder->get_widget("privChatWindow", chatWindow);
    this->remoteUserId = userid;

    builder->get_widget("privSendChat", sendButton);
    sendButton->signal_clicked().connect(sigc::mem_fun(this, &ManglerPrivChat::chatWindowSendChat_clicked_cb));

    builder->get_widget("privChatClose", closeButton);
    closeButton->signal_clicked().connect(sigc::mem_fun(this, &ManglerPrivChat::chatWindowCloseChat_clicked_cb));

    builder->get_widget("privChatBox", chatBox);
    builder->get_widget("privChatMessage", chatMessage);

    v3_start_privchat(remoteUserId);
    chatWindow->present();
}

void ManglerPrivChat::chatWindowSendChat_clicked_cb(void) {
    if(chatMessage->get_text_length()) {
        v3_send_privchat_message(remoteUserId, (char *)ustring_to_c(chatMessage->get_text()).c_str());
        chatMessage->set_text("");
    }
}

void ManglerPrivChat::chatWindowCloseChat_clicked_cb(void) {
    v3_end_privchat(remoteUserId);
    chatWindow->hide();
}

void ManglerPrivChat::addMessage(Glib::ustring message) {
    Glib::RefPtr<Gtk::TextBuffer> buffer = chatBox->get_buffer();
    buffer->set_text(buffer->get_text() + message + "\n");

    Gtk::TextIter end = buffer->end();
    Glib::RefPtr<Gtk::TextMark> end_mark = buffer->create_mark(end);
    chatBox->scroll_to(end_mark, 0.0);
}

void ManglerPrivChat::remoteClosed() {
    addMessage("\n*** remote user closed connection");
    sendButton->set_sensitive(false);
}

void ManglerPrivChat::remoteAway() {
    addMessage("\n*** remote user is now away");
}

void ManglerPrivChat::remoteBack() {
    addMessage("\n*** remote user is back");
}

void ManglerPrivChat::remoteReopened() {
    addMessage("\n*** remote user has reopened chat window");
}

void ManglerPrivChat::addChatMessage(uint16_t id, Glib::ustring message) {
    addMessage("[" + nameFromId(id) + "]: " + message);
}

Glib::ustring ManglerPrivChat::nameFromId(uint16_t user_id) {
    v3_user *u = v3_get_user(user_id);
    Glib::ustring name = "";
    if (u) {
        name = c_to_ustring(u->name);
        v3_free_user(u);
    } else {
        name = "unknown";
    }
    return name;
}

/*
void ManglerChat::chatClose_clicked_cb() {
    chatWindow->hide();
}

void ManglerChat::addRconMessage(Glib::ustring message) {
    addMessage("[RCON]: " + message);
}

void ManglerChat::addUser(uint16_t user_id) {
    if (isUserInChat(user_id)) {
        return;
    }
    Glib::ustring name = nameFromId(user_id);
    chatUserIter = chatUserTreeModel->append();
    chatUserRow = *chatUserIter;
    chatUserRow[chatUserColumns.id] = user_id;
    chatUserRow[chatUserColumns.name] = name;
    addMessage("* " + name + " has joined the chat.");
}

void ManglerChat::removeUser(uint16_t user_id) {
    Gtk::TreeModel::Row row;
    Gtk::TreeModel::Children::iterator iter = chatUserTreeModel->children().begin();

    while (iter != chatUserTreeModel->children().end()) {
        row = *iter;
        uint32_t rowId = row[chatUserColumns.id];
        if (rowId == user_id) {
            addMessage("* " + row[chatUserColumns.name] + " has left the chat.");
            chatUserTreeModel->erase(row);
            return;
        }
        iter++;
    }
    return;
}

void ManglerChat::clear(void) {
    chatUserTreeModel->clear();
}

*/
