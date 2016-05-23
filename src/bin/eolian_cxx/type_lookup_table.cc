#include "type_lookup.hh"

namespace eolian_cxx {

using efl::eolian::eolian_type;

const lookup_table_type
type_lookup_table
{
  {"Ecore_Cb", eolian_type::callback_, {"Ecore.h"}},
  {"Ecore_Task_Cb", eolian_type::callback_, {"Ecore.h"}},
  {"Ecore_Timeline_Cb", eolian_type::callback_, {"Ecore.h"}},
  {"Edje_Signal_Cb", eolian_type::callback_, {"Edje.h"}},
  {"Eina_Accessor *", eolian_type::complex_, false, false, true, false, "::efl::eina::accessor", {"eina_accessor.hh"}},
  {"Eina_Bool", eolian_type::simple_, false, false, false, false, "bool", {}},
  {"Eina_Bool *", eolian_type::simple_, false, false, false, false, "bool*", {}},
  {"Eina_Inlist *", eolian_type::complex_, false, false, true, true, "::efl::eina::range_inlist", {"eina_inlist.hh"}},
  {"Eina_Inlist *", eolian_type::complex_, false, true, true, true, "::efl::eina::inlist", {"eina_inlist.hh"}},
  {"Eina_Iterator *", eolian_type::complex_, false, false, true, true, "::efl::eina::iterator", {"eina_iterator.hh"}},
  {"Eina_List *", eolian_type::complex_, false, false, true, true, "::efl::eina::range_list", {"eina_list.hh"}},
  {"Eina_List *", eolian_type::complex_, false, true, true, true, "::efl::eina::list", {"eina_list.hh"}},
  {"const Eina_List *", eolian_type::complex_, true, false, true, true, "::efl::eina::crange_list", {"eina_list.hh"}},
  {"Eina_Array *", eolian_type::complex_, false, false, true, true, "::efl::eina::range_array", {"eina_array.hh"}},
  {"Eina_Array *", eolian_type::complex_, false, true, true, true, "::efl::eina::array", {"eina_array.hh"}},
  {"const Eina_Array *", eolian_type::complex_, true, false, true, true, "::efl::eina::crange_array", {"eina_array.hh"}},
  {"Eio_Filter_Direct_Cb", eolian_type::callback_, {"Eio.h"}},
  {"Eo *", eolian_type::simple_, false, true, true, false, "::efl::eo::concrete", {"eo_concrete.hh"}},
  {"Eo *", eolian_type::simple_, false, false, true, false, "::efl::eo::concrete", {"eo_concrete.hh"}},
  //{"Evas_Object_Box_Layout", eolian_type::callback_, {"Evas.h"}},
  //{"char *", eolian_type::simple_, false, true, true, false, "std::unique_ptr<char*>", {"memory"}},
  {"const Eina_Inlist *", eolian_type::complex_, false, false, true, true, "::efl::eina::range_inlist", {"eina_inlist.hh"}},
  {"const Eina_List *", eolian_type::complex_, false, false, true, true, "::efl::eina::range_list", {"eina_ptrlist.hh"}},
  {"const char *", eolian_type::simple_, false, false, true, true, "::efl::eina::string_view", {"string"}},
};

}
