#!/bin/env python3
import os
import subprocess
import argparse
import json
from elftools.elf.elffile import ELFFile


#meson changed behaviour from 0.49 to 0.50 so we need this:
def meson_fetch_filename(filename_object):
  if isinstance(filename_object, str):
    return filename_object
  else:
    return filename_object[0]


def needed_libs(filename):
  print('Processing file:', filename)
  result = []
  with open(filename, 'rb') as f:
    elffile = ELFFile(f)
    for section in elffile.iter_sections():
      if section.name.startswith('.dynamic'):
        for tag in section.iter_tags():
          if tag.entry.d_tag == 'DT_NEEDED':
            result.append(getattr(tag, tag.entry.d_tag[3:].lower()))
  return result


parser = argparse.ArgumentParser(description='Check that when build with efl-one that no module nor efl-one lib does drag in libeina or the likes')
parser.add_argument('builddir', metavar='build', help='the path where to find the meson build directory')

G = parser.parse_args()

#Run meson to fetch all examples
meson_introspect = subprocess.Popen(["meson", "introspect", G.builddir, "--targets"],
      stdout = subprocess.PIPE,
      stderr = subprocess.PIPE,
)
meson_introspect.poll()
build_targets = json.loads(meson_introspect.stdout.read())
build_modules = [meson_fetch_filename(b["filename"]) for b in build_targets if "modules" in meson_fetch_filename(b["filename"]) and meson_fetch_filename(b["filename"]).endswith('.so')]

for build_modules in build_modules:
  libs = needed_libs(build_modules)
  lib_es = [lib for lib in libs if lib.startswith("libe") and lib != "libefl-one.so.1"]
  if len(lib_es) != 0:
    print("Error, {} requies lib {}".format(build_modules, lib_es[0]))
    exit(-1)

print("Nothing wrong found!")

