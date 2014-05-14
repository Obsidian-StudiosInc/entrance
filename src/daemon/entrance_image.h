#ifndef ENTRANCE_IMAGE_H_
#define ENTRANCE_IMAGE_H_

Eina_List* entrance_image_system_icons();
Eina_List* entrance_image_system_backgrounds();
Eina_List* entrance_image_user_icons(const char *username);
Eina_List* entrance_image_user_backgrounds(const char *username);
Eina_List* entrance_image_user_pool_get(Eina_List *users);
void entrance_image_user_pool_free(Eina_List *user_pool);

#endif /* ENTRANCE_H_ */
