#!/usr/bin/env python3
# encoding: utf-8

"""Helper module with naming rules  for the C# binding."""

import os
import sys

# Hackish way of detecting pyolian...
script_path = os.path.dirname(os.path.realpath(__file__))

if "EFL_DIR" in os.environ:
    root_path = os.environ["EFL_DIR"]
else:
    root_path = os.path.abspath(os.path.join(script_path, "..", "..", ".."))

sys.path.insert(0, os.path.join(root_path, "src", "scripts"))

from pyolian import eolian

def remove_underlines(name):
    """Removes underlines from name"""
    return name.replace("_", "")


def managed_namespaces(namespaces):
    """Converts an eolian list of namespaces into the managed namespace"""
    return ".".join(remove_underlines(nsp) for nsp in namespaces)


def class_managed_name(cls):
    """Gets the full managed name of the given eolian class"""
    ret = managed_namespaces(cls.namespaces)

    if ret:
        ret += "."

    if cls.type in (eolian.Eolian_Class_Type.INTERFACE, eolian.Eolian_Class_Type.MIXIN):
        ret += "I"

    ret += remove_underlines(cls.short_name)

    if ret == 'Efl.Class':
        return 'System.Type'

    return ret

def type_managed_name(type):
    """Gets the full managed name of a given type."""
    if type.type == eolian.Eolian_Type_Type.CLASS:
        return class_managed_name(type.class_)

    ret = managed_namespaces(type.namespaces)

    if ret:
        ret += "."

    ret += remove_underlines(type.short_name)

    return ret
