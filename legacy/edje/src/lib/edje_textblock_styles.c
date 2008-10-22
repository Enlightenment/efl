/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "edje_private.h"

static int
_edje_font_is_embedded(Edje_File *edf, char *font)
{
   if (!edf->font_dir) return 0;
   return 1;
}

/* Put a \ before and Space( ), \ or ' in a string.
 * A newly allocated string is returned.
 */
static char *
_edje_str_escape(const char *str)
{
   char *s2, *d;
   const char *s;

   s2 = malloc((strlen(str) * 2) + 1);
   if (!s2) return NULL;
   for (s = str, d = s2; *s != 0; s++, d++)
     {
	if ((*s == ' ') || (*s == '\\') || (*s == '\''))
	  {
	     *d = '\\';
	     d++;
	  }
	*d = *s;
     }
   *d = 0;
   return s2;
}

static void
_edje_format_param_parse(char *item, char **key, char **val)
{
   char *p;
   char *k, *v;

   p = strchr(item, '=');
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
   char *item, *ds;
   const char *p;
   const char *ss;
   const char *s1 = NULL;
   const char *s2 = NULL;

   p = *s;
   if ((!p) || (*p == 0)) return NULL;
   for (;;)
     {
	if (!s1)
	  {
	     if (*p != ' ') s1 = p;
	     if (*p == 0) break;
	  }
	else if (!s2)
	  {
	     if ((p > *s) && (p[-1] != '\\'))
	       {
		  if (*p == ' ') s2 = p;
	       }
	     if (*p == 0) s2 = p;
	  }
	p++;
	if (s1 && s2)
	  {
	     item = malloc(s2 - s1 + 1);
	     if (item)
	       {
		  ds = item;
		  for (ds = item, ss = s1; ss < s2; ss++, ds++)
		    {
		       if ((*ss == '\\') && (ss < (s2 - 1))) ss++;
		       *ds = *ss;
		    }
		  *ds = 0;
	       }
	     *s = s2;
	     return item;
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
_edje_strbuf_append(char *s, const char *s2, int *len, int *alloc)
{
   int l2;
   int tlen;

   if (!s2) return s;
   l2 = strlen(s2);
   tlen = *len + l2;
   if (tlen > *alloc)
     {
	char *ts;
	int talloc;

	talloc = ((tlen + 31) >> 5) << 5;
	ts = realloc(s, talloc + 1);
	if (!ts) return s;
	s = ts;
	*alloc = talloc;
     }
   strcpy(s + *len, s2);
   *len = tlen;
   return s;
}

static char *
_edje_strbuf_append_escaped(char *s, const char *unescaped_s, int *len, int *alloc)
{
   char *tmp;

   tmp = _edje_str_escape(unescaped_s);
   if (!tmp)
     return s;

   s = _edje_strbuf_append(s, tmp, len, alloc);
   free(tmp);

   return s;
}

static char *
_edje_format_reparse(Edje_File *edf, const char *str, Edje_Style_Tag **tag_ret)
{
   char *s2;
   char *item;
   char *newstr = NULL;
   const char *s;
   int newlen = 0, newalloc = 0;

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
			    char     *tmpstr = NULL;
			    int       tmplen = 0;
			    int       tmpalloc = 0;

			    tmpstr = _edje_strbuf_append(tmpstr, "fonts/", &tmplen, &tmpalloc);
			    tmpstr = _edje_strbuf_append(tmpstr, val, &tmplen, &tmpalloc);
			    (*tag_ret)->font = eina_stringshare_add(tmpstr);
			    free(tmpstr);
			 }
		       else
			 {
			    (*tag_ret)->font = eina_stringshare_add(val);
			 }
		    }
	       }
	     else /* Otherwise add to tag buffer */
	       {
		  s2 = _edje_str_escape(item);
		  if (s2)
		    {
		       if (newstr) newstr = _edje_strbuf_append(newstr, " ", &newlen, &newalloc);
		       newstr = _edje_strbuf_append(newstr, s2, &newlen, &newalloc);
		       free(s2);
		    }
	       }
	     free(key);
	     free(val);
	  }
	else
	  {
	     if (newstr) newstr = _edje_strbuf_append(newstr, " ", &newlen, &newalloc);
	     newstr = _edje_strbuf_append(newstr, item, &newlen, &newalloc);
	  }
	free(item);
     }
   return newstr;
}

/* Update all evas_styles which are in an edje
 *
 * @param ed	The edje containing styles which need to be updated
 */
void
_edje_textblock_style_all_update(Edje *ed)
{
   Eina_List *l, *ll;
   Edje_Style *stl;

   if (!ed->file) return;

   EINA_LIST_FOREACH(ed->file->styles, l, stl)
     {

	Edje_Style_Tag *tag;
	Edje_Text_Class *tc;
	char *buf = NULL;
	int bufalloc = 0;
	int buflen = 0;
	int found = 0;
	char *fontset = NULL, *fontsource = NULL;

	/* Make sure the style is already defined */
	if (!stl->style) break;

	/* Make sure the style contains a text_class */
	EINA_LIST_FOREACH(stl->tags, ll, tag)
	  if (tag->text_class)
	    found = 1;

	/* No text classes , goto next style */
	if (!found) continue;
	found = 0;

	if (_edje_fontset_append)
	  fontset = _edje_str_escape(_edje_fontset_append);
	fontsource = _edje_str_escape(ed->file->path);

	/* Build the style from each tag */
	EINA_LIST_FOREACH(stl->tags, ll, tag)
	  {
	     if (!tag->key) continue;

	     /* Add Tag Key */
	     buf = _edje_strbuf_append(buf, tag->key, &buflen, &bufalloc);
	     buf = _edje_strbuf_append(buf, "='", &buflen, &bufalloc);

	     /* Configure fonts from text class if it exists */
	     if ((tc = _edje_text_class_find(ed, tag->text_class)))
	       {
		  /* Only update if not clearing, If clear leave it at zero */
		  if (tc->font) found = 1;
	       }

	     /* Add and Ha`ndle tag parsed data */
	     buf = _edje_strbuf_append(buf, tag->value, &buflen, &bufalloc);

	     if (!strcmp(tag->key, "DEFAULT"))
	       {
		  if (fontset)
		    {
		       buf = _edje_strbuf_append(buf, " ", &buflen, &bufalloc);
		       buf = _edje_strbuf_append(buf, "font_fallbacks=", &buflen, &bufalloc);
		       buf = _edje_strbuf_append(buf, fontset, &buflen, &bufalloc);
		    }
		  buf = _edje_strbuf_append(buf, " ", &buflen, &bufalloc);
		  buf = _edje_strbuf_append(buf, "font_source=", &buflen, &bufalloc);
		  buf = _edje_strbuf_append(buf, fontsource, &buflen, &bufalloc);
	       }
	     if (tag->font_size != 0)
	       {
		  char font_size[32];

		  if (found)
		    snprintf(font_size, sizeof(font_size), "%f", (double) _edje_text_size_calc(tag->font_size, tc));
		  else
		    snprintf(font_size, sizeof(font_size), "%f", tag->font_size);

		  buf = _edje_strbuf_append(buf, " ", &buflen, &bufalloc);
		  buf = _edje_strbuf_append(buf, "font_size=", &buflen, &bufalloc);
		  buf = _edje_strbuf_append(buf, font_size, &buflen, &bufalloc);
	       }
	     /* Add font name last to save evas from multiple loads */
	     if (tag->font)
	       {
		  const char *f;

		  buf = _edje_strbuf_append(buf, " ", &buflen, &bufalloc);
		  buf = _edje_strbuf_append(buf, "font=", &buflen, &bufalloc);

		  f = (found) ? tc->font : tag->font;
		  buf = _edje_strbuf_append_escaped(buf, f, &buflen, &bufalloc);
	       }
	     found = 0;

	     buf = _edje_strbuf_append(buf, "'", &buflen, &bufalloc);
	  }
	if (fontset) free(fontset);
	if (fontsource) free(fontsource);

	/* Configure the style */
	evas_textblock_style_set(stl->style, buf);
	free(buf);
     }
}

void
_edje_textblock_styles_add(Edje *ed)
{
   Eina_List *l, *ll;
   Edje_Style *stl;

   if (!ed->file) return;

   EINA_LIST_FOREACH(ed->file->styles, l, stl)
     {
	Edje_Style_Tag *tag;

	/* Make sure the style contains the text_class */
	EINA_LIST_FOREACH(stl->tags, ll, tag)
	  {
	    if (!tag->text_class) continue;
	    _edje_text_class_member_add(ed, tag->text_class);
	  }
     }
}

void
_edje_textblock_styles_del(Edje *ed)
{
   Eina_List *l, *ll;
   Edje_Style *stl;

   if (!ed->file) return;

   EINA_LIST_FOREACH(ed->file->styles, l, stl)
     {
	Edje_Style_Tag *tag;

	/* Make sure the style contains the text_class */
	EINA_LIST_FOREACH(stl->tags, ll, tag)
	  {
	     if (!tag->text_class) continue;
	     _edje_text_class_member_del(ed, tag->text_class);
	  }
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
   Eina_List *l, *ll;
   Edje_Style *stl;

   EINA_LIST_FOREACH(edf->styles, l, stl)
     {
	Edje_Style_Tag *tag;
	char *buf = NULL;
	int bufalloc = 0;
	int buflen = 0;
	char *fontset = NULL, *fontsource = NULL, *ts;

	if (stl->style) break;

	stl->style = evas_textblock_style_new();
	evas_textblock_style_set(stl->style, buf);

	if (_edje_fontset_append)
	  fontset = _edje_str_escape(_edje_fontset_append);
	fontsource = _edje_str_escape(edf->path);

	/* Build the style from each tag */
	EINA_LIST_FOREACH(stl->tags, ll, tag)
	  {
	     if (!tag->key) continue;

	     /* Add Tag Key */
	     buf = _edje_strbuf_append(buf, tag->key, &buflen, &bufalloc);
	     buf = _edje_strbuf_append(buf, "='", &buflen, &bufalloc);

	     ts = _edje_format_reparse(edf, tag->value, &(tag));

	     /* Add and Handle tag parsed data */
	     if (ts)
	       {
		  if (eet_dictionary_string_check(eet_dictionary_get(edf->ef), tag->value) == 0)
		    eina_stringshare_del(tag->value);
		  tag->value = eina_stringshare_add(ts);
		  buf = _edje_strbuf_append(buf, tag->value, &buflen, &bufalloc);
		  free(ts);
	       }

	     if (!strcmp(tag->key, "DEFAULT"))
	       {
		  if (fontset)
		    {
		       buf = _edje_strbuf_append(buf, " ", &buflen, &bufalloc);
		       buf = _edje_strbuf_append(buf, "font_fallbacks=", &buflen, &bufalloc);
		       buf = _edje_strbuf_append(buf, fontset, &buflen, &bufalloc);
		    }
		  buf = _edje_strbuf_append(buf, " ", &buflen, &bufalloc);
		  buf = _edje_strbuf_append(buf, "font_source=", &buflen, &bufalloc);
		  buf = _edje_strbuf_append(buf, fontsource, &buflen, &bufalloc);
	       }
	     if (tag->font_size > 0)
	       {
		  char font_size[32];

		  snprintf(font_size, sizeof(font_size), "%f", tag->font_size);
		  buf = _edje_strbuf_append(buf, " ", &buflen, &bufalloc);
		  buf = _edje_strbuf_append(buf, "font_size=", &buflen, &bufalloc);
		  buf = _edje_strbuf_append(buf, font_size, &buflen, &bufalloc);
	       }
	     /* Add font name last to save evas from multiple loads */
	     if (tag->font)
	       {
		  buf = _edje_strbuf_append(buf, " ", &buflen, &bufalloc);
		  buf = _edje_strbuf_append(buf, "font=", &buflen, &bufalloc);
		  buf = _edje_strbuf_append_escaped(buf, tag->font, &buflen, &bufalloc);
	       }
	     buf = _edje_strbuf_append(buf, "'", &buflen, &bufalloc);
	  }
	if (fontset) free(fontset);
	if (fontsource) free(fontsource);

	/* Configure the style */
	evas_textblock_style_set(stl->style, buf);
	free(buf);
     }
}

void
_edje_textblock_style_cleanup(Edje_File *edf)
{
   while (edf->styles)
     {
	Edje_Style *stl;

	stl = edf->styles->data;
	edf->styles = eina_list_remove_list(edf->styles, edf->styles);
	while (stl->tags)
	  {
	     Edje_Style_Tag *tag;

	     tag = stl->tags->data;
	     stl->tags = eina_list_remove_list(stl->tags, stl->tags);
             if (edf->free_strings)
               {
                  if (tag->key) eina_stringshare_del(tag->key);
/*                FIXME: Find a proper way to handle it. */
                  if (tag->value) eina_stringshare_del(tag->value);
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
