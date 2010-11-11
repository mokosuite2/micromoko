/*
 * Micromoko
 * Authorization window
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
#include "auth.h"

#include <glib/gi18n-lib.h>

static MokoWin* win = NULL;
static Evas_Object* btn_auth = NULL;
static Evas_Object* msg_status = NULL;
static Evas_Object* pin_entry = NULL;

static void _delete(void* mokowin, Evas_Object* obj, void* event_info)
{
    // TODO che famo?
    mokowin_destroy((MokoWin *)mokowin);
}

static void update_status(const char* status)
{
    elm_label_label_set(msg_status, status);
}

static void _access_token(twitter_session* session, void* userdata)
{
    // TODO now what?
    remote_config_service_set_string(home_config, "auth", "access_token", session->access.key);
    remote_config_service_set_string(home_config, "auth", "access_token_secret", session->access.secret);
}

static bool _pin_response(void* userdata)
{
    // remove auth button
    evas_object_del(btn_auth);
    btn_auth = NULL;

    pin_entry = elm_entry_add(win->win);
    evas_object_size_hint_weight_set(pin_entry, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(pin_entry, EVAS_HINT_FILL, 0.5);

    elm_box_pack_before(win->win, pin_entry, msg_status);
    evas_object_show(pin_entry);

    update_status(_("Paste PIN code above."));
    return FALSE;
}

static void _request_token(twitter_session* session, void* userdata)
{
    update_status(_("Starting browser..."));

    // access token for pin :)
    char* cmd = g_strdup_printf("xdg-open \"%s%s?oauth_token=%s\"",
        TWITTER_BASE_URI, TWITTER_AUTHORIZE_FUNC, session->request.key);
    system(cmd);
    g_free(cmd);

    ecore_timer_add(5, _pin_response, NULL);
    //twitter_session_oauth_access_token(session, pin, _access_token, userdata);
    // TODO swich UI to pin request :)
}

static void _authorize(void* data, Evas_Object* obj, void* event_info)
{
    // start authentication
    elm_object_disabled_set(obj, TRUE);
    update_status(_("Requesting token..."));
    twitter_session_oauth_request_token(global_session, _request_token, NULL);
}

void auth_win(void)
{
    if (!win) {
        win = mokowin_new("micromoko_auth", TRUE);
        if (win == NULL) {
            EINA_LOG_ERR("cannot create authorization window.");
            return;
        }

        win->delete_callback = _delete;

        elm_win_title_set(win->win, _("Authorization"));

        mokowin_create_vbox(win, FALSE);
        mokowin_set_title(win, _("Authorization"));

        //mokowin_menu_enable(win);
        //mokowin_menu_set(win, make_menu());

        // big auth button :D
        btn_auth = elm_button_add(win->win);
        elm_button_label_set(btn_auth, _("Authorize to Twitter"));
        evas_object_size_hint_weight_set(btn_auth, EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(btn_auth, EVAS_HINT_FILL, 0.5);
        evas_object_smart_callback_add(btn_auth, "clicked", _authorize, NULL);

        mokowin_pack_end(win, btn_auth, FALSE);
        evas_object_show(btn_auth);

        // label/status
        msg_status = elm_label_add(win->win);
        elm_label_label_set(msg_status, _("Click the button to authorize."));
        evas_object_size_hint_weight_set(msg_status, EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(msg_status, 0.5, 0.5);

        mokowin_pack_end(win, msg_status, FALSE);
        evas_object_show(msg_status);

        // TEST
        evas_object_resize(win->win, 480, 640);
    }

    mokowin_activate(win);
}
