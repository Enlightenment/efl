#!/usr/bin/env python3
# encoding: utf-8

import os
import sys
import datetime

script_path = os.path.dirname(os.path.realpath(__file__))

if "EFL_DIR" in os.environ:
    root_path = os.environ["EFL_DIR"]
else:
    root_path = os.path.abspath(os.path.join(script_path, "..", "..", ".."))

sys.path.insert(0, os.path.join(root_path, "src", "scripts"))

from pyolian import eolian
from pyolian import pyratemp
from testgen.suitegen import SuiteGen

# Use .eo files from the source tree (not the installed ones)
SCAN_FOLDER = os.path.join(root_path, "src", "lib")

# create main eolian state
eolian_db = eolian.Eolian_State()
if not isinstance(eolian_db, eolian.Eolian_State):
    raise (RuntimeError("Eolian, failed to create Eolian state"))

# eolian source tree scan
if not eolian_db.directory_add(SCAN_FOLDER):
    raise (RuntimeError("Eolian, failed to scan source directory"))

# Parse all known eo files
if not eolian_db.all_eot_files_parse():
    raise (RuntimeError("Eolian, failed to parse all EOT files"))

if not eolian_db.all_eo_files_parse():
    raise (RuntimeError("Eolian, failed to parse all EO files"))

# cleanup the database on exit
import atexit


def cleanup_db():
    global eolian_db
    del eolian_db


atexit.register(cleanup_db)

class Template(pyratemp.Template):
    def __init__(
        self,
        filename,
        encoding="utf-8",
        loader_class=pyratemp.LoaderFile,
        parser_class=pyratemp.Parser,
        renderer_class=pyratemp.Renderer,
        eval_class=pyratemp.EvalPseudoSandbox,
    ):

        global_ctx = {}
        global_ctx.update(
            {
                # Template info
                "date": datetime.datetime.now(),
                "template_file": os.path.basename(filename),
            }
        )

        self.template_filename = filename
        pyratemp.Template.__init__(
            self,
            filename=filename,
            encoding=encoding,
            data=global_ctx,
            loader_class=loader_class,
            parser_class=parser_class,
            renderer_class=renderer_class,
            eval_class=eval_class,
        )

    def render(self, suite, verbose=True):
        # Build the context for the template
        ctx = {}
        ctx["suite"] = suite
        # render with the augmented context
        output = self(**ctx)

        if suite.filename is not None:
            # write to file
            with open(suite.filename, "w") as f:
                f.write(output)


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(description="Eolian Test Generator.")
    parser.add_argument("testname", help="The Test Name used to find custom and template files. (REQUIRED)")
    parser.add_argument("suitename", help="The Suite Name used to find custom files. (REQUIRED)")
    parser.add_argument("filename", help="Generated test file destination. (REQUIRED)")
    parser.add_argument("eofiles", nargs="*", help="The Eolian Files to use.")

    args = parser.parse_args()

    testdir = os.path.join(root_path, "src", "tests", args.testname)

    suite = SuiteGen(args.suitename, args.testname, args.filename, testdir)
    suite.loadFiles(eolian_db, args.eofiles)

    t = Template(suite.template)
    #    try:
    t.render(suite)
#    except:
#        print("ERROR RENDERING - Cannot create file: {}".format(suite.filename))
