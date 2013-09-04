#include "entrance.h"

#define ENTRANCE_SESSION_KEY "session"
#define ENTRANCE_HISTORY_FILE "entrance.hst"

static void _entrance_history_read();
static void _entrance_history_write();
static void _entrance_user_init();
static void _entrance_user_shutdown();
const char *_entrance_history_match(const char *login);


static Eet_Data_Descriptor *_eddh;
static Entrance_History *_entrance_history;
static Eina_List *_lusers = NULL;
static Eina_Bool _history_update = EINA_FALSE;

void
entrance_history_init()
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor_Class eddc;
   // TODO add idler to load history and thread stuff
   // TODO screenshot a new session after 3 min

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Login);
   edd = eet_data_descriptor_stream_new(&eddc);
#define EET_LOGIN_ADD(NAME, TYPE) \
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Login, # NAME, NAME, TYPE);
   EET_LOGIN_ADD(login, EET_T_STRING);
   EET_LOGIN_ADD(session, EET_T_STRING);
   EET_LOGIN_ADD(icon.path, EET_T_STRING);
   EET_LOGIN_ADD(icon.group, EET_T_STRING);
   EET_LOGIN_ADD(background.path, EET_T_STRING);
   EET_LOGIN_ADD(background.group, EET_T_STRING);
#undef EET_LOGIN_ADD

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_History);
   _eddh = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_LIST(_eddh, Entrance_History, "history", history, edd);

   _entrance_history_read();
   _entrance_user_init();
}

Eina_List
*entrance_history_get()
{
   return _lusers;
}

void
entrance_history_shutdown()
{
   _entrance_history_write();
   _entrance_user_shutdown();
}

static void
_entrance_history_read()
{
   Eet_File *ef;

   ef = eet_open("/var/cache/"PACKAGE"/"ENTRANCE_HISTORY_FILE,
                 EET_FILE_MODE_READ_WRITE);
   if (!(ef) || !(_entrance_history = eet_data_read(ef, _eddh, ENTRANCE_SESSION_KEY)))
     {
        PT("Error on reading last session login\n");
        _entrance_history = calloc(1, sizeof(Entrance_History));
     }
   eet_close(ef);
}

static void
_entrance_history_write()
{
   Eet_File *ef;
   Entrance_Login *el;


   if (_history_update)
     {
        PT("writing history file\n");
        ef = eet_open("/var/cache/"PACKAGE"/"ENTRANCE_HISTORY_FILE,
                      EET_FILE_MODE_READ_WRITE);
        if (!ef)
          ef = eet_open("/var/cache/"PACKAGE"/"ENTRANCE_HISTORY_FILE,
                        EET_FILE_MODE_WRITE);

        if (!eet_data_write(ef, _eddh, ENTRANCE_SESSION_KEY,
                            _entrance_history, 1))
          PT("Error on updating last session login\n");

        eet_close(ef);
     }
   EINA_LIST_FREE(_entrance_history->history, el)
     {
        eina_stringshare_del(el->login);
        eina_stringshare_del(el->session);
     }
}

void
entrance_history_push(const char *login, const char *session)
{
   Eina_List *l;
   Entrance_Login *el;

   EINA_LIST_FOREACH(_entrance_history->history, l, el)
     {
        if (!strcmp(login, el->login))
          {
             if (!session)
               {
                  eina_stringshare_del(el->session);
                  el->session = NULL;
               }
             else if (el->session && strcmp(session, el->session))
               {
                  eina_stringshare_replace(&el->session, session);
                  _history_update = EINA_TRUE;
               }
             break;
          }
     }
   if (!el)
     {
        if ((el = calloc(1, sizeof(Entrance_Login))))
          {
             el->login = eina_stringshare_add(login);
             if (session) el->session = eina_stringshare_add(session);
             else el->session = NULL;
             _entrance_history->history =
                eina_list_append(_entrance_history->history, el);
             _history_update = EINA_TRUE;
          }
     }
}


const char *
_entrance_history_match(const char *login)
{
   Eina_List *l;
   Entrance_Login *el;
   const char *ret = NULL;
   EINA_LIST_FOREACH(_entrance_history->history, l, el)
     {
        if (!strcmp(el->login, login))
          ret = el->session;
     }
   return ret;
}

static void
_entrance_user_init()
{
   char buf[PATH_MAX];
   FILE *f;
   Entrance_User_Event *eu;
   Eina_List *lu = NULL;
   char *token;
   char *user;
   int uid;

   PT("scan for users\n");
   f = fopen("/etc/passwd", "r");
   if (f)
     {
        while (fgets(buf, sizeof(buf), f))
          {
             user = strtok(buf, ":");
             strtok(NULL, ":");
             token = strtok(NULL, ":");
             uid = atoi(token);
             if (uid > 999 && uid < 3000)
               lu = eina_list_append(lu, eina_stringshare_add(user));
          }
        fclose(f);
     }
   EINA_LIST_FREE(lu, user)
     {
        if ((eu = (Entrance_User_Event *) calloc(1, sizeof(Entrance_User_Event))))
          {
             eu->login = eina_stringshare_add(user);
             snprintf(buf, sizeof(buf),
                      "/var/cache/"PACKAGE"/users/%s.edj", user);
             if (ecore_file_exists(buf))
               eu->image.path = eina_stringshare_add(buf);
             eu->lsess = _entrance_history_match(user);
             eina_stringshare_del(user);
             _lusers = eina_list_append(_lusers, eu);
          }
     }
}

static void
_entrance_user_shutdown()
{
   Entrance_User_Event *eu;
   EINA_LIST_FREE(_lusers, eu)
     {
        eina_stringshare_del(eu->login);
        free(eu);
     }
   free(_entrance_history);
}

