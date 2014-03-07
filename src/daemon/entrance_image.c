#include "entrance.h"

const char *extn_images[] = {".png",".jpg",NULL};

static Eina_List *_entrance_image_readdir(const char *path);
static Eina_List *_entrance_image_get(Eina_List *src, char *stdfile, char *mask);
static char *_entrance_image_homedir_get(const char *usr);

static Eina_List *
_entrance_image_readdir(const char *path)
{
   Eina_List *files;
   Eina_List *targets = NULL;
   char *filename, buf[PATH_MAX];
   int i = 0;

   files = ecore_file_ls(path);
   if (!files) return NULL;
   EINA_LIST_FREE(files, filename)
     {
        snprintf(buf, sizeof(buf), "%s/%s", path, filename);
        if ((!ecore_file_is_dir(buf)) && (filename[0] != '.'))
          {
             for (i = 0; extn_images[i];i ++)
               {
                  if (eina_str_has_extension(filename, extn_images[i]))
                    {
                       targets = eina_list_append(targets, eina_stringshare_add(buf));
                    }
               }

          }
     }
   return targets;
}

static Eina_List *
_entrance_image_get(Eina_List *src, char *stdfile, char *mask)
{
   //If srdfile is NULL we will set the src string to file, if not we will set the stdfile. And the src as group.
   Eina_List *result = NULL;
   char *src_str;
   char path[PATH_MAX];
   Entrance_Image *img;
   EINA_LIST_FREE(src, src_str)
     {
        img = calloc(1, sizeof(Entrance_Image));
        if (stdfile)
          {
            if (mask)
              {
                 snprintf(path, sizeof(path), mask, src_str);
                 img->group = eina_stringshare_add(path);
                 eina_stringshare_del(src_str);
              }
            else
              img->group = src_str;
            img->path = eina_stringshare_add(stdfile);
          }
        else
          img->path = src_str;
        result = eina_list_append(result,img);
     }
   return result;
}

static char *
_entrance_image_homedir_get(const char *usr)
{
   char *name;
   struct passwd *pw;

   pw = getpwnam(usr);
   if (!pw) return NULL;
   name = pw->pw_dir;
   return name;
}

Eina_List *
entrance_image_system_icons(void)
{
   return _entrance_image_get(
      _entrance_image_readdir(PACKAGE_DATA_DIR"/images/icons/"),
                              NULL, NULL);
}

Eina_List *
 entrance_image_system_backgrounds(void)
{
   return _entrance_image_get(
      _entrance_image_readdir(PACKAGE_DATA_DIR"/images/backgrounds/"),
                              NULL, NULL);
}

Eina_List *
entrance_image_user_icons(const char *username)
{
   char path[PATH_MAX];
   char *homedir;

   homedir = _entrance_image_homedir_get(username);
   if (!homedir) return NULL;
   snprintf(path, sizeof(path),"%s/.config/entrance/images/icons/", homedir);
   return _entrance_image_get(_entrance_image_readdir(path), NULL, NULL);
}

Eina_List *
entrance_image_user_backgrounds(const char *username)
{
   char path[PATH_MAX];
   char  *homedir;

   homedir = _entrance_image_homedir_get(username);
   if (!homedir) return NULL;
   snprintf(path, sizeof(path),"%s/.config/entrance/images/backgrounds/", homedir);
   return _entrance_image_get(_entrance_image_readdir(path), NULL, NULL);
}