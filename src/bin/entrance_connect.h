#ifndef ENTRANCE_CONNECT_
#define ENTRANCE_CONNECT_
void entrance_connect_init();
void entrance_connect_auth_send(const char *login, const char *password, const char *session);
void entrance_connect_action_send(int id);
void entrance_connect_shutdown();
#endif /* ENTRANCE_CONNECT_ */
