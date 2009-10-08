/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 */

#include <gtkmm.h>
#include <iostream>
#include "channeltree.h"
#include "manglernetwork.h"
#include "mangleraudio.h"
#include "manglersettings.h"

extern "C" {
#include <ventrilo3.h>
}

#ifndef _MANGLER_H
#define _MANGLER_H

class Mangler
{
    public:
        Mangler(struct _cli_options *options);
        void initialize(void);
        Gtk::Window  *manglerWindow;
        Glib::RefPtr<Gtk::Builder>          builder;
        Gtk::Button                         *button;
        Gtk::Dialog                         *dialog;
        Gtk::MessageDialog                  *msgdialog;
        Gtk::Window                         *window;
        Gtk::ProgressBar                    *progressbar;
        Gtk::Statusbar                      *statusbar;
        Gtk::Label                          *label;
        Gtk::Entry                          *entry;
        std::map<std::string, Glib::RefPtr<Gdk::Pixbuf> >  icons;
        Glib::RefPtr<Gtk::StatusIcon>       statusIcon;
        ManglerChannelTree                  *channelTree;
        ManglerNetwork                      *network;
        ManglerAudio                        *audio;
        ManglerSettings                     *settings;

        Glib::Thread                        *networkThread;
        Glib::Thread                        *audioInputThread;
        Glib::Thread                        *audioOutputThread;

        
    protected:
        // signal handlers
        void quickConnectButton_clicked_cb(void);
        void serverConfigButton_clicked_cb(void);
        void connectButton_clicked_cb(void);
        void commentButton_clicked_cb(void);
        void chatButton_clicked_cb(void);
        void settingsButton_clicked_cb(void);
        void aboutButton_clicked_cb(void);
        void xmitButton_pressed_cb(void);
        void xmitButton_released_cb(void);
        bool getNetworkEvent(void);

        // quick connect signal handlers
        void qcConnectButton_clicked_cb(void);
        void qcCancelButton_clicked_cb(void);
};

struct _cli_options {
    bool uifromfile;
    std::string uifilename;
};

class ManglerError
{
    public:
        uint32_t        code;
        std::string     message;
        std::string     module;
        ManglerError(uint32_t code, std::string message, std::string module = "");
};

extern Mangler *mangler;

#endif

