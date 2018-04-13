#include "entrance_client.h"
#include <Ecore.h>
#include <Ecore_X.h>
#include <Ecore_Getopt.h>

#define NOBODY 65534

static const Ecore_Getopt options =
{
   "entrance_client",
   "%prog [options]",
   VERSION,
   "(C) 2017 Obsidian-Studios, Inc. see AUTHORS.",
   "GPL, see COPYING",
   "Launch gui client.",
   EINA_TRUE,
   {
      ECORE_GETOPT_STORE_STR('d', "display", "specify the display to use"),
      ECORE_GETOPT_STORE_INT('g', "gid", "specify the group to run under"),
      ECORE_GETOPT_STORE_STR('t', "theme", "specify the theme to use"),
      ECORE_GETOPT_STORE_INT('u', "uid", "specify the user to run under"),
      ECORE_GETOPT_HELP ('h', "help"),
      ECORE_GETOPT_VERSION('V', "version"),
      ECORE_GETOPT_COPYRIGHT('R', "copyright"),
      ECORE_GETOPT_LICENSE('L', "license"),
      ECORE_GETOPT_SENTINEL
   }
};

int
main(int argc, char **argv)
{
   int args;
   unsigned char quit_option = 0;
   char *display = NULL;
   char *theme = NULL;
   int gid = 0;
   int uid = 0;

   Ecore_Getopt_Value values[] =
     {
        ECORE_GETOPT_VALUE_STR(display),
        ECORE_GETOPT_VALUE_INT(gid),
        ECORE_GETOPT_VALUE_STR(theme),
        ECORE_GETOPT_VALUE_INT(uid),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_BOOL(quit_option),
        ECORE_GETOPT_VALUE_BOOL(quit_option)
     };
   args = ecore_getopt_parse(&options, values, argc, argv);
   if (args < 0)
     return EXIT_FAILURE;
   if (quit_option)
     return EXIT_SUCCESS;
   if(gid>0)
     setgid(gid);
   else
     {
       PT("root gid not allowed, defaulting to nobody");
       setgid(NOBODY);
     }
   if(uid>0)
     setuid(uid);
   else
     {
       PT("root uid not allowed, defaulting to nobody");
       setuid(NOBODY);
     }
   eina_init();
   if (!display)
     {
        PT("A display is required!");
        eina_shutdown();
        return EXIT_FAILURE;
     }
   if (!theme)
     theme = "default";
   ecore_init();
   ecore_x_init(display);
   elm_init(argc, argv);
   PT("login init");
   entrance_login_init();
   PT("gui init");
   if (!entrance_gui_init(theme)) return EXIT_FAILURE;
   PT("connect init");
   entrance_connect_init();
   elm_run();
   PT("connect shutdown");
   entrance_connect_shutdown();
   PT("gui shutdown");
   entrance_gui_shutdown();
   PT("login shutdown");
   entrance_login_shutdown();
   PT("elm shutdown");
   elm_shutdown();
   PT("ecore shutdown");
   ecore_x_shutdown();
   ecore_shutdown();
   PT("eina shutdown");
   eina_shutdown();
   PT("exit");
   return EXIT_SUCCESS;
}

