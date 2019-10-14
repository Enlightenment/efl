#!/usr/bin/env python3
# encoding: utf-8
"""
Efl documentation generator

Use this script without arguments to generate the full documentation of the Efl
namespace in a folder called 'dokuwiki' (-v to see all generated files)

  --help to see all other options

"""
import os
import sys
import argparse
import atexit


# Use .eo files from the source tree (not the installed ones)
script_path = os.path.dirname(os.path.realpath(__file__))
root_path = os.path.abspath(os.path.join(script_path, '..', '..', '..'))
SCAN_FOLDER = os.path.join(root_path, 'src', 'lib')


# Use pyolian from source (not installed)
pyolian_path = os.path.join(root_path, 'src', 'scripts')
sys.path.insert(0, pyolian_path)
from pyolian import eolian
from pyolian.generator import Template


# parse args
parser = argparse.ArgumentParser(description='Pyolian DocuWiki generator.')
parser.add_argument('--root-path', '-r', metavar='FOLDER', default='dokuwiki',
                    help='where to write files to (root of dokuwiki) '
                         'default to: "./dokuwiki"')
parser.add_argument('--verbose', '-v', action='store_true',
                    help='print a line for each rendered file')
parser.add_argument('--exclude-beta', '-e', action='store_true',
                    help='do not generate docs for class in beta state')
parser.add_argument('--namespace', '-n', metavar='ROOT', default='Efl',
                    help='root namespace of the docs. (default to "Efl")')
_choices = ['start', 'classes', 'enums', 'structs', 'aliases']
parser.add_argument('--step', '-s', metavar='STEP', default=None,
                    choices=_choices,
                    help='A single step to run (default to all), '
                         'valid choices: ' + ', '.join(_choices))
args = parser.parse_args()


# load the whole eolian db (from .eo files in source tree)
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
def cleanup_db():
    global eolian_db
    del eolian_db


atexit.register(cleanup_db)


# calculate the full path for the txt page of the given object
def page_path_for_object(obj):
    path = ['data', 'pages', 'develop', 'api']
    for ns in obj.namespaces:
        path.append(ns.lower())
    output_filename = obj.short_name.lower() + '.txt'
    return os.path.join(args.root_path, *path, output_filename)


class BetaNamespaceWrapper(eolian.Namespace):
    """ A Namespace wrapper that hide objects marked as beta to the template """
    def __init__(self, eolian_ns):
        super(BetaNamespaceWrapper, self).__init__(eolian_ns.unit, eolian_ns.name)
        self._ns = eolian_ns

    @property
    def sub_namespaces(self):
        return [BetaNamespaceWrapper(ns) for ns in self._ns.sub_namespaces]

    @property
    def classes(self):
        return [c for c in self._ns.classes if not (args.exclude_beta and c.is_beta)]

    @property
    def regulars(self):
        return [c for c in self._ns.regulars if not (args.exclude_beta and c.is_beta)]

    @property
    def abstracts(self):
        return [c for c in self._ns.abstracts if not (args.exclude_beta and c.is_beta)]

    @property
    def mixins(self):
        return [c for c in self._ns.mixins if not (args.exclude_beta and c.is_beta)]

    @property
    def interfaces(self):
        return [c for c in self._ns.interfaces if not (args.exclude_beta and c.is_beta)]

    @property
    def aliases(self):
        return [c for c in self._ns.aliases if not (args.exclude_beta and c.is_beta)]

    @property
    def structs(self):
        return [c for c in self._ns.structs if not (args.exclude_beta and c.is_beta)]

    @property
    def enums(self):
        return [c for c in self._ns.enums if not (args.exclude_beta and c.is_beta)]


# render a (temporary) page for analizying the namespaces hierarchy
t = Template('namespaces.template')
nspaces = [BetaNamespaceWrapper(ns) for ns in eolian_db.all_namespaces
           if ns.name.startswith(args.namespace)]

tot_classes = tot_regulars = tot_abstracts = tot_mixins = tot_ifaces = 0
tot_enums = tot_structs = tot_aliases = 0
for ns in nspaces:
    for cls in ns.classes:
        tot_classes += 1
        if cls.type == eolian.Eolian_Class_Type.REGULAR:
            tot_regulars += 1
        elif cls.type == eolian.Eolian_Class_Type.ABSTRACT:
            tot_abstracts += 1
        elif cls.type == eolian.Eolian_Class_Type.MIXIN:
            tot_mixins += 1
        elif cls.type == eolian.Eolian_Class_Type.INTERFACE:
            tot_ifaces += 1
    tot_enums += len(ns.enums)
    tot_structs += len(ns.structs)
    tot_aliases += len(ns.aliases)


totals = [
    ('Namespaces', len(nspaces)),
    ('ALL Classes', tot_classes),
    ('Regular classes', tot_regulars),
    ('Abstract classes', tot_abstracts),
    ('Mixins', tot_mixins),
    ('Interfaces', tot_ifaces),
    ('Enums', tot_enums),
    ('Structs', tot_structs),
    ('Aliases', tot_aliases),
]

root_ns = BetaNamespaceWrapper(eolian_db.namespace_get_by_name(args.namespace))

output_file = os.path.join(args.root_path, 'data', 'pages', 'develop', 'api', 'namespaces.txt')
t.render(output_file, args.verbose, root_ns=root_ns, totals=totals)


# render the main start.txt page
if args.step in ('start', None):
    t = Template('doc_start.template')

    nspaces = [BetaNamespaceWrapper(ns) for ns in eolian_db.all_namespaces
               if ns.name.startswith(args.namespace)]

    output_file = os.path.join(args.root_path, 'data', 'pages', 'develop', 'api', 'start.txt')
    t.render(output_file, args.verbose, nspaces=nspaces)


# render a page for each Class
if args.step in ('classes', None):
    t = Template('doc_class.template')
    for cls in eolian_db.classes:
        if cls.name.startswith(args.namespace):
            if not (args.exclude_beta and cls.is_beta):
                output_file = page_path_for_object(cls)
                t.render(output_file, args.verbose, cls=cls.name)

# render a page for each Enum
if args.step in ('enums', None):
    t = Template('doc_enum.template')
    for enum in eolian_db.enums:
        if enum.name.startswith(args.namespace):
            if not (args.exclude_beta and enum.is_beta):
                output_file = page_path_for_object(enum)
                t.render(output_file, args.verbose, enum=enum.name)

# render a page for each Struct
if args.step in ('structs', None):
    t = Template('doc_struct.template')
    for struct in eolian_db.structs:
        if struct.name.startswith(args.namespace):
            if not (args.exclude_beta and struct.is_beta):
                output_file = page_path_for_object(struct)
                t.render(output_file, args.verbose, struct=struct.name)

# render a page for each Alias
if args.step in ('aliases', None):
    t = Template('doc_alias.template')
    for alias in eolian_db.aliases:
        if alias.name.startswith(args.namespace):
            if not (args.exclude_beta and alias.is_beta):
                output_file = page_path_for_object(alias)
                t.render(output_file, args.verbose, alias=alias.name)
