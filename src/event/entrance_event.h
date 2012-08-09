#ifndef ENTRANCE_EVENT_
#define ENTRANCE_EVENT_

typedef enum Entrance_Event_Type_
{
   ENTRANCE_EVENT_UNKNOWN,
   ENTRANCE_EVENT_AUTH,
   ENTRANCE_EVENT_STATUS,
   ENTRANCE_EVENT_XSESSIONS,
   ENTRANCE_EVENT_USERS,
   ENTRANCE_EVENT_ACTIONS,
   ENTRANCE_EVENT_ACTION,
   ENTRANCE_EVENT_MAXTRIES
} Entrance_Event_Type;

typedef struct Entrance_Xsession_
{
    const char *name;
    const char *command;
    const char *icon;
} Entrance_Xsession;

typedef struct Entrance_Xsessions_Event_
{
   Eina_List *xsessions;
} Entrance_Xsessions_Event;

typedef struct Entrance_Auth_Event_
{
   const char *login;
   const char *password;
   const char *session;
} Entrance_Auth_Event;

typedef struct Entrance_Maxtries_Event_
{
   Eina_Bool maxtries;
} Entrance_Maxtries_Event;

typedef struct Entrance_Status_Event_
{
   Eina_Bool granted;
} Entrance_Status_Event;

typedef struct Entrance_Action_Event_
{
   int action;
} Entrance_Action_Event;

typedef struct Entrance_User_
{
   const char *login;
   const char *image;
   const char *lsess;
} Entrance_User;


typedef struct Entrance_Users_Event_
{
   Eina_List *users;
} Entrance_Users_Event;

typedef struct Entrance_Action_
{
   int id;
   const char *label;
} Entrance_Action;

typedef struct Entrance_Actions_Event_
{
   Eina_List *actions;
} Entrance_Actions_Event;

typedef struct Entrance_Event_
{
   Entrance_Event_Type type;
   union
     {
        Entrance_Xsessions_Event xsessions;
        Entrance_Auth_Event auth;
        Entrance_Maxtries_Event maxtries;
        Entrance_Status_Event status;
        Entrance_Users_Event users;
        Entrance_Actions_Event actions;
        Entrance_Action_Event action;
     } event;
} Entrance_Event;

void *entrance_event_encode(Entrance_Event *ev, int *size);
Entrance_Event *entrance_event_decode(void *data, int size);
#endif /* ENTRANCE_EVENT_ */
