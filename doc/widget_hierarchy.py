#!/usr/bin/env python

import re
import os
import sys
import pickle
from optparse import OptionParser

def print_node(a, b, on_tree=False):
    if on_tree:
        rank = "source"
    else:
        rank = "same"

    i_list = ifaces.get(a)
    if i_list:
        for iface in i_list:
            if_instance = a + iface
            if_node_tuple = (if_instance, a) if on_tree else (a, if_instance)

            print '"%s" [label="",shape=circle,width=0.2]' % if_instance
            print '"%s" -> "%s" [label="%s", color=transparent]' \
                % (if_instance, if_instance, iface)
            print '{rank="%s" "%s" -> "%s" [arrowhead="none"];}' \
                % ((rank,) + if_node_tuple)

    i_list = ifaces.get(b)
    if i_list:
        for iface in i_list:
            if_instance = b + iface
            if_node_tuple = (if_instance, b) if on_tree else (b, if_instance)

            print '"%s" [label="",shape=circle,width=0.2]' % if_instance
            print '"%s" -> "%s" [label="%s", color=transparent]' \
                % (if_instance, if_instance, iface)
            print '{rank="%s" "%s" -> "%s" [arrowhead="none"];}' \
                % ((rank,) + if_node_tuple)

    print '"%s" -> "%s"' % (a, b), '[arrowhead="empty"];' \
        if on_tree else '[arrowtail="empty",dir=back];'

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
        iface_m = re.search(class_iface_re, contents)
        if m is not None:
            items.setdefault(m.group(3), []).append(m.group(1))
            if m.group(2) != m.group(3):
                cls_map[m.group(2)] = m.group(1)
        if iface_m is not None:
            items.setdefault(iface_m.group(3), []).append(iface_m.group(1))
            if iface_m.group(2) != iface_m.group(3):
                cls_map[iface_m.group(2)] = iface_m.group(1)
            #hardcoding scrollable now, for brevity -- it may change in future
            ifaces.setdefault(iface_m.group(1), []).append('scrollable')

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

#widget name, widget class, parent class
class_iface_re = \
    'EVAS_SMART_SUBCLASS_IFACE_NEW.*?,.*?_elm_(\w+).*?,.*?(\w+).*?,.*?(\w+)'

usage = "usage: %prog -s <DIRECTORY> -o <OUTPUT_FILE>\n" \
    "       %prog -w <WIDGET_NAME> -i <INPUT_FILE>\n" \
    "       %prog -t -i <INPUT_FILE>\n"
parser = OptionParser(usage=usage)
parser.add_option(
    "-s", "--scan", dest="scan_dir", default=None, type="str",
    help="scan for .h/.c files, at the given path, and build the whole" +
    " widget tree")
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

if (not opts.scan_dir and not opts.widget and not opts.tree) \
      or (opts.scan_dir and opts.widget) \
      or (opts.scan_dir and opts.tree) or \
      (opts.tree and opts.widget):
    sys.exit(parser.print_usage())

if opts.scan_dir and not opts.output_file:
    sys.exit(parser.print_usage())

if opts.widget and not opts.input_file:
    sys.exit(parser.print_usage())

if opts.tree and not opts.input_file:
    sys.exit(parser.print_usage())

items = {}
hierarchy = {}
ifaces = {}

if opts.scan_dir:
    files = files_list_build(opts.scan_dir)
    hierachy_build(files)
    pickle.dump([hierarchy, ifaces], open(opts.output_file, "wb" ))
    sys.exit()

if opts.tree:
    print "digraph elm { node [shape=box];"
    print "rankdir=RL;"

    f = open(opts.input_file)
    if not f:
        sys.exit("Bad input file path")

    hierarchy, ifaces = pickle.load(f)

    for cls, parent in hierarchy.items():
        print_node(cls, parent, True);

    print "}"

if opts.widget:
    print "digraph elm { node [shape=box];"

    f = open(opts.input_file)
    if not f:
        sys.exit("Bad input file path")

    hierarchy, ifaces = pickle.load(f)
    l = topological_sort(hierarchy, opts.widget)

    def pairs(lst):
        for i in range(1, len(lst)):
            yield lst[i-1], lst[i]

    for i1, i2 in pairs(l):
        print_node(i1, i2);

    print "}"
