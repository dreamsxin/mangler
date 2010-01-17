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

#ifndef _MANGLERSERVERLIST_H
#define _MANGLERSERVERLIST_H

#include "manglerconfig.h"

class ManglerServerList {
    public:
        ManglerServerList(Glib::RefPtr<Gtk::Builder> builder);
        Gtk::Window *serverListWindow;

        class serverListModelColumns : public Gtk::TreeModel::ColumnRecord {
            public:
                serverListModelColumns() { add(id); add(name); add(hostname); add(port); add(username); }
                Gtk::TreeModelColumn<uint32_t>              id;
                Gtk::TreeModelColumn<Glib::ustring>         name;
                Gtk::TreeModelColumn<Glib::ustring>         hostname;
                Gtk::TreeModelColumn<Glib::ustring>         port;
                Gtk::TreeModelColumn<Glib::ustring>         username;
        };
        serverListModelColumns       serverListColumns;
        Glib::RefPtr<Gtk::ListStore> serverListTreeModel;
        Gtk::TreeView                *serverListView;
        Glib::RefPtr<Gtk::TreeSelection> serverListSelection;



        // Entry fields
        int32_t   editorId;
        Gtk::Entry *serverListServerNameEntry;
        Gtk::Entry *serverListHostnameEntry;
        Gtk::Entry *serverListPortEntry;
        Gtk::Entry *serverListUsernameEntry;
        Gtk::Entry *serverListPasswordEntry;
        Gtk::Entry *serverListPhoneticEntry;
        Gtk::Entry *serverListCommentEntry;

        // CheckButton fields
        Gtk::CheckButton *serverListPageCheckButton;
        Gtk::CheckButton *serverListUtUCheckButton;
        Gtk::CheckButton *serverListPrivateChatCheckButton;
        Gtk::CheckButton *serverListRecordCheckButton;
        Gtk::CheckButton *serverListPersistentConnectionCheckButton;
        Gtk::CheckButton *serverListPersistentCommentsCheckButton;

        // Character Set Combobox
        Gtk::ComboBoxEntry *serverListCharsetComboBox;
        class charsetModelColumns : public Gtk::TreeModel::ColumnRecord
        {
            public:
                charsetModelColumns() { add(name); }
                Gtk::TreeModelColumn<Glib::ustring> name;
        };
        charsetModelColumns  charsetColumns;
        Glib::RefPtr<Gtk::ListStore> charsetTreeModel;

        // Editor Buttons
        Gtk::Button *serverListServerSaveButton;

        void serverListWindow_show_cb(void);
        void serverListSelection_changed_cb(void);
        void serverListAddButton_clicked_cb(void);
        void serverListDeleteButton_clicked_cb(void);
        void serverListCloneButton_clicked_cb(void);
        void serverListSetDefaultButton_clicked_cb(void);
        void serverListCloseButton_clicked_cb(void);
        void serverListServerSaveButton_clicked_cb(void);
        void serverListServerClearButton_clicked_cb(void);

        void editRow(uint32_t id);
        void saveRow();
        void clearEntries(void);

        Glib::ustring trim(Glib::ustring const& orig);

        // generic types for builder
        Gtk::Button     *button;
};

#endif
