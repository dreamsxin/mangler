/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 *
 * $LastChangedDate: 2010-02-02 00:40:26 -0500 (Tue, 02 Feb 2010) $
 * $Revision: 577 $
 * $LastChangedBy: ekilfoil $
 * $URL: http://svn.mangler.org/mangler/trunk/src/manglerchat.cpp $
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

#include "mangler.h"
#include "mangleradmin.h"
//#include <cstdlib>
//#include <cstring>

/* this is defined in channeltree.cpp */
int natsort(const char *l, const char *r);

ManglerAdmin::ManglerAdmin(Glib::RefPtr<Gtk::Builder> builder) {/*{{{*/
    this->builder = builder;
    Gtk::TreeModel::Row row;

    builder->get_widget("adminWindow", adminWindow);
    adminWindow->signal_show().connect(sigc::mem_fun(this, &ManglerAdmin::adminWindow_show_cb));

    builder->get_widget("ServerTab", ServerTab);
    builder->get_widget("ChannelsTab", ChannelsTab);
    builder->get_widget("UsersTab", UsersTab);
    builder->get_widget("RanksTab", RanksTab);

    builder->get_widget("ChannelEditorTree", ChannelEditorTree);
    ChannelEditorTreeModel = Gtk::TreeStore::create(ChannelEditorColumns);
    ChannelEditorTree->set_model(ChannelEditorTreeModel);
    Gtk::TreeView::Column* pColumn = Gtk::manage( new Gtk::TreeView::Column("Channels") );
    pColumn->pack_start(adminRecord.name);
    pColumn->set_expand(true);
    ChannelEditorTree->append_column(*pColumn);
    ChannelEditorTree->signal_cursor_changed().connect(sigc::mem_fun(this, &ManglerAdmin::ChannelTree_cursor_changed_cb));

    builder->get_widget("ChannelEditor", ChannelEditor);
    ChannelEditor->set_sensitive(false);

    builder->get_widget("ChannelRemove", ChannelRemove);
    ChannelRemove->set_sensitive(false);
    ChannelRemove->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::RemoveChannel_clicked_cb));
    
    builder->get_widget("ChannelAdd", ChannelAdd);
    ChannelAdd->set_sensitive(false);
    ChannelAdd->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::AddChannel_clicked_cb));
    
    builder->get_widget("ChannelUpdate", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::UpdateChannel_clicked_cb));
    
    currentChannelID = 0xffff;
    currentChannelParent = 0xffff;
    
    clearChannels();

    builder->get_widget("ChannelProtMode", combobox);
    ChannelProtModel = Gtk::TreeStore::create(ChannelProtColumns);
    combobox->set_model(ChannelProtModel);
    combobox->pack_start(adminRecord.name);
    row = *(ChannelProtModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "Open to Public";
    row = *(ChannelProtModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Channel Password";
    row = *(ChannelProtModel->append());
    row[adminRecord.id] = 2; row[adminRecord.name] = "User Authorization";
    combobox->signal_changed().connect(sigc::mem_fun(this, &ManglerAdmin::ChannelProtMode_changed_cb));

    builder->get_widget("ChannelVoiceMode", combobox);
    ChannelVoiceModel = Gtk::TreeStore::create(ChannelVoiceColumns);
    combobox->set_model(ChannelVoiceModel);
    combobox->pack_start(adminRecord.name);
    row = *(ChannelVoiceModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "Normal";
    row = *(ChannelVoiceModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Queued";
    row = *(ChannelVoiceModel->append());
    row[adminRecord.id] = 2; row[adminRecord.name] = "Muted";
    combobox->signal_changed().connect(sigc::mem_fun(this, &ManglerAdmin::ChannelVoiceMode_changed_cb));
    
    builder->get_widget("ChannelSpecificCodec", ChannelSpecificCodec);

    builder->get_widget("ChannelCodec", combobox);
    ChannelCodecModel = Gtk::TreeStore::create(ChannelCodecColumns);
    combobox->set_model(ChannelCodecModel);
    combobox->pack_start(adminRecord.name);
    row = *(ChannelCodecModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "GSM";
    row = *(ChannelCodecModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Codec 1";
    row = *(ChannelCodecModel->append());
    row[adminRecord.id] = 2; row[adminRecord.name] = "Codec 2";
    row = *(ChannelCodecModel->append());
    row[adminRecord.id] = 3; row[adminRecord.name] = "Speex";
    row = *(ChannelCodecModel->append());
    row[adminRecord.id] = 4; row[adminRecord.name] = "Server Default";
    combobox->signal_changed().connect(sigc::mem_fun(this, &ManglerAdmin::LoadCodecFormats));
    
    builder->get_widget("ChannelFormat", combobox);
    ChannelFormatModel = Gtk::TreeStore::create(ChannelCodecColumns);
    combobox->set_model(ChannelFormatModel);
    combobox->pack_start(adminRecord.name);
    
    builder->get_widget("CloseButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::CloseButton_clicked_cb));

}/*}}}*/

void
ManglerAdmin::ChannelTree_cursor_changed_cb() {/*{{{*/
    Gtk::TreeModel::Path path;
    Gtk::TreeViewColumn *column;
    ChannelEditorTree->get_cursor(path, column);
    Gtk::TreeModel::iterator iter = ChannelEditorTreeModel->get_iter(path);
    Gtk::TreeModel::Row row = *iter;
    currentChannelID = row[adminRecord.id];
    
    if (currentChannelID) {
        // load channel data into editor
        v3_channel *channel = v3_get_channel(currentChannelID);
        if (! channel) {
            fprintf(stderr, "failed to retrieve channel information for channel id %d\n", currentChannelID);
            currentChannelID = 0xffff;
            currentChannelParent = 0xffff;
            return;
        }
        populateChannelEditor(channel);
        v3_free_channel(channel);
    }
    // get user permissions
    const v3_permissions *perms = v3_get_permissions();
    // enable or disable editor and necessary buttons
    bool isCA( perms->srv_admin || mangler->isAdmin || v3_is_channel_admin(currentChannelID) );
    if (isCA) {
        if (ChannelProtModel->children().size() == 2) {
            row = *(ChannelProtModel->append());
            row[adminRecord.id] = 2;
            row[adminRecord.name] = "User Authorization";
        }
    } else {
        Gtk::TreeModel::Children items = ChannelProtModel->children();
        if (items.size() == 3) {
            iter = items.begin();   // 0
            if (iter) iter++;       // 1
            if (iter) iter++;       // 2 !!
            if (iter) ChannelProtModel->erase(iter);
        }
    }
    ChannelEditor->set_sensitive(isCA && currentChannelID);
    ChannelRemove->set_sensitive(isCA && currentChannelID);
    ChannelAdd->set_sensitive(isCA);
}/*}}}*/
Gtk::TreeModel::Row
ManglerAdmin::getChannel(uint32_t id, Gtk::TreeModel::Children children) {/*{{{*/
    Gtk::TreeModel::Children::iterator iter = children.begin();
    while (iter != children.end()) {
        Gtk::TreeModel::Row row = *iter;
        uint32_t rowId = row[adminRecord.id];
        if (rowId == id) {
            return row;
        }
        if (row.children().size()) {
            if (row = getChannel(id, row->children())) {
                return row;
            }
        }
        iter++;
    }
    return *iter;
}/*}}}*/
void
ManglerAdmin::channelUpdated(v3_channel *channel) {/*{{{*/
    Gtk::TreeModel::Row chanrow;
    if (! (chanrow = getChannel(channel->id, ChannelEditorTreeModel->children()))) {
        fprintf(stderr, "channel missing: id: %d - name: %s - parent; %d\n", channel->id, channel->name, channel->parent);
    }
    chanrow[adminRecord.id] = channel->id;
    chanrow[adminRecord.name] = channel->name;
    if (currentChannelID == channel->id) {
        populateChannelEditor(channel);
    }
}/*}}}*/
void
ManglerAdmin::channelRemoved(uint32_t chanid) {/*{{{*/
    Gtk::TreeModel::Row chanrow;
    if (chanid > 0 && ! (chanrow = getChannel(chanid, ChannelEditorTreeModel->children()))) {
        fprintf(stderr, "could not find channel id %d to delete\n", chanid);
        return;
    }
    ChannelEditorTreeModel->erase(chanrow);
}/*}}}*/
void
ManglerAdmin::channelRemoved(v3_channel *channel) {/*{{{*/
    channelRemoved(channel->id);
}/*}}}*/
void
ManglerAdmin::channelAdded(v3_channel *channel) {/*{{{*/
    Gtk::TreeModel::Row parent;
    Gtk::TreeModel::iterator channelIter;
    Gtk::TreeModel::Row channelRow;
    parent = getChannel(channel->parent, ChannelEditorTreeModel->children());
    if (parent) {
        channelIter = ChannelEditorTreeModel->append(parent.children());
    } else {
        channelIter = ChannelEditorTreeModel->append();
    }
    channelRow = *channelIter;
    channelRow[adminRecord.id] = channel->id;
    channelRow[adminRecord.name] = channel->name;
}/*}}}*/
void
ManglerAdmin::populateChannelEditor(v3_channel *channel) {/*{{{*/
    currentChannelID = channel->id;
    currentChannelParent = channel->parent;
    //fprintf(stderr, "Populate: channel %lu, parent %lu\n", currentChannelID, currentChannelParent);
    builder->get_widget("ChannelName", entry);
    entry->set_text(channel->name);
    builder->get_widget("ChannelPhonetic", entry);
    entry->set_text(channel->phonetic);
    builder->get_widget("ChannelComment", entry);
    entry->set_text(channel->comment);
    builder->get_widget("ChannelPassword", entry);
    entry->set_text("");
    builder->get_widget("ChannelProtMode", combobox);
    combobox->set_active(channel->protect_mode);
    builder->get_widget("ChannelVoiceMode", combobox);
    combobox->set_active(channel->voice_mode);
    builder->get_widget("AllowRecording", checkbutton);
    checkbutton->set_active(channel->allow_recording);
    builder->get_widget("AllowCCxmit", checkbutton);
    checkbutton->set_active(channel->allow_cross_channel_transmit);
    builder->get_widget("AllowPaging", checkbutton);
    checkbutton->set_active(channel->allow_paging);
    builder->get_widget("AllowWaveBinds", checkbutton);
    checkbutton->set_active(channel->allow_wave_file_binds);
    builder->get_widget("AllowTTSBinds", checkbutton);
    checkbutton->set_active(channel->allow_tts_binds);
    builder->get_widget("AllowU2Uxmit", checkbutton);
    checkbutton->set_active(channel->allow_u2u_transmit);
    builder->get_widget("AllowPhantoms", checkbutton);
    checkbutton->set_active(channel->allow_phantoms);
    builder->get_widget("AllowGuests", checkbutton);
    checkbutton->set_active(channel->allow_guests);
    builder->get_widget("AllowVoiceTargets", checkbutton);
    checkbutton->set_active(channel->allow_voice_target);
    builder->get_widget("AllowCommandTargets", checkbutton);
    checkbutton->set_active(channel->allow_command_target);
    builder->get_widget("TimerExempt", checkbutton);
    checkbutton->set_active(channel->inactive_exempt);
    builder->get_widget("MuteGuests", checkbutton);
    checkbutton->set_active(channel->disable_guest_transmit);
    builder->get_widget("DisableSoundEvents", checkbutton);
    checkbutton->set_active(channel->disable_sound_events);
    builder->get_widget("ChannelCodec", combobox);
    if (v3_is_licensed()) {
        if (channel->channel_codec == 0xffff) {
            combobox->set_active(4);
        } else {
            combobox->set_active(channel->channel_codec);
            builder->get_widget("ChannelFormat", combobox);
            combobox->set_active(channel->channel_format);
        }
        ChannelSpecificCodec->set_sensitive(true);
        builder->get_widget("AllowVoiceTargets", checkbutton);
        checkbutton->set_sensitive(true);
        builder->get_widget("AllowCommandTargets", checkbutton);
        checkbutton->set_sensitive(true);
    } else {
        combobox->set_active(4);
        ChannelSpecificCodec->set_sensitive(false);
        builder->get_widget("AllowVoiceTargets", checkbutton);
        checkbutton->set_sensitive(false);
        builder->get_widget("AllowCommandTargets", checkbutton);
        checkbutton->set_sensitive(false);
    }
    builder->get_widget("ChannelEditorLabel", label);
    Glib::ustring labelText = "Editing: ";
    labelText.append(channel->name);
    label->set_text(labelText);
    //LoadCodecFormats();
}/*}}}*/
void
ManglerAdmin::AddChannel_clicked_cb(void) {/*{{{*/
    builder->get_widget("ChannelName", entry);
    entry->set_text("");
    builder->get_widget("ChannelPhonetic", entry);
    entry->set_text("");
    builder->get_widget("ChannelComment", entry);
    entry->set_text("");
    builder->get_widget("ChannelPassword", entry);
    entry->set_text("");
    builder->get_widget("ChannelProtMode", combobox);
    combobox->set_active(0);
    builder->get_widget("ChannelVoiceMode", combobox);
    combobox->set_active(0);
    builder->get_widget("AllowRecording", checkbutton);
    checkbutton->set_active(true);
    builder->get_widget("AllowCCxmit", checkbutton);
    checkbutton->set_active(true);
    builder->get_widget("AllowPaging", checkbutton);
    checkbutton->set_active(true);
    builder->get_widget("AllowWaveBinds", checkbutton);
    checkbutton->set_active(true);
    builder->get_widget("AllowTTSBinds", checkbutton);
    checkbutton->set_active(true);
    builder->get_widget("AllowU2Uxmit", checkbutton);
    checkbutton->set_active(true);
    builder->get_widget("AllowPhantoms", checkbutton);
    checkbutton->set_active(true);
    builder->get_widget("AllowGuests", checkbutton);
    checkbutton->set_active(true);
    builder->get_widget("AllowVoiceTargets", checkbutton);
    checkbutton->set_active(true);
    builder->get_widget("AllowCommandTargets", checkbutton);
    checkbutton->set_active(true);
    builder->get_widget("TimerExempt", checkbutton);
    checkbutton->set_active(false);
    builder->get_widget("MuteGuests", checkbutton);
    checkbutton->set_active(false);
    builder->get_widget("DisableSoundEvents", checkbutton);
    checkbutton->set_active(false);
    builder->get_widget("ChannelCodec", combobox);
    combobox->set_active(4);
    bool isLicensed( v3_is_licensed() );
    ChannelSpecificCodec->set_sensitive(isLicensed);
    builder->get_widget("AllowVoiceTargets", checkbutton);
    checkbutton->set_sensitive(isLicensed);
    builder->get_widget("AllowCommandTargets", checkbutton);
    checkbutton->set_sensitive(isLicensed);
    builder->get_widget("ChannelEditorLabel", label);
    label->set_text("Editing: NEW CHANNEL");
    currentChannelParent = currentChannelID;
    currentChannelID = 0xffff;
    //fprintf(stderr, "Add: channel %lu, parent %lu\n", currentChannelID, currentChannelParent);
    ChannelAdd->set_sensitive(false);
    ChannelRemove->set_sensitive(false);
    ChannelEditor->set_sensitive(true);
}/*}}}*/
void
ManglerAdmin::RemoveChannel_clicked_cb(void) {/*{{{*/
    v3_channel_remove(currentChannelID);
}/*}}}*/
void
ManglerAdmin::UpdateChannel_clicked_cb(void) {/*{{{*/
    Gtk::TreeModel::iterator iter;
    v3_channel channel;
    Glib::ustring password;

    memset(&channel, 0, sizeof(v3_channel));

    if (currentChannelID == 0xffff) {
        // New Record
        channel.id = 0;
    } else {
        channel.id = currentChannelID;
    }
    channel.parent = currentChannelParent;
    //fprintf(stderr, "Update: channel %lu, parent %lu\n", channel.id, channel.parent);

    builder->get_widget("ChannelName", entry);
    channel.name = ::strdup(entry->get_text().c_str());
    builder->get_widget("ChannelPhonetic", entry);
    channel.phonetic = ::strdup(entry->get_text().c_str());
    builder->get_widget("ChannelComment", entry);
    channel.comment = ::strdup(entry->get_text().c_str());
    builder->get_widget("ChannelPassword", entry);
    password = entry->get_text().c_str();
    if (password.length()) {
        channel.password_protected = 1;
    }
    builder->get_widget("ChannelProtMode", combobox);
    iter = combobox->get_active();
    channel.protect_mode = (iter ? (*iter)[adminRecord.id] : 0);
    builder->get_widget("ChannelVoiceMode", combobox);
    iter = combobox->get_active();
    channel.voice_mode = (iter ? (*iter)[adminRecord.id] : 0);
    builder->get_widget("AllowRecording", checkbutton);
    channel.allow_recording = checkbutton->get_active();
    builder->get_widget("AllowCCxmit", checkbutton);
    channel.allow_cross_channel_transmit = checkbutton->get_active();
    builder->get_widget("AllowPaging", checkbutton);
    channel.allow_paging = checkbutton->get_active();
    builder->get_widget("AllowWaveBinds", checkbutton);
    channel.allow_wave_file_binds = checkbutton->get_active();
    builder->get_widget("AllowTTSBinds", checkbutton);
    channel.allow_tts_binds = checkbutton->get_active();
    builder->get_widget("AllowU2Uxmit", checkbutton);
    channel.allow_u2u_transmit = checkbutton->get_active();
    builder->get_widget("AllowPhantoms", checkbutton);
    channel.allow_phantoms = checkbutton->get_active();
    builder->get_widget("AllowGuests", checkbutton);
    channel.allow_guests = checkbutton->get_active();
    builder->get_widget("AllowVoiceTargets", checkbutton);
    channel.allow_voice_target = checkbutton->get_active();
    builder->get_widget("AllowCommandTargets", checkbutton);
    channel.allow_command_target = checkbutton->get_active();
    builder->get_widget("TimerExempt", checkbutton);
    channel.inactive_exempt = checkbutton->get_active();
    builder->get_widget("MuteGuests", checkbutton);
    channel.disable_guest_transmit = checkbutton->get_active();
    builder->get_widget("DisableSoundEvents", checkbutton);
    channel.disable_sound_events = checkbutton->get_active();
    if (v3_is_licensed()) {
        builder->get_widget("ChannelCodec", combobox);
        iter = combobox->get_active();
        if (iter && (*iter)[adminRecord.id] < 4) {
            channel.channel_codec = (*iter)[adminRecord.id];
            builder->get_widget("ChannelFormat", combobox);
            iter = combobox->get_active();
            channel.channel_format = (iter ? (*iter)[adminRecord.id] : 0);
        } else {
            channel.channel_codec = 0xffff;
            channel.channel_format = 0xffff;
        }
    } else {
        channel.channel_codec = 0xffff;
        channel.channel_format = 0xffff;
    }
    v3_channel_update(&channel, password.c_str());
    ::free(channel.name);
    ::free(channel.phonetic);
    ::free(channel.comment);
}/*}}}*/
void
ManglerAdmin::CloseButton_clicked_cb(void) {/*{{{*/
    adminWindow->hide();
}/*}}}*/
void
ManglerAdmin::channelSort(bool alphanumeric) {/*{{{*/
    channelsortAlphanumeric = alphanumeric;
    if (alphanumeric) {
        ChannelEditorTreeModel->set_sort_func(0, sigc::mem_fun (*this, &ManglerAdmin::channelSortFunction));
        ChannelEditorTreeModel->set_sort_column(adminRecord.name, Gtk::SORT_ASCENDING);
    } else {
        ChannelEditorTreeModel->set_sort_func(adminRecord.id, sigc::mem_fun (*this, &ManglerAdmin::channelSortFunction));
        ChannelEditorTreeModel->set_sort_column(adminRecord.id, Gtk::SORT_ASCENDING);
    }
}/*}}}*/
int
ManglerAdmin::channelSortFunction(const Gtk::TreeModel::iterator &left, const Gtk::TreeModel::iterator &right) {/*{{{*/
    if (channelsortAlphanumeric) {
        Glib::ustring leftstr = (*left)[adminRecord.name];
        Glib::ustring rightstr = (*right)[adminRecord.name];
        return natsort(leftstr.c_str(), rightstr.c_str());
    } else {
        if ((*left)[adminRecord.id] < (*right)[adminRecord.id]) {
            return -1;
        } else if ((*left)[adminRecord.id] > (*right)[adminRecord.id]) {
            return 1;
        } else {
            return 0;
        }
    }
}/*}}}*/
void
ManglerAdmin::clearChannels(void) {/*{{{*/
    ChannelEditorTreeModel->clear();
    Gtk::TreeModel::Row lobby = *(ChannelEditorTreeModel->append());
    lobby[adminRecord.id] = 0;
    lobby[adminRecord.name] = "(Lobby)";
    currentChannelID = 0xffff;
    currentChannelParent = 0xffff;
    ChannelEditor->set_sensitive(false);
    ChannelRemove->set_sensitive(false);
    ChannelAdd->set_sensitive(false);
}/*}}}*/
void
ManglerAdmin::LoadCodecFormats(void) {/*{{{*/
    builder->get_widget("ChannelCodec", combobox);
    uint16_t c = 4;
    Gtk::TreeModel::iterator iter = combobox->get_active();
    if (iter) c = (*iter)[adminRecord.id];
    builder->get_widget("ChannelFormat", combobox);
    combobox->set_sensitive(c < 4);
    uint16_t f = 0;
    const v3_codec *codec;
    Gtk::TreeModel::Row row;
    ChannelFormatModel->clear();
    while ((codec = v3_get_codec(c, f))) {
        row = *(ChannelFormatModel->append());
        row[adminRecord.id] = f;
        row[adminRecord.name] = codec->name;
        f++;
    }
    if (c < 4 && ! combobox->get_active()) {
        combobox->set_active(0);
    } else {
        combobox->set_active(-1);
    }
}/*}}}*/
void
ManglerAdmin::adminWindow_show_cb(void) {/*{{{*/
    const v3_permissions *perms = v3_get_permissions();
#if 0
    if (perms->srv_admin) {
        ServerTab->show();
        UsersTab->show();
    } else {
        ServerTab->hide();
        UsersTab->hide();
    }
    if (perms->edit_rank) {
        RanksTab->show();
    } else {
        RanksTab->hide();
    }
#else
    ServerTab->hide();
    UsersTab->hide();
    RanksTab->hide();
#endif
    ChannelEditorTree->expand_all();
    Gtk::TreeModel::iterator iter = ChannelEditorTreeModel->children().begin();
    if (iter) {
        Gtk::TreeModel::Path lobbypath = ChannelEditorTreeModel->get_path(iter);
        ChannelEditorTree->set_cursor(lobbypath);
    }
}/*}}}*/
void
ManglerAdmin::ChannelProtMode_changed_cb(void) {/*{{{*/
    Gtk::TreeModel::iterator iter;
    builder->get_widget("ChannelProtMode", combobox);
    iter = combobox->get_active();
    bool isPassword( iter && (*iter)[adminRecord.id] == 1);
    builder->get_widget("ChannelPassword", entry);
    entry->set_sensitive(isPassword);
}/*}}}*/
void
ManglerAdmin::ChannelVoiceMode_changed_cb(void) {/*{{{*/
    Gtk::TreeModel::iterator iter;
    builder->get_widget("ChannelVoiceMode", combobox);
    iter = combobox->get_active();
    bool notNormal( iter && (*iter)[adminRecord.id] );
    builder->get_widget("TransmitRank", spinbutton);
    spinbutton->set_sensitive(notNormal);
}/*}}}*/
