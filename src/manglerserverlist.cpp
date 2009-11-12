/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2009-10-01 13:25:43 -0700 (Thu, 01 Oct 2009) $
 * $Revision: 504 $
 * $LastChangedBy: ekilfoil $
 * $URL: http://svn.manglerproject.net/svn/mangler/trunk/libventrilo3/libventrilo3.h $
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
#include "manglerserverlist.h"

ManglerServerList::ManglerServerList(Glib::RefPtr<Gtk::Builder> builder) {
    serverListTreeModel = Gtk::ListStore::create(serverListColumns);
    builder->get_widget("serverListWindow", serverListWindow);
    serverListWindow->signal_show().connect(sigc::mem_fun(this, &ManglerServerList::serverListWindow_show_cb));

    builder->get_widget("serverListView", serverListView);
    Gtk::TreeView::Column *column;
    serverListView->set_model(serverListTreeModel);
    serverListView->append_column("Name", serverListColumns.name);
    column = serverListView->get_column(0);
    if (column) {
        column->set_sort_column(serverListColumns.name);
    }
    serverListView->append_column("Username", serverListColumns.username);
    column = serverListView->get_column(1);
    if (column) {
        column->set_sort_column(serverListColumns.username);
    }
    serverListView->append_column("Hostname", serverListColumns.hostname);
    column = serverListView->get_column(2);
    if (column) {
        column->set_sort_column(serverListColumns.hostname);
    }
    serverListView->append_column("Port", serverListColumns.port);
    column = serverListView->get_column(3);
    if (column) {
        column->set_sort_column(serverListColumns.port);
    }
    serverListSelection = serverListView->get_selection();
    serverListSelection->signal_changed().connect(sigc::mem_fun(this, &ManglerServerList::serverListSelection_changed_cb));

    builder->get_widget("serverListAddButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerServerList::serverListAddButton_clicked_cb));

    builder->get_widget("serverListDeleteButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerServerList::serverListDeleteButton_clicked_cb));

    builder->get_widget("serverListCloseButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerServerList::serverListCloseButton_clicked_cb));

    builder->get_widget("serverListServerSaveButton", serverListServerSaveButton);
    serverListServerSaveButton->signal_clicked().connect(sigc::mem_fun(this, &ManglerServerList::serverListServerSaveButton_clicked_cb));

    builder->get_widget("serverListServerNameEntry",    serverListServerNameEntry);
    builder->get_widget("serverListHostnameEntry",      serverListHostnameEntry);
    builder->get_widget("serverListPortEntry",          serverListPortEntry);
    builder->get_widget("serverListUsernameEntry",      serverListUsernameEntry);
    builder->get_widget("serverListPasswordEntry",      serverListPasswordEntry);
    builder->get_widget("serverListPhoneticEntry",      serverListPhoneticEntry);
    builder->get_widget("serverListCommentEntry",       serverListCommentEntry);

    // CheckButton fields
    builder->get_widget("serverListPageCheckButton",        serverListPageCheckButton);
    builder->get_widget("serverListUtUCheckButton",         serverListUtUCheckButton);
    builder->get_widget("serverListPrivateChatCheckButton", serverListPrivateChatCheckButton);
    builder->get_widget("serverListRecordCheckButton",      serverListRecordCheckButton);
    builder->get_widget("serverListPersistentCommentsCheckButton", serverListPersistentCommentsCheckButton);

    editorId = -1;
}

void ManglerServerList::serverListWindow_show_cb(void) {
}

void ManglerServerList::serverListSelection_changed_cb(void) {
    Gtk::TreeModel::iterator iter = serverListSelection->get_selected();
    if(iter) {
        Gtk::TreeModel::Row row = *iter;
        uint32_t rowId = row[serverListColumns.id];
        editRow(rowId);
    }
}

void ManglerServerList::serverListDeleteButton_clicked_cb(void) {
    uint32_t id;
    Gtk::TreeModel::iterator iter = serverListSelection->get_selected();
    if(iter) {
        Gtk::TreeModel::Row row = *iter;
        id = row[serverListColumns.id];
        serverListTreeModel->erase(row);
        mangler->settings->config.removeserver(id);
        mangler->settings->config.save();
        editorId = -1;
        clearEntries();
        serverListServerNameEntry->set_sensitive(false);
        serverListHostnameEntry->set_sensitive(false);
        serverListPortEntry->set_sensitive(false);
        serverListUsernameEntry->set_sensitive(false);
        serverListPasswordEntry->set_sensitive(false);
        serverListPhoneticEntry->set_sensitive(false);
        serverListCommentEntry->set_sensitive(false);
        serverListPageCheckButton->set_sensitive(false);
        serverListUtUCheckButton->set_sensitive(false);
        serverListPrivateChatCheckButton->set_sensitive(false);
        serverListRecordCheckButton->set_sensitive(false);
        serverListPersistentCommentsCheckButton->set_sensitive(false);
        serverListServerSaveButton->set_sensitive(false);
    }
}

void ManglerServerList::clearEntries(void) {
    serverListServerNameEntry->set_text("");
    serverListHostnameEntry->set_text("");
    serverListPortEntry->set_text("");
    serverListUsernameEntry->set_text("");
    serverListPasswordEntry->set_text("");
    serverListPhoneticEntry->set_text("");
    serverListCommentEntry->set_text("");
}

void ManglerServerList::serverListAddButton_clicked_cb(void) {
    uint32_t id;
    ManglerServerConfig *server;
    Gtk::TreeModel::Row row;

    id = mangler->settings->config.addserver();
    server = mangler->settings->config.getserver(id);
    server->name = "New Server";
    row = *(serverListTreeModel->append());
    row[serverListColumns.id] = id;
    row[serverListColumns.name] = "New Server";
    row[serverListColumns.hostname] = "";
    row[serverListColumns.port] = "";
    row[serverListColumns.username] = "";
    serverListSelection = serverListView->get_selection();
    serverListSelection->select(row);
    editRow(id);
}

void ManglerServerList::serverListCloseButton_clicked_cb(void) {
    serverListWindow->hide();
}

void ManglerServerList::serverListServerSaveButton_clicked_cb(void) {
    if (editorId >= 0) {
        saveRow();
    }
}

void ManglerServerList::editRow(uint32_t id) {
    Gtk::TreeModel::Row row;
    Gtk::TreeModel::Children::iterator iter = serverListTreeModel->children().begin();
    ManglerServerConfig *server;

    while (iter != serverListTreeModel->children().end()) {
        row = *iter;
        uint32_t rowId = row[serverListColumns.id];
        if (rowId == id) {
            break;
        }
        iter++;
    }
    serverListServerNameEntry->grab_focus();
    editorId = id;
    server = mangler->settings->config.getserver(id);
    serverListServerNameEntry->set_sensitive(true);
    serverListHostnameEntry->set_sensitive(true);
    serverListPortEntry->set_sensitive(true);
    serverListUsernameEntry->set_sensitive(true);
    serverListPasswordEntry->set_sensitive(true);
    serverListPhoneticEntry->set_sensitive(true);
    serverListCommentEntry->set_sensitive(true);
    serverListPageCheckButton->set_sensitive(true);
    serverListUtUCheckButton->set_sensitive(true);
    serverListPrivateChatCheckButton->set_sensitive(true);
    serverListRecordCheckButton->set_sensitive(true);
    serverListPersistentCommentsCheckButton->set_sensitive(true);
    serverListServerSaveButton->set_sensitive(true);

    serverListServerNameEntry->set_text(server->name);
    serverListHostnameEntry->set_text(server->hostname);
    serverListPortEntry->set_text(server->port);
    serverListUsernameEntry->set_text(server->username);
    serverListPasswordEntry->set_text(server->password);
    serverListPhoneticEntry->set_text(server->phonetic);
    serverListCommentEntry->set_text(server->comment);
    serverListPageCheckButton->set_active(server->acceptPages);
    serverListUtUCheckButton->set_active(server->acceptU2U);
    serverListPrivateChatCheckButton->set_active(server->acceptPrivateChat);
    serverListRecordCheckButton->set_active(server->allowRecording);
    serverListPersistentCommentsCheckButton->set_active(server->persistentComments);
}

void ManglerServerList::saveRow() {
    Gtk::TreeModel::Row row;
    Gtk::TreeModel::Children::iterator iter = serverListTreeModel->children().begin();
    ManglerServerConfig *server;

    while (iter != serverListTreeModel->children().end()) {
        row = *iter;
        int32_t rowId = row[serverListColumns.id];
        if (rowId == editorId) {
            break;
        }
        iter++;
    }
    server = mangler->settings->config.getserver(editorId);
    server->name = serverListServerNameEntry->get_text();
    server->hostname = serverListHostnameEntry->get_text();
    server->port = serverListPortEntry->get_text();
    server->username = serverListUsernameEntry->get_text();
    server->password = serverListPasswordEntry->get_text();
    server->phonetic = serverListPhoneticEntry->get_text();
    server->comment = serverListCommentEntry->get_text();
    server->acceptPages = serverListPageCheckButton->get_active();
    server->acceptU2U = serverListUtUCheckButton->get_active();
    server->acceptPrivateChat = serverListPrivateChatCheckButton->get_active();
    server->allowRecording = serverListRecordCheckButton->get_active();
    server->persistentComments = serverListPersistentCommentsCheckButton->get_active();

    row[serverListColumns.name] = server->name;
    row[serverListColumns.hostname] = server->hostname;
    row[serverListColumns.port] = server->port;
    row[serverListColumns.username] = server->username;
    mangler->settings->config.save();
}
