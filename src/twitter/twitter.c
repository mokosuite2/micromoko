/*
 * Micromoko Twitter library
 * Library entry point
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
#include <rest/rest-proxy.h>
#include <rest/oauth-proxy.h>

#include "twitter.h"
#include "private.h"

// default log domain
int _micromoko_twitter_utils_log_dom = -1;

// global
RemoteConfigService* config = NULL;


static void oauth_token_copy(oauth_token* dest, oauth_token* src)
{
    if (dest == NULL || src == NULL) return;

    dest->key = strdup(src->key);
    dest->secret = strdup(src->secret);
}

static void oauth_token_free_keys(oauth_token* token)
{
    free(token->key);
    free(token->secret);
}

static void oauth_proxy(twitter_session* session)
{
    if (!session->oauth_proxy)
        session->oauth_proxy = oauth_proxy_new(session->consumer.key, session->consumer.secret, TWITTER_BASE_URI, FALSE);
}

static void _access_token(OAuthProxy* proxy, GError* error, GObject* weak_object, void* userdata)
{
    DEBUG("got access token %s", oauth_proxy_get_token(proxy));
    twitter_session* sess = userdata;
    sess->access.key = strdup(oauth_proxy_get_token(proxy));
    sess->access.secret = strdup(oauth_proxy_get_token_secret(proxy));

    // user callback
    OAuthTokenCallback cb = sess->access_token_cb;
    if (cb)
        (cb)(sess, sess->access_token_data);
}

static void _request_token(OAuthProxy* proxy, GError* error, GObject* weak_object, void* userdata)
{
    DEBUG("got request token %s", oauth_proxy_get_token(proxy));
    twitter_session* sess = userdata;
    sess->request.key = strdup(oauth_proxy_get_token(proxy));
    sess->request.secret = strdup(oauth_proxy_get_token_secret(proxy));

    // user callback
    OAuthTokenCallback cb = sess->request_token_cb;
    if (cb)
        (cb)(sess, sess->request_token_data);
}

/**
 * Requests an OAuth access token to Twitter.
 * @param session
 * @param callback callback to be called when the access token has been requested
 * @return TRUE on success
 */
bool twitter_session_oauth_access_token(twitter_session* session, const char* pin, OAuthTokenCallback callback, void* userdata)
{
    session->access_token_cb = callback;
    session->access_token_data = userdata;

    oauth_proxy(session);
    return oauth_proxy_access_token_async(OAUTH_PROXY(session->oauth_proxy),
        TWITTER_ACCESS_TOKEN_FUNC, pin, _access_token, NULL, session, NULL);
}

/**
 * Requests an OAuth request token to Twitter.
 * @param session
 * @param callback callback to be called when the request token has been requested
 * @return TRUE on success
 */
bool twitter_session_oauth_request_token(twitter_session* session, OAuthTokenCallback callback, void* userdata)
{
    session->request_token_cb = callback;
    session->request_token_data = userdata;

    oauth_proxy(session);
    return oauth_proxy_request_token_async(OAUTH_PROXY(session->oauth_proxy),
        TWITTER_REQUEST_TOKEN_FUNC, "oob", _request_token, NULL, session, NULL);
}

/**
 * Sets an access token for this session.
 * @param session
 * @param access
 */
void twitter_session_set_access_token(twitter_session* session, oauth_token* access)
{
    oauth_token_copy(&session->access, access);
}

/**
 * Creates a new Twitter session.
 * @param consumer API consumer token
 * @return a new session ready for use
 */
twitter_session* twitter_session_new(oauth_token* consumer)
{
    twitter_session* s = calloc(1, sizeof(twitter_session));
    s->consumer.key = strdup(consumer->key);
    s->consumer.secret = strdup(consumer->secret);

    return s;
}

void twitter_init(RemoteConfigService* config)
{
    // initialize log
    _micromoko_twitter_utils_log_dom = eina_log_domain_register(MICROMOKO_TWITTER_LOG_NAME, MICROMOKO_TWITTER_LOG_COLOR);
    if (_micromoko_twitter_utils_log_dom < 0)
        printf("Cannot create log domain.\n");

    eina_log_domain_level_set(MICROMOKO_TWITTER_LOG_NAME, TWITTER_LOG_LEVEL);
    INFO("%s Twitter library version %s", PACKAGE_NAME, VERSION);
}
