/*
 * Micromoko Twitter library
 * Private definitions
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

#ifndef __MICROMOKO_TWITTER_PRIVATE_H
#define __MICROMOKO_TWITTER_PRIVATE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Eina.h>

#ifdef DEBUG
#define TWITTER_LOG_LEVEL   EINA_LOG_LEVEL_DBG
#else
#define TWITTER_LOG_LEVEL   EINA_LOG_LEVEL_INFO
#endif

#define MICROMOKO_TWITTER_LOG_NAME        "micromoko_twitter"

extern int _micromoko_twitter_utils_log_dom;
#ifdef MICROMOKO_TWITTER_LOG_COLOR
#undef MICROMOKO_TWITTER_LOG_COLOR
#endif
#define MICROMOKO_TWITTER_LOG_COLOR   EINA_COLOR_BLUE

#define MICROMOKO_TWITTER_LOG_DOM _micromoko_twitter_utils_log_dom

#ifdef ERROR
#undef ERROR
#endif
#define ERROR(...) EINA_LOG_DOM_ERR(MICROMOKO_TWITTER_LOG_DOM, __VA_ARGS__)
#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG(...) EINA_LOG_DOM_DBG(MICROMOKO_TWITTER_LOG_DOM, __VA_ARGS__)
#ifdef INFO
#undef INFO
#endif
#define INFO(...) EINA_LOG_DOM_INFO(MICROMOKO_TWITTER_LOG_DOM, __VA_ARGS__)
#ifdef WARN
#undef WARN
#endif
#define WARN(...) EINA_LOG_DOM_WARN(MICROMOKO_TWITTER_LOG_DOM, __VA_ARGS__)


#define DEFAULT_CALL_TYPE       "xml"

typedef struct _callback_pack
{
    void* callback;
    void* userdata;
} callback_pack;

#endif  /* __MICROMOKO_TWITTER_PRIVATE_H */
