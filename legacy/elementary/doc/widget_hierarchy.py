#!/usr/bin/env python

import re
import os
import sys
import pickle
from optparse import OptionParser

def print_node(a, b, back=False):
    print '"%s" -> "%s"' % (a, b), " [dir=back];" if back else ";"

def topological_sort(dep_map, value):
    hierarchy = []

    def sort_do(dep_map, value):
        if value is None:
            return

        hierarchy.insert(0, value)
        sort_do(dep_map, dep_map.get(value, None))

    sort_do(dep_map, value)

    return hierarchy

def hierachy_build(files_list):
    cls_map = {"Elm_Widget_Smart_Class": "widget"}

    for path in files_list:
        contents = ''.join(l[:-1] for l in open(path))
        m = re.search(class_re, contents)
        if m is not None:
            items.setdefault(m.group(3), []).append(m.group(1))
            if m.group(2) != m.group(3):
                cls_map[m.group(2)] = m.group(1)

    for k, v in items.iteritems():
        clsname = cls_map.get(k, k)
        for c in v:
            hierarchy[c] = clsname

def files_list_build(d):
    files_list = []
    for f in os.listdir(d):
        if f.endswith('.c'):
            files_list.append(d + '/' + f)

    return files_list

#widget name, widget class, parent class
class_re = 'EVAS_SMART_SUBCLASS_NEW.*?,.*?_elm_(\w+).*?,.*?(\w+).*?,.*?(\w+)'

usage = "usage: %prog -s -d <DIRECTORY> -o <OUTPUT_FILE>\n" \
    "       %prog -w <WIDGET_NAME> -i <INPUT_FILE>\n" \
    "       %prog -t -i <INPUT_FILE>\n"
parser = OptionParser(usage=usage)
parser.add_option(
    "-s", "--scan", action="store_true", dest="scan",
    help="scan for .h/.c files and build the whole widget tree")
parser.add_option(
    "-d", "--directory", dest="scan_dir", default=None, type="str",
    help="directory where to scan for .h/.c files")
parser.add_option(
    "-o", "--output", dest="output_file", default=None, type="str",
    help="path of the output scanning file (widget tree)")

parser.add_option(
    "-w", "--widget", dest="widget", default=None, type="str",
    help="name of an specific widget to generate a hierarchy tree for")
parser.add_option(
    "-i", "--input", dest="input_file", default=None, type="str",
    help="path of the input (widget tree) where to get data from")

parser.add_option(
    "-t", "--tree", action="store_true", dest="tree",
    help="generate the whole hierarchy tree")

opts, args = parser.parse_args()

if (not opts.scan and not opts.widget and not opts.tree) \
      or (opts.scan and opts.widget) \
      or (opts.scan and opts.tree) or \
      (opts.tree and opts.widget):
    sys.exit(parser.print_usage())

if opts.scan and (not opts.scan_dir or not opts.output_file):
    sys.exit(parser.print_usage())

if opts.widget and not opts.input_file:
    sys.exit(parser.print_usage())

if opts.tree and not opts.input_file:
    sys.exit(parser.print_usage())

items = {}
hierarchy = {}

if opts.scan:
    files = files_list_build(opts.scan_dir)
    hierachy_build(files)
    pickle.dump(hierarchy, open(opts.output_file, "wb" ))
    sys.exit()

if opts.tree:
    print "digraph elm { node [shape=box];"
    print "rankdir=RL;"

    f = open(opts.input_file)
    if not f:
        sys.exit("Bad input file path")

    hierarchy = pickle.load(f)
    for cls, parent in hierarchy.items():
        print_node(cls, parent, True);

    print "}"

if opts.widget:
    print "digraph elm { node [shape=box];"

    f = open(opts.input_file)
    if not f:
        sys.exit("Bad input file path")

    hierarchy = pickle.load(f)
    l = topological_sort(hierarchy, opts.widget)

    def pairs(lst):
        for i in range(1, len(lst)):
            yield lst[i-1], lst[i]

    for i1, i2 in pairs(l):
        print_node(i1, i2);

    print "}"
