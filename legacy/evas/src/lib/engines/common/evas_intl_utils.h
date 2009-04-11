#ifndef _EVAS_INTL_UTILS
#define _EVAS_INTL_UTILS

#include "config.h"

#ifdef HAVE_FRIBIDI_FRIBIDI_H
#define USE_FRIBIDI 1
#define INTERNATIONAL_SUPPORT
#endif

#ifdef USE_FRIBIDI
#include <fribidi/fribidi.h>

/* whether should fix arabic specifix issues */
#define ARABIC_SUPPORT 1

#ifdef ARABIC_SUPPORT
#include "evas_intl/evas_intl_arabic.h"
#endif

int
evas_intl_is_rtl_char(FriBidiLevel *embedded_level_list, FriBidiStrIndex i);

char *
evas_intl_utf8_to_visual(const char *text, int *ret_len, FriBidiCharType *direction,
			FriBidiLevel **embedding_level_list);
#endif

#endif
