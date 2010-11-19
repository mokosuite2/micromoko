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
#include <mokosuite/utils/misc.h>
#include <mokosuite/ui/gui.h>

#include "globals.h"
#include "twitter/twitter.h"
#include "timeline.h"

#include <glib/gi18n-lib.h>

static void _delete(void* mokowin, Evas_Object* obj, void* event_info)
{
    // TODO che famo?
    mokowin_destroy(MOKO_WIN(mokowin));
    elm_exit();
}

static Evas_Object* status_bubble(MokoWin* win, twitter_status* e)
{
    Evas_Object* msg = elm_bubble_add(win->win);
    evas_object_size_hint_weight_set(msg, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(msg, EVAS_HINT_FILL, 0.0);
    elm_bubble_label_set(msg, e->user->name);
    if (e->timestamp >= 0)
        elm_bubble_info_set(msg, get_time_repr(e->timestamp));

    elm_bubble_corner_set(msg, "top_left");
    evas_object_show(msg);

    Evas_Object* lbl = elm_anchorblock_add(win->win);
    evas_object_size_hint_weight_set(lbl, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(lbl, 0.0, 0.0);
    evas_object_data_set(lbl, "twitter_status", e);

    char* fin_text = g_strdup_printf("%s<br><font_size=7>via %s</>", e->text, e->source);
    elm_anchorblock_text_set(lbl, fin_text);
    g_free(fin_text);

    evas_object_show(lbl);
    elm_bubble_content_set(msg, lbl);

    return msg;
}

static void _home_timeline(twitter_session* session, twitter_call* call, Eina_List* timeline, void* mokowin)
{
    EINA_LOG_DBG("got home timeline (timeline=%p)", timeline);

    MokoWin* win = MOKO_WIN(mokowin);
    Eina_List* iter;
    twitter_status* e;
    EINA_LIST_FOREACH(timeline, iter, e) {
        //EINA_LOG_DBG("Status via %s: id=%s, text=\"%s\"", e->source, e->id, e->text);
        Evas_Object* msg = status_bubble(win, e);
        elm_box_pack_end(win->vbox, msg);
    }
}

static void _status_sent(twitter_session* session, twitter_call* call, twitter_status* e, void* mokowin)
{
    MokoWin* win = MOKO_WIN(mokowin);
    void** data = (void**) win->data;
    Evas_Object* entry = data[TL_DATA_STATUS_ENTRY];
    Evas_Object* button = data[TL_DATA_TWEET_BUTTON];

    if (e) {
        Evas_Object* msg = status_bubble(win, e);
        mokowin_pack_start(win, msg, FALSE);
        elm_entry_entry_set(entry, "");
    }
    else {
        EINA_LOG_WARN("status update failed!");
        moko_popup_alert_new(win, _("Status update failed."));
    }

    elm_object_disabled_set(entry, FALSE);
    elm_object_disabled_set(button, FALSE);
}

static void _update_status(void* mokowin, Evas_Object* obj, void* event_info)
{
    MokoWin* win = MOKO_WIN(mokowin);
    void** data = (void**) win->data;

    Evas_Object* entry = data[TL_DATA_STATUS_ENTRY];
    Evas_Object* button = data[TL_DATA_TWEET_BUTTON];
    elm_object_disabled_set(entry, TRUE);
    elm_object_disabled_set(button, TRUE);

    const char* text = elm_entry_entry_get(entry);
    twitter_update_status(global_session, text, NULL, _status_sent, win);
}

static Evas_Object* make_composer(MokoWin* win)
{
    void** data = (void**) win->data;

    Evas_Object* hbox = elm_box_add(win->win);
    elm_box_horizontal_set(hbox, TRUE);
    evas_object_size_hint_weight_set(hbox, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(hbox, EVAS_HINT_FILL, 0.0);

    Evas_Object* reply = elm_entry_add(win->win);
    elm_entry_single_line_set(reply, FALSE);
    evas_object_size_hint_weight_set(reply, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(reply, EVAS_HINT_FILL, EVAS_HINT_FILL);

    data[TL_DATA_STATUS_ENTRY] = reply;
    elm_box_pack_start(hbox, reply);
    evas_object_show(reply);

    Evas_Object* send = elm_button_add(win->win);
    elm_button_label_set(send, _("Tweet!"));
    evas_object_size_hint_weight_set(send, 0.0, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(send, 1.0, EVAS_HINT_FILL);
    evas_object_smart_callback_add(send, "clicked", _update_status, win);

    data[TL_DATA_TWEET_BUTTON] = send;
    elm_box_pack_end(hbox, send);
    evas_object_show(send);

    return hbox;
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
        case TIMELINE_HOME:
            title = "Home timeline";
            break;
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

    // TODO menu hover
    //mokowin_menu_enable(win);
    //mokowin_menu_set(win, make_menu());

    // user data
    void** data = calloc(TL_DATA_MAX, sizeof(void*));
    win->data = (void*) data;

    Evas_Object* reply = make_composer(win);
    mokowin_pack_end(win, reply, TRUE);
    evas_object_show(reply);

    #ifdef DEBUG
    evas_object_resize(win->win, 480, 640);
    #endif
    mokowin_activate(win);

    // TEST
    twitter_get_home_timeline(global_session, _home_timeline, win);

    return win;
}
