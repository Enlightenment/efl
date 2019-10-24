/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _EINA_LISTS_AUXILIARY_HH
#define _EINA_LISTS_AUXILIARY_HH

#include <Eina.h>

/**
 * @addtogroup Eina_Cxx_Containers_Group
 *
 * @{
 */

namespace efl { namespace eina {

/**
 * @internal
 */
inline Eina_List* _eina_list_prepend_relative_list(Eina_List* list, const void* data, Eina_List* relative) EINA_ARG_NONNULL(2)
{
  if(relative)
    return ::eina_list_prepend_relative_list(list, data, relative);
  else
    return ::eina_list_append(list, data);
}

/**
 * @internal
 */
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

/**
 * @}
 */

#endif
