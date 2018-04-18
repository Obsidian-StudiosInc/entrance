#include "entrance.h"
#include <sys/file.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <Eina.h>
#include "Ecore_Getopt.h"
#include <xcb/xcb.h>
#include <assert.h>

#define ENTRANCE_DISPLAY ":0.0"
#define ENTRANCE_XEPHYR ":1.0"
#define ENTRANCE_CONFIG_HOME_PATH "/var/cache/entrance/client"
#define ENTRANCE_USER_MAX 33

static Eina_Bool _entrance_autologin_lock_get(void);
static Eina_Bool _entrance_client_error(void *data, int type, void *event);
static Eina_Bool _entrance_client_data(void *data, int type, void *event);
static Eina_Bool _entrance_client_del(void *data, int type, void *event);
static Eina_Bool _open_log();
static void _entrance_autologin_lock_set(void);
static void _entrance_start(const char *dname);
static void _entrance_uid_gid_init();
static void _entrance_wait(void);
static void _remove_lock();
static void _signal_cb(int sig);
static void _signal_log(int sig);

static Eina_Bool _xephyr = 0;
static Ecore_Exe *_entrance_client = NULL;

static char *entrance_home_path = NULL;
static const char *entrance_user = NULL;
static pid_t entrance_client_pid = 0;
static gid_t entrance_gid = 0;
static uid_t entrance_uid = 0;

static const Ecore_Getopt options =
{
  PACKAGE,
  "%prog [options]",
  VERSION,
  "(C) 201e Enlightenment, see AUTHORS",
  "GPL, see COPYING",
  "Entrance is a login manager, written using core efl libraries",
  EINA_TRUE,
  {
    ECORE_GETOPT_STORE_TRUE('n', "nodaemon", "Don't daemonize."),
    ECORE_GETOPT_STORE_TRUE('x', "xephyr", "run under Xephyr."),
    ECORE_GETOPT_HELP ('h', "help"),
    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_COPYRIGHT('R', "copyright"),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_SENTINEL
  }
};


static Eina_Bool
_entrance_autologin_lock_get(void)
{
   FILE *f;
   char buf[4096];
   double sleep_time;
   double uptime;
   struct stat st_login;

   f = fopen("/proc/uptime", "r");
   if (f)
     {
        fgets(buf,  sizeof(buf), f);
        if(fscanf(f, "%lf %lf", &uptime, &sleep_time) <= 0)
          PT("Could not read uptime input stream");
        fclose(f);
        if (stat("/var/run/entrance/login", &st_login) > 0)
           return EINA_FALSE;
        else
           return EINA_TRUE;
     }
   return EINA_FALSE;
}

static void
_entrance_autologin_lock_set(void)
{
   system("touch /var/cache/entrance/login");
}

static Eina_Bool
_entrance_client_data(void *d EINA_UNUSED, int t EINA_UNUSED, void *event)
{
   char buf[4096];
   Ecore_Exe_Event_Data *ev;
   size_t size;

   ev = event;
   size = ev->size;


   if ((unsigned int)ev->size > sizeof(buf) - 1)
     size = sizeof(buf) - 1;

   snprintf(buf, size + 1, "%s", (char*)ev->data);
   EINA_LOG_DOM_INFO(_entrance_client_log, "%s", buf);
   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_entrance_client_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Exe_Event_Del *ev;

   ev = event;
   if (ev->exe != _entrance_client)
     {
       PT("received pid %d, kill %d", ev->pid, entrance_client_pid);
       if(!kill(entrance_client_pid,SIGTERM))
         {
           PT("escalating to kill -9 %d", entrance_client_pid);
           kill(entrance_client_pid,SIGKILL);
         }
       return ECORE_CALLBACK_PASS_ON;
     }
   PT("client terminated");
   ecore_main_loop_quit();
   _entrance_client = NULL;

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_entrance_client_error(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   char buf[4096];
   Ecore_Exe_Event_Data *ev;
   size_t size;

   ev = event;
   size = ev->size;

   if ((unsigned int)ev->size > sizeof(buf) - 1)
     size = sizeof(buf) - 1;

   strncpy(buf, (char*)ev->data, size);
   EINA_LOG_DOM_ERR(_entrance_client_log, "%s", buf);
   return ECORE_CALLBACK_DONE;
}

static void
_entrance_start(const char *dname)
{
   char buf[PATH_MAX];
   char *home_path = ENTRANCE_CONFIG_HOME_PATH;
   int home_dir;
   struct stat st;

   PT("starting...");
   if (entrance_config->autologin)
     {
       ecore_main_loop_quit();
       return;
     }
   if (_entrance_client)
     return;
   ecore_event_handler_add(ECORE_EXE_EVENT_DEL, _entrance_client_del, NULL);
   ecore_event_handler_add(ECORE_EXE_EVENT_ERROR, _entrance_client_error, NULL);
   ecore_event_handler_add(ECORE_EXE_EVENT_DATA, _entrance_client_data, NULL);
   if(entrance_home_path)
       home_path = entrance_home_path;
   home_dir = open(home_path, O_RDONLY);
   if(!home_dir || home_dir<0)
     {
       PT("Failed to open home directory %s", home_path);
       ecore_main_loop_quit();
       return;
     }
   if(flock(home_dir, LOCK_SH)==-1)
     {
       PT("Failed to lock home directory %s", home_path);
       close(home_dir);
       ecore_main_loop_quit();
       return;
     }
   if(fstat(home_dir, &st)!= -1)
     {
       if ((st.st_uid != entrance_uid)
           || (st.st_gid != entrance_gid))
         {
            PT("chown home directory %s", home_path);
            fchown(home_dir, entrance_uid, entrance_gid);
         }
       snprintf(buf, sizeof(buf),
                "export HOME=%s; export USER=%s;"
                "export LD_LIBRARY_PATH="PACKAGE_LIB_DIR";%s "
                PACKAGE_BIN_DIR"/entrance_client -d %s -t %s -g %d -u %d",
                home_path, entrance_user, entrance_config->command.session_login,
                dname, entrance_config->theme,
                entrance_gid,entrance_uid);
       PT("Exec entrance_client: %s", buf);

       _entrance_client =
          ecore_exe_pipe_run(buf,
                             ECORE_EXE_PIPE_READ | ECORE_EXE_PIPE_ERROR,
                             NULL);
       if(_entrance_client)
         {
           entrance_client_pid = ecore_exe_pid_get(_entrance_client);
           PT("entrance_client started pid %d", entrance_client_pid);
         }
     }
   flock(home_dir, LOCK_UN);
   close(home_dir);
}

static void
_entrance_uid_gid_init()
{
   struct passwd *pwd = NULL;

   if (entrance_config->start_user
       && entrance_config->start_user[0])
     pwd = getpwnam(entrance_config->start_user);
   if (!pwd)
     {
       PT("The given user %s, is not valid."
          "Falling back to nobody", entrance_config->start_user);
        pwd = getpwnam("nobody");
        entrance_user = "nobody";
        assert(pwd);
     }
   else
     entrance_user = entrance_config->start_user;
   PT("running under user : %s",entrance_user);
   entrance_gid = pwd->pw_gid;
   entrance_uid = pwd->pw_uid;
   if (!pwd->pw_dir ||
       !strcmp(pwd->pw_dir, "/") ||
       !strcmp(pwd->pw_dir, "/nonexistent"))
     {
        PT("No home directory for client");
        if (!ecore_file_exists(ENTRANCE_CONFIG_HOME_PATH))
          {
             PT("Creating new home directory for client");
             ecore_file_mkdir(ENTRANCE_CONFIG_HOME_PATH);
             chown(ENTRANCE_CONFIG_HOME_PATH, entrance_uid, entrance_gid);
          }
        else
          {
             if (!ecore_file_is_dir(ENTRANCE_CONFIG_HOME_PATH))
               {
                  PT("Hum a file already exists here "
                     ENTRANCE_CONFIG_HOME_PATH" sorry but"
                     "I remove it, I need it ^^");
                  ecore_file_remove(ENTRANCE_CONFIG_HOME_PATH);
                  ecore_file_mkdir(ENTRANCE_CONFIG_HOME_PATH);
                  chown(ENTRANCE_CONFIG_HOME_PATH, entrance_uid, entrance_gid);
               }
          }
     }
   else
     entrance_home_path = strdup(pwd->pw_dir);
   PT("Home directory %s", entrance_home_path);
}

static void
_entrance_wait(void)
{
   // XXX: use eina_prefix! hardcoding paths . :(
   execl(PACKAGE_BIN_DIR"/entrance_wait", PACKAGE_SBIN_DIR"/entrance", NULL);
   PT("HUM HUM HUM can't wait ...");
   _exit(1);
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
             PT("Couldn't create lockfile!");
             return (EINA_FALSE);
          }
        snprintf(buf, sizeof(buf), "%d", my_pid);
        if (!fwrite(buf, strlen(buf), 1, f))
          {
             fclose(f);
             PT("Couldn't write the lockfile");
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

        PT("A lock file are present another instance are present ?");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_open_log()
{
   FILE *elog;
   elog = fopen(entrance_config->logfile, "a");
   if (!elog)
     {
        PT("could not open logfile !");
        return EINA_FALSE;
     }
   fclose(elog);
   if (!freopen(entrance_config->logfile, "a", stdout))
     PT("Error on reopen stdout");
   setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
   if (!freopen(entrance_config->logfile, "a", stderr))
     PT("Error on reopen stderr");
   setvbuf(stderr, NULL, _IONBF, BUFSIZ);
   return EINA_TRUE;
}

static void
_remove_lock()
{
   if(remove(entrance_config->lockfile)== -1)
     PT("Could not remove lockfile");
}

static void
_signal_cb(int sig)
{
   PT("signal %d received", sig);
   if (_entrance_client)
     {
       PT("terminate client");
       kill(entrance_client_pid,SIGTERM);
     }
   else
     ecore_main_loop_quit();
}

static void
_signal_log(int sig EINA_UNUSED)
{
   PT("reopen the log file");
   entrance_close_log();
   _open_log();
}

static void
_update_lock()
{
   FILE *f;
   char buf[128];
   f = fopen(entrance_config->lockfile, "w");
   if(!f)
     {
       PT("Could not open lockfile");
       return;
     }
   snprintf(buf, sizeof(buf), "%d", getpid());
   if (!fwrite(buf, strlen(buf), 1, f))
     PT("Could not update lockfile");
   fclose(f);
}

void
entrance_close_log()
{
  fclose(stderr);
  fclose(stdout);
}

int
main (int argc, char ** argv)
{
   int args;
   int pid = -1;
   char *dname;
   unsigned char nodaemon = 0;
   unsigned char quit_option = 0;

   Ecore_Getopt_Value values[] =
     {
        ECORE_GETOPT_VALUE_BOOL(nodaemon),
        ECORE_GETOPT_VALUE_BOOL(_xephyr),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_NONE
     };

   if (getuid() != 0)
     {
        fprintf(stderr, "Sorry, only root can run this program!");
        return 1;
     }
   
   args = ecore_getopt_parse(&options, values, argc, argv);
   if (args < 0)
     {
        fprintf(stderr, "ERROR: could not parse options.");
        return -1;
     }

   if (quit_option)
     return 0;

   eina_init();
   eina_log_threads_enable();
   ecore_init();
   _entrance_log = eina_log_domain_register("entrance", EINA_COLOR_CYAN);
   _entrance_client_log = eina_log_domain_register("entrance_client",
                                                   EINA_COLOR_CYAN);
   eina_log_domain_level_set("entrance", 5);
   eina_log_domain_level_set("entrance_client", 5);

   if (!_xephyr && getenv("ENTRANCE_XEPHYR"))
     _xephyr = EINA_TRUE;

   if (_xephyr)
     {
        nodaemon = EINA_TRUE;
        dname = strdup(ENTRANCE_XEPHYR);
        putenv(strdup("ENTRANCE_XEPHYR=1"));
     }
   else
     dname = strdup(ENTRANCE_DISPLAY);

   eet_init();
   efreet_init();
   entrance_config_init();
   if (!entrance_config)
     {
        PT("No config loaded, sorry must quit ...");
        exit(1);
     }
   if (!_get_lock())
        exit(1);

   if (!nodaemon && entrance_config->daemonize)
     {
        if (daemon(0, 1) == -1)
          {
             PT("Error on daemonize !");
             entrance_config_shutdown();
             exit(1);
          }
        _update_lock();
        int fd;
        if ((fd = open("/dev/null", O_RDONLY))>0)
          {
             dup2(fd, 0);
             close(fd);
          }
     }

   if (!_open_log())
     {
        PT("Can't open log file !!!!");
        entrance_config_shutdown();
        exit(1);
     }

   entrance_user = getenv("ENTRANCE_USER");
   if (entrance_user)
     {
        char *user = NULL;
        user = strndup(entrance_user,ENTRANCE_USER_MAX);
        if(user)
          {
            entrance_xserver_wait();
            entrance_session_init(dname);
            entrance_session_end(user);
            free(user);
          }
     }
   PT("Welcome");
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
   entrance_session_init(dname);
   entrance_session_cookie();

   if(!entrance_config->autologin)
     _entrance_uid_gid_init();

   if (!_xephyr)
     {
        PT("xserver init");
        pid = entrance_xserver_init(_entrance_start, dname);
     }
   else
     {
        putenv(strdup("ENTRANCE_XPID=-1"));
        _entrance_start(dname);
     }

   PT("history init");
   entrance_history_init();
   if ((entrance_config->autologin) && _entrance_autologin_lock_get())
     {
        PT("autologin init");
        xcb_connection_t *disp = NULL;
        disp = xcb_connect(dname, NULL);
        PT("main loop begin");
        ecore_main_loop_begin();
        PT("auth user");
#ifdef HAVE_PAM
        entrance_pam_init(PACKAGE, dname, entrance_config->userlogin);
#endif
        PT("login user");
        entrance_session_login(
           entrance_history_user_session_get(entrance_config->userlogin),
           EINA_FALSE);
        sleep(30);
        xcb_disconnect(disp);
     }
   else
     {
        PT("action init");
        entrance_action_init();
        PT("server init");
        entrance_server_init(entrance_uid, entrance_gid);
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
   if (_xephyr)
     {
        //ecore_exe_terminate(xephyr);
        PT("Xephyr shutdown");
     }
   else
     {
        entrance_xserver_shutdown();
        PT("xserver shutdown");
     }
#ifdef HAVE_PAM
   entrance_pam_shutdown();
   PT("pam shutdown");
#endif
   _entrance_autologin_lock_set();
   PT("ecore shutdown");
   ecore_shutdown();
   PT("session shutdown");
   entrance_session_shutdown();
   if (entrance_session_logged_get())
     {
        PT("user logged, waiting...");
        _entrance_wait();
        /* no more running here */
     }
   _remove_lock();
   PT("config shutdown");
   entrance_config_shutdown();
   PT("eet shutdown");
   eet_shutdown();
   if(entrance_home_path)
     free(entrance_home_path);
   free(dname);
   if (!_xephyr)
     {
        PT("ending xserver");
        kill(pid, SIGTERM);
        entrance_xserver_end();
        entrance_xserver_wait();
     }
   else
     PT("No session to wait, exiting");
   PT("close log and exit");
   entrance_close_log();
   efreet_shutdown();
   return 0;
}

