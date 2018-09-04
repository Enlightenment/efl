#!/usr/bin/env python3
# encoding: utf-8
from enum import IntEnum


class Function_List_Type(IntEnum):
    INHERITIS = 1
    INHERITIS_FULL = 2
    CLASS_IMPLEMENTS = 3
    CLASS_ONLY = 4


class EKeys:
    def __init__(self, ext):
        self.ext = ext
        self.dicttypes = {}
        self.keywords = []
        self.verbs = []
        self.blacklist = []
        self.keyloads = ["init", "shutdown", "custom"]
        self.implementsbl = ["construtor", "destructor", "finalize"]
        self.funclist = Function_List_Type.CLASS_IMPLEMENTS

    def type_convert(self, eotype):
        return eotype.name

    def event_convert(self, event):
        return event.c_name

    def print_arg(self, eoarg):
        return "arg_{}".format(eoarg.name)

    def format_name(self, func):
        return self.name


class EMonoKeys(EKeys):
    def __init__(self, ext):
        super().__init__(ext)
        self.dicttypes = {
            "byte": "sbyte",
            "llong": "long",
            "int8": "sbyte",
            "int16": "short",
            "int32": "int",
            "int64": "long",
            "ssize": "long",
            "ubyte": "byte",
            "ullong": "ulong",
            "uint8": "byte",
            "uint16": "ushort",
            "uint32": "uint",
            "uint64": "ulong",
            "size": "ulong",
            "ptrdiff": "long",
            "intptr": "System.IntPtr",
            "uintptr": "System.IntPtr",
            "void_ptr": "System.IntPtr",
            "void": "System.IntPtr",  # only if is out/inout
            "Error": "eina.Error",
            "string": "System.String",
            "mstring": "System.String",
            "stringshare": "System.String",
            "any_value": "eina.Value",
            "any_value_ptr": "eina.Value"
            # complex Types
            ,
            "list": "eina.List",
            "inlist": "eina.Inlist",
            "array": "eina.Array",
            "inarray": "eina.Inarray",
            "hash": "eina.Hash",
            "promise": "int",
            "future": "int",
            "iterator": "eina.Iterator",
            "accessor": "int",
        }

        self.keywords = [
            "delete",
            "do",
            "lock",
            "event",
            "in",
            "object",
            "interface",
            "string",
            "internal",
            "fixed",
            "base",
        ]

        self.verbs = [
            "add",
            "get",
            "is",
            "del",
            "thaw",
            "freeze",
            "save",
            "wait",
            "eject",
            "raise",
            "lower",
            "load",
            "dup",
            "reset",
            "unload",
            "close",
            "set",
            "interpolate",
            "has",
            "grab",
            "check",
            "find",
            "ungrab",
            "unset",
            "clear",
            "pop",
            "new",
            "peek",
            "push",
            "update",
            "show",
            "move",
            "hide",
            "calculate",
            "resize",
            "attach",
            "pack",
            "unpack",
            "emit",
            "call",
            "append",
        ]

        self.blacklist = [
            "efl_event_callback_array_priority_add",
            "efl_player_position_get",
            "efl_text_font_source",
            "efl_ui_focus_manager_focus_get",
            "efl_ui_widget_focus",
            "efl_ui_text_password",
            "elm_interface_scrollable_repeat_events",
            "elm_wdg_item_del",
            "elm_wdg_item_focus",
            "elm_interface_scrollable_mirrored_set",
            "evas_obj_table_mirrored",
            "edje_obj_load_error_get",
            "efl_ui_focus_user_parent_get",
            "efl_canvas_object_scale",  # duplicated signature
            "efl_access_parent_get",
            "efl_access_name",
            "efl_access_root_get",
            "efl_access_type_get",
            "efl_access_role_get",
            "efl_access_action_description",
            "efl_access_image_description",
            "efl_access_component_layer_get",  # duplicated signature
            "efl_access_component_alpha_get",
            "efl_access_component_size_get",
            "efl_ui_spin_button_loop_get",
            "efl_ui_list_model_size_get",
            "efl_ui_list_relayout_layout_do",
        ]

    def escape_keyword(self, key):
        key = "kw_{}".format(key) if key.lower() in self.keywords else key
        return "{}Add".format(key) if key == "Finalize" else key

    def direction_get(self, direction):
        if direction == direction.INOUT:
            return "ref"
        if direction == direction.OUT:
            return "out"
        return None

    def klass_name(self, eotype):
        *namespaces, name = eotype.name.split(".")
        namespaces = [self.escape_keyword(x.lower()) for x in namespaces]
        is_interface = eotype.type == eotype.type.CLASS
        k_name = ("I" if is_interface else "") + name
        return ".".join(namespaces + [k_name])

    def type_convert(self, eotype):
        if eotype.type == eotype.type.VOID:
            return "System.IntPtr"

        new_type = self.dicttypes.get(eotype.name, self.klass_name(eotype))
        if new_type != "int" and eotype.base_type:
            new_type = "{}<{}>".format(
                new_type,
                self.dicttypes.get(
                    eotype.base_type.name, self.klass_name(eotype.base_type)
                ),
            )

        return new_type

    def event_convert(self, event):
        return "{}Evt".format("".join([i.capitalize() for i in event.name.split(",")]))

    def print_arg(self, eoarg):
        r = super().print_arg(eoarg)
        prefix = self.direction_get(eoarg.direction) or None

        if prefix == "out" and (eoarg.type.name in ("Eina.Slice", "Eina.Rw_Slice")):
            prefix = "ref"

        if (
            not prefix
            and eoarg.type.is_ptr
            and eoarg.type.type == eoarg.type.type.REGULAR
            and eoarg.type.typedecl
            and eoarg.type.typedecl.type == eoarg.type.typedecl.type.STRUCT
        ):
            prefix = "ref"

        return " ".join([prefix, r]) if prefix else r

    def format_name(self, func):
        names = func.comp.name.split("_")

        if func.type == func.type.METHOD and names[-1] in self.verbs:
            names.insert(0, names.pop())

        fname = "".join([name.capitalize() for name in names])

        if func.type == func.type.METHOD:
            fname = self.escape_keyword(fname)
            fname = "Do{}".format(fname) if fname == func.class_.short_name else fname

        return fname


def GetKey(ext):
    if ext == ".cs":
        return EMonoKeys(ext)
    return EKeys(ext)
