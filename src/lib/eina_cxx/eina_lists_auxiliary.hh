#ifndef _EINA_LISTS_AUXILIARY_HH
#define _EINA_LISTS_AUXILIARY_HH

#include <Eina.h>

namespace efl { namespace eina {

inline Eina_List* _eina_list_prepend_relative_list(Eina_List* list, const void* data, Eina_List* relative) EINA_ARG_NONNULL(2)
{
  if(relative)
    return ::eina_list_prepend_relative_list(list, data, relative);
  else
    return ::eina_list_append(list, data);
}

inline Eina_Inlist *_eina_inlist_prepend_relative(Eina_Inlist *in_list,
                                                  Eina_Inlist *in_item,
                                                  Eina_Inlist *in_relative) EINA_ARG_NONNULL(2)
{
  if(in_relative)
    return ::eina_inlist_prepend_relative(in_list, in_item, in_relative);
  else
    return ::eina_inlist_append(in_list, in_item);
}

} }

#endif
