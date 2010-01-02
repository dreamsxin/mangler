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

#ifndef _CHANNELTREE_H
#define _CHANNELTREE_H
#include <sys/types.h>

class channelModelColumns : public Gtk::TreeModel::ColumnRecord/*{{{*/
{
    public:
        channelModelColumns() {
            add(displayName);
            add(icon);
            add(isUser);
            add(isGuest);
            add(id);
            add(parent_id);
            add(name);
            add(comment);
            add(phonetic);
            add(url);
            add(integration_text);
            add(last_transmit);
            add(password);
        }

        Gtk::TreeModelColumn<Glib::ustring>                 displayName;
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> >    icon;
        Gtk::TreeModelColumn<bool>                          isUser;
        Gtk::TreeModelColumn<bool>                          isGuest;
        Gtk::TreeModelColumn<uint32_t>                      id;
        Gtk::TreeModelColumn<uint32_t>                      parent_id;
        Gtk::TreeModelColumn<Glib::ustring>                 name;
        Gtk::TreeModelColumn<Glib::ustring>                 comment;
        Gtk::TreeModelColumn<Glib::ustring>                 phonetic;
        Gtk::TreeModelColumn<Glib::ustring>                 url;
        Gtk::TreeModelColumn<Glib::ustring>                 integration_text;
        Gtk::TreeModelColumn<Glib::ustring>                 last_transmit;
        Gtk::TreeModelColumn<Glib::ustring>                 password;
};/*}}}*/
class ManglerChannelStore : public Gtk::TreeStore
{
    public:
        static Glib::RefPtr<ManglerChannelStore> create();
        channelModelColumns                 c;

    protected:
        ManglerChannelStore() {
            set_column_types(c);
        }
        virtual bool row_draggable_vfunc(const Gtk::TreeModel::Path& path) const;
        virtual bool row_drop_possible_vfunc(const Gtk::TreeModel::Path& dest, const Gtk::SelectionData& selection_data) const;
        virtual bool drag_data_received_vfunc(const Gtk::TreeModel::Path& dest, const Gtk::SelectionData& selection_data);
};

class ManglerChannelTree
{
    private:
        Glib::RefPtr<Gtk::Builder>          builder;
        channelModelColumns                 channelRecord;
        Glib::RefPtr<ManglerChannelStore>   channelStore;
        Gtk::TreeModel::iterator            channelIter;
        Gtk::TreeModel::Row                 channelRow;
        Gtk::TreeViewColumn                 *column;
        Gtk::CellRendererPixbuf             *pixrenderer;
        Gtk::CellRendererText               *textrenderer;
        Gtk::MenuItem                       *menuitem;
        void renderCellData(Gtk::CellRenderer *cell, const Gtk::TreeModel::iterator& iter);

    public:
        ManglerChannelTree(Glib::RefPtr<Gtk::Builder> builder);
        Gtk::TreeView                       *channelView;
        Gtk::Menu                           *rcmenu_user;
        Gtk::Menu                           *rcmenu_channel;
        Gtk::Window                         *window;
        Gtk::Label                          *label;
        Gtk::VScale                         *volumevscale;
        Gtk::Adjustment                     *volumeAdjustment;
        sigc::connection                    volumeAdjustSignalConnection;
        Gtk::VBox                           *vbox;
        void addChannel(uint8_t protect_mode, uint32_t id, uint32_t parent_id, Glib::ustring name, Glib::ustring comment = "", Glib::ustring phonetic = "");
        void addUser(uint32_t id, uint32_t channel, Glib::ustring name, Glib::ustring comment = "", Glib::ustring phonetic = "", Glib::ustring url = "", Glib::ustring integration_text = "", bool guest = false, bool phantom = false);
        void updateLobby(Glib::ustring name, Glib::ustring comment = "", Glib::ustring phonetic = "");
        void updateUser(uint32_t id, uint32_t parent_id, Glib::ustring name, Glib::ustring comment, Glib::ustring phonetic, Glib::ustring url, Glib::ustring integration_text, bool guest, bool phantom);
        void updateChannel(uint8_t protect_mode, uint32_t id, uint32_t parent_id, Glib::ustring name, Glib::ustring comment, Glib::ustring phonetic);
        Glib::ustring getLastTransmit(uint16_t userid);
        void setLastTransmit(uint16_t userid, Glib::ustring last_transmit);
        void removeUser(uint32_t id);
        void removeChannel(uint32_t id);
        void setUserIcon(uint16_t id, Glib::ustring color);
        Gtk::TreeModel::Row getChannel(uint32_t id, Gtk::TreeModel::Children children);
        Gtk::TreeModel::Row getUser(uint32_t id, Gtk::TreeModel::Children children);
        uint16_t getUserChannelId(uint16_t userid);
        Glib::ustring getChannelSavedPassword(uint16_t channel_id);
        void setChannelSavedPassword(uint16_t channel_id, Glib::ustring password);
        void forgetChannelSavedPassword(uint16_t channel_id);
        bool expand_all(void);
        bool collapse_all(void);
        void clear(void);

        void channelView_row_activated_cb(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
        void channelView_buttonpress_event_cb(GdkEventButton* event);
        bool channelView_drag_drop_cb(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, guint time);
        void copyCommentMenuItem_activate_cb(void);
        void copyURLMenuItem_activate_cb(void);
        void addPhantomMenuItem_activate_cb(void);
        void removePhantomMenuItem_activate_cb(void);
        void volumeAdjustment_value_changed_cb(uint16_t);

};

Glib::ustring getTimeString(void);


#endif
