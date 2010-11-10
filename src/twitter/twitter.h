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
#include <mokosuite/utils/remote-config-service.h>

// default log domain
#undef EINA_LOG_DOMAIN_DEFAULT
#define EINA_LOG_DOMAIN_DEFAULT _log_dom
extern int _log_dom;

#ifdef DEBUG
#define LOG_LEVEL   EINA_LOG_LEVEL_DBG
#else
#define LOG_LEVEL   EINA_LOG_LEVEL_INFO
#endif


void twitter_init(RemoteConfigService* config);


#endif  /* __MICROMOKO_TWITTER_H */
