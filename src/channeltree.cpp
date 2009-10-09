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
    Gtk::TreeView::Column* pColumn = Gtk::manage( new Gtk::TreeView::Column("Symbol") );
    pColumn->pack_start(channelRecord.icon, false);
    pColumn->pack_start(channelRecord.displayName);
    channelView->append_column(*pColumn);
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
ManglerChannelTree::addUser(uint32_t id, uint32_t parent_id, std::string name, std::string comment, std::string phonetic, std::string url, std::string integration_text) {/*{{{*/
    std::string displayName = "";
    Gtk::TreeModel::Row parent;
    gsize tmp;

    if (id == 0) {
        updateLobby(name, comment, phonetic);
        return;
    }
    if (! (parent = getChannel(parent_id, channelStore->children())) && id > 0) {
        fprintf(stderr, "orphaned user: id %d: %s is supposed to be in channel %d\n", id, name.c_str(), parent_id);
        return;
    }
    
    displayName = name;
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
    channelRow[channelRecord.displayName]       = g_locale_to_utf8(displayName.c_str(), -1, NULL, &tmp, NULL);
    channelRow[channelRecord.icon]              = mangler->icons["blue_circle"]->scale_simple(9, 9, Gdk::INTERP_BILINEAR);
    channelRow[channelRecord.isUser]            = id == 0 ? false : true;
    channelRow[channelRecord.id]                = id;
    channelRow[channelRecord.parent_id]         = parent_id;
    channelRow[channelRecord.name]              = name;
    channelRow[channelRecord.comment]           = comment;
    channelRow[channelRecord.phonetic]          = phonetic;
    channelRow[channelRecord.url]               = url;
    channelRow[channelRecord.integration_text]  = integration_text;
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
ManglerChannelTree::addChannel(uint32_t id, uint32_t parent_id, std::string name, std::string comment, std::string phonetic) {/*{{{*/
    std::string displayName = "";
    Gtk::TreeModel::Row parent;
    gsize tmp;

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
    channelRow[channelRecord.displayName]       = g_locale_to_utf8(displayName.c_str(), -1, NULL, &tmp, NULL);
    channelRow[channelRecord.icon]              = mangler->icons["black_circle"]->scale_simple(9, 9, Gdk::INTERP_BILINEAR);;
    channelRow[channelRecord.isUser]            = false;
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
    std::string displayName = "";
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
ManglerChannelTree::updateLobby(std::string name, std::string comment, std::string phonetic) {/*{{{*/
    std::string displayName = "";
    Gtk::TreeModel::Row lobby;
    gsize tmp;

    if (! (lobby = getChannel(0, channelStore->children()))) {
        channelIter                                 = channelStore->append();
        lobby                                       = *channelIter;
    }
    displayName = name;
    if (! comment.empty()) {
        displayName = displayName + " (" + comment + ")";
    }
    lobby[channelRecord.displayName]       = g_locale_to_utf8(displayName.c_str(), -1, NULL, &tmp, NULL);
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
ManglerChannelTree::userIsTalking(uint16_t id, bool isTalking) {
    Gtk::TreeModel::Row user = getUser(id, channelStore->children());
    if (isTalking) {
        channelRow[channelRecord.icon]              = mangler->icons["green_circle"]->scale_simple(9, 9, Gdk::INTERP_BILINEAR);
    } else {
        channelRow[channelRecord.icon]              = mangler->icons["blue_circle"]->scale_simple(9, 9, Gdk::INTERP_BILINEAR);
    }
}


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

