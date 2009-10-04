/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 */

#include <gtkmm.h>
#include "channeltree.h"
#include "manglernetwork.h"

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
        Gtk::MessageDialog                  *mdialog;
        Gtk::Window                         *window;
        std::map<std::string, Glib::RefPtr<Gdk::Pixbuf> >  icons;
        Glib::RefPtr<Gtk::StatusIcon>       statusIcon;
        ManglerChannelTree                     *channelTree;
        ManglerNetwork                         *network;


        Glib::Thread                        *networkThread;

        
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

        // quick connect signal handlers
        void qcConnectButton_clicked_cb(void);
        void qcCancelButton_clicked_cb(void);

        void disconnect(void);
};

struct _cli_options {
    bool uifromfile;
    std::string uifilename;
};


extern Mangler *mangler;

#endif

