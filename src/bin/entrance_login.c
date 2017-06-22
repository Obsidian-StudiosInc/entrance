#include "entrance_client.h"
#include <Eina.h>

#define ENTRANCE_PASSWD_LEN 256

typedef struct Entrance_Gui_Login_ Entrance_Gui_Login;

static void _login_reset(Evas_Object *widget);
static void _login_backspace(Evas_Object *widget);
static void _login_delete(Evas_Object *widget);
static void _login_select(Evas_Object *widget);
static void _login_update(Evas_Object *widget);
static void _login_check_auth(Evas_Object *widget);
static void _login_password_catch(Evas_Object *widget, Eina_Bool catch);
static Eina_Bool _login_key_down_cb(void *data, int type, void *event);
static void _login_xsession_update(Evas_Object *obj);
static void _login_xsession_guess(void *data, const char *user);
static void _login_xsession_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static Eina_Bool _login_input_event_cb(void *data EINA_UNUSED, Evas_Object *obj, Evas_Object *src, Evas_Callback_Type type, void *event_info);
static void _login_login_unfocused_cb(void *data, Evas_Object *obj, void *event);
static void _login_password_focused_cb(void *data, Evas_Object *obj, void *event);
static void _login_password_unfocused_cb(void *data, Evas_Object *obj, void *event);
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
   char passwd[ENTRANCE_PASSWD_LEN];
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
_login_reset(Evas_Object *widget)
{
   LOGIN_GET(widget);
   memset(login->passwd, 0, sizeof(char) * ENTRANCE_PASSWD_LEN);
   _login_update(widget);
}

static void
_login_backspace(Evas_Object *widget)
{
   int len, val, pos;

   LOGIN_GET(widget);
   len = strlen(login->passwd);
   if (len > 0)
     {
        pos = evas_string_char_prev_get(login->passwd, len, &val);
        if ((pos < len) && (pos >= 0))
          {
             login->passwd[pos] = '\0';
             _login_update(widget);
          }
     }
}

static void
_login_delete(Evas_Object *widget)
{
   _login_backspace(widget);
}

static void
_login_select(Evas_Object *widget)
{
   LOGIN_GET(widget);
   Evas_Object *o;
   o = elm_object_part_content_get(widget, "entrance.password");
   elm_entry_select_all(o);
   login->selected = EINA_TRUE;
}

static void
_login_unselect(Evas_Object *widget)
{
   Evas_Object *o;
   LOGIN_GET(widget);
   o = elm_object_part_content_get(widget, "entrance.password");
   elm_entry_select_none(o);
   login->selected = EINA_FALSE;
}

static void
_login_update(Evas_Object *widget)
{
   Evas_Object *o;
   char str[ENTRANCE_PASSWD_LEN];
   int len;
   LOGIN_GET(widget);

   len = eina_unicode_utf8_get_len(login->passwd);

   memset(str, '*', sizeof(char) * len);
   str[len] = '\0';

   o = elm_object_part_content_get(widget, "entrance.password");
   elm_object_text_set(o, str);
   elm_entry_cursor_end_set(o);
}

static void
_login_check_auth(Evas_Object *widget)
{
   Evas_Object *o;
   const char *host;
   LOGIN_GET(widget);

   o = elm_object_part_content_get(widget, "entrance.login");
   host = elm_entry_markup_to_utf8(elm_object_text_get(o));
   login->wait = EINA_TRUE;
   if (!login->auth)
     login->auth = entrance_connect_auth_cb_add(_login_auth_cb, widget);
   if (login->session)
     entrance_connect_auth_send(host, login->passwd,
                                login->session->name,
                                login->open_session);
   else
     entrance_connect_auth_send(host, login->passwd,
                                NULL, login->open_session);

   _login_reset(widget);

   elm_object_signal_emit(widget,
                          "entrance,auth,checking", "");
   elm_object_signal_emit(
      elm_object_part_content_get(widget, "entrance.login"),
      "entrance,auth,checking", "login");
   elm_object_signal_emit(
      elm_object_part_content_get(widget, "entrance.password"),
      "entrance,auth,checking", "password");
}

static void
_login_password_catch(Evas_Object *widget, Eina_Bool catch)
{
   LOGIN_GET(widget);
   if (login->catch != catch)
     {
        if (catch)
          {
             PT("catch password");
             login->handler = ecore_event_handler_add(
                ECORE_EVENT_KEY_DOWN, _login_key_down_cb, widget);
          }
        else
          {
             PT("uncatch password");
             ecore_event_handler_del(login->handler);
             login->handler = NULL;
          }
     }
   login->catch = catch;
}

static Eina_Bool
_login_key_down_cb(void *data, int type EINA_UNUSED, void *event)
{
   Ecore_Event_Key *ev;
   LOGIN_GET(data) ECORE_CALLBACK_PASS_ON;
   ev = event;


   elm_object_signal_emit(data, "entrance,auth,changed", "");
   elm_object_signal_emit(
      elm_object_part_content_get(data, "entrance.password"),
      "entrance,auth,changed", "");

   if (!strcmp(ev->key, "KP_Enter"))
     {
        _login_check_auth(data);
     }
   else if (!strcmp(ev->key, "Return"))
     {
        _login_check_auth(data);
     }
   else if (!strcmp(ev->key, "BackSpace"))
     {
        if (login->selected)
          {
             _login_reset(data);
             _login_unselect(data);
          }
        else
          _login_backspace(data);
     }
   else if (!strcmp(ev->key, "Delete"))
     {
        if (login->selected)
          {
             _login_reset(data);
          }
        else
          _login_delete(data);
     }
   else if ((!strcmp(ev->key, "Tab"))
            || (!strcmp(ev->key, "ISO_Left_Tab")))
     {
        if (ev->modifiers & ECORE_EVENT_MODIFIER_SHIFT)
          {
             PT("focus previous");
             elm_object_focus_next(data, ELM_FOCUS_PREVIOUS);
          }
        else
          {
             PT("focus next");
             elm_object_focus_next(data, ELM_FOCUS_NEXT);
          }
     }
   else if ((!strcmp(ev->key, "u"))
            && (ev->modifiers & ECORE_EVENT_MODIFIER_CTRL))
     {
        _login_reset(data);
     }
   else if ((!strcmp(ev->key, "a"))
            && (ev->modifiers & ECORE_EVENT_MODIFIER_CTRL))
     {
        _login_select(data);
     }
   else
     {
        if (ev->compose)
          {
             if (login->selected)
               {
                  _login_reset(data);
                  _login_unselect(data);
               }
             if (strlen(login->passwd) <
                 (ENTRANCE_PASSWD_LEN - strlen(ev->compose)))
               {
                  strcat(login->passwd, ev->compose);
                  _login_update(data);
               }
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_login_input_event_cb(void *data, Evas_Object *obj EINA_UNUSED, Evas_Object *src EINA_UNUSED, Evas_Callback_Type type, void *event_info EINA_UNUSED)
{
   LOGIN_GET(data) EINA_FALSE;
   return (login->catch
           && ((type == EVAS_CALLBACK_KEY_UP)
               || (type == EVAS_CALLBACK_KEY_DOWN)));
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

static void
_login_password_focused_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   _login_password_catch(data, EINA_TRUE);
}

static void
_login_password_unfocused_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
   _login_password_catch(data, EINA_FALSE);
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
             entrance_gui_user_bg_set(eu->bg.path, eu->bg.group);
             break;
          }
     }

   if (!l)
     {
        _entrance_login_session_set(data, NULL);
        entrance_gui_user_bg_set(NULL, NULL);
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
        if (login->func.login)
          login->func.login(login->func.data, user, granted);
        login->wait = EINA_FALSE;
        entrance_connect_auth_cb_del(login->auth);
        login->auth = NULL;
        if (!granted)
          {
             elm_object_signal_emit(data,
                                    "entrance,auth,error", "");
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

////////////////////////////////////////////////////////////////////////////////

void
entrance_login_init(void)
{
   _login_fill = entrance_fill_new(NULL, _login_xsession_text_get,
                                   NULL, NULL, NULL);
}

void
entrance_login_shutdown(void)
{
   // TODO callback_del on widget
   //_login_password_catch(NULL, EINA_FALSE);
   //free(_login);
   entrance_fill_del(_login_fill);
}

Evas_Object *
entrance_login_add(Evas_Object *obj, Entrance_Login_Cb login_cb, void *data)
{
   Evas_Object *o, *h, *p;
   Entrance_Gui_Login *login;

   /* layout */
   login = calloc(1, sizeof(Entrance_Gui_Login));
   login->func.login = login_cb;
   login->func.data = data;
   o = entrance_gui_theme_get(obj, "entrance/login");
   evas_object_data_set(o, "entrance", login);


   /* login */
   h = elm_entry_add(o);
   elm_entry_single_line_set(h, EINA_TRUE);
   elm_entry_scrollable_set(h, EINA_TRUE);
   elm_object_part_content_set(o, "entrance.login", h);
   elm_object_focus_set(h, EINA_TRUE);
   evas_object_show(h);

   /* password */
   p = elm_entry_add(o);
   elm_entry_password_set(p, EINA_TRUE);
   elm_entry_single_line_set(p, EINA_TRUE);
   elm_entry_scrollable_set(p, EINA_TRUE);
   elm_object_part_content_set(o, "entrance.password", p);
   evas_object_show(p);

   /* callbacks */
   elm_object_event_callback_add(o, _login_input_event_cb, o);
   evas_object_smart_callback_add(h, "activated",
                                  _login_login_activated_cb, p);
   evas_object_smart_callback_add(h, "unfocused",
                                  _login_login_unfocused_cb, o);
   evas_object_smart_callback_add(h, "changed,user",
                                  _login_login_changed_cb, o);
   evas_object_smart_callback_add(p, "focused",
                                  _login_password_focused_cb, o);
   evas_object_smart_callback_add(p, "unfocused",
                                  _login_password_unfocused_cb, o);
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

