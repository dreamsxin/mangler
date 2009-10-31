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

#ifndef _MANGLERSERVERLIST_H
#define _MANGLERSERVERLIST_H

#include "manglerconfig.h"

class ManglerServerList {
    public:
        class serverListModelColumns : public Gtk::TreeModel::ColumnRecord {
            public:
                serverListModelColumns() { add(id); add(server); }
                Gtk::TreeModelColumn<uint32_t>              id;
                Gtk::TreeModelColumn<Glib::ustring>         name;
                Gtk::TreeModelColumn<Glib::ustring>         hostname;
                Gtk::TreeModelColumn<Glib::ustring>         port;
                Gtk::TreeModelColumn<Glib::ustring>         username;
                Gtk::TreeModelColumn<ManglerServerConfig>   server;
        };
        serverListModelColumns       serverListColumns;
        Glib::RefPtr<Gtk::ListStore> serverListTreeModel;
        Gtk::TreeView                *serverListView;

        // Entry fields
        Gtk::Entry *serverListServerNameEntry;
        Gtk::Entry *serverListServerHostnameEntry;
        Gtk::Entry *serverListServerPortEntry;
        Gtk::Entry *serverListServerUsernameEntry;
        Gtk::Entry *serverListServerPasswordEntry;
        Gtk::Entry *serverListServerPhoneticEntry;
        Gtk::Entry *serverListServerCommentEntry;

        // CheckButton fields
        Gtk::CheckButton *serverListServerPageCheckButton;
        Gtk::CheckButton *serverListServerUtUCheckButton;
        Gtk::CheckButton *serverListServerPrivateChatCheckButton;
        Gtk::CheckButton *serverListServerRecordCheckButton;

        void serverListAddButton_clicked_cb(void);
        void serverListDeleteButton_clicked_cb(void);
        void serverListSetDefaultButton_clicked_cb(void);
        void serverListCloseButton_clicked_cb(void);
        void serverListServerSaveButton_clicked_cb(void);
        void serverListServerClearButton_clicked_cb(void);
};

#endif
