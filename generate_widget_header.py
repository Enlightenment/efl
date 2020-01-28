#!/usr/bin/env python3
import sys
import os
from ete3 import Tree, TreeStyle, AttrFace, faces, NodeStyle
import argparse

parser = argparse.ArgumentParser(description='Create a image showing all widgets')
parser.add_argument('sourcedir', metavar='sourcedir', help='the path where to find efls source')

G = parser.parse_args()
sys.path.insert(0, os.path.join(G.sourcedir, 'src', 'scripts', 'pyolian'))

import eolian

SCAN_FOLDER = os.path.join(G.sourcedir, 'src', 'lib')

eolian_db = eolian.Eolian_State()

if not eolian_db.directory_add(SCAN_FOLDER):
  raise(RuntimeError('Eolian, failed to scan source directory'))

# Parse all known eo files
if not eolian_db.all_eot_files_parse():
  raise(RuntimeError('Eolian, failed to parse all EOT files'))

if not eolian_db.all_eo_files_parse():
  raise(RuntimeError('Eolian, failed to parse all EO files'))

widget = eolian_db.class_by_name_get("Efl.Ui.Widget")
assert(widget)

def is_widget(obj, rd = 0):
  if obj == widget:
    return rd
  elif obj.parent != None:
    return is_widget(obj.parent, rd + 1)
  return -1

list_of_widgets = []

for kl in eolian_db.classes:
  widget_depth = is_widget(kl)
  if widget_depth != -1:
    insert = {
      'depth' : widget_depth,
      'name' : kl.name,
      'eolian_obj' : kl,
      'beta' : kl.is_beta,
    }
    list_of_widgets.append(insert)

def layout(node):
    # Add node name to laef nodes
    N = AttrFace("name", fsize=9, fgcolor="black")
    faces.add_face_to_node(N, node, 0)


class Widget_Tree():
  def __init__(self, name, parent, is_beta):
    self.name = name
    self.children = []
    if parent != None:
      self.parent = parent
      parent.children.append(self)
      self.tree_node = parent.tree_node.add_child(None, self.name, 1)
    else:
      self.tree_node = Tree(name = self.name).add_child(None, 'Efl.Ui.Widget', 1)
    nstyle = NodeStyle()
    if is_beta:
      nstyle["fgcolor"] = "red"
    else:
      nstyle["fgcolor"] = "blue"
    self.tree_node.set_style(nstyle)

  def dump(self, prev):
    print(prev + self.name)
    self.children.sort(key = lambda x:x.name)
    for c in self.children:
      c.dump(prev + " ")

sorted_widgets = sorted(list_of_widgets, key=lambda x: x['depth'])
widget_tree = {}

for widget in sorted_widgets:
  if widget['depth'] == 0:
    widget_tree[widget['name']] = Widget_Tree(widget['name'], None, widget['beta'])
  else:
    widget_tree[widget['name']] = Widget_Tree(widget['name'], widget_tree[widget['eolian_obj'].parent.name], widget['beta'])

ts = TreeStyle()
ts.layout_fn = layout
ts.show_leaf_name = False
ts.show_scale = False

print("Created widget-overview.svg in $CWD")

widget_tree['Efl.Ui.Widget'].tree_node.render("widget-overview.svg", units="mm", tree_style=ts)
