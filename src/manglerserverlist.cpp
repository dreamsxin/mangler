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
#include "manglerserverlist.h"

#include "manglersettings.h"
#include "manglercharset.h"

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

    builder->get_widget("serverListDeleteButton", serverListServerDeleteButton);
    serverListServerDeleteButton->signal_clicked().connect(sigc::mem_fun(this, &ManglerServerList::serverListDeleteButton_clicked_cb));

    builder->get_widget("serverListCloneButton", serverListServerCloneButton);
    serverListServerCloneButton->signal_clicked().connect(sigc::mem_fun(this, &ManglerServerList::serverListCloneButton_clicked_cb));

    builder->get_widget("serverListCloseButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerServerList::serverListCloseButton_clicked_cb));

    builder->get_widget("serverListServerSaveButton", serverListServerSaveButton);
    serverListServerSaveButton->signal_clicked().connect(sigc::mem_fun(this, &ManglerServerList::serverListServerSaveButton_clicked_cb));

    builder->get_widget("serverListServerNameEntry",    serverListServerNameEntry);
    builder->get_widget("serverListHostnameEntry",      serverListHostnameEntry);
    builder->get_widget("serverListPortEntry",          serverListPortEntry);
    builder->get_widget("serverListDefaultChannelEntry",serverListDefaultChannelEntry);
    builder->get_widget("serverListUsernameEntry",      serverListUsernameEntry);
    builder->get_widget("serverListPasswordEntry",      serverListPasswordEntry);
    builder->get_widget("serverListPhoneticEntry",      serverListPhoneticEntry);
    builder->get_widget("serverListCommentEntry",       serverListCommentEntry);

    // CheckButton fields
    builder->get_widget("serverListPageCheckButton",        serverListPageCheckButton);
    builder->get_widget("serverListUtUCheckButton",         serverListUtUCheckButton);
    builder->get_widget("serverListPrivateChatCheckButton", serverListPrivateChatCheckButton);
    builder->get_widget("serverListRecordCheckButton",      serverListRecordCheckButton);
    builder->get_widget("serverListPersistentConnectionCheckButton", serverListPersistentConnectionCheckButton);
    builder->get_widget("serverListPersistentCommentsCheckButton", serverListPersistentCommentsCheckButton);

    // Charset combobox
    builder->get_widget("serverListCharsetComboBox",        serverListCharsetComboBox);
    charsetTreeModel = Gtk::ListStore::create(charsetColumns);
    serverListCharsetComboBox->set_model(charsetTreeModel);
    serverListCharsetComboBox->set_text_column(charsetColumns.name);
    for (int ctr = 0; charsetslist[ctr] != NULL; ctr++) {
        Gtk::TreeModel::Row charsetRow = *(charsetTreeModel->append());
        charsetRow[charsetColumns.name] = charsetslist[ctr];
    }
    serverListCharsetComboBox->get_entry()->set_text("");

    editorName = "";
}

void ManglerServerList::serverListWindow_show_cb(void) {
}

void ManglerServerList::serverListSelection_changed_cb(void) {
    Gtk::TreeModel::iterator iter = serverListSelection->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        editorName = Glib::locale_from_utf8(row[serverListColumns.name]);
        editRow(editorName);
        serverListServerCloneButton->set_sensitive(true);
        serverListServerDeleteButton->set_sensitive(true);
    } else {
        serverListServerCloneButton->set_sensitive(false);
        serverListServerDeleteButton->set_sensitive(false);
    }
}

void ManglerServerList::serverListDeleteButton_clicked_cb(void) {
    Gtk::TreeModel::iterator iter = serverListSelection->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring name = row[serverListColumns.name];
        Mangler::config.servers.erase(name);
        Mangler::config.servers.save();
        serverListTreeModel->erase(row);
        editorName = "";
        clearEntries();
        serverListServerNameEntry->set_sensitive(false);
        serverListHostnameEntry->set_sensitive(false);
        serverListPortEntry->set_sensitive(false);
        serverListDefaultChannelEntry->set_sensitive(false);
        serverListUsernameEntry->set_sensitive(false);
        serverListPasswordEntry->set_sensitive(false);
        serverListPhoneticEntry->set_sensitive(false);
        serverListCommentEntry->set_sensitive(false);
        serverListPageCheckButton->set_sensitive(false);
        serverListUtUCheckButton->set_sensitive(false);
        serverListPrivateChatCheckButton->set_sensitive(false);
        serverListRecordCheckButton->set_sensitive(false);
        serverListPersistentConnectionCheckButton->set_sensitive(false);
        serverListPersistentCommentsCheckButton->set_sensitive(false);
        serverListCharsetComboBox->set_sensitive(false);
        serverListServerSaveButton->set_sensitive(false);
        serverListServerCloneButton->set_sensitive(false);
        serverListServerDeleteButton->set_sensitive(false);
    }
}

void ManglerServerList::serverListCloneButton_clicked_cb(void) {
    Gtk::TreeModel::iterator curIter = serverListSelection->get_selected();
    if (! curIter) return; // should never happen
    Glib::ustring server_name = (*curIter)[serverListColumns.name];
    editorName = "";
    editRow(server_name);
}

void ManglerServerList::clearEntries(void) {
    serverListServerNameEntry->set_text("");
    serverListHostnameEntry->set_text("");
    serverListPortEntry->set_text("");
    serverListDefaultChannelEntry->set_text("");
    serverListUsernameEntry->set_text("");
    serverListPasswordEntry->set_text("");
    serverListPhoneticEntry->set_text("");
    serverListCommentEntry->set_text("");
}

void ManglerServerList::serverListAddButton_clicked_cb(void) {
    editorName = "";
    editRow("");
}

void ManglerServerList::serverListCloseButton_clicked_cb(void) {
    serverListWindow->hide();
}

void ManglerServerList::serverListServerSaveButton_clicked_cb(void) {
    saveRow();
}

void ManglerServerList::editRow(const std::string &name) {
    if (editorName.empty()) serverListSelection->unselect_all();
    serverListServerCloneButton->set_sensitive(! editorName.empty());
    serverListServerDeleteButton->set_sensitive(! editorName.empty());
    serverListServerNameEntry->grab_focus();
    serverListServerNameEntry->set_sensitive(true);
    serverListHostnameEntry->set_sensitive(true);
    serverListPortEntry->set_sensitive(true);
    serverListDefaultChannelEntry->set_sensitive(false); //Saving name from right click menu only
    serverListUsernameEntry->set_sensitive(true);
    serverListPasswordEntry->set_sensitive(true);
    serverListPhoneticEntry->set_sensitive(true);
    serverListCommentEntry->set_sensitive(true);
    serverListPageCheckButton->set_sensitive(true);
    serverListUtUCheckButton->set_sensitive(true);
    serverListPrivateChatCheckButton->set_sensitive(true);
    serverListRecordCheckButton->set_sensitive(true);
    serverListPersistentConnectionCheckButton->set_sensitive(true);
    serverListPersistentCommentsCheckButton->set_sensitive(true);
    serverListCharsetComboBox->set_sensitive(true);
    serverListServerSaveButton->set_sensitive(true);
    
    iniSection server;
    if (! name.empty()) server = Mangler::config.servers[name];
    serverListServerNameEntry->set_text(name);
    serverListHostnameEntry->set_text(server["Hostname"].toUString());
    serverListPortEntry->set_text(server["Port"].toUString());
    // TODO: get the name since i took it out of the config
    // get the whole path instead of just the name
    // maybe use something from the admin window code?
    uint32_t server_defaultchan = server["DefaultChannel"].toULong();
    serverListDefaultChannelEntry->set_text("");
    serverListUsernameEntry->set_text(server["Username"].toUString());
    serverListPasswordEntry->set_text(server["Password"].toUString());
    serverListPhoneticEntry->set_text(server["Phonetic"].toUString());
    serverListCommentEntry->set_text(server["Comment"].toUString());
    serverListPageCheckButton->set_active(server["AcceptPages"].toBool());
    serverListUtUCheckButton->set_active(server["AcceptU2U"].toBool());
    serverListPrivateChatCheckButton->set_active(server["AcceptPrivateChat"].toBool());
    serverListRecordCheckButton->set_active(server["AllowRecording"].toBool());
    serverListPersistentConnectionCheckButton->set_active(server["PersistentConnection"].toBool());
    serverListPersistentCommentsCheckButton->set_active(server["PersistentComments"].toBool());
    std::string server_charset = server["Charset"].toString();
    if (server_charset.empty()) server_charset = charsetslist[0];
    serverListCharsetComboBox->get_entry()->set_text(server_charset);
}

void ManglerServerList::saveRow() {
    Glib::ustring charset;
    Gtk::TreeModel::iterator curIter;
    if (editorName.empty()) curIter = serverListTreeModel->append();
    else curIter = serverListSelection->get_selected();
    if (! curIter) return; // should never happen
    Gtk::TreeModel::Row row = *curIter;

    if (serverListServerNameEntry->get_text().empty()) {
        mangler->errorDialog("Cannot save a server without a name");
        return;
    }
    
    Glib::ustring server_name = trim(serverListServerNameEntry->get_text());
    
    // check for duplicate
    Gtk::TreeModel::Children::iterator ckIter = serverListTreeModel->children().begin();
    while (ckIter != serverListTreeModel->children().end()) {
        if (ckIter != curIter && (*ckIter)[serverListColumns.name] == server_name) {
            mangler->errorDialog("Server names must be unique");
            return;
        }
        ckIter++;
    }
    uint32_t server_defaultchan = 0;
    // if name changed, remove old section first
    if (! editorName.empty() && server_name != editorName) {
        server_defaultchan = Mangler::config.servers[Glib::locale_from_utf8(editorName)]["DefaultChannelID"].toULong();
        Mangler::config.servers.erase(editorName);
    }
    
    // save to config
    iniSection &server( Mangler::config.servers[server_name] );
    server["Hostname"] = trim(serverListHostnameEntry->get_text());
    server["Port"] = trim(serverListPortEntry->get_text());
    if (server_defaultchan) server["DefaultChannelID"] = server_defaultchan;
    server["Username"] = trim(serverListUsernameEntry->get_text());
    server["Password"] = trim(serverListPasswordEntry->get_text());
    server["Phonetic"] = trim(serverListPhoneticEntry->get_text());
    server["Comment"] = trim(serverListCommentEntry->get_text());
    server["AcceptPages"] = serverListPageCheckButton->get_active();
    server["AcceptU2U"] = serverListUtUCheckButton->get_active();
    server["AcceptPrivateChat"] = serverListPrivateChatCheckButton->get_active();
    server["AllowRecording"] = serverListRecordCheckButton->get_active();
    server["PersistentConnection"] = serverListPersistentConnectionCheckButton->get_active();
    server["PersistentComments"] = serverListPersistentCommentsCheckButton->get_active();
    server["Charset"] = serverListCharsetComboBox->get_active_text();
    
    row[serverListColumns.name] = server_name;
    row[serverListColumns.hostname] = server["Hostname"].toUString();
    row[serverListColumns.port] = server["Port"].toUString();
    row[serverListColumns.username] = server["Username"].toUString();
    if (editorName.empty()) {
        serverListSelection = serverListView->get_selection();
        serverListSelection->select(curIter);
    }
    editorName = server_name;
    Mangler::config.servers.save();
}

Glib::ustring ManglerServerList::trim(Glib::ustring const& orig) {
    char const blankChars[] = " \t\n\r";

    Glib::ustring::size_type const first = orig.find_first_not_of(blankChars);
    return ( first==Glib::ustring::npos )
        ? Glib::ustring()
        : orig.substr(first, orig.find_last_not_of(blankChars)-first+1);
}
