#!/usr/bin/python

tests = [
  ["Efl.Pack_Linear" , "Efl.Ui.Box"],
  ["Efl.Pack" , "Efl.Ui.Table"],
  ["Efl.Content" , "Efl.Ui.Button", "Efl.Ui.Frame", "Efl.Ui.Grid_Default_Item",
                   "Efl.Ui.List_Default_Item", "Efl.Ui.List_Empty_Item",
                   "Efl.Ui.Navigation_Layout", "Efl.Ui.Panel", "Efl.Ui.Progressbar",
                   "Efl.Ui.Radio", "Efl.Ui.Popup", "Efl.Ui.Tab_Page", "Efl.Ui.Scroller"]
]

fixture_gen_template = """
static void
_{}_fixture(void)
{{
  _setup_window_and_widget({});
}}
"""

tcase_gen_template = """
static void
{}(TCase *tc)
{{
  tcase_add_checked_fixture(tc, _{}_fixture, NULL);
  tcase_add_checked_fixture(tc, _setup, _shutdown);
  {}(tc);
}}

"""

file_gen_template = """
#ifdef LIST_DECL
  {}
#else
  {}
#endif
"""

import sys

list_of_tcases = "static const Efl_Test_Case etc[] = {\n"
list_entry = "  {{ \"{}-{}\", {}}},\n"
generated_api = ""

def interface_to_api(interface_name):
  return interface_name+"_behavior_test"

def to_func_name(class_name):
  return class_name.replace('.','_').lower()

def to_class_getter(class_name):
  return class_name.replace('.','_').upper()+'_CLASS'

widgets = []

for test in tests:
  interface_test = to_func_name(test[0])
  for widget_class in test[1:]:
    combo_name = "_{}_{}".format(to_func_name(interface_test), to_func_name(widget_class));
    list_of_tcases += list_entry.format(interface_test, to_func_name(widget_class), combo_name)
    generated_api += tcase_gen_template.format(combo_name, to_func_name(widget_class), interface_to_api(interface_test))
    if widget_class not in widgets:
      widgets += [widget_class]

for widget in widgets:
  generated_api = fixture_gen_template.format(to_func_name(widget), to_class_getter(widget)) + generated_api

list_of_tcases += "  { NULL, NULL }\n};"

output = open(sys.argv[1], "w")
output.write(file_gen_template.format(list_of_tcases, generated_api))
