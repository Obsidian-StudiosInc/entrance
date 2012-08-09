#include "entrance.h"
#include <sys/stat.h>

#define ENTRANCE_CONFIG_KEY "config"

static void _defaults_set(Entrance_Config *config);
static void _users_get();
static void _config_free(Entrance_Config *config);
static Entrance_Config *_cache_get(Eet_Data_Descriptor *edd);

static void
_defaults_set(Entrance_Config *config)
{
   config->session_path = eina_stringshare_add("/bin:/usr/bin:/usr/local/bin");
   config->command.xinit_path = eina_stringshare_add("/usr/bin/X");
   config->command.xinit_args = eina_stringshare_add("-nolisten tcp -br vt7");
   config->command.xauth_path = eina_stringshare_add("/usr/bin/xauth");
   config->command.xauth_file = eina_stringshare_add("/var/run/entrance.auth");
   config->command.session_start = eina_stringshare_add("/usr/bin/sessreg -a -l :0.0" );
   config->command.session_login = eina_stringshare_add("exec /bin/bash -login /etc/X11/xinit/xinitrc");
   config->command.session_stop = eina_stringshare_add("/usr/bin/sessreg -d -l :0.0");
   config->command.shutdown = eina_stringshare_add("/usr/bin/shutdown -h now");
   config->command.reboot = eina_stringshare_add("/usr/bin/shutdown -r now");
   config->command.suspend = eina_stringshare_add("/usr/sbin/suspend");
   config->daemonize = EINA_TRUE;
   config->numlock = EINA_FALSE;
   config->xsessions = EINA_FALSE;
   config->autologin = EINA_FALSE;
   config->userlogin = eina_stringshare_add("mylogintouse");
   config->lockfile = eina_stringshare_add("/var/run/entrance.pid");
   config->logfile = eina_stringshare_add("/var/log/entrance.log");
   config->theme = eina_stringshare_add("default");
}


static void
_users_get()
{
   Eet_File *ef;
   FILE *f;
   int textlen;
   char *text;

   if (!ecore_file_is_dir("/var/cache/"PACKAGE))
     ecore_file_mkdir("/var/cache/"PACKAGE);
   ef = eet_open("/var/cache/"PACKAGE"/"ENTRANCE_CONFIG_FILE,
                 EET_FILE_MODE_READ_WRITE);
   if (!ef)
     ef = eet_open("/var/cache/"PACKAGE"/"ENTRANCE_CONFIG_FILE,
                   EET_FILE_MODE_WRITE);
   f = fopen(SYSTEM_CONFIG_DIR"/entrance.conf", "rb");
   if (!f)
     {
        fprintf(stderr,
                PACKAGE": Could not open "SYSTEM_CONFIG_DIR"/entrance.conf\n");
        return;
     }

   fseek(f, 0, SEEK_END);
   textlen = ftell(f);
   rewind(f);
   text = (char *)malloc(textlen);
   if (!text)
     {
        fclose(f);
        eet_close(ef);
        return;
     }

   if (fread(text, textlen, 1, f) != 1)
     {
        free(text);
        fclose(f);
        eet_close(ef);
        return;
     }

   fclose(f);
   if (eet_data_undump(ef, ENTRANCE_CONFIG_KEY, text, textlen, 1))
     fprintf(stderr, PACKAGE": Updating configuration\n");
   free(text);
   eet_close(ef);
}

static Entrance_Config *
_cache_get(Eet_Data_Descriptor *edd)
{
   Entrance_Config *config = NULL;
   Eet_File *file;

   if (!ecore_file_is_dir("/var/cache/"PACKAGE))
     ecore_file_mkdir("/var/cache/"PACKAGE);
   file = eet_open("/var/cache/"PACKAGE"/"ENTRANCE_CONFIG_FILE,
                   EET_FILE_MODE_READ);

   config = eet_data_read(file, edd, ENTRANCE_CONFIG_KEY);
   if (!config)
     {
        fprintf(stderr, PACKAGE": Warning no configuration found! This must \
                not append, we will go back to default configuration\n");
        config = (Entrance_Config *) calloc(1, sizeof(Entrance_Config));
        _defaults_set(config);
     }

   eet_close(file);

   return config;
}

static void
_config_free(Entrance_Config *config)
{
   eina_stringshare_del(config->session_path);
   eina_stringshare_del(config->command.xinit_path);
   eina_stringshare_del(config->command.xinit_args);
   eina_stringshare_del(config->command.xauth_path);
   eina_stringshare_del(config->command.xauth_file);
   eina_stringshare_del(config->command.session_start);
   eina_stringshare_del(config->command.session_login);
   eina_stringshare_del(config->command.session_stop);
   eina_stringshare_del(config->command.shutdown);
   eina_stringshare_del(config->command.reboot);
   eina_stringshare_del(config->command.suspend);
   eina_stringshare_del(config->userlogin);
   eina_stringshare_del(config->lockfile);
   eina_stringshare_del(config->logfile);
   eina_stringshare_del(config->theme);
   free(config);
}

void
entrance_config_init()
{
   Eet_Data_Descriptor *edd;
   Eet_Data_Descriptor_Class eddc;
   struct stat cache;
   struct stat conf;


   EET_EINA_STREAM_DATA_DESCRIPTOR_CLASS_SET(&eddc, Entrance_Config);
   edd = eet_data_descriptor_stream_new(&eddc);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "session_path", session_path, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "xinit_path", command.xinit_path, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "xinit_args", command.xinit_args, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "xauth_path", command.xauth_path, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "xauth_file", command.xauth_file, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "session_start", command.session_start, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "session_login", command.session_login, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "session_stop", command.session_stop, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "shutdown", command.shutdown, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "reboot", command.reboot, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "suspend", command.suspend, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "daemonize", daemonize, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "numlock", numlock, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "xsessions", xsessions, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "autologin", autologin, EET_T_UCHAR);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "userlogin", userlogin, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "lockfile", lockfile, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "logfile", logfile, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd, Entrance_Config, "theme", theme, EET_T_STRING);

   if (stat( "/var/cache/"PACKAGE"/"ENTRANCE_CONFIG_FILE, &cache) == -1)
     {
        _users_get();
     }
   else
     {
        stat(SYSTEM_CONFIG_DIR"/entrance.conf", &conf);
        if (cache.st_mtime < conf.st_mtime)
          {
             _users_get();
          }
     }
   entrance_config = _cache_get(edd);
   eet_data_descriptor_free(edd);
}


void
entrance_config_shutdown()
{
   _config_free(entrance_config);
}

