#ifndef ENTRANCE_GUI_H_
#define ENTRANCE_GUI_H_

#define ENTRANCE_GUI_GET(edj, name) edje_object_part_external_object_get(elm_layout_edje_get(edj), name)

int entrance_gui_init(const char *theme);
void entrance_gui_shutdown(void);
void entrance_gui_run(void);
Evas_Object *entrance_gui_theme_get (Evas_Object *win, const char *group);
void entrance_gui_auth_valid(void);
void entrance_gui_auth_error(void);
void entrance_gui_auth_max_tries(void);
void entrance_gui_xsession_set(Eina_List *xsessions);
void entrance_gui_actions_set(Eina_List *actions);
void entrance_gui_users_set(Eina_List *users);
const Eina_List *entrance_gui_users_get(void);
void entrance_gui_xsessions_set(Eina_List *users);
const Eina_List *entrance_gui_xsessions_get(void);
void entrance_gui_conf_set(const Entrance_Conf_Gui_Event *conf);
Eina_List *entrance_gui_stringlist_get(const char *name);
void entrance_gui_stringlist_free(Eina_List *list);
const char *entrance_gui_theme_name_get(void);
void entrance_gui_theme_name_set(const char *theme);
void entrance_gui_background_get(const char **path, const char **group);
Eina_Bool entrance_gui_vkbd_enabled_get(void);
const char *entrance_gui_theme_path_get(void);
void entrance_gui_user_bg_set(const char *path, const char *group);

/*
char *entrance_gui_user_get();
char *entrance_gui_password_get();
char *entrance_gui_login_command_get();
*/

#endif /* ENTRANCE_GUI_H_ */
