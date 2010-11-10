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

#include "twitter.h"
#include "private.h"

// TEST
#include "oauth/oauth.h"
#include "libsoup/soup.h"
#include <readline/readline.h>
#include <readline/history.h>

// default log domain
int _micromoko_twitter_utils_log_dom = -1;

// global
RemoteConfigService* config = NULL;

// TEST
const char *req_c_key         = "dY49wvIY7386ET15vCRVQ"; //< consumer key
const char *req_c_secret      = "D8wKnvbTQqxJmxBC0JyHVY6LpHdpTBbJwyISlrUg8"; //< consumer secret

static void _access_token(SoupSession *session, SoupMessage *msg, gpointer user_data)
{
    DEBUG("HTTP response %d %s", msg->status_code, msg->reason_phrase);
    SoupMessageBody* body = msg->response_body;
    DEBUG("Response data: %s", body->data);
}

static void _request_token(SoupSession *session, SoupMessage *msg, gpointer user_data)
{
    const char *authorize_uri     = "https://api.twitter.com/oauth/authorize";
    const char *access_token_uri  = "https://api.twitter.com/oauth/access_token";

    DEBUG("HTTP response %d %s", msg->status_code, msg->reason_phrase);
    SoupMessageBody* body = msg->response_body;
    DEBUG("Response data: %s", body->data);

    int rc = 0;
    char** rv = NULL;

    rc = oauth_split_url_parameters(body->data, &rv);
    char* key = strdup(get_param(rv, rc, "oauth_token"));
    char* secret = strdup(get_param(rv, rc, "oauth_token_secret"));
    free_array(&rc, &rv);

    DEBUG("\toauth_token=%s", key);
    DEBUG("\toauth_token_secret=%s", secret);
    INFO("Goto URL:\n\t%s?oauth_token=%s", authorize_uri, key);

    char* postarg = NULL;
    char* pin = readline("PIN: ");

    char* uri = g_strdup_printf("%s?oauth_verifier=%s", access_token_uri, pin);
    char* req_url = oauth_sign_url2(uri, &postarg, OA_HMAC, NULL, req_c_key, req_c_secret, key, secret);
    g_free(uri);

    DEBUG("request URL: %s", req_url);
    DEBUG("post args: %s", postarg);

    msg = soup_message_new("POST", req_url);
    soup_message_set_request(msg, "application/x-www-form-urlencoded", SOUP_MEMORY_COPY, postarg, strlen(postarg));
    soup_session_queue_message(session, msg, _access_token, NULL);

    free(req_url);
    free(postarg);
    free(pin);
}
// TEST

void twitter_init(RemoteConfigService* config)
{
    // initialize log
    _micromoko_twitter_utils_log_dom = eina_log_domain_register(MICROMOKO_TWITTER_LOG_NAME, MICROMOKO_TWITTER_LOG_COLOR);
    if (_micromoko_twitter_utils_log_dom < 0)
        printf("Cannot create log domain.\n");

    eina_log_domain_level_set(MICROMOKO_TWITTER_LOG_NAME, TWITTER_LOG_LEVEL);
    INFO("%s Twitter library version %s", PACKAGE_NAME, VERSION);

    // TEST
    const char *request_token_uri = "https://api.twitter.com/oauth/request_token";

    char *postarg = NULL;
    char *req_url;

    req_url = oauth_sign_url2(request_token_uri, &postarg, OA_HMAC, NULL, req_c_key, req_c_secret, NULL, NULL);

    DEBUG("request URL: %s", req_url);
    DEBUG("post args: %s", postarg);

    // prova una richiesta async
    SoupSession* session = soup_session_async_new();

    SoupMessage *msg;
    msg = soup_message_new("POST", req_url);
    soup_message_set_request(msg, "application/x-www-form-urlencoded", SOUP_MEMORY_COPY, postarg, strlen(postarg));
    soup_session_queue_message(session, msg, _request_token, NULL);

    free(postarg);
    free(req_url);
    // TEST
}
