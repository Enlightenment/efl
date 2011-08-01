#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#ifdef BUILD_FONT_LOADER_EET
#include <Eet.h>
#endif

#ifdef HAVE_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif

#include "evas_common.h"
#include "evas_private.h"

/* font dir cache */
static Eina_Hash *font_dirs = NULL;
static Eina_List *fonts_cache = NULL;
static Eina_List *fonts_zero = NULL;

typedef struct _Fndat Fndat;

struct _Fndat
{
   Evas_Font_Description *fdesc;
   const char      *source;
   Evas_Font_Size   size;
   Evas_Font_Set   *font;
   int              ref;
   Font_Rend_Flags  wanted_rend;

#ifdef HAVE_FONTCONFIG
   FcFontSet *set;
   FcPattern *p_nm;
#endif
};

/* private methods for font dir cache */
static Eina_Bool font_cache_dir_free(const Eina_Hash *hash, const void *key, void *data, void *fdata);
static Evas_Font_Dir *object_text_font_cache_dir_update(char *dir, Evas_Font_Dir *fd);
static Evas_Font *object_text_font_cache_font_find_x(Evas_Font_Dir *fd, char *font);
static Evas_Font *object_text_font_cache_font_find_file(Evas_Font_Dir *fd, char *font);
static Evas_Font *object_text_font_cache_font_find_alias(Evas_Font_Dir *fd, char *font);
static Evas_Font *object_text_font_cache_font_find(Evas_Font_Dir *fd, char *font);
static Evas_Font_Dir *object_text_font_cache_dir_add(char *dir);
static void object_text_font_cache_dir_del(char *dir, Evas_Font_Dir *fd);
static int evas_object_text_font_string_parse(char *buffer, char dest[14][256]);

#ifdef HAVE_FONTCONFIG
static int fc_init = 0;
#endif

void
evas_font_dir_cache_free(void)
{
   if (!font_dirs) return;

   eina_hash_foreach(font_dirs, font_cache_dir_free, NULL);
   eina_hash_free(font_dirs);
   font_dirs = NULL;

#ifdef HAVE_FONTCONFIG
/* this is bad i got a:
 * fccache.c:512: FcCacheFini: Assertion fcCacheChains[i] == ((void *)0)' failed.   
 * 
 * all i can do for now is shut this puppy down. butthat breaks, so disable
 * it as in reality - there is little reason to care about the memory not
 * being freed etc.
 * 
 *   fc_init--;
 *   if (fc_init == 0) FcFini();
 */
#endif
}

const char *
evas_font_dir_cache_find(char *dir, char *font)
{
   Evas_Font_Dir *fd = NULL;

   if (!font_dirs) font_dirs = eina_hash_string_superfast_new(NULL);
   else fd = eina_hash_find(font_dirs, dir);
   fd = object_text_font_cache_dir_update(dir, fd);
   if (fd)
     {
	Evas_Font *fn;

	fn = object_text_font_cache_font_find(fd, font);
	if (fn)
	  {
	     return fn->path;
	  }
     }
   return NULL;
}

static Eina_List *
evas_font_set_get(const char *name)
{
   Eina_List *fonts = NULL;
   char *p;

   p = strchr(name, ',');
   if (!p)
     {
	fonts = eina_list_append(fonts, eina_stringshare_add(name));
     }
   else
     {
	const char *pp;
	char *nm;

	pp = name;
	while (p)
	  {
	     nm = alloca(p - pp + 1);
	     strncpy(nm, pp, p - pp);
	     nm[p - pp] = 0;
	     fonts = eina_list_append(fonts, eina_stringshare_add(nm));
	     pp = p + 1;
	     p = strchr(pp, ',');
	     if (!p) fonts = eina_list_append(fonts, eina_stringshare_add(pp));
	  }
     }
   return fonts;
}

void
evas_fonts_zero_free(Evas *evas)
{
   Fndat *fd;

   EINA_LIST_FREE(fonts_zero, fd)
     {
        if (fd->fdesc) evas_font_desc_unref(fd->fdesc);
	if (fd->source) eina_stringshare_del(fd->source);
	evas->engine.func->font_free(evas->engine.data.output, fd->font);
#ifdef HAVE_FONTCONFIG
	if (fd->set) FcFontSetDestroy(fd->set);
	if (fd->p_nm) FcPatternDestroy(fd->p_nm);
#endif
	free(fd);
     }
}

void
evas_fonts_zero_presure(Evas *evas)
{
   Fndat *fd;

   while (fonts_zero
	  && eina_list_count(fonts_zero) > 4) /* 4 is arbitrary */
     {
	fd = eina_list_data_get(fonts_zero);

	if (fd->ref != 0) break;
	fonts_zero = eina_list_remove_list(fonts_zero, fonts_zero);

        if (fd->fdesc) evas_font_desc_unref(fd->fdesc);
	if (fd->source) eina_stringshare_del(fd->source);
	evas->engine.func->font_free(evas->engine.data.output, fd->font);
#ifdef HAVE_FONTCONFIG
	if (fd->set) FcFontSetDestroy(fd->set);
	if (fd->p_nm) FcPatternDestroy(fd->p_nm);
#endif
	free(fd);

	if (eina_list_count(fonts_zero) < 5) break;
     }
}

void
evas_font_free(Evas *evas, void *font)
{
   Eina_List *l;
   Fndat *fd;

   EINA_LIST_FOREACH(fonts_cache, l, fd)
     {
	if (fd->font == font)
	  {
	     fd->ref--;
	     if (fd->ref == 0)
	       {
		  fonts_cache = eina_list_remove_list(fonts_cache, l);
		  fonts_zero = eina_list_append(fonts_zero, fd);
	       }
	     break;
	  }
     }
   while (fonts_zero
	  && eina_list_count(fonts_zero) > 42) /* 42 is arbitrary */
     {
	fd = eina_list_data_get(fonts_zero);

	if (fd->ref != 0) break;
	fonts_zero = eina_list_remove_list(fonts_zero, fonts_zero);

        if (fd->fdesc) evas_font_desc_unref(fd->fdesc);
	if (fd->source) eina_stringshare_del(fd->source);
	evas->engine.func->font_free(evas->engine.data.output, fd->font);
#ifdef HAVE_FONTCONFIG
	if (fd->set) FcFontSetDestroy(fd->set);
	if (fd->p_nm) FcPatternDestroy(fd->p_nm);
#endif
	free(fd);

	if (eina_list_count(fonts_zero) < 43) break;
     }
}

static void
evas_font_init(void)
{
   static int done = 0;
   if (done) return;
   done = 1;
#ifdef HAVE_FONTCONFIG
   fc_init++;
   if (fc_init == 1)
     {
        FcInit();
        FcConfigEnableHome(1);
     }
#endif
}

#ifdef HAVE_FONTCONFIG
static Evas_Font_Set *
evas_load_fontconfig(Evas *evas, FcFontSet *set, int size,
      Font_Rend_Flags wanted_rend)
{
   Evas_Font_Set *font = NULL;
   int i;

   /* Do loading for all in family */
   for (i = 0; i < set->nfont; i++)
     {
	FcValue filename;

	FcPatternGet(set->fonts[i], FC_FILE, 0, &filename);

	if (font)
	  evas->engine.func->font_add(evas->engine.data.output, font, (char *)filename.u.s, size, wanted_rend);
	else
	  font = evas->engine.func->font_load(evas->engine.data.output, (char *)filename.u.s, size, wanted_rend);
     }

   return font;
}
#endif

struct _FcPattern {   
   int             num;
   int             size;
   intptr_t        elts_offset;
   int             ref;
};

#ifdef HAVE_FONTCONFIG
/* In sync with Evas_Font_Style, Evas_Font_Weight and Evas_Font_Width */
static int _fc_slant_map[] =
{
   FC_SLANT_ROMAN,
   FC_SLANT_OBLIQUE,
   FC_SLANT_ITALIC
};

static int _fc_weight_map[] =
{
   FC_WEIGHT_NORMAL,
   FC_WEIGHT_THIN,
   FC_WEIGHT_ULTRALIGHT,
   FC_WEIGHT_LIGHT,
   FC_WEIGHT_BOOK,
   FC_WEIGHT_MEDIUM,
   FC_WEIGHT_SEMIBOLD,
   FC_WEIGHT_BOLD,
   FC_WEIGHT_ULTRABOLD,
   FC_WEIGHT_BLACK,
   FC_WEIGHT_EXTRABLACK
};

# ifdef FC_WIDTH
static int _fc_width_map[] =
{
   FC_WIDTH_NORMAL,
   FC_WIDTH_ULTRACONDENSED,
   FC_WIDTH_EXTRACONDENSED,
   FC_WIDTH_CONDENSED,
   FC_WIDTH_SEMICONDENSED,
   FC_WIDTH_SEMIEXPANDED,
   FC_WIDTH_EXPANDED,
   FC_WIDTH_EXTRAEXPANDED,
   FC_WIDTH_ULTRAEXPANDED
};
# endif

#endif

struct _Style_Map
{
   const char *name;
   int type;
};
typedef struct _Style_Map Style_Map;

static Style_Map _style_width_map[] =
{
     {"normal", EVAS_FONT_WIDTH_NORMAL},
     {"ultracondensed", EVAS_FONT_WIDTH_ULTRACONDENSED},
     {"extracondensed", EVAS_FONT_WIDTH_EXTRACONDENSED},
     {"condensed", EVAS_FONT_WIDTH_CONDENSED},
     {"semicondensed", EVAS_FONT_WIDTH_SEMICONDENSED},
     {"semiexpanded", EVAS_FONT_WIDTH_SEMIEXPANDED},
     {"expanded", EVAS_FONT_WIDTH_EXPANDED},
     {"extraexpanded", EVAS_FONT_WIDTH_EXTRAEXPANDED},
     {"ultraexpanded", EVAS_FONT_WIDTH_ULTRAEXPANDED},
};

static Style_Map _style_weight_map[] =
{
     {"normal", EVAS_FONT_WEIGHT_NORMAL},
     {"thin", EVAS_FONT_WEIGHT_THIN},
     {"ultralight", EVAS_FONT_WEIGHT_ULTRALIGHT},
     {"light", EVAS_FONT_WEIGHT_LIGHT},
     {"book", EVAS_FONT_WEIGHT_BOOK},
     {"medium", EVAS_FONT_WEIGHT_MEDIUM},
     {"semibold", EVAS_FONT_WEIGHT_SEMIBOLD},
     {"bold", EVAS_FONT_WEIGHT_BOLD},
     {"ultrabold", EVAS_FONT_WEIGHT_ULTRABOLD},
     {"black", EVAS_FONT_WEIGHT_BLACK},
     {"extrablack", EVAS_FONT_WEIGHT_EXTRABLACK}
};

static Style_Map _style_slant_map[] =
{
     {"normal", EVAS_FONT_SLANT_NORMAL},
     {"oblique", EVAS_FONT_SLANT_OBLIQUE},
     {"italic", EVAS_FONT_SLANT_ITALIC}
};

#define _STYLE_MAP_LEN(x) (sizeof(x) / sizeof(*(x)))
/**
 * @internal
 * Find a certain attribute from the map in the style.
 * @return the index of the found one.
 */
static int
_evas_font_style_find_internal(const char *style, const char *style_end,
      Style_Map _map[], size_t map_len)
{
   size_t i;
   while (style < style_end)
     {
        for (i = 0 ; i < map_len ; i++)
          {
             size_t len;
             const char *cur = _map[i].name;
             len = strlen(cur);
             if (!strncasecmp(style, cur, len) &&
                   (!cur[len] || (cur[len] == ' ')))
               {
                  return _map[i].type;
               }
          }
        style = strchr(style, ' ');
        if (!style)
           break;

        while (*style && (*style == ' '))
           style++;
     }
   return 0;
}

int
evas_font_style_find(const char *start, const char *end,
      Evas_Font_Style style)
{
#define _RET_STYLE(x) \
   return _evas_font_style_find_internal(start, end, \
                   _style_##x##_map, _STYLE_MAP_LEN(_style_##x##_map));
   switch (style)
     {
        case EVAS_FONT_STYLE_SLANT:
           _RET_STYLE(slant);
        case EVAS_FONT_STYLE_WEIGHT:
           _RET_STYLE(weight);
        case EVAS_FONT_STYLE_WIDTH:
           _RET_STYLE(width);
        default:
           return 0;
     }
#undef _RET_STYLE
}

void
evas_font_desc_unref(Evas_Font_Description *fdesc)
{
   if (--(fdesc->ref) == 0)
     {
        eina_stringshare_del(fdesc->name);
        eina_stringshare_del(fdesc->fallbacks);
        eina_stringshare_del(fdesc->lang);
        free(fdesc);
     }
}

Evas_Font_Description *
evas_font_desc_ref(Evas_Font_Description *fdesc)
{
   fdesc->ref++;
   return fdesc;
}

Evas_Font_Description *
evas_font_desc_new(void)
{
   Evas_Font_Description *fdesc;
   fdesc = calloc(1, sizeof(*fdesc));
   fdesc->ref = 1;
   fdesc->new = EINA_TRUE;

   return fdesc;
}

Evas_Font_Description *
evas_font_desc_dup(const Evas_Font_Description *fdesc)
{
   Evas_Font_Description *new;
   new = evas_font_desc_new();
   memcpy(new, fdesc, sizeof(*new));
   new->ref = 1;
   new->new = EINA_TRUE;
   new->name = eina_stringshare_ref(new->name);

   return new;
}

int
evas_font_desc_cmp(const Evas_Font_Description *a,
      const Evas_Font_Description *b)
{
   /* FIXME: Do actual comparison, i.e less than and bigger than. */
   return !((a->name == b->name) && (a->weight == b->weight) &&
         (a->slant == b->slant) && (a->width == b->width) &&
         (a->lang == b->lang));
}

void
evas_font_name_parse(Evas_Font_Description *fdesc, const char *name)
{
   const char *end;

   end = strchr(name, ':');
   if (!end)
      eina_stringshare_replace(&(fdesc->name), name);
   else
      eina_stringshare_replace_length(&(fdesc->name), name, end - name);

   while (end)
     {
        const char *tend;
        name = end;
        end = strchr(end + 1, ':');
        if (!end)
           tend = name + strlen(name);
        else
           tend = end;

        if (!strncmp(name, ":style=", 7))
          {
#define _SET_STYLE(x) \
             fdesc->x = _evas_font_style_find_internal(name + 7, tend, \
                   _style_##x##_map, _STYLE_MAP_LEN(_style_##x##_map));
             _SET_STYLE(slant);
             _SET_STYLE(weight);
             _SET_STYLE(width);
#undef _SET_STYLE
          }
        else if (!strncmp(name, ":lang=", 6))
          {
             const char *tmp = name + 6;
             eina_stringshare_replace_length(&(fdesc->lang), tmp, tend - tmp);
          }
     }
}

void *
evas_font_load(Evas *evas, Evas_Font_Description *fdesc, const char *source, Evas_Font_Size size)
{
#ifdef HAVE_FONTCONFIG
   FcPattern *p_nm = NULL;
   FcFontSet *set = NULL;
#endif

   Evas_Font_Set *font = NULL;
   Eina_List *fonts, *l;
   Fndat *fd;
   char *nm;
   Font_Rend_Flags wanted_rend = 0;

   if (!fdesc) return NULL;
   fdesc->new = EINA_FALSE;

   if (fdesc->slant != EVAS_FONT_SLANT_NORMAL)
      wanted_rend |= FONT_REND_ITALIC;
   if (fdesc->weight == EVAS_FONT_WEIGHT_BOLD)
      wanted_rend |= FONT_REND_BOLD;

   evas_font_init();

   EINA_LIST_FOREACH(fonts_cache, l, fd)
     {
        if (!evas_font_desc_cmp(fdesc, fd->fdesc))
	  {
	     if (((!source) && (!fd->source)) ||
		 ((source) && (fd->source) && (!strcmp(source, fd->source))))
	       {
		  if ((size == fd->size) &&
                        (wanted_rend == fd->wanted_rend))
		    {
		       fonts_cache = eina_list_promote_list(fonts_cache, l);
		       fd->ref++;
		       return fd->font;
		    }
#ifdef HAVE_FONTCONFIG
		  else if (fd->set && fd->p_nm)
		    {
		       font = evas_load_fontconfig(evas, fd->set, size,
                             wanted_rend);
		       goto on_find;
		    }
#endif
	       }
	  }
     }

   EINA_LIST_FOREACH(fonts_zero, l, fd)
     {
        if (!evas_font_desc_cmp(fdesc, fd->fdesc))
	  {
	     if (((!source) && (!fd->source)) ||
		 ((source) && (fd->source) && (!strcmp(source, fd->source))))
	       {
		  if ((size == fd->size) &&
                        (wanted_rend == fd->wanted_rend))
		    {
		       fonts_zero = eina_list_remove_list(fonts_zero, l);
		       fonts_cache = eina_list_prepend(fonts_cache, fd);
		       fd->ref++;
		       return fd->font;
		    }
#ifdef HAVE_FONTCONFIG
		  else if (fd->set && fd->p_nm)
		    {
		       font = evas_load_fontconfig(evas, fd->set, size,
                             wanted_rend);
		       goto on_find;
		    }
#endif
	       }
	  }
     }

   fonts = evas_font_set_get(fdesc->name);
   EINA_LIST_FOREACH(fonts, l, nm) /* Load each font in append */
     {
	if (l == fonts || !font) /* First iteration OR no font */
	  {
#ifdef BUILD_FONT_LOADER_EET
	     if (source) /* Load Font from "eet" source */
	       {
		  Eet_File *ef;
		  char *fake_name;

		  fake_name = evas_file_path_join(source, nm);
		  if (fake_name)
		    {
		       font = evas->engine.func->font_load(evas->engine.data.output, fake_name, size, wanted_rend);
		       if (!font) /* Load from fake name failed, probably not cached */
			 {
			    /* read original!!! */
			    ef = eet_open(source, EET_FILE_MODE_READ);
			    if (ef)
			      {
				 void *fdata;
				 int fsize = 0;

				 fdata = eet_read(ef, nm, &fsize);
				 if ((fdata) && (fsize > 0))
				   {
				      font = evas->engine.func->font_memory_load(evas->engine.data.output, fake_name, size, fdata, fsize, wanted_rend);
				      free(fdata);
				   }
				 eet_close(ef);
			      }
			 }
		       free(fake_name);
		    }
	       }
	     if (!font) /* Source load failed */
	       {
#endif
		  if (evas_file_path_is_full_path((char *)nm)) /* Try filename */
		    font = evas->engine.func->font_load(evas->engine.data.output, (char *)nm, size, wanted_rend);
		  else /* search font path */
		    {
		       Eina_List *ll;
		       char *dir;

		       EINA_LIST_FOREACH(evas->font_path, ll, dir)
			 {
			    const char *f_file;

			    f_file = evas_font_dir_cache_find(dir, (char *)nm);
			    if (f_file)
			      {
				 font = evas->engine.func->font_load(evas->engine.data.output, f_file, size, wanted_rend);
				 if (font) break;
			      }
			 }
		    }
#ifdef BUILD_FONT_LOADER_EET
	       }
#endif
	  }
	else /* Base font loaded, append others */
	  {
#ifdef BUILD_FONT_LOADER_EET
	     void *ok = NULL;

	     if (source)
	       {
		  Eet_File *ef;
		  char *fake_name;

		  fake_name = evas_file_path_join(source, nm);
		  if (fake_name)
		    {
		       /* FIXME: make an engine func */
		       if (!evas->engine.func->font_add(evas->engine.data.output, font, fake_name, size, wanted_rend))
			 {
			    /* read original!!! */
			    ef = eet_open(source, EET_FILE_MODE_READ);
			    if (ef)
			      {
				 void *fdata;
				 int fsize = 0;

				 fdata = eet_read(ef, nm, &fsize);
				 if ((fdata) && (fsize > 0))
				   {
				      ok = evas->engine.func->font_memory_add(evas->engine.data.output, font, fake_name, size, fdata, fsize, wanted_rend);
				      free(fdata);
				   }
				 eet_close(ef);
			      }
			 }
		       else
			 ok = (void *)1;
		       free(fake_name);
		    }
	       }
	     if (!ok)
	       {
#endif
		  if (evas_file_path_is_full_path((char *)nm))
		    evas->engine.func->font_add(evas->engine.data.output, font, (char *)nm, size, wanted_rend);
		  else
		    {
		       Eina_List *ll;
		       char *dir;

		       EINA_LIST_FOREACH(evas->font_path, ll, dir)
			 {
			    const char *f_file;

			    f_file = evas_font_dir_cache_find(dir, (char *)nm);
			    if (f_file)
			      {
				 if (evas->engine.func->font_add(evas->engine.data.output, font, f_file, size, wanted_rend))
				   break;
			      }
			 }
		    }
#ifdef BUILD_FONT_LOADER_EET
	       }
#endif
	  }
	eina_stringshare_del(nm);
     }
   fonts = eina_list_free(fonts);

#ifdef HAVE_FONTCONFIG
   
   if (!font) /* Search using fontconfig */
     {
	FcResult res;

        p_nm = FcPatternBuild (NULL,
              FC_WEIGHT, FcTypeInteger, _fc_weight_map[fdesc->weight],
              FC_SLANT,  FcTypeInteger, _fc_slant_map[fdesc->slant],
#ifdef FC_WIDTH
              FC_WIDTH,  FcTypeInteger, _fc_width_map[fdesc->width],
#endif
              NULL);
        FcPatternAddString (p_nm, FC_FAMILY, (FcChar8*) fdesc->name);

        /* Handle font fallbacks */
        if (fdesc->fallbacks)
          {
             while (1)
               {
                  const char *start, *end;
                  start = fdesc->fallbacks;
                  end = strchr(start, ',');
                  if (end)
                    {
                       char *tmp;
                       tmp = strndup(start, end - start);
                       FcPatternAddString (p_nm, FC_FAMILY, (FcChar8*) start);
                       free(tmp);
                    }
                  else
                    {
                       FcPatternAddString (p_nm, FC_FAMILY, (FcChar8*) start);
                       break;
                    }
               }
          }

        if (fdesc->lang)
           FcPatternAddString (p_nm, FC_LANG, (FcChar8 *) fdesc->lang);

	FcConfigSubstitute(NULL, p_nm, FcMatchPattern);
	FcDefaultSubstitute(p_nm);

	/* do matching */
	set = FcFontSort(NULL, p_nm, FcTrue, NULL, &res);
	if (!set)
	  {
	     ERR("No fontconfig font matches '%s'. It was the last resource, no font found!", fdesc->name);
	     FcPatternDestroy(p_nm);
	     p_nm = NULL;
	  }
	else
          {
             font = evas_load_fontconfig(evas, set, size, wanted_rend);
          }
     }
#endif

 on_find:
   fd = calloc(1, sizeof(Fndat));
   if (fd)
     {
	fd->fdesc = evas_font_desc_ref(fdesc);
	if (source) fd->source = eina_stringshare_add(source);
	fd->font = font;
        fd->wanted_rend = wanted_rend;
	fd->ref = 1;
	fonts_cache = eina_list_prepend(fonts_cache, fd);
#ifdef HAVE_FONTCONFIG
	fd->set = set;
	fd->p_nm = p_nm;
#endif
     }

   if (font)
     evas->engine.func->font_hinting_set(evas->engine.data.output, font,
					 evas->hinting);
   return font;
}

void
evas_font_load_hinting_set(Evas *evas, void *font, int hinting)
{
   evas->engine.func->font_hinting_set(evas->engine.data.output, font,
				       hinting);
}

Eina_List *
evas_font_dir_available_list(const Evas *evas)
{
   Eina_List *l;
   Eina_List *ll;
   Eina_List *available = NULL;
   char *dir;

#ifdef HAVE_FONTCONFIG
   /* Add font config fonts */
   FcPattern *p;
   FcFontSet *set = NULL;
   FcObjectSet *os;
   int i;

   evas_font_init();

   p = FcPatternCreate();
   os = FcObjectSetBuild(FC_FAMILY, FC_STYLE, NULL);

   if (p && os) set = FcFontList(NULL, p, os);

   if (p) FcPatternDestroy(p);
   if (os) FcObjectSetDestroy(os);

   if (set)
     {
	for (i = 0; i < set->nfont; i++)
	  {
	     char *font;

	     font = (char *)FcNameUnparse(set->fonts[i]);
	     available = eina_list_append(available, eina_stringshare_add(font));
	     free(font);
	  }

	FcFontSetDestroy(set);
     }
#endif

   /* Add fonts in evas font_path*/
   if (!evas->font_path)
     return available;

   if (!font_dirs) font_dirs = eina_hash_string_superfast_new(NULL);

   EINA_LIST_FOREACH(evas->font_path, l, dir)
     {
	Evas_Font_Dir *fd;

	fd = eina_hash_find(font_dirs, dir);
	fd = object_text_font_cache_dir_update(dir, fd);
	if (fd && fd->aliases)
	  {
	     Evas_Font_Alias *fa;

	     EINA_LIST_FOREACH(fd->aliases, ll, fa)
	       available = eina_list_append(available, eina_stringshare_add((char *)fa->alias));
	  }
     }

   return available;
}

void
evas_font_dir_available_list_free(Eina_List *available)
{
   while (available)
     {
	eina_stringshare_del(available->data);
	available = eina_list_remove(available, available->data);
     }
}

/* private stuff */
static Eina_Bool
font_cache_dir_free(const Eina_Hash *hash __UNUSED__, const void *key, void *data, void *fdata __UNUSED__)
{
   object_text_font_cache_dir_del((char *) key, data);
   return 1;
}

static Evas_Font_Dir *
object_text_font_cache_dir_update(char *dir, Evas_Font_Dir *fd)
{
   DATA64 mt;
   char *tmp;

   if (fd)
     {
	mt = evas_file_modified_time(dir);
	if (mt != fd->dir_mod_time)
	  {
	     object_text_font_cache_dir_del(dir, fd);
	     eina_hash_del(font_dirs, dir, fd);
	  }
	else
	  {
	     tmp = evas_file_path_join(dir, "fonts.dir");
	     if (tmp)
	       {
		  mt = evas_file_modified_time(tmp);
		  free(tmp);
		  if (mt != fd->fonts_dir_mod_time)
		    {
		       object_text_font_cache_dir_del(dir, fd);
		       eina_hash_del(font_dirs, dir, fd);
		    }
		  else
		    {
		       tmp = evas_file_path_join(dir, "fonts.alias");
		       if (tmp)
			 {
			    mt = evas_file_modified_time(tmp);
			    free(tmp);
			 }
		       if (mt != fd->fonts_alias_mod_time)
			 {
			    object_text_font_cache_dir_del(dir, fd);
			    eina_hash_del(font_dirs, dir, fd);
			 }
		       else
			 return fd;
		    }
	       }
	  }
     }
   return object_text_font_cache_dir_add(dir);
}

static Evas_Font *
object_text_font_cache_font_find_x(Evas_Font_Dir *fd, char *font)
{
   Eina_List *l;
   char font_prop[14][256];
   int num;
   Evas_Font *fn;

   num = evas_object_text_font_string_parse(font, font_prop);
   if (num != 14) return NULL;
   EINA_LIST_FOREACH(fd->fonts, l, fn)
     {
	if (fn->type == 1)
	  {
	     int i;
	     int match = 0;

	     for (i = 0; i < 14; i++)
	       {
		  if ((font_prop[i][0] == '*') && (font_prop[i][1] == 0))
		    match++;
		  else
		    {
		       if (!strcasecmp(font_prop[i], fn->x.prop[i])) match++;
		       else break;
		    }
	       }
	     if (match == 14) return fn;
	  }
     }
   return NULL;
}

static Evas_Font *
object_text_font_cache_font_find_file(Evas_Font_Dir *fd, char *font)
{
   Eina_List *l;
   Evas_Font *fn;

   EINA_LIST_FOREACH(fd->fonts, l, fn)
     {
	if (fn->type == 0)
	  {
	     if (!strcasecmp(font, fn->simple.name)) return fn;
	  }
     }
   return NULL;
}

static Evas_Font *
object_text_font_cache_font_find_alias(Evas_Font_Dir *fd, char *font)
{
   Eina_List *l;
   Evas_Font_Alias *fa;

   EINA_LIST_FOREACH(fd->aliases, l, fa)
     if (!strcasecmp(fa->alias, font)) return fa->fn;
   return NULL;
}

static Evas_Font *
object_text_font_cache_font_find(Evas_Font_Dir *fd, char *font)
{
   Evas_Font *fn;

   fn = eina_hash_find(fd->lookup, font);
   if (fn) return fn;
   fn = object_text_font_cache_font_find_alias(fd, font);
   if (!fn) fn = object_text_font_cache_font_find_x(fd, font);
   if (!fn) fn = object_text_font_cache_font_find_file(fd, font);
   if (!fn) return NULL;
   eina_hash_add(fd->lookup, font, fn);
   return fn;
}

static Evas_Font_Dir *
object_text_font_cache_dir_add(char *dir)
{
   Evas_Font_Dir *fd;
   char *tmp, *tmp2;
   Eina_List *fdir;
   Evas_Font *fn;

   fd = calloc(1, sizeof(Evas_Font_Dir));
   if (!fd) return NULL;
   fd->lookup = eina_hash_string_superfast_new(NULL);

   eina_hash_add(font_dirs, dir, fd);

   /* READ fonts.alias, fonts.dir and directory listing */

   /* fonts.dir */
   tmp = evas_file_path_join(dir, "fonts.dir");
   if (tmp)
     {
	FILE *f;

	f = fopen(tmp, "rb");
	if (f)
	  {
	     int num;
	     char fname[4096], fdef[4096];

	     if (fscanf(f, "%i\n", &num) != 1) goto cant_read;
	     /* read font lines */
	     while (fscanf(f, "%4090s %[^\n]\n", fname, fdef) == 2)
	       {
		  char font_prop[14][256];
		  int i;

		  /* skip comments */
		  if ((fdef[0] == '!') || (fdef[0] == '#')) continue;
		  /* parse font def */
		  num = evas_object_text_font_string_parse((char *)fdef, font_prop);
		  if (num == 14)
		    {
		       fn = calloc(1, sizeof(Evas_Font));
		       if (fn)
			 {
			    fn->type = 1;
			    for (i = 0; i < 14; i++)
			      fn->x.prop[i] = eina_stringshare_add(font_prop[i]);
			    tmp2 = evas_file_path_join(dir, fname);
			    if (tmp2)
			      {
				 fn->path = eina_stringshare_add(tmp2);
				 free(tmp2);
			      }
			    fd->fonts = eina_list_append(fd->fonts, fn);
			 }
		    }
	       }
	     cant_read: ;
	     fclose(f);
	  }
	free(tmp);
     }

   /* directoy listing */
   fdir = evas_file_path_list(dir, "*.ttf", 0);
   while (fdir)
     {
	tmp = evas_file_path_join(dir, fdir->data);
	if (tmp)
	  {
	     fn = calloc(1, sizeof(Evas_Font));
	     if (fn)
	       {
		  char *p;

		  fn->type = 0;
		  tmp2 = alloca(strlen(fdir->data) + 1);
		  strcpy(tmp2, fdir->data);
		  p = strrchr(tmp2, '.');
		  if (p) *p = 0;
		  fn->simple.name = eina_stringshare_add(tmp2);
		  tmp2 = evas_file_path_join(dir, fdir->data);
		  if (tmp2)
		    {
		       fn->path = eina_stringshare_add(tmp2);
		       free(tmp2);
		    }
		  fd->fonts = eina_list_append(fd->fonts, fn);
	       }
	     free(tmp);
	  }
	free(fdir->data);
	fdir = eina_list_remove(fdir, fdir->data);
     }

   /* fonts.alias */
   tmp = evas_file_path_join(dir, "fonts.alias");
   if (tmp)
     {
	FILE *f;

	f = fopen(tmp, "rb");
	if (f)
	  {
	     char fname[4096], fdef[4096];

	     /* read font alias lines */
	     while (fscanf(f, "%4090s %[^\n]\n", fname, fdef) == 2)
	       {
		  Evas_Font_Alias *fa;

		  /* skip comments */
		  if ((fname[0] == '!') || (fname[0] == '#')) continue;
		  fa = calloc(1, sizeof(Evas_Font_Alias));
		  if (fa)
		    {
		       fa->alias = eina_stringshare_add(fname);
		       fa->fn = object_text_font_cache_font_find_x(fd, fdef);
		       if ((!fa->alias) || (!fa->fn))
			 {
			    if (fa->alias) eina_stringshare_del(fa->alias);
			    free(fa);
			 }
		       else
			 fd->aliases = eina_list_append(fd->aliases, fa);
		    }
	       }
	     fclose(f);
	  }
	free(tmp);
     }

   fd->dir_mod_time = evas_file_modified_time(dir);
   tmp = evas_file_path_join(dir, "fonts.dir");
   if (tmp)
     {
	fd->fonts_dir_mod_time = evas_file_modified_time(tmp);
	free(tmp);
     }
   tmp = evas_file_path_join(dir, "fonts.alias");
   if (tmp)
     {
	fd->fonts_alias_mod_time = evas_file_modified_time(tmp);
	free(tmp);
     }

   return fd;
}

static void
object_text_font_cache_dir_del(char *dir __UNUSED__, Evas_Font_Dir *fd)
{
   if (fd->lookup) eina_hash_free(fd->lookup);
   while (fd->fonts)
     {
	Evas_Font *fn;
	int i;

	fn = fd->fonts->data;
	fd->fonts = eina_list_remove(fd->fonts, fn);
	for (i = 0; i < 14; i++)
	  {
	     if (fn->x.prop[i]) eina_stringshare_del(fn->x.prop[i]);
	  }
	if (fn->simple.name) eina_stringshare_del(fn->simple.name);
	if (fn->path) eina_stringshare_del(fn->path);
	free(fn);
     }
   while (fd->aliases)
     {
	Evas_Font_Alias *fa;

	fa = fd->aliases->data;
	fd->aliases = eina_list_remove(fd->aliases, fa);
	if (fa->alias) eina_stringshare_del(fa->alias);
	free(fa);
     }
   free(fd);
}

static int
evas_object_text_font_string_parse(char *buffer, char dest[14][256])
{
   char *p;
   int n, m, i;

   n = 0;
   m = 0;
   p = buffer;
   if (p[0] != '-') return 0;
   i = 1;
   while (p[i])
     {
	dest[n][m] = p[i];
	if ((p[i] == '-') || (m == 255))
	  {
	     dest[n][m] = 0;
	     n++;
	     m = -1;
	  }
	i++;
	m++;
	if (n == 14) return n;
     }
   dest[n][m] = 0;
   n++;
   return n;
}

EAPI void
evas_font_path_clear(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   while (e->font_path)
     {
	eina_stringshare_del(e->font_path->data);
	e->font_path = eina_list_remove(e->font_path, e->font_path->data);
     }
}

EAPI void
evas_font_path_append(Evas *e, const char *path)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (!path) return;
   e->font_path = eina_list_append(e->font_path, eina_stringshare_add(path));
}

EAPI void
evas_font_path_prepend(Evas *e, const char *path)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (!path) return;
   e->font_path = eina_list_prepend(e->font_path, eina_stringshare_add(path));
}

EAPI const Eina_List *
evas_font_path_list(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   return e->font_path;
}

static void
evas_font_object_rehint(Evas_Object *obj)
{
   if (obj->smart.smart)
     {
	EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(obj), obj)
	  evas_font_object_rehint(obj);
     }
   else
     {
	if (!strcmp(obj->type, "text"))
	  _evas_object_text_rehint(obj);
	if (!strcmp(obj->type, "textblock"))
	  _evas_object_textblock_rehint(obj);
     }
}

EAPI void
evas_font_hinting_set(Evas *e, Evas_Font_Hinting_Flags hinting)
{
   Evas_Layer *lay;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   if (e->hinting == hinting) return;
   e->hinting = hinting;

   EINA_INLIST_FOREACH(e->layers, lay)
     {
	Evas_Object *obj;

	EINA_INLIST_FOREACH(lay->objects, obj)
	  evas_font_object_rehint(obj);
     }
}

EAPI Evas_Font_Hinting_Flags
evas_font_hinting_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return EVAS_FONT_HINTING_BYTECODE;
   MAGIC_CHECK_END();
   return e->hinting;
}

EAPI Eina_Bool
evas_font_hinting_can_hint(const Evas *e, Evas_Font_Hinting_Flags hinting)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   if (e->engine.func->font_hinting_can_hint)
     return e->engine.func->font_hinting_can_hint(e->engine.data.output,
						  hinting);
   return EINA_FALSE;
}

EAPI void
evas_font_cache_flush(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   e->engine.func->font_cache_flush(e->engine.data.output);
}

EAPI void
evas_font_cache_set(Evas *e, int size)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if (size < 0) size = 0;
   e->engine.func->font_cache_set(e->engine.data.output, size);
}

EAPI int
evas_font_cache_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();

   return e->engine.func->font_cache_get(e->engine.data.output);
}

EAPI Eina_List *
evas_font_available_list(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();

   return evas_font_dir_available_list(e);
}

EAPI void
evas_font_available_list_free(Evas *e, Eina_List *available)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   evas_font_dir_available_list_free(available);
}

