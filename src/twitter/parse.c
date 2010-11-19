/*
 * Micromoko Twitter library
 * XML nodes parsing functions
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

#include <glib.h>
#include <rest/rest-xml-parser.h>
#include <stdarg.h>
#include <time.h>

#include "twitter.h"
#include "private.h"

static char* string_content(RestXmlNode* node, const char* name)
{
    RestXmlNode* n = rest_xml_node_find(node, name);
    if (n && n->content) return strdup(n->content);

    return NULL;
}

twitter_user* twitter_parse_user(RestXmlNode* user)
{
    twitter_user* e = calloc(1, sizeof(twitter_user));

    e->id = string_content(user, "id");
    e->name = string_content(user, "name");
    e->screen_name = string_content(user, "screen_name");
    e->location = string_content(user, "location");
    e->description = string_content(user, "description");

    // TODO tutto il resto :)
    return e;
}

twitter_status* twitter_parse_status(RestXmlNode* status)
{
    RestXmlNode* n;

    twitter_status* e = calloc(1, sizeof(twitter_status));

    e->id = string_content(status, "id");
    char* ts = string_content(status, "created_at");

    struct tm tm;
    time_t epoch = -1;

    // backup locale
    char* old_locale = getenv("LC_TIME");
    if (!old_locale)
        old_locale = getenv("LANG");

    setlocale(LC_TIME, "C");
    if ( strptime(ts, "%a %b %d %H:%M:%S %z %Y", &tm) != NULL )
        epoch = mktime(&tm);
    else
        INFO("cannot parse timestamp");

    // restore locale
    setlocale(LC_TIME, old_locale);

    e->timestamp = (gint64) epoch;

    e->text = string_content(status, "text");
    e->source = string_content(status, "source");

    n = rest_xml_node_find(status, "user");
    if (n) e->user = twitter_parse_user(n);

    // TODO tutto il resto :)
    return e;
}
