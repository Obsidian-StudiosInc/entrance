#ifndef ENTRANCE_HISTORY_
#define ENTRANCE_HISTORY_

void entrance_history_init();
void entrance_history_shutdown();
void entrance_history_push(const char *login, const char *session);
Eina_List *entrance_history_get();

typedef struct _Entrance_Login Entrance_Login;
typedef struct _Entrance_History Entrance_History;
typedef struct _Entrance_Background Entrance_Background;

struct _Entrance_Background
{
   const char *group;
   const char *path;
};

struct _Entrance_Login
{
   const char *login;
   const char *session;
   struct
     {
        const char *path;
        const char *group;
     } icon;
   Entrance_Background background;
   Eina_Bool ignore_last_session;
};

struct _Entrance_History
{
   Eina_List *history;
};


#endif /* ENTRANCE_HISTORY_ */
