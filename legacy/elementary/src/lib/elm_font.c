#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include <Elementary.h>
#include "elm_priv.h"

Elm_Font_Properties *
_elm_font_properties_get(Eina_Hash **font_hash,
                         const char *font)
{
   Elm_Font_Properties *efp = NULL;
   char *s1;

   s1 = strchr(font, ':');
   if (s1)
     {
        char *s2, *name, *style;
        int len;

        len = s1 - font;
        name = calloc(sizeof(char), len + 1);
        strncpy(name, font, len);

        /* get subname (should be english)  */
        s2 = strchr(name, ',');
        if (s2)
          {
             len = s2 - name;
             name = realloc(name, sizeof(char) * len + 1);
             memset(name, 0, sizeof(char) * len + 1);
             strncpy(name, font, len);
          }

        if (!strncmp(s1, ELM_FONT_TOKEN_STYLE, strlen(ELM_FONT_TOKEN_STYLE)))
          {
             style = s1 + strlen(ELM_FONT_TOKEN_STYLE);

             if (font_hash) efp = eina_hash_find(*font_hash, name);
             if (!efp)
               {
                  efp = calloc(1, sizeof(Elm_Font_Properties));
                  efp->name = eina_stringshare_add(name);
                  if (font_hash)
                    {
                       if (!*font_hash)
                         *font_hash = eina_hash_string_superfast_new(NULL);
                       eina_hash_add(*font_hash, name, efp);
                    }
               }
             s2 = strchr(style, ',');
             if (s2)
               {
                  char *style_old;

                  len = s2 - style;
                  style_old = style;
                  style = calloc(sizeof(char), len + 1);
                  strncpy(style, style_old, len);
                  efp->styles = eina_list_append(efp->styles,
                                                 eina_stringshare_add(style));
                  free(style);
               }
             else
               efp->styles = eina_list_append(efp->styles,
                                              eina_stringshare_add(style));
          }
        free(name);
     }
   else
     {
        if (font_hash) efp = eina_hash_find(*font_hash, font);
        if (!efp)
          {
             efp = calloc(1, sizeof(Elm_Font_Properties));
             efp->name = eina_stringshare_add(font);
             if (font_hash)
               {
                  if (!*font_hash)
                    *font_hash = eina_hash_string_superfast_new(NULL);
                  eina_hash_add(*font_hash, font, efp);
               }
          }
     }
   return efp;
}

/* FIXME: do we really need it? */
Eina_Hash *
_elm_font_available_hash_add(Eina_Hash  *font_hash,
                             const char *full_name)
{
   _elm_font_properties_get(&font_hash, full_name);
   return font_hash;
}

static void
_elm_font_properties_free(Elm_Font_Properties *efp)
{
   const char *str;

   EINA_LIST_FREE(efp->styles, str)
     if (str) eina_stringshare_del(str);

   if (efp->name) eina_stringshare_del(efp->name);
   free(efp);
}

static Eina_Bool
_font_hash_free_cb(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata __UNUSED__)
{
   Elm_Font_Properties *efp;

   efp = data;
   _elm_font_properties_free(efp);
   return EINA_TRUE;
}

void
_elm_font_available_hash_del(Eina_Hash *hash)
{
   if (!hash) return ;

   eina_hash_foreach(hash, _font_hash_free_cb, NULL);
}
