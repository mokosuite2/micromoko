/*
 * Micromoko
 * Generic timeline window
 * Copyright (C) 2009-2010 Daniele Ricci <daniele.athome@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <Elementary.h>
#include <mokosuite/utils/utils.h>
#include <mokosuite/ui/gui.h>

#include "globals.h"
#include "twitter/twitter.h"
#include "timeline.h"

#include <glib/gi18n-lib.h>

static void _delete(void* mokowin, Evas_Object* obj, void* event_info)
{
    // TODO che famo?
    mokowin_destroy((MokoWin *)mokowin);
}

static void _home_timeline(twitter_session* session, twitter_call* call, const char* payload, void* userdata)
{
    EINA_LOG_DBG("PAYLOAD:\n%s", payload);
}

MokoWin* timeline_new(int type)
{
    MokoWin* win = mokowin_new("micromoko_timeline", TRUE);
    if (win == NULL) {
        EINA_LOG_ERR("cannot create timeline window.");
        return NULL;
    }

    win->delete_callback = _delete;

    elm_win_title_set(win->win, _("Micromoko"));

    mokowin_create_vbox(win, TRUE);
    const char* title;
    switch (type) {
        case TIMELINE_USER:
            title = "User timeline";
            break;
        case TIMELINE_PUBLIC:
            title = "Public timeline";
            break;
        default:
            title = "Timeline";
    }
    mokowin_set_title(win, _(title));

    //mokowin_menu_enable(win);
    //mokowin_menu_set(win, make_menu());

    // TODO robbaccia

    #ifdef DEBUG
    evas_object_resize(win->win, 480, 640);
    #endif
    mokowin_activate(win);

    // TEST
    twitter_session_call_new(global_session,
        "statuses/home_timeline", "GET",
        _home_timeline, NULL);

    return win;
}
