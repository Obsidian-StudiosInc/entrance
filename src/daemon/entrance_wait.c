#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

static void _entrance_wait_action(int sig, siginfo_t *si, void *data);
static void kill_wait();


static pid_t _x_pid = 0;

static void
_entrance_wait_action(int sig, siginfo_t * si __UNUSED__, void *data __UNUSED__)
{
    kill_wait();
    if (sig != SIGCHLD)
      setenv("ENTRANCE_QUIT", "1", 1);
}

static void
kill_wait(void)
{
   kill(_x_pid, SIGTERM);
}

int
main (int argc __UNUSED__, char **argv __UNUSED__)
{
   int status = 0;
   char *pid, *exit_policy;
   struct sigaction action;

   pid_t rpid;
   pid = getenv("ENTRANCE_XPID");
   if (!pid) return -1;
   _x_pid = atoi(pid);
   
   exit_policy = getenv("ENTRANCE_FAST_QUIT");

   action.sa_sigaction = _entrance_wait_action;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGCHLD, &action, NULL);
   sigaction(SIGQUIT, &action, NULL);
   sigaction(SIGTERM, &action, NULL);
   sigaction(SIGKILL, &action, NULL);
   sigaction(SIGINT, &action, NULL);
   sigaction(SIGHUP, &action, NULL);
   sigaction(SIGPIPE, &action, NULL);
   sigaction(SIGALRM, &action, NULL);

   while ((rpid = wait(&status)) != _x_pid)
     {
        if (rpid == -1)
          {
             if ((errno == ECHILD) || (errno == EINVAL))
               break;
          }
     }
   if (_x_pid == rpid)
     {
        if (exit_policy)
          {
             setenv("ENTRANCE_QUIT", "1", 1);
          }
        else
          {
             if ( WIFEXITED(status) && WEXITSTATUS(status))
               {
                  setenv("ENTRANCE_QUIT", "1", 1);
               }
          }
        execlp(PACKAGE_SBIN_DIR"/entrance", PACKAGE_SBIN_DIR"/entrance", "--nodaemon", NULL);
     }
   return -1;
}

