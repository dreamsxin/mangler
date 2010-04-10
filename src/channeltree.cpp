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

#include "manglerchat.h"
#include "manglerconfig.h"
#include "manglersettings.h"
#include "manglerprivchat.h"
#include "manglercharset.h"

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
    pColumn->set_reorderable(true);
    channelView->append_column(*pColumn);
    channelView->set_expander_column(*pColumn);
    pColumn = Gtk::manage( new Gtk::TreeView::Column("Last Transmit") );
    pColumn->pack_start(channelRecord.last_transmit);
    pColumn->set_reorderable(true);
    channelView->append_column(*pColumn);
    channelView->signal_columns_changed().connect(sigc::mem_fun(this, &ManglerChannelTree::channelView_columns_changed_cb));
    if (Mangler::config["ChannelViewReverseHeaderOrder"].toBool()) {
        channelView->move_column_to_start(*pColumn);
    }

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
    builder->get_widget("setDefaultChannel", checkmenuitem);
    signalDefaultChannel = checkmenuitem->signal_toggled().connect(sigc::mem_fun(this, &ManglerChannelTree::setDefaultChannel_toggled_cb));


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

    if (parent) {
        channelIter                             = channelStore->prepend(parent.children());
    } else {
        channelIter                             = channelStore->prepend();
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
    channelRow[channelRecord.last_transmit]     = "";
    channelRow[channelRecord.password]          = "";
    channelRow[channelRecord.phantom]           = phantom;
    refreshUser(id);
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
    if (! (user = getUser(id)) && id > 0) {
        fprintf(stderr, "missing user: id %d: %s is supposed to be in channel %d\n", id, name.c_str(), parent_id);
        return;
    }

    user[channelRecord.displayName]       = displayName;
    if (id == v3_get_user_id()) {
        if (mangler->isTransmitting) {
            user[channelRecord.icon]      = mangler->icons["user_icon_green"]->scale_simple(15, 15, Gdk::INTERP_BILINEAR);
        } else {
            user[channelRecord.icon]      = mangler->icons["user_icon_red"]->scale_simple(15, 15, Gdk::INTERP_BILINEAR);
        }
    }
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
    refreshUser(id);
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
        channelIter                             = channelStore->append(parent.children());
    } else {
        channelIter                             = channelStore->append();
    }
    channelRow                                  = *channelIter;
    channelRow[channelRecord.displayName]       = displayName;
    switch (protect_mode) {
        case 0:
            channelRow[channelRecord.icon]      = mangler->icons["black_circle"]->scale_simple(12, 12, Gdk::INTERP_BILINEAR);;
            break;
        case 1:
            channelRow[channelRecord.icon]      = mangler->icons["red_circle"]->scale_simple(12, 12, Gdk::INTERP_BILINEAR);;
            break;
        case 2:
            channelRow[channelRecord.icon]      = mangler->icons["yellow_circle"]->scale_simple(12, 12, Gdk::INTERP_BILINEAR);;
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
            channel[channelRecord.icon]      = mangler->icons["black_circle"]->scale_simple(12, 12, Gdk::INTERP_BILINEAR);;
            break;
        case 1:
            channel[channelRecord.icon]      = mangler->icons["red_circle"]->scale_simple(12, 12, Gdk::INTERP_BILINEAR);;
            break;
        case 2:
            channel[channelRecord.icon]      = mangler->icons["yellow_circle"]->scale_simple(12, 12, Gdk::INTERP_BILINEAR);;
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
    if (!sortAlphanumeric) {
        channelStore->set_sort_func (0, sigc::mem_fun (*this, &ManglerChannelTree::sortFunction));
        channelStore->set_sort_column(channelRecord.displayName, Gtk::SORT_ASCENDING);
    } else {
        channelStore->set_sort_func (channelRecord.id, sigc::mem_fun (*this, &ManglerChannelTree::sortFunction));
        channelStore->set_sort_column(channelRecord.id, Gtk::SORT_ASCENDING);
    }
}/*}}}*/

int
ManglerChannelTree::sortFunction(const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b){/*{{{*/
    bool isUser_a = (*a)[channelRecord.isUser];
    bool isUser_b = (*b)[channelRecord.isUser];
//    Glib::ustring sort_a = (isUser_a) ? (*a)[channelRecord.name] : (*a)[channelRecord.displayName];
//    Glib::ustring sort_b = (isUser_b) ? (*b)[channelRecord.name] : (*b)[channelRecord.displayName];

    Glib::ustring sort_a = (*a)[channelRecord.name];
    Glib::ustring sort_b = (*b)[channelRecord.name];
    const char *sortName_a = sort_a.c_str();
    const char *sortName_b = sort_b.c_str();
    if (isUser_a) {
        if (isUser_b) {
            return natsort(sortName_a, sortName_b);
        } else {
            //A = User, B = Channel
            return -1;
        }
    } else {
        if (isUser_b) {
            //A = Channel, B = User
            return 1;
        } else {
            // Both are Channels
            if (!sortAlphanumeric) {
                //Sort alphabetical
                return natsort(sortName_a, sortName_b);
            } else {
                //Sort manual
                if ((*a)[channelRecord.id] > (*b)[channelRecord.id]) {
                    return 1;
                } else if ((*a)[channelRecord.id] < (*b)[channelRecord.id]) {
                    return -1;
                }
            }
            return 0;
        }
    }
    fprintf(stderr, "sorting failed to find if one or both options were users or channels\n");
    return 0;
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
    v3_user *u;
    Gtk::TreeModel::Row user;
    Glib::ustring name;
    Glib::ustring url;
    Glib::ustring integration_text;
    Glib::ustring phonetic;
    Glib::ustring comment;
    Glib::ustring rank;
    Glib::ustring flags;
    bool guest;
    bool phantom;
    bool muted;
    bool global_mute;
    bool channel_mute;

    if (!(user = getUser(id)) && id > 0) {
        fprintf(stderr, "channel missing: id: %d\n", id);
    }
    if ((u = v3_get_user(id))) {
        global_mute = u->global_mute;
        channel_mute = u->channel_mute;
        // update xmit icons
        if (id == v3_get_user_id() && mangler->isTransmitting) {
            // we're transmitting
            setUserIcon(id, "orange", true);
        } else if (u->is_transmitting) {
            // transmitting without playback
            setUserIcon(id, "yellow", true);
        } else {
            // not transmitting
            setUserIcon(id, "red");
        }
        v3_free_user(u);
    }
    name             = user[channelRecord.name];
    comment          = user[channelRecord.comment];
    url              = user[channelRecord.url];
    integration_text = user[channelRecord.integration_text];
    phonetic         = user[channelRecord.phonetic];
    rank             = user[channelRecord.rank];
    phantom          = user[channelRecord.phantom];
    guest            = user[channelRecord.isGuest];
    muted            = user[channelRecord.muted];
    displayName      = name;
    if (!rank.empty()) {
        displayName = "[" + rank + "] " + displayName;
    }
    flags = "";
    if (phantom) {
        flags += "P";
    }
    if (channel_mute) {
        flags += "N";
    }
    if (global_mute) {
        flags += "G";
    }
    if (muted) {
        flags += "M";
    }
    if (mangler->chat->isUserInChat(id)) {
        flags += "C";
    }
    if (flags.length()) {
        displayName = "[" + flags + "] " + displayName;
    }
    if (guest && !Mangler::config["guestFlagHidden"].toBool()) {
        displayName = displayName + " (GUEST)";
    }
    if (! comment.empty()) {
        displayName = displayName + " (" + (url.empty() ? "" : "U: ") + comment + ")";
    } else if (comment.empty() && !url.empty()) {
        displayName = displayName + " (" + (url.empty() ? "" : "U: ") + url + ")";
    }
    if (! integration_text.empty()) {
        displayName = displayName + " {" + integration_text + "}";
    }

    user[channelRecord.displayName] = displayName;
    mangler->chat->updateUser(id);
    mangler->chat->chatUserTreeModelFilter->refilter();
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

    if (! (user = getUser(id)) && id > 0) {
        fprintf(stderr, "removeUser: could not find user id %d to delete\n", id);
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
ManglerChannelTree::getUser(uint32_t id) {/*{{{*/
    Gtk::TreeModel::iterator iter;
    channelStore->foreach_iter(sigc::bind(sigc::mem_fun(*this,&ManglerChannelTree::_getUser), id, &iter));
    if (iter) {
        if ((bool)(*iter)[channelRecord.isUser]) {
            return *iter;
        }
    }
    Gtk::TreeModel::Row empty;
    return empty;
}/*}}}*/

bool
ManglerChannelTree::_getUser(const Gtk::TreeModel::iterator &iter, uint32_t id, Gtk::TreeModel::iterator *r_iter) {/*{{{*/
    if((uint32_t)(*iter)[channelRecord.id] == id) {
        *r_iter = iter;
        return true;
    }
    return false;
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
ManglerChannelTree::setUserIcon(uint16_t id, Glib::ustring color, bool updateLastTransmit) {/*{{{*/
    Gtk::TreeModel::Row user = getUser(id);
    Gtk::TreeModel::Row me   = getUser(v3_get_user_id());
    Glib::ustring iconname = "user_icon_" + color;
    if (! mangler->icons[iconname]) {
        iconname = "user_icon_red";
    }
    if (!user) {
        fprintf(stderr, "setUserIcon: failed to retrieve row for user id %d\n", id);
        return;
    }
    if (!me) {
        fprintf(stderr, "setUserIcon: failed to retrieve row for my id %d\n", v3_get_user_id());
        return;
    }
    user[channelRecord.icon] = mangler->icons[iconname]->scale_simple(15, 15, Gdk::INTERP_BILINEAR);
    if (updateLastTransmit) {
        user[channelRecord.last_transmit]     = getTimeString();
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
        userSettingsWindow(row);
    } else {
        // double clicked a channel
        Gtk::TreeModel::Row user = getUser(v3_get_user_id());
        int curchannel = user[channelRecord.parent_id];
        uint16_t pw_cid;
        Gtk::TreeModel::Row pwrow;
        if (!user) {
            fprintf(stderr, "failed to retrieve row for id %d\n", id);
            return;
        }
        if (id == curchannel) {
            // we're already in this channel
            return;
        }
        if (id != 0) {
            channel = v3_get_channel(id);
            const v3_permissions *perms = v3_get_permissions();
            if (! channel) {
                fprintf(stderr, "failed to retrieve channel information for channel id %d\n", id);
                return;
            }
            if (!perms->srv_admin && (pw_cid = v3_channel_requires_password(channel->id))) {  // Channel is password protected
                password_required = true;
                password = getChannelSavedPassword(pw_cid);
                // if we didn't find a saved password, prompt the user
                if (password.empty()) {
                    password = mangler->getPasswordEntry("Channel Password");
                }
                setChannelSavedPassword(pw_cid, password);
                //if (! mangler->connectedServerName.empty()) {
                //    Mangler::config.ChannelPassword(mangler->connectedServerName, pw_cid) = password;
                //}
            }
            v3_free_channel(channel);
        }
        if (password_required && password.empty()) {
            return;
        }
        v3_change_channel(id, (char *)password.c_str());
    }
}/*}}}*/

void
ManglerChannelTree::channelView_columns_changed_cb(void) {/*{{{*/
    Gtk::TreeViewColumn *col0 = channelView->get_column(0);
    if (col0->get_title() == "Last Transmit") {
            Mangler::config["ChannelViewReverseHeaderOrder"] = true;
    } else {
            Mangler::config["ChannelViewReverseHeaderOrder"] = false;
    }
}
/*}}}*/

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
                bool isOurPhantom = user->real_user_id == v3_get_user_id();
                builder->get_widget("removePhantom", menuitem);
                if (isOurPhantom) {
                    // we clicked on one of our own phantoms
                    menuitem->show();
                } else {
                    menuitem->hide();
                }
                builder->get_widget("copyComment", menuitem);
                menuitem->set_sensitive(comment.empty() ? false : true);
                builder->get_widget("copyURL", menuitem);
                menuitem->set_sensitive(url.empty() ? false : true);
                if (user->id == v3_get_user_id()) {
                    // we clicked ourself
                    builder->get_widget("privateChat", menuitem);
                    menuitem->hide();
                    builder->get_widget("userSettings", menuitem);
                    menuitem->hide();
                    builder->get_widget("muteUser", menuitem);
                    menuitem->hide();
                    builder->get_widget("userRightClickMenuSeparator", menuitem);
                    menuitem->hide();
                    builder->get_widget("kickUser", menuitem);
                    menuitem->hide();
                    builder->get_widget("banUser", menuitem);
                    menuitem->hide();
                    builder->get_widget("muteUserGlobal", menuitem);
                    menuitem->hide();
                } else {
                    builder->get_widget("privateChat", menuitem);
                    if (perms->start_priv_chat && !isOurPhantom) {
                        menuitem->show();
                    } else {
                        menuitem->hide();
                    }
                    builder->get_widget("userSettings", menuitem);
                    if (!isOurPhantom) {
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
                    if (!isOurPhantom) {
                        menuitem->show();
                    } else {
                        menuitem->hide();
                    }
                    builder->get_widget("userRightClickMenuSeparator", menuitem);
                    if ((perms->kick_user || perms->ban_user || perms->mute_glbl) && !isOurPhantom) {
                        menuitem->show();
                    } else {
                        menuitem->hide();
                    }
                    builder->get_widget("kickUser", menuitem);
                    if (perms->kick_user && !isOurPhantom) {
                        menuitem->show();
                    } else {
                        menuitem->hide();
                    }
                    builder->get_widget("banUser", menuitem);
                    if (perms->ban_user && !isOurPhantom) {
                        menuitem->show();
                    } else {
                        menuitem->hide();
                    }
                    builder->get_widget("muteUserGlobal", menuitem);
                    if (perms->mute_glbl && !isOurPhantom) {
                        menuitem->set_sensitive(false);
                        menuitem->show();
                    } else {
                        menuitem->hide();
                    }
                }
                rcmenu_user->popup(event->button, event->time);
                v3_free_user(user);
            } else {
                builder->get_widget("setDefaultChannel", checkmenuitem);
                if (mangler->connectedServerName.empty()) { //Hide default channel on quick connects
                    checkmenuitem->set_sensitive(false);
                    checkmenuitem->hide();
                } else {
                    checkmenuitem->set_sensitive(true);
                    checkmenuitem->show();
                    string server = Mangler::config[mangler->connectedServerName].toString();
                    if (Mangler::config["DefaultChannelID"].toULong() == row[channelRecord.id]) {
                        signalDefaultChannel.block();
                        checkmenuitem->set_active(true);
                        signalDefaultChannel.unblock();
                    } else {
                        signalDefaultChannel.block();
                        checkmenuitem->set_active(false);
                        signalDefaultChannel.unblock();
                    }
                }
                rcmenu_channel->set_title("test");
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
        //fprintf(stderr, "opening chat with %d\n", id);
        mangler->privateChatWindows[id] = new ManglerPrivChat(id);
        mangler->privateChatWindows[id]->addMessage("*** opened private chat with " + name);
        //fprintf(stderr, "opened chat window with %d\n", mangler->privateChatWindows[id]->remoteUserId);
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
        //fprintf(stderr, "removing phantom id %d\n", parent_id);
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
        Glib::ustring name = row[channelRecord.name];
        bool muted = row[channelRecord.muted];
        uint16_t id = row[channelRecord.id];
        if (muted) {
            row[channelRecord.muted] = false;
        } else {
            row[channelRecord.muted] = true;
        }
        Mangler::config.UserMuted(mangler->connectedServerName, name) = (bool)row[channelRecord.muted];
        Mangler::config.servers.save();
        refreshUser(id);
    }
}/*}}}*/

void
ManglerChannelTree::muteUserGlobalMenuItem_activate_cb(void) {/*{{{*/
}/*}}}*/

uint16_t
ManglerChannelTree::getUserChannelId(uint16_t userid) {/*{{{*/
    Gtk::TreeModel::Row user;
    if (! (user = getUser(userid)) && userid > 0) {
        fprintf(stderr, "getUserChannelId: could not find user id %d\n", userid);
        return 0;
    }
    return(user[channelRecord.parent_id]);
}/*}}}*/

Glib::ustring
ManglerChannelTree::getChannelSavedPassword(uint16_t channel_id) {/*{{{*/
    Gtk::TreeModel::Row channel = getChannel(channel_id, channelStore->children());
    Glib::ustring pw = channel[channelRecord.password];
    if (pw.length() == 0) {
        if (! mangler->connectedServerName.empty()) {
            pw = Mangler::config.ChannelPassword(mangler->connectedServerName, channel_id).toCString();
        }
    }
    return(pw);
}/*}}}*/

void
ManglerChannelTree::setChannelSavedPassword(uint16_t channel_id, Glib::ustring password) {/*{{{*/
    Gtk::TreeModel::Row channel = getChannel(channel_id, channelStore->children());
    channel[channelRecord.password] = password;
    if (! mangler->connectedServerName.empty()) {
        Mangler::config.ChannelPassword(mangler->connectedServerName, channel_id) = password;
        Mangler::config.servers.save();
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
    if (! mangler->connectedServerName.empty()) {
        v3_user *u;
        if ((u = v3_get_user(id)) != NULL) {
            Mangler::config.UserVolume(mangler->connectedServerName, u->name) = volumeAdjustment->get_value();
            Mangler::config.servers.save();
        }
    }
    v3_set_volume_user(id, (int)volumeAdjustment->get_value());
}/*}}}*/

Glib::ustring
ManglerChannelTree::getLastTransmit(uint16_t userid) {/*{{{*/
    Gtk::TreeModel::Row user;
    if (! (user = getUser(userid)) && userid > 0) {
        fprintf(stderr, "getLastTransmit: could not find user id %d\n", userid);
        return NULL;
    }
    return(user[channelRecord.last_transmit]);
}/*}}}*/

bool
ManglerChannelTree::isMuted(uint16_t userid) {/*{{{*/
    Gtk::TreeModel::Row user;
    if (! (user = getUser(userid)) && userid > 0) {
        fprintf(stderr, "isMuted: could not find user id %d\n", userid);
        return false;
    }
    return(user[channelRecord.muted]);
}/*}}}*/

void
ManglerChannelTree::muteUserToggle(uint16_t userid) {/*{{{*/
    Gtk::TreeModel::Row user;
    if (! (user = getUser(userid)) && userid > 0) {
        fprintf(stderr, "muteUser: could not find user id %d\n", userid);
        return;
    }
    user[channelRecord.muted] = user[channelRecord.muted] ? false : true;
    return;
}/*}}}*/

void
ManglerChannelTree::setLastTransmit(uint16_t userid, Glib::ustring last_transmit) {/*{{{*/
    Gtk::TreeModel::Row user;
    if (! (user = getUser(userid)) && userid > 0) {
        fprintf(stderr, "setLastTransmit: could not find user id %d\n", userid);
        return;
    }
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

Glib::RefPtr<ManglerChannelStore>
ManglerChannelStore::create() {/*{{{*/
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

void
ManglerChannelTree::userSettingsWindow(Gtk::TreeModel::Row row) {/*{{{*/
    int id = row[channelRecord.id];
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
}/*}}}*/
/*
 * Arbitrary channel changing (default channels)
 */
bool
ManglerChannelTree::channelView_getPathFromId(const Gtk::TreeModel::Path &path, uint32_t id, Glib::ustring *r_path) {/*{{{*/
    Gtk::TreeModel::iterator iter = channelStore->get_iter(path);
    if((uint32_t)(*iter)[channelRecord.id] == id) {
        r_path->assign(path.to_string());
        return true;
    }
    return false;
}/*}}}*/

bool
ManglerChannelTree::channelView_findDefault(const Gtk::TreeModel::iterator &iter, Gtk::TreeModel::iterator *oldDefault) {/*{{{*/
    if((bool)(*iter)[channelRecord.isDefault] && !(bool)(*iter)[channelRecord.isUser]) {
        *oldDefault = iter;
        return true;
    }
    return false;
}/*}}}*/

void
ManglerChannelTree::setDefaultChannel_toggled_cb(void) {/*{{{*/
    Glib::RefPtr<Gtk::TreeSelection> sel = channelView->get_selection();
    Gtk::TreeModel::iterator iter = sel->get_selected();
    if(iter) {
        Gtk::TreeModel::Row row = *iter;
        if (row[channelRecord.isUser] || mangler->connectedServerName.empty()) {
            //Silently failing on users, or quick connected servers... they shouldn't happen
            return;
        }
        //Initial checks are done, time to get to work
        bool isDefault = row[channelRecord.isDefault];
        Glib::ustring name = row[channelRecord.name];
        //Already default, so unset it's status
        if(isDefault) {
            row[channelRecord.isDefault] = false;
            Mangler::config.servers[mangler->connectedServerName]["DefaultChannelID"] = 0;
        } else {
            //Not default, so we need to unset the default if one exists, active status will be handled when its opened
            Gtk::TreeModel::iterator oldDefault;
            channelStore->foreach_iter(sigc::bind(sigc::mem_fun(*this,&ManglerChannelTree::channelView_findDefault), &oldDefault));
            if (oldDefault) {
                Gtk::TreeModel::Row oldRow = *oldDefault;
                oldRow[channelRecord.isDefault] = false;
            }
            //Now, lets set this one active
            row[channelRecord.isDefault] = true;
            Mangler::config.servers[mangler->connectedServerName]["DefaultChannelID"]
                = (unsigned int)( row[channelRecord.id] );
        }
        Mangler::config.servers.save();
    }
}/*}}}*/

void
ManglerChannelTree::channelView_switchChannel2Default(uint32_t defaultChannelId) {/*{{{*/
    Glib::ustring defaultChannelPath = "";
    channelStore->foreach_path(sigc::bind(sigc::mem_fun(*this,&ManglerChannelTree::channelView_getPathFromId), defaultChannelId, &defaultChannelPath));
    if (defaultChannelPath == "") {
        fprintf(stderr, "default channel id %u is no longer valid\n", defaultChannelId);
        return;
    }
    Gtk::TreeModel::iterator iter = channelStore->get_iter(defaultChannelPath);
    if(iter) {
        Gtk::TreeModel::Row row  = *iter;
        Gtk::TreeModel::Row user;
        if (! (user = getUser(v3_get_user_id()))) {
            fprintf(stderr, "channelView_switchChannel2Default: could not find user id %d\n", v3_get_user_id());
            return;
        }
        if ((bool)row[channelRecord.isUser]) {    //Can't switch into a user
            return;
        }
        if ((uint32_t)(row[channelRecord.id] == (uint32_t)user[channelRecord.parent_id] )) {
            //No point switching to the same channel
            return;
        }
        if (mangler->connectedServerName.empty()) {
            //No quick connected servers
            return;
        }
        //If the one to go to isn't the lobby, lets do something
        uint32_t id = row[channelRecord.id];
        if (id != 0) {
            v3_channel *channel = v3_get_channel(id);
            Glib::ustring password;
            bool password_required = false;
            uint16_t pw_cid;
            Gtk::TreeModel::Row pwrow;

            if (! channel) {
                fprintf(stderr, "failed to retrieve channel information for channel id %d\n", id);
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
                //Mangler::config.ChannelPassword(mangler->connectedServerName, pw_cid) = password;
            }
            v3_free_channel(channel);
            if (password_required && password.empty()) {
                return;
            }
            v3_change_channel(id, (char *)password.c_str());
        }
    }
}/*}}}*/


/*
   The Alphanum Algorithm is an improved sorting algorithm for strings
   containing numbers.  Instead of sorting numbers in ASCII order like a
   standard sort, this algorithm sorts numbers in numeric order.

   The Alphanum Algorithm is discussed at http://www.DaveKoelle.com

   This implementation is Copyright (c) 2008 Dirk Jagdmann <doj@cubic.org>.
   It is a cleanroom implementation of the algorithm and not derived by
   other's works. In contrast to the versions written by Dave Koelle this
   source code is distributed with the libpng/zlib license.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
      distribution. */
int natsort(const char *l, const char *r) {/*{{{*/
    enum mode_t { STRING, NUMBER } mode=STRING;

    while(*l && *r)
    {
        if(mode == STRING)
        {
            char l_char, r_char;
            while((l_char=*l) && (r_char=*r))
            {
                // check if this are digit characters
                const bool l_digit=isdigit(l_char), r_digit=isdigit(r_char);
                // if both characters are digits, we continue in NUMBER mode
                if(l_digit && r_digit)
                {
                    mode=NUMBER;
                    break;
                }
                // if only the left character is a digit, we have a result
                if(l_digit) return -1;
                // if only the right character is a digit, we have a result
                if(r_digit) return +1;
                // compute the difference of both characters
                const int diff=l_char - r_char;
                // if they differ we have a result
                if(diff != 0) return diff;
                // otherwise process the next characters
                ++l;
                ++r;
            }
        }
        else // mode==NUMBER
        {
            // get the left number
            unsigned long l_int=0;
            while(*l && isdigit(*l))
            {
                // TODO: this can overflow
                l_int=l_int*10 + *l-'0';
                ++l;
            }

            // get the right number
            unsigned long r_int=0;
            while(*r && isdigit(*r))
            {
                // TODO: this can overflow
                r_int=r_int*10 + *r-'0';
                ++r;
            }

            // if the difference is not equal to zero, we have a comparison result
            const long diff=l_int-r_int;
            if(diff != 0)
                return diff;

            // otherwise we process the next substring in STRING mode
            mode=STRING;
        }
    }

    if(*r) return -1;
    if(*l) return +1;
    return 0;
}/*}}}*/

