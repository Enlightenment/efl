#ifndef __EVIL_LANGINFO_H__
#define __EVIL_LANGINFO_H__


#if ! (defined(__CEGCC__) || defined(__MINGW32CE__))

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
  _NL_CTYPE_CODESET = __NL_ITEM( LC_CTYPE, 0 ),

  /*
   * Dummy entry, to terminate the list.
   */
  _NL_ITEM_CLASSIFICATION_END
};

/*
 * Define the public aliases for the enumerated classification indices...
 */
# define CODESET       _NL_CTYPE_CODESET

EAPI char *nl_langinfo(nl_item index);

#endif /* __CEGCC__ || __MINGW32CE__ */


#endif /*__EVIL_LANGINFO_H__ */
