#ifndef ENTRANCE_H_
#define ENTRANCE_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#include <Elementary.h>

#include "entrance_gui.h"
#include "entrance_connect.h"
#include "entrance_client.h"
#include "entrance_gui.h"
#include "entrance_fill.h"
#include "../event/entrance_event.h"

#define PT(x)                                                        \
do                                                                   \
{                                                                    \
   current_time = time(NULL);                                        \
   local_time = localtime(&current_time);                            \
   memset(entrance_time_d, 0, sizeof(entrance_time_d));              \
   strftime(entrance_time_d, sizeof(entrance_time_d),                \
            "%b %_2d %T", local_time);                               \
   fprintf(stderr, "(%s) "PACKAGE"_client: %s", entrance_time_d, x); \
} while (0)

extern   time_t current_time;
extern   struct tm *local_time;
extern   char entrance_time_d[4096];

int entrance_client_main();

#endif /* ENTRANCE_H_ */
