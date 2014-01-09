#include <Ecore_Con.h>
#include "entrance.h"
#include "../event/entrance_event.h"

static Eina_Bool _entrance_server_add(void *data, int type, void *event);
static Eina_Bool _entrance_server_del(void *data, int type, void *event);
static Eina_Bool _entrance_server_data(void *data, int type, void *event);


Ecore_Con_Server *_entrance_server = NULL;
Eina_List *_handlers = NULL;

static Eina_Bool
_entrance_server_add(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   Entrance_Event eev;

   PT("server client connected\n");
   PT("Sending users\n");
   eev.type = ENTRANCE_EVENT_USERS;
   eev.event.users.users = entrance_history_get();
   entrance_event_send(&eev);

   PT("Sending actions\n");
   eev.type = ENTRANCE_EVENT_ACTIONS;
   eev.event.actions.actions = entrance_action_get();
   entrance_event_send(&eev);
   if (entrance_config->xsessions != ENTRANCE_SESSION_DESKTOP_NONE)
     {
        PT("Sending xsessions\n");
        eev.type = ENTRANCE_EVENT_XSESSIONS;
        eev.event.xsessions.xsessions = entrance_session_list_get();
        entrance_event_send(&eev);
     }
   if (entrance_config->custom_conf)
     {
        PT("Sending custom settings is enabled\n");
        eev.type = ENTRANCE_EVENT_CONF_GUI;
        eev.event.conf_gui.enabled = EINA_TRUE;
        eev.event.conf_gui.bg.path = entrance_config->bg.path;
        eev.event.conf_gui.bg.group = entrance_config->bg.group;
        entrance_event_send(&eev);
     }
   return ECORE_CALLBACK_RENEW;
}


static Eina_Bool
_entrance_server_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   PT("server client disconnected\n");

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_entrance_server_data(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{

   Ecore_Con_Event_Client_Data *ev;

   ev = event;
   entrance_event_received(ev->data, ev->size);

   return ECORE_CALLBACK_RENEW;
}


static Eina_Bool
_entrance_server_read_cb(const void *data, size_t size EINA_UNUSED, void *user_data EINA_UNUSED)
{
   const Entrance_Event *eev;
   Entrance_Event neev;
   eev = data;
   if (eev->type == ENTRANCE_EVENT_AUTH)
     {
        neev.type = ENTRANCE_EVENT_STATUS;
        if (entrance_session_authenticate(eev->event.auth.login,
                                          eev->event.auth.password))
          {
             PT("server authenticate granted\n");
             neev.event.status.login = entrance_session_login_get();
             neev.event.status.granted = EINA_TRUE;
             if (eev->event.auth.open_session)
               {
                  PT("opening session now ...\n");
                  entrance_session_login(eev->event.auth.session, EINA_TRUE);
               }
             else
               entrance_session_close(EINA_FALSE);
          }
        else
          {
             entrance_session_close(EINA_FALSE);
             neev.event.status.login = NULL;
             neev.event.status.granted = EINA_FALSE;
             PT("server authenticate error\n");
          }
        entrance_event_send(&neev);

     }
   else if (eev->type == ENTRANCE_EVENT_ACTION)
     {
        PT("Action received\n");
        entrance_action_run(eev->event.action.action);
     }
   else if (eev->type == ENTRANCE_EVENT_CONF_GUI)
     {
        PT("Conf Gui received\n");
        entrance_config_set(&eev->event.conf_gui);
     }
   else if (eev->type == ENTRANCE_EVENT_CONF_USER)
     {
        PT("Conf user received\n");
        entrance_history_user_update(&eev->event.conf_user);
     }
   else
     PT("UNKNOW signal server\n");
   return EINA_TRUE;
}

static Eina_Bool
_entrance_server_write_cb(const void *data, size_t size, void *user_data EINA_UNUSED)
{
   const Eina_List *l;
   Ecore_Con_Client *ecc;
   EINA_LIST_FOREACH(ecore_con_server_clients_get(_entrance_server), l, ecc)
      ecore_con_client_send(ecc, data, size);

   return EINA_TRUE;
}

void
entrance_server_init(void)
{
   Ecore_Event_Handler *h;
   ecore_con_init();
   entrance_event_init(_entrance_server_read_cb,
                       _entrance_server_write_cb,
                       NULL);

   _entrance_server = ecore_con_server_add(ECORE_CON_LOCAL_SYSTEM,
                                        "entrance", 42, NULL);
   if (!_entrance_server)
     PT("server init fail\n");

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
entrance_server_shutdown(void)
{
   Ecore_Event_Handler *h;
   if (_entrance_server)
     ecore_con_server_del(_entrance_server);
   EINA_LIST_FREE(_handlers, h)
     ecore_event_handler_del(h);
   entrance_event_shutdown();
   ecore_con_shutdown();
}

void
entrance_server_client_wait(void)
{
   Entrance_Event eev;
   eev.type = ENTRANCE_EVENT_MAXTRIES;
   eev.event.maxtries.maxtries = EINA_TRUE;
   entrance_event_send(&eev);
}

