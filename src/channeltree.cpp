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

#include "iostream"
#include "mangler.h"
#include "channeltree.h"
#include "time.h"

using namespace std;

ManglerChannelTree::ManglerChannelTree(Glib::RefPtr<Gtk::Builder> builder)/*{{{*/
{
    this->builder = builder;
    // Create the Channel Store
    channelStore = ManglerChannelStore::create();

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
    channelView->signal_button_press_event().connect_notify(sigc::mem_fun(this, &ManglerChannelTree::channelView_buttonpress_event_cb));

    // setup drag and drop 
    channelView->enable_model_drag_source();
    channelView->enable_model_drag_dest();

    // create our right click context menu for users and connect its signal
    builder->get_widget("userRightClickMenu", rcmenu_user);
    builder->get_widget("userSettings", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &ManglerChannelTree::userSettingsMenuItem_activate_cb));
    builder->get_widget("copyComment", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &ManglerChannelTree::copyCommentMenuItem_activate_cb));
    builder->get_widget("privateChat", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &ManglerChannelTree::privateChatMenuItem_activate_cb));
    builder->get_widget("copyURL", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &ManglerChannelTree::copyURLMenuItem_activate_cb));
    builder->get_widget("kickUser", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &ManglerChannelTree::kickUserMenuItem_activate_cb));
    builder->get_widget("banUser", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &ManglerChannelTree::banUserMenuItem_activate_cb));
    builder->get_widget("muteUser", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &ManglerChannelTree::muteUserMenuItem_activate_cb));
    builder->get_widget("muteUserGlobal", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &ManglerChannelTree::muteUserGlobalMenuItem_activate_cb));

    builder->get_widget("channelRightClickMenu", rcmenu_channel);
    builder->get_widget("addPhantom", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &ManglerChannelTree::addPhantomMenuItem_activate_cb));
    builder->get_widget("removePhantom", menuitem);
    menuitem->signal_activate().connect(sigc::mem_fun(this, &ManglerChannelTree::removePhantomMenuItem_activate_cb));

    //int colnum = channelView->append_column("Name", channelRecord.displayName) - 1;
    // TODO: Write a sort routine to make sure users are always immediately
    // below the channel, otherwise users get sorted within the subchannels
    channelStore->set_sort_column(channelRecord.name, Gtk::SORT_ASCENDING);
    /*
    channelView->get_column(colnum)->set_cell_data_func(
                *channelView->get_column_cell_renderer(colnum),
                sigc::mem_fun(*this, &ManglerChannelTree::renderCellData)
                );
     */
    channelStore->set_sort_func(channelRecord.name, sigc::mem_fun (*this, &ManglerChannelTree::on_sort_compare));

    /*
     * We have to finish off our user settings window.  I can't find a way to
     * do this in builder, so let's pack our volume adjustment manually
     */
    volumeAdjustment = new Gtk::Adjustment(79, 0, 148, 1, 10, 10);
    volumevscale = new Gtk::VScale(*volumeAdjustment);
    volumevscale->add_mark(138, Gtk::POS_LEFT, "200%");
    volumevscale->add_mark(79, Gtk::POS_LEFT, "100%");
    volumevscale->add_mark(0, Gtk::POS_LEFT, "0%");
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
ManglerChannelTree::addUser(uint32_t id, uint32_t parent_id, Glib::ustring name, Glib::ustring comment, Glib::ustring phonetic, Glib::ustring url, Glib::ustring integration_text, bool guest, bool phantom, Glib::ustring rank) {/*{{{*/
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
    if (!rank.empty()) {
        displayName = "[" + rank + "] " + displayName; 
    }
    if (phantom) {
        displayName = "\"P\" " + displayName; 
    }
    if (guest && !mangler->settings->config.guestFlagHidden) {
        displayName = displayName + " (GUEST)";
    }
    if (mangler->chat->isUserInChat(id)) {
        displayName = "[C] " + displayName;
    }
    if (! comment.empty()) {
        displayName = displayName + " (" + (url.empty() ? "" : "U: ") + comment + ")";
    } else if (comment.empty() && !url.empty()) {
        displayName = displayName + " (" + (url.empty() ? "" : "U: ") + url + ")";
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
    channelRow[channelRecord.icon]              = mangler->icons["user_icon_red"]->scale_simple(15, 15, Gdk::INTERP_BILINEAR);
    channelRow[channelRecord.isUser]            = id == 0 ? false : true;
    channelRow[channelRecord.isGuest]           = guest;
    channelRow[channelRecord.id]                = id;
    channelRow[channelRecord.parent_id]         = parent_id;
    channelRow[channelRecord.name]              = name;
    channelRow[channelRecord.comment]           = comment;
    channelRow[channelRecord.phonetic]          = phonetic;
    channelRow[channelRecord.url]               = url;
    channelRow[channelRecord.integration_text]  = integration_text;
    channelRow[channelRecord.rank]              = rank;
    channelRow[channelRecord.last_transmit]     = id != 0 ? "unknown" : "";
    channelRow[channelRecord.password]          = "";
    channelRow[channelRecord.phantom]           = phantom;
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
ManglerChannelTree::updateUser(uint32_t id, uint32_t parent_id, Glib::ustring name, Glib::ustring comment, Glib::ustring phonetic, Glib::ustring url, Glib::ustring integration_text, bool guest, bool phantom, Glib::ustring rank) {/*{{{*/
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
    if (!rank.empty()) {
        displayName = "[" + rank + "] " + displayName; 
    }
    if (phantom) {
        displayName = "\"P\" " + displayName; 
    }
    if (guest && !mangler->settings->config.guestFlagHidden) {
        displayName = displayName + " (GUEST)";
    }
    if (mangler->chat->isUserInChat(id)) {
        displayName = "[C] " + displayName;
    }
    if (! comment.empty()) {
        displayName = displayName + " (" + (url.empty() ? "" : "U: ") + comment + ")";
    } else if (comment.empty() && !url.empty()) {
        displayName = displayName + " (" + (url.empty() ? "" : "U: ") + url + ")";
    }

    if (! integration_text.empty()) {
        displayName = displayName + " {" + integration_text + "}";
    }
    user[channelRecord.displayName]       = displayName;
    user[channelRecord.icon]              = mangler->icons["user_icon_red"]->scale_simple(15, 15, Gdk::INTERP_BILINEAR);
    user[channelRecord.isUser]            = id == 0 ? false : true;
    user[channelRecord.isGuest]           = guest;
    user[channelRecord.id]                = id;
    user[channelRecord.parent_id]         = parent_id;
    user[channelRecord.name]              = name;
    user[channelRecord.comment]           = comment;
    user[channelRecord.phonetic]          = phonetic;
    user[channelRecord.url]               = url;
    user[channelRecord.integration_text]  = integration_text;
    user[channelRecord.rank]              = rank;
    user[channelRecord.phantom]           = phantom;
}/*}}}*/

/*
 * Add a channel to the channel tree
 *
 * id                       user's ventrilo id
 * parent_id                the channel id of the parent channel
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
    if (v3_is_channel_admin(id)) {
        displayName = "[A] " + displayName;
    }
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
            channelRow[channelRecord.icon]              = mangler->icons["black_circle"]->scale_simple(12, 12, Gdk::INTERP_BILINEAR);;
            break;
        case 1:
            channelRow[channelRecord.icon]              = mangler->icons["red_circle"]->scale_simple(12, 12, Gdk::INTERP_BILINEAR);;
            break;
        case 2:
            channelRow[channelRecord.icon]              = mangler->icons["yellow_circle"]->scale_simple(12, 12, Gdk::INTERP_BILINEAR);;
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
    channelRow[channelRecord.rank]              = "";
    channelRow[channelRecord.password]          = "";
}/*}}}*/

/*
 * Add a channel to the channel tree
 *
 * id                       user's ventrilo id
 * parent_id                the channel id of the parent channel
 * name                     the user name
 * comment = ""
 * phonetic = ""
 */
void
ManglerChannelTree::updateChannel(uint8_t protect_mode, uint32_t id, uint32_t parent_id, Glib::ustring name, Glib::ustring comment, Glib::ustring phonetic) {/*{{{*/
    Glib::ustring displayName = "";
    Gtk::TreeModel::Row channel;

    if (! (channel = getChannel(id, channelStore->children())) && id > 0) {
        fprintf(stderr, "channel missing: id: %d - name: %s - parent; %d\n", id, name.c_str(), parent_id);
    }
    displayName = name;
    if (v3_is_channel_admin(id)) {
        displayName = "[A] " + displayName;
    }
    if (! comment.empty()) {
        displayName = displayName + " (" + comment + ")";
    }
    channel[channelRecord.displayName]       = displayName;
    switch (protect_mode) {
        case 0:
            channel[channelRecord.icon]              = mangler->icons["black_circle"]->scale_simple(12, 12, Gdk::INTERP_BILINEAR);;
            break;
        case 1:
            channel[channelRecord.icon]              = mangler->icons["red_circle"]->scale_simple(12, 12, Gdk::INTERP_BILINEAR);;
            break;
        case 2:
            channel[channelRecord.icon]              = mangler->icons["yellow_circle"]->scale_simple(12, 12, Gdk::INTERP_BILINEAR);;
            break;
    }
    channel[channelRecord.isUser]            = false;
    channel[channelRecord.isGuest]           = false;
    channel[channelRecord.id]                = id;
    channel[channelRecord.parent_id]         = parent_id;
    channel[channelRecord.name]              = name;
    channel[channelRecord.comment]           = comment;
    channel[channelRecord.phonetic]          = phonetic;
    channel[channelRecord.url]               = "";
    channel[channelRecord.integration_text]  = "";
    channel[channelRecord.rank]              = "";
    channel[channelRecord.password]          = "";
}/*}}}*/

void
ManglerChannelTree::refreshChannel(uint32_t id) {/*{{{*/
    Glib::ustring displayName = "";
    Gtk::TreeModel::Row channel;
    Glib::ustring name;
    Glib::ustring phonetic;
    Glib::ustring comment;

    if (! (channel = getChannel(id, channelStore->children())) && id > 0) {
        fprintf(stderr, "channel missing: id: %d\n", id);
    }
    name = channel[channelRecord.name];
    comment = channel[channelRecord.comment];
    phonetic = channel[channelRecord.phonetic];
    displayName = name;
    if (v3_is_channel_admin(id)) {
        displayName = "[A] " + displayName;
    }
    if (! comment.empty()) {
        displayName = displayName + " (" + comment + ")";
    }
    channel[channelRecord.displayName]       = displayName;
}/*}}}*/

void
ManglerChannelTree::refreshAllChannels(void) {/*{{{*/
    _refreshAllChannels(channelStore->children());
}/*}}}*/

void
ManglerChannelTree::_refreshAllChannels(Gtk::TreeModel::Children children) {/*{{{*/
    Gtk::TreeModel::Children::iterator iter = children.begin();
    while (iter != children.end()) {
        Gtk::TreeModel::Row row = *iter;
        uint32_t id = row[channelRecord.id];
        uint32_t isUser = row[channelRecord.isUser];
        if (!isUser) {
            refreshChannel(id);
        }
        if (row.children().size()) {
            _refreshAllChannels(row.children());
        }
        iter++;
    }
    return;
}/*}}}*/

void
ManglerChannelTree::refreshUser(uint32_t id) {/*{{{*/
    Glib::ustring displayName = "";
    Gtk::TreeModel::Row user;
    Glib::ustring name;
    Glib::ustring url;
    Glib::ustring integration_text;
    Glib::ustring phonetic;
    Glib::ustring comment;
    Glib::ustring rank;
    bool guest;
    bool phantom;

    if (! (user = getUser(id, channelStore->children())) && id > 0) {
        fprintf(stderr, "channel missing: id: %d\n", id);
    }
    name = user[channelRecord.name];
    comment = user[channelRecord.comment];
    url = user[channelRecord.url];
    integration_text = user[channelRecord.integration_text];
    phonetic = user[channelRecord.phonetic];
    rank = user[channelRecord.rank];
    phantom = user[channelRecord.phantom];
    guest = user[channelRecord.isGuest];
    displayName = name;
    if (!rank.empty()) {
        displayName = "[" + rank + "] " + displayName; 
    }
    if (phantom) {
        displayName = "\"P\" " + displayName; 
    }
    if (guest && !mangler->settings->config.guestFlagHidden) {
        displayName = displayName + " (GUEST)";
    }
    if (mangler->chat->isUserInChat(id)) {
        displayName = "[C] " + displayName;
    }
    if (! comment.empty()) {
        displayName = displayName + " (" + (url.empty() ? "" : "U: ") + comment + ")";
    } else if (comment.empty() && !url.empty()) {
        displayName = displayName + " (" + (url.empty() ? "" : "U: ") + url + ")";
    }

    if (! integration_text.empty()) {
        displayName = displayName + " {" + integration_text + "}";
    }

    user[channelRecord.displayName]       = displayName;
}/*}}}*/

void
ManglerChannelTree::refreshAllUsers(void) {/*{{{*/
    _refreshAllUsers(channelStore->children());
}/*}}}*/

void
ManglerChannelTree::_refreshAllUsers(Gtk::TreeModel::Children children) {/*{{{*/
    Gtk::TreeModel::Children::iterator iter = children.begin();
    while (iter != children.end()) {
        Gtk::TreeModel::Row row = *iter;
        uint32_t id = row[channelRecord.id];
        uint32_t isUser = row[channelRecord.isUser];
        if (isUser) {
            refreshUser(id);
        }
        if (row.children().size()) {
            _refreshAllUsers(row.children());
        }
        iter++;
    }
    return;
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
    if (mangler->isAdmin) {
        displayName = displayName + " [ADMIN]";
    }
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
ManglerChannelTree::setUserIcon(uint16_t id, Glib::ustring color) {/*{{{*/
    Gtk::TreeModel::Row user = getUser(id, channelStore->children());
    Gtk::TreeModel::Row me   = getUser(v3_get_user_id(), channelStore->children());
    Glib::ustring iconname = "user_icon_" + color;
    if (! mangler->icons[iconname]) {
        iconname = "user_icon_red";
    }
    user[channelRecord.icon] = mangler->icons[iconname]->scale_simple(15, 15, Gdk::INTERP_BILINEAR);
    user[channelRecord.last_transmit]     = getTimeString();
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
        userSettingsWindow(row);
    } else {
        // double clicked a channel
        Gtk::TreeModel::Row user = getUser(v3_get_user_id(), channelStore->children());
        int curchannel = user[channelRecord.parent_id];
        uint16_t pw_cid;
        Gtk::TreeModel::Row pwrow;

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
            if ((pw_cid = v3_channel_requires_password(channel->id))) {  // Channel is password protected
                password_required = true;
                password = getChannelSavedPassword(pw_cid);
                // if we didn't find a saved password, prompt the user
                if (password.empty()) {
                    password = mangler->getPasswordEntry("Channel Password");
                }
                setChannelSavedPassword(pw_cid, password);
                if (mangler->connectedServerId != -1) {
                    ManglerServerConfig *server;
                    server = mangler->settings->config.getserver(mangler->connectedServerId);
                    server->channelpass[pw_cid] = password;
                }
            }
            v3_free_channel(channel);
        }
        if (password_required && password.empty()) {
            return;
        }
        v3_change_channel(id, (char *)password.c_str());
    }
}/*}}}*/

/*
 * Right-click menu is handled here
 */
void
ManglerChannelTree::channelView_buttonpress_event_cb(GdkEventButton* event) {/*{{{*/
    Gtk::TreeModel::Path path;
    Gtk::TreeModel::Row row;
    Gtk::TreeModel::iterator iter;
    if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3)) {
        if (channelView->get_path_at_pos((int)event->x, (int)event->y, path)) {
            iter = channelStore->get_iter(path);
            row = *iter;
            uint16_t id = row[channelRecord.id];
            bool isUser = row[channelRecord.isUser];
            bool muted = row[channelRecord.muted];
            Glib::ustring comment = row[channelRecord.comment];
            Glib::ustring url = row[channelRecord.url];
            const v3_permissions *perms = v3_get_permissions();

            if (isUser) {
                v3_user *user = v3_get_user(id);
                builder->get_widget("copyComment", menuitem);
                menuitem->set_sensitive(comment.empty() ? false : true);
                builder->get_widget("copyURL", menuitem);
                menuitem->set_sensitive(url.empty() ? false : true);
                builder->get_widget("removePhantom", menuitem);
                if (user->real_user_id == v3_get_user_id()) {
                    // we clicked on one of our own phantoms
                    menuitem->show();
                } else {
                    menuitem->hide();
                }
                if (user->id == v3_get_user_id()) {
                    // we clicked ourself
                    builder->get_widget("userSettings", menuitem);
                    menuitem->hide();
                    builder->get_widget("kickUser", menuitem);
                    menuitem->hide();
                    builder->get_widget("banUser", menuitem);
                    menuitem->hide();
                    builder->get_widget("muteUser", menuitem);
                    menuitem->hide();
                    builder->get_widget("muteUserGlobal", menuitem);
                    menuitem->hide();
                } else {
                    builder->get_widget("userSettings", menuitem);
                    menuitem->show();
                    builder->get_widget("kickUser", menuitem);
                    if (perms->kick_user) {
                        menuitem->show();
                    } else {
                        menuitem->hide();
                    }
                    builder->get_widget("banUser", menuitem);
                    if (perms->ban_user) {
                        menuitem->show();
                    } else {
                        menuitem->hide();
                    }
                    builder->get_widget("muteUser", menuitem);
                    if (muted) {
                        menuitem->set_label("Unmute");
                    } else {
                        menuitem->set_label("Mute");
                    }
                    menuitem->show();
                    builder->get_widget("muteUserGlobal", menuitem);
                    if (perms->mute_glbl) {
                        menuitem->set_sensitive(false);
                        menuitem->show();
                    } else {
                        menuitem->hide();
                    }
                }
                rcmenu_user->popup(event->button, event->time);
                v3_free_user(user);
            } else {
                rcmenu_channel->popup(event->button, event->time);
            }
        }
    }
}/*}}}*/

void
ManglerChannelTree::userSettingsMenuItem_activate_cb(void) {/*{{{*/
    Glib::RefPtr<Gtk::TreeSelection> sel = channelView->get_selection();
    Gtk::TreeModel::iterator iter = sel->get_selected();
    if(iter) {
        Gtk::TreeModel::Row row = *iter;
        userSettingsWindow(row);
    }
}/*}}}*/

void
ManglerChannelTree::copyCommentMenuItem_activate_cb(void) {/*{{{*/
    Glib::RefPtr<Gtk::Clipboard> clipboard = Gtk::Clipboard::get();
    Glib::RefPtr<Gtk::TreeSelection> sel = channelView->get_selection();
    Gtk::TreeModel::iterator iter = sel->get_selected();
    if(iter) {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring comment = row[channelRecord.comment];
        clipboard->set_text(comment);
    }
}/*}}}*/

void
ManglerChannelTree::privateChatMenuItem_activate_cb(void) {/*{{{*/
    Glib::RefPtr<Gtk::TreeSelection> sel = channelView->get_selection();
    Gtk::TreeModel::iterator iter = sel->get_selected();
    if(iter) {
        Gtk::TreeModel::Row row = *iter;
        v3_user *u;
        uint16_t id = row[channelRecord.id];
        if ((u = v3_get_user(id))) {
            if (id == v3_get_user_id() || u->real_user_id == v3_get_user_id()) {
                v3_free_user(u);
                return;
            }
            v3_free_user(u);
        } else {
            return;
        }
        Glib::ustring name = row[channelRecord.name];
        fprintf(stderr, "opening chat with %d\n", id);
        mangler->privateChatWindows[id] = new ManglerPrivChat(id);
        mangler->privateChatWindows[id]->addMessage("*** opened private chat with " + name);
        fprintf(stderr, "opened chat window with %d\n", mangler->privateChatWindows[id]->remoteUserId);
    }
}/*}}}*/

void
ManglerChannelTree::copyURLMenuItem_activate_cb(void) {/*{{{*/
    Glib::RefPtr<Gtk::Clipboard> clipboard = Gtk::Clipboard::get();
    Glib::RefPtr<Gtk::TreeSelection> sel = channelView->get_selection();
    Gtk::TreeModel::iterator iter = sel->get_selected();
    if(iter) {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring url = row[channelRecord.url];
        clipboard->set_text(url);
    }
}/*}}}*/

void
ManglerChannelTree::addPhantomMenuItem_activate_cb(void) {/*{{{*/
    Glib::RefPtr<Gtk::TreeSelection> sel = channelView->get_selection();
    Gtk::TreeModel::iterator iter = sel->get_selected();
    if(iter) {
        Gtk::TreeModel::Row row = *iter;
        bool isUser = row[channelRecord.isUser];
        uint16_t id = row[channelRecord.id];
        Glib::ustring name = row[channelRecord.name];
        if (!isUser) {
            v3_phantom_add(id);
        }
    }
}/*}}}*/

void
ManglerChannelTree::removePhantomMenuItem_activate_cb(void) {/*{{{*/
    Glib::RefPtr<Gtk::TreeSelection> sel = channelView->get_selection();
    Gtk::TreeModel::iterator iter = sel->get_selected();
    if(iter) {
        Gtk::TreeModel::Row row = *iter;
        uint16_t parent_id = row[channelRecord.parent_id];
        fprintf(stderr, "removing phantom id %d\n", parent_id);
        v3_phantom_remove(parent_id);
    }
}/*}}}*/

void
ManglerChannelTree::kickUserMenuItem_activate_cb(void) {/*{{{*/
    Glib::RefPtr<Gtk::TreeSelection> sel = channelView->get_selection();
    Gtk::TreeModel::iterator iter = sel->get_selected();
    if(iter) {
        Gtk::TreeModel::Row row = *iter;
        bool isUser = row[channelRecord.isUser];
        uint16_t id = row[channelRecord.id];
        Glib::ustring name = row[channelRecord.name];
        if (isUser) {
            if ( mangler->getReasonEntry("Kick Reason") ) {
               v3_admin_boot(V3_BOOT_KICK, id, (char *)ustring_to_c(mangler->reason).c_str());
            }
        }
    }
}/*}}}*/

void
ManglerChannelTree::banUserMenuItem_activate_cb(void) {/*{{{*/
    Glib::RefPtr<Gtk::TreeSelection> sel = channelView->get_selection();
    Gtk::TreeModel::iterator iter = sel->get_selected();
    if(iter) {
        Gtk::TreeModel::Row row = *iter;
        bool isUser = row[channelRecord.isUser];
        uint16_t id = row[channelRecord.id];
        Glib::ustring name = row[channelRecord.name];
        if (isUser) {
            if ( mangler->getReasonEntry("Ban Reason") ) {
               v3_admin_boot(V3_BOOT_BAN, id, (char *)ustring_to_c(mangler->reason).c_str());
            }
        }
    }
}/*}}}*/

void
ManglerChannelTree::muteUserMenuItem_activate_cb(void) {/*{{{*/
    Glib::RefPtr<Gtk::TreeSelection> sel = channelView->get_selection();
    Gtk::TreeModel::iterator iter = sel->get_selected();
    if(iter) {
        Gtk::TreeModel::Row row = *iter;
        bool muted = row[channelRecord.muted];
        if (muted) {
            row[channelRecord.muted] = false;
        } else {
            row[channelRecord.muted] = true;
        }
    }
}/*}}}*/

void
ManglerChannelTree::muteUserGlobalMenuItem_activate_cb(void) {/*{{{*/
}/*}}}*/

uint16_t
ManglerChannelTree::getUserChannelId(uint16_t userid) {/*{{{*/
    Gtk::TreeModel::Row user = getUser(userid, channelStore->children());
    return(user[channelRecord.parent_id]);
}/*}}}*/

Glib::ustring
ManglerChannelTree::getChannelSavedPassword(uint16_t channel_id) {/*{{{*/
    Gtk::TreeModel::Row channel = getChannel(channel_id, channelStore->children());
    Glib::ustring pw = channel[channelRecord.password];
    if (pw.length() == 0) {
        if (mangler->connectedServerId != -1) {
            ManglerServerConfig *server;
            server = mangler->settings->config.getserver(mangler->connectedServerId);
            pw = server->channelpass[channel_id];
        }
    }
    return(pw);
}/*}}}*/

void
ManglerChannelTree::setChannelSavedPassword(uint16_t channel_id, Glib::ustring password) {/*{{{*/
    Gtk::TreeModel::Row channel = getChannel(channel_id, channelStore->children());
    channel[channelRecord.password] = password;
    if (mangler->connectedServerId != -1) {
        ManglerServerConfig *server;
        server = mangler->settings->config.getserver(mangler->connectedServerId);
        server->channelpass[channel_id] = password;
        mangler->settings->config.save();
    }
    return;
}/*}}}*/

void
ManglerChannelTree::forgetChannelSavedPassword(uint16_t channel_id) {/*{{{*/
    setChannelSavedPassword(channel_id, "");
    return;
}/*}}}*/

void
ManglerChannelTree::volumeAdjustment_value_changed_cb(uint16_t id) {/*{{{*/
    if (mangler->connectedServerId != -1) {
        ManglerServerConfig *server;
        server = mangler->settings->config.getserver(mangler->connectedServerId);
        v3_user *u;
        if ((u = v3_get_user(id)) != NULL) {
            server->uservolumes[u->name] = volumeAdjustment->get_value();
            mangler->settings->config.save();
        }
    }
    v3_set_volume_user(id, volumeAdjustment->get_value());
}/*}}}*/

Glib::ustring
ManglerChannelTree::getLastTransmit(uint16_t userid) {/*{{{*/
    Gtk::TreeModel::Row user = getUser(userid, channelStore->children());
    return(user[channelRecord.last_transmit]);
}/*}}}*/

bool
ManglerChannelTree::isMuted(uint16_t userid) {/*{{{*/
    Gtk::TreeModel::Row user = getUser(userid, channelStore->children());
    return(user[channelRecord.muted]);
}/*}}}*/

void
ManglerChannelTree::setLastTransmit(uint16_t userid, Glib::ustring last_transmit) {/*{{{*/
    Gtk::TreeModel::Row user = getUser(userid, channelStore->children());
    user[channelRecord.last_transmit] = last_transmit;
}/*}}}*/

Glib::ustring
getTimeString(void) {/*{{{*/
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
}/*}}}*/

Glib::RefPtr<ManglerChannelStore> ManglerChannelStore::create() {/*{{{*/
    return Glib::RefPtr<ManglerChannelStore>( new ManglerChannelStore() );
}/*}}}*/

bool
ManglerChannelStore::row_draggable_vfunc(const Gtk::TreeModel::Path& path) const {/*{{{*/
    const v3_permissions *perms = v3_get_permissions();
    ManglerChannelStore* unconstThis = const_cast<ManglerChannelStore*>(this);
    const_iterator iter = unconstThis->get_iter(path);
    if (!iter) 
        return Gtk::TreeStore::row_draggable_vfunc(path);
    
    Row row = *iter;
    if (row[c.isUser] && perms->move_user && (perms->srv_admin || v3_is_channel_admin(v3_get_user_channel(row[c.id]))))
        return true;
    return false;
}/*}}}*/

bool
ManglerChannelStore::row_drop_possible_vfunc(const Gtk::TreeModel::Path& dest, const Gtk::SelectionData& selection_data) const {/*{{{*/
    Gtk::TreeModel::Path dest_parent = dest;
    bool dest_is_not_top_level = dest_parent.up();
    if(!dest_is_not_top_level || dest_parent.empty()) {
        return false;
    }
    return true;
}/*}}}*/

bool
ManglerChannelStore::drag_data_received_vfunc(const Gtk::TreeModel::Path& dest, const Gtk::SelectionData& selection_data) {/*{{{*/
    // This is confusing... i'll try to explain
    // First, let's find out who we're moving
    Gtk::TreeModel::Path path_dragged_row;
    Gtk::TreeModel::Path::get_from_selection_data(selection_data, path_dragged_row);
    Gtk::TreeModel::iterator srciter = get_iter(path_dragged_row);
    Gtk::TreeModel::Row srcrow = *srciter;
    int srcid = srcrow[c.id];
    Glib::ustring srcname = srcrow[c.name];
    //fprintf(stderr, "moving user %d - %s to ", srcid, (char *)srcname.c_str());

    // because GTK allows you to drop things in places that don't really make
    // sense in terms of ventrilo, we need to modify the drop path to make sense
    // basically, if the path doesn't end in a 0, it's either in between users
    // or in between channels.  Instead of adding to the parrent channel, tack
    // a 0 on to the end of the path and decrement the destination to give us
    // the previous channel/user
    Gtk::TreeModel::Path dest_parent = dest;
    if (dest_parent[dest_parent.get_depth()-1] != 0) {
        dest_parent[dest_parent.get_depth()-1]--;
        dest_parent.push_back(0);
    }
    // The dest path will always be the where the channel would end up as a
    // child, so next is to go up a node in the tree and see if it's a
    // channel
    dest_parent.up();
    Gtk::TreeModel::iterator destiter = get_iter(dest_parent);
    Gtk::TreeModel::Row destrow = *destiter;
    bool isUser = destrow[c.isUser];
    if (isUser) {
        // If it's a user, go up another node to get that user's channel
        dest_parent.up();
        destiter = get_iter(dest_parent);
        destrow = *destiter;
    }
    int destid = destrow[c.id];
    Glib::ustring destname = destrow[c.name];
    //fprintf(stderr, " %d - %s\n", destid, (char *)destname.c_str());

    v3_force_channel_move(srcid, destid);

    // we always return false... if the move succeeds, we'll get an event
    // telling us to move the user
    return false;
}/*}}}*/

int
ManglerChannelTree::on_sort_compare(const Gtk::TreeModel::iterator& a_, const Gtk::TreeModel::iterator& b_) {/*{{{*/
    Gtk::TreeModel::Row row1 = *a_;
    Gtk::TreeModel::Row row2 = *b_;
    Glib::ustring row1_name = row1[channelRecord.name];
    Glib::ustring row2_name = row2[channelRecord.name];
    //fprintf(stderr, "%s == %s\n", (char *)row1_name.c_str(), (char *)row2_name.c_str());
    bool row1_isUser = row1[channelRecord.isUser];
    bool row2_isUser = row2[channelRecord.isUser];
    if (row1_isUser && row2_isUser) {
        if (row1_name == row2_name) {
            return 0;
        }
        if (row1_name < row2_name) {
            return -1;
        }
        return 1;
    }
    return 0;
}/*}}}*/

void
ManglerChannelTree::userSettingsWindow(Gtk::TreeModel::Row row) {

    int id = row ;
    // double clicked a user
    if (id == v3_get_user_id()) {
        // clicked on ourself
    } else {
        v3_user *u;
        uint16_t id = row[channelRecord.id];
        Glib::ustring name = row[channelRecord.name];
        Glib::ustring comment = row[channelRecord.comment];
        Glib::ustring url = row[channelRecord.url];
        bool  accept_pages = false, accept_u2u = false, accept_chat = false, allow_recording = false;
        if ((u = v3_get_user(id)) != NULL) {
            accept_pages = u->accept_pages;
            accept_u2u = u->accept_u2u;
            accept_chat = u->accept_chat;
            allow_recording = u->allow_recording;
            v3_free_user(u);
        }

        // disconnect whatever was connected before and reconnect
        volumeAdjustSignalConnection.disconnect();
        volumeAdjustSignalConnection = volumeAdjustment->signal_value_changed().connect(sigc::bind(sigc::mem_fun(this, &ManglerChannelTree::volumeAdjustment_value_changed_cb), id));

        // set the value label
        builder->get_widget("userSettingsNameValueLabel", label);
        label->set_text(name);
        builder->get_widget("userSettingsCommentValue", label);
        label->set_text(comment);
        builder->get_widget("userSettingsURLValue", linkbutton);
        linkbutton->set_uri(url);
        linkbutton->set_label(url);
        builder->get_widget("userSettingsU2UValue", label);
        label->set_text(accept_u2u ? "Yes" : "No");
        builder->get_widget("userSettingsRecordValue", label);
        label->set_text(allow_recording ? "Yes" : "No");
        builder->get_widget("userSettingsPageValue", label);
        label->set_text(accept_pages ? "Yes" : "No");
        builder->get_widget("userSettingsChatValue", label);
        label->set_text(accept_chat ? "Yes" : "No");

        // set the current volume level for this user
        volumeAdjustment->set_value(v3_get_volume_user(id));

        builder->get_widget("userSettingsWindow", window);
        window->show_all();
        window->queue_resize();
        window->present();
    }
}
