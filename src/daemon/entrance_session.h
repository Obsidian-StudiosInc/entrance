#ifndef ENTRANCE_SESSION_H_
#define ENTRANCE_SESSION_H_
#include <pwd.h>

void entrance_session_init(const char *file);
void entrance_session_end(const char *login);
void entrance_session_shutdown();
Eina_Bool entrance_session_authenticate(const char *login, const char *pwd);
Eina_Bool entrance_session_login(const char *command, Eina_Bool push);
void entrance_session_pid_set(pid_t pid);
pid_t entrance_session_pid_get();
long  entrance_session_seed_get();
char *entrance_session_login_get();
int entrance_session_logged_get();
Eina_List *entrance_session_list_get();

#endif /* ENTRANCE_SESSION_H_ */
