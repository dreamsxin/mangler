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

#ifndef _MANGLERADMIN_H
#define _MANGLERADMIN_H

#include <vector>

extern "C" {
#include <ventrilo3.h>
}

class iniFile;

class ManglerAdmin {
    public:
        ManglerAdmin(Glib::RefPtr<Gtk::Builder> builder);
        ~ManglerAdmin();
        Gtk::Window         *adminWindow;
        void channelUpdated(v3_channel *channel);
        void channelRemoved(uint32_t chanid);
        void channelRemoved(v3_channel *channel);
        void channelAdded(v3_channel *channel);
        void channelSort(bool alphanumeric);
        void clearChannels(void);
        void accountUpdated(v3_account *account);
        void accountAdded(v3_account *account);
        void accountRemoved(uint32_t acctid);
        void accountRemoved(v3_account *account);
        static void trimString(Glib::ustring &s);
        void rankUpdated(v3_rank *rank);
        void rankAdded(v3_rank *rank);
        void rankRemoved(uint16_t rankid);
        void rankRemoved(v3_rank *rank);
        Gtk::Alignment                      *ServerTab;
        Gtk::Alignment                      *ChannelsTab;
        Gtk::Alignment                      *UsersTab;
        Gtk::Alignment                      *RanksTab;
        Gtk::Button                         *UserAdd;

    protected:
        Glib::RefPtr<Gtk::Builder>          builder;
        Gtk::Statusbar                      *AdminStatusbar;
        guint                               StatusbarCount;
        time_t                              StatusbarTime;
        iniFile                             *usertemplates;
       
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

        /* user editor stuff */
        class adminCheckModelColumns : public Gtk::TreeModel::ColumnRecord {
            public:
                adminCheckModelColumns() { add(id); add(on); add(name); }
                Gtk::TreeModelColumn<uint32_t>              id;
                /* Gtk::CellRendererToggle                     toggle; */
                Gtk::TreeModelColumn<bool>                  on;
                Gtk::TreeModelColumn<Glib::ustring>         name;
        } adminCheckRecord;
        

        adminModelColumns                   UserEditorColumns;
        Glib::RefPtr<Gtk::TreeStore>        UserEditorTreeModel;
        Gtk::TreeView                       *UserEditorTree;

        adminCheckModelColumns              UserChanAdminColumns;
        Glib::RefPtr<Gtk::TreeStore>        UserChanAdminModel;
        Gtk::TreeView                       *UserChanAdminTree;

        adminCheckModelColumns              UserChanAuthColumns;
        Glib::RefPtr<Gtk::TreeStore>        UserChanAuthModel;
        Gtk::TreeView                       *UserChanAuthTree;
        
        adminModelColumns                   UserOwnerColumns;
        Glib::RefPtr<Gtk::TreeStore>        UserOwnerModel;
        
        adminModelColumns                   UserRankColumns;
        Glib::RefPtr<Gtk::TreeStore>        UserRankModel;

        adminModelColumns                   UserDuplicateIPsColumns;
        Glib::RefPtr<Gtk::TreeStore>        UserDuplicateIPsModel;
        
        adminModelColumns                   UserDefaultChannelColumns;
        Glib::RefPtr<Gtk::TreeStore>        UserDefaultChannelModel;
        
        Gtk::ComboBoxEntry                  *UserTemplate;
        adminModelColumns                   UserTemplateColumns;
        Glib::RefPtr<Gtk::TreeStore>        UserTemplateModel;

        Gtk::VBox                           *UserEditor;
        Gtk::Table                          *UserInfoSection;
        Gtk::VBox                           *UserNetworkSection;
        Gtk::VBox                           *UserTransmitSection;
        Gtk::VBox                           *UserDisplaySection;
        Gtk::VBox                           *UserAdminSection;
        Gtk::Button                         *UserRemove;
        uint32_t                            currentUserID;
        
        /* rank editor stuff */
        class rankModelColumns : public Gtk::TreeModel::ColumnRecord {
            public:
                rankModelColumns() { add(id); add(name); add(level); add(description); }
                Gtk::TreeModelColumn<uint16_t>              id;
                Gtk::TreeModelColumn<long>                  level;
                Gtk::TreeModelColumn<Glib::ustring>         name;
                Gtk::TreeModelColumn<Glib::ustring>         description;
        } rankRecord;
        
        rankModelColumns                    RankEditorColumns;
        Glib::RefPtr<Gtk::TreeStore>        RankEditorModel;
        Gtk::TreeView                       *RankEditorTree;
        Gtk::VBox                           *RankEditor;
        uint16_t                            currentRankID;

        /* generic pointers and window pointer */
        Gtk::Button         *button;
        Gtk::Entry          *entry;
        Gtk::CheckButton    *checkbutton;
        Gtk::ComboBox       *combobox;
        Gtk::Label          *label;
        Gtk::SpinButton     *spinbutton;
        Gtk::ToggleButton   *togglebutton;
        Gtk::Arrow          *arrow;

        /* admin window main functions and callbacks */
        void adminWindow_show_cb(void);
        void adminWindow_hide_cb(void);
        void copyToEntry(const char *widgetName, Glib::ustring src);
        void copyToSpinbutton(const char *widgetName, uint32_t src);
        void copyToCheckbutton(const char *widgetName, bool src);
        void copyToCombobox(const char *widgetName, uint32_t src, uint32_t deflt = 0);
        Glib::ustring getFromEntry(const char *widgetName);
        uint32_t getFromSpinbutton(const char *widgetName);
        bool getFromCheckbutton(const char *widgetName);
        uint32_t getFromCombobox(const char *widgetName, uint32_t deflt = 0);
        void setWidgetSensitive(const char *widgetName, bool widgetSens = true);
        bool statusbarPop(void);
        void statusbarPush(Glib::ustring msg);

        /* channel editor functions and callbacks */
        Glib::ustring getChannelPathString(uint32_t id, Gtk::TreeModel::Children children);
        Gtk::TreeModel::Row getChannel(uint32_t id, Gtk::TreeModel::Children children, bool hasCheckbox = false);
        void populateChannelEditor(v3_channel *channel);
        int channelSortFunction(const Gtk::TreeModel::iterator &left, const Gtk::TreeModel::iterator &right);
        void ChannelTree_cursor_changed_cb(void);
        void AddChannel_clicked_cb(void);
        void RemoveChannel_clicked_cb(void);
        void UpdateChannel_clicked_cb(void);
        void CloseButton_clicked_cb(void);
        void LoadCodecFormats(void);
        void ChannelProtMode_changed_cb(void);
        void ChannelVoiceMode_changed_cb(void);

        /* user editor functions and callbacks */
        Gtk::TreeModel::Row getAccount(uint32_t id, Gtk::TreeModel::Children children);
        void populateUserEditor(v3_account *account);
        void setAdminCheckTree(Gtk::TreeModel::Children children, uint16_t *chanids, int chan_count);
        void getAdminCheckTree(Gtk::TreeModel::Children children, std::vector<uint16_t> &chanids);
        void getAdminCheckTree(Gtk::TreeModel::Children children, uint16_t *&chanids, int &chan_count);
        void UserTree_cursor_changed_cb(void);
        void UserAdd_clicked_cb(void);
        void UserRemove_clicked_cb(void);
        void UserUpdate_clicked_cb(void);
        void UserInfoButton_toggled_cb(void);
        void UserNetworkButton_toggled_cb(void);
        void UserTransmitButton_toggled_cb(void);
        void UserDisplayButton_toggled_cb(void);
        void UserAdminButton_toggled_cb(void);
        void UserChanAdminButton_toggled_cb(void);
        void UserChanAuthButton_toggled_cb(void);
        /* user editor 'profile' stuff */
        void readUserTemplates(void);
        void UserTemplate_changed_cb(void);
        void UserTemplateLoad_clicked_cb(void);
        void UserTemplateSave_clicked_cb(void);

        /* rank editor callbacks */
        Gtk::TreeModel::iterator getRank(uint16_t id, Gtk::TreeModel::Children children);
        void RankEditorTree_cursor_changed_cb(void);
        void RankAdd_clicked_cb(void);
        void RankRemove_clicked_cb(void);
        void RankUpdate_clicked_cb(void);

};

#endif
