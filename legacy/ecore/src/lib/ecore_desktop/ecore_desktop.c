#include "Ecore_Desktop.h"
#include "ecore_desktop_private.h"

extern int reject_count, not_over_count;

static Ecore_Hash *ini_file_cache;
static Ecore_Hash *desktop_cache;


Ecore_Hash *
ecore_desktop_parse_ini_file(char *file)
{
   Ecore_Hash *result;

/* FIXME: should probably look in ini_file_cache first. */
   result = ecore_hash_new(ecore_str_hash, ecore_str_compare);
   if (result)
     {
        FILE *f;
        char buffer[MAX_PATH];
        Ecore_Hash *current = NULL;

        f = fopen(file, "r");
        if (!f)
          {
             fprintf(stderr, "ERROR: Cannot Open File %s\n", file);
             ecore_hash_destroy(result);
             return NULL;
          }
        ecore_hash_set_free_key(result, free);
        ecore_hash_set_free_value(result, (Ecore_Free_Cb) ecore_hash_destroy);
        *buffer = '\0';
#ifdef DEBUG
        fprintf(stdout, "PARSING INI %s\n", file);
#endif
        while (fgets(buffer, sizeof(buffer), f) != NULL)
          {
             char *c;
             char *key;
             char *value;

             c = buffer;
             /* Strip preceeding blanks. */
             while (((*c == ' ') || (*c == '\t')) && (*c != '\n') && (*c != '\0'))
                c++;
             /* Skip blank lines and comments */
             if ((*c == '\0') || (*c == '\n') || (*c == '#'))
                continue;
             if (*c == '[')     /* New group. */
               {
                  key = c + 1;
                  while ((*c != ']') && (*c != '\n') && (*c != '\0'))
                     c++;
                  *c++ = '\0';
                  current = ecore_hash_new(ecore_str_hash, ecore_str_compare);
                  if (current)
                    {
                       ecore_hash_set_free_key(current, free);
                       ecore_hash_set_free_value(current, free);
                       ecore_hash_set(result, strdup(key), current);
#ifdef DEBUG
                       fprintf(stdout, "  GROUP [%s]\n", key);
#endif
                    }
               }
             else if (current)  /* key=value pair of current group. */
               {
                  key = c;
                  /* Find trailing blanks or =. */
                  while ((*c != '=') && (*c != ' ') && (*c != '\t') && (*c != '\n') && (*c != '\0'))
                     c++;
                  if (*c != '=')        /* Find equals. */
                    {
                       *c++ = '\0';
                       while ((*c != '=') && (*c != '\n') && (*c != '\0'))
                          c++;
                    }
                  if (*c == '=')        /* Equals found. */
                    {
                       *c++ = '\0';
                       /* Strip preceeding blanks. */
                       while (((*c == ' ') || (*c == '\t')) && (*c != '\n') && (*c != '\0'))
                          c++;
                       value = c;
                       /* Find end. */
                       while ((*c != '\n') && (*c != '\0'))
                          c++;
                       *c++ = '\0';
                       /* FIXME: should strip space at end, then unescape value. */
                       ecore_hash_set(current, strdup(key), strdup(value));
#ifdef DEBUG
                       fprintf(stdout, "    %s=%s\n", key, value);
#endif
                    }
               }

          }
        buffer[0] = (char)0;

        fclose(f);
        ecore_hash_set(ini_file_cache, strdup(file), result);
     }
   return result;
}


Ecore_Desktop *
ecore_desktop_parse_file(char *file)
{
   Ecore_Desktop *result;

   result = (Ecore_Desktop *) ecore_hash_get(desktop_cache, file);
   if (!result)
     {
        result = calloc(1, sizeof(Ecore_Desktop));
        if (result)
          {
             result->data = ecore_desktop_parse_ini_file(file);
             if (result->data)
               {
                  result->group = (Ecore_Hash *) ecore_hash_get(result->data, "Desktop Entry");
                  if (!result->group)
                     result->group = (Ecore_Hash *) ecore_hash_get(result->data, "KDE Desktop Entry");
                  if (result->group)
                    {
                       char *value;

                       result->name = (char *)ecore_hash_get(result->group, "Name");
                       result->generic = (char *)ecore_hash_get(result->group, "GenericName");
                       result->comment = (char *)ecore_hash_get(result->group, "Comment");
                       result->type = (char *)ecore_hash_get(result->group, "Type");
                       result->exec = (char *)ecore_hash_get(result->group, "Exec");
                       result->window_class = (char *)ecore_hash_get(result->group, "StartupWMClass");
                       result->icon = (char *)ecore_hash_get(result->group, "Icon");
                       result->categories = (char *)ecore_hash_get(result->group, "Categories");
                       if (result->categories)
                          result->Categories = ecore_desktop_hash_from_paths(result->categories);
                       value = (char *)ecore_hash_get(result->group, "OnlyShowIn");
                       if (value)
                          result->OnlyShowIn = ecore_desktop_hash_from_paths(value);
                       value = (char *)ecore_hash_get(result->group, "NotShowIn");
                       if (value)
                          result->NotShowIn = ecore_desktop_hash_from_paths(value);
                       value = (char *)ecore_hash_get(result->group, "X-KDE-StartupNotify");
                       if (value)
                          result->startup = (!strcmp(value, "true")) ? "1" : "0";
                       value = (char *)ecore_hash_get(result->group, "StartupNotify");
                       if (value)
                          result->startup = (!strcmp(value, "true")) ? "1" : "0";
                    }

                  ecore_hash_set(desktop_cache, strdup(file), result);
               }
             else
               {
                  free(result);
                  result = NULL;
               }
          }
     }
   return result;
}


void
ecore_desktop_init()
{
   if (!ini_file_cache)
     {
        ini_file_cache = ecore_hash_new(ecore_str_hash, ecore_str_compare);
        if (ini_file_cache)
          {
             ecore_hash_set_free_key(ini_file_cache, free);
             ecore_hash_set_free_value(ini_file_cache, (Ecore_Free_Cb) ecore_hash_destroy);
          }
     }
   if (!desktop_cache)
     {
        desktop_cache = ecore_hash_new(ecore_str_hash, ecore_str_compare);
        if (desktop_cache)
          {
             ecore_hash_set_free_key(desktop_cache, free);
             ecore_hash_set_free_value(desktop_cache, (Ecore_Free_Cb) ecore_desktop_destroy);
          }
     }
}


void
ecore_desktop_shutdown()
{
   if (ini_file_cache)
     {
        ecore_hash_destroy(ini_file_cache);
        ini_file_cache = NULL;
     }
   if (desktop_cache)
     {
        ecore_hash_destroy(desktop_cache);
        desktop_cache = NULL;
     }
}


void
ecore_desktop_destroy(Ecore_Desktop * desktop)
{
   if (desktop->NotShowIn)
      ecore_hash_destroy(desktop->NotShowIn);
   if (desktop->OnlyShowIn)
      ecore_hash_destroy(desktop->OnlyShowIn);
   if (desktop->Categories)
      ecore_hash_destroy(desktop->Categories);
   free(desktop);
}


char *
ecore_desktop_get_home()
{
   char *d;
   int length;
   char home[MAX_PATH];

   /* Get Home Dir, check for trailing '/', strip it */
   snprintf(home, sizeof(home), "%s", getenv("HOME"));
   d = strrchr(home, '/');
   if (d)
     {
        if (strlen(d) == 1)
          {
             if (home[(length = strlen(home) - 1)] == '/')
                home[length] = '\0';
          }
     }
   return strdup(home);
}
