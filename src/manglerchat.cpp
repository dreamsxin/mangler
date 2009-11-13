#include "mangler.h"
#include "manglerchat.h"

ManglerChat::ManglerChat(Glib::RefPtr<Gtk::Builder> builder) {
    builder->get_widget("chatWindow", chatWindow);
    chatWindow->signal_show().connect(sigc::mem_fun(this, &ManglerChat::chatWindow_show_cb));
    chatWindow->signal_hide().connect(sigc::mem_fun(this, &ManglerChat::chatWindow_hide_cb));
    
    builder->get_widget("sendChat", button);
    button->signal_clicked().connect(sigc::mem_fun(this, &ManglerChat::chatWindowSendChat_clicked_cb));
    
    builder->get_widget("chatMessage", chatMessage);
    chatMessage->set_activates_default(true);
}

void ManglerChat::chatWindow_show_cb() {
    if(v3_is_loggedin()) {
        v3_join_chat();
    }
}

void ManglerChat::chatWindow_hide_cb() {
    if(v3_is_loggedin()) {
        v3_leave_chat();
    }
}

void ManglerChat::chatWindowSendChat_clicked_cb() {
    if(chatMessage->get_text_length()) {
        v3_send_chat_message((char *)ustring_to_c(chatMessage->get_text()).c_str());
        chatMessage->set_text("");
    }
}
