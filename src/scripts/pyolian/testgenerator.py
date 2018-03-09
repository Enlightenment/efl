#!/usr/bin/env python3
# encoding: utf-8

import os
import datetime
import eolian
import pyratemp
from copy import copy

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


def zeronull_parameters_get(parameters, cparam=[]):
    return [ cparams[i] if i < len(cparams) else 'NULL' if param.type.is_ptr else "0" for i, param in enumerate(parameters) ]

def load_file(filename):
    if os.path.isfile(filename):
        with open(filename, "r") as f:
            return f.read()
    return None

"""
It will find methods and functions with owned return and without other params
"""
def custom_loads(cls, filedir):
    cls.custom = load_file(os.path.join(filedir, "custom.c"))
    cls.init = load_file(os.path.join(filedir, "init.c"))
    cls.shutdown = load_file(os.path.join(filedir, "shutdown.c"))

    for func in cls.methods:
        func.init = load_file(os.path.join(filedir, func.name, "init.c"))
        func.shutdown = load_file(os.path.join(filedir, func.name, "shutdown.c"))
        func.arg_init = load_file(os.path.join(filedir, func.name, "arg_init.c")) or "/* Zero/NULL args init */"
        func.arg_shutdown = load_file(os.path.join(filedir, func.name, "arg_shutdown.c")) or "/* Zero/NULL args shutdown */"

    for func in cls.properties:
        if func.getter_scope:
            func.init = load_file(os.path.join(filedir, '{}_get'.format(func.name), "init.c"))
            func.shutdown = load_file(os.path.join(filedir, '{}_get'.format(func.name), "shutdown.c"))
            func.arg_get_init = load_file(os.path.join(filedir, '{}_get'.format(func.name), "arg_init.c")) or "/* Zero/NULL args getter init */"
            func.arg_get_shutdown = load_file(os.path.join(filedir, '{}_set'.format(func.name), "arg_shutdown.c")) or "/* Zero/NULL args getter shutdown */"

        if func.setter_scope:
            func.init = load_file(os.path.join(filedir, '{}_set'.format(func.name), "init.c"))
            func.shutdown = load_file(os.path.join(filedir, '{}_set'.format(func.name), "shutdown.c"))
            func.arg_set_init = load_file(os.path.join(filedir, '{}_set'.format(func.name), "arg_init.c")) or "/* Zero/NULL args setter init */"
            func.arg_set_shutdown = load_file(os.path.join(filedir, '{}_set'.format(func.name), "arg_shutdown.c")) or "/* Zero/NULL args setter shutdown */"

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

    def render(self, cls, filename=None, verbose=True):
        # Build the context for the template
        ctx = {}
        ctx['cls'] = cls

        if verbose and filename:
            print("rendering: {} => {}".format(
                  self.template_filename, filename))

        # render with the augmented context
        output = self(**ctx)

        if filename is not None:
            # write to file
            with open(filename, "w") as f:
                f.write(output)
        else:
            # or print to stdout
            print(output)


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description='Pyolian search owned functions.')
    parser.add_argument('testcase', help='The TestCase Name to use. (REQUIRED)')
    parser.add_argument('classname', help='The Class Name to use. (REQUIRED)')

    args = parser.parse_args()

    testdir = os.path.join(root_path, 'src', 'tests', args.testcase)
    template = os.path.join(testdir, "{}.template".format(args.testcase))
    if not os.path.isfile(template):
        template = os.path.join(testdir, "automated.template")

    clsname = args.classname

    cls = eolian_db.class_by_name_get(clsname)
    if cls:
        custom_loads(cls, os.path.join(testdir, cls.c_name.lower()))
        filename = os.path.join(testdir, "{}_test.c".format(cls.c_name.lower()))

        t = Template(template)
        t.render(cls, filename)
    else:
        print("Did not find Eolian Class: {}".format(clsname))


