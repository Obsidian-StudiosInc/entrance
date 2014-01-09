#include <Ecore_Con.h>
#include "entrance_client.h"

typedef struct
{
   Entrance_Connect_Auth_Cb func;
   void *data;
} Entrance_Connect_Auth;

static Eina_Bool _entrance_connect_add(void *data, int type, void *event);
static Eina_Bool _entrance_connect_del(void *data, int type, void *event);
static Eina_Bool _entrance_connect_data(void *data, int type, void *event);
static void _entrance_connect_auth(const char *login, Eina_Bool granted);

static Ecore_Con_Server *_entrance_connect;
static Eina_List *_handlers = NULL;
static Eina_List *_auth_list = NULL;

static Eina_Bool
_entrance_connect_add(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   PT("connected\n");
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_entrance_connect_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   PT("disconnected\n");
   _entrance_connect = NULL;

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_entrance_connect_data(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Server_Data *ev;
   ev = event;

   entrance_event_received(ev->data, ev->size);

   return ECORE_CALLBACK_RENEW;
}

static void
_entrance_connect_auth(const char *login, Eina_Bool granted)
{
   Entrance_Connect_Auth *auth;
   Eina_List *l, *ll;

   EINA_LIST_FOREACH_SAFE(_auth_list, l, ll, auth)
     {
        if (auth->func)
          auth->func(auth->data, login, granted);
     }
}

static Eina_Bool
_entrance_connect_read_cb(const void *data, size_t size EINA_UNUSED, void *user_data EINA_UNUSED)
{
   const Entrance_Event *eev;
   eev = data;
   if (eev)
     {
        if (eev->type == ENTRANCE_EVENT_STATUS)
          {
             if (eev->event.status.granted)
               PT("Auth granted :)\n");
             else
               PT("Auth error :(\n");
             _entrance_connect_auth(eev->event.status.login,
                                    eev->event.status.granted);
          }
        else if (eev->type == ENTRANCE_EVENT_MAXTRIES)
          {
             PT("Max tries !\n");
             entrance_gui_auth_max_tries();
          }
        else if (eev->type == ENTRANCE_EVENT_XSESSIONS)
          {
             PT("Xsession received\n");
             entrance_gui_xsessions_set(eev->event.xsessions.xsessions);
          }
        else if (eev->type == ENTRANCE_EVENT_USERS)
          {
             PT("Users received\n");
             entrance_gui_users_set(eev->event.users.users);
          }
        else if (eev->type == ENTRANCE_EVENT_ACTIONS)
          {
             PT("Action received\n");
             entrance_gui_actions_set(eev->event.actions.actions);
          }
        else if (eev->type == ENTRANCE_EVENT_CONF_GUI)
          {
             PT("Gui conf received\n");
             entrance_gui_conf_set(&(eev->event.conf_gui));
          }
        else
          {
             PT("UNKNOW signal ");
             fprintf(stderr, "%d\n", eev->type);
          }
        //free(eev);
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_entrance_connect_write_cb(const void *data, size_t size, void *user_data EINA_UNUSED)
{
   ecore_con_server_send(_entrance_connect, data, size);
   return ECORE_CALLBACK_RENEW;
}

void
entrance_connect_auth_send(const char *login, const char *password, const char *session, Eina_Bool open_session)
{
   Entrance_Event eev;

   PT("Request auth\n");
   eev.event.auth.login = login;
   eev.event.auth.password = password;
   eev.event.auth.session = session;
   eev.event.auth.open_session = open_session;
   eev.type = ENTRANCE_EVENT_AUTH;
   entrance_event_send(&eev);
}

void
entrance_connect_action_send(unsigned char id)
{
   Entrance_Event eev;

   PT("Request action %d\n", id);
   eev.event.action.action = id;
   eev.type = ENTRANCE_EVENT_ACTION;
   entrance_event_send(&eev);
}

void
entrance_connect_conf_gui_send(Entrance_Conf_Gui_Event *ev)
{
   Entrance_Event eev;
   PT("Send gui config\n");
   eev.event.conf_gui.bg.path = ev->bg.path;
   eev.event.conf_gui.bg.group = ev->bg.group;

   eev.type = ENTRANCE_EVENT_CONF_GUI;
   entrance_event_send(&eev);
}

void
entrance_connect_conf_user_send(Entrance_Login *el)
{
   Entrance_Event eev;
   PT("Send user config\n");
   eev.event.conf_user.login = el->login;
   eev.event.conf_user.lsess = el->lsess;
   eev.event.conf_user.image.group = el->image.group;
   eev.event.conf_user.image.path = el->image.path;
   eev.event.conf_user.bg.path = el->bg.path;
   eev.event.conf_user.bg.group = el->bg.group;
   eev.event.conf_user.remember_session = el->remember_session;
   eev.type = ENTRANCE_EVENT_CONF_USER;
   entrance_event_send(&eev);
}

void *
entrance_connect_auth_cb_add(Entrance_Connect_Auth_Cb func, void *data)
{
   PT("auth handler add\n");
   Entrance_Connect_Auth *auth;
   auth = malloc(sizeof(Entrance_Connect_Auth));
   auth->func = func;
   auth->data = data;
   _auth_list = eina_list_append(_auth_list, auth);
   return auth;
}

void
entrance_connect_auth_cb_del(void *auth)
{
   PT("auth handler remove\n");
   _auth_list = eina_list_remove(_auth_list, auth);
}

void
entrance_connect_init(void)
{
   Ecore_Event_Handler *h;
   ecore_con_init();
   entrance_event_init(_entrance_connect_read_cb,
                       _entrance_connect_write_cb,
                       NULL);
   _entrance_connect = ecore_con_server_connect(ECORE_CON_LOCAL_SYSTEM,
                                                "entrance", 42, NULL);
   if (_entrance_connect)
     PT("client server init ok\n");
   else
     PT("client server init fail\n");
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
entrance_connect_shutdown(void)
{
   Ecore_Event_Handler *h;
   EINA_LIST_FREE(_handlers, h)
      ecore_event_handler_del(h);
   entrance_event_shutdown();
   ecore_con_shutdown();
}

