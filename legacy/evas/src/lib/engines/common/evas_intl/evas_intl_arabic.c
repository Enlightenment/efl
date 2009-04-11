/* Authors:
 * 	Tom Hacohen (tom@stsob.com)
 */

#include "../evas_intl_utils.h"

#ifdef ARABIC_SUPPORT
/* arabic contextualizing */

/* arabic input forms */
#define ARABIC_ALEPH_MADDA	0x0622
#define ARABIC_ALEPH		0x0627
#define ARABIC_BET		0x0628
#define ARABIC_TA_MARBUTA	0x0629
#define ARABIC_TAW		0x062A
#define ARABIC_TA		0x062B
#define ARABIC_GIMEL		0x062C
#define ARABIC_HETH		0x062D
#define ARABIC_HA		0x062E
#define	ARABIC_DALET		0x062F
#define ARABIC_DAL		0x0630
#define ARABIC_RESH		0x0631
#define ARABIC_ZAYIN		0x0632
#define ARABIC_SHIN		0x0633
#define ARABIC_SH		0x0634
#define ARABIC_TSADE		0x0635
#define ARABIC_DAD		0x0636
#define ARABIC_TETH		0x0637
#define ARABIC_ZA		0x0638
#define ARABIC_AYIN		0x0639
#define ARABIC_GHAIN		0x063A
#define ARABIC_PE		0x0641
#define ARABIC_QOPH		0x0642
#define ARABIC_KAPH		0x0643
#define ARABIC_LAMED		0x0644
#define ARABIC_MEM		0x0645
#define ARABIC_NUN		0x0646
#define ARABIC_HE		0x0647
#define ARABIC_WAW		0x0648
#define ARABIC_ALEPH_MAQSURA	0x0649
#define ARABIC_YODH		0x064A
/* arabic contextual forms */
#define ARABIC_ISOLATED_ALEPH_MADDA	0xFE81
#define ARABIC_ISOLATED_ALEPH		0xFE8D
#define ARABIC_ISOLATED_TA_MARBUTA	0xFE93
#define ARABIC_ISOLATED_BET		0xFE8F
#define ARABIC_ISOLATED_TAW		0xFE95
#define ARABIC_ISOLATED_TA		0xFE99
#define ARABIC_ISOLATED_GIMEL		0xFE9D
#define ARABIC_ISOLATED_HETH		0xFEA1
#define ARABIC_ISOLATED_HA		0xFEA5
#define	ARABIC_ISOLATED_DALET		0xFEA9
#define ARABIC_ISOLATED_DAL		0xFEAB
#define ARABIC_ISOLATED_RESH		0xFEAD
#define ARABIC_ISOLATED_ZAYIN		0xFEAF
#define ARABIC_ISOLATED_SHIN		0xFEB1
#define ARABIC_ISOLATED_SH		0xFEB5
#define ARABIC_ISOLATED_TSADE		0xFEB9
#define ARABIC_ISOLATED_DAD		0xFEBD
#define ARABIC_ISOLATED_TETH		0xFEC1
#define ARABIC_ISOLATED_ZA		0xFEC5
#define ARABIC_ISOLATED_AYIN		0xFEC9
#define ARABIC_ISOLATED_GHAIN		0xFECD
#define ARABIC_ISOLATED_PE		0xFED1
#define ARABIC_ISOLATED_QOPH		0xFED5
#define ARABIC_ISOLATED_KAPH		0xFED9
#define ARABIC_ISOLATED_LAMED		0xFEDD
#define ARABIC_ISOLATED_MEM		0xFEE1
#define ARABIC_ISOLATED_NUN		0xFEE5
#define ARABIC_ISOLATED_HE		0xFEE9
#define ARABIC_ISOLATED_WAW		0xFEED
#define ARABIC_ISOLATED_ALEPH_MAQSURA	0xFEEF
#define ARABIC_ISOLATED_YODH		0xFEF1

#define ARABIC_IS_SPECIAL_LETTER(c)	((c) == ARABIC_ISOLATED_ALEPH ||  \
					  (c) == ARABIC_ISOLATED_DALET || \
					  (c) == ARABIC_ISOLATED_DAL ||   \
					  (c) == ARABIC_ISOLATED_RESH ||  \
					  (c) == ARABIC_ISOLATED_ZAYIN || \
					  (c) == ARABIC_ISOLATED_WAW  || \
					  (c) == ARABIC_ISOLATED_TA_MARBUTA)
/* from the first to last (including all forms, and special cases
 * like aleph maqsura in some forms*/
#define ARABIC_IS_CONTEXT(c)	(((c) >= ARABIC_ISOLATED_ALEPH && (c) <= ARABIC_ISOLATED_YODH + 3) || \
				 ((c) >= ARABIC_ISOLATED_ALEPH_MADDA && (c) <= ARABIC_ISOLATED_ALEPH_MADDA + 3) || \
				 (c) == 0xFBE8 || \
				 (c) == 0xFBE9) 
#define ARABIC_IS_LETTER(c)	ARABIC_IS_CONTEXT(c)
/* used for arabic context logic */
/* each value is the offset from the regular char in unicode */
enum _ArabicContext {
		ARABIC_CONTEXT_ISOLATED = 0,
		ARABIC_CONTEXT_FINAL = 1,
		ARABIC_CONTEXT_INITIAL = 2,
		ARABIC_CONTEXT_MEDIAL = 3
};
typedef enum _ArabicContext ArabicContext;

static FriBidiChar
_evas_intl_arabic_isolated_to_context(FriBidiChar chr, ArabicContext context);

static int
_evas_intl_arabic_text_to_isolated(FriBidiChar *text);

static FriBidiChar
_evas_intl_arabic_general_to_isolated(FriBidiChar chr);

/* FIXME: there are issues with text that's already in context
 * vowels support is needed (skip them when analysing context)*/
int
evas_intl_arabic_to_context(FriBidiChar *text)
{
	int i;
	int len;
	int start_of_context = 1; /* assume the first is special/non arabic */
	int last_is_first = 0;
	int last_letter = 0;
	
	/* check for empty string */
	if (!*text)
		return;

	len = _evas_intl_arabic_text_to_isolated(text);
	/*FIXME: make it skip vowels */
	for (i = 0 ; i < len ; i++)  {

		if (! ARABIC_IS_LETTER(text[i])) {
			/* mark so it won't be touched,
			 * though start formating */
			if (last_letter && !start_of_context) {
				ArabicContext tmp = (last_is_first) ?
					ARABIC_CONTEXT_ISOLATED
					:
					ARABIC_CONTEXT_FINAL;
				text[i-1] = _evas_intl_arabic_isolated_to_context(
					last_letter,
					tmp);
				
			}
			last_is_first = 0;
			start_of_context = 1;
			last_letter = 0;
			continue;
		}
		/* adjust the last letter */
		last_letter = text[i];
		if (ARABIC_IS_SPECIAL_LETTER(text[i])) {
			if (!start_of_context) 
				text[i] = _evas_intl_arabic_isolated_to_context(text[i], ARABIC_CONTEXT_FINAL);
			/* else: leave isolated */
				
			start_of_context = 1;
			last_is_first = 0;
			continue;
		}

		if (start_of_context) {
			text[i] = _evas_intl_arabic_isolated_to_context(text[i], ARABIC_CONTEXT_INITIAL);
			last_is_first = 1;
		}
		else {
			text[i] = _evas_intl_arabic_isolated_to_context(text[i], ARABIC_CONTEXT_MEDIAL);
			last_is_first = 0;
		}
		/* spceial chars don't get here. */
		start_of_context = 0;
		
	}
	/* if it's arabic and not isolated, the last is always final */
	i--;
	if (last_letter && !start_of_context) {
				ArabicContext tmp = (last_is_first) ? ARABIC_CONTEXT_ISOLATED : ARABIC_CONTEXT_FINAL;
				/* because it's medial atm, and should be isolated */
				text[i] = _evas_intl_arabic_isolated_to_context(
					last_letter,
					tmp);
			}

	return len;
}

/* I wish I could think about a simpler way to do it.
 * Just match every word with it's isolated form */
static FriBidiChar
_evas_intl_arabic_general_to_isolated(FriBidiChar chr)
{
	switch (chr) {
		case ARABIC_ALEPH_MADDA:
			return ARABIC_ISOLATED_ALEPH_MADDA;

		case ARABIC_ALEPH:
			return ARABIC_ISOLATED_ALEPH;

		case ARABIC_TA_MARBUTA:
			return ARABIC_ISOLATED_TA_MARBUTA;
		case ARABIC_BET:
			return ARABIC_ISOLATED_BET;
		
		case ARABIC_TAW:
			return ARABIC_ISOLATED_TAW;

		case ARABIC_TA:
			return ARABIC_ISOLATED_TA;
		
		case ARABIC_GIMEL:
			return ARABIC_ISOLATED_GIMEL;
		
		case ARABIC_HETH:
			return ARABIC_ISOLATED_HETH;
		
		case ARABIC_HA:
			return ARABIC_ISOLATED_HA;
		
		case ARABIC_DALET:
			return ARABIC_ISOLATED_DALET;
		
		case ARABIC_DAL:
			return ARABIC_ISOLATED_DAL;
		
		case ARABIC_RESH:
			return ARABIC_ISOLATED_RESH;
		
		case ARABIC_ZAYIN:
			return ARABIC_ISOLATED_ZAYIN;
		
		case ARABIC_SHIN:
			return ARABIC_ISOLATED_SHIN;
		
		case ARABIC_SH:
			return ARABIC_ISOLATED_SH;
		
		case ARABIC_TSADE:
			return ARABIC_ISOLATED_TSADE;
		
		case ARABIC_DAD:
			return ARABIC_ISOLATED_DAD;
		
		case ARABIC_TETH:
			return ARABIC_ISOLATED_TETH;
		
		case ARABIC_ZA:
			return ARABIC_ISOLATED_ZA;
		
		case ARABIC_AYIN:
			return ARABIC_ISOLATED_AYIN;
		
		case ARABIC_GHAIN:
			return ARABIC_ISOLATED_GHAIN;
		
		case ARABIC_PE:
			return ARABIC_ISOLATED_PE;
		
		case ARABIC_QOPH:
			return ARABIC_ISOLATED_QOPH;
		
		case ARABIC_KAPH:
			return ARABIC_ISOLATED_KAPH;
		
		case ARABIC_LAMED:
			return ARABIC_ISOLATED_LAMED;

		case ARABIC_MEM:
			return ARABIC_ISOLATED_MEM;
		
		case ARABIC_NUN:
			return ARABIC_ISOLATED_NUN;
		
		case ARABIC_HE:
			return ARABIC_ISOLATED_HE;
		
		case ARABIC_WAW:
			return ARABIC_ISOLATED_WAW;

		case ARABIC_ALEPH_MAQSURA:
			return ARABIC_ISOLATED_ALEPH_MAQSURA;
		
		case ARABIC_YODH:
			return ARABIC_ISOLATED_YODH;
		default:
			return chr;
	}
}

static FriBidiChar
_evas_intl_arabic_isolated_to_context(FriBidiChar chr, ArabicContext context)
{
	if (ARABIC_IS_SPECIAL_LETTER(chr)) {
		if (context == ARABIC_CONTEXT_INITIAL)
			return chr;
		else
			return chr + ARABIC_CONTEXT_FINAL;
	}
	/* HACK AROUND ALIF MAQSURA */
	else if (chr == ARABIC_ISOLATED_ALEPH_MAQSURA && context > 1) {
		chr = 0xFBE8; /* the initial form */
		context -= 2;
	}
	return chr + context;
}

static int
_evas_intl_arabic_text_to_isolated(FriBidiChar *text)
{
	int i=0;
	while (*text) {
		/* if it's not arabic/it's already in context
		 * it's just returned the same */
		*text = _evas_intl_arabic_general_to_isolated(*text);
		text++;
		i++;
	}
	return i;
}
#endif