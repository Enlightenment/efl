#ifndef __EVIL_LANGINFO_H__
#define __EVIL_LANGINFO_H__


#include <locale.h>


typedef int            nl_item;

#define __NL_ITEM( CATEGORY, INDEX )  ((CATEGORY << 16) | INDEX)
#define __NL_ITEM_CATEGORY( ITEM )    (ITEM >> 16)
#define __NL_ITEM_INDEX( ITEM )       (ITEM & 0xffff)

enum {
  /*
   * LC_CTYPE category...
   * Character set classification items.
   */
  _NL_CTYPE_CODESET     = __NL_ITEM( LC_CTYPE, 0 ),
  _NL_NUMERIC_RADIXCHAR = __NL_ITEM( LC_NUMERIC, 0 ),

  D_T_FMT, /* Date and time format for strftime*/
#define D_T_FMT D_T_FMT
  D_FMT,   /* Date format for strftime*/
#define D_FMT D_FMT
  T_FMT,   /* Time format for strftime*/
#define T_FMT T_FMT
  T_FMT_AMPM,  /*12-hour time format for strftime*/
#define T_FMT_AMPM T_FMT_AMPM
  /*
   * Dummy entry, to terminate the list.
   */
  _NL_ITEM_CLASSIFICATION_END
};

/*
 * Define the public aliases for the enumerated classification indices...
 */
# define CODESET       _NL_CTYPE_CODESET
# define RADIXCHAR     _NL_NUMERIC_RADIXCHAR

EAPI char *nl_langinfo(nl_item index);


#endif /*__EVIL_LANGINFO_H__ */
