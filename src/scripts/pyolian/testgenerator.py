#!/usr/bin/env python3
# encoding: utf-8

import os
import datetime
import eolian
import pyratemp
from copy import copy
import itertools

# Use .eo files from the source tree (not the installed ones)
script_path = os.path.dirname(os.path.realpath(__file__))
root_path = os.path.abspath(os.path.join(script_path, '..', '..', '..'))
SCAN_FOLDER = os.path.join(root_path, 'src', 'lib')

# create main eolian state
eolian_db = eolian.Eolian_State()
if not isinstance(eolian_db, eolian.Eolian_State):
    raise(RuntimeError('Eolian, failed to create Eolian state'))

# eolian source tree scan
if not eolian_db.directory_add(SCAN_FOLDER):
    raise(RuntimeError('Eolian, failed to scan source directory'))

# Parse all known eo files
if not eolian_db.all_eot_files_parse():
    raise(RuntimeError('Eolian, failed to parse all EOT files'))

if not eolian_db.all_eo_files_parse():
    raise(RuntimeError('Eolian, failed to parse all EO files'))

# cleanup the database on exit
import atexit
def cleanup_db():
    global eolian_db
    del eolian_db
atexit.register(cleanup_db)

def load_file(filename):
    if os.path.isfile(filename):
        with open(filename, "r") as f:
            return f.read()
    return None


def cs_types(suite):
    suite.dicttypes = {"byte": "sbyte"
           , "llong": "long"
           , "int8": "sbyte"
           , "int16": "short"
           , "int32": "int"
           , "int64": "long"
           , "ssize": "long"
           , "ubyte": "byte"
           , "ullong": "ulong"
           , "uint8": "byte"
           , "uint16": "ushort"
           , "uint32": "uint"
           , "uint64": "ulong"
           , "size": "ulong"
           , "ptrdiff": "long"
           , "intptr": "System.IntPtr"
           , "uintptr": "System.IntPtr"
           , "void_ptr": "System.IntPtr"
           , "void": "System.IntPtr" #only if is out/inout
           , "Error": "eina.Error"
           , "string": "System.String"
           , "mstring": "System.String"
           , "stringshare": "System.String"
           , "any_value": "eina.Value"
           , "any_value_ptr": "eina.Value"
           #complex Types
           , "list": "eina.List"
           , "inlist": "eina.Inlist"
           , "array": "eina.Array"
           , "inarray": "eina.Inarray"
           , "hash": "eina.Hash"
           , "promise": "int"
           , "future": "int"
           , "iterator": "eina.Iterator"
           , "accessor": "int"
           }

    suite.dictwords = {'INOUT':'ref ', 'OUT': 'out '}

    suite.keywords = ["delete", "do",  "lock", "event", "in",  "object",
            "interface", "string", "internal", "fixed", "base"]
    suite.blist_func = [
          "efl_event_callback_array_priority_add"
        , "efl_player_position_get"
        , "efl_text_font_source_get"
        , "efl_text_font_source_set"
        , "efl_ui_focus_manager_focus_get"
        , "efl_ui_widget_focus_set"
        , "efl_ui_widget_focus_get"
        , "efl_ui_text_password_get"
        , "efl_ui_text_password_set"
        , "elm_interface_scrollable_repeat_events_get"
        , "elm_interface_scrollable_repeat_events_set"
        , "elm_wdg_item_del"
        , "elm_wdg_item_focus_get"
        , "elm_wdg_item_focus_set"
        , "elm_interface_scrollable_mirrored_set"
        , "evas_obj_table_mirrored_get"
        , "evas_obj_table_mirrored_set"
        , "edje_obj_load_error_get"
        , "efl_ui_focus_user_parent_get"
        , "efl_canvas_object_scale_get" # duplicated signature
        , "efl_canvas_object_scale_set" # duplicated signature
        , "efl_access_parent_get"
        , "efl_access_name_get"
        , "efl_access_name_set"
        , "efl_access_root_get"
        , "efl_access_type_get"
        , "efl_access_role_get"
        , "efl_access_action_description_get"
        , "efl_access_action_description_set"
        , "efl_access_image_description_get"
        , "efl_access_image_description_set"
        , "efl_access_component_layer_get" # duplicated signature
        , "efl_access_component_alpha_get"
        , "efl_access_component_size_get"
        , "efl_ui_spin_button_loop_get"
        , "efl_ui_list_model_size_get"
        , "efl_ui_list_relayout_layout_do"
            ]

def escape_keyword(suite, key):
    if key in suite.keywords:
        return "kw_{}".format(key)
    return key

def to_csharp_klass_name(suite, eo_name):
    names = eo_name.split('.')
    namespaces = [escape_keyword(suite, x.lower())  for x in names[:-1]]
    klass_name = names[-1]
    return '.'.join(namespaces + [klass_name])

"""
It will find methods and functions with owned return and without other params
"""

class Template(pyratemp.Template):

    def __init__(self, filename, encoding='utf-8',
                       loader_class=pyratemp.LoaderFile,
                       parser_class=pyratemp.Parser,
                       renderer_class=pyratemp.Renderer,
                       eval_class=pyratemp.EvalPseudoSandbox):

        global_ctx = {}
        global_ctx.update({
            # Template info
            'date': datetime.datetime.now(),
            'template_file': os.path.basename(filename),
        })

        self.template_filename = filename
        pyratemp.Template.__init__(self, filename=filename, encoding=encoding,
                                   data=global_ctx,
                                   loader_class=loader_class,
                                   parser_class=parser_class,
                                   renderer_class=renderer_class,
                                   eval_class=eval_class)

    def render(self, suite, verbose=True):
        #if verbose and filename:
        #    print("rendering: {} => {}".format(self.template_filename, filename))

        # Build the context for the template
        ctx = {}
        ctx['suite'] = suite
        # render with the augmented context
        output = self(**ctx)

        if suite.filename is not None:
            # write to file
            with open(suite.filename, "w") as f:
                f.write(output)
        else:
            # or print to stdout
            print("FILENAME IS NONE", output)


class SuiteCase():
    def __init__(self, name, testname, filename):
        self.name = name
        self.testname = testname
        self.fullname = "{}_{}".format(name, testname)
        self.filename = filename
        self.ext = os.path.splitext(filename)[1]
        self.template = os.path.join(script_path, "testgenerator_suite{}.template".format(self.ext))
        self.clslist = []
        self.verbs = ["add", "get", "is", "del", "thaw", "freeze", "save", "wait", "eject", "raise", "lower", "load",
                      "dup", "reset", "unload", "close", "set", "interpolate", "has", "grab", "check", "find", "ungrab",
                      "unset", "clear", "pop", "new", "peek", "push", "update", "show", "move", "hide", "calculate",
                      "resize", "attach", "pack", "unpack", "emit", "call", "append"]

        self.dicttypes = {}
        self.dictwords = {}
        self.blist_func = []
        self.keywords = []

        if self.ext == '.cs':
            cs_types(self)

    def __del__(self):
        for cls in self.clslist:
            cls.myname = None
            cls.custom = None
            cls.init = None
            cls.mlist = None
            cls.plist = None

        self.clslist = None

    def type_convert(self, eotype):
        if eotype.type == eotype.type.VOID:
            return 'System.IntPtr'

        new_type = self.dicttypes.get(eotype.name, to_csharp_klass_name(self, eotype.name))
        if new_type != 'int' and eotype.base_type:
            new_type = "{}<{}>".format(new_type, self.dicttypes.get(eotype.base_type.name, to_csharp_klass_name(self, eotype.base_type.name)))

        return new_type

    def print_arg(self, eoarg):
        r = 'arg_{}'.format(eoarg.name)

        prefix = self.dictwords.get(eoarg.direction.name)

        if prefix == 'out' and (eoarg.type.name == "Eina.Slice" or eoarg.type.name == "Eina.Rw_Slice"):
            prefix = 'ref'

        if not prefix and eoarg.type.is_ptr and eoarg.type.type == eoarg.type.type.REGULAR and eoarg.type.typedecl and eoarg.type.typedecl.type == eoarg.type.typedecl.type.STRUCT:
            prefix = 'ref'

        return prefix and ' '.join([prefix, r]) or r

    def load(self, testdir, eofiles):
        self.clslist = []
        self.custom = load_file(os.path.join(testdir, "{}_custom{}".format(self.name, self.ext))) or ''
        self.init = load_file(os.path.join(testdir, "{}_init{}".format(self.name, self.ext))) or ''
        self.shutdown = load_file(os.path.join(testdir, "{}_shutdown{}".format(self.name, self.ext))) or ''
        for eofile in eofiles:
            cls = eolian_db.class_by_file_get(os.path.basename(eofile))
            if not cls or cls.type != cls.type.REGULAR:
                continue

            self.clslist.append(cls)
            cls.myname = os.path.splitext(cls.file)[0]
            cls.myfullname = "{}_{}".format(self.fullname, cls.myname)
            filedir = os.path.join(testdir, cls.myname)
            cls.custom = load_file(os.path.join(filedir, "custom{}".format(self.ext))) or ''
            cls.init = load_file(os.path.join(filedir, "init{}".format(self.ext)))
            cls.shutdown = load_file(os.path.join(filedir, "shutdown{}".format(self.ext)))

            cls.mlist =  [ m for m in cls.methods if m.full_c_method_name not in self.blist_func ]
            for func in cls.mlist:
                cls.custom += load_file(os.path.join(filedir, func.name, "custom{}".format(self.ext))) or ''
                func.init = load_file(os.path.join(filedir, func.name, "init{}".format(self.ext)))
                func.shutdown = load_file(os.path.join(filedir, func.name, "shutdown{}".format(self.ext)))
                func.arg_init = load_file(os.path.join(filedir, func.name, "arg_init{}".format(self.ext))) or ''
                func.arg_shutdown = load_file(os.path.join(filedir, func.name, "arg_shutdown{}".format(self.ext))) or ''

            cls.plist = [ p for p in cls.properties if (p.getter_scope == p.getter_scope.PUBLIC and p.full_c_getter_name not in self.blist_func) or
                    (p.setter_scope == p.setter_scope.PUBLIC and p.full_c_setter_name not in self.blist_func)]
            for func in cls.plist:
                if func.getter_scope == func.getter_scope.PUBLIC:
                    cls.custom += load_file(os.path.join(filedir, '{}_get'.format(func.name), "custom{}".format(self.ext))) or ''
                    func.get_init = load_file(os.path.join(filedir, '{}_get'.format(func.name), "init{}".format(self.ext)))
                    func.get_shutdown = load_file(os.path.join(filedir, '{}_get'.format(func.name), "shutdown{}".format(self.ext))) or ''
                    func.arg_get_init = load_file(os.path.join(filedir, '{}_get'.format(func.name), "arg_init{}".format(self.ext))) or ''
                    func.arg_get_shutdown = load_file(os.path.join(filedir, '{}_set'.format(func.name), "arg_shutdown{}".format(self.ext))) or ''

                if func.setter_scope == func.setter_scope.PUBLIC:
                    cls.custom += load_file(os.path.join(filedir, '{}_set'.format(func.name), "custom{}".format(self.ext))) or ''
                    func.set_init = load_file(os.path.join(filedir, '{}_set'.format(func.name), "init{}".format(self.ext)))
                    func.set_shutdown = load_file(os.path.join(filedir, '{}_set'.format(func.name), "shutdown{}".format(self.ext))) or ''
                    func.arg_set_init = load_file(os.path.join(filedir, '{}_set'.format(func.name), "arg_init{}".format(self.ext))) or ''
                    func.arg_set_shutdown = load_file(os.path.join(filedir, '{}_set'.format(func.name), "arg_shutdown{}".format(self.ext))) or ''

            if self.ext == ".cs":
                for func in itertools.chain(cls.mlist, cls.plist):
                    names = func.name.split('_')
                    if names[-1] in self.verbs:
                        names.insert(0, names.pop())
                    func.csname = ''.join([ name.capitalize() for name in names ])

if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description='Pyolian search owned functions.')
    parser.add_argument('testname', help='The Test Name to use. (REQUIRED)')
    parser.add_argument('suitename', help='The Suite Name to use. (REQUIRED)')
    parser.add_argument('filename', help='Build file dest. (REQUIRED)')
    parser.add_argument('eofiles', nargs='*', help='The Eolian File to use.')

    args = parser.parse_args()

    testdir = os.path.join(root_path, 'src', 'tests', args.testname)

    suite = SuiteCase(args.suitename, args.testname, args.filename)
    suite.load(testdir, args.eofiles)

    t = Template(suite.template)
#    try:
    t.render(suite)
#    except:
#        print("ERROR RENDERING - Cannot create file: {}".format(suite.filename))
