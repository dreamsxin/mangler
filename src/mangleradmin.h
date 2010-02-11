/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2010-01-27 04:54:08 -0500 (Wed, 27 Jan 2010) $
 * $Revision: 567 $
 * $LastChangedBy: ekilfoil $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglerchat.h $
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

#ifndef _MANGLERADMIN_H
#define _MANGLERADMIN_H

extern "C" {
#include <ventrilo3.h>
}

class ManglerAdmin {
    public:
        ManglerAdmin(Glib::RefPtr<Gtk::Builder> builder);
        Gtk::Window         *adminWindow;
        void channelUpdated(v3_channel *channel);
        void channelRemoved(uint32_t chanid);
        void channelRemoved(v3_channel *channel);
        void channelAdded(v3_channel *channel);
        void channelSort(bool alphanumeric);
        void clearChannels(void);

    protected:
        Glib::RefPtr<Gtk::Builder>          builder;
        Gtk::Alignment                      *ServerTab;
        Gtk::Alignment                      *ChannelsTab;
        Gtk::Alignment                      *UsersTab;
        Gtk::Alignment                      *RanksTab;
        
       
        /* channel editor stuff */
        class adminModelColumns : public Gtk::TreeModel::ColumnRecord {
            public:
                adminModelColumns() { add(id); add(name); }
                Gtk::TreeModelColumn<uint32_t>              id;
                Gtk::TreeModelColumn<Glib::ustring>         name;
        } adminRecord;
        adminModelColumns                   ChannelEditorColumns;
        Glib::RefPtr<Gtk::TreeStore>        ChannelEditorTreeModel;
        Gtk::Frame                          *ChannelSpecificCodec;
        adminModelColumns                   ChannelCodecColumns;
        Glib::RefPtr<Gtk::TreeStore>        ChannelCodecModel;
        adminModelColumns                   ChannelFormatColumns;
        Glib::RefPtr<Gtk::TreeStore>        ChannelFormatModel;
        adminModelColumns                   ChannelProtColumns;
        Glib::RefPtr<Gtk::TreeStore>        ChannelProtModel;
        adminModelColumns                   ChannelVoiceColumns;
        Glib::RefPtr<Gtk::TreeStore>        ChannelVoiceModel;
        Gtk::TreeView                       *ChannelEditorTree;
        Gtk::VBox                           *ChannelEditor;
        Gtk::Button                         *ChannelRemove;
        Gtk::Button                         *ChannelAdd;
        uint32_t                            currentChannelID;
        uint32_t                            currentChannelParent;
        bool                                channelsortAlphanumeric;
        /* generic pointers and window pointer */
        Gtk::Button         *button;
        Gtk::Entry          *entry;
        Gtk::CheckButton    *checkbutton;
        Gtk::ComboBox       *combobox;
        Gtk::Label          *label;
        Gtk::SpinButton     *spinbutton;

        Gtk::TreeModel::Row getChannel(uint32_t id, Gtk::TreeModel::Children children);
        void populateChannelEditor(v3_channel *channel);
        int channelSortFunction(const Gtk::TreeModel::iterator &left, const Gtk::TreeModel::iterator &right);
        void ChannelTree_cursor_changed_cb(void);
        void AddChannel_clicked_cb(void);
        void RemoveChannel_clicked_cb(void);
        void UpdateChannel_clicked_cb(void);
        void CloseButton_clicked_cb(void);
        void LoadCodecFormats(void);
        void adminWindow_show_cb(void);
        void ChannelProtMode_changed_cb(void);
        void ChannelVoiceMode_changed_cb(void);
};

#endif
