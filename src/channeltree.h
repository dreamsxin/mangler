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

#ifndef _CHANNELTREE_H
#define _CHANNELTREE_H

class channelModelColumns : public Gtk::TreeModelColumnRecord/*{{{*/
{
    public:
        channelModelColumns() {
            add(displayName);
            add(icon);
            add(isUser);
            add(id);
            add(parent_id);
            add(name);
            add(comment);
            add(phonetic);
            add(url);
            add(integration_text);
        }

        Gtk::TreeModelColumn<Glib::ustring> displayName;
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
        Gtk::TreeModelColumn<bool>          isUser;
        Gtk::TreeModelColumn<uint32_t>      id;
        Gtk::TreeModelColumn<uint32_t>      parent_id;
        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<Glib::ustring> comment;
        Gtk::TreeModelColumn<Glib::ustring> phonetic;
        Gtk::TreeModelColumn<Glib::ustring> url;
        Gtk::TreeModelColumn<Glib::ustring> integration_text;
};/*}}}*/
class ManglerChannelTree
{
    private:
        Glib::RefPtr<Gtk::Builder>          builder;
        channelModelColumns                 channelRecord;
        Glib::RefPtr<Gtk::TreeStore>        channelStore;
        Gtk::TreeModel::iterator            channelIter;
        Gtk::TreeModel::Row                 channelRow;
        Gtk::TreeViewColumn                 *column;
        Gtk::CellRendererPixbuf             *pixrenderer;
        Gtk::CellRendererText               *textrenderer;
        void renderCellData(Gtk::CellRenderer *cell, const Gtk::TreeModel::iterator& iter);

    public:
        ManglerChannelTree(Glib::RefPtr<Gtk::Builder> builder);
        Gtk::TreeView                       *channelView;
        void addChannel(uint32_t id, uint32_t parent_id, std::string name, std::string comment = "", std::string phonetic = "");
        void addUser(uint32_t id, uint32_t channel, std::string name, std::string comment = "", std::string phonetic = "", std::string url = "", std::string integration_text = "");
        void updateLobby(std::string name, std::string comment = "", std::string phonetic = "");
        void removeUser(uint32_t id);
        void removeChannel(uint32_t id);
        void userIsTalking(uint16_t id, bool isTalking);
        Gtk::TreeModel::Row getChannel(uint32_t id, Gtk::TreeModel::Children children);
        Gtk::TreeModel::Row getUser(uint32_t id, Gtk::TreeModel::Children children);
        bool expand_all(void);
        bool collapse_all(void);
};

#endif
