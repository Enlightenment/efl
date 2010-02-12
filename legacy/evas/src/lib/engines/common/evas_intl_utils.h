#ifndef _EVAS_INTL_UTILS
#define _EVAS_INTL_UTILS

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_FRIBIDI
# define USE_FRIBIDI
# define INTERNATIONAL_SUPPORT
#endif

#ifdef USE_FRIBIDI
# include <fribidi/fribidi.h>

/* abstract fribidi */
typedef FriBidiChar	EvasIntlChar;
typedef FriBidiCharType	EvasIntlParType;
typedef FriBidiStrIndex EvasIntlStrIndex;
typedef FriBidiLevel	EvasIntlLevel;


/* whether should fix arabic specifix issues */
# define ARABIC_SUPPORT

# ifdef ARABIC_SUPPORT
#  include "evas_intl/evas_intl_arabic.h"
# endif

# define evas_intl_position_logical_to_visual(list, position) \
		(list) ? list[position] : position;

# define evas_intl_position_visual_to_logical(list, position) \
		(list) ? list[position] : position;
				

int
evas_intl_is_rtl_char(EvasIntlLevel *embedded_level_list, EvasIntlStrIndex i);

char *
evas_intl_utf8_to_visual(const char *text,
			int *ret_len,
			EvasIntlParType *direction,
			EvasIntlStrIndex **position_L_to_V_list,
			EvasIntlStrIndex **position_V_to_L_list,
			EvasIntlLevel **embedding_level_list);
#endif

#endif
