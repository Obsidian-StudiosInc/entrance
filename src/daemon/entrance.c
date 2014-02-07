#include "entrance.h"
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "Ecore_Getopt.h"
#include <xcb/xcb.h>

#define ENTRANCE_DISPLAY ":0.0"
#define ENTRANCE_XEPHYR ":1.0"
time_t current_time;
struct tm *local_time;
char entrance_time_d[4096];

static Eina_Bool _open_log();
static Eina_Bool _entrance_main(const char *dname);
static void _remove_lock();
static void _signal_cb(int sig);
static void _signal_log(int sig);
static Eina_Bool _entrance_client_del(void *data, int type, void *event);

static Eina_Bool _testing = 0;
static Eina_Bool _xephyr = 0;
static Ecore_Exe *_entrance_client = NULL;



static void
_signal_cb(int sig)
{
   PT("signal %d received\n", sig);
   //FIXME  if I don't have main loop at this time ?
   if (_entrance_client)
     ecore_exe_terminate(_entrance_client);
   else
     ecore_main_loop_quit();
}

static void
_signal_log(int sig EINA_UNUSED)
{
   PT("reopen the log file\n");
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
             PT("Couldn't create lockfile!\n");
             return (EINA_FALSE);
          }
        snprintf(buf, sizeof(buf), "%d\n", my_pid);
        if (!fwrite(buf, strlen(buf), 1, f))
          {
             fclose(f);
             PT("Couldn't write the lockfile\n");
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

        PT("A lock file are present another instance are present ?\n");
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
     PT("Coudn't update lockfile\n");
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
        PT("could not open logfile !\n");
        return EINA_FALSE;
     }
   fclose(elog);
   if (!freopen(entrance_config->logfile, "a", stdout))
     PT("Error on reopen stdout\n");
   setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
   if (!freopen(entrance_config->logfile, "a", stderr))
     PT("Error on reopen stderr\n");
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
_entrance_wait(void)
{
   // XXX: use eina_prefix! hardcoding paths . :(
   execl(PACKAGE_BIN_DIR"/entrance_wait", PACKAGE_SBIN_DIR"/entrance", NULL);
   PT("HUM HUM HUM can't wait ...\n\n\n");
   _exit(1);
}

static Eina_Bool
_entrance_main(const char *dname)
{
   PT("starting...\n");
   if (!entrance_config->autologin)
     {
        if (!_entrance_client)
          {
             char buf[PATH_MAX];
             ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                                     _entrance_client_del, NULL);
             snprintf(buf, sizeof(buf),
                      PACKAGE_BIN_DIR"/entrance_client -d %s -t %s",
                      dname, entrance_config->theme);
             PT("Exec entrance_client: %s\n", buf);

             _entrance_client = ecore_exe_run(buf, NULL);
          }
     }
   else
     ecore_main_loop_quit();
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_entrance_client_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Exe_Event_Del *ev;

   ev = event;
   if (ev->exe != _entrance_client)
     return ECORE_CALLBACK_PASS_ON;
   PT("client have terminated\n");
   ecore_main_loop_quit();
   _entrance_client = NULL;

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
      ECORE_GETOPT_STORE_TRUE('e', "fastexit", "Will change the way entrance \
handles the exit of the created session.\n If set, entrance will exit if the session \
quits.\n If not, entrance will restart if the session is quit because of an error, \
or if the environment variable ENTRANCE_RESTART is set."),
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
   int pid = -1;
   char *dname;
   char *entrance_user = NULL;
   unsigned char nodaemon = 0;
   unsigned char fastexit = 0;
   unsigned char quit_option = 0;

   Ecore_Getopt_Value values[] =
     {
        ECORE_GETOPT_VALUE_BOOL(nodaemon),
        ECORE_GETOPT_VALUE_BOOL(_testing),
        ECORE_GETOPT_VALUE_BOOL(fastexit),
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
        PT("ERROR: could not parse options.\n");
        return -1;
     }

   if (quit_option)
     return 0;

   if (getuid() != 0)
     {
        fprintf(stderr, "Sorry, only root can run this program!\n");
        return 1;
     }
   if (!_xephyr && getenv("ENTRANCE_XEPHYR"))
     _xephyr = EINA_TRUE;

   if (fastexit)
     {
        putenv(strdup("ENTRANCE_FAST_QUIT=1"));
        PT("Fast exit enabled !\n");
     }

   if (_xephyr)
     {
        _testing = EINA_TRUE;
        dname = strdup(ENTRANCE_XEPHYR);
        putenv(strdup("ENTRANCE_XEPHYR=1"));
     }
   else
     dname = strdup(ENTRANCE_DISPLAY);
   if (!_testing && getenv("ENTRANCE_TESTING"))
     _testing = EINA_TRUE;

   if (_testing)
     {
        putenv(strdup("ENTRANCE_TESTING=1"));
        nodaemon = EINA_TRUE;
     }

   eet_init();
   entrance_config_init();
   if (!entrance_config)
     {
        PT("No config loaded, sorry must quit ...");
        exit(1);
     }
   if (!_testing && !_get_lock())
     {
        exit(1);
     }

   if (!nodaemon && entrance_config->daemonize)
     {
        if (daemon(0, 1) == -1)
          {
             PT("Error on daemonize !\n");
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
   if (!_testing && !_open_log())
     {
        PT("Can't open log file !!!!\n");
        entrance_config_shutdown();
        exit(1);
     }


   entrance_user = getenv("ENTRANCE_USER");
   if (entrance_user)
     {
        char *quit;
        entrance_session_init(dname);
        entrance_session_end(entrance_user);
        entrance_session_shutdown();
        quit = getenv("ENTRANCE_QUIT");
        if (quit)
          {
             unsetenv("ENTRANCE_QUIT");
             PT("Last DE Session quit with error!\n");
          }
        PT("ending xserver\n");
        entrance_xserver_end();
        _remove_lock();
        PT("Entrance will quit, bye bye :).\n");
        entrance_close_log();
        exit(1);
     }
   PT("Welcome\n");
   ecore_init();
   efreet_init();
   /* Initialise event handler */

   signal(SIGQUIT, _signal_cb);
   signal(SIGTERM, _signal_cb);
   signal(SIGKILL, _signal_cb);
   signal(SIGINT, _signal_cb);
   signal(SIGHUP, _signal_cb);
   signal(SIGPIPE, _signal_cb);
   signal(SIGALRM, _signal_cb);
   signal(SIGUSR2, _signal_log);

   PT("session init\n");
   entrance_session_init(dname);
   entrance_session_cookie();
   if (!_xephyr)
     {
        PT("xserver init\n");
        pid = entrance_xserver_init(_entrance_main, dname);
     }
   else
     _entrance_main(dname);
   PT("history init\n");
   entrance_history_init();
   if (entrance_config->autologin && !entrance_user)
     {
        PT("autologin init\n");
        xcb_connection_t *disp = NULL;
        disp = xcb_connect(dname, NULL);
        PT("main loop begin\n");
        ecore_main_loop_begin();
        PT("auth user\n");
#ifdef HAVE_PAM
        entrance_pam_item_set(ENTRANCE_PAM_ITEM_USER, entrance_config->userlogin);
#endif
        PT("login user\n");
        entrance_session_login(NULL, EINA_FALSE);
        sleep(30);
        xcb_disconnect(disp);
     }
   else
     {
        PT("action init\n");
        entrance_action_init();
        PT("server init\n");
        entrance_server_init();
        PT("starting main loop\n");
        ecore_main_loop_begin();
        PT("main loop end\n");
        entrance_server_shutdown();
        PT("server shutdown\n");
        entrance_action_shutdown();
        PT("action shutdown\n");
     }
   entrance_history_shutdown();
   PT("history shutdown\n");
   if (_xephyr)
     {
        //ecore_exe_terminate(xephyr);
        PT("Xephyr shutdown\n");
     }
   else
     {
        entrance_xserver_shutdown();
        PT("xserver shutdown\n");
     }
#ifdef HAVE_PAM
   entrance_pam_shutdown();
   PT("pam shutdown\n");
#endif
   efreet_shutdown();
   PT("ecore shutdown\n");
   ecore_shutdown();
   PT("session shutdown\n");
   entrance_session_shutdown();
   free(dname);
   if (entrance_session_logged_get())
     {
        PT("user logged, waiting...\n");
        _entrance_wait();
        /* no more running here */
     }
   _remove_lock();
   PT("config shutdown\n");
   entrance_config_shutdown();
   PT("eet shutdown\n");
   eet_shutdown();
   free(dname);
   if (!_xephyr)
     {
        PT("ending xserver\n");
        kill(pid, SIGTERM);
        entrance_xserver_end();
     }
   else
     PT("No session to wait, exiting\n");
   entrance_close_log();
   return 0;
}

