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
#include <glib.h>
#include <rest/rest-proxy.h>
#include <rest/oauth-proxy.h>
#include <rest/rest-xml-parser.h>
#include <stdarg.h>

#include "twitter.h"
#include "private.h"

// default log domain
int _micromoko_twitter_utils_log_dom = -1;

// global
RemoteConfigService* config = NULL;

static void _status_update(twitter_session* session, twitter_call* call, const char* payload, goffset length, void* userdata)
{
    callback_pack* data = userdata;

    DEBUG("got status update response! (%llu bytes)", length);

    twitter_status* e = NULL;
    RestXmlParser* parse = rest_xml_parser_new();
    RestXmlNode* root = rest_xml_parser_parse_from_data(parse, payload, length);
    if (!root || strcmp(root->name, "status")) {
        DEBUG("%s", payload);
        WARN("no status data!!");
    }
    else {
        e = twitter_parse_status(root);
    }

    // user callback
    TwitterStatusUpdateCallback cb = data->callback;
    if (cb)
        (cb)(session, call, e, data->userdata);
    free(data);
}

/**
 * Update the status.
 * @param session
 * @param status
 * @param reply_to_id
 * @param callback
 * @param userdata
 * @return the call on success
 */
twitter_call* twitter_update_status(twitter_session* session, const char* text, const char* reply_to_id, TwitterStatusUpdateCallback callback, void* userdata)
{
    callback_pack* data = malloc(sizeof(callback_pack));
    data->callback = callback;
    data->userdata = userdata;

    return twitter_session_call_new(session, "statuses/update", "POST", _status_update, data, "status", text, "in_reply_to_status_id", reply_to_id, NULL);
}


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

static void oauth_proxy(twitter_session* session, const char* key, const char* secret)
{
    if (!session->oauth_proxy) {
        if (key != NULL && secret != NULL)
            session->oauth_proxy = oauth_proxy_new_with_token(session->consumer.key, session->consumer.secret, key, secret, TWITTER_BASE_URI, FALSE);
        else
            session->oauth_proxy = oauth_proxy_new(session->consumer.key, session->consumer.secret, TWITTER_BASE_URI, FALSE);
    }
}

static RestProxyCall* call_proxy(twitter_session* session, const char* function, const char* method)
{
    char* req = g_strdup_printf("%s.%s", function, DEFAULT_CALL_TYPE);
    DEBUG("creating call for request \"%s\" (oauth_proxy=%p)", req, session->oauth_proxy);
    RestProxyCall* call = rest_proxy_new_call(session->oauth_proxy);
    rest_proxy_call_set_function(call, req);

    rest_proxy_call_set_method(call, method);

    g_free(req);
    return call;
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

static void _call_response(RestProxyCall *call_proxy, GError* error, GObject* weak_object, void* userdata)
{
    twitter_call* call = userdata;

    const char* payload = rest_proxy_call_get_payload(call->call_proxy);
    goffset len = rest_proxy_call_get_payload_length(call->call_proxy);
    //DEBUG("response:\n%s", payload);

    TwitterCallCallback cb = call->callback;
    if (cb)
        (cb)(call->session, call, payload, len, call->userdata);

    // destroy call
    twitter_call_destroy(call);
}

/**
 * Destroys a Twitter command call.
 * @param call
 */
void twitter_call_destroy(twitter_call* call)
{
    free(call->function);
    free(call->method);
    g_object_unref(call->call_proxy);
    free(call);
}

/**
 * Creates a new Twitter command request and immediately starts it.
 * @param session
 * @param function
 * @param method
 * @param ... arguments
 * @return the new call
 */
twitter_call* twitter_session_call_new(twitter_session* session,
            const char* function, const char* method,
            TwitterCallCallback callback, void* userdata, ...)
{
    twitter_call* call = calloc(1, sizeof(twitter_call));
    va_list ap;
    int i = 0;
    const char* arg;
    const char* arg_name = NULL;

    call->session = session;
    call->function = strdup(function);
    call->method = strdup(method);
    call->callback = callback;
    call->userdata = userdata;
    call->call_proxy = call_proxy(session, function, method);

    va_start(ap, userdata);
    while ((arg = va_arg(ap, const char*))) {
        if (i % 2) {
            DEBUG("argument: name=\"%s\", value=\"%s\"", arg_name, arg);
            rest_proxy_call_add_param(call->call_proxy, arg_name, arg);
        }
        else
            arg_name = arg;

        i++;
    }
    va_end(ap);

    rest_proxy_call_async (call->call_proxy, _call_response, NULL, call, NULL);
    return call;
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

    oauth_proxy(session, NULL, NULL);
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

    oauth_proxy(session, NULL, NULL);
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
    if (session->oauth_proxy) {
        oauth_proxy_set_token(OAUTH_PROXY(session->oauth_proxy), access->key);
        oauth_proxy_set_token_secret(OAUTH_PROXY(session->oauth_proxy), access->secret);
    }
    else {
        oauth_proxy(session, access->key, access->secret);
    }
}

void twitter_session_destroy(twitter_session* session)
{
    free(session->username);
    oauth_token_free_keys(&session->consumer);
    oauth_token_free_keys(&session->request);
    oauth_token_free_keys(&session->access);

    g_object_unref(session->oauth_proxy);
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
