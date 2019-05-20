#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"

Elm_Font_Properties *
_elm_font_properties_get(Eina_Hash **font_hash,
                         const char *font)
{
   Elm_Font_Properties *efp = NULL;
   char *token = strchr(font, ':');

   if (token &&
       !strncmp(token, ELM_FONT_TOKEN_STYLE, strlen(ELM_FONT_TOKEN_STYLE)))
     {
        char *name, *subname, *style, *substyle;
        int len;

        /* get font name */
        len = token - font;
        name = calloc(len + 1, sizeof(char));
        if (!name) return NULL;
        strncpy(name, font, len);

        /* remove subnames from the font name (should be english)  */
        subname = strchr(name, ',');
        if (subname)
          {
             len = subname - name;
             *subname = '\0';
          }

        /* add a font name */
        if (font_hash)
          efp = eina_hash_find(*font_hash, name);
        if (!efp)
          {
             efp = calloc(1, sizeof(Elm_Font_Properties));
             if (!efp)
               {
                  free(name);
                  return NULL;
               }

             efp->name = eina_stringshare_add_length(name, len);
             if (font_hash)
               {
                  if (!*font_hash)
                    *font_hash = eina_hash_string_superfast_new(NULL);
                  eina_hash_add(*font_hash, name, efp);
               }
          }

        free(name);

        style = token + strlen(ELM_FONT_TOKEN_STYLE);
        substyle = strchr(style, ',');

        //TODO: Seems to need to add all styles. not only one.
        if (substyle)
          {
             char *style_old = style;

             len = substyle - style;
             style = calloc(len + 1, sizeof(char));
             if (style)
               {
                  strncpy(style, style_old, len);
                  efp->styles = eina_list_append(efp->styles,
                                                 eina_stringshare_add(style));
                  free(style);
               }
          }
        else
          efp->styles = eina_list_append(efp->styles,
                                         eina_stringshare_add(style));
     }
   else if ((font_hash) && (!eina_hash_find(*font_hash, font)))
     {
        efp = calloc(1, sizeof(Elm_Font_Properties));
        if (!efp) return NULL;

        efp->name = eina_stringshare_add(font);
        if (!*font_hash) *font_hash = eina_hash_string_superfast_new(NULL);
        eina_hash_add(*font_hash, font, efp);
     }

   return efp;
}

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
     eina_stringshare_del(str);

   eina_stringshare_del(efp->name);
   free(efp);
}

static Eina_Bool
_font_hash_free_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata EINA_UNUSED)
{
   Elm_Font_Properties *efp;

   efp = data;
   _elm_font_properties_free(efp);
   return EINA_TRUE;
}

void
_elm_font_available_hash_del(Eina_Hash *hash)
{
   if (!hash) return;

   eina_hash_foreach(hash, _font_hash_free_cb, NULL);
   eina_hash_free(hash);
}

EAPI Elm_Font_Properties *
elm_font_properties_get(const char *font)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(font, NULL);
   return _elm_font_properties_get(NULL, font);
}

EAPI void
elm_font_properties_free(Elm_Font_Properties *efp)
{
   const char *str;

   EINA_SAFETY_ON_NULL_RETURN(efp);
   EINA_LIST_FREE(efp->styles, str)
     eina_stringshare_del(str);
   eina_stringshare_del(efp->name);
   free(efp);
}

EAPI char *
elm_font_fontconfig_name_get(const char *name,
                             const char *style)
{
   char buf[256];

   EINA_SAFETY_ON_NULL_RETURN_VAL(name, NULL);
   if (!style || style[0] == 0) return (char *) eina_stringshare_add(name);
   snprintf(buf, 256, "%s" ELM_FONT_TOKEN_STYLE "%s", name, style);
   return (char *) eina_stringshare_add(buf);
}

EAPI void
elm_font_fontconfig_name_free(char *name)
{
   eina_stringshare_del(name);
}

EAPI Eina_Hash *
elm_font_available_hash_add(Eina_List *list)
{
   Eina_Hash *font_hash;
   Eina_List *l;
   void *key;

   font_hash = NULL;

   /* populate with default font families */
   //FIXME: Need to check whether fonts are being added multiple times.
   font_hash = _elm_font_available_hash_add(font_hash, "Sans:style=Regular");
   font_hash = _elm_font_available_hash_add(font_hash, "Sans:style=Bold");
   font_hash = _elm_font_available_hash_add(font_hash, "Sans:style=Oblique");
   font_hash = _elm_font_available_hash_add(font_hash, "Sans:style=Bold Oblique");
   font_hash = _elm_font_available_hash_add(font_hash, "Serif:style=Regular");
   font_hash = _elm_font_available_hash_add(font_hash, "Serif:style=Bold");
   font_hash = _elm_font_available_hash_add(font_hash, "Serif:style=Oblique");
   font_hash = _elm_font_available_hash_add(font_hash, "Serif:style=Bold Oblique");
   font_hash = _elm_font_available_hash_add(font_hash, "Monospace:style=Regular");
   font_hash = _elm_font_available_hash_add(font_hash, "Monospace:style=Bold");
   font_hash = _elm_font_available_hash_add(font_hash, "Monospace:style=Oblique");
   font_hash = _elm_font_available_hash_add(font_hash, "Monospace:style=Bold Oblique");

   EINA_LIST_FOREACH(list, l, key)
     if (key) font_hash = _elm_font_available_hash_add(font_hash, key);

   return font_hash;
}

EAPI void
elm_font_available_hash_del(Eina_Hash *hash)
{
   _elm_font_available_hash_del(hash);
}
