#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef _WIN32
# include <Evil.h>
#endif

#include <Eet.h>

#ifdef HAVE_FONTCONFIG
#include <fontconfig/fontconfig.h>
#include <fontconfig/fcfreetype.h>
#endif

#include "evas_font.h"

/* General types - used for script type chceking */
#define OPAQUE_TYPE(type) struct __##type { int a; }; \
   typedef struct __##type type

OPAQUE_TYPE(Evas_Font_Set); /* General type for RGBA_Font */
OPAQUE_TYPE(Evas_Font_Instance); /* General type for RGBA_Font_Int */

/* font dir cache */
static Eina_Hash *font_dirs = NULL;
static Eina_List *fonts_cache = NULL;
static Eina_List *fonts_zero = NULL;
static Eina_List *global_font_path = NULL;

typedef struct _Fndat Fndat;

struct _Fndat
{
   Evas_Font_Description *fdesc;
   const char      *source;
   Evas_Font_Size   size;
   Evas_Font_Set   *font;
   int              ref;
   Font_Rend_Flags  wanted_rend;
   Efl_Text_Font_Bitmap_Scalable bitmap_scalable;

#ifdef HAVE_FONTCONFIG
   FcFontSet *set;
   FcPattern *p_nm;

   Eina_Bool file_font : 1; /* Indicates this is a font that uses a file rather than fontconfig. */
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
static FcConfig *fc_config = NULL;
#endif

/* FIXME move these helper function to eina_file or eina_path */
/* get the casefold feature! */
#include <fnmatch.h>
#include <unistd.h>
#include <sys/param.h>
int
_file_path_is_full_path(const char *path)
{
   if (!path) return 0;
#ifdef _WIN32
   if (evil_path_is_absolute(path)) return 1;
#else
   if (path[0] == '/') return 1;
#endif
   return 0;
}

static DATA64
_file_modified_time(const char *file)
{
   struct stat st;

   if (stat(file, &st) < 0) return 0;
   if (st.st_ctime > st.st_mtime) return (DATA64)st.st_ctime;
   else return (DATA64)st.st_mtime;
   return 0;
}

Eina_List *
_file_path_list(char *path, const char *match, int match_case)
{
   Eina_File_Direct_Info *info;
   Eina_Iterator *it;
   Eina_List *files = NULL;
   int flags;

   flags = FNM_PATHNAME;
#ifdef FNM_CASEFOLD
   if (!match_case)
     flags |= FNM_CASEFOLD;
#elif defined FNM_IGNORECASE
   if (!match_case)
     flags |= FNM_IGNORECASE;
#else
/*#warning "Your libc does not provide case-insensitive matching!"*/
#endif

   it = eina_file_direct_ls(path);
   EINA_ITERATOR_FOREACH(it, info)
     {
        if (match)
          {
             if (fnmatch(match, info->path + info->name_start, flags) == 0)
               files = eina_list_append(files, strdup(info->path + info->name_start));
          }
        else
          files = eina_list_append(files, strdup(info->path + info->name_start));
     }
   if (it) eina_iterator_free(it);
   return files;
}

static void
evas_font_init(void)
{
#ifdef HAVE_FONTCONFIG
   if (!fc_config)
     {
        Eina_List *l;
        char *path;

        fc_config = FcInitLoadConfigAndFonts();

        EINA_LIST_FOREACH(global_font_path, l, path)
           FcConfigAppFontAddDir(fc_config, (const FcChar8 *) path);
     }
#endif
}

void
evas_font_dir_cache_free(void)
{
   if (font_dirs)
     {
        eina_hash_foreach(font_dirs, font_cache_dir_free, NULL);
        eina_hash_free(font_dirs);
        font_dirs = NULL;
     }
#ifdef HAVE_FONTCONFIG
   if (fc_config)
     {
        FcConfigDestroy(fc_config);
        fc_config = NULL;
     }
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
evas_fonts_zero_free()
{
   Fndat *fd;

   EINA_LIST_FREE(fonts_zero, fd)
     {
        if (fd->fdesc) evas_font_desc_unref(fd->fdesc);
        if (fd->source) eina_stringshare_del(fd->source);
        evas_common_font_free((RGBA_Font *)fd->font);
#ifdef HAVE_FONTCONFIG
   if (fd->set) FcFontSetDestroy(fd->set);
   if (fd->p_nm) FcPatternDestroy(fd->p_nm);
#endif
        free(fd);
     }
}

void
evas_fonts_zero_pressure()
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
        evas_common_font_free((RGBA_Font *)fd->font);
      #ifdef HAVE_FONTCONFIG
        if (fd->set) FcFontSetDestroy(fd->set);
        if (fd->p_nm) FcPatternDestroy(fd->p_nm);
      #endif
        free(fd);

        if (eina_list_count(fonts_zero) < 5) break;
     }
}

void
evas_font_free(void *font)
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
        evas_common_font_free((RGBA_Font *)fd->font);
      #ifdef HAVE_FONTCONFIG
        if (fd->set) FcFontSetDestroy(fd->set);
        if (fd->p_nm) FcPatternDestroy(fd->p_nm);
      #endif
        free(fd);

        if (eina_list_count(fonts_zero) < 43) break;
     }
}

#ifdef HAVE_FONTCONFIG
static Evas_Font_Set *
_evas_load_fontconfig(Evas_Font_Set *font, FcFontSet *set, int size,
      Font_Rend_Flags wanted_rend, Efl_Text_Font_Bitmap_Scalable bitmap_scalable)
{
   int i;

   /* Do loading for all in family */
   for (i = 0; i < set->nfont; i++)
     {
        FcValue filename;

        if (FcPatternGet(set->fonts[i], FC_FILE, 0, &filename) == FcResultMatch)
          {
             if (font)
               evas_common_font_add((RGBA_Font *)font, (char *)filename.u.s, size, wanted_rend, bitmap_scalable);
             else
               font = (Evas_Font_Set *)evas_common_font_load((char *)filename.u.s, size, wanted_rend, bitmap_scalable);
          }
     }

   return font;
}
#endif

#ifdef HAVE_FONTCONFIG
/* In sync with Evas_Font_Style, Evas_Font_Weight and Evas_Font_Width */
static int _fc_slant_map[] =
{
   FC_SLANT_ROMAN,
   FC_SLANT_OBLIQUE,
   FC_SLANT_ITALIC
};

/* Apparently EXTRABLACK is not always available, hardcode. */
# ifndef FC_WEIGHT_EXTRABLACK
#  define FC_WEIGHT_EXTRABLACK 215
# endif
static int _fc_weight_map[] =
{
   FC_WEIGHT_NORMAL,
   FC_WEIGHT_THIN,
   FC_WEIGHT_ULTRALIGHT,
   FC_WEIGHT_EXTRALIGHT,
   FC_WEIGHT_LIGHT,
   FC_WEIGHT_BOOK,
   FC_WEIGHT_MEDIUM,
   FC_WEIGHT_SEMIBOLD,
   FC_WEIGHT_BOLD,
   FC_WEIGHT_ULTRABOLD,
   FC_WEIGHT_EXTRABOLD,
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

static int _fc_spacing_map[] =
{
   FC_PROPORTIONAL,
   FC_DUAL,
   FC_MONO,
   FC_CHARCELL
};

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
     {"extralight", EVAS_FONT_WEIGHT_EXTRALIGHT},
     {"light", EVAS_FONT_WEIGHT_LIGHT},
     {"book", EVAS_FONT_WEIGHT_BOOK},
     {"medium", EVAS_FONT_WEIGHT_MEDIUM},
     {"semibold", EVAS_FONT_WEIGHT_SEMIBOLD},
     {"bold", EVAS_FONT_WEIGHT_BOLD},
     {"ultrabold", EVAS_FONT_WEIGHT_ULTRABOLD},
     {"extrabold", EVAS_FONT_WEIGHT_EXTRABOLD},
     {"black", EVAS_FONT_WEIGHT_BLACK},
     {"extrablack", EVAS_FONT_WEIGHT_EXTRABLACK}
};

static Style_Map _style_slant_map[] =
{
     {"normal", EVAS_FONT_SLANT_NORMAL},
     {"oblique", EVAS_FONT_SLANT_OBLIQUE},
     {"italic", EVAS_FONT_SLANT_ITALIC}
};

static Style_Map _style_spacing_map[] =
{
     {"proportional", EVAS_FONT_SPACING_PROPORTIONAL},
     {"dualwidth", EVAS_FONT_SPACING_DUAL},
     {"monospace", EVAS_FONT_SPACING_MONO},
     {"charcell", EVAS_FONT_SPACING_CHARCELL}
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
        eina_stringshare_del(fdesc->style);
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
   fdesc->is_new = EINA_TRUE;

   return fdesc;
}

Evas_Font_Description *
evas_font_desc_dup(const Evas_Font_Description *fdesc)
{
   Evas_Font_Description *new;
   new = evas_font_desc_new();
   memcpy(new, fdesc, sizeof(*new));
   new->ref = 1;
   new->is_new = EINA_TRUE;
   new->name = eina_stringshare_ref(new->name);
   new->fallbacks = eina_stringshare_ref(new->fallbacks);
   new->lang = eina_stringshare_ref(new->lang);
   new->style = eina_stringshare_ref(new->style);

   return new;
}

int
evas_font_desc_cmp(const Evas_Font_Description *a,
      const Evas_Font_Description *b)
{
   /* FIXME: Do actual comparison, i.e less than and bigger than. */
   return !((a->name == b->name) && (a->weight == b->weight) &&
            (a->slant == b->slant) && (a->width == b->width) &&
            (a->spacing == b->spacing) && (a->lang == b->lang));
}

const char *
evas_font_lang_normalize(const char *lang)
{
   if (!lang || !strcmp(lang, "none")) return NULL;

   if (!strcmp(lang, "auto"))
     return evas_common_language_from_locale_full_get();

   return lang;
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
#define _SET_STYLE(x, len) \
             fdesc->x = _evas_font_style_find_internal(name + len, tend, \
                   _style_##x##_map, _STYLE_MAP_LEN(_style_##x##_map));
             eina_stringshare_replace_length(&(fdesc->style), name + 7, tend - (name + 7));
             _SET_STYLE(slant, 7);
             _SET_STYLE(weight, 7);
             _SET_STYLE(width, 7);
          }
        else if (!strncmp(name, ":slant=", 7))
          {
             _SET_STYLE(slant, 7);
          }
        else if (!strncmp(name, ":weight=", 8))
          {
             _SET_STYLE(weight, 8);
          }
        else if (!strncmp(name, ":width=", 7))
          {
             _SET_STYLE(width, 7);
          }
        else if (!strncmp(name, ":spacing=", 9))
          {
             _SET_STYLE(spacing, 9);
#undef _SET_STYLE
          }
        else if (!strncmp(name, ":lang=", 6))
          {
             const char *tmp = name + 6;
             eina_stringshare_replace_length(&(fdesc->lang), tmp, tend - tmp);
             eina_stringshare_replace(&(fdesc->lang), evas_font_lang_normalize(fdesc->lang));
          }
        else if (!strncmp(name, ":fallbacks=", 11))
          {
             const char *tmp = name + 11;
             eina_stringshare_replace_length(&(fdesc->fallbacks), tmp, tend - tmp);
          }
     }
}

void *
evas_font_load(const Eina_List *font_paths, int hinting, Evas_Font_Description *fdesc, const char *source, Evas_Font_Size size, Efl_Text_Font_Bitmap_Scalable bitmap_scalable)
{
#ifdef HAVE_FONTCONFIG
   FcPattern *p_nm = NULL;
   FcFontSet *set = NULL;
   Eina_Bool file_font = EINA_FALSE;
#endif

   Evas_Font_Set *font = NULL;
   Eina_List *fonts, *l, *l_next;
   Fndat *fd;
#ifdef HAVE_FONTCONFIG
   Fndat *found_fd = NULL;
#endif
   char *nm;
   Font_Rend_Flags wanted_rend = 0;

   if (!fdesc) return NULL;
   fdesc->is_new = EINA_FALSE;

   if (fdesc->slant != EVAS_FONT_SLANT_NORMAL)
      wanted_rend |= FONT_REND_SLANT;
   if (fdesc->weight == EVAS_FONT_WEIGHT_BOLD)
      wanted_rend |= FONT_REND_WEIGHT;

   evas_font_init();

   EINA_LIST_FOREACH(fonts_cache, l, fd)
     {
        if (!evas_font_desc_cmp(fdesc, fd->fdesc))
          {
              if (((!source) && (!fd->source)) ||
                  ((source) && (fd->source) && (!strcmp(source, fd->source))))
                {
                   if ((size == fd->size) &&
                       (wanted_rend == fd->wanted_rend) &&
                       (bitmap_scalable == fd->bitmap_scalable))
                     {
                        fonts_cache = eina_list_promote_list(fonts_cache, l);
                        fd->ref++;
                        return fd->font;
                     }
                #ifdef HAVE_FONTCONFIG
                   else if (fd->set && fd->p_nm && !fd->file_font)
                     {
                        found_fd = fd;
                     }
                #endif
                }
           }
     }

#ifdef HAVE_FONTCONFIG
   if (found_fd)
     {
        font = _evas_load_fontconfig(font, found_fd->set, size, wanted_rend, bitmap_scalable);
        goto on_find;
     }
#endif

   EINA_LIST_FOREACH_SAFE(fonts_zero, l, l_next, fd)
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
                  else if (fd->set && fd->p_nm && !fd->file_font)
                    {
                                 found_fd = fd;
                    }
               #endif
               }
          }
     }

#ifdef HAVE_FONTCONFIG
   if (found_fd)
     {
        font = _evas_load_fontconfig(font, found_fd->set, size, wanted_rend, bitmap_scalable);
        goto on_find;
     }
#endif

   fonts = evas_font_set_get(fdesc->name);
   EINA_LIST_FOREACH(fonts, l, nm) /* Load each font in append */
     {
        if (l == fonts || !font) /* First iteration OR no font */
          {
             if (source) /* Load Font from "eet" source */
               {
                  Eet_File *ef;
                  char fake_name[PATH_MAX];

                   eina_file_path_join(fake_name, PATH_MAX, source, nm);
                   font = (Evas_Font_Set *)evas_common_font_load(fake_name, size, wanted_rend, bitmap_scalable);
                   if (!font) /* Load from fake name failed, probably not cached */
                     {
                        /* read original!!! */
                        ef = eet_open(source, EET_FILE_MODE_READ);
                        if (ef)
                          {
                             void *fdata;
                             int fsize = 0;

                             fdata = eet_read(ef, nm, &fsize);
                             if (fdata)
                               {
                                  font = (Evas_Font_Set *)evas_common_font_memory_load(source, nm, size, fdata, fsize, wanted_rend, bitmap_scalable);
                                  free(fdata);
                               }
                             eet_close(ef);
                          }
                     }
               }
             if (!font) /* Source load failed */
               {
                  if (_file_path_is_full_path((char *)nm)) /* Try filename */
                    font = (Evas_Font_Set *)evas_common_font_load((char *)nm, size, wanted_rend, bitmap_scalable);
                  else /* search font path */
                    {
                       const Eina_List *ll;
                       char *dir;

                       EINA_LIST_FOREACH(font_paths, ll, dir)
                         {
                            const char *f_file;

                            f_file = evas_font_dir_cache_find(dir, (char *)nm);
                            if (f_file)
                              {
                                 font = (Evas_Font_Set *)evas_common_font_load(f_file, size, wanted_rend, bitmap_scalable);
                                 if (font) break;
                              }
                         }

                       if (!font)
                         {
                            EINA_LIST_FOREACH(global_font_path, ll, dir)
                              {
                                 const char *f_file;

                                 f_file = evas_font_dir_cache_find(dir, (char *)nm);
                                 if (f_file)
                                   {
                                      font = (Evas_Font_Set *)evas_common_font_load(f_file, size, wanted_rend, bitmap_scalable);
                                      if (font) break;
                                   }
                              }
                         }
                    }
               }
          }
        else /* Base font loaded, append others */
          {
             void *ok = NULL;

             if (source)
               {
                  Eet_File *ef;
                  char fake_name[PATH_MAX];

                  eina_file_path_join(fake_name, PATH_MAX, source, nm);
                  if (!evas_common_font_add((RGBA_Font *)font, fake_name, size, wanted_rend, bitmap_scalable))
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
                                 ok = evas_common_font_memory_add((RGBA_Font *)font, source, nm, size, fdata, fsize, wanted_rend, bitmap_scalable);
                              }
                            eet_close(ef);
                            free(fdata);
                         }
                    }
                  else
                    ok = (void *)1;
               }
             if (!ok)
               {
                  if (_file_path_is_full_path((char *)nm))
                    evas_common_font_add((RGBA_Font *)font, (char *)nm, size, wanted_rend, bitmap_scalable);
                  else
                    {
                       const Eina_List *ll;
                       char *dir;
                       RGBA_Font *fn = NULL;

                       EINA_LIST_FOREACH(font_paths, ll, dir)
                         {
                            const char *f_file;

                            f_file = evas_font_dir_cache_find(dir, (char *)nm);
                            if (f_file)
                              {
                                 fn = evas_common_font_add((RGBA_Font *)font, f_file, size, wanted_rend, bitmap_scalable);
                                 if (fn)
                                   break;
                              }
                         }

                       if (!fn)
                         {
                            EINA_LIST_FOREACH(global_font_path, ll, dir)
                              {
                                 const char *f_file;

                                 f_file = evas_font_dir_cache_find(dir, (char *)nm);
                                 if (f_file)
                                   {
                                      fn = evas_common_font_add((RGBA_Font *)font, f_file, size, wanted_rend, bitmap_scalable);
                                      if (fn)
                                         break;
                                   }
                              }
                         }
                    }
               }
          }
       eina_stringshare_del(nm);
     }
   eina_list_free(fonts);

#ifdef HAVE_FONTCONFIG
   if (!font) /* Search using fontconfig */
     {
        FcResult res;

        p_nm = FcPatternBuild (NULL,
              FC_WEIGHT, FcTypeInteger, _fc_weight_map[fdesc->weight],
              FC_SLANT,  FcTypeInteger, _fc_slant_map[fdesc->slant],
              FC_SPACING,  FcTypeInteger, _fc_spacing_map[fdesc->spacing],
#ifdef FC_WIDTH
              FC_WIDTH,  FcTypeInteger, _fc_width_map[fdesc->width],
#endif
              NULL);
        FcPatternAddString (p_nm, FC_FAMILY, (FcChar8*) fdesc->name);

        if (fdesc->style)
          FcPatternAddString (p_nm, FC_STYLE, (FcChar8*) fdesc->style);

        /* Handle font fallbacks */
        if (fdesc->fallbacks)
          {
             const char *start, *end;
             start = fdesc->fallbacks;

             while (start)
               {
                  end = strchr(start, ',');
                  if (end)
                    {
                       char *tmp = alloca((end - start) + 1);
                       strncpy(tmp, start, end - start);
                       tmp[end - start] = 0;
                       FcPatternAddString (p_nm, FC_FAMILY, (FcChar8*) tmp);
                       start += end - start + 1;
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

        FcConfigSubstitute(fc_config, p_nm, FcMatchPattern);
        FcDefaultSubstitute(p_nm);

        /* do matching */
        set = FcFontSort(fc_config, p_nm, FcTrue, NULL, &res);
        if (!set)
          {
              //FIXME add ERR log capability
             //ERR("No fontconfig font matches '%s'. It was the last resource, no font found!", fdesc->name);
             FcPatternDestroy(p_nm);
             p_nm = NULL;
          }
        else
          {
             font = _evas_load_fontconfig(font, set, size, wanted_rend, bitmap_scalable);
          }
     }
   else /* Add a fallback list from fontconfig according to the found font. */
     {
#if FC_MAJOR >= 2 && FC_MINOR >= 11
        FcResult res;

        FT_Face face = evas_common_font_freetype_face_get((RGBA_Font *) font);

        file_font = EINA_TRUE;

        if (face)
          {
             p_nm = FcFreeTypeQueryFace(face, (FcChar8 *) "", 0, NULL);
             FcConfigSubstitute(fc_config, p_nm, FcMatchPattern);
             FcDefaultSubstitute(p_nm);

             /* do matching */
             set = FcFontSort(fc_config, p_nm, FcTrue, NULL, &res);
             if (!set)
               {
                  FcPatternDestroy(p_nm);
                  p_nm = NULL;
               }
             else
               {
                  font = _evas_load_fontconfig(font, set, size, wanted_rend, bitmap_scalable);
               }
          }
#endif
     }
#endif

#ifdef HAVE_FONTCONFIG
 on_find:
#endif
   fd = calloc(1, sizeof(Fndat));
   if (fd)
     {
        fd->fdesc = evas_font_desc_ref(fdesc);
        if (source) fd->source = eina_stringshare_add(source);
        fd->font = font;
        fd->wanted_rend = wanted_rend;
        fd->size = size;
        fd->bitmap_scalable = bitmap_scalable;
        fd->ref = 1;
        fonts_cache = eina_list_prepend(fonts_cache, fd);
#ifdef HAVE_FONTCONFIG
        fd->set = set;
        fd->p_nm = p_nm;
        fd->file_font = file_font;
#endif
     }

   if (font)
      evas_common_font_hinting_set((RGBA_Font *)font, hinting);
   return font;
}

void
evas_font_load_hinting_set(void *font, int hinting)
{
   evas_common_font_hinting_set((RGBA_Font *) font, hinting);
}

Eina_List *
evas_font_dir_available_list(const Eina_List *font_paths)
{
   const Eina_List *l;
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

   if (p && os) set = FcFontList(fc_config, p, os);

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

   /* Add fonts in font_paths*/
   if (font_paths)
     {
        if (!font_dirs) font_dirs = eina_hash_string_superfast_new(NULL);

        EINA_LIST_FOREACH(font_paths, l, dir)
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
     }

   if (global_font_path)
     {
        if (!font_dirs) font_dirs = eina_hash_string_superfast_new(NULL);

        EINA_LIST_FOREACH(global_font_path, l, dir)
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
font_cache_dir_free(const Eina_Hash *hash EINA_UNUSED, const void *key, void *data, void *fdata EINA_UNUSED)
{
   object_text_font_cache_dir_del((char *) key, data);
   return 1;
}

static Evas_Font_Dir *
object_text_font_cache_dir_update(char *dir, Evas_Font_Dir *fd)
{
   char file_path[PATH_MAX];
   DATA64 mt;

   if (fd)
     {
        mt = _file_modified_time(dir);
        if (mt != fd->dir_mod_time)
          {
             eina_hash_del(font_dirs, dir, fd);
             object_text_font_cache_dir_del(dir, fd);
          }
        else
          {
             eina_file_path_join(file_path, PATH_MAX, dir, "fonts.dir");
             mt = _file_modified_time(file_path);
             if (mt != fd->fonts_dir_mod_time)
               {
                  eina_hash_del(font_dirs, dir, fd);
                  object_text_font_cache_dir_del(dir, fd);
               }
             else
               {
                  eina_file_path_join(file_path, PATH_MAX, dir, "fonts.alias");
                  mt = _file_modified_time(file_path);
                  if (mt != fd->fonts_alias_mod_time)
                    {
                       eina_hash_del(font_dirs, dir, fd);
                       object_text_font_cache_dir_del(dir, fd);
                    }
                  else
                    return fd;
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
   char file_path[PATH_MAX];
   Evas_Font_Dir *fd;
   char *file;
   char tmp2[PATH_MAX];
   Eina_List *fdir;
   Evas_Font *fn;
   FILE *f;

   fd = calloc(1, sizeof(Evas_Font_Dir));
   if (!fd) return NULL;
   fd->lookup = eina_hash_string_superfast_new(NULL);

   eina_hash_add(font_dirs, dir, fd);

   /* READ fonts.alias, fonts.dir and directory listing */

   /* fonts.dir */
   eina_file_path_join(file_path, PATH_MAX, dir, "fonts.dir");

   f = fopen(file_path, "rb");
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
                       eina_file_path_join(tmp2, PATH_MAX, dir, fname);
                       fn->path = eina_stringshare_add(tmp2);
                       fd->fonts = eina_list_append(fd->fonts, fn);
                    }
               }
          }
        cant_read: ;
        fclose(f);
     }

   /* directoy listing */
   fdir = _file_path_list(dir, "*.ttf", 0);
   EINA_LIST_FREE(fdir, file)
     {
        eina_file_path_join(file_path, PATH_MAX, dir, file);
        fn = calloc(1, sizeof(Evas_Font));
        if (fn)
          {
             char *p;

             fn->type = 0;
             strcpy(tmp2, file);
             p = strrchr(tmp2, '.');
             if (p) *p = 0;
             fn->simple.name = eina_stringshare_add(tmp2);
             eina_file_path_join(tmp2, PATH_MAX, dir, file);
             fn->path = eina_stringshare_add(tmp2);
             fd->fonts = eina_list_append(fd->fonts, fn);
          }
        free(file);
     }

   /* fonts.alias */
   eina_file_path_join(file_path, PATH_MAX, dir, "fonts.alias");

   f = fopen(file_path, "rb");
   if (f)
     {
        char fname[4096], fdef[4096];

        /* read font alias lines */
        while (fscanf(f, "%4090s %4090[^\n]\n", fname, fdef) == 2)
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

   fd->dir_mod_time = _file_modified_time(dir);

   eina_file_path_join(file_path, PATH_MAX, dir, "fonts.dir");
   fd->fonts_dir_mod_time = _file_modified_time(file_path);

   eina_file_path_join(file_path, PATH_MAX, dir, "fonts.alias");
   fd->fonts_alias_mod_time = _file_modified_time(file_path);

   return fd;
}

static void
object_text_font_cache_dir_del(char *dir EINA_UNUSED, Evas_Font_Dir *fd)
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
evas_font_path_global_append(const char *path)
{
   if (!path) return;
   global_font_path = eina_list_append(global_font_path, eina_stringshare_add(path));
#ifdef HAVE_FONTCONFIG
   if (fc_config)
     FcConfigAppFontAddDir(fc_config, (const FcChar8 *) path);
#endif
}

EAPI void
evas_font_path_global_prepend(const char *path)
{
   if (!path) return;
   global_font_path = eina_list_prepend(global_font_path, eina_stringshare_add(path));
#ifdef HAVE_FONTCONFIG
   if (fc_config)
     FcConfigAppFontAddDir(fc_config, (const FcChar8 *) path);
#endif
}

EAPI void
evas_font_path_global_clear(void)
{
   while (global_font_path)
     {
        eina_stringshare_del(global_font_path->data);
        global_font_path = eina_list_remove(global_font_path, global_font_path->data);
     }
#ifdef HAVE_FONTCONFIG
   if (fc_config)
     FcConfigAppFontClear(fc_config);
#endif
}

EAPI const Eina_List *
evas_font_path_global_list(void)
{
   return global_font_path;
}

EAPI void
evas_font_reinit(void)
{
#ifdef HAVE_FONTCONFIG
   Eina_List *l;
   char *path;

   if (fc_config)
     {
        FcConfigDestroy(fc_config);
        fc_config = FcInitLoadConfigAndFonts();

        EINA_LIST_FOREACH(global_font_path, l, path)
           FcConfigAppFontAddDir(fc_config, (const FcChar8 *) path);
     }
#endif
}
