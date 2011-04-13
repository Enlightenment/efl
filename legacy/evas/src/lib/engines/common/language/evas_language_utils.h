#ifndef _EVAS_LANGUAGE_UTILS
#define _EVAS_LANGUAGE_UTILS

#include <Eina.h>
#include "evas_bidi_utils.h"

/* Unicode Script property - conforming to HARFBUZZ's */
typedef enum
{
  EVAS_SCRIPT_INVALID_CODE = -1,
  EVAS_SCRIPT_COMMON       = 0,   /* Zyyy */
  EVAS_SCRIPT_INHERITED,          /* Qaai */
  EVAS_SCRIPT_ARABIC,             /* Arab */
  EVAS_SCRIPT_ARMENIAN,           /* Armn */
  EVAS_SCRIPT_BENGALI,            /* Beng */
  EVAS_SCRIPT_BOPOMOFO,           /* Bopo */
  EVAS_SCRIPT_CHEROKEE,           /* Cher */
  EVAS_SCRIPT_COPTIC,             /* Qaac */
  EVAS_SCRIPT_CYRILLIC,           /* Cyrl (Cyrs) */
  EVAS_SCRIPT_DESERET,            /* Dsrt */
  EVAS_SCRIPT_DEVANAGARI,         /* Deva */
  EVAS_SCRIPT_ETHIOPIC,           /* Ethi */
  EVAS_SCRIPT_GEORGIAN,           /* Geor (Geon, Geoa) */
  EVAS_SCRIPT_GOTHIC,             /* Goth */
  EVAS_SCRIPT_GREEK,              /* Grek */
  EVAS_SCRIPT_GUJARATI,           /* Gujr */
  EVAS_SCRIPT_GURMUKHI,           /* Guru */
  EVAS_SCRIPT_HAN,                /* Hani */
  EVAS_SCRIPT_HANGUL,             /* Hang */
  EVAS_SCRIPT_HEBREW,             /* Hebr */
  EVAS_SCRIPT_HIRAGANA,           /* Hira */
  EVAS_SCRIPT_KANNADA,            /* Knda */
  EVAS_SCRIPT_KATAKANA,           /* Kana */
  EVAS_SCRIPT_KHMER,              /* Khmr */
  EVAS_SCRIPT_LAO,                /* Laoo */
  EVAS_SCRIPT_LATIN,              /* Latn (Latf, Latg) */
  EVAS_SCRIPT_MALAYALAM,          /* Mlym */
  EVAS_SCRIPT_MONGOLIAN,          /* Mong */
  EVAS_SCRIPT_MYANMAR,            /* Mymr */
  EVAS_SCRIPT_OGHAM,              /* Ogam */
  EVAS_SCRIPT_OLD_ITALIC,         /* Ital */
  EVAS_SCRIPT_ORIYA,              /* Orya */
  EVAS_SCRIPT_RUNIC,              /* Runr */
  EVAS_SCRIPT_SINHALA,            /* Sinh */
  EVAS_SCRIPT_SYRIAC,             /* Syrc (Syrj, Syrn, Syre) */
  EVAS_SCRIPT_TAMIL,              /* Taml */
  EVAS_SCRIPT_TELUGU,             /* Telu */
  EVAS_SCRIPT_THAANA,             /* Thaa */
  EVAS_SCRIPT_THAI,               /* Thai */
  EVAS_SCRIPT_TIBETAN,            /* Tibt */
  EVAS_SCRIPT_CANADIAN_ABORIGINAL, /* Cans */
  EVAS_SCRIPT_YI,                 /* Yiii */
  EVAS_SCRIPT_TAGALOG,            /* Tglg */
  EVAS_SCRIPT_HANUNOO,            /* Hano */
  EVAS_SCRIPT_BUHID,              /* Buhd */
  EVAS_SCRIPT_TAGBANWA,           /* Tagb */

  /* Unicode-4.0 additions */
  EVAS_SCRIPT_BRAILLE,            /* Brai */
  EVAS_SCRIPT_CYPRIOT,            /* Cprt */
  EVAS_SCRIPT_LIMBU,              /* Limb */
  EVAS_SCRIPT_OSMANYA,            /* Osma */
  EVAS_SCRIPT_SHAVIAN,            /* Shaw */
  EVAS_SCRIPT_LINEAR_B,           /* Linb */
  EVAS_SCRIPT_TAI_LE,             /* Tale */
  EVAS_SCRIPT_UGARITIC,           /* Ugar */

  /* Unicode-4.1 additions */
  EVAS_SCRIPT_NEW_TAI_LUE,        /* Talu */
  EVAS_SCRIPT_BUGINESE,           /* Bugi */
  EVAS_SCRIPT_GLAGOLITIC,         /* Glag */
  EVAS_SCRIPT_TIFINAGH,           /* Tfng */
  EVAS_SCRIPT_SYLOTI_NAGRI,       /* Sylo */
  EVAS_SCRIPT_OLD_PERSIAN,        /* Xpeo */
  EVAS_SCRIPT_KHAROSHTHI,         /* Khar */

  /* Unicode-5.0 additions */
  EVAS_SCRIPT_UNKNOWN,            /* Zzzz */
  EVAS_SCRIPT_BALINESE,           /* Bali */
  EVAS_SCRIPT_CUNEIFORM,          /* Xsux */
  EVAS_SCRIPT_PHOENICIAN,         /* Phnx */
  EVAS_SCRIPT_PHAGS_PA,           /* Phag */
  EVAS_SCRIPT_NKO,                /* Nkoo */

  /* Unicode-5.1 additions */
  EVAS_SCRIPT_KAYAH_LI,           /* Kali */
  EVAS_SCRIPT_LEPCHA,             /* Lepc */
  EVAS_SCRIPT_REJANG,             /* Rjng */
  EVAS_SCRIPT_SUNDANESE,          /* Sund */
  EVAS_SCRIPT_SAURASHTRA,         /* Saur */
  EVAS_SCRIPT_CHAM,               /* Cham */
  EVAS_SCRIPT_OL_CHIKI,           /* Olck */
  EVAS_SCRIPT_VAI,                /* Vaii */
  EVAS_SCRIPT_CARIAN,             /* Cari */
  EVAS_SCRIPT_LYCIAN,             /* Lyci */
  EVAS_SCRIPT_LYDIAN,             /* Lydi */

  /* Unicode-5.2 additions */
  EVAS_SCRIPT_AVESTAN,                /* Avst */
  EVAS_SCRIPT_BAMUM,                  /* Bamu */
  EVAS_SCRIPT_EGYPTIAN_HIEROGLYPHS,   /* Egyp */
  EVAS_SCRIPT_IMPERIAL_ARAMAIC,       /* Armi */
  EVAS_SCRIPT_INSCRIPTIONAL_PAHLAVI,  /* Phli */
  EVAS_SCRIPT_INSCRIPTIONAL_PARTHIAN, /* Prti */
  EVAS_SCRIPT_JAVANESE,               /* Java */
  EVAS_SCRIPT_KAITHI,                 /* Kthi */
  EVAS_SCRIPT_LISU,                   /* Lisu */
  EVAS_SCRIPT_MEITEI_MAYEK,           /* Mtei */
  EVAS_SCRIPT_OLD_SOUTH_ARABIAN,      /* Sarb */
  EVAS_SCRIPT_OLD_TURKIC,             /* Orkh */
  EVAS_SCRIPT_SAMARITAN,              /* Samr */
  EVAS_SCRIPT_TAI_THAM,               /* Lana */
  EVAS_SCRIPT_TAI_VIET                /* Tavt */
} Evas_Script_Type;

int
evas_common_language_script_end_of_run_get(const Eina_Unicode *str, const Evas_BiDi_Paragraph_Props *bidi_props, size_t start, int len);

Evas_Script_Type
evas_common_language_script_type_get(const Eina_Unicode *str, size_t len);

Evas_Script_Type
evas_common_language_char_script_get(Eina_Unicode unicode);

const char *
evas_common_language_from_locale_get(void);

void *
evas_common_language_unicode_funcs_get(void);
#endif

