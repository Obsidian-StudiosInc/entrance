#include "entrance_client.h"
#include <Eina.h>

#define ENTRANCE_PASSWD_LEN 256

typedef struct Entrance_Gui_Login_ Entrance_Gui_Login;

static void _login_check_auth(Evas_Object *widget);
static void _login_xsession_update(Evas_Object *obj);
static void _login_xsession_guess(void *data, const char *user);
static void _login_xsession_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _login_login_unfocused_cb(void *data, Evas_Object *obj, void *event);
static void _login_login_activated_cb(void *data, Evas_Object *obj, void *event);
static char *_login_xsession_text_get(void *data, Evas_Object *obj, const char *part);
static void _login_auth_cb(void *data, const char *user, Eina_Bool granted);
static void _entrance_login_session_set(Evas_Object *widget, const char *name);
static void _entrance_login_auth_check_cb(void *data, Evas_Object *obj, const char *signal, const char *source);

static Entrance_Fill *_login_fill;

struct Entrance_Gui_Login_
{
   Ecore_Timer *write_timer;
   Ecore_Event_Handler *handler;
   Entrance_Xsession *session;
   struct
     {
        Entrance_Login_Cb login;
        void *data;
     } func;
   void *auth;
   Eina_Bool open_session : 1;
   Eina_Bool selected : 1;
   Eina_Bool catch : 1;
   Eina_Bool wait : 1;
};

#define LOGIN_GET(widget) \
   Entrance_Gui_Login *login; \
   login = evas_object_data_get(widget, "entrance"); \
   if (!login) return

static void
_login_check_auth(Evas_Object *widget)
{
   Evas_Object *o;
   const char *host, *passwd;
   LOGIN_GET(widget);

   o = elm_object_part_content_get(widget, "entrance.login");
   host = elm_entry_markup_to_utf8(elm_object_text_get(o));
   login->wait = EINA_TRUE;
   if (!login->auth)
     login->auth = entrance_connect_auth_cb_add(_login_auth_cb, widget);
   o = elm_object_part_content_get(widget, "entrance.password");
   passwd = elm_entry_markup_to_utf8(elm_object_text_get(o));
   if (login->session)
     entrance_connect_auth_send(host, passwd,
                                login->session->name,
                                login->open_session);
   else
     entrance_connect_auth_send(host, passwd, NULL, login->open_session);

   elm_object_signal_emit(widget, "entrance,auth,checking", "");
}

static void
_login_login_unfocused_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   Evas_Object *o;
   const char *hostname;
   LOGIN_GET(data);

   if (login->write_timer)
     {
        ecore_timer_del(login->write_timer);
        login->write_timer = NULL;
     }

   o = elm_object_part_content_get(data, "entrance.login");
   hostname = elm_entry_markup_to_utf8(elm_object_text_get(o));

   _login_xsession_guess(data, hostname);
}

static Eina_Bool
_login_login_timer_cb(void *data)
{
   Evas_Object *o;
   const char *hostname;
   Entrance_Gui_Login *login;

   login = evas_object_data_get(data, "entrance");
   if (!login) return ECORE_CALLBACK_CANCEL;

   o = elm_object_part_content_get(data, "entrance.login");

   hostname = elm_entry_markup_to_utf8(elm_object_text_get(o));
   _login_xsession_guess(data, hostname);

   login->write_timer = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_login_login_activated_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   elm_object_focus_set(data, EINA_TRUE);
   edje_object_signal_emit(data,
                           "entrance,auth,enable", "");
}

static void
_login_login_changed_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   LOGIN_GET(data);

   if (login->write_timer)
     ecore_timer_del(login->write_timer);
   login->write_timer = ecore_timer_add(0.5, _login_login_timer_cb, data);
}

static char *
_login_xsession_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part)
{
   Entrance_Xsession *xsession;
   xsession = data;
   if ((part) && (!strcmp(part, "icon")))
     {
        if (xsession->icon)
          return strdup(xsession->icon);
        else
          return NULL;
     }
   return strdup(xsession->name);

}

static void
_login_xsession_update(Evas_Object *obj)
{
   Evas_Object *o, *icon;
   LOGIN_GET(obj);
   o = elm_object_part_content_get(obj, "entrance.xsessions");
   if (!login->session) return;
   elm_object_text_set(o, login->session->name);
   icon = elm_object_part_content_get(o, "icon");
   if (login->session->icon &&
       strcmp(login->session->icon,""))
     {
       Eina_Stringshare *path;
       if (!icon)
          icon = elm_icon_add(o);
        path = entrance_gui_theme_path_get();
        if(login->session->icon[0]!='/')
          elm_icon_standard_set(icon, login->session->icon);
        else
          elm_image_file_set(icon, login->session->icon, path);
        eina_stringshare_del(path);
        elm_object_part_content_set(o, "icon", icon);
     }
   else
     {
        evas_object_del(icon);
        elm_object_part_content_set(o, "icon", NULL);
     }
}

static void
_login_xsession_guess(void *data, const char *user)
{
   const Eina_List *users, *l;
   Entrance_Login *eu;
   LOGIN_GET(data);

   users = entrance_gui_users_get();
   EINA_LIST_FOREACH(users, l, eu)
     {
        if (!strcmp(eu->login, user))
          {
             _entrance_login_session_set(data, eu->lsess);
             break;
          }
     }

   if (!l)
     {
        _entrance_login_session_set(data, NULL);
     }
}

static void
_login_xsession_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   LOGIN_GET(data);
   login->session = elm_object_item_data_get(event_info);
   _login_xsession_update(data);
}

static void
_login_auth_cb(void *data, const char *user, Eina_Bool granted)
{
   LOGIN_GET(data);
   if (login->wait)
     {
        login->wait = EINA_FALSE;
        entrance_connect_auth_cb_del(login->auth);
        login->auth = NULL;
        if (!granted)
          {
            elm_object_text_set(
                elm_object_part_content_get(data, "entrance.label"),
                _("Login failed"));
            elm_object_signal_emit(data, "entrance,auth,error", "");
            elm_object_signal_emit(
                elm_object_part_content_get(data, "entrance.login"),
                "entrance,auth,error", "login");
            elm_object_signal_emit(
                elm_object_part_content_get(data, "entrance.password"),
                "entrance,auth,error", "password");
          }
        else
          {
             elm_object_signal_emit(data,
                                    "entrance,auth,valid", "");
             elm_object_signal_emit(
                elm_object_part_content_get(data, "entrance.login"),
                "entrance,auth,valid", "login");
             elm_object_signal_emit(
                elm_object_part_content_get(data, "entrance.password"),
                "entrance,auth,valid", "password");
          }
     }
}

static void
_entrance_login_session_set(Evas_Object *widget, const char *name)
{
   Entrance_Xsession *sess;
   const Eina_List *l = NULL;
   LOGIN_GET(widget);
   if (name)
     {
        EINA_LIST_FOREACH(entrance_gui_xsessions_get(), l, sess)
          {
             if ((sess->name) &&
                 (!strcmp(sess->name, name)))
               {
                  break;
               }
          }
     }
   if (l)
     login->session = sess;
   else
     login->session = eina_list_data_get(entrance_gui_xsessions_get());
   _login_xsession_update(widget);
}

static void
_entrance_login_auth_check_cb(void *data, Evas_Object *obj EINA_UNUSED, const char *signal EINA_UNUSED, const char *source EINA_UNUSED)
{
   _login_check_auth(data);
}

static void
_login_auth_check_cb(void *data,
                     Evas_Object *obj EINA_UNUSED,
                     void *event EINA_UNUSED)
{
   _login_check_auth(data);
}

////////////////////////////////////////////////////////////////////////////////

void
entrance_login_init(void)
{
   _login_fill = entrance_fill_new(NULL, _login_xsession_text_get, NULL, NULL);
}

void
entrance_login_shutdown(void)
{
   // TODO callback_del on widget
   //free(_login);
   entrance_fill_del(_login_fill);
}

Evas_Object *
entrance_login_add(Evas_Object *obj, void *data)
{
   Evas_Object *h, *l, *o, *p, *t;
   Entrance_Gui_Login *login;

   /* layout */
   login = calloc(1, sizeof(Entrance_Gui_Login));
   login->func.data = data;
   o = entrance_gui_theme_get(obj, "entrance/login");
   evas_object_data_set(o, "entrance", login);

   /* label */
   t = elm_label_add(o);
   elm_object_part_content_set(o, "entrance.label", t);
   evas_object_show(t);

   /* login label */
   t = elm_label_add(o);
   elm_object_text_set (t, _("Login"));
   elm_object_part_content_set(o, "entrance.login_label", t);
   evas_object_show(t);

   /* login */
   l = elm_entry_add(o);
   elm_entry_single_line_set(l, EINA_TRUE);
   elm_entry_scrollable_set(l, EINA_TRUE);
   elm_object_part_content_set(o, "entrance.login", l);
   elm_object_focus_set(l, EINA_TRUE);
   evas_object_show(l);

   /* password label */
   t = elm_label_add(o);
   elm_object_text_set (t, _("Password"));
   elm_object_part_content_set(o, "entrance.password_label", t);
   evas_object_show(t);

   /* password */
   p = elm_entry_add(o);
   elm_entry_password_set(p, EINA_TRUE);
   elm_entry_single_line_set(p, EINA_TRUE);
   elm_entry_scrollable_set(p, EINA_TRUE);
   elm_object_part_content_set(o, "entrance.password", p);
   evas_object_show(p);

   /* callbacks */
   evas_object_smart_callback_add(l, "activated", _login_login_activated_cb, p);
   evas_object_smart_callback_add(l, "unfocused", _login_login_unfocused_cb, o);
   evas_object_smart_callback_add(l, "changed,user", _login_login_changed_cb, o);
   evas_object_smart_callback_add(p, "activated", _login_auth_check_cb, o);
   elm_object_signal_callback_add(o, "entrance,auth,check", "",
                                  _entrance_login_auth_check_cb, o);
   h = elm_hoversel_add(o);
   elm_hoversel_hover_parent_set(h, obj);
   evas_object_data_set(o, "entrance", login);
   elm_object_part_content_set(o, "entrance.xsessions", h);
   _login_xsession_update(o);
   return o;
}

void
entrance_login_xsessions_populate(Evas_Object *widget, Eina_List *xsessions)
{
   PT("Session set");
   Evas_Object *o;
   LOGIN_GET(widget);

   o = elm_object_part_content_get(widget, "entrance.xsessions");
   entrance_fill(o, _login_fill, xsessions, NULL,
                 _login_xsession_clicked_cb, widget);
   login->session = eina_list_data_get(xsessions);
   _login_xsession_update(widget);
}

void
entrance_login_login_set(Evas_Object *widget, const char *user)
{
   Evas_Object *o;
   o = elm_object_part_content_get(widget, "entrance.login");
   elm_object_text_set(o, user);
   elm_object_signal_emit(widget,
                           "entrance,auth,enable", "");
   o = elm_object_part_content_get(widget, "entrance.password");
   elm_object_focus_set(o, EINA_TRUE);

  _login_xsession_guess(widget, user);
}

void
entrance_login_open_session_set(Evas_Object *widget, Eina_Bool open_session)
{
   Evas_Object *o;
   LOGIN_GET(widget);
   open_session = !!open_session;
   login->open_session = open_session;
   o = elm_object_part_content_get(widget, "entrance.xsessions");
   if (login->open_session)
     {
        elm_object_signal_emit(widget,
                               "entrance,xsession,enabled", "");
        evas_object_show(o);
     }
   else
     {
        elm_object_signal_emit(widget,
                               "entrance,xsession,disabled", "");
        evas_object_hide(o);
     }
}

