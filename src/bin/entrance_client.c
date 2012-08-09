#include "entrance_client.h"
#include <Ecore.h>
#include <Ecore_X.h>
#include <Ecore_Getopt.h>

static const Ecore_Getopt options =
{
   "entrance_client",
   "%prog [options]",
   VERSION,
   "(C) 2011 Enlightenment, see AUTHORS.",
   "GPL, see COPYING",
   "Launch gui client.",
   EINA_TRUE,
   {
      ECORE_GETOPT_STORE_STR('d', "display", "specify the display to use"),
      ECORE_GETOPT_STORE_STR('t', "theme", "specify the theme to use"),
      ECORE_GETOPT_HELP ('h', "help"),
      ECORE_GETOPT_VERSION('V', "version"),
      ECORE_GETOPT_COPYRIGHT('R', "copyright"),
      ECORE_GETOPT_LICENSE('L', "license"),
      ECORE_GETOPT_SENTINEL
   }
};

int
entrance_client_main(const char *theme)
{
   fprintf(stderr, PACKAGE"_client: client init\n");
   if (entrance_gui_init(theme)) return EXIT_FAILURE;
   fprintf(stderr, PACKAGE"_client: client run\n");
   entrance_connect_init();
   elm_run();
   entrance_connect_shutdown();
   fprintf(stderr, PACKAGE"_client: client shutdown\n");
   entrance_gui_shutdown();
   return EXIT_SUCCESS;
}

int
main(int argc, char **argv)
{
   int args;
   unsigned char quit_option = 0;
   char *display = NULL;
   char *theme = NULL;

   Ecore_Getopt_Value values[] =
     {
        ECORE_GETOPT_VALUE_STR(display),
        ECORE_GETOPT_VALUE_STR(theme),
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
   if (!display)
     {
        printf("A display is required!\n");
        return EXIT_FAILURE;
     }
   eina_init();
   ecore_init();
   ecore_x_init(NULL);
   elm_init(argc, argv);
   entrance_client_main(theme);
   elm_shutdown();
   ecore_x_shutdown();
   ecore_shutdown();
   eina_shutdown();
   return EXIT_SUCCESS;
}
