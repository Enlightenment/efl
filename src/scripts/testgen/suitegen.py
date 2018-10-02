import itertools
import os
from pyolian.eolian import Eolian_Function_Type, Eolian_Class_Type
from .ekeys import GetKey, Function_List_Type


class BaseItem:
    def __init__(self, path, keys, prefix=""):
        self.path = path
        self.keys = keys
        self.prefix = prefix

    def __getattr__(self, attr):
        if not attr.split("_")[-1] in self.keys.keyloads:
            raise AttributeError("Error getting {}".format(attr))

        filename = os.path.join(self.path, self.prefix + attr) + self.keys.ext
        if os.path.isfile(filename):
            with open(filename, "r") as f:
                return f.read()
        return None


class ComItem(BaseItem):
    def __init__(self, comp, path, keys):
        super().__init__(path, keys)
        self.comp = comp

    def __getattr__(self, attr):
        if hasattr(self.comp, attr):
            return getattr(self.comp, attr)
        return super().__getattr__(attr)


class FuncItem(ComItem):
    def __init__(self, comp, path, keys):
        super().__init__(comp, os.path.join(path, comp.name), keys)

        self.has_getter = (
            comp.type in (Eolian_Function_Type.PROP_GET, Eolian_Function_Type.PROPERTY)
            and comp.full_c_getter_name not in keys.blacklist
            and not os.path.isfile("{}_get".format(os.path.join(path, comp.name)))
        )
        self.has_setter = (
            comp.type in (Eolian_Function_Type.PROP_SET, Eolian_Function_Type.PROPERTY)
            and comp.full_c_setter_name not in keys.blacklist
            and not os.path.isfile("{}_set".format(os.path.join(path, comp.name)))
        )

        self.is_enum = (
            lambda arg: arg.type
            and arg.type.typedecl
            and arg.type.typedecl.type == arg.type.typedecl.type.ENUM
        )
        self.is_number = lambda arg: arg.type and arg.type.builtin_type in (
            arg.type.builtin_type.INT,
            arg.type.builtin_type.UINT,
            arg.type.builtin_type.LONG,
            arg.type.builtin_type.ULONG,
            arg.type.builtin_type.LLONG,
            arg.type.builtin_type.ULLONG,
            arg.type.builtin_type.INT8,
            arg.type.builtin_type.UINT8,
            arg.type.builtin_type.INT16,
            arg.type.builtin_type.UINT16,
            arg.type.builtin_type.INT32,
            arg.type.builtin_type.UINT32,
            arg.type.builtin_type.INT64,
            arg.type.builtin_type.UINT64,
            arg.type.builtin_type.INT128,
            arg.type.builtin_type.UINT128,
        )

    @property
    def getter_args(self):
        return itertools.chain(self.getter_values, self.getter_keys)

    @property
    def setter_args(self):
        return itertools.chain(self.setter_values, self.setter_keys)

    @property
    def format_name(self):
        return self.keys.format_name(self)


class EventItem(ComItem):
    def __init__(self, comp, path, keys):
        self.myname = comp.name.replace(",", "_")
        super().__init__(comp, os.path.join(path, self.myname), keys)
        self.format_name = self.keys.event_convert(self)


class ClassItem(ComItem):
    def __init__(self, comp, path, keys):
        self.myname = os.path.splitext(comp.file)[0]
        super().__init__(comp, os.path.join(path, self.myname), keys)

        mfilter = (
            lambda f: f.full_c_method_name not in self.keys.blacklist
            and not os.path.isfile(os.path.join(self.path, f.name))
        )

        self.methods = [
            FuncItem(m, self.path, keys) for m in filter(mfilter, self.comp.methods)
        ]
        self._properties = [
            FuncItem(p, self.path, keys) for p in filter(mfilter, self.comp.properties)
        ]
        self.events = [EventItem(s, self.path, keys) for s in self.comp.events]

        if self.keys.funclist in (
            Function_List_Type.INHERITIS,
            Function_List_Type.INHERITIS_FULL,
        ):
            for eoclass in (
                eoclass.inherits
                if self.keys.funclist == Function_List_Type.INHERITIS
                else eoclass.inherits_full
            ):
                for f in filter(mfilter, eoclass.methods):
                    self.methods.append(FuncItem(f, self.path, keys))
                for f in filter(mfilter, eoclass.properties):
                    self._properties.append(FuncItem(f, self.path, keys))
        elif self.keys.funclist == Function_List_Type.CLASS_IMPLEMENTS:
            for imp in filter(
                lambda i: not i.namespace == self.name
                and not i.short_name.lower() in self.keys.implementsbl,
                comp.implements,
            ):  # FIXME implements list
                f = imp.function
                if f.type == Eolian_Function_Type.METHOD and mfilter(f):
                    self.methods.append(FuncItem(f, self.path, keys))
                if f.type in (
                    Eolian_Function_Type.PROPERTY,
                    Eolian_Function_Type.PROP_GET,
                    Eolian_Function_Type.PROP_SET,
                ) and mfilter(f):
                    self._properties.append(FuncItem(f, self.path, keys))

    @property
    def properties(self):
        return filter(lambda p: p.has_setter or p.has_getter, self._properties)

    @property
    def properties_get(self):
        return filter(lambda p: p.has_getter, self._properties)

    @property
    def properties_set(self):
        return filter(lambda p: p.has_setter, self._properties)

    def __iter__(self):
        return itertools.chain(self.methods, self.properties)


class SuiteGen(BaseItem):
    def __init__(self, name, testname, filename, path, template=None):
        keys = GetKey(os.path.splitext(filename)[1])
        super().__init__(path, keys, name + "_")
        self.name = name
        self.testname = testname
        self.fullname = "_".join([name, testname]) if testname else name
        self.filename = filename
        self.template = template
        self.clslist = []

        if not self.template:
            script_path = os.path.dirname(os.path.realpath(__file__))
            self.template = os.path.join(
                script_path, "testgenerator{}.template".format(self.keys.ext)
            )

    def __iter__(self):
        return iter(self.clslist)

    def type_convert(self, eotype):
        return self.keys.type_convert(eotype)

    def print_arg(self, eoarg):
        return self.keys.print_arg(eoarg)

    def loadFiles(self, eolian_db, eofiles):
        self.clslist.clear()
        for eofile in eofiles:
            eocls = eolian_db.class_by_file_get(os.path.basename(eofile))
            if not eocls or eocls.type != Eolian_Class_Type.REGULAR:
                continue
            self.loadObj(eocls)

    def loadObj(self, eocls):
        cls = ClassItem(eocls, self.path, self.keys)
        if not os.path.isfile(cls.path):
            cls.myfullname = "{}_{}".format(self.fullname, cls.myname)
            self.clslist.append(cls)
        else:
            print("removing {} Class from generated list".format(cls.name))
