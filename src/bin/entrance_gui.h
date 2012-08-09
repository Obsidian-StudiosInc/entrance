#ifndef ENTRANCE_GUI_H_
#define ENTRANCE_GUI_H_

int entrance_gui_init(const char *theme);
void entrance_gui_run();
void entrance_gui_shutdown();
char *entrance_gui_user_get();
char *entrance_gui_password_get();
void entrance_gui_auth_error();
void entrance_gui_auth_valid();
void entrance_gui_auth_wait();
char *entrance_gui_login_command_get();
void entrance_gui_xsession_set(Eina_List *xsessions);
void entrance_gui_users_set(Eina_List *users);
void entrance_gui_actions_set(Eina_List *actions);

#endif /* ENTRANCE_GUI_H_ */
