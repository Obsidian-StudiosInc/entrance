#include "entrance.h"
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "Ecore_Getopt.h"
#include <xcb/xcb.h>

#define ENTRANCE_DISPLAY ":0.0"

#define PT(x)                                                   \
{                                                               \
   ct = time(NULL);                                             \
   lc = localtime(&ct);                                         \
   memset(entrance_time_d, 0, sizeof(entrance_time_d));         \
   strftime(entrance_time_d, sizeof(entrance_time_d),           \
            "%b %_2d %T", lc);                                  \
   fprintf(stderr, "(%s) "PACKAGE": %s\n", entrance_time_d, x); \
}


static Eina_Bool _open_log();
static int _entrance_main(const char *dname);
static void _remove_lock();
static void _signal_cb();
static void _signal_log();
static Eina_Bool _entrance_client_del(void *data, int type, void *event);

static Eina_Bool _testing = 0;
static Eina_Bool _xephyr = 0;
static Ecore_Exe *_entrance_client = NULL;



static void
_signal_cb(int sig)
{
   fprintf(stderr, PACKAGE": signal %d received\n", sig);
   //FIXME  if I don't have main loop at this time ?
   if (_entrance_client) ecore_exe_terminate(_entrance_client);
   /*
   entrance_session_shutdown();
   entrance_xserver_shutdown();
   exit(1);
   */
}

static void
_signal_log(int sig)
{
   fprintf(stderr, PACKAGE": signal %d received reopen the log file\n", sig);
   entrance_close_log();
   _open_log();
}

static Eina_Bool
_get_lock()
{
   FILE *f;
   char buf[128];
   int my_pid;

   my_pid = getpid();
   f = fopen(entrance_config->lockfile, "r");
   if (!f)
     {
        /* No lockfile, so create one */
        f = fopen(entrance_config->lockfile, "w");
        if (!f)
          {
             fprintf(stderr, PACKAGE": Couldn't create lockfile %s!\n",
                     entrance_config->lockfile);
             return (EINA_FALSE);
          }
        snprintf(buf, sizeof(buf), "%d\n", my_pid);
        if (!fwrite(buf, strlen(buf), 1, f))
          {
             fclose(f);
             fprintf(stderr, PACKAGE": Couldn't write the lockfile\n");
             return EINA_FALSE;
          }
        fclose(f);
     }
   else
     {
        int pid = 0;
        /* read the lockfile */
        if (fgets(buf, sizeof(buf), f))
          pid = atoi(buf);
        fclose(f);
        if (pid == my_pid)
          return EINA_TRUE;
        fprintf(stderr, "A lock file are present another instance are present ?\n");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static void
_update_lock()
{
   FILE *f;
   char buf[128];
   f = fopen(entrance_config->lockfile, "w");
   snprintf(buf, sizeof(buf), "%d\n", getpid());
   if (!fwrite(buf, strlen(buf), 1, f))
     fprintf(stderr, PACKAGE": Coudn't update lockfile\n");
   fclose(f);
}

static void
_remove_lock()
{
   remove(entrance_config->lockfile);
}

static Eina_Bool
_open_log()
{
   FILE *elog;
   elog = fopen(entrance_config->logfile, "a");
   if (!elog)
     {
        fprintf(stderr, PACKAGE": could not open logfile %s!!!\n",
                entrance_config->logfile);
        return EINA_FALSE;
     }
   fclose(elog);
   if (!freopen(entrance_config->logfile, "a", stdout))
     fprintf(stderr, PACKAGE": Error on reopen stdout\n");
   setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
   if (!freopen(entrance_config->logfile, "a", stderr))
     fprintf(stderr, PACKAGE": Error on reopen stderr\n");
   setvbuf(stderr, NULL, _IONBF, BUFSIZ);
   return EINA_TRUE;
}

void
entrance_close_log()
{
   {
      fclose(stderr);
      fclose(stdout);
   }
}

static void
_entrance_wait()
{
   execl(PACKAGE_BIN_DIR"/entrance_wait", "/usr/sbin/entrance", NULL);
   fprintf(stderr, PACKAGE": HUM HUM HUM ...\n\n\n");
}

static int
_entrance_main(const char *dname)
{
   if (!entrance_config->autologin)
     {
        if (!_entrance_client)
          {
             char buf[PATH_MAX];
             ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                                     _entrance_client_del, NULL);
             fprintf(stderr, PACKAGE": Exec entrance_client\n");
             snprintf(buf, sizeof(buf),
                      PACKAGE_BIN_DIR"/entrance_client -d %s -t %s",
                      dname, entrance_config->theme);
             _entrance_client = ecore_exe_run(buf, NULL);
          }
     }
   else
     ecore_main_loop_quit();
   return 0;
}

static Eina_Bool
_entrance_client_del(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Exe_Event_Del *ev;

   ev = event;
   if (ev->exe != _entrance_client)
     return ECORE_CALLBACK_PASS_ON;
   ecore_main_loop_quit();
   _entrance_client = NULL;
   fprintf(stderr, PACKAGE": client have terminated\n");

   return ECORE_CALLBACK_DONE;
}


static const Ecore_Getopt options =
{
   PACKAGE,
   "%prog [options]",
   VERSION,
   "(C) 2011 Enlightenment, see AUTHORS",
   "GPL, see COPYING",
   "Entrance is a login manager, written using core efl libraries",
   EINA_TRUE,
   {
      ECORE_GETOPT_STORE_TRUE('n', "nodaemon", "Don't daemonize."),
      ECORE_GETOPT_STORE_TRUE('t', "test", "run in test mode."),
      ECORE_GETOPT_STORE_TRUE('x', "xephyr", "run in test mode and use Xephyr."),
      ECORE_GETOPT_HELP ('h', "help"),
      ECORE_GETOPT_VERSION('V', "version"),
      ECORE_GETOPT_COPYRIGHT('R', "copyright"),
      ECORE_GETOPT_LICENSE('L', "license"),
      ECORE_GETOPT_SENTINEL
   }
};

int
main (int argc, char ** argv)
{
   int args;
   int pid;
   char *dname = strdup(ENTRANCE_DISPLAY);
   char *entrance_user = NULL;
   unsigned char nodaemon = 0;
   unsigned char quit_option = 0;
   time_t ct;
   struct tm *lc;
   char entrance_time_d[4096];

   Ecore_Getopt_Value values[] =
     {
        ECORE_GETOPT_VALUE_BOOL(nodaemon),
        ECORE_GETOPT_VALUE_BOOL(_testing),
        ECORE_GETOPT_VALUE_BOOL(_xephyr),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_NONE
     };

   args = ecore_getopt_parse(&options, values, argc, argv);
   if (args < 0)
     {
        fprintf(stderr, PACKAGE": ERROR: could not parse options.\n");
        return -1;
     }

   if (quit_option)
     return 0;

   if (getuid() != 0)
     {
        fprintf(stderr, "Only root can run this program\n");
        return 1;
     }

   if (_testing)
     nodaemon = EINA_TRUE;

   eet_init();
   entrance_config_init();
   if (!_testing && !_get_lock())
     {
        exit(1);
     }

   if (!nodaemon && entrance_config->daemonize)
     {
        if (daemon(0, 1) == -1)
          {
             fprintf(stderr, PACKAGE": Error on daemonize !");
             quit_option = EINA_TRUE;
          }
        _update_lock();
        int fd;
        if ((fd = open("/dev/null", O_RDONLY)))
          {
             dup2(fd, 0);
             close(fd);
          }
     }

   if (quit_option)
     {
        entrance_config_shutdown();
        exit(1);
     }
   if (!_open_log())
     {
        fprintf(stderr, PACKAGE": Can't open log file !!!!\n");
        entrance_config_shutdown();
        exit(1);
     }


   entrance_user = getenv("ENTRANCE_USER");
#ifdef HAVE_PAM
   entrance_pam_init(PACKAGE, dname, entrance_user);
#endif
   if (entrance_user)
     {
        char *quit;
        entrance_session_end(entrance_user);
        sleep(2);
        entrance_xserver_end();
        quit = getenv("ENTRANCE_QUIT");
        if (quit)
          {
             unsetenv("ENTRANCE_QUIT");
             _remove_lock();
             entrance_config_shutdown();
             PT("Good bye");
             entrance_close_log();
             exit(0);
          }
        entrance_close_log();
        PT("Nice to see you again.");
        exit(1);
     }
   PT("Welcome");
   ecore_init();
   /* Initialise event handler */

   signal(SIGQUIT, _signal_cb);
   signal(SIGTERM, _signal_cb);
   signal(SIGKILL, _signal_cb);
   signal(SIGINT, _signal_cb);
   signal(SIGHUP, _signal_cb);
   signal(SIGPIPE, _signal_cb);
   signal(SIGALRM, _signal_cb);
   signal(SIGUSR2, _signal_log);

   PT("session init");
   entrance_session_init(entrance_config->command.xauth_file);
   PT("xserver init");
   pid = entrance_xserver_init(_entrance_main, dname);
   PT("history init");
   entrance_history_init();
   if (entrance_config->autologin && !entrance_user)
     {
        PT("autologin init");
        xcb_connection_t *disp = NULL;
        disp = xcb_connect(dname, NULL);
        PT("main loop begin");
        ecore_main_loop_begin();
        PT("auth user");
#ifdef HAVE_PAM
        entrance_pam_item_set(ENTRANCE_PAM_ITEM_USER, entrance_config->userlogin);
#endif
        PT("login user");
        entrance_session_login(entrance_config->command.session_login, EINA_FALSE);
        sleep(30);
        xcb_disconnect(disp);
     }
   else
     {
        PT("action init");
        entrance_action_init();
        PT("server init");
        entrance_server_init();
        PT("starting main loop");
        ecore_main_loop_begin();
        PT("main loop end");
        entrance_server_shutdown();
        PT("server shutdown");
        entrance_action_shutdown();
        PT("action shutdown");
     }
   entrance_history_shutdown();
   PT("history shutdown");
   entrance_xserver_shutdown();
   PT("xserver shutdown");
#ifdef HAVE_PAM
   entrance_pam_shutdown();
   PT("pam shutdown");
#endif
   ecore_shutdown();
   PT("ecore shutdown");
   entrance_config_shutdown();
   PT("config shutdown");
   entrance_session_shutdown();
   PT("session shutdown");
   eet_shutdown();
   PT("eet shutdown");
   free(dname);
   if (entrance_session_logged_get())
     {
        PT("Bye, see you. Now go to code !\n");
        entrance_close_log();
        _entrance_wait();
     }
   PT("ending xserver");
   kill(pid, SIGTERM);
   entrance_xserver_end();
   PT("Bye, see you.\n");
   entrance_close_log();
   return 0;
}

