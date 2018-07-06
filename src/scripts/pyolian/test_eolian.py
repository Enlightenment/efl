#!/usr/bin/env python3
# encoding: utf-8
"""
Pyolian test suite.

Just run this file to execute the full suite.

A return value of 0 means all test passed successfully.

"""
import os
import unittest

import eolian


# Use .eo files from the source tree (not the installed ones)
script_path = os.path.dirname(os.path.realpath(__file__))
root_path = os.path.abspath(os.path.join(script_path, '..', '..', '..'))
SCAN_FOLDER = os.path.join(root_path, 'src', 'lib')

# the main Eolian unit state
eolian_db = None


class TestBaseObject(unittest.TestCase):
    def test_base_object_equality(self):
        cls1 = eolian_db.class_get_by_name('Efl.Loop.Timer')
        cls2 = eolian_db.class_get_by_file('efl_loop_timer.eo')
        self.assertIsInstance(cls1, eolian.Class)
        self.assertIsInstance(cls2, eolian.Class)
        self.assertEqual(cls1, cls2)
        self.assertEqual(cls1, 'Efl.Loop.Timer')
        self.assertEqual(cls2, 'Efl.Loop.Timer')
        self.assertNotEqual(cls1, 'another string')
        self.assertNotEqual(cls1, 1234)
        self.assertNotEqual(cls1, None)
        self.assertNotEqual(cls1, 0)

        enum1 = eolian_db.typedecl_enum_get_by_name('Efl.Ui.Focus.Direction')
        enum2 = eolian_db.typedecl_enum_get_by_name('Efl.Ui.Focus.Direction')
        self.assertIsInstance(enum1, eolian.Typedecl)
        self.assertIsInstance(enum2, eolian.Typedecl)
        self.assertEqual(enum1, enum2)
        self.assertEqual(enum1, 'Efl.Ui.Focus.Direction')
        self.assertEqual(enum2, 'Efl.Ui.Focus.Direction')
        self.assertNotEqual(enum1, 'another string')
        self.assertNotEqual(enum1, 1234)
        self.assertNotEqual(enum1, None)
        self.assertNotEqual(enum1, 0)

        self.assertNotEqual(cls1, enum1)


class TestEolianUnit(unittest.TestCase):
    def test_file_listing(self):
        l = list(eolian_db.all_eo_file_paths)
        self.assertGreater(len(l), 400)
        self.assertTrue(l[0].endswith('.eo'))

        l = list(eolian_db.all_eo_files)
        self.assertGreater(len(l), 400)
        self.assertTrue(l[0].endswith('.eo'))

        l = list(eolian_db.all_eot_file_paths)
        self.assertGreater(len(l), 10)
        self.assertTrue(l[0].endswith('.eot'))

        l = list(eolian_db.all_eot_files)
        self.assertGreater(len(l), 10)
        self.assertTrue(l[0].endswith('.eot'))

    def test_enum_listing(self):
        l = list(eolian_db.typedecl_enums_get_by_file('efl_ui_win.eo'))
        self.assertGreater(len(l), 5)
        self.assertIsInstance(l[0], eolian.Typedecl)
        self.assertEqual(l[0].type, eolian.Eolian_Typedecl_Type.ENUM)

        all_count = 0
        for enum in eolian_db.typedecl_all_enums:
            self.assertIsInstance(enum, eolian.Typedecl)
            self.assertEqual(enum.type, eolian.Eolian_Typedecl_Type.ENUM)
            all_count += 1
        self.assertGreater(all_count, 50)

    def test_struct_listing(self):
        l = list(eolian_db.typedecl_structs_get_by_file('eina_types.eot'))
        self.assertGreater(len(l), 10)
        self.assertIsInstance(l[0], eolian.Typedecl)
        self.assertIn(l[0].type, (
                        eolian.Eolian_Typedecl_Type.STRUCT,
                        eolian.Eolian_Typedecl_Type.STRUCT_OPAQUE))

        all_count = 0
        for struct in eolian_db.typedecl_all_structs:
            self.assertIsInstance(struct, eolian.Typedecl)
            self.assertIn(struct.type, (
                            eolian.Eolian_Typedecl_Type.STRUCT,
                            eolian.Eolian_Typedecl_Type.STRUCT_OPAQUE))
            all_count += 1
        self.assertGreater(all_count, 50)

    def test_alias_listing(self):
        l = list(eolian_db.typedecl_aliases_get_by_file('edje_types.eot'))
        self.assertGreater(len(l), 5)
        self.assertIsInstance(l[0], eolian.Typedecl)

        all_count = 0
        for alias in eolian_db.typedecl_all_aliases:
            self.assertIsInstance(alias, eolian.Typedecl)
            self.assertIn(alias.type, (
                            eolian.Eolian_Typedecl_Type.ALIAS,
                            eolian.Eolian_Typedecl_Type.FUNCTION_POINTER)) # TODO is this correct ??
            all_count += 1
        self.assertGreater(all_count, 10)

    def test_variable_listing(self):
        l = list(eolian_db.variable_all_constants)
        self.assertGreater(len(l), 2)
        self.assertIsInstance(l[0], eolian.Variable)

        l = list(eolian_db.variable_all_globals)
        self.assertGreater(len(l), 20)
        self.assertIsInstance(l[0], eolian.Variable)

        l = list(eolian_db.variable_constants_get_by_file('efl_gfx_stack.eo'))
        self.assertGreater(len(l), 1)
        self.assertIsInstance(l[0], eolian.Variable)

        l = list(eolian_db.variable_globals_get_by_file('efl_net_http_types.eot'))
        self.assertGreater(len(l), 10)
        self.assertIsInstance(l[0], eolian.Variable)

    def test_declaration_listing(self):
        l = list(eolian_db.declarations_get_by_file('eina_types.eot'))
        self.assertGreater(len(l), 10)
        self.assertIsInstance(l[0], eolian.Declaration)

        l = list(eolian_db.all_declarations)
        self.assertGreater(len(l), 100)
        self.assertIsInstance(l[0], eolian.Declaration)

    def test_class_listing(self):
        all_count = 0
        for cls in eolian_db.all_classes:
            self.assertIsInstance(cls, eolian.Class)
            all_count += 1
        self.assertGreater(all_count, 400)


class TestEolianNamespace(unittest.TestCase):
    def test_all_namespace(self):
        count = 0
        for ns in eolian_db.all_namespaces:
            self.assertIsInstance(ns, eolian.Namespace)
            count += 1
        self.assertGreater(count, 200)

    def test_namespace_equality(self):
        ns1 = eolian.Namespace(eolian_db, 'Efl.Io')
        ns2 = eolian.Namespace(eolian_db, 'Efl.Net')
        self.assertIsInstance(ns1, eolian.Namespace)
        self.assertIsInstance(ns2, eolian.Namespace)
        self.assertNotEqual(ns1, ns2)
        self.assertEqual(ns1, eolian.Namespace(eolian_db, 'Efl.Io'))
        self.assertEqual(ns2, eolian.Namespace(eolian_db, 'Efl.Net'))

    def test_namespace_sorting(self):
        nspaces = eolian_db.all_namespaces
        nspaces.sort(reverse=True)
        self.assertGreater(nspaces[0], nspaces[-1])
        self.assertLess(nspaces[1], nspaces[0])

    def test_namespace_by_name(self):
        ns = eolian.Namespace(eolian_db, 'Efl.Net')
        self.assertIsInstance(ns, eolian.Namespace)
        self.assertEqual(ns.name, 'Efl.Net')
        self.assertEqual(ns.namespaces, ['Efl', 'Net'])

        ns = eolian_db.namespace_get_by_name('Efl')
        self.assertIsInstance(ns, eolian.Namespace)
        self.assertEqual(ns.name, 'Efl')

        self.assertGreater(len(ns.classes), 30)
        for cls in ns.classes:
            self.assertIsInstance(cls, eolian.Class)
        self.assertGreater(len(ns.regulars), 4)
        for cls in ns.regulars:
            self.assertIsInstance(cls, eolian.Class)
            self.assertEqual(cls.type, eolian.Eolian_Class_Type.REGULAR)
        self.assertGreater(len(ns.mixins), 0)
        for cls in ns.mixins:
            self.assertIsInstance(cls, eolian.Class)
            self.assertEqual(cls.type, eolian.Eolian_Class_Type.MIXIN)
        self.assertGreater(len(ns.interfaces), 15)
        for cls in ns.interfaces:
            self.assertIsInstance(cls, eolian.Class)
            self.assertEqual(cls.type, eolian.Eolian_Class_Type.INTERFACE)

        self.assertGreater(len(ns.enums), 1)
        for td in ns.enums:
            self.assertIsInstance(td, eolian.Typedecl)
            self.assertEqual(td.type, eolian.Eolian_Typedecl_Type.ENUM)
        self.assertGreater(len(ns.aliases), 0)
        for td in ns.aliases:
            self.assertIsInstance(td, eolian.Typedecl)
            # TODO eolian_typedecl_all_aliases_get also return FUNCTION_POINTER
            # is this correct? or an eolian bug ?
            #  self.assertEqual(td.type, eolian.Eolian_Typedecl_Type.ALIAS)
        self.assertGreater(len(ns.structs), 2)
        for td in ns.structs:
            self.assertIsInstance(td, eolian.Typedecl)
            self.assertEqual(td.type, eolian.Eolian_Typedecl_Type.STRUCT)


class TestEolianClass(unittest.TestCase):
    def test_class(self):
        cls = eolian_db.class_get_by_file('efl_loop_timer.eo')
        self.assertIsInstance(cls, eolian.Class)

        cls = eolian_db.class_get_by_name('Efl.Loop.Timer')
        self.assertIsInstance(cls, eolian.Class)

        self.assertEqual(cls.name, 'Timer')
        self.assertEqual(cls.full_name, 'Efl.Loop.Timer')
        self.assertEqual(cls.file, 'efl_loop_timer.eo')
        self.assertEqual(list(cls.namespaces), ['Efl', 'Loop'])
        self.assertEqual(cls.type, eolian.Eolian_Class_Type.REGULAR)
        self.assertIsInstance(cls.documentation, eolian.Documentation)
        self.assertEqual(cls.legacy_prefix, 'ecore_timer')
        self.assertIsNone(cls.eo_prefix)  # TODO fin a class with a value
        self.assertIsNone(cls.event_prefix)  # TODO same as above
        self.assertIsNone(cls.data_type)  # TODO same as above
        self.assertEqual(cls.base_class.full_name, 'Efl.Loop.Consumer')
        self.assertEqual([c.full_name for c in cls.inherits], ['Efl.Loop.Consumer'])
        self.assertEqual([c.full_name for c in cls.inherits_full], ['Efl.Loop.Consumer', 'Efl.Object'])
        self.assertEqual([c.full_name for c in cls.hierarchy], ['Efl.Loop.Consumer', 'Efl.Object'])
        self.assertFalse(cls.ctor_enable)
        self.assertFalse(cls.dtor_enable)
        self.assertEqual(cls.c_get_function_name, 'efl_loop_timer_class_get')
        self.assertEqual(cls.c_name, 'EFL_LOOP_TIMER_CLASS')
        self.assertEqual(cls.c_data_type, 'Efl_Loop_Timer_Data')
        self.assertEqual([f.name for f in cls.methods], ['reset','loop_reset','delay'])
        self.assertEqual([f.name for f in cls.properties], ['interval','pending'])
        self.assertGreater(len(list(cls.implements)), 5)
        self.assertIsInstance(list(cls.implements)[0], eolian.Implement)


class TestEolianFunction(unittest.TestCase):
    def test_function(self):
        cls = eolian_db.class_get_by_name('Efl.Loop.Timer')
        f = cls.function_get_by_name('delay')
        self.assertIsInstance(f, eolian.Function)
        self.assertEqual(f.name, 'delay')
        self.assertEqual(f.type, eolian.Eolian_Function_Type.METHOD)
        self.assertEqual(f.method_scope, eolian.Eolian_Object_Scope.PUBLIC)
        self.assertEqual(f.getter_scope, eolian.Eolian_Object_Scope.UNKNOWN)  # TODO correct?
        self.assertEqual(f.setter_scope, eolian.Eolian_Object_Scope.UNKNOWN)  # TODO correct?
        self.assertEqual(f.full_c_method_name, 'efl_loop_timer_delay')
        self.assertEqual(f.full_c_getter_name, 'efl_loop_timer_delay_get')
        self.assertEqual(f.full_c_setter_name, 'efl_loop_timer_delay_set')
        self.assertEqual(f.full_c_method_name_legacy, 'ecore_timer_delay')
        self.assertEqual(f.full_c_getter_name_legacy, 'ecore_timer_delay_get')
        self.assertEqual(f.full_c_setter_name_legacy, 'ecore_timer_delay_set')
        self.assertIsNone(f.method_return_type)  # TODO correct ?
        self.assertIsNone(f.setter_return_type)  # TODO correct ?
        self.assertIsNone(f.getter_return_type)  # TODO correct ?
        self.assertFalse(f.is_legacy_only(eolian.Eolian_Function_Type.PROP_GET))
        self.assertFalse(f.is_class)
        self.assertFalse(f.is_beta)
        self.assertFalse(f.is_constructor(cls))
        #  # #assert f.is_function_pointer == False  # TODO broken somehow
        self.assertEqual(len(list(f.getter_values)), 1)
        self.assertEqual(len(list(f.getter_values)), 1)
        self.assertEqual(len(list(f.parameters)), 1)
        self.assertFalse(f.return_is_warn_unused(eolian.Eolian_Function_Type.METHOD))
        self.assertFalse(f.object_is_const)
        self.assertEqual(f.class_.full_name, 'Efl.Loop.Timer')
        self.assertIsInstance(f.implement, eolian.Implement)

    def test_function_parameter(self):
        cls = eolian_db.class_get_by_name('Efl.Loop.Timer')
        f = cls.function_get_by_name('delay')
        p = list(f.parameters)[0]
        self.assertEqual(p.direction, eolian.Eolian_Parameter_Dir.IN)
        self.assertEqual(p.name, 'add')
        self.assertIsNone(p.default_value)
        self.assertFalse(p.is_nonull)  # TODO is correct ?? 'add' can be null?
        self.assertFalse(p.is_nullable)
        self.assertFalse(p.is_optional)
        self.assertEqual(p.type.name, 'double')
        self.assertIsInstance(p.documentation, eolian.Documentation)


class TestEolianImplement(unittest.TestCase):
    def test_implement(self):
        cls = eolian_db.class_get_by_name('Efl.Loop.Timer')
        f = cls.function_get_by_name('delay')
        im = f.implement
        self.assertIsInstance(im, eolian.Implement)
        self.assertEqual(im.full_name, 'Efl.Loop.Timer.delay')
        self.assertIsInstance(im.class_, eolian.Class)
        self.assertIsInstance(im.function, eolian.Function)
        self.assertIsInstance(im.documentation_get(), eolian.Documentation) # TODO is UNRESOLVED correct ?
        self.assertFalse(im.is_auto())
        self.assertFalse(im.is_empty())
        self.assertFalse(im.is_pure_virtual())
        self.assertFalse(im.is_prop_set)
        self.assertFalse(im.is_prop_get)
        self.assertFalse(im.is_property)
        self.assertTrue(im.is_method)


class TestEolianEvent(unittest.TestCase):
    def test_event(self):
        cls = eolian_db.class_get_by_name('Efl.Loop.Timer')
        self.assertEqual([e.name for e in cls.events], ['tick'])
        ev = cls.event_get_by_name('tick')
        self.assertIsInstance(ev, eolian.Event)
        self.assertEqual(ev.name, 'tick')
        self.assertEqual(ev.c_name, 'EFL_LOOP_TIMER_EVENT_TICK')
        self.assertIsNone(ev.type)  # TODO is this correct
        self.assertIsInstance(ev.documentation, eolian.Documentation)
        self.assertEqual(ev.scope, eolian.Eolian_Object_Scope.PUBLIC)
        self.assertFalse(ev.is_beta)
        self.assertFalse(ev.is_hot)
        self.assertFalse(ev.is_restart)


class TestEolianPart(unittest.TestCase):
    def test_part(self):
        cls = eolian_db.class_get_by_name('Efl.Ui.Popup')
        parts = list(cls.parts)
        self.assertGreater(len(parts), 0)

        part = parts[0]
        self.assertEqual(part.name, 'backwall')
        self.assertIsInstance(part.class_, eolian.Class)
        self.assertEqual(part.class_.full_name, 'Efl.Ui.Popup.Part')
        self.assertIsInstance(part.documentation, eolian.Documentation)


class TestEolianConstructor(unittest.TestCase):
    def test_constructor(self):
        cls = eolian_db.class_get_by_name('Efl.Ui.Win')
        ctors = list(cls.constructors)
        self.assertGreater(len(ctors), 0)
        ctor = ctors[0]
        self.assertIsInstance(ctor, eolian.Constructor)
        self.assertEqual(ctor.full_name, 'Efl.Ui.Win.win_name')
        self.assertFalse(ctor.is_optional)
        self.assertIsInstance(ctor.class_, eolian.Class)
        self.assertEqual(ctor.class_.full_name, 'Efl.Ui.Win')
        self.assertIsInstance(ctor.function, eolian.Function)
        self.assertEqual(ctor.function.name, 'win_name')


class TestEolianDocumentation(unittest.TestCase):
    def test_documentation(self):
        td = eolian_db.class_get_by_name('Efl.Net.Control')
        doc = td.documentation
        self.assertIsInstance(doc, eolian.Documentation)
        self.assertIsInstance(doc.summary, str)
        self.assertGreater(len(doc.summary), 10)
        self.assertIsInstance(doc.description, str)
        self.assertGreater(len(doc.description), 20)
        self.assertEqual(doc.since, '1.19')


class TestEolianVariable(unittest.TestCase):
    def test_variable_global(self):
        var = eolian_db.variable_global_get_by_name('Efl.Net.Http.Error.BAD_CONTENT_ENCODING')
        self.assertIsInstance(var, eolian.Variable)
        self.assertEqual(var.full_name, 'Efl.Net.Http.Error.BAD_CONTENT_ENCODING')
        self.assertEqual(var.name, 'BAD_CONTENT_ENCODING')
        self.assertEqual(var.type, eolian.Eolian_Variable_Type.GLOBAL)
        self.assertEqual(var.file, 'efl_net_http_types.eot')
        self.assertFalse(var.is_extern)
        self.assertEqual(list(var.namespaces), ['Efl','Net','Http','Error'])
        self.assertIsInstance(var.documentation, eolian.Documentation)
        self.assertIsInstance(var.base_type, eolian.Type)
        self.assertIsNone(var.value)  # TODO is None correct here? no value?

    def test_variable_constant(self):
        var = eolian_db.variable_constant_get_by_name('Efl.Gfx.Size.Hint.Fill')
        self.assertIsInstance(var, eolian.Variable)
        self.assertEqual(var.full_name, 'Efl.Gfx.Size.Hint.Fill')
        self.assertEqual(var.name, 'Fill')
        self.assertEqual(var.type, eolian.Eolian_Variable_Type.CONSTANT)
        self.assertEqual(var.file, 'efl_gfx_size_hint.eo')
        self.assertFalse(var.is_extern)
        self.assertEqual(list(var.namespaces), ['Efl','Gfx','Size','Hint'])
        self.assertIsInstance(var.documentation, eolian.Documentation)
        self.assertIsInstance(var.base_type, eolian.Type)
        self.assertIsInstance(var.value, eolian.Expression)
        self.assertEqual(float(var.value.serialize), +1.0)


class TestEolianTypedecl(unittest.TestCase):
    def test_typedecl_enum(self):
        td = eolian_db.typedecl_enum_get_by_name('Efl.Net.Http.Version')
        self.assertIsInstance(td, eolian.Typedecl)
        self.assertEqual(td.name, 'Version')
        self.assertEqual(td.full_name, 'Efl.Net.Http.Version')
        self.assertEqual(td.file, 'efl_net_http_types.eot')
        self.assertEqual(list(td.namespaces), ['Efl','Net','Http'])
        self.assertIsInstance(td.documentation, eolian.Documentation)
        self.assertIsNone(td.base_type)  # TODO find a better test
        self.assertIsNone(td.free_func)  # TODO find a better test
        self.assertIsNone(td.function_pointer)  # TODO find a better test
        self.assertFalse(td.is_extern)
        self.assertEqual(len(list(td.enum_fields)), 3)
        self.assertEqual(td.c_type, 'enum Efl_Net_Http_Version { v1_0 = 100, v1_1 = 101, v2_0 = 200 }')

    def test_typedecl_enum_field(self):
        td = eolian_db.typedecl_enum_get_by_name('Efl.Net.Http.Version')
        field = td.enum_field_get('v1_0')
        self.assertIsInstance(field, eolian.Enum_Type_Field)
        self.assertEqual(field.name, 'v1_0')
        self.assertEqual(field.c_name, 'EFL_NET_HTTP_VERSION_V1_0')
        self.assertIsInstance(field.documentation, eolian.Documentation)
        self.assertIsInstance(field.value, eolian.Expression)

    def test_typedecl_struct(self):
        td = eolian_db.typedecl_struct_get_by_name('Efl.Gfx.Color32')
        self.assertIsInstance(td, eolian.Typedecl)
        self.assertEqual(td.name, 'Color32')
        self.assertEqual(td.full_name, 'Efl.Gfx.Color32')
        self.assertEqual(td.file, 'efl_canvas_filter_internal.eo')
        self.assertEqual(list(td.namespaces), ['Efl','Gfx'])
        self.assertIsInstance(td.documentation, eolian.Documentation)
        self.assertIsNone(td.base_type)  # TODO find a better test
        self.assertIsNone(td.free_func)  # TODO find a better test
        self.assertIsNone(td.function_pointer)  # TODO find a better test
        self.assertFalse(td.is_extern)
        self.assertEqual(len(list(td.struct_fields)), 4)
        self.assertEqual(td.c_type, 'struct Efl_Gfx_Color32 { uint8_t r; uint8_t g; uint8_t b; uint8_t a; }')

    def test_typedecl_struct_field(self):
        td = eolian_db.typedecl_struct_get_by_name('Efl.Gfx.Color32')
        field = td.struct_field_get('b')
        self.assertIsInstance(field, eolian.Struct_Type_Field)
        self.assertEqual(field.name, 'b')
        self.assertIsInstance(field.type, eolian.Type)
        self.assertIsInstance(field.documentation, eolian.Documentation)

    def test_typedecl_alias(self):
        alias = eolian_db.typedecl_alias_get_by_name('Eina.Error')
        self.assertIsInstance(alias, eolian.Typedecl)
        self.assertEqual(alias.type, eolian.Eolian_Typedecl_Type.ALIAS)
        self.assertEqual(alias.name, 'Error')
        self.assertEqual(alias.full_name, 'Eina.Error')
        self.assertIsInstance(alias.aliased_base, eolian.Type)
        self.assertEqual(alias.aliased_base.name, 'int')
        self.assertEqual(alias.c_type, 'typedef int Eina_Error')


class TestEolianType(unittest.TestCase):
    def test_type_regular_builtin(self):
        cls = eolian_db.class_get_by_name('Efl.Loop.Timer')
        func = cls.function_get_by_name('delay')
        param = list(func.parameters)[0]
        t = param.type  # type: double
        self.assertIsInstance(t, eolian.Type)
        self.assertEqual(t.name, 'double')
        self.assertEqual(t.full_name, 'double')
        self.assertEqual(t.type, eolian.Eolian_Type_Type.REGULAR)
        self.assertEqual(t.builtin_type, eolian.Eolian_Type_Builtin_Type.DOUBLE)
        self.assertEqual(t.file, 'efl_loop_timer.eo') # TODO is this correct ?
        self.assertIsNone(t.base_type)  # TODO find a better test
        self.assertIsNone(t.next_type)  # TODO find a better test
        self.assertFalse(t.is_owned)
        self.assertFalse(t.is_const)
        self.assertFalse(t.is_ptr)
        self.assertEqual(list(t.namespaces), [])
        self.assertIsNone(t.free_func)
        self.assertIsNone(t.class_)
        self.assertEqual(t, t.aliased_base)  # TODO find a better test

        self.assertEqual(t.c_type_default, 'double')  # TODO find a better test
        self.assertEqual(t.c_type_param, 'double')
        self.assertEqual(t.c_type_return, 'double')

    def test_type_regular(self):
        cls = eolian_db.class_get_by_name('Efl.Gfx')
        func = cls.function_get_by_name('geometry')
        param = list(func.setter_values)[0]
        t = param.type  # type: Eina.Rect
        self.assertIsInstance(t, eolian.Type)
        self.assertEqual(t.name, 'Rect')
        self.assertEqual(t.full_name, 'Eina.Rect')
        self.assertEqual(t.type, eolian.Eolian_Type_Type.REGULAR)
        self.assertEqual(t.builtin_type, eolian.Eolian_Type_Builtin_Type.INVALID)
        self.assertEqual(t.file, 'efl_gfx.eo')  # TODO is this correct ?
        self.assertEqual(list(t.namespaces), ['Eina'])
        self.assertEqual(t.free_func, 'eina_rectangle_free')
        self.assertIsNone(t.class_)
        self.assertEqual(t, t.aliased_base)

        self.assertEqual(t.c_type_default, 'Eina_Rect')  # TODO find a better test
        self.assertEqual(t.c_type_param, 'Eina_Rect')
        self.assertEqual(t.c_type_return, 'Eina_Rect')

        td = t.typedecl
        self.assertIsInstance(td, eolian.Typedecl)
        self.assertEqual(td.full_name, 'Eina.Rect')

    def test_type_class(self):
        cls = eolian_db.class_get_by_name('Efl.Content')
        func = cls.function_get_by_name('content')
        param = list(func.setter_values)[0]
        t = param.type  # type: Efl.Gfx (class interface)
        self.assertIsInstance(t, eolian.Type)
        self.assertEqual(t.name, 'Gfx')
        self.assertEqual(t.full_name, 'Efl.Gfx')
        self.assertEqual(t.type, eolian.Eolian_Type_Type.CLASS)
        self.assertEqual(t.builtin_type, eolian.Eolian_Type_Builtin_Type.INVALID)
        self.assertEqual(t.file, 'efl_content.eo')  # TODO is this correct ?
        self.assertEqual(list(t.namespaces), ['Efl'])
        self.assertEqual(t.free_func, 'efl_del')
        self.assertEqual(t, t.aliased_base)

        self.assertEqual(t.c_type_default, 'Efl_Gfx *')  # TODO find a better test
        self.assertEqual(t.c_type_param, 'Efl_Gfx *')
        self.assertEqual(t.c_type_return, 'Efl_Gfx *')

        cls = t.class_
        self.assertIsInstance(cls, eolian.Class)
        self.assertEqual(cls.full_name, 'Efl.Gfx')


class TestEolianDeclaration(unittest.TestCase):
    def test_declaration(self):
        d = eolian_db.declaration_get_by_name('Eina.File')
        self.assertIsInstance(d, eolian.Declaration)
        self.assertEqual(d.name, 'Eina.File')
        self.assertEqual(d.type, eolian.Eolian_Declaration_Type.STRUCT)
        #  self.assertIsNone(d.class_)  # TODO find a better test
        #  self.assertIsNone(d.variable)  # TODO find a better test
        self.assertIsInstance(d.data_type, eolian.Typedecl)
        self.assertEqual(d.data_type.full_name, 'Eina.File')


class TestEolianExpression(unittest.TestCase):
    def test_expression_simple(self):
        td = eolian_db.typedecl_enum_get_by_name('Efl.Net.Http.Version')
        field = td.enum_field_get('v1_0')
        exp = field.value
        self.assertIsInstance(exp, eolian.Expression)
        self.assertEqual(exp.type, eolian.Eolian_Expression_Type.INT)
        self.assertEqual(exp.serialize, '100')

    def test_expression_unary(self):
        var = eolian_db.variable_constant_get_by_name('Efl.Gfx.Size.Hint.Fill')
        exp = var.value
        self.assertIsInstance(exp, eolian.Expression)
        self.assertEqual(exp.type, eolian.Eolian_Expression_Type.UNARY)
        self.assertEqual(float(exp.serialize), 1.0)  # TODO is this a bug? isn't -1.0 ?
        self.assertEqual(exp.unary_operator, eolian.Eolian_Unary_Operator.UNM)
        unary = exp.unary_expression
        self.assertIsInstance(unary, eolian.Expression)
        self.assertEqual(unary.type, eolian.Eolian_Expression_Type.DOUBLE)
        self.assertEqual(float(exp.serialize), 1.0)

        # TODO test_expression_binary
        #  exp.binary_operator # TODO find a better test (only works for BINARY expr)
        #  exp.binary_lhs # TODO find a better test (only works for BINARY expr)
        #  exp.binary_rhs # TODO find a better test (only works for BINARY expr)



if __name__ == '__main__':
    # create main eolian state
    eolian_db = eolian.Eolian()
    if not isinstance(eolian_db, eolian.Eolian):
        raise(RuntimeError('Eolian, failed to create Eolian state'))

    # eolian system scan (BROKEN)
    #  if not eolian_db.system_directory_scan():
        #  raise(RuntimeError('Eolian, failed to scan system directories'))

    # eolian source tree scan
    if not eolian_db.directory_scan(SCAN_FOLDER):
        raise(RuntimeError('Eolian, failed to scan source directory'))

    # Parse all known eo files
    if not eolian_db.all_eot_files_parse():
        raise(RuntimeError('Eolian, failed to parse all EOT files'))

    if not eolian_db.all_eo_files_parse():
        raise(RuntimeError('Eolian, failed to parse all EO files'))

    # start the test suite
    suite = unittest.main(verbosity=2, exit=False)

    # cleanup (or it will segfault on gc, that happend after atexit)
    del eolian_db

    # exit 0 (success) or 1 (failure)
    exit(0 if suite.result.wasSuccessful() else 1)
