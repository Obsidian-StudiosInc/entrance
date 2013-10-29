#include "entrance.h"

typedef void (*Entrance_Action_Cb)(void *data);

static void _entrance_action_shutdown(void *data);
static void _entrance_action_reboot(void *data);
static void _entrance_action_suspend(void *data);
static Eina_Bool _entrance_action_exe_event_del_cb(void *data, int type, void *event);
#ifdef HAVE_GRUB2
#define GRUB2_FILE "/boot/grub/grub.cfg"

static void _entrance_action_grub2_get(void);
#endif

static Eina_List *_entrance_actions = NULL;

typedef struct Entrance_Action_Data__
{
   unsigned char id;
   const char *label;
   Entrance_Action_Cb func;
   void *data;
} Entrance_Action_Data;

static Ecore_Exe *_action_exe = NULL;

static Entrance_Action_Data *
_entrance_action_add(const char *label, Entrance_Action_Cb func, void *data)
{
   Entrance_Action_Data *ead;
   ead = calloc(1, sizeof(Entrance_Action_Data));
   ead->label = eina_stringshare_add(label);
   ead->func = func;
   ead->data = data;
   ead->id = (unsigned char)eina_list_count(_entrance_actions);
   ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                           _entrance_action_exe_event_del_cb, NULL);
   return ead;
}


void
entrance_action_init(void)
{
   _entrance_actions = eina_list_append(_entrance_actions,
      _entrance_action_add("Shutdown", _entrance_action_shutdown, NULL));
   _entrance_actions = eina_list_append(_entrance_actions,
      _entrance_action_add("Reboot", _entrance_action_reboot, NULL));
   _entrance_actions = eina_list_append(_entrance_actions,
      _entrance_action_add("Suspend", _entrance_action_suspend, NULL));
#ifdef HAVE_GRUB2
   _entrance_action_grub2_get();
#endif
}

Eina_List *
entrance_action_get(void)
{
   Entrance_Action_Data *ead;
   Entrance_Action *ea;
   Eina_List *l, *ret = NULL;

   EINA_LIST_FOREACH(_entrance_actions, l, ead)
     {
        ea = calloc(1, sizeof(Entrance_Action));
        ea->label = eina_stringshare_add(ead->label);
        ea->id = ead->id;
        ret = eina_list_append(ret, ea);
     }
   return ret;
}

void
entrance_action_shutdown(void)
{
   Entrance_Action_Data *ead;
   EINA_LIST_FREE(_entrance_actions, ead)
     {
        eina_stringshare_del(ead->label);
        free(ead);
     }
}


void
entrance_action_run(int action)
{
   Entrance_Action_Data *ead;

   ead = eina_list_nth(_entrance_actions, action);
   if (ead)
     ead->func(ead->data);
}

static void
_entrance_action_suspend(void *data EINA_UNUSED)
{
   PT("Suspend");
   _action_exe = NULL;
   ecore_exe_run(entrance_config->command.suspend, NULL);
}

static void
_entrance_action_shutdown(void *data EINA_UNUSED)
{
   PT("Shutdown");
   _action_exe = ecore_exe_run(entrance_config->command.shutdown, NULL);
}

static void
_entrance_action_reboot(void *data EINA_UNUSED)
{
   PT("Reboot\n");
   _action_exe = ecore_exe_run(entrance_config->command.reboot, NULL);
}

static Eina_Bool
_entrance_action_exe_event_del_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Exe_Event_Del *ev;
   Eina_Bool ret = ECORE_CALLBACK_PASS_ON;
   ev = event;
   if (!ev->exe) return ret;
   if (ev->exe == _action_exe)
     {
        PT("action quit requested by user\n");
        ecore_main_loop_quit();
        ret = ECORE_CALLBACK_DONE;
     }
   return ret;
}

/* grub2 action */
#ifdef HAVE_GRUB2
static void
_entrance_action_grub2(void *data)
{
   size_t i = 0;
   char buf[PATH_MAX];
   i = (size_t)data;

   snprintf(buf, sizeof(buf),
            "grub-reboot %d && %s", i, entrance_config->command.reboot);
   _action_exe = ecore_exe_run(buf, NULL);

}

static char *
_entrance_memstr(char *data, size_t length, char *look, unsigned int size)
{
   char *tmp;

   while (length >= size)
     {
        tmp = memchr(data, *look, length);
        if (!tmp) return NULL;

        if (strncmp(tmp + 1, look + 1, size - 1) == 0)
          return tmp;
        length = tmp - data;
        data = tmp;
     }

   return NULL;
}


static void
_entrance_action_grub2_get(void)
{
   Eina_File *f;
   unsigned char grub2_ok = 0;
   size_t menuentry = 0;
   char *data;
   char *r, *r2;
   char *s;
   int i;

   PT("trying to open "GRUB2_FILE);
   f = eina_file_open(GRUB2_FILE, EINA_FALSE);
   if (!f) return ;
   fprintf(stderr, " o");

   data = eina_file_map_all(f, EINA_FILE_SEQUENTIAL);
   if (!data) goto on_error;
   fprintf(stderr, "k\n");

   s = data;
   r2 = NULL;
   for (i = eina_file_size_get(f); i > 0; --i, s++)
     {
        int size;

        /* working line by line */
        r = memchr(s, '\n', i);
        if (!r)
          {
             r = s + i;
             i = 0;
          }
        size = r - s;

        if (*s == '#')
          goto end_line;

        /* look if the word is in this line */
        if (!grub2_ok)
          r2 = _entrance_memstr(s, size, "default=\"${saved_entry}\"", 24);
        else
          r2 = _entrance_memstr(s, size, "menuentry", 9);

        /* still some lines to read */
        if (!r2) goto end_line;

        if (!grub2_ok)
          {
             grub2_ok = 1;
             PT("GRUB2 save mode found\n");
          }
        else
          {
             char *action;
             char *local;
             char *tmp;

             r2 += 10;
             size -= 10;

             tmp = memchr(r2, '\'', size);
             if (!tmp) goto end_line;

             size -= tmp - r2 + 1;
             r2 = tmp + 1;
             tmp = memchr(r2, '\'', size);
             if (!tmp) goto end_line;

             local = alloca(tmp - r2 + 1);
             memcpy(local, r2, tmp - r2);
             local[tmp - r2] = '\0';

             action = malloc((tmp - r2 + 1 + 11) * sizeof (char));
             if (!action) goto end_line;

             sprintf(action, "Reboot on %s", local);
             PT("GRUB2 '%s'\n", action);
             _entrance_actions =
                eina_list_append(_entrance_actions,
                                 _entrance_action_add(action,
                                                  _entrance_action_grub2,
                                                  (void*)(menuentry++)));

          }

     end_line:
        i -= size;
        s = r;
     }

   eina_file_map_free(f, data);
 on_error:
   eina_file_close(f);
}
#endif
