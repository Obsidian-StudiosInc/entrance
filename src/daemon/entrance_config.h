#ifndef ENTRANCE_CONFIG_H_
#define ENTRANCE_CONFIG_H_

#define ENTRANCE_CONFIG_FILE        "entrance.cfg"

typedef struct _Entrance_Config Entrance_Config;

struct _Entrance_Config
{
   const char *session_path;
   struct
     {
        const char *xinit_path;
        const char *xinit_args;
        const char *xauth_path;
        const char *xauth_file;
        const char *session_start;
        const char *session_login;
        const char *session_stop;
        const char *shutdown;
        const char *reboot;
        const char *suspend;
     } command;
   Eina_Bool daemonize;// :1;
   Eina_Bool numlock;// :1;
   Eina_Bool xsessions;
   Eina_Bool autologin;
   const char *userlogin;
   const char *lockfile;
   const char *logfile;
   const char *theme;
};

void entrance_config_init();
void entrance_config_shutdown();

Entrance_Config *entrance_config;

#endif /* ENTRANCE_CONFIG_H_ */
