#include <sys/types.h>
#include <unistd.h>
#include <grp.h>
#ifndef HAVE_PAM
#include <shadow.h>
#include <crypt.h>
#endif

#include "entrance.h"

#define HAVE_SHADOW 1

static char *_mcookie;
static char **env;
static char *_login = NULL;
static unsigned char _logged = 0;
//static Eina_List *_user_list = NULL;
static pid_t _session_pid;
static Eina_List *_xsessions = NULL;
static int _entrance_session_userid_set(struct passwd *pwd);

static void _entrance_session_run(struct passwd *pwd, const char *cmd, const char *cookie);

static void _entrance_session_desktops_scan_file(const char *path);
static void _entrance_session_desktops_scan(const char *dir);
static void _entrance_session_desktops_init();
//static void _entrance_session_desktops_shutdown();
static const char *_entrance_session_find_command(const char *path, const char *session);
static struct passwd *_entrance_session_session_open();

long
entrance_session_seed_get()
{
    struct timespec ts;
    long pid = getpid();
    long tm = time(NULL);
    if (clock_gettime(CLOCK_MONOTONIC, &ts))
       ts.tv_sec = ts.tv_nsec = 0;
    return pid + tm + (ts.tv_sec ^ ts.tv_nsec);
}

static int
_entrance_session_cookie_add(const char *mcookie, const char *display,
                         const char *xauth_cmd, const char *auth_file)
{
    char buf[PATH_MAX];
    FILE *cmd;

    if (!xauth_cmd || !auth_file) return 1;
    snprintf(buf, sizeof(buf), "%s -f %s -q", xauth_cmd, auth_file);
    PT("write auth");
    cmd = popen(buf, "w");
    if (!cmd)
      {
         fprintf(stderr, " fail !\n");
         return 1;
      }
    fprintf(cmd, "remove %s\n", display);
    fprintf(cmd, "add %s . %s\n", display, mcookie);
    fprintf(cmd, "exit\n");
    pclose(cmd);
    fprintf(stderr, " done\n");
    return 0;
}

static int
_entrance_session_userid_set(struct passwd *pwd)
{
   if (!pwd)
     {
        PT("no passwd !\n");
        return 1;
     }
   if (initgroups(pwd->pw_name, pwd->pw_gid) != 0)
     {
        PT("can't init group\n");
        return 1;
     }
   if (setgid(pwd->pw_gid) != 0)
     {
        PT("can't set gid\n");
        return 1;
     }
   if (setuid(pwd->pw_uid) != 0)
     {
        PT("can't set uid\n");
        return 1;
     }

/*   PT("name -> %s, gid -> %d, uid -> %d\n",
           pwd->pw_name, pwd->pw_gid, pwd->pw_uid); */
   return 0;
}

static Eina_Bool
_entrance_session_begin(struct passwd *pwd, const char *cookie)
{
   PT("Session Init\n");
   if (pwd->pw_shell[0] == '\0')
     {
        setusershell();
        strcpy(pwd->pw_shell, getusershell());
        endusershell();
     }
#ifdef HAVE_PAM
   char *term = getenv("TERM");
   if (term) entrance_pam_env_set("TERM", term);
   entrance_pam_env_set("HOME", pwd->pw_dir);
   entrance_pam_env_set("SHELL", pwd->pw_shell);
   entrance_pam_env_set("USER", pwd->pw_name);
   entrance_pam_env_set("LOGNAME", pwd->pw_name);
   entrance_pam_env_set("PATH", entrance_config->session_path);
   entrance_pam_env_set("DISPLAY", ":0.0");
   entrance_pam_env_set("MAIL", "");
   entrance_pam_env_set("XAUTHORITY", cookie);
   entrance_pam_env_set("XDG_SESSION_CLASS", "greeter");
#endif
   return EINA_TRUE;
}

static void
_entrance_session_run(struct passwd *pwd, const char *cmd, const char *cookie)
{
   //char **tmp;
   char buf[PATH_MAX];
   pid_t pid;
   pid = fork();
   if (pid == 0)
     {

        PT("Session Run\n");
#ifdef HAVE_PAM
        env = entrance_pam_env_list_get();
        entrance_pam_end();
#else
        int n = 0;
        char *term = getenv("TERM");
        env = (char **)malloc(10 * sizeof(char *));
        if(term)
          {
             snprintf(buf, sizeof(buf), "TERM=%s", term);
             env[n++]=strdup(buf);
          }
        snprintf(buf, sizeof(buf), "HOME=%s", pwd->pw_dir);
        env[n++]=strdup(buf);
        snprintf(buf, sizeof(buf), "SHELL=%s", pwd->pw_shell);
        env[n++]=strdup(buf);
        snprintf(buf, sizeof(buf), "USER=%s", pwd->pw_name);
        env[n++]=strdup(buf);
        snprintf(buf, sizeof(buf), "LOGNAME=%s", pwd->pw_name);
        env[n++]=strdup(buf);
        snprintf(buf, sizeof(buf), "PATH=%s", entrance_config->session_path);
        env[n++]=strdup(buf);
        snprintf(buf, sizeof(buf), "DISPLAY=%s", ":0.0");
        env[n++]=strdup(buf);
        snprintf(buf, sizeof(buf), "MAIL=");
        env[n++]=strdup(buf);
        snprintf(buf, sizeof(buf), "XAUTHORITY=%s", cookie);
        env[n++]=strdup(buf);
        env[n++]=0;
#endif
        snprintf(buf, sizeof(buf),
                 "%s %s ",
                 entrance_config->command.session_start,
                 pwd->pw_name);
        if (-1 == system(buf))
          PT("Error on session start command\n");
        if(_entrance_session_userid_set(pwd)) return;
        _entrance_session_cookie_add(_mcookie, ":0",
                                 entrance_config->command.xauth_path, cookie);
        if (chdir(pwd->pw_dir))
          {
             PT("change directory for user fail");
             return;
          }
//        PT("Open %s`s session\n", pwd->pw_name);
        snprintf(buf, sizeof(buf), "%s/.entrance_session.log", pwd->pw_dir);
        remove(buf);

#ifdef HAVE_CONSOLEKIT
        snprintf(buf, sizeof(buf), PACKAGE_BIN_DIR"/entrance_ck_launch %s > %s/.entrance_session.log 2>&1",
                 cmd, pwd->pw_dir);
#else
        snprintf(buf, sizeof(buf), "%s > %s/.entrance_session.log 2>&1",
                 cmd, pwd->pw_dir);
#endif
        execle(pwd->pw_shell, pwd->pw_shell, "-c", buf, NULL, env);
        PT("The Xsessions are not launched :(\n");
     }
}

void
entrance_session_end(const char *user)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf),
            "%s %s ", entrance_config->command.session_stop, user);
   if (-1 == system(buf))
     PT("Error on session stop command\n");
#ifdef HAVE_PAM
   entrance_pam_close_session();
   entrance_pam_end();
   entrance_pam_shutdown();
#endif
}

void
entrance_session_pid_set(pid_t pid)
{
   fprintf(stderr, "%s: session pid %d\n", PACKAGE, pid);
   _session_pid = pid;
}

pid_t
entrance_session_pid_get()
{
   return _session_pid;
}

static const char *dig = "0123456789abcdef";

void
entrance_session_init(const char *file)
{
   uint16_t word;
   uint8_t hi, lo;
   int i;
   char buf[PATH_MAX];

   PT("Session init\n");

   _mcookie = calloc(33, sizeof(char));
   _mcookie[0] = 'a';

   srand(entrance_session_seed_get());
   for (i=0; i<32; i+=4)
     {
        word = rand() & 0xffff;
        lo = word & 0xff;
        hi = word >> 8;
        _mcookie[i] = dig[lo & 0x0f];
        _mcookie[i+1] = dig[lo >> 4];
        _mcookie[i+2] = dig[hi & 0x0f];
        _mcookie[i+3] = dig[hi >> 4];
     }
//   remove(file);
   snprintf(buf, sizeof(buf), "XAUTHORITY=%s", file);
   putenv(strdup(buf));
   //PT("cookie %s \n", _mcookie);
   _entrance_session_cookie_add(_mcookie, ":0",
                            entrance_config->command.xauth_path, file);
   _entrance_session_desktops_init();
}

void
entrance_session_shutdown()
{
   Entrance_Xsession *xsession;

   EINA_LIST_FREE(_xsessions, xsession)
     {
        eina_stringshare_del(xsession->name);
        eina_stringshare_del(xsession->icon);
        if (xsession->command) eina_stringshare_del(xsession->command);
        free(xsession);
     }
}

Eina_Bool
entrance_session_authenticate(const char *login, const char *passwd)
{
   _login = strdup(login);
#ifdef HAVE_PAM
   return (!entrance_pam_auth_set(login, passwd)
           && !entrance_pam_authenticate());
#else
   char *enc, *v;
   struct passwd *pwd;

   pwd = getpwnam(login);
   endpwent();
   if(!pwd)
     return EINA_FALSE;
#ifdef HAVE_SHADOW
   struct spwd *spd;
   spd = getspnam(pwd->pw_name);
   endspent();
   if(spd)
     v = spd->sp_pwdp;
   else
#endif
     v = pwd->pw_passwd;
   if(!v || *v == '\0')
     return EINA_TRUE;
   enc = crypt(passwd, v);
   return !strcmp(enc, v);
#endif
}

static struct passwd *
_entrance_session_session_open()
{
#ifdef HAVE_PAM
   if (!entrance_pam_open_session())
      return getpwnam(entrance_pam_item_get(ENTRANCE_PAM_ITEM_USER));
   return NULL;
#else
   return getpwnam(entrance_session_login_get());
#endif
}

Eina_Bool
entrance_session_login(const char *session, Eina_Bool push)
{
   struct passwd *pwd;
   const char *cmd;
   char buf[PATH_MAX];

   pwd = _entrance_session_session_open();
   endpwent();
   if (!pwd) return ECORE_CALLBACK_CANCEL;
   _logged = EINA_TRUE;
   snprintf(buf, sizeof(buf), "%s/.Xauthority", pwd->pw_dir);
   if (!_entrance_session_begin(pwd, buf))
     {
        fprintf(stderr, "Entrance: couldn't open session\n");
        exit(1);
     }
   if (push) entrance_history_push(pwd->pw_name, session);
   cmd = _entrance_session_find_command(pwd->pw_dir, session);
   PT("launching session for user ");
   fprintf(stderr, "%s\n", _login);
   _entrance_session_run(pwd, cmd, buf);
   return ECORE_CALLBACK_CANCEL;
}

static const char *
_entrance_session_find_command(const char *path, const char *session)
{
   Eina_List *l;
   Entrance_Xsession *xsession;
   char buf[PATH_MAX];
   if (session)
     {
        EINA_LIST_FOREACH(_xsessions, l, xsession)
          {
             if (!strcmp(xsession->name, session))
               {
                  if (xsession->command)
                    return xsession->command;
               }
          }
     }
   snprintf(buf, sizeof(buf), "%s/%s", path, ".Xsession");
   if (ecore_file_can_exec(buf))
     return eina_stringshare_add(buf);
   return (entrance_config->command.session_login);
}

char *
entrance_session_login_get()
{
   return _login;
}

int
entrance_session_logged_get()
{
   return !!_logged;
}

Eina_List *
entrance_session_list_get()
{
   return _xsessions;
}

static void
_entrance_session_desktops_init()
{
   char buf[PATH_MAX];
   Eina_List *dirs;
   const char *path;
   Entrance_Xsession *xsession;
   Eina_List *l;

   xsession = calloc(1, sizeof(Entrance_Xsession));
   xsession->name = eina_stringshare_add("System");
   xsession->icon = eina_stringshare_add("entrance/system");
   _xsessions = eina_list_append(_xsessions, xsession);

   efreet_desktop_type_alias(EFREET_DESKTOP_TYPE_APPLICATION, "XSession");
   PT("scanning directory: ");
   /* Maybee need to scan other directories ?
    * _entrance_session_desktops_scan("/etc/share/xsessions");
    */
   _entrance_session_desktops_scan("/etc/X11/dm/Sessions");
   snprintf(buf, sizeof(buf), "%s/xsessions", efreet_data_home_get());
   _entrance_session_desktops_scan(buf);
   dirs = efreet_data_dirs_get();
   EINA_LIST_FOREACH(dirs, l, path)
     {
        snprintf(buf, sizeof(buf), "%s/xsessions", path);
        _entrance_session_desktops_scan(buf);
     }
   fprintf(stderr, "\n");
   PT("scan directory end\n");
}

static void
_entrance_session_desktops_scan(const char *dir)
{
   Eina_List *files;
   char *filename;
   char path[PATH_MAX];

   if (ecore_file_is_dir(dir))
     {
        fprintf(stderr, "%s", dir);
        files = ecore_file_ls(dir);
        EINA_LIST_FREE(files, filename)
          {
             snprintf(path, sizeof(path), "%s/%s", dir, filename);
             _entrance_session_desktops_scan_file(path);
             free(filename);
          }
     }
}

static void
_entrance_session_desktops_scan_file(const char *path)
{
   Efreet_Desktop *desktop;
   Eina_List *commands;
   Eina_List *l;
   Entrance_Xsession *xsession;
   char *command = NULL;

   desktop = efreet_desktop_get(path);
   if (!desktop) return;
   EINA_LIST_FOREACH(_xsessions, l, xsession)
     {
        if (!strcmp(xsession->name, desktop->name))
          {
             efreet_desktop_free(desktop);
             return;
          }
     }

   commands = efreet_desktop_command_local_get(desktop, NULL);
   if (commands)
     command = eina_list_data_get(commands);
   if (command && desktop->name)
     {
        xsession= calloc(1, sizeof(Entrance_Xsession));
        xsession->command = eina_stringshare_add(command);
        xsession->name = eina_stringshare_add(desktop->name);
        if (desktop->icon) xsession->icon = eina_stringshare_add(desktop->icon);
        _xsessions = eina_list_append(_xsessions, xsession);
     }
   EINA_LIST_FREE(commands, command)
     free(command);
   efreet_desktop_free(desktop);
}

