/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 */

#ifndef _CHANNELTREE_H
#define _CHANNELTREE_H

class channelModelColumns : public Gtk::TreeModelColumnRecord/*{{{*/
{
    public:
        channelModelColumns() {
            add(displayName);
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
        Gtk::TreeModelColumn<bool>          isUser;
        Gtk::TreeModelColumn<uint32_t>      id;
        Gtk::TreeModelColumn<uint32_t>      parent_id;
        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<Glib::ustring> comment;
        Gtk::TreeModelColumn<Glib::ustring> phonetic;
        Gtk::TreeModelColumn<Glib::ustring> url;
        Gtk::TreeModelColumn<Glib::ustring> integration_text;
};/*}}}*/
class ManglerChannelTree/*{{{*/
{
    private:
        Glib::RefPtr<Gtk::Builder>          builder;
        channelModelColumns                 channelRecord;
        Glib::RefPtr<Gtk::TreeStore>        channelStore;
        Gtk::TreeModel::iterator            channelIter;
        Gtk::TreeModel::Row                 channelRow;
        Gtk::TreeViewColumn                 *column;
        Gtk::CellRendererText               *renderer;
        void renderCellData(Gtk::CellRenderer *cell, const Gtk::TreeModel::iterator& iter);

    public:
        ManglerChannelTree(Glib::RefPtr<Gtk::Builder> builder);
        Gtk::TreeView                       *channelView;
        void addChannel(uint32_t id, uint32_t parent_id, std::string name, std::string comment = "", std::string phonetic = "");
        void addUser(uint32_t id, uint32_t channel, std::string name, std::string comment = "", std::string phonetic = "", std::string url = "", std::string integration_text = "");
        void updateLobby(std::string name, std::string comment = "", std::string phonetic = "");
        void removeUser(uint32_t id);
        Gtk::TreeModel::Row getChannel(uint32_t id, Gtk::TreeModel::Children children);
        Gtk::TreeModel::Row getUser(uint32_t id, Gtk::TreeModel::Children children);
        bool expand_all(void);
        bool collapse_all(void);
};/*}}}*/

#endif
