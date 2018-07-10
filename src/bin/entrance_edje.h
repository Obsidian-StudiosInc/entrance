/*
 * This file is part of entrance.
 *
 * entrance is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * entrance is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with entrance.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * File:   entrance_edje.h
 * Author: William L. Thomson Jr. <wlt@o-sinc.com>
 *         Copyright 2017 Obsidian-Studios, Inc.
 *
 * EDJE names for mapping between edc and c files
 *
 */

#ifndef ENTRANCE_EDJE_H
#define ENTRANCE_EDJE_H


#define ENTRANCE_EDJE_GROUP_ENTRANCE            "entrance"
#define ENTRANCE_EDJE_GROUP_LOGIN               "entrance/login"
#define ENTRANCE_EDJE_GROUP_USER                "entrance/user"
#define ENTRANCE_EDJE_GROUP_WALLPAPER           "entrance/wallpaper/default"

#define ENTRANCE_EDJE_PART_ACTIONS              "entrance.actions"
#define ENTRANCE_EDJE_PART_CLOCK                "entrance.clock"
#define ENTRANCE_EDJE_PART_CONF                 "entrance.conf"
#define ENTRANCE_EDJE_PART_DATE                 "entrance.date"
#define ENTRANCE_EDJE_PART_ICON                 "entrance.icon"
#define ENTRANCE_EDJE_PART_LABEL                "entrance.label"
#define ENTRANCE_EDJE_PART_LOGIN                "entrance.login"
#define ENTRANCE_EDJE_PART_LOGIN_LABEL          "entrance.login_label"
#define ENTRANCE_EDJE_PART_PASSWORD             "entrance.password"
#define ENTRANCE_EDJE_PART_PASSWORD_LABEL       "entrance.password_label"
#define ENTRANCE_EDJE_PART_SCREEN               "entrance.screen"
#define ENTRANCE_EDJE_PART_UNAME                "entrance.uname"
#define ENTRANCE_EDJE_PART_USERS                "entrance.users"
#define ENTRANCE_EDJE_PART_WALLPAPER            "entrance.wallpaper.default"
#define ENTRANCE_EDJE_PART_XSESSIONS            "entrance.xsessions"

#define ENTRANCE_EDJE_SIGNAL_ACTION_ENABLED     "entrance,action,enabled"
#define ENTRANCE_EDJE_SIGNAL_AUTH_CHANGED       "entrance,auth,changed"
#define ENTRANCE_EDJE_SIGNAL_AUTH_CHECK         "entrance,auth,check"
#define ENTRANCE_EDJE_SIGNAL_AUTH_CHECKING      "entrance,auth,checking"
#define ENTRANCE_EDJE_SIGNAL_AUTH_ERROR         "entrance,auth,error"
#define ENTRANCE_EDJE_SIGNAL_AUTH_VALID         "entrance,auth,valid"
#define ENTRANCE_EDJE_SIGNAL_CONF_ENABLED       "entrance,conf,enabled"
#define ENTRANCE_EDJE_SIGNAL_CONF_DISABLED      "entrance,conf,disabled"
#define ENTRANCE_EDJE_SIGNAL_USERS_ENABLED      "entrance,users,enabled"
#define ENTRANCE_EDJE_SIGNAL_USERS_DISABLED     "entrance,users,disabled"
#define ENTRANCE_EDJE_SIGNAL_WALLPAPER_DEFAULT  "entrance,wallpaper,default"


#endif /* ENTRANCE_EDJE_H */

