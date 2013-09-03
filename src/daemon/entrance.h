#ifndef ENTRANCE_H_
#define ENTRANCE_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#include <Eina.h>
#include <Eet.h>
#include <Ecore.h>
#include <Ecore_File.h>
#include <Efreet.h>

#include "entrance_session.h"
#ifdef HAVE_PAM
#include "entrance_pam.h"
#endif
#include "../event/entrance_event.h"
#include "entrance_config.h"
#include "entrance_xserver.h"
#include "entrance_server.h"
#include "entrance_history.h"
#include "entrance_action.h"

#define PT(f, x...)                                                     \
do                                                                   \
{                                                                    \
   current_time = time(NULL);                                        \
   local_time = localtime(&current_time);                            \
   memset(entrance_time_d, 0, sizeof(entrance_time_d));              \
   strftime(entrance_time_d, sizeof(entrance_time_d),                \
            "%b %_2d %T", local_time);                               \
   fprintf(stderr, "(%s) "PACKAGE": "f, entrance_time_d, ##x); \
} while (0)

extern   time_t current_time;
extern   struct tm *local_time;
extern   char entrance_time_d[4096];

void entrance_close_log();

#endif /* ENTRANCE_H_ */
