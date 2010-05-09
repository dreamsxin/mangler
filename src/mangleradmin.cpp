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

#include "mangler.h"
#include "mangleradmin.h"
#include "manglercharset.h"
#include "inilib.h"

int natsort(const char *l, const char *r);

void
ManglerAdmin::trimString(Glib::ustring &s) {/*{{{*/
    return;
    int len = s.length();
    int left = 0;
    while (left < len && s[left] == ' ') left++;
    int right = len - 1;
    while (right > 0 && s[right] == ' ') right--;
    if (right < len - 1) s.erase(right + 1);
    if (left > 0) s.erase(0, left - 1);
}/*}}}*/

ManglerAdmin::ManglerAdmin(Glib::RefPtr<Gtk::Builder> builder) {/*{{{*/
    /* set up the basic window variables */
    this->builder = builder;
    Gtk::TreeModel::Row row;
    Gtk::TreeView::Column *pColumn; 
    Gtk::CellRendererText *renderer;

    builder->get_widget("adminWindow", adminWindow);
    adminWindow->signal_show().connect(sigc::mem_fun(this, &ManglerAdmin::adminWindow_show_cb));
    adminWindow->signal_hide().connect(sigc::mem_fun(this, &ManglerAdmin::adminWindow_hide_cb));
    builder->get_widget("CloseButton", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::CloseButton_clicked_cb));

    builder->get_widget("ServerTab", ServerTab);
    ServerTab->hide();
    builder->get_widget("ChannelsTab", ChannelsTab);
    builder->get_widget("UsersTab", UsersTab);
    builder->get_widget("RanksTab", RanksTab);
    builder->get_widget("BansTab", BansTab);
    builder->get_widget("AdminStatusbar", AdminStatusbar);
    AdminStatusbar->set_has_resize_grip(false);

    StatusbarTime = ::time(NULL);
    StatusbarCount = 0;

    Glib::signal_timeout().connect_seconds(sigc::mem_fun(this, &ManglerAdmin::statusbarPop), 1);
    
    /* set up the server settings editor stuff */
    builder->get_widget("ServerChatFilter", combobox);
    SrvChatFilterModel = Gtk::TreeStore::create(adminRecord);
    combobox->set_model(SrvChatFilterModel);
    combobox->pack_start(adminRecord.name);
    row = *(SrvChatFilterModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "Global to Server";
    row = *(SrvChatFilterModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Per Channel";
    
    builder->get_widget("ServerChannelOrdering", combobox);
    SrvChanOrderModel = Gtk::TreeStore::create(adminRecord);
    combobox->set_model(SrvChanOrderModel);
    combobox->pack_start(adminRecord.name);
    row = *(SrvChanOrderModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "Sort Alphabetically";
    row = *(SrvChanOrderModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Manual";
    
    builder->get_widget("ServerAction", combobox);
    SrvInactActionModel = Gtk::TreeStore::create(adminRecord);
    combobox->set_model(SrvInactActionModel);
    combobox->pack_start(adminRecord.name);
    row = *(SrvInactActionModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "Kick User";
    row = *(SrvInactActionModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Move to Channel";
    
    builder->get_widget("ServerChannel", combobox);
    SrvInactChannelModel = Gtk::TreeStore::create(adminRecord);
    combobox->set_model(SrvInactChannelModel);
    combobox->pack_start(adminRecord.name);
    renderer = (Gtk::CellRendererText*)(*(combobox->get_cells().begin()));
    renderer->property_ellipsize() = Pango::ELLIPSIZE_MIDDLE;
    
    builder->get_widget("ServerSpamFilterChannelAction", combobox);
    SrvSpamFilterChannelModel = Gtk::TreeStore::create(adminRecord);
    combobox->set_model(SrvSpamFilterChannelModel);
    combobox->pack_start(adminRecord.name);
    row = *(SrvSpamFilterChannelModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "Do Nothing";
    row = *(SrvSpamFilterChannelModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Kick User";
    row = *(SrvSpamFilterChannelModel->append());
    row[adminRecord.id] = 2; row[adminRecord.name] = "Ban User";

    builder->get_widget("ServerSpamFilterChatAction", combobox);
    SrvSpamFilterChatModel = Gtk::TreeStore::create(adminRecord);
    combobox->set_model(SrvSpamFilterChatModel);
    combobox->pack_start(adminRecord.name);
    row = *(SrvSpamFilterChatModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "Do Nothing";
    row = *(SrvSpamFilterChatModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Kick User";
    row = *(SrvSpamFilterChatModel->append());
    row[adminRecord.id] = 2; row[adminRecord.name] = "Ban User";

    builder->get_widget("ServerSpamFilterCommentAction", combobox);
    SrvSpamFilterCommentModel = Gtk::TreeStore::create(adminRecord);
    combobox->set_model(SrvSpamFilterCommentModel);
    combobox->pack_start(adminRecord.name);
    row = *(SrvSpamFilterCommentModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "Do Nothing";
    row = *(SrvSpamFilterCommentModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Kick User";
    row = *(SrvSpamFilterCommentModel->append());
    row[adminRecord.id] = 2; row[adminRecord.name] = "Ban User";

    builder->get_widget("ServerSpamFilterTTSAction", combobox);
    SrvSpamFilterTTSModel = Gtk::TreeStore::create(adminRecord);
    combobox->set_model(SrvSpamFilterTTSModel);
    combobox->pack_start(adminRecord.name);
    row = *(SrvSpamFilterTTSModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "Do Nothing";
    row = *(SrvSpamFilterTTSModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Kick User";
    row = *(SrvSpamFilterTTSModel->append());
    row[adminRecord.id] = 2; row[adminRecord.name] = "Ban User";

    builder->get_widget("ServerSpamFilterWaveAction", combobox);
    SrvSpamFilterWaveModel = Gtk::TreeStore::create(adminRecord);
    combobox->set_model(SrvSpamFilterWaveModel);
    combobox->pack_start(adminRecord.name);
    row = *(SrvSpamFilterWaveModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "Do Nothing";
    row = *(SrvSpamFilterWaveModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Kick User";
    row = *(SrvSpamFilterWaveModel->append());
    row[adminRecord.id] = 2; row[adminRecord.name] = "Ban User";

    builder->get_widget("ServerUpdate", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::ServerUpdate_clicked_cb));

    /* set up the channel editor stuff */
    builder->get_widget("ChannelEditorTree", ChannelEditorTree);
    ChannelEditorTreeModel = Gtk::TreeStore::create(ChannelEditorColumns);
    ChannelEditorTree->set_model(ChannelEditorTreeModel);
    pColumn = Gtk::manage( new Gtk::TreeView::Column("Channels") );
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
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::ChannelUpdate_clicked_cb));

    currentChannelID = 0xffff;
    currentChannelParent = 0xffff;

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

    /* set up the user editor stuff */
    UserEditorTreeModel = Gtk::TreeStore::create(UserEditorColumns);
    builder->get_widget("UserEditorTree", UserEditorTree);
    UserEditorTree->set_model(UserEditorTreeModel);
    pColumn = Gtk::manage( new Gtk::TreeView::Column("Users") );
    pColumn->pack_start(adminRecord.name);
    pColumn->set_expand(true);
    UserEditorTree->append_column(*pColumn);
    UserEditorTree->signal_cursor_changed().connect(sigc::mem_fun(this, &ManglerAdmin::UserTree_cursor_changed_cb));

    UserChanAdminModel = Gtk::TreeStore::create(UserChanAdminColumns);
    builder->get_widget("UserChanAdminTree", UserChanAdminTree);
    UserChanAdminTree->set_model(UserChanAdminModel);
    pColumn = Gtk::manage( new Gtk::TreeView::Column("Channels") );
    pColumn->pack_start(adminCheckRecord.name);
    pColumn->set_expand(true);
    UserChanAdminTree->append_column_editable("Select", adminCheckRecord.on);
    UserChanAdminTree->append_column(*pColumn);

    UserChanAuthModel = Gtk::TreeStore::create(UserChanAuthColumns);
    builder->get_widget("UserChanAuthTree", UserChanAuthTree);
    UserChanAuthTree->set_model(UserChanAuthModel);
    pColumn = Gtk::manage( new Gtk::TreeView::Column("Channels") );
    pColumn->pack_start(adminCheckRecord.name);
    pColumn->set_expand(true);
    UserChanAuthTree->append_column_editable("Select", adminCheckRecord.on);
    UserChanAuthTree->append_column(*pColumn);

    builder->get_widget("UserInfoSection", UserInfoSection);
    builder->get_widget("UserNetworkSection", UserNetworkSection);
    builder->get_widget("UserTransmitSection", UserTransmitSection);
    builder->get_widget("UserDisplaySection", UserDisplaySection);
    builder->get_widget("UserAdminSection", UserAdminSection);

    builder->get_widget("UserEditor", UserEditor);

    builder->get_widget("UserOwner", combobox);
    UserOwnerModel = Gtk::TreeStore::create(UserEditorColumns);
    combobox->set_model(UserOwnerModel);
    pColumn = Gtk::manage( new Gtk::TreeView::Column("Owners") );
    pColumn->pack_start(adminCheckRecord.name);
    pColumn->set_expand(true);
    combobox->pack_start(adminRecord.name);

    builder->get_widget("UserRank", combobox);
    UserRankModel = Gtk::TreeStore::create(UserRankColumns);
    combobox->set_model(UserRankModel);
    combobox->pack_start(adminRecord.name);
    row = *(UserRankModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "None";

    builder->get_widget("UserDuplicateIPs", combobox);
    UserDuplicateIPsModel = Gtk::TreeStore::create(UserDuplicateIPsColumns);
    combobox->set_model(UserDuplicateIPsModel);
    combobox->pack_start(adminRecord.name);
    row = *(UserDuplicateIPsModel->append());
    row[adminRecord.id] = 0; row[adminRecord.name] = "No Limit";
    row = *(UserDuplicateIPsModel->append());
    row[adminRecord.id] = 1; row[adminRecord.name] = "Do Not Allow Duplicates";
    row = *(UserDuplicateIPsModel->append());
    row[adminRecord.id] = 2; row[adminRecord.name] = "2";
    row = *(UserDuplicateIPsModel->append());
    row[adminRecord.id] = 3; row[adminRecord.name] = "3";
    row = *(UserDuplicateIPsModel->append());
    row[adminRecord.id] = 4; row[adminRecord.name] = "4";
    row = *(UserDuplicateIPsModel->append());
    row[adminRecord.id] = 5; row[adminRecord.name] = "5";
    row = *(UserDuplicateIPsModel->append());
    row[adminRecord.id] = 6; row[adminRecord.name] = "6";
    row = *(UserDuplicateIPsModel->append());
    row[adminRecord.id] = 7; row[adminRecord.name] = "7";

    builder->get_widget("UserDefaultChannel", combobox);
    UserDefaultChannelModel = Gtk::TreeStore::create(UserDefaultChannelColumns);
    combobox->set_model(UserDefaultChannelModel);
    combobox->pack_start(adminRecord.name);
    renderer = (Gtk::CellRendererText*)(*(combobox->get_cells().begin()));
    renderer->property_ellipsize() = Pango::ELLIPSIZE_MIDDLE;

    builder->get_widget("UserAdd", UserAdd);
    UserAdd->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::UserAdd_clicked_cb));

    builder->get_widget("UserRemove", UserRemove);
    UserRemove->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::UserRemove_clicked_cb));

    currentUserID = 0xffff;

    builder->get_widget("UserUpdate", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::UserUpdate_clicked_cb));

    builder->get_widget("UserInfoButton", togglebutton);
    togglebutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerAdmin::UserInfoButton_toggled_cb));

    builder->get_widget("UserNetworkButton", togglebutton);
    togglebutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerAdmin::UserNetworkButton_toggled_cb));

    builder->get_widget("UserTransmitButton", togglebutton);
    togglebutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerAdmin::UserTransmitButton_toggled_cb));

    builder->get_widget("UserDisplayButton", togglebutton);
    togglebutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerAdmin::UserDisplayButton_toggled_cb));

    builder->get_widget("UserAdminButton", togglebutton);
    togglebutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerAdmin::UserAdminButton_toggled_cb));

    builder->get_widget("UserChanAdminButton", togglebutton);
    togglebutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerAdmin::UserChanAdminButton_toggled_cb));

    builder->get_widget("UserChanAuthButton", togglebutton);
    togglebutton->signal_toggled().connect(sigc::mem_fun(this, &ManglerAdmin::UserChanAuthButton_toggled_cb));

    builder->get_widget("UserTemplate", UserTemplate);
    UserTemplateModel = Gtk::TreeStore::create(UserTemplateColumns);
    UserTemplate->set_model(UserTemplateModel);
    UserTemplate->pack_start(adminRecord.name);
    UserTemplate->set_text_column(adminRecord.name);
    UserTemplate->signal_changed().connect(sigc::mem_fun(this, &ManglerAdmin::UserTemplate_changed_cb));

    builder->get_widget("UserTemplateLoad", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::UserTemplateLoad_clicked_cb));

    builder->get_widget("UserTemplateSave", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::UserTemplateSave_clicked_cb));

    /* set up the rank editor stuff */
    RankEditorModel = Gtk::TreeStore::create(RankEditorColumns);
    builder->get_widget("RankTree", RankEditorTree);
    RankEditorTree->set_model(RankEditorModel);
    RankEditorTree->append_column("Name", rankRecord.name);
    RankEditorTree->append_column("Level", rankRecord.level);
    RankEditorTree->append_column("Description", rankRecord.description);
    RankEditorTree->signal_cursor_changed().connect(sigc::mem_fun(this, &ManglerAdmin::RankEditorTree_cursor_changed_cb));

    builder->get_widget("RankEditor", RankEditor);

    builder->get_widget("RankAdd", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::RankAdd_clicked_cb));

    builder->get_widget("RankRemove", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::RankRemove_clicked_cb));

    builder->get_widget("RankUpdate", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::RankUpdate_clicked_cb));

    /* set up the ban editor stuff */
    BanEditorModel = Gtk::TreeStore::create(BanEditorColumns);
    builder->get_widget("BanTree", BanEditorTree);
    BanEditorTree->set_model(BanEditorModel);
    BanEditorTree->append_column("IP Address", banRecord.ip);
    BanEditorTree->append_column("Netmask", banRecord.netmask);
    BanEditorTree->append_column("User", banRecord.user);
    BanEditorTree->append_column("Admin", banRecord.by);
    BanEditorTree->append_column("Reason", banRecord.reason);
    BanEditorTree->signal_cursor_changed().connect(sigc::mem_fun(this, &ManglerAdmin::BanEditorTree_cursor_changed_cb));

    builder->get_widget("BanEditor", BanEditor);

    builder->get_widget("BanNetmask", combobox);
    BanNetmaskModel = Gtk::TreeStore::create(adminRecord);
    combobox->set_model(BanNetmaskModel);
    combobox->pack_start(adminRecord.name);
    for (uint32_t ctr = 8; _v3_bitmasks[ctr] != NULL; ctr++) {
        Gtk::TreeModel::iterator iter = BanNetmaskModel->append();
        (*iter)[adminRecord.id] = ctr;
        (*iter)[adminRecord.name] = c_to_ustring(_v3_bitmasks[ctr]);
        combobox->set_active(iter);
        BanNetmaskDefault = ctr;
    }

    builder->get_widget("BanAdd", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::BanAdd_clicked_cb));

    builder->get_widget("BanRemove", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::BanRemove_clicked_cb));

    builder->get_widget("BanUpdate", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerAdmin::BanUpdate_clicked_cb));

    /* set up the editor lists */
    readUserTemplates();
    clear();

    isOpen = false;
}/*}}}*/
ManglerAdmin::~ManglerAdmin() {/*{{{*/
    if (usertemplates) {
        delete usertemplates;
    }
}/*}}}*/

void
ManglerAdmin::show(void) {/*{{{*/
    adminWindow->present();
}/*}}}*/
void
ManglerAdmin::adminWindow_show_cb(void) {/*{{{*/
    const v3_permissions *perms = v3_get_permissions();
    if (perms->srv_admin) {
        v3_admin_ban_list();
        BansTab->show();
        v3_userlist_open();
        UsersTab->show();
        Gtk::TreeModel::Row row;
        row = *(UserOwnerModel->append());
        row[adminRecord.id] = 0; row[adminRecord.name] = "None";
        UserAdd->set_sensitive( perms->add_user );
        if (SrvIsNotUpdating) {
            v3_serverprop_open();
        }
    } else {
        ServerTab->hide();
        UsersTab->hide();
        BansTab->hide();
    }
    if (perms->edit_rank) {
        v3_ranklist_open();
        RanksTab->show();
    } else {
        RanksTab->hide();
    }
    ChannelEditorTree->expand_all();
    UserChanAdminTree->expand_all();
    UserChanAuthTree->expand_all();
    Gtk::TreeModel::iterator iter = ChannelEditorTreeModel->children().begin();
    if (iter) {
        Gtk::TreeModel::Path lobbypath = ChannelEditorTreeModel->get_path(iter);
        ChannelEditorTree->set_cursor(lobbypath);
    }
    isOpen = true;
}/*}}}*/
void
ManglerAdmin::adminWindow_hide_cb(void) {/*{{{*/
    const v3_permissions *perms = v3_get_permissions();
    if (perms->srv_admin) {
        if (SrvIsNotUpdating) {
            v3_serverprop_close();
        }
        ServerTab->hide();
        v3_userlist_close();
        UserEditorTreeModel->clear();
        UserOwnerModel->clear();
        UserEditor->set_sensitive(false);
        currentUserID = 0xffff;
        UserRemove->set_sensitive(false);
        UserAdd->set_sensitive(false);
    }
    if (perms->edit_rank) {
        v3_ranklist_close();
    }
    isOpen = false;
}/*}}}*/
void
ManglerAdmin::statusbarPush(Glib::ustring msg) {/*{{{*/
    AdminStatusbar->push(msg);
    StatusbarCount++;
    StatusbarTime = time(NULL);
}/*}}}*/
bool
ManglerAdmin::statusbarPop(void) {/*{{{*/
    if (StatusbarTime + 3 > ::time(NULL)) return true;
    while (StatusbarCount) {
        AdminStatusbar->pop();
        StatusbarCount--;
    }
    return true;
}/*}}}*/
void
ManglerAdmin::CloseButton_clicked_cb(void) {/*{{{*/
    adminWindow->hide();
}/*}}}*/
void
ManglerAdmin::copyToEntry(const char *widgetName, Glib::ustring src) {/*{{{*/
    builder->get_widget(widgetName, entry);
    //if (src) entry->set_text(src);
    //else entry->set_text("");
    entry->set_text(src);
}/*}}}*/
void
ManglerAdmin::copyToSpinbutton(const char *widgetName, uint32_t src) {/*{{{*/
    builder->get_widget(widgetName, spinbutton);
    spinbutton->set_value(src);
}/*}}}*/
void
ManglerAdmin::copyToCheckbutton(const char *widgetName, bool src) {/*{{{*/
    builder->get_widget(widgetName, checkbutton);
    checkbutton->set_active(src);
}/*}}}*/
void
ManglerAdmin::copyToCombobox(const char *widgetName, uint32_t src, uint32_t deflt) {/*{{{*/
    builder->get_widget(widgetName, combobox);
    Glib::RefPtr<const Gtk::TreeModel> mdl = combobox->get_model();
    Gtk::TreeModel::Children children = mdl->children();
    if (! children || ! children.size()) {
        combobox->set_sensitive(false);
        return;
    } else combobox->set_sensitive(true);
    Gtk::TreeModel::Children::iterator iter = children.begin();
    Gtk::TreeModel::Children::iterator dIter = iter;
    while (iter != children.end()) {
        if ((*iter)[adminRecord.id] == deflt) dIter = iter;
        if ((*iter)[adminRecord.id] == src) break;
        iter++;
    }
    if (iter == children.end()) combobox->set_active(dIter);
    else combobox->set_active(iter);
}/*}}}*/
Glib::ustring
ManglerAdmin::getFromEntry(const char *widgetName) {/*{{{*/
    builder->get_widget(widgetName, entry);
    return entry->get_text();
}/*}}}*/
uint32_t
ManglerAdmin::getFromSpinbutton(const char *widgetName) {/*{{{*/
    builder->get_widget(widgetName, spinbutton);
    return uint32_t( spinbutton->get_value() );
}/*}}}*/
bool
ManglerAdmin::getFromCheckbutton(const char *widgetName) {/*{{{*/
    builder->get_widget(widgetName, checkbutton);
    return checkbutton->get_active();
}/*}}}*/
uint32_t
ManglerAdmin::getFromCombobox(const char *widgetName, uint32_t deflt) {/*{{{*/
    builder->get_widget(widgetName, combobox);
    Gtk::TreeModel::iterator iter = combobox->get_active();
    if (iter) return (*iter)[adminRecord.id];
    else return deflt;
}/*}}}*/
void
ManglerAdmin::setWidgetSensitive(const char *widgetName, bool widgetSens) {/*{{{*/
    Gtk::Widget *w;
    builder->get_widget(widgetName, w);
    w->set_sensitive(widgetSens);
}/*}}}*/

/* ----------  Server Settings Related Methods  ---------- */
void
ManglerAdmin::ServerUpdate_clicked_cb(void) {/*{{{*/
    v3_server_prop prop;

    memset(&prop, 0, sizeof(v3_server_prop));
    strncpy(prop.server_comment, ustring_to_c(getFromEntry("ServerComment")).c_str(), sizeof(prop.server_comment));
    prop.chat_filter = getFromCombobox("ServerChatFilter");
    prop.channel_order = getFromCombobox("ServerChannelOrdering");
    prop.motd_always = getFromCheckbutton("ServerAlwaysDisplayMOTD");
    // guest accounts
    prop.max_guest = getFromSpinbutton("ServerMaxGuests");
    prop.autokick_time = getFromSpinbutton("ServerKickGuests");
    prop.autoban_time = getFromSpinbutton("ServerBanGuests");
    // inactivity
    prop.inactivity_timeout = getFromSpinbutton("ServerTimeout");
    prop.inactivity_action = getFromCombobox("ServerAction");
    prop.inactivity_channel = getFromCombobox("ServerChannel", 0);
    // spam filters
    prop.channel_spam_filter.action = getFromCombobox("ServerSpamFilterChannelAction");
    prop.channel_spam_filter.interval = getFromSpinbutton("ServerSpamFilterChannelInterval");
    prop.channel_spam_filter.times = getFromSpinbutton("ServerSpamFilterChannelTimes");
    prop.chat_spam_filter.action = getFromCombobox("ServerSpamFilterChatAction");
    prop.chat_spam_filter.interval = getFromSpinbutton("ServerSpamFilterChatInterval");
    prop.chat_spam_filter.times = getFromSpinbutton("ServerSpamFilterChatTimes");
    prop.comment_spam_filter.action = getFromCombobox("ServerSpamFilterCommentAction");
    prop.comment_spam_filter.interval = getFromSpinbutton("ServerSpamFilterCommentInterval");
    prop.comment_spam_filter.times = getFromSpinbutton("ServerSpamFilterCommentTimes");
    prop.tts_spam_filter.action = getFromCombobox("ServerSpamFilterTTSAction");
    prop.tts_spam_filter.interval = getFromSpinbutton("ServerSpamFilterTTSInterval");
    prop.tts_spam_filter.times = getFromSpinbutton("ServerSpamFilterTTSTimes");
    prop.wave_spam_filter.action = getFromCombobox("ServerSpamFilterWaveAction");
    prop.wave_spam_filter.interval = getFromSpinbutton("ServerSpamFilterWaveInterval");
    prop.wave_spam_filter.times = getFromSpinbutton("ServerSpamFilterWaveTimes");
    // bind filters
    prop.tts_bind_filter = getFromCheckbutton("ServerBindFilterTTS");
    prop.wave_bind_filter = getFromCheckbutton("ServerBindFilterWave");
    // remote status
    prop.rem_srv_comment = getFromCheckbutton("ServerRemoteStatusServerComment");
    prop.rem_chan_names = getFromCheckbutton("ServerRemoteStatusChannelNames");
    prop.rem_chan_comments = getFromCheckbutton("ServerRemoteStatusChannelComments");
    prop.rem_user_names = getFromCheckbutton("ServerRemoteStatusUserNames");
    prop.rem_user_comments = getFromCheckbutton("ServerRemoteStatusUserComments");
    prop.rem_show_login_names = getFromCheckbutton("ServerRemoteStatusUseless");

    setWidgetSensitive("ServerVBox", false);
    SrvIsNotUpdating = false;
    v3_serverprop_update(&prop);
    statusbarPush("Sending server properties...");
}/*}}}*/
void
ManglerAdmin::serverSettingsUpdated(v3_server_prop &prop) {/*{{{*/
    copyToEntry("ServerComment", c_to_ustring(prop.server_comment));
    copyToCombobox("ServerChatFilter", prop.chat_filter, 0);
    copyToCombobox("ServerChannelOrdering", prop.channel_order, 0);
    copyToCheckbutton("ServerAlwaysDisplayMOTD", prop.motd_always);
    // guest accounts
    copyToSpinbutton("ServerMaxGuests", prop.max_guest);
    copyToSpinbutton("ServerKickGuests", prop.autokick_time);
    copyToSpinbutton("ServerBanGuests", prop.autoban_time);
    // inactivity
    copyToSpinbutton("ServerTimeout", prop.inactivity_timeout);
    copyToCombobox("ServerAction", prop.inactivity_action, 0);
    copyToCombobox("ServerChannel", prop.inactivity_channel, 0);
    // spam filters
    copyToCombobox("ServerSpamFilterChannelAction", prop.channel_spam_filter.action, 0);
    copyToSpinbutton("ServerSpamFilterChannelInterval", prop.channel_spam_filter.interval);
    copyToSpinbutton("ServerSpamFilterChannelTimes", prop.channel_spam_filter.times);
    copyToCombobox("ServerSpamFilterChatAction", prop.chat_spam_filter.action, 0);
    copyToSpinbutton("ServerSpamFilterChatInterval", prop.chat_spam_filter.interval);
    copyToSpinbutton("ServerSpamFilterChatTimes", prop.chat_spam_filter.times);
    copyToCombobox("ServerSpamFilterCommentAction", prop.comment_spam_filter.action, 0);
    copyToSpinbutton("ServerSpamFilterCommentInterval", prop.comment_spam_filter.interval);
    copyToSpinbutton("ServerSpamFilterCommentTimes", prop.comment_spam_filter.times);
    copyToCombobox("ServerSpamFilterTTSAction", prop.tts_spam_filter.action, 0);
    copyToSpinbutton("ServerSpamFilterTTSInterval", prop.tts_spam_filter.interval);
    copyToSpinbutton("ServerSpamFilterTTSTimes", prop.tts_spam_filter.times);
    copyToCombobox("ServerSpamFilterWaveAction", prop.wave_spam_filter.action, 0);
    copyToSpinbutton("ServerSpamFilterWaveInterval", prop.wave_spam_filter.interval);
    copyToSpinbutton("ServerSpamFilterWaveTimes", prop.wave_spam_filter.times);
    // bind filters
    copyToCheckbutton("ServerBindFilterTTS", prop.tts_bind_filter);
    copyToCheckbutton("ServerBindFilterWave", prop.wave_bind_filter);
    // remote status
    copyToCheckbutton("ServerRemoteStatusServerComment", prop.rem_srv_comment);
    copyToCheckbutton("ServerRemoteStatusChannelNames", prop.rem_chan_names);
    copyToCheckbutton("ServerRemoteStatusChannelComments", prop.rem_chan_comments);
    copyToCheckbutton("ServerRemoteStatusUserNames", prop.rem_user_names);
    copyToCheckbutton("ServerRemoteStatusUserComments", prop.rem_user_comments);
    copyToCheckbutton("ServerRemoteStatusUseless", prop.rem_show_login_names);

    setWidgetSensitive("ServerVBox", true);
    SrvIsNotUpdating = true;
    ServerTab->show();
}/*}}}*/
void
ManglerAdmin::serverSettingsSendDone(void) {/*{{{*/
    statusbarPush("Sending server properties... done.");
    v3_serverprop_close();
    if (isOpen) {
        v3_serverprop_open();
    } else {
        setWidgetSensitive("ServerVBox", true);
        SrvIsNotUpdating = true;
    }
}/*}}}*/

/* ----------  Channel Editor Related Methods  ---------- */
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
    } else {
        builder->get_widget("ChannelEditorLabel", label);
        label->set_text("Editing: NONE");
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
ManglerAdmin::getChannel(uint32_t id, Gtk::TreeModel::Children children, bool hasCheckbox) {/*{{{*/
    Gtk::TreeModel::Children::iterator iter = children.begin();
    while (iter != children.end()) {
        Gtk::TreeModel::Row row = *iter;
        uint32_t rowId = hasCheckbox ? row[adminCheckRecord.id] : row[adminRecord.id];
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
Glib::ustring
ManglerAdmin::getChannelPathString(uint32_t id, Gtk::TreeModel::Children children) {/*{{{*/
    Gtk::TreeModel::Children::iterator iter = children.begin();
    while (iter != children.end()) {
        Gtk::TreeModel::Row row = *iter;
        uint32_t rowId = row[adminRecord.id];
        if (rowId == id) {
            return row[adminRecord.name];
        }
        if (row.children().size()) {
            Glib::ustring retstr = getChannelPathString(id, row->children());
            if (! retstr.empty()) {
                if (rowId == 0) return retstr;
                else return Glib::ustring::compose("%1 : %2", row[adminRecord.name], retstr);
            }
        }
        iter++;
    }
    return "";
}/*}}}*/
void
ManglerAdmin::channelUpdated(v3_channel *channel) {/*{{{*/
    /* channel editor tree */
    Gtk::TreeModel::Row chanrow;
    Gtk::TreeModel::Children::iterator iter;

    chanrow = getChannel(channel->id, ChannelEditorTreeModel->children());
    if (chanrow) {
        chanrow[adminRecord.id] = channel->id;
        chanrow[adminRecord.name] = c_to_ustring(channel->name);
        if (currentChannelID == channel->id) populateChannelEditor(channel);
    }
    /* server inactive "move to" channel combo box */
    Gtk::TreeModel::Children siChildren = SrvInactChannelModel->children();
    bool found( false );
    if (siChildren.size()) {
        iter = siChildren.begin();
        while (iter != siChildren.end()) {
            if ((*iter)[adminRecord.id] == channel->id) {
                found = true;
                break;
            }
            iter++;
        }
    }
    if (! found) iter = SrvInactChannelModel->append();
    (*iter)[adminRecord.id] = channel->id;
    (*iter)[adminRecord.name] = getChannelPathString(channel->id, ChannelEditorTreeModel->children());
    /* user default channel combo box */
    Gtk::TreeModel::Children udChildren = UserDefaultChannelModel->children();
    found = false;
    if (udChildren.size()) {
        iter = udChildren.begin();
        while (iter != udChildren.end()) {
            if ((*iter)[adminRecord.id] == channel->id) {
                found = true;
                break;
            }
            iter++;
        }
    }
    if (! found) iter = UserDefaultChannelModel->append();
    (*iter)[adminRecord.id] = channel->id;
    (*iter)[adminRecord.name] = getChannelPathString(channel->id, ChannelEditorTreeModel->children());
    /* user channel admin tree */
    chanrow = getChannel(channel->id, UserChanAdminModel->children(), true);
    if (chanrow) {
        chanrow[adminCheckRecord.id] = channel->id;
        chanrow[adminCheckRecord.name] = c_to_ustring(channel->name);
    }
    /* user channel auth tree */
    chanrow = getChannel(channel->id, UserChanAuthModel->children(), true);
    if (chanrow) {
        if (channel->protect_mode == 2) {
            chanrow[adminCheckRecord.id] = channel->id;
            chanrow[adminCheckRecord.name] = c_to_ustring(channel->name);
        } else UserChanAuthModel->erase(chanrow);
    } else if (channel->protect_mode == 2) {
        chanrow = *(UserChanAuthModel->append());
        if (chanrow) {
            chanrow[adminCheckRecord.id] = channel->id;
            chanrow[adminCheckRecord.name] = c_to_ustring(channel->name);
        }
    }
    /* update status bar */
    statusbarPush(Glib::ustring::compose("Channel %1 (%2) Updated.", Glib::ustring::format(channel->id), c_to_ustring(channel->name)));
}/*}}}*/
void
ManglerAdmin::channelRemoved(uint32_t chanid) {/*{{{*/
    /* channel editor tree */
    Gtk::TreeModel::Row chanrow;
    Gtk::TreeModel::Children::iterator iter;

    chanrow = getChannel(chanid, ChannelEditorTreeModel->children());
    if (chanrow) ChannelEditorTreeModel->erase(chanrow);
    /* server inactive "move to" channel combo box */
    Gtk::TreeModel::Children siChildren = SrvInactChannelModel->children();
    if (siChildren.size()) {
        iter = siChildren.begin();
        while (iter != siChildren.end()) {
            if ((*iter)[adminRecord.id] == chanid) {
                SrvInactChannelModel->erase(*iter);
                break;
            }
            iter++;
        }
    }
    /* user default channel combo box */
    Gtk::TreeModel::Children udChildren = UserDefaultChannelModel->children();
    if (udChildren.size()) {
        iter = udChildren.begin();
        while (iter != udChildren.end()) {
            if ((*iter)[adminRecord.id] == chanid) {
                UserDefaultChannelModel->erase(*iter);
                break;
            }
            iter++;
        }
    }
    /* user channel admin tree */
    chanrow = getChannel(chanid, UserChanAdminModel->children());
    if (chanrow) UserChanAdminModel->erase(chanrow);
    /* user channel auth tree */
    chanrow = getChannel(chanid, UserChanAuthModel->children());
    if (chanrow) UserChanAuthModel->erase(chanrow);
    /* update status bar */
    statusbarPush(Glib::ustring::compose("Channel %1 Removed.", Glib::ustring::format(chanid)));
}/*}}}*/
void
ManglerAdmin::channelRemoved(v3_channel *channel) {/*{{{*/
    channelRemoved(channel->id);
}/*}}}*/
void
ManglerAdmin::channelAdded(v3_channel *channel) {/*{{{*/
    /* channel editor tree */
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
    channelRow[adminRecord.name] = c_to_ustring(channel->name);
    /* server inactive "move to" channel combo box */
    channelIter = SrvInactChannelModel->append();
    (*channelIter)[adminRecord.id] = channel->id;
    (*channelIter)[adminRecord.name] = getChannelPathString(channel->id, ChannelEditorTreeModel->children());
    /* user default channel combo box */
    channelIter = UserDefaultChannelModel->append();
    (*channelIter)[adminRecord.id] = channel->id;
    (*channelIter)[adminRecord.name] = getChannelPathString(channel->id, ChannelEditorTreeModel->children());
    /* user channel admin tree */
    parent = getChannel(channel->parent, UserChanAdminModel->children(), true);
    if (parent) {
        channelIter = UserChanAdminModel->append(parent.children());
    } else {
        channelIter = UserChanAdminModel->append();
    }
    channelRow = *channelIter;
    channelRow[adminCheckRecord.id] = channel->id;
    channelRow[adminCheckRecord.name] = c_to_ustring(channel->name);
    /* user channel auth tree */
    if (channel->protect_mode == 2) {
        parent = getChannel(channel->parent, UserChanAuthModel->children(), true);
        if (parent) {
            channelIter = UserChanAuthModel->append(parent.children());
        } else {
            channelIter = UserChanAuthModel->append();
        }
        channelRow = *channelIter;
        channelRow[adminCheckRecord.id] = channel->id;
        channelRow[adminCheckRecord.name] = c_to_ustring(channel->name);
    }
    /* update status bar */
    statusbarPush(Glib::ustring::compose("Channel %1 (%2) Added.", Glib::ustring::format(channel->id), c_to_ustring(channel->name)));
}/*}}}*/
void
ManglerAdmin::populateChannelEditor(v3_channel *channel) {/*{{{*/
    currentChannelID = channel->id;
    currentChannelParent = channel->parent;
    //fprintf(stderr, "Populate: channel %lu, parent %lu\n", currentChannelID, currentChannelParent);
    copyToEntry("ChannelName", c_to_ustring(channel->name));
    copyToEntry("ChannelPhonetic", c_to_ustring(channel->phonetic));
    copyToEntry("ChannelComment", c_to_ustring(channel->comment));
    if (channel->password_protected) copyToEntry("ChannelPassword", "        ");
    else copyToEntry("ChannelPassword", "");
    copyToCombobox("ChannelProtMode", channel->protect_mode, 0);
    copyToCombobox("ChannelVoiceMode", channel->voice_mode, 0);
    copyToCheckbutton("AllowRecording", channel->allow_recording);
    copyToCheckbutton("AllowCCxmit", channel->allow_cross_channel_transmit);
    copyToCheckbutton("AllowPaging", channel->allow_paging);
    copyToCheckbutton("AllowWaveBinds", channel->allow_wave_file_binds);
    copyToCheckbutton("AllowTTSBinds", channel->allow_tts_binds);
    copyToCheckbutton("AllowU2Uxmit", channel->allow_u2u_transmit);
    copyToCheckbutton("AllowPhantoms", channel->allow_phantoms);
    copyToCheckbutton("AllowGuests", channel->allow_guests);
    copyToCheckbutton("AllowVoiceTargets", channel->allow_voice_target);
    copyToCheckbutton("AllowCommandTargets", channel->allow_command_target);
    copyToCheckbutton("TimerExempt", channel->inactive_exempt);
    copyToCheckbutton("MuteGuests", channel->disable_guest_transmit);
    copyToCheckbutton("DisableSoundEvents", channel->disable_sound_events);

    if (v3_is_licensed()) {
        copyToCombobox("ChannelCodec" , channel->channel_codec, 4);
        if (channel->channel_codec != 0xffff)
            copyToCombobox("ChannelFormat", channel->channel_format);
        builder->get_widget("ChannelCodecLabel", widget);
        widget->set_sensitive(true);
        builder->get_widget("ChannelCodec", widget);
        widget->set_sensitive(true);
        builder->get_widget("ChannelFormatLabel", widget);
        widget->set_sensitive(true);
        builder->get_widget("ChannelFormat", widget);
        widget->set_sensitive(true);
        builder->get_widget("AllowVoiceTargets", checkbutton);
        checkbutton->set_sensitive(true);
        builder->get_widget("AllowCommandTargets", checkbutton);
        checkbutton->set_sensitive(true);
    } else {
        copyToCombobox("ChannelCodec" , 4);
        builder->get_widget("ChannelCodecLabel", widget);
        widget->set_sensitive(false);
        builder->get_widget("ChannelCodec", widget);
        widget->set_sensitive(false);
        builder->get_widget("ChannelFormatLabel", widget);
        widget->set_sensitive(false);
        builder->get_widget("ChannelFormat", widget);
        widget->set_sensitive(false);
        builder->get_widget("AllowVoiceTargets", checkbutton);
        checkbutton->set_sensitive(false);
        builder->get_widget("AllowCommandTargets", checkbutton);
        checkbutton->set_sensitive(false);
    }
    builder->get_widget("ChannelEditorLabel", label);
    Glib::ustring labelText = "Editing: ";
    labelText.append(c_to_ustring(channel->name));
    label->set_text(labelText);
    //LoadCodecFormats();
}/*}}}*/
void
ManglerAdmin::AddChannel_clicked_cb(void) {/*{{{*/
    Glib::RefPtr<Gtk::TreeSelection> chanSelection = ChannelEditorTree->get_selection();
    chanSelection->unselect_all();
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
    builder->get_widget("ChannelCodecLabel", widget);
    widget->set_sensitive(isLicensed);
    builder->get_widget("ChannelCodec", widget);
    widget->set_sensitive(isLicensed);
    builder->get_widget("ChannelFormatLabel", widget);
    widget->set_sensitive(isLicensed);
    builder->get_widget("ChannelFormat", widget);
    widget->set_sensitive(isLicensed);
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
    v3_channel *channel = v3_get_channel(currentChannelID);
    if (!channel) return;
    Gtk::MessageDialog confirmDlg( Glib::ustring::compose("Are you sure you want to remove channel %1 (%2)?", Glib::ustring::format(channel->id), c_to_ustring(channel->name)), false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true );
    v3_free_channel(channel);
    if (confirmDlg.run() == Gtk::RESPONSE_YES) {
        v3_channel_remove(currentChannelID);
    }
}/*}}}*/
void
ManglerAdmin::ChannelUpdate_clicked_cb(void) {/*{{{*/
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

    channel.name = ::strdup(ustring_to_c(getFromEntry("ChannelName")).c_str());
    channel.phonetic = ::strdup(ustring_to_c(getFromEntry("ChannelPhonetic")).c_str());
    channel.comment = ::strdup(ustring_to_c(getFromEntry("ChannelComment")).c_str());
    password = getFromEntry("ChannelPassword");
    trimString(password);
    if (password.length()) channel.password_protected = 1;
    channel.protect_mode = getFromCombobox("ChannelProtMode", 0);
    channel.voice_mode = getFromCombobox("ChannelVoiceMode", 0);
    channel.allow_recording = getFromCheckbutton("AllowRecording") ? 1 : 0;
    channel.allow_cross_channel_transmit = getFromCheckbutton("AllowCCxmit") ? 1 : 0;
    channel.allow_paging = getFromCheckbutton("AllowPaging") ? 1 : 0;
    channel.allow_wave_file_binds = getFromCheckbutton("AllowWaveBinds") ? 1 : 0;
    channel.allow_tts_binds = getFromCheckbutton("AllowTTSBinds") ? 1 : 0;
    channel.allow_u2u_transmit = getFromCheckbutton("AllowU2Uxmit") ? 1 : 0;
    channel.allow_phantoms = getFromCheckbutton("AllowPhantoms") ? 1 : 0;
    channel.allow_guests = getFromCheckbutton("AllowGuests") ? 1 : 0;
    channel.allow_voice_target = getFromCheckbutton("AllowVoiceTargets") ? 1 : 0;
    channel.allow_command_target = getFromCheckbutton("AllowCommandTargets") ? 1 : 0;
    channel.inactive_exempt = getFromCheckbutton("TimerExempt") ? 1 : 0;
    channel.disable_guest_transmit = getFromCheckbutton("MuteGuests") ? 1 : 0;
    channel.disable_sound_events = getFromCheckbutton("DisableSoundEvents") ? 1 : 0;
    if (v3_is_licensed()) {
        channel.channel_codec = getFromCombobox("ChannelCodec", 4);
        if (channel.channel_codec < 4) {
            channel.channel_format = getFromCombobox("ChannelFormat", 0);
        } else {
            channel.channel_codec = 0xffff;
            channel.channel_format = 0xffff;
        }
    } else {
        channel.channel_codec = 0xffff;
        channel.channel_format = 0xffff;
    }
    //fprintf(stderr, "Updating:\nname: %s\nphonetic: %s\ncomment: %s\npassword: %s\n",
    //    channel.name, channel.phonetic, channel.comment, password.c_str());
    v3_channel_update(&channel, password.c_str());
    ::free(channel.name);
    ::free(channel.phonetic);
    ::free(channel.comment);
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
    SrvInactChannelModel->clear();
    lobby = *(SrvInactChannelModel->append());
    lobby[adminRecord.id] = 0;
    lobby[adminRecord.name] = "(Lobby)";
    UserDefaultChannelModel->clear();
    lobby = *(UserDefaultChannelModel->append());
    lobby[adminRecord.id] = 0;
    lobby[adminRecord.name] = "(Lobby)";
    UserChanAdminModel->clear();
    UserChanAuthModel->clear();
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
    while ((codec = v3_get_codec(c, f)) && codec->codec != (uint8_t)-1) { // TODO: fix v3_get_codec*() !!!
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

/* ----------  User Editor Related Methods  ---------- */
Gtk::TreeModel::Row
ManglerAdmin::getAccount(uint32_t id, Gtk::TreeModel::Children children) {/*{{{*/
    Gtk::TreeModel::Children::iterator iter = children.begin();
    while (iter != children.end()) {
        if ((*iter)[adminRecord.id] == id) break;
        iter++;
    }
    return *iter;
}/*}}}*/
void
ManglerAdmin::accountUpdated(v3_account *account) {/*{{{*/
    /* main user list */
    Gtk::TreeModel::Row acct;
    acct = getAccount(account->perms.account_id, UserEditorTreeModel->children());
    if (! acct) return;
    acct[adminRecord.name] = c_to_ustring(account->username);
    if (account->perms.account_id == currentUserID) populateUserEditor(account);
    /* User Owner combo box */
    acct = getAccount(account->perms.account_id, UserOwnerModel->children());
    if (acct) {
        if (account->perms.srv_admin) {
            /* update name in owner list */
            acct[adminRecord.name] = c_to_ustring(account->username);
        } else {
            /* needs to be removed, no longer an admin */
            UserOwnerModel->erase(acct);
        }
    } else if (account->perms.srv_admin) {
        /* needs to be added to owner list */
        acct = *(UserOwnerModel->append());
        acct[adminRecord.id] = account->perms.account_id;
        acct[adminRecord.name] = c_to_ustring(account->username);
    }
    /* update status bar */
    statusbarPush(Glib::ustring::compose("User %1 (%2) Updated.", Glib::ustring::format(account->perms.account_id), c_to_ustring(account->username)));
}/*}}}*/
void
ManglerAdmin::accountAdded(v3_account *account) {/*{{{*/
    /* main user list */
    Gtk::TreeModel::iterator iter;
    Gtk::TreeModel::Row acct;
    iter = UserEditorTreeModel->append();
    acct = *iter;
    acct[adminRecord.id] = account->perms.account_id;
    acct[adminRecord.name] = c_to_ustring(account->username);
    /* User Owner combo box */
    if (account->perms.srv_admin) {
        /* needs to be added to owner list */
        acct = *(UserOwnerModel->append());
        acct[adminRecord.id] = account->perms.account_id;
        acct[adminRecord.name] = c_to_ustring(account->username);
    }
    /* update status bar */
    statusbarPush(Glib::ustring::compose("User %1 (%2) Added.", Glib::ustring::format(account->perms.account_id), c_to_ustring(account->username)));
}/*}}}*/
void
ManglerAdmin::accountRemoved(uint32_t acctid) {/*{{{*/
    /* main user list */
    Gtk::TreeModel::Row acct = getAccount(acctid, UserEditorTreeModel->children());
    if (! acct) return;
    UserEditorTreeModel->erase(acct);
    /* User Owner combo box */
    acct = getAccount(acctid, UserOwnerModel->children());
    if (acct) UserOwnerModel->erase(acct);
    /* update status bar */
    statusbarPush(Glib::ustring::compose("User %1 Removed.", Glib::ustring::format(acctid)));
}/*}}}*/
void
ManglerAdmin::accountRemoved(v3_account *account) {/*{{{*/
    if (account) accountRemoved(account->perms.account_id);
}/*}}}*/
void
ManglerAdmin::UserTree_cursor_changed_cb(void) {/*{{{*/
    Gtk::TreeModel::Path path;
    Gtk::TreeViewColumn *column;
    UserEditorTree->get_cursor(path, column);
    Gtk::TreeModel::iterator iter = UserEditorTreeModel->get_iter(path);
    Gtk::TreeModel::Row row = *iter;
    currentUserID = row[adminRecord.id];
    
    if (currentUserID) {
        // load user data into editor
        v3_account *account = v3_get_account(currentUserID);
        if (! account) {
            fprintf(stderr, "failed to retrieve user information for account id %d\n", currentUserID);
            currentUserID = 0xffff;
            return;
        }
        populateUserEditor(account);
        v3_free_account(account);
    }
    // get user permissions
    const v3_permissions *perms = v3_get_permissions();
    // enable or disable editor and necessary buttons
    bool editAccess( currentUserID && perms->srv_admin );
    UserEditor->set_sensitive( editAccess );
    UserRemove->set_sensitive( editAccess && perms->del_user );
    UserAdd->set_sensitive( perms->srv_admin && perms->add_user );
}/*}}}*/
void
ManglerAdmin::populateUserEditor(v3_account *account) {/*{{{*/
    bool isLicensed( v3_is_licensed() );
    copyToEntry("UserLogin", c_to_ustring(account->username));
    copyToEntry("UserPassword", "");
    copyToCombobox("UserRank", account->perms.rank_id, 0);
    copyToCombobox("UserOwner", account->perms.replace_owner_id, 0);
    Gtk::TextView *textview;
    builder->get_widget("UserNotes", textview);
    if (account->notes) textview->get_buffer()->set_text(c_to_ustring(account->notes));
    else textview->get_buffer()->set_text("");
    copyToCheckbutton("UserLocked", account->perms.lock_acct);
    copyToEntry("UserLockedReason", c_to_ustring(account->lock_reason));
    copyToCheckbutton("UserInReservedList", account->perms.in_reserve_list);
    copyToCheckbutton("UserReceiveBroadcasts", account->perms.recv_bcast);
    copyToCheckbutton("UserAddPhantoms", account->perms.add_phantom);
    copyToCheckbutton("UserAllowRecord", account->perms.record);
    copyToCheckbutton("UserIgnoreTimers", account->perms.inactive_exempt);
    copyToCheckbutton("UserSendComplaints", account->perms.send_complaint);
    copyToCheckbutton("UserReceiveComplaints", account->perms.recv_complaint);
    copyToCombobox("UserDuplicateIPs", account->perms.dupe_ip, 0);
    copyToCheckbutton("UserSwitchChannels", account->perms.switch_chan);
    copyToCombobox("UserDefaultChannel", account->perms.dfl_chan);
    copyToCheckbutton("UserBroadcast", account->perms.bcast);
    copyToCheckbutton("UserBroadcastLobby", account->perms.bcast_lobby);
    copyToCheckbutton("UserBroadcastU2U", account->perms.bcast_user);
    copyToCheckbutton("UserBroadcastxChan", account->perms.bcast_x_chan);
    copyToCheckbutton("UserSendTTSBinds", account->perms.send_tts_bind);
    copyToCheckbutton("UserSendWaveBinds", account->perms.send_wav_bind);
    copyToCheckbutton("UserSendPages", account->perms.send_page);
    copyToCheckbutton("UserSetPhonetic", account->perms.set_phon_name);
    copyToCheckbutton("UserSendComment", account->perms.send_comment);
    copyToCheckbutton("UserGenCommentSounds", account->perms.gen_comment_snds);
    copyToCheckbutton("UserEventSounds", account->perms.event_snds);
    copyToCheckbutton("UserMuteGlobally", account->perms.mute_glbl);
    copyToCheckbutton("UserMuteOthers", account->perms.mute_other);
    copyToCheckbutton("UserGlobalChat", account->perms.glbl_chat);
    copyToCheckbutton("UserPrivateChat", account->perms.start_priv_chat);
    setWidgetSensitive("UserPrivateChat", isLicensed);
    copyToCheckbutton("UserEqOut", account->perms.eq_out);
    copyToCheckbutton("UserSeeGuests", account->perms.see_guest);
    copyToCheckbutton("UserSeeNonGuests", account->perms.see_nonguest);
    copyToCheckbutton("UserSeeMOTD", account->perms.see_motd);
    copyToCheckbutton("UserSeeServerComment", account->perms.see_srv_comment);
    copyToCheckbutton("UserSeeChannelList", account->perms.see_chan_list);
    copyToCheckbutton("UserSeeChannelComments", account->perms.see_chan_comment);
    copyToCheckbutton("UserSeeUserComments", account->perms.see_user_comment);
    copyToCheckbutton("UserServerAdmin", account->perms.srv_admin);
    copyToCheckbutton("UserRemoveUsers", account->perms.del_user);
    copyToCheckbutton("UserAddUsers", account->perms.add_user);
    copyToCheckbutton("UserBanUsers", account->perms.ban_user);
    copyToCheckbutton("UserKickUsers", account->perms.kick_user);
    copyToCheckbutton("UserMoveUsers", account->perms.move_user);
    copyToCheckbutton("UserAssignChanAdmin", account->perms.assign_chan_admin);
    copyToCheckbutton("UserAssignRank", account->perms.assign_rank);
    copyToCheckbutton("UserEditRanks", account->perms.edit_rank);
    copyToCheckbutton("UserEditMOTD", account->perms.edit_motd);
    copyToCheckbutton("UserEditGuestMOTD", account->perms.edit_guest_motd);
    copyToCheckbutton("UserIssueRcon", account->perms.issue_rcon_cmd);
    copyToCheckbutton("UserEditVoiceTargets", account->perms.edit_voice_target);
    setWidgetSensitive("UserEditVoiceTargets", isLicensed);
    copyToCheckbutton("UserEditCommandTargets", account->perms.edit_command_target);
    setWidgetSensitive("UserEditCommandTargets", isLicensed);
    copyToCheckbutton("UserAssignReserved", account->perms.assign_reserved);
    setAdminCheckTree(UserChanAdminModel->children(), account->chan_admin, account->chan_admin_count);
    setAdminCheckTree(UserChanAuthModel->children(), account->chan_auth, account->chan_auth_count);
    builder->get_widget("UserEditorLabel", label);
    Glib::ustring labelText = "Editing: ";
    if (currentUserID == 0xffff) {
        labelText.append("NEW USER");
        setWidgetSensitive("UserLogin", true);
    } else {
        labelText.append(c_to_ustring(account->username));
        setWidgetSensitive("UserLogin", false);
    }
    label->set_text(labelText);
    setWidgetSensitive("UserPassword", account->perms.account_id != 1);
}/*}}}*/
void
ManglerAdmin::setAdminCheckTree(Gtk::TreeModel::Children children, uint16_t *chanids, int chan_count) {/*{{{*/
    Gtk::TreeModel::Children::iterator iter = children.begin();
    Gtk::TreeModel::Row row;
    uint32_t rowId;
    int i;
    bool found;
    while (iter != children.end()) {
        row = *iter;
        rowId = row[adminCheckRecord.id];
        found = false;
        for (i = 0; i < chan_count; ++i) {
            if (chanids[i] == rowId) {
                found = true;
                break;
            }
        }
        row[adminCheckRecord.on] = found;
        if (row.children().size()) setAdminCheckTree(row->children(), chanids, chan_count);
        iter++;
    }
}/*}}}*/
void
ManglerAdmin::getAdminCheckTree(Gtk::TreeModel::Children children, std::vector<uint16_t> &chanids) {/*{{{*/
    Gtk::TreeModel::Children::iterator iter = children.begin();
    Gtk::TreeModel::Row row;
    uint32_t rowId;
    bool rowOn;
    while (iter != children.end()) {
        row = *iter;
        rowId = row[adminCheckRecord.id];
        rowOn = row[adminCheckRecord.on];
        if (rowOn) chanids.push_back(rowId);
        if (row.children().size()) getAdminCheckTree(row->children(), chanids);
        iter++;
    }
}/*}}}*/
void
ManglerAdmin::getAdminCheckTree(Gtk::TreeModel::Children children, uint16_t *&chanids, int &chan_count) {/*{{{*/
    std::vector<uint16_t> chanvec;
    getAdminCheckTree(children, chanvec);
    chan_count = chanvec.size();
    if (chan_count) {
        chanids = (uint16_t*)::malloc(sizeof(uint16_t) * chanvec.size());
        if (chanids) {
            for (int i = 0; i < chan_count; ++i) chanids[i] = chanvec[i];
        } else chan_count = 0; // failed malloc
    } else chanids = NULL;
}/*}}}*/
void
ManglerAdmin::UserAdd_clicked_cb(void) {/*{{{*/
    Glib::RefPtr<Gtk::TreeSelection> userSelection = UserEditorTree->get_selection();
    userSelection->unselect_all();
    v3_account blankAcct;
    ::memset(&blankAcct, 0, sizeof(v3_account));
    label->set_text("Editing: NEW USER");
    currentUserID = 0xffff;
    populateUserEditor(&blankAcct);
    // enable or disable editor and necessary buttons
    UserEditor->set_sensitive( true );
    UserRemove->set_sensitive( false );
    UserAdd->set_sensitive( false );
}/*}}}*/
void
ManglerAdmin::UserRemove_clicked_cb(void) {/*{{{*/
    v3_account *account = v3_get_account(currentUserID);
    if (!account) return;
    Gtk::MessageDialog confirmDlg( Glib::ustring::compose("Are you sure you want to remove user %1 (%2)?", Glib::ustring::format(account->perms.account_id), c_to_ustring(account->username)), false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true );
    v3_free_account(account);
    if (confirmDlg.run() == Gtk::RESPONSE_YES) {
        v3_userlist_remove(currentUserID);
    }
}/*}}}*/
void
ManglerAdmin::UserUpdate_clicked_cb(void) {/*{{{*/
    v3_account account;
    ::memset(&account, 0, sizeof(v3_account));
    account.perms.account_id = currentUserID == 0xffff ? 0 : currentUserID;
    account.username = ::strdup(ustring_to_c(getFromEntry("UserLogin")).c_str());
    Glib::ustring password = getFromEntry("UserPassword");
    trimString(password);
    if (password.length()) {
        _v3_hash_password((uint8_t *)(ustring_to_c(password).c_str()), (uint8_t *)account.perms.hash_password);
    }
    account.perms.rank_id = getFromCombobox("UserRank", 0);
    uint16_t ownerID = getFromCombobox("UserOwner", 0);
    account.perms.replace_owner_id = ownerID;
    if (ownerID) {
        Gtk::TreeModel::Row ownerRow = getAccount(ownerID, UserEditorTreeModel->children());
        Glib::ustring ownerName = ownerRow[adminRecord.name];
        account.owner = ::strdup(ownerName.c_str());
    } else {
        account.owner = ::strdup("");
    }
    Gtk::TextView *textview;
    builder->get_widget("UserNotes", textview);
    account.notes = ::strdup(ustring_to_c(textview->get_buffer()->get_text()).c_str());
    account.perms.lock_acct = getFromCheckbutton("UserLocked");
    account.lock_reason = ::strdup(ustring_to_c(getFromEntry("UserLockedReason")).c_str());
    account.perms.in_reserve_list = getFromCheckbutton("UserInReservedList");
    account.perms.recv_bcast = getFromCheckbutton("UserReceiveBroadcasts");
    account.perms.add_phantom = getFromCheckbutton("UserAddPhantoms");
    account.perms.record = getFromCheckbutton("UserAllowRecord");
    account.perms.inactive_exempt = getFromCheckbutton("UserIgnoreTimers");
    account.perms.send_complaint = getFromCheckbutton("UserSendComplaints");
    account.perms.recv_complaint = getFromCheckbutton("UserReceiveComplaints");
    account.perms.dupe_ip = getFromCombobox("UserDuplicateIPs", 0);
    account.perms.switch_chan = getFromCheckbutton("UserSwitchChannels");
    account.perms.dfl_chan = getFromCombobox("UserDefaultChannel");
    account.perms.bcast = getFromCheckbutton("UserBroadcast");
    account.perms.bcast_lobby = getFromCheckbutton("UserBroadcastLobby");
    account.perms.bcast_user = getFromCheckbutton("UserBroadcastU2U");
    account.perms.bcast_x_chan = getFromCheckbutton("UserBroadcastxChan");
    account.perms.send_tts_bind = getFromCheckbutton("UserSendTTSBinds");
    account.perms.send_wav_bind = getFromCheckbutton("UserSendWaveBinds");
    account.perms.send_page = getFromCheckbutton("UserSendPages");
    account.perms.set_phon_name = getFromCheckbutton("UserSetPhonetic");
    account.perms.send_comment = getFromCheckbutton("UserSendComment");
    account.perms.gen_comment_snds = getFromCheckbutton("UserGenCommentSounds");
    account.perms.event_snds = getFromCheckbutton("UserEventSounds");
    account.perms.mute_glbl = getFromCheckbutton("UserMuteGlobally");
    account.perms.mute_other = getFromCheckbutton("UserMuteOthers");
    account.perms.glbl_chat = getFromCheckbutton("UserGlobalChat");
    account.perms.start_priv_chat = getFromCheckbutton("UserPrivateChat");
    account.perms.eq_out = getFromCheckbutton("UserEqOut");
    account.perms.see_guest = getFromCheckbutton("UserSeeGuests");
    account.perms.see_nonguest = getFromCheckbutton("UserSeeNonGuests");
    account.perms.see_motd = getFromCheckbutton("UserSeeMOTD");
    account.perms.see_srv_comment = getFromCheckbutton("UserSeeServerComment");
    account.perms.see_chan_list = getFromCheckbutton("UserSeeChannelList");
    account.perms.see_chan_comment = getFromCheckbutton("UserSeeChannelComments");
    account.perms.see_user_comment = getFromCheckbutton("UserSeeUserComments");
    account.perms.srv_admin = getFromCheckbutton("UserServerAdmin");
    account.perms.del_user = getFromCheckbutton("UserRemoveUsers");
    account.perms.add_user = getFromCheckbutton("UserAddUsers");
    account.perms.ban_user = getFromCheckbutton("UserBanUsers");
    account.perms.kick_user = getFromCheckbutton("UserKickUsers");
    account.perms.move_user = getFromCheckbutton("UserMoveUsers");
    account.perms.assign_chan_admin = getFromCheckbutton("UserAssignChanAdmin");
    account.perms.assign_rank = getFromCheckbutton("UserAssignRank");
    account.perms.edit_rank = getFromCheckbutton("UserEditRanks");
    account.perms.edit_motd = getFromCheckbutton("UserEditMOTD");
    account.perms.edit_guest_motd = getFromCheckbutton("UserEditGuestMOTD");
    account.perms.issue_rcon_cmd = getFromCheckbutton("UserIssueRcon");
    account.perms.edit_voice_target = getFromCheckbutton("UserEditVoiceTargets");
    account.perms.edit_command_target = getFromCheckbutton("UserEditCommandTargets");
    account.perms.assign_reserved = getFromCheckbutton("UserAssignReserved");
    getAdminCheckTree(UserChanAdminModel->children(), account.chan_admin, account.chan_admin_count);
    v3_userlist_update(&account);
    ::free(account.username);
    ::free(account.owner);
    ::free(account.notes);
    ::free(account.lock_reason);
    if (account.chan_admin_count) ::free(account.chan_admin);
    if (account.chan_auth_count) ::free(account.chan_auth);
}/*}}}*/
void
ManglerAdmin::UserInfoButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("UserInfoButton", togglebutton);
    builder->get_widget("UserInfoArrow", arrow);
    if (togglebutton->get_active()) {
        UserInfoSection->show();
        arrow->set(Gtk::ARROW_DOWN, Gtk::SHADOW_NONE);
    } else {
        UserInfoSection->hide();
        arrow->set(Gtk::ARROW_RIGHT, Gtk::SHADOW_NONE);
    }
}/*}}}*/
void
ManglerAdmin::UserNetworkButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("UserNetworkButton", togglebutton);
    builder->get_widget("UserNetworkArrow", arrow);
    if (togglebutton->get_active()) {
        UserNetworkSection->show();
        arrow->set(Gtk::ARROW_DOWN, Gtk::SHADOW_NONE);
    } else {
        UserNetworkSection->hide();
        arrow->set(Gtk::ARROW_RIGHT, Gtk::SHADOW_NONE);
    }
}/*}}}*/
void
ManglerAdmin::UserTransmitButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("UserTransmitButton", togglebutton);
    builder->get_widget("UserTransmitArrow", arrow);
    if (togglebutton->get_active()) {
        UserTransmitSection->show();
        arrow->set(Gtk::ARROW_DOWN, Gtk::SHADOW_NONE);
    } else {
        UserTransmitSection->hide();
        arrow->set(Gtk::ARROW_RIGHT, Gtk::SHADOW_NONE);
    }
}/*}}}*/
void
ManglerAdmin::UserDisplayButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("UserDisplayButton", togglebutton);
    builder->get_widget("UserDisplayArrow", arrow);
    if (togglebutton->get_active()) {
        UserDisplaySection->show();
        arrow->set(Gtk::ARROW_DOWN, Gtk::SHADOW_NONE);
    } else {
        UserDisplaySection->hide();
        arrow->set(Gtk::ARROW_RIGHT, Gtk::SHADOW_NONE);
    }
}/*}}}*/
void
ManglerAdmin::UserAdminButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("UserAdminButton", togglebutton);
    builder->get_widget("UserAdminArrow", arrow);
    if (togglebutton->get_active()) {
        UserAdminSection->show();
        arrow->set(Gtk::ARROW_DOWN, Gtk::SHADOW_NONE);
    } else {
        UserAdminSection->hide();
        arrow->set(Gtk::ARROW_RIGHT, Gtk::SHADOW_NONE);
    }
}/*}}}*/
void
ManglerAdmin::UserChanAdminButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("UserChanAdminButton", togglebutton);
    builder->get_widget("UserChanAdminArrow", arrow);
    if (togglebutton->get_active()) {
        UserChanAdminTree->show();
        arrow->set(Gtk::ARROW_DOWN, Gtk::SHADOW_NONE);
    } else {
        UserChanAdminTree->hide();
        arrow->set(Gtk::ARROW_RIGHT, Gtk::SHADOW_NONE);
    }
}/*}}}*/
void
ManglerAdmin::UserChanAuthButton_toggled_cb(void) {/*{{{*/
    builder->get_widget("UserChanAuthButton", togglebutton);
    builder->get_widget("UserChanAuthArrow", arrow);
    if (togglebutton->get_active()) {
        UserChanAuthTree->show();
        arrow->set(Gtk::ARROW_DOWN, Gtk::SHADOW_NONE);
    } else {
        UserChanAuthTree->hide();
        arrow->set(Gtk::ARROW_RIGHT, Gtk::SHADOW_NONE);
    }
}/*}}}*/
/* user editor 'profile' methods */
void
ManglerAdmin::readUserTemplates(void) {/*{{{*/
    std::string utfilename = getenv("HOME");
    int utflen = utfilename.length();
    if (utflen == 0) utfilename = "/etc/"; // should never happen
    if (utfilename[utflen-1] != '/') utfilename.append("/");
    utfilename.append(".mangler/usertemplates.ini");
    usertemplates = new iniFile( utfilename );
    iniFile::iterator iter = usertemplates->begin();
    while (iter != usertemplates->end()) {
        Gtk::TreeStore::Row row = *UserTemplateModel->append();
        row[adminRecord.name] = iter->first;
        iter++;
    }
    builder->get_widget("UserTemplateLoad", button);
    button->set_sensitive(false);
    builder->get_widget("UserTemplateSave", button);
    button->set_sensitive(false);
}/*}}}*/
void
ManglerAdmin::UserTemplate_changed_cb(void) {/*{{{*/
    string tmplname = UserTemplate->get_active_text();
    builder->get_widget("UserTemplateLoad", button);
    button->set_sensitive(usertemplates->contains(tmplname));
    builder->get_widget("UserTemplateSave", button);
    button->set_sensitive(tmplname.length());
}/*}}}*/
void
ManglerAdmin::UserTemplateLoad_clicked_cb(void) {/*{{{*/
    string tmplname = UserTemplate->get_active_text();
    iniSection &tmpl = usertemplates->at(tmplname);
    //copyToCombobox("UserRank", account->perms.rank_id, 0);
    //copyToCombobox("UserOwner", account->perms.replace_owner_id, 0);
    //Gtk::TextView *textview;
    //builder->get_widget("UserNotes", textview);
    //if (account->notes) textview->get_buffer()->set_text(c_to_ustring(tmpl["notes"]));
    //else textview->get_buffer()->set_text("");
    copyToCheckbutton("UserLocked", tmpl["Locked"].toBool());
    copyToEntry("UserLockedReason", c_to_ustring(tmpl["LockedReason"].toCString()));
    copyToCheckbutton("UserInReservedList", tmpl["InReservedList"].toBool());
    copyToCheckbutton("UserReceiveBroadcasts", tmpl["ReceiveBroadcasts"].toBool());
    copyToCheckbutton("UserAddPhantoms", tmpl["AddPhantoms"].toBool());
    copyToCheckbutton("UserAllowRecord", tmpl["AllowRecord"].toBool());
    copyToCheckbutton("UserIgnoreTimers", tmpl["IgnoreTimers"].toBool());
    copyToCheckbutton("UserSendComplaints", tmpl["SendComplaints"].toBool());
    copyToCheckbutton("UserReceiveComplaints", tmpl["ReceiveComplaints"].toBool());
    copyToCombobox("UserDuplicateIPs", tmpl["DuplicateIPs"].toInt(), 0);
    copyToCheckbutton("UserSwitchChannels", tmpl["SwitchChannels"].toBool());
    copyToCombobox("UserDefaultChannel", tmpl["DefaultChannel"].toInt());
    copyToCheckbutton("UserBroadcast", tmpl["Broadcast"].toBool());
    copyToCheckbutton("UserBroadcastLobby", tmpl["BroadcastLobby"].toBool());
    copyToCheckbutton("UserBroadcastU2U", tmpl["BroadcastU2U"].toBool());
    copyToCheckbutton("UserBroadcastxChan", tmpl["BroadcastxChan"].toBool());
    copyToCheckbutton("UserSendTTSBinds", tmpl["SendTTSBinds"].toBool());
    copyToCheckbutton("UserSendWaveBinds", tmpl["SendWaveBinds"].toBool());
    copyToCheckbutton("UserSendPages", tmpl["SendPages"].toBool());
    copyToCheckbutton("UserSetPhonetic", tmpl["SetPhonetic"].toBool());
    copyToCheckbutton("UserSendComment", tmpl["SendComment"].toBool());
    copyToCheckbutton("UserGenCommentSounds", tmpl["GenCommentSounds"].toBool());
    copyToCheckbutton("UserEventSounds", tmpl["EventSounds"].toBool());
    copyToCheckbutton("UserMuteGlobally", tmpl["MuteGlobally"].toBool());
    copyToCheckbutton("UserMuteOthers", tmpl["MuteOthers"].toBool());
    copyToCheckbutton("UserGlobalChat", tmpl["GlobalChat"].toBool());
    copyToCheckbutton("UserPrivateChat", tmpl["PrivateChat"].toBool());
    copyToCheckbutton("UserEqOut", tmpl["EqOut"].toBool());
    copyToCheckbutton("UserSeeGuests", tmpl["SeeGuests"].toBool());
    copyToCheckbutton("UserSeeNonGuests", tmpl["SeeNonGuests"].toBool());
    copyToCheckbutton("UserSeeMOTD", tmpl["SeeMOTD"].toBool());
    copyToCheckbutton("UserSeeServerComment", tmpl["SeeServerComment"].toBool());
    copyToCheckbutton("UserSeeChannelList", tmpl["SeeChannelList"].toBool());
    copyToCheckbutton("UserSeeChannelComments", tmpl["SeeChannelComments"].toBool());
    copyToCheckbutton("UserSeeUserComments", tmpl["SeeUserComments"].toBool());
    copyToCheckbutton("UserServerAdmin", tmpl["ServerAdmin"].toBool());
    copyToCheckbutton("UserRemoveUsers", tmpl["RemoveUsers"].toBool());
    copyToCheckbutton("UserAddUsers", tmpl["AddUsers"].toBool());
    copyToCheckbutton("UserBanUsers", tmpl["BanUsers"].toBool());
    copyToCheckbutton("UserKickUsers", tmpl["KickUsers"].toBool());
    copyToCheckbutton("UserMoveUsers", tmpl["MoveUsers"].toBool());
    copyToCheckbutton("UserAssignChanAdmin", tmpl["AssignChanAdmin"].toBool());
    copyToCheckbutton("UserAssignRank", tmpl["AssignRank"].toBool());
    copyToCheckbutton("UserEditRanks", tmpl["EditRanks"].toBool());
    copyToCheckbutton("UserEditMOTD", tmpl["EditMOTD"].toBool());
    copyToCheckbutton("UserEditGuestMOTD", tmpl["EditGuestMOTD"].toBool());
    copyToCheckbutton("UserIssueRcon", tmpl["IssueRcon"].toBool());
    copyToCheckbutton("UserEditVoiceTargets", tmpl["EditVoiceTargets"].toBool());
    copyToCheckbutton("UserEditCommandTargets", tmpl["EditCommandTargets"].toBool());
    copyToCheckbutton("UserAssignReserved", tmpl["AssignReserved"].toBool());
    
    if (tmpl.contains("ChannelAdmin")) {
        int chan_admin_count = tmpl["ChannelAdmin"].count();
        uint16_t chan_admin[chan_admin_count];
        for (int n = 0; n < chan_admin_count; ++n) {
            chan_admin[n] = tmpl["ChannelAdmin"][n].toULong();
        }
        setAdminCheckTree(UserChanAdminModel->children(), chan_admin, chan_admin_count);
    } else setAdminCheckTree(UserChanAdminModel->children(), NULL, 0);

    if (tmpl.contains("ChannelAuth")) {
        int chan_auth_count = tmpl["ChannelAuth"].count();
        uint16_t chan_auth[chan_auth_count];
        for (int n = 0; n < chan_auth_count; ++n) {
            chan_auth[n] = tmpl["ChannelAuth"][n].toULong();
        }
        setAdminCheckTree(UserChanAuthModel->children(), chan_auth, chan_auth_count);
    } else setAdminCheckTree(UserChanAuthModel->children(), NULL, 0);

}/*}}}*/
void
ManglerAdmin::UserTemplateSave_clicked_cb(void) {/*{{{*/
    string tmplname = UserTemplate->get_active_text();
    iniSection &tmpl = (*usertemplates)[tmplname];
    //copyToCombobox("UserRank", account->perms.rank_id, 0);
    //copyToCombobox("UserOwner", account->perms.replace_owner_id, 0);
    //Gtk::TextView *textview;
    //builder->get_widget("UserNotes", textview);
    //if (account->notes) textview->get_buffer()->set_text(c_to_ustring(tmpl["notes"]));
    //else textview->get_buffer()->set_text("");
    tmpl["Locked"] = getFromCheckbutton("UserLocked");
    tmpl["LockedReason"] = getFromEntry("UserLockedReason").raw();
    tmpl["InReservedList"] = getFromCheckbutton("UserInReservedList");
    tmpl["ReceiveBroadcasts"] = getFromCheckbutton("UserReceiveBroadcasts");
    tmpl["AddPhantoms"] = getFromCheckbutton("UserAddPhantoms");
    tmpl["AllowRecord"] = getFromCheckbutton("UserAllowRecord");
    tmpl["IgnoreTimers"] = getFromCheckbutton("UserIgnoreTimers");
    tmpl["SendComplaints"] = getFromCheckbutton("UserSendComplaints");
    tmpl["ReceiveComplaints"] = getFromCheckbutton("UserReceiveComplaints");
    tmpl["DuplicateIPs"] = getFromCombobox("UserDuplicateIPs", 0);
    tmpl["SwitchChannels"] = getFromCheckbutton("UserSwitchChannels");
    tmpl["DefaultChannel"] = getFromCombobox("UserDefaultChannel");
    tmpl["Broadcast"] = getFromCheckbutton("UserBroadcast");
    tmpl["BroadcastLobby"] = getFromCheckbutton("UserBroadcastLobby");
    tmpl["BroadcastU2U"] = getFromCheckbutton("UserBroadcastU2U");
    tmpl["BroadcastxChan"] = getFromCheckbutton("UserBroadcastxChan");
    tmpl["SendTTSBinds"] = getFromCheckbutton("UserSendTTSBinds");
    tmpl["SendWaveBinds"] = getFromCheckbutton("UserSendWaveBinds");
    tmpl["SendPages"] = getFromCheckbutton("UserSendPages");
    tmpl["SetPhonetic"] = getFromCheckbutton("UserSetPhonetic");
    tmpl["SendComment"] = getFromCheckbutton("UserSendComment");
    tmpl["GenCommentSounds"] = getFromCheckbutton("UserGenCommentSounds");
    tmpl["EventSounds"] = getFromCheckbutton("UserEventSounds");
    tmpl["MuteGlobally"] = getFromCheckbutton("UserMuteGlobally");
    tmpl["MuteOthers"] = getFromCheckbutton("UserMuteOthers");
    tmpl["GlobalChat"] = getFromCheckbutton("UserGlobalChat");
    tmpl["PrivateChat"] = getFromCheckbutton("UserPrivateChat");
    tmpl["EqOut"] = getFromCheckbutton("UserEqOut");
    tmpl["SeeGuests"] = getFromCheckbutton("UserSeeGuests");
    tmpl["SeeNonGuests"] = getFromCheckbutton("UserSeeNonGuests");
    tmpl["SeeMOTD"] = getFromCheckbutton("UserSeeMOTD");
    tmpl["SeeServerComment"] = getFromCheckbutton("UserSeeServerComment");
    tmpl["SeeChannelList"] = getFromCheckbutton("UserSeeChannelList");
    tmpl["SeeChannelComments"] = getFromCheckbutton("UserSeeChannelComments");
    tmpl["SeeUserComments"] = getFromCheckbutton("UserSeeUserComments");
    tmpl["ServerAdmin"] = getFromCheckbutton("UserServerAdmin");
    tmpl["RemoveUsers"] = getFromCheckbutton("UserRemoveUsers");
    tmpl["AddUsers"] = getFromCheckbutton("UserAddUsers");
    tmpl["BanUsers"] = getFromCheckbutton("UserBanUsers");
    tmpl["KickUsers"] = getFromCheckbutton("UserKickUsers");
    tmpl["MoveUsers"] = getFromCheckbutton("UserMoveUsers");
    tmpl["AssignChanAdmin"] = getFromCheckbutton("UserAssignChanAdmin");
    tmpl["AssignRank"] = getFromCheckbutton("UserAssignRank");
    tmpl["EditRanks"] = getFromCheckbutton("UserEditRanks");
    tmpl["EditMOTD"] = getFromCheckbutton("UserEditMOTD");
    tmpl["EditGuestMOTD"] = getFromCheckbutton("UserEditGuestMOTD");
    tmpl["IssueRcon"] = getFromCheckbutton("UserIssueRcon");
    tmpl["EditVoiceTargets"] = getFromCheckbutton("UserEditVoiceTargets");
    tmpl["EditCommandTargets"] = getFromCheckbutton("UserEditCommandTargets");
    tmpl["AssignReserved"] = getFromCheckbutton("UserAssignReserved");
    
    std::vector<uint16_t> chan_admin, chan_auth;
    getAdminCheckTree(UserChanAdminModel->children(), chan_admin);
    if (chan_admin.size()) {
        tmpl["ChannelAdmin"] = chan_admin[0];
        for (unsigned n = 1; n < chan_admin.size(); ++n) {
            tmpl["ChannelAdmin"] += chan_admin[n];
        }
    } else tmpl.erase("ChannelAdmin");

    getAdminCheckTree(UserChanAuthModel->children(), chan_auth);
    if (chan_auth.size()) {
        tmpl["ChannelAuth"] = chan_auth[0];
        for (unsigned n = 1; n < chan_auth.size(); ++n) {
            tmpl["ChannelAuth"] += chan_auth[n];
        }
    } else tmpl.erase("ChannelAuth");

    usertemplates->save();
    Gtk::TreeStore::Row row = *UserTemplateModel->append();
    row[adminRecord.name] = tmplname;
}/*}}}*/
/* ----------  Rank Editor Related Methods  ---------- */
Gtk::TreeModel::iterator
ManglerAdmin::getRank(uint16_t id, Gtk::TreeModel::Children children) {/*{{{*/
    Gtk::TreeModel::Children::iterator iter = children.begin();
    while (iter != children.end()) {
        if ((*iter)[rankRecord.id] == id) break;
        iter++;
    }
    return iter;
}/*}}}*/
void
ManglerAdmin::rankUpdated(v3_rank *rank) {/*{{{*/
    Gtk::TreeModel::iterator iter = getRank(rank->id, RankEditorModel->children());
    if (! iter) iter = RankEditorModel->append();
    (*iter)[rankRecord.id] = rank->id;
    (*iter)[rankRecord.level] = rank->level;
    (*iter)[rankRecord.name] = c_to_ustring(rank->name);
    (*iter)[rankRecord.description] = c_to_ustring(rank->description);
    /* now handle the User Rank combo box */
    /* the poorly named getAccount() will work fine for this */
    Gtk::TreeModel::Row row = getAccount(rank->id, UserRankModel->children());
    if (! row) row = *(UserRankModel->append());
    row[adminRecord.id] = rank->id; row[adminRecord.name] = c_to_ustring(rank->name);
    /* update status bar */
    statusbarPush(Glib::ustring::compose("Rank %1 (%2) Updated.", Glib::ustring::format(rank->id), c_to_ustring(rank->name)));
}/*}}}*/
void
ManglerAdmin::rankAdded(v3_rank *rank) {/*{{{*/
    Gtk::TreeModel::iterator iter = RankEditorModel->append();
    (*iter)[rankRecord.id] = rank->id;
    (*iter)[rankRecord.level] = rank->level;
    (*iter)[rankRecord.name] = c_to_ustring(rank->name);
    (*iter)[rankRecord.description] = rank->description;
    /* now handle the User Rank combo box */
    iter = UserRankModel->append();
    (*iter)[adminRecord.id] = rank->id; (*iter)[adminRecord.name] = c_to_ustring(rank->name);
    /* update status bar */
    statusbarPush(Glib::ustring::compose("Rank %1 (%2) Added.", Glib::ustring::format(rank->id), c_to_ustring(rank->name)));
}/*}}}*/
void
ManglerAdmin::rankRemoved(uint16_t rankid) {/*{{{*/
    Gtk::TreeModel::iterator iter = getRank(rankid, RankEditorModel->children());
    if (iter) RankEditorModel->erase(*iter);
    /* now handle the User Rank combo box */
    /* the poorly named getAccount() will work fine for this */
    Gtk::TreeModel::Row row = getAccount(rankid, UserRankModel->children());
    if (row) UserRankModel->erase(row);
    /* update status bar */
    statusbarPush(Glib::ustring::compose("Rank %1 Removed.", Glib::ustring::format(rankid)));
}/*}}}*/
void
ManglerAdmin::rankRemoved(v3_rank *rank) {/*{{{*/
    rankRemoved(rank->id);
}/*}}}*/
void
ManglerAdmin::RankUpdate_clicked_cb(void) {/*{{{*/
    v3_rank rank;
    rank.id = currentRankID;
    rank.name = ::strdup(ustring_to_c(getFromEntry("RankName")).c_str());
    rank.description = ::strdup(ustring_to_c(getFromEntry("RankDescription")).c_str());
    rank.level = uint16_t( getFromSpinbutton("RankLevel") );
    v3_rank_update(&rank);
    ::free(rank.name);
    ::free(rank.description);
}/*}}}*/
void
ManglerAdmin::RankEditorTree_cursor_changed_cb(void) {/*{{{*/
    Gtk::TreeModel::Path path;
    Gtk::TreeViewColumn *column;
    RankEditorTree->get_cursor(path, column);
    Gtk::TreeModel::iterator iter = RankEditorModel->get_iter(path);
    bool isRank( false );
    if (iter) {
        isRank = true;
        currentRankID = (*iter)[rankRecord.id];
        copyToEntry("RankName", (*iter)[rankRecord.name]);
        copyToEntry("RankDescription", (*iter)[rankRecord.description]);
        copyToSpinbutton("RankLevel", (*iter)[rankRecord.level]);
    }
    setWidgetSensitive("RankAdd", true);
    setWidgetSensitive("RankRemove", isRank);
    RankEditor->set_sensitive(isRank);
}/*}}}*/
void
ManglerAdmin::RankAdd_clicked_cb(void) {/*{{{*/
    Glib::RefPtr<Gtk::TreeSelection> rankSelection = RankEditorTree->get_selection();
    rankSelection->unselect_all();
    currentRankID = 0;
    copyToEntry("RankName", "");
    copyToEntry("RankDescription", "");
    copyToSpinbutton("RankLevel", 0);
    setWidgetSensitive("RankRemove", false);
    setWidgetSensitive("RankAdd", false);
    RankEditor->set_sensitive(true);
}/*}}}*/
void
ManglerAdmin::RankRemove_clicked_cb(void) {/*{{{*/
    Gtk::TreeModel::Path path;
    Gtk::TreeViewColumn *column;
    RankEditorTree->get_cursor(path, column);
    Gtk::TreeModel::iterator iter = RankEditorModel->get_iter(path);
    if (! iter) return;
    uint16_t rankid = (*iter)[rankRecord.id];
    Glib::ustring rankname = (*iter)[rankRecord.name];
    Gtk::MessageDialog confirmDlg( Glib::ustring::compose("Are you sure you want to remove rank %1 (%2)?", Glib::ustring::format(rankid), rankname), false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true );
    if (confirmDlg.run() == Gtk::RESPONSE_YES) {
        v3_rank_remove(rankid);
    }
}/*}}}*/
void
ManglerAdmin::clearRanks(void) {/*{{{*/
    RankEditorModel->clear();

    RankEditor->set_sensitive(false);
    setWidgetSensitive("RankAdd", true);
    setWidgetSensitive("RankRemove", false);
    setWidgetSensitive("RankUpdate", true);

    currentRankID = 0xff;
}/*}}}*/
/* ----------  Ban Editor Related Methods  ---------- */
void
ManglerAdmin::banList(uint16_t id, uint16_t count, uint16_t bitmask_id, uint32_t ip_address, char *user, char *by, char *reason) {/*{{{*/
    if (!id) {
        BanEditorModel->clear();
    }
    if (!count) {
        clearBans();
        return;
    }
    Gtk::TreeModel::iterator iter = BanEditorModel->append();
    (*iter)[banRecord.id] = id;
    (*iter)[banRecord.ip_val] = ip_address;
    (*iter)[banRecord.netmask_id] = bitmask_id;
    (*iter)[banRecord.ip] = Glib::ustring::compose("%1.%2.%3.%4",
            Glib::ustring::format((ip_address >> 24) & 0xff),
            Glib::ustring::format((ip_address >> 16) & 0xff),
            Glib::ustring::format((ip_address >> 8) & 0xff),
            Glib::ustring::format(ip_address & 0xff));
    (*iter)[banRecord.netmask] = c_to_ustring(_v3_bitmasks[bitmask_id]);
    (*iter)[banRecord.user] = c_to_ustring(user);
    (*iter)[banRecord.by] = c_to_ustring(by);
    (*iter)[banRecord.reason] = c_to_ustring(reason);
}/*}}}*/
void
ManglerAdmin::BanEditorTree_cursor_changed_cb(void) {/*{{{*/
    Gtk::TreeModel::Path path;
    Gtk::TreeViewColumn *column;
    BanEditorTree->get_cursor(path, column);
    Gtk::TreeModel::iterator iter = BanEditorModel->get_iter(path);
    bool isBan( false );
    if (iter) {
        isBan = true;
        currentBanID = (*iter)[banRecord.id];
        copyToEntry("BanIPAddress", (*iter)[banRecord.ip]);
        copyToCombobox("BanNetmask", (*iter)[banRecord.netmask_id], BanNetmaskDefault);
        copyToEntry("BanReason", (*iter)[banRecord.reason]);
    }
    setWidgetSensitive("BanAdd", true);
    setWidgetSensitive("BanRemove", isBan);
    BanEditor->set_sensitive(isBan);
}/*}}}*/
void
ManglerAdmin::BanAdd_clicked_cb(void) {/*{{{*/
    Glib::RefPtr<Gtk::TreeSelection> banSelection = BanEditorTree->get_selection();
    banSelection->unselect_all();
    currentBanID = BanEditorModel->children().size() + 1;
    copyToEntry("BanIPAddress", "");
    copyToCombobox("BanNetmask", BanNetmaskDefault);
    copyToEntry("BanReason", "");
    setWidgetSensitive("BanRemove", false);
    setWidgetSensitive("BanAdd", false);
    BanEditor->set_sensitive(true);
}/*}}}*/
void
ManglerAdmin::BanRemove_clicked_cb(void) {/*{{{*/
    Gtk::TreeModel::Path path;
    Gtk::TreeViewColumn *column;
    BanEditorTree->get_cursor(path, column);
    Gtk::TreeModel::iterator iter = BanEditorModel->get_iter(path);
    if (! iter) return;
    uint32_t ip_address = (*iter)[banRecord.ip_val];
    uint16_t bitmask_id = (*iter)[banRecord.netmask_id];
    v3_admin_ban_remove(bitmask_id, ip_address);
    v3_admin_ban_list();
}/*}}}*/
void
ManglerAdmin::BanUpdate_clicked_cb(void) {/*{{{*/
    uint32_t ip_address = 0;
    char *ip_str = ::strdup(ustring_to_c(getFromEntry("BanIPAddress") + ".").c_str());
    char *ip_pos[5] = { ip_str };
    int ctr = 0;
    while (ctr < 4 && (ip_pos[ctr+1] = strchr(ip_pos[ctr], '.')) && ++ctr) {
        *ip_pos[ctr] = 0;
        ip_pos[ctr] += 1;
        ip_address |= ((uint8_t)atoi(ip_pos[ctr-1])) << (8*(4-ctr));
    }
    ::free(ip_str);
    if (ctr != 4 || !ip_address) {
        statusbarPush("Invalid IP address.");
        return;
    }
    uint16_t bitmask_id = getFromCombobox("BanNetmask", BanNetmaskDefault);
    char *reason = ::strdup(ustring_to_c(getFromEntry("BanReason")).c_str());
    v3_admin_ban_remove(bitmask_id, ip_address);
    v3_admin_ban_add(bitmask_id, ip_address, "Remotely added IP", reason);
    ::free(reason);
    v3_admin_ban_list();
}/*}}}*/
void
ManglerAdmin::clearBans(void) {/*{{{*/
    BanEditorModel->clear();

    BanEditor->set_sensitive(false);
    setWidgetSensitive("BanAdd", true);
    setWidgetSensitive("BanRemove", false);
    setWidgetSensitive("BanUpdate", true);

    currentBanID = 0xff;
}/*}}}*/

void
ManglerAdmin::clear(void) {/*{{{*/
    setWidgetSensitive("ServerVBox", true);
    SrvIsNotUpdating = true;
    clearChannels();
    clearRanks();
    clearBans();
}/*}}}*/

