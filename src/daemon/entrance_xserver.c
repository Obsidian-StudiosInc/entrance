#include "entrance.h"
#include <sys/wait.h>
#include <unistd.h>

typedef struct Entrance_Xserver_
{
   const char *dname;
   Entrance_X_Cb start;
} Entrance_Xserver;

Entrance_Xserver *_xserver;
Ecore_Event_Handler *_handler_start;

/*
 * man Xserver
 * SIGUSR1 This  signal  is  used  quite  differently  from  either of the
 * above.  When the server starts, it checks to see if it has inherite
 * SIGUSR1 as SIG_IGN instead of the usual SIG_DFL.  In this case, the server
 * sends a SIGUSR1 to its parent process after it has set up the various
 * connection schemes.  Xdm uses this feature to recognize when connecting to
 * the server is possible.
 * */
static void
_env_set(const char *dname)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "DISPLAY=%s", dname);
   putenv(strdup(buf));
}

static int
_xserver_start(void)
{
   char *abuf = NULL;
   char *buf = NULL;
   char **args = NULL;
   char *saveptr = NULL;
   pid_t pid;

   PT("Launching xserver");
   pid = fork();
   if (!pid)
     {
        char *token;
        int num_token = 0;
        entrance_close_log();
        signal(SIGTTIN, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGUSR1, SIG_IGN);

        if (!(buf = strdup(entrance_config->command.xinit_args)))
          goto xserver_error;
        token = strtok_r(buf, " ", &saveptr);
        while(token)
          {
            ++num_token;
            token = strtok_r(NULL, " ", &saveptr);
          }
        if (buf) free(buf);
        if (num_token)
          {
             int i;
             if (!(abuf = strdup(entrance_config->command.xinit_args)))
               goto xserver_error;
             if (!(args = calloc(num_token + 2, sizeof(char *))))
               {
                  if (abuf) free(abuf);
                  goto xserver_error;
               }
             args[0] = (char *)entrance_config->command.xinit_path;
             token = strtok_r(abuf, " ", &saveptr);
             ++num_token;
             for(i = 1; i < num_token; ++i)
               {
                  if (token)
                    args[i] = token;
                  token = strtok_r(NULL, " ", &saveptr);
               }
             args[num_token] = NULL;
          }
        else
          {
             if (!(args = calloc(2, sizeof(char*))))
               goto xserver_error;
             args[0] = (char *)entrance_config->command.xinit_path;
             args[1] = NULL;
          }
        execv(args[0], args);
        if (abuf) free(abuf);
        free(args);
        PT("Couldn't launch Xserver ...");
     }
   return pid;
xserver_error:
   _exit(EXIT_FAILURE);
}

static Eina_Bool
_xserver_started(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   PT("xserver started");
   _env_set(_xserver->dname);
   _xserver->start(_xserver->dname);
   return ECORE_CALLBACK_PASS_ON;
}

int
entrance_xserver_init(Entrance_X_Cb start, const char *dname)
{
   int pid;
   char buf[64];
   sigset_t newset;
   sigemptyset(&newset);

   _xserver = calloc(1, sizeof(Entrance_Xserver));
   _xserver->dname = eina_stringshare_add(dname);
   _xserver->start = start;
   pid = _xserver_start();
   snprintf(buf, sizeof(buf), "ENTRANCE_XPID=%d", pid);
   putenv(strdup(buf));
   PT("xserver adding signal user handler");
   _handler_start = ecore_event_handler_add(ECORE_EVENT_SIGNAL_USER,
                                            _xserver_started,
                                            NULL);
   return pid;
}

void
entrance_xserver_wait(void)
{
   const char *xpid;
   int pid;

   PT("xserver end");
   xpid = getenv("ENTRANCE_XPID");
   if (xpid)
     {
        pid = atoi(xpid);
        while (waitpid(pid, NULL, WUNTRACED | WCONTINUED) > 0)
          {
             PT("Waiting ...");
             sleep(1);
          }
        unsetenv("ENTRANCE_XPID");
     }
}

void
entrance_xserver_end(void)
{
   const char *xpid;
   PT("xserver end");
   xpid = getenv("ENTRANCE_XPID");
   if (xpid)
     kill(atoi(xpid), SIGTERM);

}

void
entrance_xserver_shutdown(void)
{
   eina_stringshare_del(_xserver->dname);
   free(_xserver);
   ecore_event_handler_del(_handler_start);
}

