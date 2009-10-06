/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
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
    channelView->append_column("Name", channelRecord.displayName);
}/*}}}*/

void
ManglerChannelTree::addUser(uint16_t id, uint16_t parent_id, std::string name, std::string comment, std::string phonetic, std::string url, std::string integration_text) {/*{{{*/
    std::string displayName = "";
    Gtk::TreeModel::Row parent;
    gsize tmp;

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
    channelRow[channelRecord.isUser]            = false;
    channelRow[channelRecord.id]                = id;
    channelRow[channelRecord.parent_id]         = parent_id;
    channelRow[channelRecord.name]              = name;
    channelRow[channelRecord.comment]           = comment;
    channelRow[channelRecord.phonetic]          = phonetic;
    channelRow[channelRecord.url]               = url;
    channelRow[channelRecord.integration_text]  = integration_text;
}/*}}}*/

void
ManglerChannelTree::addChannel(uint16_t id, uint16_t parent_id, std::string name, std::string comment, std::string phonetic) {/*{{{*/
    std::string displayName = "";
    Gtk::TreeModel::Row parent;
    gsize tmp;

    if (! (parent = getChannel(parent_id, channelStore->children())) && id > 0) {
        fprintf(stderr, "orphaned channel: id %d: %s is supposed to be a child of %d\n", id, name.c_str(), parent_id);
        return;
    }
    displayName = name;
    if (! comment.empty()) {
        displayName = displayName + " (" + comment + ")";
    }
    if (parent) {
        channelIter                                 = channelStore->append(parent.children());
    } else {
        channelIter                                 = channelStore->append();
    }
    channelRow                                  = *channelIter;
    channelRow[channelRecord.displayName]       = g_locale_to_utf8(displayName.c_str(), -1, NULL, &tmp, NULL);
    channelRow[channelRecord.isUser]            = false;
    channelRow[channelRecord.id]                = id;
    channelRow[channelRecord.parent_id]         = parent_id;
    channelRow[channelRecord.name]              = name;
    channelRow[channelRecord.comment]           = comment;
    channelRow[channelRecord.phonetic]          = phonetic;
    channelRow[channelRecord.url]               = "";
    channelRow[channelRecord.integration_text]  = "";
}/*}}}*/

void
ManglerChannelTree::removeUser(uint16_t id) {/*{{{*/
    std::string displayName = "";
    Gtk::TreeModel::Row user;

    if (! (user = getChannel(id, channelStore->children())) && id > 0) {
        fprintf(stderr, "could not find user id %d to delete\n", id);
        return;
    }
    channelStore->erase(user);
}/*}}}*/

Gtk::TreeModel::Row
ManglerChannelTree::getChannel(uint16_t id, Gtk::TreeModel::Children children) {/*{{{*/
    Gtk::TreeModel::Children::iterator iter = children.begin();
    //std::cerr << "looking for id : " << id  << endl;
    while (iter != children.end()) {
        Gtk::TreeModel::Row row = *iter;
        int rowId = row[channelRecord.id];
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

Gtk::TreeModel::Row
ManglerChannelTree::getUser(uint16_t id, Gtk::TreeModel::Children children) {/*{{{*/
    Gtk::TreeModel::Children::iterator iter = children.begin();
    //std::cerr << "looking for id : " << id  << endl;
    while (iter != children.end()) {
        Gtk::TreeModel::Row row = *iter;
        int rowId = row[channelRecord.id];
        bool isUser = row[channelRecord.isUser];
        //std::cerr << "iterating: " << rowId << " | isUser: " << isUser << " | name: " << row[channelRecord.name] << endl;
        if (rowId == id && isUser == true) {
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

