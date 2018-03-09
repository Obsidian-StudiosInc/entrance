#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <Eina.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

static void _entrance_wait_action(int sig, siginfo_t *si, void *data);
static void kill_wait();


static pid_t _x_pid = 0;

static void
_entrance_wait_action(int sig EINA_UNUSED,
                      siginfo_t * si EINA_UNUSED,
                      void *data EINA_UNUSED)
{
    kill_wait();
    setenv("ENTRANCE_QUIT", "1", 1);
}

static void
kill_wait(void)
{
   kill(_x_pid, SIGTERM);
}

int
main (int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   int status = 0;
   char *pid;
   char *sid;
   struct sigaction action;
   pid_t rpid;
   pid_t spid;

   pid = getenv("ENTRANCE_XPID");
   sid = getenv("ENTRANCE_SPID");
   if (!pid) return -1;
   if (!sid) return -1;
   spid = atoi(sid);
   _x_pid = atoi(pid);
   printf("waiting\n");

   action.sa_sigaction = _entrance_wait_action;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGQUIT, &action, NULL);
   sigaction(SIGTERM, &action, NULL);
   sigaction(SIGKILL, &action, NULL);
   sigaction(SIGINT, &action, NULL);
   sigaction(SIGHUP, &action, NULL);
   sigaction(SIGPIPE, &action, NULL);
   sigaction(SIGALRM, &action, NULL);

   while ((rpid = wait(&status)))
     {
        if (rpid == -1)
          {
             if ((errno == ECHILD) || (errno == EINVAL))
               return -1;
          }
        else if (rpid == _x_pid)
          {
             break;
          }
        else if (rpid == spid)
          {
             kill_wait();
          }
     }

   if (WIFEXITED(status) && WEXITSTATUS(status))
     setenv("ENTRANCE_QUIT", "1", 1);
   execlp(PACKAGE_SBIN_DIR"/entrance", PACKAGE_SBIN_DIR"/entrance", "--nodaemon", NULL);

   return -1;
}

