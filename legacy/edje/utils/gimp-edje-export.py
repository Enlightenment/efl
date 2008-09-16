#!/usr/bin/env python

#   Gimp-Python - allows the writing of Gimp plugins in Python.
#   Copyright (C) 2007  Renato Chencarek <renato.chencarek@openbossa.org>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# INSTALL:
#
#      cp gimp-edje-save.py ~/.gimp/plug-ins/
#      chmod +x ~/.gimp/plug-ins/gimp-edje-save.py
#
#    Notes:
#     - ~/.gimp may change depending on gimp version, ie: ~/.gimp-2.4.
#     - Make sure this file is marked as executable!
#
# USAGE:
#    Load an image and go to "Python-Fu > Edje Save" that will
#    appear on the image's window.
#

from gimpfu import *
import os.path
import re, gettext


def save_layer(layer, filename):
    temp_image = pdb.gimp_image_new (layer.width, layer.height, layer.image.base_type)
    temp_drawable = pdb.gimp_layer_new_from_drawable (layer, temp_image)
    temp_image.add_layer (temp_drawable, -1)
    pdb.gimp_file_save(temp_image, temp_drawable, filename, filename)
    gimp.delete(temp_image)
    return

def write_image_section(layers, edje_file, filenames):
    edje_file.write("images {\n")
    for layer in layers:
        edje_file.write('   image: "%s" COMP;\n' % filenames[layer.name])
    edje_file.write("\n}\n")

def write_layer_data(layer, edje_file, filename):
    expr = re.compile('\W')
    name = expr.sub("_", layer.name)
    data = {'name':name, 'width':layer.width, 'height':layer.height,
            'x0':layer.offsets[0], 'y0':layer.offsets[1],
            'x1':layer.offsets[0] + layer.width, 'y1':layer.offsets[1] + layer.height,
            'path':filename, 'visible':int(layer.visible)
            }
    text = """\
         part {
            name: "%(name)s";
            type: IMAGE;
            mouse_events: 0;
            description {
               state: "default" 0.0;
               min: %(width)d %(height)d;
               max: %(width)d %(height)d;
               rel1 {
                  relative: 0.0 0.0;
                  offset: %(x0)d %(y0)d;
               }
               rel2 {
                  relative: 0.0 0.0;
                  offset: %(x1)d %(y1)d;
               }
               image {
                  normal: "%(path)s";
               }
               visible: %(visible)d;
            }
         }\n""" % data
    edje_file.write(text)

def fix_layers_name(img, save_path):
    filenames = {}
    for layer in img.layers:
        expr = re.compile('\W')
        name = expr.sub("_", layer.name)
        if not name:
            name = "unnamed"
        layer.name = name
        if name in filenames.keys():
            continue
        filename = os.path.join(save_path, name + ".png")
        filenames[name] = filename

    return filenames

def edje_save(img, drawable, save_path, save_layers, edje_filename, invisible):
    layers = []
    filenames = {}
    for layer in img.layers:
        if layer.visible or invisible:
            expr = re.compile('\W')
            name = expr.sub("_", layer.name)
            layer.name = name
            if name in filenames.keys():
                continue
            filename = os.path.join(save_path, name + ".png")
            filenames[name] = filename
            layers.append(layer)

    edje_file = open(os.path.join(save_path, edje_filename), 'w')
    write_image_section(layers, edje_file, filenames)

    edje_file.write('collections {\n   group {\n      name: "%s";\n' % img.name)
    edje_file.write('      min: %d %d;\n      max: %d %d;\n      parts {\n' % (img.width, img.height,img.width, img.height))
    layers.reverse()
    for l in layers:
        if save_layers:
            save_layer(l, filenames[l.name])
        write_layer_data(l, edje_file,filenames[l.name])
    edje_file.write('      }\n   }\n}\n')
    edje_file.close()

register(
    "python_fu_edje_save",
    "Export the Image as Edje file",
    "Export the Image as Edje file",
    "Renato Chencarek",
    "Renato Chencarek",
    "2007",
    "<Image>/Python-Fu/_Edje Save",
    "RGBA, GRAYA",
    [
        (PF_STRING, "save_path",  "Path",  "/tmp/"),
        (PF_TOGGLE, "save_layers",  "Save Layers ?", True),
        (PF_STRING, "edje_filename",  "Filename for edje",  "edje.edc"),
        (PF_TOGGLE, "export_invisible_layers",  "Export invisible layers", False)
    ],
    [],
    edje_save)

main()
