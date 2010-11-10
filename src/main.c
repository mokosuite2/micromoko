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

#include "globals.h"
#include "twitter/twitter.h"

#define MICROMOKO_NAME               "org.mokosuite.micromoko"
#define MICROMOKO_CONFIG_PATH        MOKOSUITE_DBUS_PATH "/Micromoko/Config"


// default log domain
int _log_dom = -1;

RemoteConfigService* home_config = NULL;


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

    elm_run();
    elm_shutdown();

    return EXIT_SUCCESS;
}
