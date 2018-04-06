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

def load_file(filename):
    if os.path.isfile(filename):
        with open(filename, "r") as f:
            return f.read()
    return None

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

    def __del__(self):
        for cls in self.clslist:
            cls.myname = None
            cls.custom = None
            cls.init = None
            cls.mlist = None
            cls.plist = None

        self.clslist = None

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

            cls.mlist = list(cls.methods)
            for func in cls.mlist:
                cls.custom += load_file(os.path.join(filedir, func.name, "custom{}".format(self.ext))) or ''
                func.init = load_file(os.path.join(filedir, func.name, "init{}".format(self.ext)))
                func.shutdown = load_file(os.path.join(filedir, func.name, "shutdown{}".format(self.ext)))
                func.arg_init = load_file(os.path.join(filedir, func.name, "arg_init{}".format(self.ext))) or "/* Zero/NULL args initialized */"
                func.arg_shutdown = load_file(os.path.join(filedir, func.name, "arg_shutdown{}".format(self.ext))) or ''

            cls.plist = [ p for p in cls.properties if p.getter_scope == p.getter_scope.PUBLIC or p.setter_scope == p.setter_scope.PUBLIC ]
            for func in cls.plist:
                if func.getter_scope == func.getter_scope.PUBLIC:
                    cls.custom += load_file(os.path.join(filedir, '{}_get'.format(func.name), "custom{}".format(self.ext))) or ""
                    func.get_init = load_file(os.path.join(filedir, '{}_get'.format(func.name), "init{}".format(self.ext)))
                    func.get_shutdown = load_file(os.path.join(filedir, '{}_get'.format(func.name), "shutdown{}".format(self.ext)))
                    func.arg_get_init = load_file(os.path.join(filedir, '{}_get'.format(func.name), "arg_init{}".format(self.ext))) or "/* Zero/NULL args initialized */"
                    func.arg_get_shutdown = load_file(os.path.join(filedir, '{}_set'.format(func.name), "arg_shutdown{}".format(self.ext))) or ''

                if func.setter_scope == func.setter_scope.PUBLIC:
                    cls.custom += load_file(os.path.join(filedir, '{}_set'.format(func.name), "custom{}".format(self.ext))) or ""
                    func.set_init = load_file(os.path.join(filedir, '{}_set'.format(func.name), "init{}".format(self.ext)))
                    func.set_shutdown = load_file(os.path.join(filedir, '{}_set'.format(func.name), "shutdown{}".format(self.ext)))
                    func.arg_set_init = load_file(os.path.join(filedir, '{}_set'.format(func.name), "arg_init{}".format(self.ext))) or "/* Zero/NULL args initialized */"
                    func.arg_set_shutdown = load_file(os.path.join(filedir, '{}_set'.format(func.name), "arg_shutdown{}".format(self.ext))) or ''


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
