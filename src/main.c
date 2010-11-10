/*
 * Micromoko
 * Entry point
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

#include <mokosuite/utils/utils.h>
#include <mokosuite/utils/remote-config-service.h>
#include <mokosuite/utils/dbus.h>
#include <mokosuite/ui/gui.h>
#include <glib.h>
#include <dbus/dbus-glib-bindings.h>

// TEST
#include <readline/readline.h>
#include <readline/history.h>

#include "globals.h"
#include "twitter/twitter.h"

#define MICROMOKO_NAME               "org.mokosuite.micromoko"
#define MICROMOKO_CONFIG_PATH        MOKOSUITE_DBUS_PATH "/Micromoko/Config"


// default log domain
int _log_dom = -1;

RemoteConfigService* home_config = NULL;

static void _access_token(twitter_session* session, void* userdata)
{
    // TODO now what?
    remote_config_service_set_string(home_config, "auth", "access_token", session->access.key);
    remote_config_service_set_string(home_config, "auth", "access_token_secret", session->access.secret);
}

static void _request_token(twitter_session* session, void* userdata)
{
    // TODO access token for pin :)
    char* cmd = g_strdup_printf("xdg-open \"%s%s?oauth_token=%s\"",
        TWITTER_BASE_URI, TWITTER_AUTHORIZE_FUNC, session->request.key);
    system(cmd);
    g_free(cmd);

    char* pin = readline("PIN: ");
    twitter_session_oauth_access_token(session, pin, _access_token, userdata);
}

int main(int argc, char* argv[])
{
    // TODO initialize Intl
    setlocale(LC_ALL, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    // initialize log
    eina_init();
    _log_dom = eina_log_domain_register(PACKAGE, EINA_COLOR_CYAN);
    eina_log_domain_level_set(PACKAGE, LOG_LEVEL);

    EINA_LOG_INFO("%s version %s", PACKAGE_NAME, VERSION);

    // glib integration
    mokosuite_utils_init();
    mokosuite_utils_glib_init(TRUE);
    mokosuite_ui_init(argc, argv);
    EINA_LOG_DBG("Loading data from %s", MICROMOKO_DATADIR);

    // dbus name
    DBusGConnection* session_bus = dbus_session_bus();
    if (!session_bus || !dbus_request_name(session_bus, MICROMOKO_NAME))
        return EXIT_FAILURE;

    // config database
    char* cfg_file = g_strdup_printf("%s/.config/%s/settings.conf", g_get_home_dir(), PACKAGE);
    home_config = remote_config_service_new(session_bus,
        MICROMOKO_CONFIG_PATH,
        cfg_file);
    g_free(cfg_file);

    // twitter init
    twitter_init(home_config);

    elm_theme_extension_add(NULL, MICROMOKO_DATADIR "/theme.edj");
    // TODO additional themes

    // TODO public timeline or last open window?

    // TEST twitter test :)
    oauth_token consumer = {
        "dY49wvIY7386ET15vCRVQ",
        "D8wKnvbTQqxJmxBC0JyHVY6LpHdpTBbJwyISlrUg8"
    };
    twitter_session* sess = twitter_session_new(&consumer);

    oauth_token access_token = {NULL, NULL};
    remote_config_service_get_string(home_config, "auth", "access_token", &access_token.key);
    remote_config_service_get_string(home_config, "auth", "access_token_secret", &access_token.secret);
    if (access_token.key != NULL)
        twitter_session_set_access_token(sess, &access_token);
    else
        twitter_session_oauth_request_token(sess, _request_token, NULL);


    // TEST twitter test

    elm_run();
    elm_shutdown();

    return EXIT_SUCCESS;
}
