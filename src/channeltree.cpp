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

#include "iostream"
#include "mangler.h"
#include "channeltree.h"
#include "time.h"

using namespace std;

ManglerChannelTree::ManglerChannelTree(Glib::RefPtr<Gtk::Builder> builder)/*{{{*/
{
    this->builder = builder;
    // Create the Channel Store
    channelStore = Gtk::TreeStore::create(channelRecord);

    // Create the Channel View
    builder->get_widget("channelView", channelView);
    channelView->set_model(channelStore);

    //channelView->append_column("ID", channelRecord.id);
    Gtk::TreeView::Column* pColumn = Gtk::manage( new Gtk::TreeView::Column("Name") );
    pColumn->pack_start(channelRecord.icon, false);
    pColumn->pack_start(channelRecord.displayName);
    pColumn->set_expand(true);
    channelView->append_column(*pColumn);
    channelView->append_column("Last Transmit", channelRecord.last_transmit);

    // connect our callbacks for clicking on rows
    channelView->signal_row_activated().connect(sigc::mem_fun(this, &ManglerChannelTree::channelView_row_activated_cb));


    //int colnum = channelView->append_column("Name", channelRecord.displayName) - 1;
    // TODO: Write a sort routine to make sure users are always immediately
    // below the channel, otherwise users get sorted within the subchannels
    //channelStore->set_sort_column(channelRecord.displayName, Gtk::SORT_ASCENDING);
    /*
    channelView->get_column(colnum)->set_cell_data_func(
                *channelView->get_column_cell_renderer(colnum),
                sigc::mem_fun(*this, &ManglerChannelTree::renderCellData)
                );
     */

    /*
     * We have to finish off our user settings window.  I can't find a way to
     * do this in builder, so let's pack our volume adjustment manually
     */
    volumeAdjustment = new Gtk::Adjustment(79, 0, 148, 1, 10, 10);
    volumevscale = new Gtk::VScale(*volumeAdjustment);
    volumevscale->add_mark(79, Gtk::POS_LEFT, "100%");
    volumevscale->set_inverted(true);
    volumevscale->set_draw_value(false);
    builder->get_widget("userSettingsVolumeAdjustVBox", vbox);
    vbox->pack_start(*volumevscale);
}/*}}}*/

/*
 *  The GTK cell renederer for the channel tree view
 *  
 *  This function sets the colors, weight, etc for each row in the tree
 */
void
ManglerChannelTree::renderCellData(Gtk::CellRenderer *cell, const Gtk::TreeModel::iterator& iter) {/*{{{*/
    Gtk::CellRendererText *crt = dynamic_cast<Gtk::CellRendererText*>(cell);
    if(iter) {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring value = row[channelRecord.displayName];
        bool isUser = row[channelRecord.isUser];
        if (!isUser) {
            crt->property_scale() = 1;
            crt->property_style() = Pango::STYLE_NORMAL;
            crt->property_weight() = 600;
            //crt->property_cell_background() = "#dfdfdf";
        } else if (isUser) {
            crt->property_scale() = 1;
            crt->property_style() = Pango::STYLE_NORMAL;
            crt->property_weight() = 400;
            //crt->property_cell_background() = "#ffffff";
        }
        crt->property_text() = value;
    }
}/*}}}*/

/*
 * Add a user to the channel tree
 *
 * id                       user's ventrilo id
 * parent_id                the channel id of the channel the user is in
 * name                     the user name
 * comment = ""
 * phonetic = ""
 * url = ""
 * integration_text = ""     
 *
 * this calculates the display name automatically
 */
void
ManglerChannelTree::addUser(uint32_t id, uint32_t parent_id, Glib::ustring name, Glib::ustring comment, Glib::ustring phonetic, Glib::ustring url, Glib::ustring integration_text, bool guest) {/*{{{*/
    Glib::ustring displayName = "";
    Gtk::TreeModel::Row parent;

    if (id == 0) {
        updateLobby(name, comment, phonetic);
        return;
    }
    if (! (parent = getChannel(parent_id, channelStore->children())) && id > 0) {
        fprintf(stderr, "orphaned user: id %d: %s is supposed to be in channel %d\n", id, name.c_str(), parent_id);
        return;
    }
    
    displayName = name;
    if (guest) {
        displayName = displayName + " (GUEST)";
    }
    if (mangler->chat->isUserInChat(id)) {
        displayName = "[C] " + displayName;
    }
    if (! comment.empty()) {
        displayName = displayName + " (" + (url.empty() ? "" : "U: ") + comment + ")";
    }
    if (! integration_text.empty()) {
        displayName = displayName + " {" + integration_text + "}";
    }
    if (parent) {
        channelIter                                 = channelStore->prepend(parent.children());
    } else {
        channelIter                                 = channelStore->prepend();
    }
    channelRow                                  = *channelIter;
    channelRow[channelRecord.displayName]       = displayName;
    channelRow[channelRecord.icon]              = mangler->icons["user_icon_noxmit"]->scale_simple(15, 15, Gdk::INTERP_BILINEAR);
    channelRow[channelRecord.isUser]            = id == 0 ? false : true;
    channelRow[channelRecord.isGuest]           = guest;
    channelRow[channelRecord.id]                = id;
    channelRow[channelRecord.parent_id]         = parent_id;
    channelRow[channelRecord.name]              = name;
    channelRow[channelRecord.comment]           = comment;
    channelRow[channelRecord.phonetic]          = phonetic;
    channelRow[channelRecord.url]               = url;
    channelRow[channelRecord.integration_text]  = integration_text;
    channelRow[channelRecord.last_transmit]     = id != 0 ? "unknown" : "";
}/*}}}*/

/*
 * Update a user in the channel tree
 *
 * id                       user's ventrilo id
 * parent_id                the channel id of the channel the user is in
 * name                     the user name
 * comment = ""
 * phonetic = ""
 * url = ""
 * integration_text = ""     
 *
 * this calculates the display name automatically
 */
void
ManglerChannelTree::updateUser(uint32_t id, uint32_t parent_id, Glib::ustring name, Glib::ustring comment, Glib::ustring phonetic, Glib::ustring url, Glib::ustring integration_text, bool guest) {/*{{{*/
    Glib::ustring displayName = "";
    Gtk::TreeModel::Row user;

    if (id == 0) {
        updateLobby(name, comment, phonetic);
        return;
    }
    if (! (user = getUser(id, channelStore->children())) && id > 0) {
        fprintf(stderr, "missing user: id %d: %s is supposed to be in channel %d\n", id, name.c_str(), parent_id);
        return;
    }
    
    displayName = name;
    if (guest) {
        displayName = displayName + " (GUEST)";
    }
    if (mangler->chat->isUserInChat(id)) {
        displayName = "[C] " + displayName;
    }
    if (! comment.empty()) {
        displayName = displayName + " (" + (url.empty() ? "" : "U: ") + comment + ")";
    }
    if (! integration_text.empty()) {
        displayName = displayName + " {" + integration_text + "}";
    }
    user[channelRecord.displayName]       = displayName;
    user[channelRecord.icon]              = mangler->icons["user_icon_noxmit"]->scale_simple(15, 15, Gdk::INTERP_BILINEAR);
    user[channelRecord.isUser]            = id == 0 ? false : true;
    user[channelRecord.isGuest]           = guest;
    user[channelRecord.id]                = id;
    user[channelRecord.parent_id]         = parent_id;
    user[channelRecord.name]              = name;
    user[channelRecord.comment]           = comment;
    user[channelRecord.phonetic]          = phonetic;
    user[channelRecord.url]               = url;
    user[channelRecord.integration_text]  = integration_text;
    user[channelRecord.last_transmit]     = id != 0 ? "unknown" : "";
}/*}}}*/

/*
 * Add a channel to the channel tree
 *
 * id                       user's ventrilo id
 * parent_id                the channel id of the channel the user is in
 * name                     the user name
 * comment = ""
 * phonetic = ""
 */
void
ManglerChannelTree::addChannel(uint8_t protect_mode, uint32_t id, uint32_t parent_id, Glib::ustring name, Glib::ustring comment, Glib::ustring phonetic) {/*{{{*/
    Glib::ustring displayName = "";
    Gtk::TreeModel::Row parent;

    if (! (parent = getChannel(parent_id, channelStore->children())) && id > 0) {
        fprintf(stderr, "orphaned channel: id %d: %s is supposed to be a child of %d\n", id, name.c_str(), parent_id);
    }
    displayName = name;
    if (! comment.empty()) {
        displayName = displayName + " (" + comment + ")";
    }
    //displayName = "<span weight=\"bold\">" + displayName + "</span>";
    if (parent) {
        channelIter                                 = channelStore->append(parent.children());
    } else {
        channelIter                                 = channelStore->append();
    }
    channelRow                                  = *channelIter;
    channelRow[channelRecord.displayName]       = displayName;
    switch (protect_mode) {
        case 0:
            channelRow[channelRecord.icon]              = mangler->icons["black_circle"]->scale_simple(9, 9, Gdk::INTERP_BILINEAR);;
            break;
        case 1:
            channelRow[channelRecord.icon]              = mangler->icons["red_circle"]->scale_simple(9, 9, Gdk::INTERP_BILINEAR);;
            break;
        case 2:
            channelRow[channelRecord.icon]              = mangler->icons["yellow_circle"]->scale_simple(9, 9, Gdk::INTERP_BILINEAR);;
            break;
    }
    channelRow[channelRecord.isUser]            = false;
    channelRow[channelRecord.isGuest]           = false;
    channelRow[channelRecord.id]                = id;
    channelRow[channelRecord.parent_id]         = parent_id;
    channelRow[channelRecord.name]              = name;
    channelRow[channelRecord.comment]           = comment;
    channelRow[channelRecord.phonetic]          = phonetic;
    channelRow[channelRecord.url]               = "";
    channelRow[channelRecord.integration_text]  = "";
}/*}}}*/

/*
 * Remove a user from the channel tree
 *
 * id                       user's ventrilo id
 */
void
ManglerChannelTree::removeUser(uint32_t id) {/*{{{*/
    Glib::ustring displayName = "";
    Gtk::TreeModel::Row user;

    if (! (user = getUser(id, channelStore->children())) && id > 0) {
        fprintf(stderr, "could not find user id %d to delete\n", id);
        return;
    }
    channelStore->erase(user);
}/*}}}*/

/*
 * Remove a channel from the channel tree
 *
 * id                       channel's ventrilo id
 */
void
ManglerChannelTree::removeChannel(uint32_t id) {/*{{{*/
    Gtk::TreeModel::Row channel;

    if (! (channel = getChannel(id, channelStore->children())) && id > 0) {
        fprintf(stderr, "could not find channel id %d to delete\n", id);
        return;
    }
    channelStore->erase(channel);
}/*}}}*/

// Recursively search the channel store for a specific channel id and returns the row
Gtk::TreeModel::Row
ManglerChannelTree::getChannel(uint32_t id, Gtk::TreeModel::Children children) {/*{{{*/
    Gtk::TreeModel::Children::iterator iter = children.begin();
    //std::cerr << "looking for channel id : " << id  << endl;
    while (iter != children.end()) {
        Gtk::TreeModel::Row row = *iter;
        uint32_t rowId = row[channelRecord.id];
        bool isUser = row[channelRecord.isUser];
        //std::cerr << "iterating: " << rowId << " | isUser: " << isUser << " | name: " << row[channelRecord.name] << endl;
        if (rowId == id && isUser == false) {
            //std::cerr << "found it" << endl;
            return row;
        }
        if (row.children().size()) {
            //std::cerr << "looking through children" << endl;
            if (row = getChannel(id, row->children())) {
                //std::cerr << "found it in a child" << endl;
                return row;
            }
        }
        iter++;
    }
    return *iter;
}/*}}}*/

// Recursively search the channel store for a specific user id and returns the row
Gtk::TreeModel::Row
ManglerChannelTree::getUser(uint32_t id, Gtk::TreeModel::Children children) {/*{{{*/
    Gtk::TreeModel::Children::iterator iter = children.begin();
    //std::cerr << "looking for user id : " << id  << endl;
    while (iter != children.end()) {
        Gtk::TreeModel::Row row = *iter;
        uint32_t rowId = row[channelRecord.id];
        bool isUser = row[channelRecord.isUser];
        //std::cerr << "iterating: " << rowId << " | isUser: " << isUser << " | name: " << row[channelRecord.name] << endl;
        if (rowId == id && isUser == true) {
            //std::cerr << "found it" << endl;
            return row;
        }
        if (row.children().size()) {
            //std::cerr << "looking through children" << endl;
            if (row = getUser(id, row->children())) {
                //std::cerr << "found it in a child" << endl;
                return row;
            }
        }
        iter++;
    }
    return *iter;
}/*}}}*/

void
ManglerChannelTree::updateLobby(Glib::ustring name, Glib::ustring comment, Glib::ustring phonetic) {/*{{{*/
    Glib::ustring displayName = "";
    Gtk::TreeModel::Row lobby;

    if (! (lobby = getChannel(0, channelStore->children()))) {
        channelIter                                 = channelStore->append();
        lobby                                       = *channelIter;
    }
    displayName = name;
    if (! comment.empty()) {
        displayName = displayName + " (" + comment + ")";
    }
    lobby[channelRecord.displayName]       = displayName;
    lobby[channelRecord.isUser]            = false;
    lobby[channelRecord.id]                = 0;
    lobby[channelRecord.parent_id]         = 0;
    lobby[channelRecord.name]              = name;
    lobby[channelRecord.comment]           = comment;
    lobby[channelRecord.phonetic]          = phonetic;
    lobby[channelRecord.url]               = "";
    lobby[channelRecord.integration_text]  = "";
}/*}}}*/

void
ManglerChannelTree::userIsTalking(uint16_t id, bool isTalking) {/*{{{*/
    Gtk::TreeModel::Row user = getUser(id, channelStore->children());
    Gtk::TreeModel::Row me   = getUser(v3_get_user_id(), channelStore->children());
    if (isTalking) {
        if (me[channelRecord.parent_id] == user[channelRecord.parent_id]) {
            user[channelRecord.icon]              = mangler->icons["user_icon_xmit"]->scale_simple(15, 15, Gdk::INTERP_BILINEAR);
        } else {
            user[channelRecord.icon]              = mangler->icons["user_icon_xmit_otherroom"]->scale_simple(15, 15, Gdk::INTERP_BILINEAR);
        }
        user[channelRecord.last_transmit]     = getTimeString();
    } else {
        user[channelRecord.icon]              = mangler->icons["user_icon_noxmit"]->scale_simple(15, 15, Gdk::INTERP_BILINEAR);
    }
}/*}}}*/

bool
ManglerChannelTree::expand_all(void) {/*{{{*/
    channelView->expand_all();
    channelView->show_all();
    return false;
}/*}}}*/

bool
ManglerChannelTree::collapse_all(void) {/*{{{*/
    channelView->collapse_all();
    return false;
}/*}}}*/

void
ManglerChannelTree::clear(void) {/*{{{*/
    channelStore->clear();
}/*}}}*/

void
ManglerChannelTree::channelView_row_activated_cb(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column) {/*{{{*/
    v3_channel *channel;
    Glib::ustring password;
    bool password_required = false;

    Gtk::TreeModel::iterator iter = channelStore->get_iter(path);
    Gtk::TreeModel::Row row = *iter;
    int id = row[channelRecord.id];
    bool isUser = row[channelRecord.isUser];
    if (isUser) {
        // double clicked a user
        if (id == v3_get_user_id()) {
            // clicked on ourself
        } else {
            Glib::ustring name = row[channelRecord.name];

            // disconnect whatever was connected before and reconnect
            volumeAdjustSignalConnection.disconnect();
            volumeAdjustSignalConnection = volumeAdjustment->signal_value_changed().connect(sigc::bind(sigc::mem_fun(this, &ManglerChannelTree::volumeAdjustment_value_changed_cb), id));

            // set the user name
            builder->get_widget("userSettingsNameValueLabel", label);
            label->set_text(name);

            // set the current volume level for this user
            volumeAdjustment->set_value(v3_get_volume_user(id));

            builder->get_widget("userSettingsWindow", window);
            window->show_all();
            window->present();
        }
    } else {
        // double clicked a channel
        Gtk::TreeModel::Row user = getUser(v3_get_user_id(), channelStore->children());
        int curchannel = user[channelRecord.parent_id];
        if (id == curchannel) {
            // we're already in this channel
            return;
        }
        if (id != 0) {
            channel = v3_get_channel(id);
            if (! channel) {
                fprintf(stderr, "failed to retrieve channel information for channel id %d", id);
                return;
            }
            if (v3_channel_requires_password(channel->id)) {  // Channel is password protected
                password = mangler->getPasswordEntry("Channel Password");
                password_required = true;
            }
            v3_free_channel(channel);
        }
        if (password_required && password.empty()) {
            return;
        }
        v3_change_channel(id, (char *)password.c_str());
    }
}/*}}}*/

uint16_t
ManglerChannelTree::getUserChannelId(uint16_t userid) {/*{{{*/
    Gtk::TreeModel::Row user = getUser(userid, channelStore->children());
    return(user[channelRecord.parent_id]);
}/*}}}*/

void
ManglerChannelTree::volumeAdjustment_value_changed_cb(uint16_t id) {
    v3_set_volume_user(id, volumeAdjustment->get_value());
}

Glib::ustring getTimeString(void) {
    char buf[64];
    time_t t;
    struct tm *tmp;
    Glib::ustring cppbuf;

    t = time(NULL);
    tmp = localtime(&t);
    if (tmp == NULL) {
        strcpy(buf, "");
    } else {
        if (strftime(buf, sizeof(buf), "%X", tmp) == 0) {
            strcpy(buf, "");
        }
    }
    cppbuf = buf;
    return cppbuf;
}

