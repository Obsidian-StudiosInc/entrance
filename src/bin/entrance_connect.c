#include <Ecore_Con.h>
#include "entrance_client.h"

Ecore_Con_Server *_entrance_connect;
Eina_List *_handlers;

static Eina_Bool _entrance_connect_add(void *data, int type, void *event);
static Eina_Bool _entrance_connect_del(void *data, int type, void *event);
static Eina_Bool _entrance_connect_data(void *data, int type, void *event);



static Eina_Bool
_entrance_connect_add(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Con_Event_Server_Add *ev;
   ev = event;
   fprintf(stderr, PACKAGE": client connected\n");

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_entrance_connect_del(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Con_Event_Server_Del *ev;
   ev = event;
   fprintf(stderr, PACKAGE": client disconnected\n");
   _entrance_connect = NULL;

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_entrance_connect_data(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Con_Event_Server_Data *ev;
   Entrance_Event *eev;
   ev = event;

   eev = entrance_event_decode(ev->data, ev->size);
   if (eev)
     {
        if (eev->type == ENTRANCE_EVENT_STATUS)
          {
             if (eev->event.status.granted)
               entrance_gui_auth_valid();
             else
               entrance_gui_auth_error();
          }
        else if (eev->type == ENTRANCE_EVENT_MAXTRIES)
          entrance_gui_auth_wait();
        else if (eev->type == ENTRANCE_EVENT_XSESSIONS)
          entrance_gui_xsession_set(eev->event.xsessions.xsessions);
        else if (eev->type == ENTRANCE_EVENT_USERS)
          entrance_gui_users_set(eev->event.users.users);
        else if (eev->type == ENTRANCE_EVENT_ACTIONS)
          entrance_gui_actions_set(eev->event.actions.actions);
        else
          fprintf(stderr, PACKAGE": unknow signal\n");
     }
   return ECORE_CALLBACK_RENEW;
}

void
entrance_connect_auth_send(const char *login, const char *password, const char *session)
{
   Entrance_Event eev;
   void *data;
   int size;

   eev.event.auth.login = login;
   eev.event.auth.password = password;
   eev.event.auth.session = session;
   eev.type = ENTRANCE_EVENT_AUTH;
   data = entrance_event_encode(&eev, &size);
   ecore_con_server_send(_entrance_connect, data, size);
}

void
entrance_connect_action_send(int id)
{
   Entrance_Event eev;
   void *data;
   int size;

   eev.event.action.action = id;
   eev.type = ENTRANCE_EVENT_ACTION;
   data = entrance_event_encode(&eev, &size);
   ecore_con_server_send(_entrance_connect, data, size);
}

void
entrance_connect_init()
{
   Ecore_Event_Handler *h;
   ecore_con_init();
   _entrance_connect = ecore_con_server_connect(ECORE_CON_LOCAL_SYSTEM,
                                            "entrance", 42, NULL);
   if (_entrance_connect)
     printf(PACKAGE": client server init ok\n");
   else
     printf(PACKAGE": client server init fail\n");
   h = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD,
                               _entrance_connect_add, NULL);
   _handlers = eina_list_append(_handlers, h);
   h = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL,
                               _entrance_connect_del, NULL);
   _handlers = eina_list_append(_handlers, h);
   h = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA,
                               _entrance_connect_data, NULL);
   _handlers = eina_list_append(_handlers, h);
}

void
entrance_connect_shutdown()
{
   Ecore_Event_Handler *h;
   printf(PACKAGE": client server shutdown\n");
   EINA_LIST_FREE(_handlers, h)
      ecore_event_handler_del(h);
   ecore_con_shutdown();
}

