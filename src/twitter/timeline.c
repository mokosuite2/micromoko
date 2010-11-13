/*
 * Micromoko Twitter library
 * Timeline functions
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
#include <glib.h>
#include <rest/rest-proxy.h>
#include <rest/rest-xml-parser.h>
#include <rest/oauth-proxy.h>
#include <stdarg.h>

#include "twitter.h"
#include "private.h"

twitter_status* parse_status(RestXmlNode* status)
{
    RestXmlNode* n;

    twitter_status* e = calloc(1, sizeof(twitter_status));

    n = rest_xml_node_find(status, "id");
    if (n) e->id = strdup(n->content);

    // TODO timestamp

    n = rest_xml_node_find(status, "text");
    if (n) e->text = strdup(n->content);

    n = rest_xml_node_find(status, "source");
    if (n) e->source = strdup(n->content);

    // TODO tutto il resto :)
    return e;
}

static void _dump_child(void* key, void* value, void* data)
{
    DEBUG("key = \"%s\", value = %p (compare=%d)", (char*) key, value, strcmp((char*)key, "status"));
}

static void _timeline(twitter_session* session, twitter_call* call, const char* payload, goffset length, void* userdata)
{
    callback_pack* data = userdata;

    DEBUG("got timeline data! (%llu bytes)", length);
    RestXmlParser* parse = rest_xml_parser_new();
    RestXmlNode* root = rest_xml_parser_parse_from_data(parse, payload, length);
    if (!root || strcmp(root->name, "statuses")) {
        WARN("no timeline data!!");
        return;
    }

    // prepare callback data
    Eina_List* list = NULL;

    // get children (status tags)
    g_hash_table_foreach(root->children, _dump_child, NULL);
    RestXmlNode* chain = rest_xml_node_find(root, "status");
    while (chain) {
        twitter_status* e = parse_status(chain);
        if (e)
            list = eina_list_append(list, e);

        chain = chain->next;
    }

    // user callback
    TwitterTimelineCallback cb = data->callback;
    if (cb)
        (cb)(session, call, list, data->userdata);

    // free all
    rest_xml_node_unref(root);
    g_object_unref(parse);
}

/**
 * Requests the home timeline.
 * @param session
 * @param callback
 * @param userdata
 * @return the call on success
 */
twitter_call* twitter_get_home_timeline(twitter_session* session, TwitterTimelineCallback callback, void* userdata)
{
    callback_pack* data = malloc(sizeof(callback_pack));
    data->callback = callback;
    data->userdata = userdata;

    return twitter_session_call_new(session, "statuses/home_timeline", "GET", _timeline, data);

}
