/*
 * vim: softtabstop=4 shiftwidth=4 cindent foldmethod=marker expandtab
 */

#ifndef _MANGLERNETWORK_H
#define _MANGLERNETWORK_H

class ManglerNetwork
{
    public:
        ManglerNetwork(Glib::RefPtr<Gtk::Builder> builder);
        void connect(std::string hostname, std::string port, std::string username, std::string password);
        void disconnect(void);
        Gtk::Button *button;
        Gtk::Label *label;
        Gtk::MessageDialog *msgdialog;
        Glib::RefPtr<Gtk::Builder> builder;
};

#endif

