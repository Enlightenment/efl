#include "evas_font_private.h"

#ifdef USE_HARFBUZZ
# include <hb.h>
# include <hb-ft.h>
# include <hb-ot.h>
#endif

#ifdef USE_HARFBUZZ
static const hb_script_t
_evas_script_to_harfbuzz[] =
{
  HB_SCRIPT_COMMON,
  HB_SCRIPT_INHERITED,
  HB_SCRIPT_ARABIC,
  HB_SCRIPT_ARMENIAN,
  HB_SCRIPT_BENGALI,
  HB_SCRIPT_BOPOMOFO,
  HB_SCRIPT_CHEROKEE,
  HB_SCRIPT_COPTIC,
  HB_SCRIPT_CYRILLIC,
  HB_SCRIPT_DESERET,
  HB_SCRIPT_DEVANAGARI,
  HB_SCRIPT_ETHIOPIC,
  HB_SCRIPT_GEORGIAN,
  HB_SCRIPT_GOTHIC,
  HB_SCRIPT_GREEK,
  HB_SCRIPT_GUJARATI,
  HB_SCRIPT_GURMUKHI,
  HB_SCRIPT_HAN,
  HB_SCRIPT_HANGUL,
  HB_SCRIPT_HEBREW,
  HB_SCRIPT_HIRAGANA,
  HB_SCRIPT_KANNADA,
  HB_SCRIPT_KATAKANA,
  HB_SCRIPT_KHMER,
  HB_SCRIPT_LAO,
  HB_SCRIPT_LATIN,
  HB_SCRIPT_MALAYALAM,
  HB_SCRIPT_MONGOLIAN,
  HB_SCRIPT_MYANMAR,
  HB_SCRIPT_OGHAM,
  HB_SCRIPT_OLD_ITALIC,
  HB_SCRIPT_ORIYA,
  HB_SCRIPT_RUNIC,
  HB_SCRIPT_SINHALA,
  HB_SCRIPT_SYRIAC,
  HB_SCRIPT_TAMIL,
  HB_SCRIPT_TELUGU,
  HB_SCRIPT_THAANA,
  HB_SCRIPT_THAI,
  HB_SCRIPT_TIBETAN,
  HB_SCRIPT_CANADIAN_ABORIGINAL,
  HB_SCRIPT_YI,
  HB_SCRIPT_TAGALOG,
  HB_SCRIPT_HANUNOO,
  HB_SCRIPT_BUHID,
  HB_SCRIPT_TAGBANWA,

  /* Unicode-4.0 additions */
  HB_SCRIPT_BRAILLE,
  HB_SCRIPT_CYPRIOT,
  HB_SCRIPT_LIMBU,
  HB_SCRIPT_OSMANYA,
  HB_SCRIPT_SHAVIAN,
  HB_SCRIPT_LINEAR_B,
  HB_SCRIPT_TAI_LE,
  HB_SCRIPT_UGARITIC,

  /* Unicode-4.1 additions */
  HB_SCRIPT_NEW_TAI_LUE,
  HB_SCRIPT_BUGINESE,
  HB_SCRIPT_GLAGOLITIC,
  HB_SCRIPT_TIFINAGH,
  HB_SCRIPT_SYLOTI_NAGRI,
  HB_SCRIPT_OLD_PERSIAN,
  HB_SCRIPT_KHAROSHTHI,

  /* Unicode-5.0 additions */
  HB_SCRIPT_UNKNOWN,
  HB_SCRIPT_BALINESE,
  HB_SCRIPT_CUNEIFORM,
  HB_SCRIPT_PHOENICIAN,
  HB_SCRIPT_PHAGS_PA,
  HB_SCRIPT_NKO,

  /* Unicode-5.1 additions */
  HB_SCRIPT_KAYAH_LI,
  HB_SCRIPT_LEPCHA,
  HB_SCRIPT_REJANG,
  HB_SCRIPT_SUNDANESE,
  HB_SCRIPT_SAURASHTRA,
  HB_SCRIPT_CHAM,
  HB_SCRIPT_OL_CHIKI,
  HB_SCRIPT_VAI,
  HB_SCRIPT_CARIAN,
  HB_SCRIPT_LYCIAN,
  HB_SCRIPT_LYDIAN,

  /* Unicode-5.2 additions */
  HB_SCRIPT_AVESTAN,
  HB_SCRIPT_BAMUM,
  HB_SCRIPT_EGYPTIAN_HIEROGLYPHS,
  HB_SCRIPT_IMPERIAL_ARAMAIC,
  HB_SCRIPT_INSCRIPTIONAL_PAHLAVI,
  HB_SCRIPT_INSCRIPTIONAL_PARTHIAN,
  HB_SCRIPT_JAVANESE,
  HB_SCRIPT_KAITHI,
  HB_SCRIPT_TAI_THAM,
  HB_SCRIPT_LISU,
  HB_SCRIPT_MEETEI_MAYEK,
  HB_SCRIPT_OLD_SOUTH_ARABIAN,
  HB_SCRIPT_OLD_TURKIC,
  HB_SCRIPT_SAMARITAN,
  HB_SCRIPT_TAI_VIET,

  /* Unicode-6.0 additions */
  HB_SCRIPT_BATAK,
  HB_SCRIPT_BRAHMI,
  HB_SCRIPT_MANDAIC
};
#endif

#ifdef OT_SUPPORT
/* FIXME: doc. returns #items */
EVAS_API int
evas_common_font_ot_cluster_size_get(const Evas_Text_Props *props, size_t char_index)
{
   int i;
   int items;
   int left_bound, right_bound;
   size_t base_cluster;
   char_index += props->start;
   base_cluster = EVAS_FONT_OT_POS_GET(props->info->ot[char_index]);
   for (i = (int) char_index ;
         (i >= (int) props->start) &&
         (EVAS_FONT_OT_POS_GET(props->info->ot[i]) == base_cluster) ;
         i--)
     ;
   left_bound = i;
   for (i = (int) char_index + 1;
         (i < (int) (props->start + props->len)) &&
         (EVAS_FONT_OT_POS_GET(props->info->ot[i]) == base_cluster) ;
         i++)
     ;
   right_bound = i;

   if (right_bound == left_bound)
     {
        items = 1;
     }
   else if (props->bidi_dir == EVAS_BIDI_DIRECTION_RTL)
     {
        if (left_bound < 0)
          {
             items = props->text_offset + props->text_len - base_cluster;
          }
        else
          {
             items = props->info->ot[left_bound].source_cluster - base_cluster;
          }
     }
   else
     {
        if (right_bound >= (int) (props->start + props->len))
          {
             items = props->text_offset + props->text_len - base_cluster;
          }
        else
          {
             items = props->info->ot[right_bound].source_cluster - base_cluster;
          }
     }
   return (items > 0) ? items : 1;
}

/* Harfbuzz font functions */

static hb_position_t
_evas_common_font_ot_hb_get_glyph_advance(hb_font_t *font,
      void *font_data, hb_codepoint_t glyph,
      void *user_data)
{
   /* Use our cache*/
   RGBA_Font_Int *fi = (RGBA_Font_Int *) font_data;
   RGBA_Font_Glyph *fg;
   (void) font;
   (void) user_data;
   fg = evas_common_font_int_cache_glyph_get(fi, glyph);
   if (fg)
     {
        return fg->advance.x >> 10;
     }
   return 0;
}

static hb_position_t
_evas_common_font_ot_hb_get_kerning(hb_font_t *font, void *font_data,
   hb_codepoint_t first_glyph, hb_codepoint_t second_glyph, void *user_data)
{
   RGBA_Font_Int *fi = (RGBA_Font_Int *) font_data;
   int kern;
   (void) font;
   (void) user_data;
   if (evas_common_font_query_kerning(fi, first_glyph, second_glyph, &kern))
      return kern;

   return 0;
}

/* End of harfbuzz font funcs */

static inline hb_font_funcs_t *
_evas_common_font_ot_font_funcs_get(void)
{
   static hb_font_funcs_t *font_funcs = NULL;
   if (!font_funcs)
     {
        font_funcs = hb_font_funcs_create();
        hb_font_funcs_set_glyph_h_advance_func(font_funcs,
            _evas_common_font_ot_hb_get_glyph_advance, NULL, NULL);
        hb_font_funcs_set_glyph_h_kerning_func(font_funcs,
            _evas_common_font_ot_hb_get_kerning, NULL, NULL);
     }

   return font_funcs;
}

static inline hb_unicode_funcs_t *
_evas_common_font_ot_unicode_funcs_get(void)
{
   static hb_unicode_funcs_t *unicode_funcs = NULL;
   if (!unicode_funcs)
     {
        unicode_funcs = hb_unicode_funcs_get_default();
     }

   return unicode_funcs;
}

static hb_font_t *
_evas_common_font_ot_hb_font_get(RGBA_Font_Int *fi)
{
   if (!fi->ft.hb_font)
     {
        hb_font_t *hb_ft_font;

        if (!fi->src || !fi->src->ft.face) return NULL;

        hb_ft_font = hb_ft_font_create(fi->src->ft.face, NULL);
        fi->ft.hb_font = hb_font_create_sub_font(hb_ft_font);
        hb_font_destroy(hb_ft_font);

        hb_font_set_funcs(fi->ft.hb_font,
              _evas_common_font_ot_font_funcs_get(), fi, NULL);
     }

   return fi->ft.hb_font;
}

static void
_evas_common_font_ot_shape(hb_buffer_t *buffer, RGBA_Font_Int *fi, Evas_Text_Props_Mode mode)
{
   if (!_evas_common_font_ot_hb_font_get(fi)) return;

   if (mode == EVAS_TEXT_PROPS_MODE_SHAPE)
     {
        hb_shape(fi->ft.hb_font, buffer, NULL, 0);
     }
   else
     {
        const char *shaper_list[] = { "fallback", NULL };
        hb_shape_full(fi->ft.hb_font, buffer, NULL, 0, shaper_list);
     }
}

EVAS_API void
evas_common_font_ot_ligature_triggers_clear(Evas_Font_Liga_Triggers *t)
{
   if (!t) return;
   free(t->extra);
   memset(t, 0, sizeof(*t));
}

EVAS_API Eina_Bool
evas_common_font_ot_ligature_triggers_get(RGBA_Font_Int *fi,
                                          Evas_Font_Liga_Triggers *out)
{
   /* Coding-ligature fonts mostly use calt; the others cover the rest. */
   static const hb_tag_t feats[] = {
      HB_TAG('l','i','g','a'), HB_TAG('c','l','i','g'),
      HB_TAG('c','a','l','t'), HB_TAG('d','l','i','g'),
      HB_TAG('r','l','i','g'), HB_TAG_NONE
   };
   hb_font_t *hb_font;
   hb_face_t *face;
   hb_set_t *lookups, *cover, *unis, *before, *input, *after, *output;
   hb_codepoint_t idx;
   Eina_Unicode *extra = NULL;
   unsigned int extra_cnt = 0, extra_alloc = 0;
   Eina_Bool found = EINA_FALSE;

   if (!out) return EINA_FALSE;
   memset(out, 0, sizeof(*out));
   if (!fi) return EINA_FALSE;

   evas_common_font_int_reload(fi);
   hb_font = _evas_common_font_ot_hb_font_get(fi);
   if (!hb_font) return EINA_FALSE;
   face = hb_font_get_face(hb_font);
   if (!face) return EINA_FALSE;
   if (!hb_ot_layout_has_substitution(face)) return EINA_FALSE;

   lookups = hb_set_create();
   cover = hb_set_create();
   unis = hb_set_create();
   before = hb_set_create();
   input = hb_set_create();
   after = hb_set_create();
   output = hb_set_create();

   /* NULL scripts and languages: all of them. */
   hb_ot_layout_collect_lookups(face, HB_OT_TAG_GSUB, NULL, NULL, feats,
                                lookups);

   idx = HB_SET_VALUE_INVALID;
   while (hb_set_next(lookups, &idx))
     {
        hb_set_clear(before);
        hb_set_clear(input);
        hb_set_clear(after);
        hb_set_clear(output);
        hb_ot_layout_lookup_collect_glyphs(face, HB_OT_TAG_GSUB, idx,
                                           before, input, after, output);
        /* Context glyphs count too: they change what their neighbours
         * become. */
        hb_set_union(cover, before);
        hb_set_union(cover, input);
        hb_set_union(cover, after);
     }
   if (hb_set_is_empty(cover)) goto done;

   /* Map covered glyphs back to codepoints. */
   hb_face_collect_unicodes(face, unis);
   idx = HB_SET_VALUE_INVALID;
   while (hb_set_next(unis, &idx))
     {
        hb_codepoint_t g;

        if (!hb_font_get_nominal_glyph(hb_font, idx, &g)) continue;
        if (!hb_set_has(cover, g)) continue;

        found = EINA_TRUE;
        if (idx < 0x80)
          {
             out->ascii[idx >> 6] |= (UINT64_C(1) << (idx & 63));
             continue;
          }
        /* hb_set_next is ascending, so extra[] stays sorted. */
        if (extra_cnt == extra_alloc)
          {
             Eina_Unicode *tmp;
             unsigned int na = extra_alloc ? (extra_alloc * 2) : 16;

             tmp = realloc(extra, na * sizeof(Eina_Unicode));
             if (!tmp) goto done;
             extra = tmp;
             extra_alloc = na;
          }
        extra[extra_cnt++] = (Eina_Unicode)idx;
     }

done:
   hb_set_destroy(output);
   hb_set_destroy(after);
   hb_set_destroy(input);
   hb_set_destroy(before);
   hb_set_destroy(unis);
   hb_set_destroy(cover);
   hb_set_destroy(lookups);

   out->extra = extra;
   out->extra_cnt = extra_cnt;
   if (!found) evas_common_font_ot_ligature_triggers_clear(out);

   return found;
}

EVAS_API Eina_Bool
evas_common_font_ot_populate_text_props(const Eina_Unicode *text,
                                        Evas_Text_Props *props, int len,
                                        Evas_Text_Props_Mode mode,
                                        const char *lang)
{
   RGBA_Font_Int *fi;
   hb_buffer_t *buffer;
   hb_glyph_position_t *positions;
   hb_glyph_info_t *infos;
   int slen;
   unsigned int i;
   Evas_Font_Glyph_Info *gl_itr;
   Evas_Font_OT_Info *ot_itr;
   Evas_Coord pen_x = 0;

   fi = props->font_instance;

   if (len < 0)
     {
        slen = eina_unicode_strlen(text);
     }
   else
     {
        slen = len;
     }

   buffer = hb_buffer_create();
   hb_buffer_set_unicode_funcs(buffer, _evas_common_font_ot_unicode_funcs_get());
   hb_buffer_set_language(buffer, hb_language_from_string(lang, -1));
   hb_buffer_set_script(buffer, _evas_script_to_harfbuzz[props->script]);
   hb_buffer_set_direction(buffer,
                           (props->bidi_dir == EVAS_BIDI_DIRECTION_RTL) ?
                           HB_DIRECTION_RTL : HB_DIRECTION_LTR);
   /* FIXME: add run-time conversions if needed, which is very unlikely */
   hb_buffer_add_utf32(buffer, (const uint32_t *) text, slen, 0, slen);

   _evas_common_font_ot_shape(buffer, fi, mode);

   props->len = hb_buffer_get_length(buffer);
   props->info->ot = calloc(props->len, sizeof(Evas_Font_OT_Info));
   props->info->glyph = calloc(props->len, sizeof(Evas_Font_Glyph_Info));
   positions = hb_buffer_get_glyph_positions(buffer, NULL);
   infos = hb_buffer_get_glyph_infos(buffer, NULL);
   gl_itr = props->info->glyph;
   ot_itr = props->info->ot;
   for (i = 0 ; i < props->len ; i++)
     {
        Evas_Coord adv;
        ot_itr->source_cluster = infos->cluster;
        ot_itr->x_offset = positions->x_offset;
        ot_itr->y_offset = positions->y_offset;
        gl_itr->index = infos->codepoint;
        adv = positions->x_advance;

        pen_x += EVAS_FONT_ROUND_26_6_TO_INT(adv);
        gl_itr->pen_after = pen_x;

        ot_itr++;
        gl_itr++;
        infos++;
        positions++;
     }

   hb_buffer_destroy(buffer);
   evas_common_font_int_use_trim();

   return EINA_FALSE;
}

#else

EVAS_API void
evas_common_font_ot_ligature_triggers_clear(Evas_Font_Liga_Triggers *t)
{
   if (!t) return;
   memset(t, 0, sizeof(*t));
}

EVAS_API Eina_Bool
evas_common_font_ot_ligature_triggers_get(RGBA_Font_Int *fi EINA_UNUSED,
                                          Evas_Font_Liga_Triggers *out)
{
   if (out) memset(out, 0, sizeof(*out));
   return EINA_FALSE;
}

#endif

