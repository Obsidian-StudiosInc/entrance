#include <Evas.h>
#include "entrance.h"

static Eina_List *_entrance_image_readdir(const char *path);
static Eina_List *_entrance_image_get(Eina_List *src, char *stdfile, char *mask);
static char *_entrance_image_homedir_get(const char *usr);

static Eina_List *
_entrance_image_readdir(const char *path)
{
   Eina_Iterator *files;
   Eina_List *targets = NULL;
   Eina_File_Direct_Info *file_stat;
   char *buf;

   files = eina_file_stat_ls(path);
   if (!files) return NULL;
   EINA_ITERATOR_FOREACH(files, file_stat)
     {
        buf = file_stat->path;
        if (file_stat->path[file_stat->name_start] != '.'
            && file_stat->type == EINA_FILE_REG
            && evas_object_image_extension_can_load_get(buf))
          targets = eina_list_append(targets, eina_stringshare_add(buf));
     }
   eina_iterator_free(files);
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

Eina_List*
entrance_image_user_pool_get(Eina_List *users)
{
   Eina_List *node, *result = NULL;
   Entrance_Login *eu;
   Entrance_User_Pool *pool;

   EINA_LIST_FOREACH(users, node, eu)
     {
        pool = calloc(1, sizeof(Entrance_User_Pool));
        pool->name = eina_stringshare_add(eu->login);
        pool->icon_pool = entrance_image_user_icons(pool->name);
        pool->background_pool = entrance_image_user_backgrounds(pool->name);
        result = eina_list_append(result, pool);
     }
   return result;
}

void
entrance_image_user_pool_free(Eina_List *user_pool)
{
   Entrance_User_Pool *pool;
   Entrance_Image *img;

   EINA_LIST_FREE(user_pool, pool)
     {
        eina_stringshare_del(pool->name);
        EINA_LIST_FREE(pool->icon_pool, img)
          {
             eina_stringshare_del(img->path);
             eina_stringshare_del(img->group);
             free(img);
          }
        EINA_LIST_FREE(pool->background_pool, img)
          {
             eina_stringshare_del(img->path);
             eina_stringshare_del(img->group);
             free(img);
          }
        free(pool);
     }
}
