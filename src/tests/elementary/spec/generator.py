#!/usr/bin/python

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
import json

output_file = sys.argv[-1]
input_files = sys.argv[1:-1]
list_of_tcases = "static const Efl_Test_Case etc[] = {\n"
list_entry = "  {{ \"{}-{}\", {}}},\n"
generated_api = ""

def interface_to_api(interface_name):
  return interface_name+"_behavior_test"

def to_func_name(class_name):
  return class_name.replace('.','_').lower()

def to_class_getter(class_name):
  return class_name.replace('.','_').upper()+'_CLASS'

tests = []

for input_file in input_files:
  with open(input_file, 'r') as content_file:
    content = content_file.read()
    start = content.index('spec-meta-start') + len('spec-meta-start')
    end = content.index('spec-meta-end')
    resulting_json = content[start:end]
    tmp = json.loads(resulting_json)
    if "test-interface" in tmp and "test-widgets" in tmp:
      tests.append(tmp)

widgets = []

for test in tests:
  interface_test = to_func_name(test["test-interface"])
  for widget_class in test["test-widgets"]:
    combo_name = "_{}_{}".format(to_func_name(interface_test), to_func_name(widget_class));
    list_of_tcases += list_entry.format(interface_test, to_func_name(widget_class), combo_name)
    generated_api += tcase_gen_template.format(combo_name, to_func_name(widget_class), interface_to_api(interface_test))
    if widget_class not in widgets:
      widgets += [widget_class]

for widget in widgets:
  generated_api = fixture_gen_template.format(to_func_name(widget), to_class_getter(widget)) + generated_api

list_of_tcases += "  { NULL, NULL }\n};"

output = open(output_file, "w")
output.write(file_gen_template.format(list_of_tcases, generated_api))
