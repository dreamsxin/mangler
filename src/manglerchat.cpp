/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2009-10-10 12:38:51 -0700 (Sat, 10 Oct 2009) $
 * $Revision: 63 $
 * $LastChangedBy: ekilfoil $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglersettings.h $
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
#include "manglerchat.h"

ManglerChat::ManglerChat(Glib::RefPtr<Gtk::Builder> builder) {
    builder->get_widget("chatWindow", chatWindow);
    chatWindow->signal_show().connect(sigc::mem_fun(this, &ManglerChat::chatWindow_show_cb));
    chatWindow->signal_hide().connect(sigc::mem_fun(this, &ManglerChat::chatWindow_hide_cb));

    builder->get_widget("sendChat", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerChat::chatWindowSendChat_clicked_cb));

    builder->get_widget("chatMessage", chatMessage);
    chatMessage->set_activates_default(true);

    builder->get_widget("chatUserListTreeView", chatUserListView);
    chatUserTreeModel = Gtk::ListStore::create(chatUserColumns);
    chatUserListView->set_model(chatUserTreeModel);
    chatUserListView->append_column("Name", chatUserColumns.name);

    builder->get_widget("chatBox", chatBox);
}

void ManglerChat::chatWindow_show_cb() {
    if(v3_is_loggedin()) {
        v3_join_chat();
    }
}

void ManglerChat::chatWindow_hide_cb() {
    if(v3_is_loggedin()) {
        v3_leave_chat();
    }
}

void ManglerChat::chatWindowSendChat_clicked_cb() {
    if(chatMessage->get_text_length()) {
        v3_send_chat_message((char *)ustring_to_c(chatMessage->get_text()).c_str());
        chatMessage->set_text("");
    }
}

void ManglerChat::addMessage(Glib::ustring username, Glib::ustring message) {
    Glib::RefPtr<Gtk::TextBuffer> buffer = chatBox->get_buffer();
    buffer->set_text(buffer->get_text() + "[" + username + "]: " + message + "\n");

    Gtk::TextIter end = buffer->end();
    Glib::RefPtr<Gtk::TextMark> end_mark = buffer->create_mark(end);
    chatBox->scroll_to(end_mark, 0.0);
}

void ManglerChat::addUser(uint16_t user_id) {
    fprintf(stderr, "adding user id %d\n", user_id);
    v3_user *u;
    u = v3_get_user(user_id);
    chatUserIter = chatUserTreeModel->append();
    chatUserRow = *chatUserIter;
    chatUserRow[chatUserColumns.id] = user_id;
    chatUserRow[chatUserColumns.name] = u->name;
    v3_free_user(u);
}

void ManglerChat::removeUser(uint16_t user_id) {
}
