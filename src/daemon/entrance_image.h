#ifndef ENTRANCE_IMAGE_H_
#define ENTRANCE_IMAGE_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#include "entrance.h"

extern const char *extn_images[];

Eina_List* entrance_image_system_icons();
Eina_List* entrance_image_system_backgrounds();
Eina_List* entrance_image_user_icons(const char *username);
Eina_List* entrance_image_user_backgrounds(const char *username);


#endif /* ENTRANCE_H_ */
