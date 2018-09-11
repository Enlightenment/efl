import itertools
import os
from testgen.ekeys import GetKey

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

    @property
    def format_name(self):
        names = self.comp.name.split("_")
        if names[-1] in self.keys.verbs:
            names.insert(0, names.pop())
        return "".join([name.capitalize() for name in names])

class ClassItem(ComItem):
    def __init__(self, comp, path, keys):
        self.myname = os.path.splitext(comp.file)[0]
        super().__init__(comp, os.path.join(path, self.myname), keys)

        self.methods = [
            FuncItem(m, self.path, keys)
            for m in self.comp.methods
            if m.full_c_method_name not in self.keys.blacklist
        ]

        self.properties = [
            FuncItem(p, self.path, keys)
            for p in self.comp.properties
            if (
                p.getter_scope == p.getter_scope.PUBLIC
                and p.full_c_getter_name not in self.keys.blacklist
            )
            or (
                p.setter_scope == p.setter_scope.PUBLIC
                and p.full_c_setter_name not in self.keys.blacklist
            )
        ]

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
            if not eocls or eocls.type != eocls.type.REGULAR:
                continue
            self.loadObj(eocls)

    def loadObj(self, eocls):
        cls = ClassItem(eocls, self.path, self.keys)
        cls.myfullname = "{}_{}".format(self.fullname, cls.myname)
        self.clslist.append(cls)
