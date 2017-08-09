#include "edje_private.h"

static int
_hex_string_get(char ch)
{
   if ((ch >= '0') && (ch <= '9')) return (ch - '0');
   else if ((ch >= 'A') && (ch <= 'F')) return (ch - 'A' + 10);
   else if ((ch >= 'a') && (ch <= 'f')) return (ch - 'a' + 10);
   return 0;
}

/**
 * @internal
 * Calculate the color string according to the given RGBA color.
 * @detail It returns a multiplied color string.
 * Return string should be free'd manually.
 *
 * @param color The existing color string from Textblock style.
 * @param r     Red color
 * @param g     Green color
 * @param b     Blue color
 * @param a     Alpha value
 * @return multiplied color string. It should be free'd manually.
 */
char *
_edje_textblock_color_calc(const char *color, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
   int cr, cg, cb, ca;
   int nr, ng, nb, na;
   int len;
   char *ret;

   cr = cg = cb = ca = 0;
   len = strlen(color);

   if (len == 7) /* #RRGGBB */
     {
        cr = (_hex_string_get(color[1]) << 4) | (_hex_string_get(color[2]));
        cg = (_hex_string_get(color[3]) << 4) | (_hex_string_get(color[4]));
        cb = (_hex_string_get(color[5]) << 4) | (_hex_string_get(color[6]));
        ca = 0xff;
     }
   else if (len == 9) /* #RRGGBBAA */
     {
        cr = (_hex_string_get(color[1]) << 4) | (_hex_string_get(color[2]));
        cg = (_hex_string_get(color[3]) << 4) | (_hex_string_get(color[4]));
        cb = (_hex_string_get(color[5]) << 4) | (_hex_string_get(color[6]));
        ca = (_hex_string_get(color[7]) << 4) | (_hex_string_get(color[8]));
     }
   else if (len == 4) /* #RGB */
     {
        cr = _hex_string_get(color[1]);
        cr = (cr << 4) | cr;
        cg = _hex_string_get(color[2]);
        cg = (cg << 4) | cg;
        cb = _hex_string_get(color[3]);
        cb = (cb << 4) | cb;
        ca = 0xff;
     }
   else if (len == 5) /* #RGBA */
     {
        cr = _hex_string_get(color[1]);
        cr = (cr << 4) | cr;
        cg = _hex_string_get(color[2]);
        cg = (cg << 4) | cg;
        cb = _hex_string_get(color[3]);
        cb = (cb << 4) | cb;
        ca = _hex_string_get(color[4]);
        ca = (ca << 4) | ca;
     }

     nr = (((int)r + 1) * cr) >> 8;
     ng = (((int)g + 1) * cg) >> 8;
     nb = (((int)b + 1) * cb) >> 8;
     na = (((int)a + 1) * ca) >> 8;

     ret = malloc(10);
     snprintf(ret, 10, "#%02x%02x%02x%02x", nr, ng, nb, na);

     return ret;
}

static int
_edje_font_is_embedded(Edje_File *edf, char *font)
{
   if (!eina_hash_find(edf->fonts, font)) return 0;
   return 1;
}

static void
_edje_format_param_parse(char *item, char **key, char **val)
{
   char *p, *k, *v;

   p = strchr(item, '=');
   if (!p) return;

   k = malloc(p - item + 1);
   strncpy(k, item, p - item);
   k[p - item] = 0;
   *key = k;
   p++;
   v = strdup(p);
   *val = v;
}

static char *
_edje_format_parse(const char **s)
{
   const char *p;
   const char *s1 = NULL;
   const char *s2 = NULL;
   Eina_Bool quote = EINA_FALSE;

   p = *s;
   if ((!p) || (*p == 0)) return NULL;
   for (;; )
     {
        if (!s1)
          {
             if (*p != ' ') s1 = p;
             if (*p == 0) break;
          }
        else if (!s2)
          {
             if (*p == '\'')
               {
                  quote = !quote;
               }

             if ((p > *s) && (p[-1] != '\\') && (!quote))
               {
                  if (*p == ' ') s2 = p;
               }
             if (*p == 0) s2 = p;
          }
        p++;
        if (s1 && s2 && (s2 > s1))
          {
             size_t len = s2 - s1;
             char *ret = malloc(len + 1);
             memcpy(ret, s1, len);
             ret[len] = '\0';
             *s = s2;
             return ret;
          }
     }
   *s = p;
   return NULL;
}

static int
_edje_format_is_param(char *item)
{
   if (strchr(item, '=')) return 1;
   return 0;
}

static char *
_edje_format_reparse(Edje_File *edf, const char *str, Edje_Style_Tag **tag_ret)
{
   Eina_Strbuf *txt, *tmp = NULL;
   char *s2, *item, *ret;
   const char *s;

   txt = eina_strbuf_new();
   s = str;
   while ((item = _edje_format_parse(&s)))
     {
        if (_edje_format_is_param(item))
          {
             char *key = NULL, *val = NULL;

             _edje_format_param_parse(item, &key, &val);
             if (!strcmp(key, "font_source"))
               {
                  /* dont allow font sources */
               }
             else if (!strcmp(key, "text_class"))
               {
                  if (tag_ret)
                    (*tag_ret)->text_class = eina_stringshare_add(val);
               }
             else if (!strcmp(key, "font_size"))
               {
                  if (tag_ret)
                    (*tag_ret)->font_size = atof(val);
               }
             else if (!strcmp(key, "font")) /* Fix fonts */
               {
                  if (tag_ret)
                    {
                       if (_edje_font_is_embedded(edf, val))
                         {
                            if (!tmp)
                              tmp = eina_strbuf_new();
                            eina_strbuf_append(tmp, "edje/fonts/");
                            eina_strbuf_append(tmp, val);
                            (*tag_ret)->font = eina_stringshare_add(eina_strbuf_string_get(tmp));
                            eina_strbuf_reset(tmp);
                         }
                       else
                         {
                            (*tag_ret)->font = eina_stringshare_add(val);
                         }
                    }
               }
             else if (!strcmp(key, "color_class"))
               {
                  if (tag_ret)
                    (*tag_ret)->color_class = eina_stringshare_add(val);
               }
             else if (!strcmp(key, "color"))
               {
                  if (tag_ret)
                    (*tag_ret)->color = eina_stringshare_add(val);
               }
             else if (!strcmp(key, "outline_color"))
               {
                  if (tag_ret)
                    (*tag_ret)->outline_color = eina_stringshare_add(val);
               }
             else if (!strcmp(key, "shadow_color"))
               {
                  if (tag_ret)
                    (*tag_ret)->shadow_color = eina_stringshare_add(val);
               }
             s2 = eina_str_escape(item);
             if (s2)
               {
                  if (eina_strbuf_length_get(txt)) eina_strbuf_append(txt, " ");
                  eina_strbuf_append(txt, s2);
                  free(s2);
               }
             free(key);
             free(val);
          }
        else
          {
             if (eina_strbuf_length_get(txt)) eina_strbuf_append(txt, " ");
             eina_strbuf_append(txt, item);
          }
        free(item);
     }
   if (tmp)
     eina_strbuf_free(tmp);
   ret = eina_strbuf_string_steal(txt);
   eina_strbuf_free(txt);
   return ret;
}


/* Update the given evas_style
 *
 * @param ed The edje containing the given style which need to be updated
 * @param style The style which need to be updated
 * @param force Update the given style forcely or not
 */
void
_edje_textblock_style_update(Edje *ed, Edje_Style *stl, Eina_Bool force)
{
   Eina_List *l;
   Eina_Strbuf *txt = NULL;
   Edje_Style_Tag *tag;
   Edje_Text_Class *tc;
   Edje_Color_Class *cc;
   int found = 0;
   char *fontset = NULL, *fontsource = NULL;

   if (!ed->file) return;

   /* Make sure the style is already defined */
   if (!stl->style) return;

   /* No need to compute it again and again and again */
   if (!force && stl->cache) return;

   /* Make sure the style contains a text_class */
   EINA_LIST_FOREACH(stl->tags, l, tag)
     {
        if (tag->text_class)
          {
             found = 1;
             break;
          }

        if (tag->color_class)
          {
             found = 1;
             break;
          }
     }

   /* No text classes , goto next style */
   if (!found) return;
   if (!txt)
     txt = eina_strbuf_new();

   if (_edje_fontset_append)
     fontset = eina_str_escape(_edje_fontset_append);
   fontsource = eina_str_escape(ed->file->path);

   /* Build the style from each tag */
   EINA_LIST_FOREACH(stl->tags, l, tag)
     {
        if (!tag->key) continue;

        /* Add Tag Key */
        eina_strbuf_append(txt, tag->key);
        eina_strbuf_append(txt, "='");

        /* Configure fonts from text class if it exists */
        tc = _edje_text_class_find(ed, tag->text_class);

        /* Configure color from color class if it exists */
        cc = _edje_color_class_find(ed, tag->color_class);

        /* Add and Handle tag parsed data */
        eina_strbuf_append(txt, tag->value);

        if (!strcmp(tag->key, "DEFAULT"))
          {
             if (fontset)
               {
                  eina_strbuf_append(txt, " ");
                  eina_strbuf_append(txt, "font_fallbacks=");
                  eina_strbuf_append(txt, fontset);
               }
             eina_strbuf_append(txt, " ");
             eina_strbuf_append(txt, "font_source=");
             eina_strbuf_append(txt, fontsource);
          }
        if (!EINA_DBL_EQ(tag->font_size, 0))
          {
             char font_size[32];

             if (tc && tc->size)
               snprintf(font_size, sizeof(font_size), "%f",
                        (double)_edje_text_size_calc(tag->font_size, tc));
             else
               snprintf(font_size, sizeof(font_size), "%f",
                        tag->font_size);

             eina_strbuf_append(txt, " ");
             eina_strbuf_append(txt, "font_size=");
             eina_strbuf_append(txt, font_size);
          }
        /* Add font name last to save evas from multiple loads */
        if (tag->font)
          {
             const char *f;
             char *sfont = NULL;

             eina_strbuf_append(txt, " ");
             eina_strbuf_append(txt, "font=");

             if (tc) f = _edje_text_font_get(tag->font, tc->font, &sfont);
             else f = tag->font;

             eina_strbuf_append_escaped(txt, f);

             if (sfont) free(sfont);
          }
        if (tag->color)
          {
             eina_strbuf_append(txt, " ");
             eina_strbuf_append(txt, "color=");

             if (cc)
               {
                  char *color;

                  color = _edje_textblock_color_calc(tag->color,
                                                     cc->r, cc->g, cc->b, cc->a);
                  eina_strbuf_append_escaped(txt, (const char *)color);
                  if (color) free(color);
               }
             else
               {
                  eina_strbuf_append_escaped(txt, tag->color);
               }
          }
        if (tag->outline_color)
          {
             eina_strbuf_append(txt, " ");
             eina_strbuf_append(txt, "outline_color=");

             if (cc)
               {
                  char *color;

                  color = _edje_textblock_color_calc(tag->outline_color,
                                                     cc->r2, cc->g2, cc->b2, cc->a2);
                  eina_strbuf_append_escaped(txt, (const char *)color);
                  if (color) free(color);
               }
             else
               {
                  eina_strbuf_append_escaped(txt, tag->outline_color);
               }
          }
        if (tag->shadow_color)
          {
             eina_strbuf_append(txt, " ");
             eina_strbuf_append(txt, "shadow_color=");

             if (cc)
               {
                  char *color;

                  color = _edje_textblock_color_calc(tag->shadow_color,
                                                     cc->r3, cc->g3, cc->b3, cc->a3);
                  eina_strbuf_append_escaped(txt, (const char *)color);
                  if (color) free(color);
               }
             else
               {
                  eina_strbuf_append_escaped(txt, tag->shadow_color);
               }
          }

        eina_strbuf_append(txt, "'");
     }
   if (fontset) free(fontset);
   if (fontsource) free(fontsource);

   /* Configure the style */
   stl->cache = EINA_TRUE;
   evas_textblock_style_set(stl->style, eina_strbuf_string_get(txt));
   if (txt)
     eina_strbuf_free(txt);
}

/* Update all evas_styles which are in an edje
 *
 * @param ed The edje containing styles which need to be updated
 */
void
_edje_textblock_style_all_update(Edje *ed)
{
   Eina_List *l;
   Edje_Style *stl;

   if (!ed->file) return;

   EINA_LIST_FOREACH(ed->file->styles, l, stl)
      _edje_textblock_style_update(ed, stl, EINA_FALSE);
}

static inline Edje_Style *
_edje_textblock_style_search(Edje *ed, const char *style)
{
   Edje_Style *stl = NULL;
   Eina_List *l;

   if (!style) return NULL;

   EINA_LIST_FOREACH(ed->file->styles, l, stl)
     {
        if ((stl->name) &&
            (stl->name == style || !strcmp(stl->name, style))) break;
        stl = NULL;
     }

   return stl;
}

static inline void
_edje_textblock_style_member_add(Edje *ed, Edje_Style *stl)
{
   Edje_Style_Tag *tag;
   Eina_List *l;

   if (!stl) return;

   EINA_LIST_FOREACH(stl->tags, l, tag)
     {
        Eina_Bool force_update = EINA_FALSE;

        if (tag->text_class)
          {
             efl_observable_observer_add(_edje_text_class_member, tag->text_class, ed->obj);

             force_update = EINA_TRUE;
          }
        if (tag->color_class)
          {
             efl_observable_observer_add(_edje_color_class_member, tag->color_class, ed->obj);

             force_update = EINA_TRUE;
          }

        /* Newly added text_class member should be updated
           according to the latest text_class's status. */
        if (force_update)
          _edje_textblock_style_update(ed, stl, EINA_TRUE);
     }
}

void
_edje_textblock_styles_add(Edje *ed, Edje_Real_Part *ep)
{
   Edje_Part *pt = ep->part;
   Edje_Part_Description_Text *desc;
   Edje_Style *stl = NULL;
   const char *style;
   unsigned int i;

   if (pt->type != EDJE_PART_TYPE_TEXTBLOCK) return;

   /* if text class exists in the textblock styles for this part,
      add the edje to the tc member list */
   desc = (Edje_Part_Description_Text *)pt->default_desc;
   style = edje_string_get(&desc->text.style);
   stl = _edje_textblock_style_search(ed, style);
   _edje_textblock_style_member_add(ed, stl);

   /* If any other classes exist add them */
   for (i = 0; i < pt->other.desc_count; ++i)
     {
        desc = (Edje_Part_Description_Text *)pt->other.desc[i];
        style = edje_string_get(&desc->text.style);
        stl = _edje_textblock_style_search(ed, style);
        _edje_textblock_style_member_add(ed, stl);
     }
}

void
_edje_textblock_styles_del(Edje *ed, Edje_Part *pt)
{
   Edje_Part_Description_Text *desc;
   Edje_Style *stl = NULL;
   const char *style;
   unsigned int i;

   if (pt->type != EDJE_PART_TYPE_TEXTBLOCK) return;

   desc = (Edje_Part_Description_Text *)pt->default_desc;
   style = edje_string_get(&desc->text.style);
   if (style)
     {
        Eina_List *l;

        EINA_LIST_FOREACH(ed->file->styles, l, stl)
          {
             if ((stl->name) && (!strcmp(stl->name, style))) break;
             stl = NULL;
          }
     }
   if (stl)
     {
        Edje_Style_Tag *tag;
        Eina_List *l;

        EINA_LIST_FOREACH(stl->tags, l, tag)
          {
             if (tag->text_class)
                efl_observable_observer_del(_edje_text_class_member, tag->text_class, ed->obj);

             if (tag->color_class)
                efl_observable_observer_del(_edje_color_class_member,
                      tag->color_class, ed->obj);
          }
     }

   for (i = 0; i < pt->other.desc_count; ++i)
     {
        desc = (Edje_Part_Description_Text *)pt->other.desc[i];
        style = edje_string_get(&desc->text.style);
        if (style)
          {
             Eina_List *l;

             EINA_LIST_FOREACH(ed->file->styles, l, stl)
               {
                  if ((stl->name) && (!strcmp(stl->name, style))) break;
                  stl = NULL;
               }
          }
        if (stl)
          {
             Edje_Style_Tag *tag;
             Eina_List *l;

             EINA_LIST_FOREACH(stl->tags, l, tag)
               {
                  if (tag->text_class)
                    efl_observable_observer_del(_edje_text_class_member, tag->text_class, ed->obj);
               }
          }
     }
}

void
_edje_textblock_styles_cache_free(Edje *ed, const char *text_class, const char *color_class)
{
   Eina_List *l, *ll;
   Edje_Style *stl;

   if (!ed->file) return;
   if (!text_class && !color_class) return;

   EINA_LIST_FOREACH(ed->file->styles, l, stl)
     {
        Edje_Style_Tag *tag;
        Eina_Bool found = EINA_FALSE;

        EINA_LIST_FOREACH(stl->tags, ll, tag)
          {
             if (!tag->text_class && !tag->color_class) continue;

             if (tag->text_class && text_class &&
                 !strcmp(tag->text_class, text_class))
               {
                  found = EINA_TRUE;
                  break;
               }

             if (tag->color_class && color_class &&
                 !strcmp(tag->color_class, color_class))
               {
                  found = EINA_TRUE;
                  break;
               }
          }

        if (found)
          stl->cache = EINA_FALSE;
     }
}

/* When we get to here the edje file had been read into memory
 * the name of the style is established as well as the name and
 * data for the tags.  This function will create the Evas_Style
 * object for each style. The style is composed of a base style
 * followed by a list of tags.
 */
void
_edje_textblock_style_parse_and_fix(Edje_File *edf)
{
   Eina_Strbuf *txt = NULL;
   Eina_List *l, *ll;
   Edje_Style *stl;

   EINA_LIST_FOREACH(edf->styles, l, stl)
     {
        Edje_Style_Tag *tag;
        char *fontset = NULL, *fontsource = NULL, *ts;

        if (stl->style) break;

        if (!txt)
          txt = eina_strbuf_new();

        stl->style = evas_textblock_style_new();
        evas_textblock_style_set(stl->style, NULL);

        if (_edje_fontset_append)
          fontset = eina_str_escape(_edje_fontset_append);
        fontsource = eina_str_escape(edf->path);

        /* Build the style from each tag */
        EINA_LIST_FOREACH(stl->tags, ll, tag)
          {
             if (!tag->key) continue;

             /* Add Tag Key */
             eina_strbuf_append(txt, tag->key);
             eina_strbuf_append(txt, "='");

             ts = _edje_format_reparse(edf, tag->value, &(tag));

             /* Add and Handle tag parsed data */
             if (ts)
               {
                  if (edf->allocated_strings &&
                      eet_dictionary_string_check(eet_dictionary_get(edf->ef), tag->value) == 0)
                    eina_stringshare_del(tag->value);
                  tag->value = eina_stringshare_add(ts);
                  eina_strbuf_append(txt, tag->value);
                  free(ts);
               }

             if (!strcmp(tag->key, "DEFAULT"))
               {
                  if (fontset)
                    {
                       eina_strbuf_append(txt, " ");
                       eina_strbuf_append(txt, "font_fallbacks=");
                       eina_strbuf_append(txt, fontset);
                    }
                  eina_strbuf_append(txt, " ");
                  eina_strbuf_append(txt, "font_source=");
                  eina_strbuf_append(txt, fontsource);
               }
             if (tag->font_size > 0)
               {
                  char font_size[32];

                  snprintf(font_size, sizeof(font_size), "%f", tag->font_size);
                  eina_strbuf_append(txt, " ");
                  eina_strbuf_append(txt, "font_size=");
                  eina_strbuf_append(txt, font_size);
               }
             if (tag->color)
               {
                  eina_strbuf_append(txt, " ");
                  eina_strbuf_append(txt, "color=");
                  eina_strbuf_append_escaped(txt, tag->color);
               }
             if (tag->outline_color)
               {
                  eina_strbuf_append(txt, " ");
                  eina_strbuf_append(txt, "outline_color=");
                  eina_strbuf_append_escaped(txt, tag->outline_color);
               }
             if (tag->shadow_color)
               {
                  eina_strbuf_append(txt, " ");
                  eina_strbuf_append(txt, "shadow_color=");
                  eina_strbuf_append_escaped(txt, tag->shadow_color);
               }
             /* Add font name last to save evas from multiple loads */
             if (tag->font)
               {
                  eina_strbuf_append(txt, " ");
                  eina_strbuf_append(txt, "font=");
                  eina_strbuf_append_escaped(txt, tag->font);
               }
             eina_strbuf_append(txt, "'");
          }
        if (fontset) free(fontset);
        if (fontsource) free(fontsource);

        /* Configure the style */
        evas_textblock_style_set(stl->style, eina_strbuf_string_get(txt));
        eina_strbuf_reset(txt);
     }
   if (txt)
     eina_strbuf_free(txt);
}

void
_edje_textblock_style_cleanup(Edje_File *edf)
{
   Edje_Style *stl;

   EINA_LIST_FREE(edf->styles, stl)
     {
        Edje_Style_Tag *tag;

        EINA_LIST_FREE(stl->tags, tag)
          {
             if (edf->allocated_strings &&
                 tag->value &&
                 eet_dictionary_string_check(eet_dictionary_get(edf->ef), tag->value) == 0)
               eina_stringshare_del(tag->value);
             if (edf->free_strings)
               {
                  if (tag->key) eina_stringshare_del(tag->key);
/*                FIXME: Find a proper way to handle it. */
                  if (tag->text_class) eina_stringshare_del(tag->text_class);
                  if (tag->font) eina_stringshare_del(tag->font);
               }
             free(tag);
          }
        if (edf->free_strings && stl->name) eina_stringshare_del(stl->name);
        if (stl->style) evas_textblock_style_free(stl->style);
        free(stl);
     }
}

