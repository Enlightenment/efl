import itertools
import os
from pyolian.eolian import Eolian_Function_Type, Eolian_Class_Type, Eolian_Object_Scope
from .ekeys import GetKey, Function_List_Type
from pyolian import eolian

from testgen import name_helpers


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
            and comp.getter_scope == Eolian_Object_Scope.PUBLIC
            and not os.path.isfile("{}_get".format(os.path.join(path, comp.name)))
        )
        self.has_setter = (
            comp.type in (Eolian_Function_Type.PROP_SET, Eolian_Function_Type.PROPERTY)
            and comp.full_c_setter_name not in keys.blacklist
            and comp.setter_scope == Eolian_Object_Scope.PUBLIC
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

        def mfilter(f):
            if f.full_c_method_name in self.keys.blacklist:
                return False

            if os.path.isfile(os.path.join(self.path, f.name)):
                return False

            if f.type == Eolian_Function_Type.PROPERTY:
                if f.getter_scope != Eolian_Object_Scope.PUBLIC:
                    scope = f.setter_scope
                else:
                    scope = f.getter_scope
            else:
                scope = f.scope_get(f.type)

            if scope != Eolian_Object_Scope.PUBLIC:
                return False

            return True

        if self.keys.funclist & Function_List_Type.OWN:
            self._methods = {
                m.name: FuncItem(m, self.path, keys)
                for m in self.comp.methods
                if mfilter(m)
            }
            self._properties = {
                p.name: FuncItem(p, self.path, keys)
                for p in self.comp.properties
                if mfilter(p)
            }
            self._events = {
                e.name: EventItem(e, self.path, keys) for e in self.comp.events
            }
        else:
            self._methods = {}
            self._properties = {}
            self._events = {}

        if self.keys.funclist & Function_List_Type.IMPLEMENTS:
            for imp in comp.implements:

                if (
                    imp.namespace == self.name
                    or imp.short_name.lower() in self.keys.implementsbl
                ):
                    continue

                f = imp.function

                if not mfilter(f):
                    continue

                if f.type == Eolian_Function_Type.METHOD:
                    if f.name in self._methods:
                        continue
                    self._methods[f.name] = FuncItem(f, self.path, keys)
                elif f.type in (
                    Eolian_Function_Type.PROPERTY,
                    Eolian_Function_Type.PROP_GET,
                    Eolian_Function_Type.PROP_SET,
                ):
                    if f.name in self._properties:
                        continue
                    self._properties[f.name] = FuncItem(f, self.path, keys)

        parents = []

        if self.keys.funclist & Function_List_Type.INHERITS_FULL:
            # Use inherits full to get inherited interfaces too
            parents = self.comp.inherits_full
        else:
            if self.keys.funclist & Function_List_Type.EXTENSIONS:
                parents = self.comp.extensions_hierarchy

            if self.keys.funclist & Function_List_Type.INHERITED:
                if parents:
                    parents = itertools.chain(self.comp.hierarchy, parents)
                else:
                    parents = self.comp.hierarchy

        for eoclass in parents:
            for f in filter(mfilter, eoclass.methods):
                if f.name in self._methods:
                    continue
                self._methods[f.name] = FuncItem(f, self.path, keys)
            for f in filter(mfilter, eoclass.properties):
                if f.name in self._properties:
                    continue
                self._properties[f.name] = FuncItem(f, self.path, keys)

    @property
    def properties(self):
        return filter(lambda p: p.has_setter or p.has_getter, self._properties.values())

    @property
    def properties_get(self):
        return filter(lambda p: p.has_getter, self._properties.values())

    @property
    def properties_set(self):
        return filter(lambda p: p.has_setter, self._properties.values())

    @property
    def methods(self):
        return self._methods.values()

    @property
    def events(self):
        return self._events.values()

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
        if eotype.type == eolian.Eolian_Type_Type.CLASS:
            return name_helpers.class_managed_name(eotype.class_)

        if eotype.typedecl:
            return name_helpers.type_managed_name(eotype)

        return self.keys.type_convert(eotype)

    def constructor_params(self, cls):
        ret = []

        constructors = itertools.chain(
            cls.constructors, *[base.constructors for base in cls.inherits_full]
        )

        for constructor in constructors:
            # Skip optional constructors for now
            if constructor.is_optional:
                continue
            func = constructor.function

            if func.type == eolian.Eolian_Function_Type.PROPERTY:
                first_param = list(func.setter_values)[0]
            else:
                first_param = list(func.parameters)[0]
            param_type = first_param.type

            ret.append("default({})".format(name_helpers.type_managed_name(param_type)))

        return (", " if ret else "") + ", ".join(ret)

    def print_arg(self, eoarg):
        return self.keys.print_arg(eoarg)

    def intersect(self, a, b):
        return list(set(a) & set(b))

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
