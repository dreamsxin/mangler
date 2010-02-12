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

#include "config.h"
#include <gtkmm.h>
#include <sys/types.h>
#include <stdint.h>
#include <iostream>

class ManglerChannelTree;
class ManglerNetwork;
class ManglerAudio;
class ManglerSettings;
class ManglerServerList;
class ManglerChat;
class ManglerPrivChat;
class ManglerIntegration;
class ManglerAdmin;

extern "C" {
#include <ventrilo3.h>
}

#ifndef _MANGLER_H
#define _MANGLER_H

class Mangler
{
    public:
        Mangler(struct _cli_options *options);
        void startTransmit(void);
        void stopTransmit(void);
        void initialize(void);
        Gtk::Window                         *manglerWindow;
        Glib::RefPtr<Gtk::Builder>          builder;
        Gtk::Button                         *button;
        Gtk::ToggleButton                   *togglebutton;
        Gtk::Dialog                         *dialog;
        Gtk::AboutDialog                    *aboutdialog;
        Gtk::MessageDialog                  *msgdialog;
        Gtk::Window                         *window;
        Gtk::ProgressBar                    *progressbar;
        Gtk::Statusbar                      *statusbar;
        Gtk::Label                          *label;
        Gtk::Entry                          *entry;
        Gtk::ComboBox                       *combobox;
        Gtk::TextView                       *textview;
        Gtk::VBox                           *vbox;
        Gtk::CheckMenuItem                  *checkmenuitem;
        Gtk::MenuItem                       *menuitem;
        Gtk::Table                          *table;
        Gtk::CheckButton                    *checkbutton;
        Gtk::ProgressBar                    *inputvumeter;


        std::map<Glib::ustring, Glib::RefPtr<Gdk::Pixbuf> >  icons;
        Glib::RefPtr<Gtk::StatusIcon>       statusIcon;
        ManglerServerList                   *serverList;
        ManglerChat                         *chat;
        ManglerChannelTree                  *channelTree;
        ManglerNetwork                      *network;
        int32_t                             connectedServerId;
        std::map<uint32_t, ManglerAudio* >  outputAudio;
        std::map<uint16_t, ManglerPrivChat *> privateChatWindows;
        ManglerAudio                        *inputAudio;
        ManglerAudio                        *audioControl;
        ManglerSettings                     *settings;
        ManglerIntegration                  *integration;
        ManglerAdmin                       *admin;

        bool                                isTransmitting;
        bool                                isTransmittingButton;
        bool                                isTransmittingKey;
        bool                                isTransmittingMouse;
        bool                                iconified;
        bool                                isAdmin;
        bool                                isChanAdmin;
        bool                                muteSound;
        bool                                muteMic;
        bool                                motdAlways;

        // Autoreconnect feature stuff - Need ID's to kill threads if needed
        bool                                wantDisconnect;
        time_t                              lastAttempt;
        uint32_t                            lastServer;

        // These are used by the password entry dialog
        Gtk::Dialog                         *passwordDialog;
        Gtk::Entry                          *passwordEntry;
        Glib::ustring                       password;
        bool                                passwordStatus;

        // These are used by the kick/ban reason entry dialog
        Gtk::Dialog                         *reasonDialog;
        Gtk::Entry                          *reasonEntry;
        Glib::ustring                       reason;
        bool                                reasonStatus;
        bool                                reasonValid;

        // These are used by the text string entry dialog
        Gtk::Dialog                         *textStringChangeDialog;
        Gtk::Entry                          *textStringChangeCommentEntry;
        Gtk::Entry                          *textStringChangeURLEntry;
        Gtk::Entry                          *textStringChangeIntegrationEntry;
        Glib::ustring                       comment;
        Glib::ustring                       url;
        Glib::ustring                       integration_text;

        Glib::Thread                        *networkThread;

        //Less intensive than looking it up every time
        uint16_t                            myID;

        Glib::ustring getPasswordEntry(Glib::ustring title = "Password", Glib::ustring prompt = "Password");
        bool getReasonEntry(Glib::ustring title = "Reason", Glib::ustring prompt = "Reason");
        uint32_t getActiveServer(void);
        void setActiveServer(uint32_t row_number);
        void errorDialog(Glib::ustring message);

    protected:
        // button signal handlers
        void quickConnectButton_clicked_cb(void);
        void serverConfigButton_clicked_cb(void);
        void connectButton_clicked_cb(void);
        void commentButton_clicked_cb(void);
        void chatButton_clicked_cb(void);
        void bindingsButton_clicked_cb(void);
        void adminButton_clicked_cb(void);
        void settingsButton_clicked_cb(void);
        void aboutButton_clicked_cb(void);
        void xmitButton_toggled_cb(void);
        void statusIcon_activate_cb(void);
        void errorOKButton_clicked_cb(void);


        // menu bar signal handlers
        void buttonMenuItem_toggled_cb(void);
        void hideServerInfoMenuItem_toggled_cb(void);
        void hideGuestFlagMenuItem_toggled_cb(void);
        void quitMenuItem_activate_cb(void);
        void adminWindowMenuItem_activated_cb(void);

        bool getNetworkEvent(void);
        bool updateIntegration(void); // music player integration
        bool checkPushToTalkKeys(void);
        bool checkPushToTalkMouse(void);
        bool updateXferAmounts(void);

        // autoreconnect implementation
        bool reconnectStatusHandler(void);
        void onDisconnectHandler(void);

        // quick mute options
        void muteSoundCheckButton_toggled_cb(void);
        void muteMicCheckButton_toggled_cb(void);

        // quick connect signal handlers
        void qcConnectButton_clicked_cb(void);
        void qcCancelButton_clicked_cb(void);

        void motdOkButton_clicked_cb(void);

        // password dialog signal handlers
        void passwordDialogOkButton_clicked_cb(void);
        void passwordDialogCancelButton_clicked_cb(void);

        // kick/ban reason dialog signal handlers
        void reasonDialogOkButton_clicked_cb(void);
        void reasonDialogCancelButton_clicked_cb(void);

        // text string change dialog signal handlers
        void textStringChangeDialogOkButton_clicked_cb(void);
        void textStringChangeDialogCancelButton_clicked_cb(void);

        // program quit callback
        bool mangler_quit_cb(void);
};

struct _cli_options {
    bool uifromfile;
    Glib::ustring uifilename;
};

class ManglerError
{
    public:
        uint32_t        code;
        Glib::ustring     message;
        Glib::ustring     module;
        ManglerError(uint32_t code, Glib::ustring message, Glib::ustring module = "");
};

GdkFilterReturn ptt_filter(GdkXEvent *gdk_xevent, GdkEvent *event, gpointer data);

extern Mangler *mangler;

#endif

