/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "Edje.h"
#include "edje_private.h"

static int
_edje_font_is_embedded(Edje_File *edf, char *font)
{
   Evas_List *l;
   
   if (!edf->font_dir) return 0;
   for (l = edf->font_dir->entries; l; l = l->next)
     {
	Edje_Font_Directory_Entry *fnt = l->data;
	
	if ((fnt->entry) && (!strcmp(fnt->entry, font)))
	  return 1;
     }
   return 1;
}

#if 0
static char *
_edje_str_deescape(char *str)
{
   char *s2, *s, *d;
   
   s2 = malloc(strlen(str) + 1);
   if (!s2) return NULL;
   for (s = str, d = s2; *s != 0; s++, d++)
     {
	if ((*s == '\\') && (s[1] != 0)) s++;
	*d = *s;
     }
   *d = 0;
   return s2;
}
#endif

static char *
_edje_str_escape(char *str)
{
   char *s2, *s, *d;
   
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
_edje_format_parse(char **s)
{
   char *p, *item, *ss, *ds;
   char *s1 = NULL, *s2 = NULL;
   
   p = *s;
   if (*p == 0) return NULL;
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
_edje_strbuf_append(char *s, char *s2, int *len, int *alloc)
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
_edje_format_reparse(Edje_File *edf, char *str)
{
   char *s, *s2;
   char *item;
   char *newstr = NULL;
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
	     else if (!strcmp(key, "font"))
	       {
		  if (_edje_font_is_embedded(edf, val))
		    {
		       if (newstr) newstr = _edje_strbuf_append(newstr, " ", &newlen, &newalloc);
		       newstr = _edje_strbuf_append(newstr, key, &newlen, &newalloc);
		       newstr = _edje_strbuf_append(newstr, "=fonts/", &newlen, &newalloc);
		       s2 = _edje_str_escape(val);
		       if (s2)
			 {
			    newstr = _edje_strbuf_append(newstr, s2, &newlen, &newalloc);
			    free(s2);
			 }
		    }
		  else
		    {
		       s2 = _edje_str_escape(item);
		       if (s2)
			 {
			    if (newstr) newstr = _edje_strbuf_append(newstr, " ", &newlen, &newalloc);
			    newstr = _edje_strbuf_append(newstr, s2, &newlen, &newalloc);
			    free(s2);
			 }
		    }
	       }
	     else
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

void
_edje_textblock_style_parse_and_fix(Edje_File *edf)
{
   Evas_List *l, *ll;
   
   for (l = edf->styles; l; l = l->next)
     {
	Edje_Style *stl;
	Edje_Style_Tag *tag;
	char *buf = NULL;
	int len = 0;
	int def_done;
	char *fontset = NULL, *fontsource = NULL, *ts;
	
	stl = l->data;
	if (stl->style) break;
	stl->style = evas_textblock_style_new();
	evas_textblock_style_set(stl->style, (const char *)buf);
	def_done = 0;
	/* FIXME: i think we have no choice by to parse the style line,
	 * look for font= tags and IF that font is in the .edj then prepend
	 * a "fonts/" to it to it's found
	 */
	if (_edje_fontset_append)
	  fontset = _edje_str_escape(_edje_fontset_append);
	fontsource = _edje_str_escape(edf->path);
	
	for (ll = stl->tags; ll; ll = ll->next)
	  {
	     tag = ll->data;
	     len += strlen(tag->key);
	     len += 1;
	     len += 1;
	     
	     ts = _edje_format_reparse(edf, tag->value);
	     if (ts)
	       {
		  len += strlen(ts);
		  free(ts);
		  len += 1;
	       }       
	  }
	
	if (fontset)
	  {
	     len += 1 + strlen("font_fallbacks=") + strlen(fontset);
	  }
	len += 1 + strlen("font_source=") + strlen(edf->path);
	buf = malloc(len + 1);
	buf[0] = 0;
	for (ll = stl->tags; ll; ll = ll->next)
	  {
	     tag = ll->data;
	     strcat(buf, tag->key);
	     strcat(buf, "='");
	     ts = _edje_format_reparse(edf, tag->value);
	     if (ts)
	       {
		  strcat(buf, ts);
		  free(ts);
	       }
	     if ((!def_done) && (!strcmp(tag->key, "DEFAULT")))
	       {
		  if (fontset)
		    {
		       strcat(buf, " ");
		       strcat(buf, "font_fallbacks=");
		       strcat(buf, fontset);
		    }
		  strcat(buf, " ");
		  strcat(buf, "font_source=");
		  strcat(buf, fontsource);
		  def_done = 1;
	       }
	     strcat(buf, "'");
	  }
	if (fontset) free(fontset);
	if (fontsource) free(fontsource);
	evas_textblock_style_set(stl->style, (const char *)buf);
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
	edf->styles = evas_list_remove_list(edf->styles, edf->styles);
	while (stl->tags)
	  {
	     Edje_Style_Tag *tag;
	     
	     tag = stl->tags->data;
	     stl->tags = evas_list_remove_list(stl->tags, stl->tags);
	     if (tag->key) evas_stringshare_del(tag->key);
	     if (tag->value) evas_stringshare_del(tag->value);
	     free(tag);
	  }
	if (stl->name) evas_stringshare_del(stl->name);
	if (stl->style) evas_textblock_style_free(stl->style);
	free(stl);
     }
}
