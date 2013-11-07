#ifndef ENTRANCE_CONFIG_H_
#define ENTRANCE_CONFIG_H_

#define ENTRANCE_CONFIG_FILE        "entrance.cfg"

typedef struct _Entrance_Config Entrance_Config;

typedef enum
{
   ENTRANCE_SESSION_DESKTOP_NONE = 0,
   ENTRANCE_SESSION_DESKTOP_FILE_CMD = 1,
   ENTRANCE_SESSION_DESKTOP_FILE_CMD_ARGS = 2
} Entrance_Session_Type;

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
        const char *session_stop;
        const char *shutdown;
        const char *reboot;
        const char *suspend;
     } command;
   const char *userlogin;
   const char *lockfile;
   const char *logfile;
   const char *theme;
   const char *elm_profile;
   struct
     {
        const char *path;
        const char *group;
     } bg;
   unsigned char xsessions;
   Eina_Bool daemonize;
   Eina_Bool numlock;
   Eina_Bool autologin;
   Eina_Bool custom_conf;
   Eina_Bool vkbd_enabled;
};

void entrance_config_init();
void entrance_config_shutdown();
void entrance_config_set(const Entrance_Conf_Gui_Event *conf);

Entrance_Config *entrance_config;

#endif /* ENTRANCE_CONFIG_H_ */
