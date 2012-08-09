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

#include "entrance_session.h"
#ifdef HAVE_PAM
#include "entrance_pam.h"
#endif
#include "entrance_config.h"
#include "entrance_xserver.h"
#include "entrance_server.h"
#include "entrance_history.h"
#include "entrance_action.h"
#include "../event/entrance_event.h"

void entrance_close_log();

#endif /* ENTRANCE_H_ */
