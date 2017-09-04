#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <ck-connector.h>
#include <dbus/dbus.h>

int
main (int argc, char **argv)
{
   CkConnector *ck = NULL;
   DBusError    error;
   pid_t        pid;
   int          status;

   ck = ck_connector_new();
   if (ck != NULL)
     {
        dbus_error_init(&error);
        if (ck_connector_open_session(ck, &error))
          {
             pid = fork();
             if (pid)
               {
                  waitpid(pid, &status, 0);
                  exit(status);
               }
             else
               setenv("XDG_SESSION_COOKIE",
                       ck_connector_get_cookie(ck), 1);
          }
        else
          fprintf(stderr, "entrance_ck: error connecting to ConsoleKit");
     }
   else
     fprintf(stderr, "entrance_ck: can't set up connection to ConsoleKit");

   if (argc > 1 && argv[1])
     {
       char exe[1024];
       int len;
       len = strlen(argv[1]);
       snprintf(exe,len,"%s",argv[1]);
       execvp(exe, argv + 1);
     }
   _exit (1);
}
