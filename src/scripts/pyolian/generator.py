#!/usr/bin/env python3
# encoding: utf-8
"""

Pyolian template based generator.

This is a really powerfull template-based, output-agnostic eolian generator.
You just need a template file and then you can render it with the
wanted eolian scope (class, namespace, enum, struct, etc...).

For example (from this source folder):

./generator.py test_gen_class.template --cls Efl.Loop.Timer
./generator.py test_gen_namespace.template --ns Efl.Ui

...of course you can pass any other class or namespace to the example above.

The generator is based on the great pyratemp engine (THANKS!), you can find
the full template syntax at: www.simple-is-better.org/template/pyratemp.html

Just keep in mind the syntax is a bit different in this implementation:

    sub_start     = "${"      Was "$!" in original pyratemp (and in docs)
    sub_end       = "}$"      Was "!$" in original pyratemp (and in docs)
    _block_start  = "<!--("
    _block_end    = ")-->"
    comment_start = "#!"
    comment_end   = "!#"


You can also import this module and use the provided Template class if you
are more confortable from within python. To import from outside this directory
you need to hack sys.path in a way that this folder will be available on
PYTHON_PATH, fe:

  pyolian_path = os.path.join(EFL_ROOT_PATH, 'src', 'scripts')
  sys.path.insert(0, pyolian_path)
  from pyolian.generator import Template


"""
import os
import datetime

try:
    from . import eolian
    from . import pyratemp
except ImportError:
    import eolian
    import pyratemp


# Use .eo files from the source tree (not the installed ones)
script_path = os.path.dirname(os.path.realpath(__file__))
root_path = os.path.abspath(os.path.join(script_path, '..', '..', '..'))
SCAN_FOLDER = os.path.join(root_path, 'src', 'lib')


# load the whole eolian db
eolian_db = eolian.Eolian_State()
if not isinstance(eolian_db, eolian.Eolian_State):
    raise(RuntimeError('Eolian, failed to create Eolian state'))

if not eolian_db.directory_add(SCAN_FOLDER):
    raise(RuntimeError('Eolian, failed to scan source directory'))

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


class Template(pyratemp.Template):
    """ Pyolian template based generator.

    You can directly use this class to generate custom outputs based
    on the eolian database and your provided templates.

    Usage is as simple as:
        t = Template(<template_file>)
        t.render(<output_file>, cls=..., ns=..., ...)

    Args:
        filename: Template file to load. (REQUIRED)
        context: User provided context for the template (dict).
    """
    def __init__(self, filename, encoding='utf-8', context=None, escape=None,
                       loader_class=pyratemp.LoaderFile,
                       parser_class=pyratemp.Parser,
                       renderer_class=pyratemp.Renderer,
                       eval_class=pyratemp.EvalPseudoSandbox):

        # Build the global context for the template
        global_ctx = {}
        # user provided context (low pri)
        if context:
            global_ctx.update(context)
        # standard names (not overwritables)
        global_ctx.update({
            # Template info
            'date': datetime.datetime.now(),
            'template_file': os.path.basename(filename),
            # Eolian info
            #  'eolian_version': eolian.__version__,
            #  'eolian_version_info': eolian.__version_info__,
            # Eolian Classes
            'Object': eolian.Object,
            'Class': eolian.Class,
            'Part': eolian.Part,
            'Constructor': eolian.Constructor,
            'Event': eolian.Event,
            'Function': eolian.Function,
            'Function_Parameter': eolian.Function_Parameter,
            'Implement': eolian.Implement,
            'Type': eolian.Type,
            'Typedecl': eolian.Typedecl,
            'Enum_Type_Field': eolian.Enum_Type_Field,
            'Struct_Type_Field': eolian.Struct_Type_Field,
            'Expression': eolian.Expression,
            'Variable': eolian.Variable,
            'Documentation': eolian.Documentation,
            'Documentation_Token': eolian.Documentation_Token,
            # Eolian Enums
            'Eolian_Function_Type': eolian.Eolian_Function_Type,
            'Eolian_Parameter_Dir': eolian.Eolian_Parameter_Dir,
            'Eolian_Class_Type': eolian.Eolian_Class_Type,
            'Eolian_Object_Scope': eolian.Eolian_Object_Scope,
            'Eolian_Typedecl_Type': eolian.Eolian_Typedecl_Type,
            'Eolian_Type_Type': eolian.Eolian_Type_Type,
            'Eolian_Type_Builtin_Type': eolian.Eolian_Type_Builtin_Type,
            'Eolian_C_Type_Type': eolian.Eolian_C_Type_Type,
            'Eolian_Expression_Type': eolian.Eolian_Expression_Type,
            'Eolian_Expression_Mask': eolian.Eolian_Expression_Mask,
            'Eolian_Variable_Type': eolian.Eolian_Variable_Type,
            'Eolian_Binary_Operator': eolian.Eolian_Binary_Operator,
            'Eolian_Unary_Operator': eolian.Eolian_Unary_Operator,
            'Eolian_Doc_Token_Type': eolian.Eolian_Doc_Token_Type,
        })

        # Call the parent __init__ func
        self.template_filename = filename
        pyratemp.Template.__init__(self, filename=filename, encoding=encoding,
                                   data=global_ctx, escape=escape,
                                   loader_class=loader_class,
                                   parser_class=parser_class,
                                   renderer_class=renderer_class,
                                   eval_class=eval_class)

    def render(self, filename=None, verbose=True, cls=None, ns=None,
                     struct=None, enum=None, alias=None, **kargs):
        # Build the context for the template
        ctx = {}
        if kargs:
            ctx.update(kargs)
        if cls:
            ctx['cls'] = eolian_db.class_by_name_get(cls)
        if ns:
            ctx['namespace'] = eolian_db.namespace_get_by_name(ns)
        if struct:
            ctx['struct'] = eolian_db.struct_by_name_get(struct)
        if enum:
            ctx['enum'] = eolian_db.enum_by_name_get(enum)
        if alias:
            ctx['alias'] = eolian_db.alias_by_name_get(alias)

        if verbose and filename:
            print("rendering: {} => {}".format(
                  self.template_filename, filename))

        # render with the augmented context
        output = self(**ctx)

        if filename is not None:
            # create directory tree if needed
            folder = os.path.dirname(filename)
            if folder and not os.path.isdir(folder):
                os.makedirs(folder)
            # write to file
            with open(filename, "w") as f:
                f.write(output)
        else:
            # or print to stdout
            print(output)


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description='Pyolian template based generator.')
    parser.add_argument('template',
                        help='The template file to use. (REQUIRED)')
    parser.add_argument('--output', '-o', metavar='FILE', default=None,
                        help='Where to write the rendered output. '
                             'If not given will print to stdout.')
    parser.add_argument('--cls', metavar='CLASS_NAME', default=None,
                        help='The full name of the class to render, ex: Efl.Loop.Timer')
    parser.add_argument('--ns', metavar='NAMESPACE', default=None,
                        help='The namespace to render, ex: Efl.Loop')
    parser.add_argument('--struct', metavar='STRUCT_NAME', default=None,
                        help='The name of the struct to render, ex: Efl.Loop.Arguments')
    parser.add_argument('--enum', metavar='ENUM_NAME', default=None,
                        help='The name of the enum to render, ex: Efl.Loop.Handler.Flags')
    parser.add_argument('--alias', metavar='ALIAS_NAME', default=None,
                        help='The name of the alias to render, ex: Efl.Font.Size')
    args = parser.parse_args()

    t = Template(args.template)
    t.render(args.output, cls=args.cls, ns=args.ns,
             struct=args.struct, enum=args.enum, alias=args.alias)
