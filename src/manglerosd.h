#ifndef _MANGLEROSD_H
#define _MANGLEROSD_H
#include "config.h"

#ifdef HAVE_XOSD

#include <stdint.h>
#include <string>
#include <list>
#include <xosd.h>
extern "C" {
#include <ventrilo3.h>
}

class ManglerOsd {/*{{{*/
    private:
        xosd                             *osd;
        int                              osd_max_lines;
        std::list<std::string>           userList;
        void createOsd(void);

    public:
        ManglerOsd();
        void destroyOsd(void);
        bool checkOsdEnabled(void);
        void updateOsd(void);
        void addUser(uint32_t);
        void removeUser(uint32_t);

};/*}}}*/

#endif
#endif

/* vim: set ts=4 sw=4: */

