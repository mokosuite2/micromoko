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

#ifndef __MICROMOKO_TWITTER_H
#define __MICROMOKO_TWITTER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libintl.h>
#include <stdio.h>
#include <stdlib.h>
#include <mokosuite/utils/utils.h>
#include <mokosuite/utils/remote-config-service.h>
#include <rest/rest-proxy.h>

// default log domain
#undef EINA_LOG_DOMAIN_DEFAULT
#define EINA_LOG_DOMAIN_DEFAULT _log_dom
extern int _log_dom;

#ifdef DEBUG
#define LOG_LEVEL   EINA_LOG_LEVEL_DBG
#else
#define LOG_LEVEL   EINA_LOG_LEVEL_INFO
#endif

#define TWITTER_BASE_URI            "https://api.twitter.com/"
#define TWITTER_REQUEST_TOKEN_FUNC  "oauth/request_token"
#define TWITTER_ACCESS_TOKEN_FUNC   "oauth/access_token"
#define TWITTER_AUTHORIZE_FUNC      "oauth/authorize"


typedef struct _oauth_token
{
    char* key;      // oauth_token
    char* secret;   // oauth_token_secret
} oauth_token;

typedef struct _twitter_session
{
    oauth_token consumer;   // for consuming APIs
    oauth_token request;    // for first-time authentication
    oauth_token access;     // for accessing APIs

    char* username;

    /* private */
    RestProxy* oauth_proxy;

    void* request_token_cb;
    void* request_token_data;
    void* access_token_cb;
    void* access_token_data;
} twitter_session;

typedef struct _twitter_call
{
    twitter_session* session;

    char* function;
    char* method;
    // TODO char** args;

    /* private */
    RestProxyCall* call_proxy;
    void* callback;
    void* userdata;
} twitter_call;

// api call callback
typedef void (*TwitterCallResponseCallback)(twitter_session* session, twitter_call* call, const char* payload, void* userdata);

twitter_call* twitter_session_call_new(twitter_session* session,
            const char* function, const char* method,
            TwitterCallResponseCallback callback, void* userdata, ...);

// request token callback
typedef void (*OAuthTokenCallback)(twitter_session* session, void* userdata);

bool twitter_session_oauth_access_token(twitter_session* session, const char* pin, OAuthTokenCallback callback, void* userdata);
bool twitter_session_oauth_request_token(twitter_session* session, OAuthTokenCallback callback, void* userdata);

void twitter_session_set_access_token(twitter_session* session, oauth_token* access);

void twitter_session_destroy(twitter_session* session);
twitter_session* twitter_session_new(oauth_token* consumer);

void twitter_init(RemoteConfigService* config);


#endif  /* __MICROMOKO_TWITTER_H */
