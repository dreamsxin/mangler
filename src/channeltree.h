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
        Gtk::TreeModelColumn<int>           id;
        Gtk::TreeModelColumn<int>           parent_id;
        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<Glib::ustring> comment;
        Gtk::TreeModelColumn<Glib::ustring> phonetic;
        Gtk::TreeModelColumn<Glib::ustring> url;
        Gtk::TreeModelColumn<Glib::ustring> integration_text;

        void updateChannelList(void);
};/*}}}*/
class ManglerChannelTree/*{{{*/
{
    private:
        Glib::RefPtr<Gtk::Builder>          builder;
        channelModelColumns                 channelRecord;
        Glib::RefPtr<Gtk::TreeStore>        channelStore;
        Gtk::TreeModel::iterator            channelIter;
        Gtk::TreeModel::Row                 channelRow;

    public:
        ManglerChannelTree(Glib::RefPtr<Gtk::Builder> builder);
        Gtk::TreeView                       *channelView;
        void addChannel(uint16_t id, uint16_t parent_id, std::string name, std::string comment = "", std::string phonetic = "");
        void addUser(uint16_t id, uint16_t channel, std::string name, std::string comment = "", std::string phonetic = "", std::string url = "", std::string integration_text = "");
        Gtk::TreeModel::Row getChannel(uint16_t id, Gtk::TreeModel::Children children);
        void expand_all(void);
        void collapse_all(void);
};/*}}}*/

#endif
