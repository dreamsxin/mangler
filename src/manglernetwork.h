/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 */

#ifndef _MANGLERNETWORK_H
#define _MANGLERNETWORK_H

class ManglerNetwork
{
    public:
        ManglerNetwork(Glib::RefPtr<Gtk::Builder> builder);
        void connect(void);
        void disconnect(void);
        Gtk::Button *button;
        Gtk::MessageDialog *msgdialog;
        Glib::RefPtr<Gtk::Builder> builder;
};

#endif

