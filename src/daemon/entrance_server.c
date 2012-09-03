#include <Ecore_Con.h>
#include "entrance.h"
#include "../event/entrance_event.h"

static Eina_Bool _entrance_server_add(void *data, int type, void *event);
static Eina_Bool _entrance_server_del(void *data, int type, void *event);
static Eina_Bool _entrance_server_data(void *data, int type, void *event);


Ecore_Con_Server *_entrance_server = NULL;
Eina_List *_handlers = NULL;


static Eina_Bool
_my_hack2(void *data)
{
   Entrance_Event eev;
   void *enc;
   int size;
   eev.type = ENTRANCE_EVENT_ACTIONS;
   eev.event.actions.actions = entrance_action_get();
   enc = entrance_event_encode(&eev, &size);
   ecore_con_client_send(data, enc, size);
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_my_hack(void *data)
{
   Entrance_Event eev;
   void *enc;
   int size;

   eev.type = ENTRANCE_EVENT_XSESSIONS;
   if (entrance_config->xsessions)
     {
        eev.event.xsessions.xsessions = entrance_session_list_get();
        enc = entrance_event_encode(&eev, &size);
        ecore_con_client_send(data, enc, size);
     }
   ecore_timer_add(0.5, _my_hack2, data);
   return ECORE_CALLBACK_CANCEL;
}


static Eina_Bool
_entrance_server_add(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Con_Event_Client_Add *ev;
   Entrance_Event eeu;
   ev = event;
   void *enc;
   int size;

   fprintf(stderr, PACKAGE": server client connected\n");
   eeu.type = ENTRANCE_EVENT_USERS;
   eeu.event.users.users = entrance_history_get();
   enc = entrance_event_encode(&eeu, &size);
   ecore_con_client_send(ev->client, enc, size);
//   ecore_con_client_flush(ev->client);
   ecore_timer_add(0.5, _my_hack, ev->client);
   /*
   eev.type = ENTRANCE_EVENT_XSESSIONS;
   if (entrance_config->xsessions)
     {
        eev.event.xsessions.xsessions = entrance_session_list_get();
        enc = entrance_event_encode(&eev, &size);
        ecore_con_client_send(data, enc, size);
     }
     */
   return ECORE_CALLBACK_RENEW;
}


static Eina_Bool
_entrance_server_del(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Con_Event_Client_Del *ev;
   ev = event;
   fprintf(stderr, PACKAGE": server client disconnected\n");

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_entrance_server_data(void *data __UNUSED__, int type __UNUSED__, void *event)
{

   Ecore_Con_Event_Client_Data *ev;
   Entrance_Event *eev, neev;
   int size;
   void *enc;

   ev = event;
   eev = entrance_event_decode(ev->data, ev->size);
   if (eev->type == ENTRANCE_EVENT_AUTH)
     {
        neev.type = ENTRANCE_EVENT_STATUS;
        if (entrance_session_authenticate(eev->event.auth.login,
                                      eev->event.auth.password))
          {
             entrance_session_login(eev->event.auth.session, EINA_TRUE);
             neev.event.status.granted = EINA_TRUE;
             fprintf(stderr, PACKAGE": server authenticate granted\n");
          }
        else
          {
             neev.event.status.granted = EINA_FALSE;
             fprintf(stderr, PACKAGE": server authenticate error\n");
          }
        enc = entrance_event_encode(&neev, &size);
        ecore_con_client_send(ev->client, enc, size);
     }
   else if (eev->type == ENTRANCE_EVENT_ACTION)
     entrance_action_run(eev->event.action.action);
   else
     fprintf(stderr, PACKAGE": UNKNOW signal server\n");

   return ECORE_CALLBACK_RENEW;
}

void
entrance_server_init()
{
   Ecore_Event_Handler *h;
   ecore_con_init();
   _entrance_server = ecore_con_server_add(ECORE_CON_LOCAL_SYSTEM,
                                        "entrance", 42, NULL);
   if (!_entrance_server)
     fprintf(stderr, PACKAGE": server init fail\n");

   h = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD,
                               _entrance_server_add, NULL);
   _handlers = eina_list_append(_handlers, h);
   h = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL,
                               _entrance_server_del, NULL);
   _handlers = eina_list_append(_handlers, h);
   h = ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA,
                               _entrance_server_data, NULL);
   _handlers = eina_list_append(_handlers, h);
}

void
entrance_server_shutdown()
{
   Ecore_Event_Handler *h;
   if (_entrance_server)
     ecore_con_server_del(_entrance_server);
   EINA_LIST_FREE(_handlers, h)
     ecore_event_handler_del(h);
   ecore_con_shutdown();
}

void
entrance_server_client_wait()
{
   const Eina_List *l;
   Entrance_Event eev;
   Ecore_Con_Client *ecc;
   void *enc;
   int size;
   eev.type = ENTRANCE_EVENT_MAXTRIES;
   eev.event.maxtries.maxtries = EINA_TRUE;
   enc = entrance_event_encode(&eev, &size);
   EINA_LIST_FOREACH(ecore_con_server_clients_get(_entrance_server), l, ecc)
      ecore_con_client_send(ecc, enc, size);
}

