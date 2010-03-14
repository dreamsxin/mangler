#include "mangler.h"
#include "manglerosd.h"

#ifdef HAVE_XOSD

using namespace std;

ManglerOsd::ManglerOsd() {/*{{{*/
    userList = list<string>::list();
    osd_max_lines = 10;
    osd = NULL;
}/*}}}*/

void
ManglerOsd::createOsd(void) {/*{{{*/
    if (osd) {
        destroyOsd();
    }
    osd = xosd_create(osd_max_lines);
    //xosd_set_pos(osd,XOSD_top);
    //xosd_set_align(osd,XOSD_center);
    xosd_set_pos(osd, (xosd_pos)(Mangler::config["OnScreenDisplayVerticalPosition"].toInt()));
    xosd_set_align(osd, (xosd_align)(Mangler::config["OnScreenDisplayHorizontalAlignment"].toInt()));
    if (Mangler::config["OnScreenDisplayFontSize"].toDouble() > 8.0) {
        Glib::ustring fontstr = Glib::ustring::compose( "-*-*-*-*-*-*-0-%1-*-*-*-*-*-*", (int)(Mangler::config["OnScreenDisplayFontSize"].toDouble() * 10.0));
        xosd_set_font(osd, fontstr.c_str());
    }
    if (Mangler::config["OnScreenDisplayColor"].length()) {
        xosd_set_colour(osd, Mangler::config["OnScreenDisplayColor"].toCString());
    }
}/*}}}*/

void
ManglerOsd::destroyOsd(void) {/*{{{*/
    userList.clear();
    if (osd) {
        xosd_destroy(osd);
        osd = NULL;
    }
}/*}}}*/

bool
ManglerOsd::checkOsdEnabled(void) {/*{{{*/
    if (!Mangler::config["OnScreenDisplayEnabled"].toBool()) {
        if (osd) {
            destroyOsd();
        }
        return false;
    } else if (!osd) {
        createOsd();
    }
    return true;
}/*}}}*/

void
ManglerOsd::updateOsd(void) {/*{{{*/
    if (!checkOsdEnabled()) {
        return;
    }
    if (userList.empty()) {
        xosd_hide(osd);
        return;
    }
    xosd_pos mosd_pos = (xosd_pos)(Mangler::config["OnScreenDisplayVerticalPosition"].toInt());

    int i = (mosd_pos == XOSD_bottom) ? osd_max_lines - 1 : 0;
    list<string>::iterator it;
    for(it=userList.begin(); it!= userList.end(); ++it) {
        xosd_display(osd,i,XOSD_string,it->c_str());
        i += (mosd_pos == XOSD_bottom) ? -1 : 1;
        if ( i >= osd_max_lines || i < 0) {
            break;
        }
    }
    
    while (i >= 0 && i < osd_max_lines) {
        xosd_display(osd,i,XOSD_string," ");
        i += (mosd_pos == XOSD_bottom) ? -1 : 1;
    }
}/*}}}*/

/*
 * Adds a user to the list of users talking
 */
void
ManglerOsd::addUser(uint32_t id) {/*{{{*/
    if (!checkOsdEnabled()) {
        return;
    }

    v3_user *u;
    list<string>::iterator it;
    if ((u = v3_get_user(id))) {
        /* Don't 're-add' existing names */
        for(it=userList.begin(); it!= userList.end(); ++it) {
            if (strcmp(it->c_str(),u->name)==0) {
                return;
            }
        }
        string s(u->name);
        userList.push_front(s);
        updateOsd();
    }
}/*}}}*/


/*
 * Remove a user to the list of users talking
 */
void
ManglerOsd::removeUser(uint32_t id) {/*{{{*/
    if (!checkOsdEnabled()) {
        return;
    }

    v3_user *u;
    if ((u = v3_get_user(id))) {
        string s(u->name);
        userList.remove(s);
        updateOsd();
    }
}/*}}}*/

#endif

/* vim: set ts=4 sw=4: */

