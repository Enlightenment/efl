#ifndef EVAS_TEXTBLOCK_HYPHENATION_H
#define EVAS_TEXTBLOCK_HYPHENATION_H
#ifdef HAVE_HYPHEN
#include <hyphen.h>

typedef struct
{
   const char *lang;
   HyphenDict *dict;
} Dict_Hyphen;

/* Hyphenation dictionaries */
static Dict_Hyphen _dicts_hyphen[64];
static size_t     _hyphens_num = 0;
static size_t     _hyphen_clients = 0;

static void
_dicts_hyphen_init(Eo *eo_obj)
{
   Efl_Canvas_Text_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);

   if (!o->hyphenating)
     {
        _hyphen_clients++;
        o->hyphenating = EINA_TRUE;
     }
}

static void *
_dict_hyphen_load(const char *lang)
{
   Eina_Iterator *it;
   Eina_File_Direct_Info *dir;
   void *dict = NULL;

   it = eina_file_direct_ls(EVAS_DICTS_HYPHEN_DIR);
   if (!it)
     {
        ERR("Couldn't list files in hyphens path: %s\n", EVAS_DICTS_HYPHEN_DIR);
        return NULL;
     }

   /* The following is based on how files are installed in arch linux:
    * the files are in the pattern of "hyph_xx_XX.dic" (e.g. hyph_en_US.dic).
    * We are actually trying a bit more in case these are installed in another
    * name. We assume that they probably end in "xx_XX.dic" anyway. */
   EINA_ITERATOR_FOREACH(it, dir)
     {
        const char *file = dir->path + dir->name_start;
        char *prefix_off; /* 'hyph_' prefix (may be in some distros) */
        char *dic_off; /* '.dic' file extension offset */

        /* Check a few assumptions and reject if aren't met. */
        prefix_off = strstr(file, "hyph_");
        dic_off = strrchr(file, '.');
        if (!dic_off || ((size_t) (dic_off - file) + 4 != dir->name_length) ||
            (dic_off - file < 5)  ||
            ((dic_off - file > 0) && !prefix_off) ||
            strncmp(dic_off, ".dic", 4) || strncmp((dic_off - 5), lang, strlen(lang)))
          {
             continue;
          }

        dict = hnj_hyphen_load(dir->path);
        if (!dict)
          {
             ERR("Couldn't load hyphen dictionary: %s\n", dic_off - 5);
             continue;
          }
        _dicts_hyphen[_hyphens_num].lang = strndup(dic_off - 5, 5);
        _dicts_hyphen[_hyphens_num++].dict = dict;
        break;
     }

   if (it) eina_iterator_free(it);

   return dict;
}

static void
_dicts_hyphen_free(void)
{
   if (_hyphens_num == 0) return;

   for (size_t i = 0; i < _hyphens_num; i++)
     {
        hnj_hyphen_free(_dicts_hyphen[i].dict);
     }

   _hyphens_num = 0;
}

static inline void
_dicts_hyphen_detach(Eo *eo_obj)
{
   Efl_Canvas_Text_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);

   if (!o->hyphenating) return;
   o->hyphenating = EINA_FALSE;
   _hyphen_clients--;
   if (_hyphen_clients == 0) _dicts_hyphen_free();
}

/* Returns the hyphen dictionary that matches the given language
 * string. The string should be in the format xx_XX e.g. en_US */
static inline void *
_hyphen_dict_get_from_lang(const char *lang)
{
   if (!lang || !(*lang))
     {
        if (!lang) lang = evas_common_language_from_locale_full_get();
        if (!lang || !(*lang)) return NULL;
     }

   for (size_t i = 0; i < _hyphens_num; i++)
     {
        if (!strcmp(_dicts_hyphen[i].lang, lang))
          {
             return _dicts_hyphen[i].dict;
          }
     }

   return _dict_hyphen_load(lang);
}

static char *
_layout_wrap_hyphens_get(const Eina_Unicode *text, const char *lang,
      int word_start, int word_len)
{
   char *utf8;
   int utf8_len; /* length of word */
   char *hyphens;
   char **rep = NULL;
   int *pos = NULL;
   int *cut = NULL;
   void *dict;

   dict = _hyphen_dict_get_from_lang(lang);
   if (!dict)
     {
        ERR("Couldn't find matching dictionary and couldn't fallback to locale %s\n", lang);
        return NULL;
     }

   utf8 = eina_unicode_unicode_to_utf8_range(
         text + word_start, word_len, &utf8_len);
   hyphens = malloc(sizeof(char) * (word_len + 5));
   hnj_hyphen_hyphenate2(dict, utf8, word_len, hyphens, NULL, &rep, &pos, &cut);
   free(utf8);
   return hyphens;
}

#endif //HAVE_HYPHEN
#endif //EVAS_TEXTBLOCK_HYPHENATION_H_
