#include "entrance.h"

#define ENTRANCE_SESSION_KEY "session"
#define ENTRANCE_HISTORY_FILE "entrance.hst"

static void _entrance_history_read(void);
static void _entrance_history_write(void);
static void _entrance_user_init(void);
static void _entrance_user_shutdown(void);
const char *_entrance_history_match(const char *login);


static Eet_Data_Descriptor *_eddh;
static Entrance_History *_entrance_history;
static Eina_List *_lusers = NULL;
static Eina_Bool _history_update = EINA_FALSE;

void
entrance_history_init(void)
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor_Class eddc;
   // TODO add idler to load history and thread stuff
   // TODO screenshot a new session after 3 min

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Login);
   edd = entrance_event_user_dd();

   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_History);
   _eddh = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_LIST(_eddh, Entrance_History, "history", history, edd);

   _entrance_history_read();
   _entrance_user_init();
}

Eina_List
*entrance_history_get(void)
{
   return _lusers;
}

void
entrance_history_shutdown(void)
{
   _entrance_history_write();
   _entrance_user_shutdown();
}

static void
_entrance_history_read(void)
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
_entrance_history_write(void)
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
        eina_stringshare_del(el->image.path);
        eina_stringshare_del(el->image.group);
        eina_stringshare_del(el->bg.path);
        eina_stringshare_del(el->bg.group);
        eina_stringshare_del(el->lsess);
     }
}

void
entrance_history_push(const char *login, const char *session)
{
   Eina_List *l;
   Entrance_Login *el;

   PT("history push for user %s session %s\n", login, session);
   EINA_LIST_FOREACH(_entrance_history->history, l, el)
     {
        if (!strcmp(login, el->login))
          {
             PT("History updating\n");
             if (el->remember_session)
               {
                  if (!session)
                    {
                       eina_stringshare_del(el->lsess);
                       el->lsess = NULL;
                    }
                  else if (el->lsess && strcmp(session, el->lsess))
                    {
                       eina_stringshare_replace(&el->lsess, session);
                       _history_update = EINA_TRUE;
                    }
               }
             break;
          }
     }
   if (!el)
     {
        PT("History create a new entry for %s\n", login);
        if ((el = calloc(1, sizeof(Entrance_Login))))
          {
             el->login = eina_stringshare_add(login);
             if (session) el->lsess = eina_stringshare_add(session);
             else el->lsess = NULL;
             el->remember_session = EINA_TRUE;
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
          ret = el->lsess;
     }
   return ret;
}

static void
_entrance_user_init(void)
{
   char buf[PATH_MAX];
   FILE *f;
   Entrance_Login *eu;
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
        if ((eu = (Entrance_Login *) calloc(1, sizeof(Entrance_Login))))
          {
             eu->login = eina_stringshare_add(user);
             snprintf(buf, sizeof(buf),
                      "/var/cache/"PACKAGE"/users/%s.edj", user);
             if (ecore_file_exists(buf))
               eu->image.path = eina_stringshare_add(buf);
             eu->lsess = _entrance_history_match(user);
             eu->remember_session = EINA_TRUE;
             eina_stringshare_del(user);
             _lusers = eina_list_append(_lusers, eu);
          }
     }
}

static void
_entrance_user_shutdown(void)
{
   Entrance_Login *eu;
   EINA_LIST_FREE(_lusers, eu)
     {
        eina_stringshare_del(eu->login);
        free(eu);
     }
   free(_entrance_history);
}

