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
        cls1 = eolian_db.class_by_name_get('Efl.Loop_Timer')
        cls2 = eolian_db.class_by_file_get('efl_loop_timer.eo')
        self.assertIsInstance(cls1, eolian.Class)
        self.assertIsInstance(cls2, eolian.Class)
        self.assertEqual(cls1, cls2)
        self.assertEqual(cls1, 'Efl.Loop_Timer')
        self.assertEqual(cls2, 'Efl.Loop_Timer')
        self.assertNotEqual(cls1, 'another string')
        self.assertNotEqual(cls1, 1234)
        self.assertNotEqual(cls1, None)
        self.assertNotEqual(cls1, 0)

        enum1 = eolian_db.enum_by_name_get('Efl.Ui.Focus.Direction')
        enum2 = eolian_db.enum_by_name_get('Efl.Ui.Focus.Direction')
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


class TestEolianState(unittest.TestCase):
    def test_unit_getters(self):
        count = 0
        for unit in eolian_db.units:
            self.assertIsInstance(unit, eolian.Eolian_Unit)
            self.assertTrue(unit.file.endswith(('.eo', '.eot')))
            count += 1
        self.assertGreater(count, 500)

        unit = eolian_db.unit_by_file_get('efl_ui_win.eo')
        self.assertIsInstance(unit, eolian.Eolian_Unit)
        self.assertEqual(unit.file, 'efl_ui_win.eo')

    def test_object_getters(self):
        obj = eolian_db.object_by_name_get('Efl.Ui.Frame')
        self.assertIsInstance(obj, eolian.Object)
        self.assertFalse(type(obj) == eolian.Object)
        self.assertEqual(obj.name, 'Efl.Ui.Frame')

        count = 0
        for obj in eolian_db.objects:
            self.assertIsInstance(obj, eolian.Object)
            self.assertFalse(type(obj) == eolian.Object)
            count += 1
        self.assertGreater(count, 800)

        count = 0
        for obj in eolian_db.objects_by_file_get('efl_loop.eo'):
            self.assertIsInstance(obj, eolian.Object)
            self.assertFalse(type(obj) == eolian.Object)
            count += 1
        self.assertGreater(count, 1)


class TestEolianUnit(unittest.TestCase):
    def test_unit_get(self):
        unit = eolian_db.unit_by_file_get('efl_ui_win.eo')
        self.assertIsInstance(unit.state, eolian.Eolian_State)
        self.assertEqual(unit.state, eolian_db)

    def test_file_get(self):
        unit = eolian_db.unit_by_file_get('efl_ui_win.eo')
        self.assertIsInstance(unit, eolian.Eolian_Unit)
        self.assertEqual(unit.file, 'efl_ui_win.eo')

    @unittest.skip('Skipped until unit/state support is fixed')
    def test_children_listing(self):
        l = list(eolian_db.children)
        self.assertGreater(len(l), 500)
        self.assertIsInstance(l[0], eolian.Eolian_Unit)

    def test_file_listing(self):
        l = list(eolian_db.eo_file_paths)
        self.assertGreater(len(l), 400)
        self.assertTrue(l[0].endswith('.eo'))

        l = list(eolian_db.eo_files)
        self.assertGreater(len(l), 400)
        self.assertTrue(l[0].endswith('.eo'))

        l = list(eolian_db.eot_file_paths)
        self.assertGreater(len(l), 10)
        self.assertTrue(l[0].endswith('.eot'))

        l = list(eolian_db.eot_files)
        self.assertGreater(len(l), 10)
        self.assertTrue(l[0].endswith('.eot'))

    def test_object_listing(self):
        unit = eolian_db.unit_by_file_get('efl_ui_win.eo')
        self.assertIsNone(unit.object_by_name_get('Efl.Ui.Frame'))

        obj = unit.object_by_name_get('Efl.Ui.Win')
        self.assertIsInstance(obj, eolian.Object)
        self.assertFalse(type(obj) == eolian.Object)
        self.assertEqual(obj.name, 'Efl.Ui.Win')

        count = 0
        for obj in unit.objects:
            self.assertIsInstance(obj, eolian.Object)
            self.assertFalse(type(obj) == eolian.Object)
            count += 1
        self.assertGreater(count, 5)

    def test_enum_listing(self):
        l = list(eolian_db.enums_by_file_get('efl_ui_win.eo'))
        self.assertGreater(len(l), 5)
        self.assertIsInstance(l[0], eolian.Typedecl)
        self.assertEqual(l[0].type, eolian.Eolian_Typedecl_Type.ENUM)

        all_count = 0
        for enum in eolian_db.enums:
            self.assertIsInstance(enum, eolian.Typedecl)
            self.assertEqual(enum.type, eolian.Eolian_Typedecl_Type.ENUM)
            all_count += 1
        self.assertGreater(all_count, 50)

    def test_struct_listing(self):
        l = list(eolian_db.structs_by_file_get('eina_types.eot'))
        self.assertGreater(len(l), 10)
        self.assertIsInstance(l[0], eolian.Typedecl)
        self.assertIn(l[0].type, (
                        eolian.Eolian_Typedecl_Type.STRUCT,
                        eolian.Eolian_Typedecl_Type.STRUCT_OPAQUE))

        all_count = 0
        for struct in eolian_db.structs:
            self.assertIsInstance(struct, eolian.Typedecl)
            self.assertIn(struct.type, (
                            eolian.Eolian_Typedecl_Type.STRUCT,
                            eolian.Eolian_Typedecl_Type.STRUCT_OPAQUE))
            all_count += 1
        self.assertGreater(all_count, 50)

    def test_alias_listing(self):
        l = list(eolian_db.aliases_by_file_get('edje_types.eot'))
        self.assertGreater(len(l), 5)
        self.assertIsInstance(l[0], eolian.Typedecl)

        all_count = 0
        for alias in eolian_db.aliases:
            self.assertIsInstance(alias, eolian.Typedecl)
            self.assertIn(alias.type, (
                            eolian.Eolian_Typedecl_Type.ALIAS,
                            eolian.Eolian_Typedecl_Type.FUNCTION_POINTER)) # TODO is this correct ??
            all_count += 1
        self.assertGreater(all_count, 10)

    def test_variable_listing(self):
        l = list(eolian_db.constants)
        self.assertGreater(len(l), 2)
        self.assertIsInstance(l[0], eolian.Variable)

        l = list(eolian_db.globals)
        self.assertGreater(len(l), 20)
        self.assertIsInstance(l[0], eolian.Variable)

        l = list(eolian_db.constants_by_file_get('efl_gfx_stack.eo'))
        self.assertGreater(len(l), 1)
        self.assertIsInstance(l[0], eolian.Variable)

        l = list(eolian_db.globals_by_file_get('efl_net_http_types.eot'))
        self.assertGreater(len(l), 10)
        self.assertIsInstance(l[0], eolian.Variable)

    def test_class_listing(self):
        all_count = 0
        for cls in eolian_db.classes:
            self.assertIsInstance(cls, eolian.Class)
            all_count += 1
        self.assertGreater(all_count, 400)


class TestEolianNamespace(unittest.TestCase):
    def test_all_namespace(self):
        count = 0
        for ns in eolian_db.all_namespaces:
            self.assertIsInstance(ns, eolian.Namespace)
            count += 1
        self.assertGreater(count, 100)

    def test_namespace_vs_class_collision(self):
        for ns in eolian_db.all_namespaces:
            cls = eolian_db.class_by_name_get(ns.name)
            self.assertIsNone(cls)

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


class TestEolianObject(unittest.TestCase):
    def test_object_instance(self):
        obj = eolian_db.object_by_name_get('Efl.Ui.Frame')
        self.assertIsInstance(obj, eolian.Class)
        self.assertEqual(obj.name, 'Efl.Ui.Frame')

    def test_unit(self):
        obj = eolian_db.object_by_name_get('Efl.Ui.Frame')
        self.assertIsInstance(obj.unit, eolian.Eolian_Unit)
        self.assertEqual(obj.unit.file, 'efl_ui_frame.eo')

    def test_name(self):
        obj = eolian_db.object_by_name_get('Efl.Ui.Frame')
        self.assertEqual(obj.name, 'Efl.Ui.Frame')

    def test_short_name(self):
        obj = eolian_db.object_by_name_get('Efl.Ui.Frame')
        self.assertEqual(obj.short_name, 'Frame')

    def test_file(self):
        obj = eolian_db.object_by_name_get('Efl.Ui.Frame')
        self.assertEqual(obj.file, 'efl_ui_frame.eo')
        
    def test_line(self):
        obj = eolian_db.object_by_name_get('Efl.Ui.Frame')
        self.assertIsInstance(obj.line, int)
        self.assertGreater(obj.line, 0)

    def test_column(self):
        obj = eolian_db.object_by_name_get('Efl.Ui.Frame')
        self.assertIsInstance(obj.column, int)
        self.assertGreater(obj.column, 0)


class TestEolianClass(unittest.TestCase):
    def test_class(self):
        cls = eolian_db.class_by_file_get('efl_loop_timer.eo')
        self.assertIsInstance(cls, eolian.Class)

        cls = eolian_db.class_by_name_get('Efl.Loop_Timer')
        self.assertIsInstance(cls, eolian.Class)

        self.assertEqual(cls.name, 'Efl.Loop_Timer')
        self.assertEqual(cls.short_name, 'Loop_Timer')
        self.assertEqual(cls.file, 'efl_loop_timer.eo')
        self.assertEqual(list(cls.namespaces), ['Efl'])
        self.assertEqual(cls.type, eolian.Eolian_Class_Type.REGULAR)
        self.assertIsInstance(cls.documentation, eolian.Documentation)
        self.assertEqual(cls.legacy_prefix, 'ecore_timer')
        self.assertIsNone(cls.eo_prefix)  # TODO fin a class with a value
        self.assertIsNone(cls.event_prefix)  # TODO same as above
        self.assertIsNone(cls.data_type)  # TODO same as above
        self.assertEqual(cls.parent.name, 'Efl.Loop_Consumer')
        self.assertEqual([c.name for c in cls.extensions], [])
        self.assertEqual([c.name for c in cls.hierarchy], ['Efl.Loop_Consumer', 'Efl.Object'])
        self.assertFalse(cls.ctor_enable)
        self.assertFalse(cls.dtor_enable)
        self.assertEqual(cls.c_get_function_name, 'efl_loop_timer_class_get')
        self.assertEqual(cls.c_macro, 'EFL_LOOP_TIMER_CLASS')
        self.assertEqual(cls.c_data_type, 'Efl_Loop_Timer_Data')
        self.assertEqual([f.name for f in cls.methods], ['reset','loop_reset','delay'])
        self.assertEqual([f.name for f in cls.properties], ['interval','pending'])
        self.assertGreater(len(list(cls.implements)), 5)
        self.assertIsInstance(list(cls.implements)[0], eolian.Implement)


class TestEolianFunction(unittest.TestCase):
    def test_function(self):
        cls = eolian_db.class_by_name_get('Efl.Loop_Timer')
        f = cls.function_by_name_get('delay')
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
        self.assertEqual(f.class_.name, 'Efl.Loop_Timer')
        self.assertIsInstance(f.implement, eolian.Implement)

    def test_function_parameter(self):
        cls = eolian_db.class_by_name_get('Efl.Loop_Timer')
        f = cls.function_by_name_get('delay')
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
        cls = eolian_db.class_by_name_get('Efl.Loop_Timer')
        f = cls.function_by_name_get('delay')
        im = f.implement
        self.assertIsInstance(im, eolian.Implement)
        self.assertEqual(im.name, 'Efl.Loop_Timer.delay')
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
        cls = eolian_db.class_by_name_get('Efl.Loop_Timer')
        self.assertEqual([e.name for e in cls.events], ['tick'])
        ev = cls.event_by_name_get('tick')
        self.assertIsInstance(ev, eolian.Event)
        self.assertEqual(ev.name, 'tick')
        self.assertEqual(ev.c_name, 'EFL_LOOP_TIMER_EVENT_TIMER_TICK')
        self.assertIsNone(ev.type)  # TODO is this correct
        self.assertIsInstance(ev.documentation, eolian.Documentation)
        self.assertEqual(ev.scope, eolian.Eolian_Object_Scope.PUBLIC)
        self.assertFalse(ev.is_beta)
        self.assertFalse(ev.is_hot)
        self.assertFalse(ev.is_restart)


class TestEolianPart(unittest.TestCase):
    def test_part(self):
        cls = eolian_db.class_by_name_get('Efl.Ui.Popup')
        parts = list(cls.parts)
        self.assertGreater(len(parts), 0)

        part = parts[0]
        self.assertEqual(part.name, 'backwall')
        self.assertIsInstance(part.class_, eolian.Class)
        self.assertEqual(part.class_.name, 'Efl.Ui.Popup_Part_Backwall')
        self.assertIsInstance(part.documentation, eolian.Documentation)


class TestEolianConstructor(unittest.TestCase):
    def test_constructor(self):
        cls = eolian_db.class_by_name_get('Efl.Ui.Win')
        ctors = list(cls.constructors)
        self.assertGreater(len(ctors), 0)
        ctor = ctors[0]
        self.assertIsInstance(ctor, eolian.Constructor)
        self.assertEqual(ctor.name, 'Efl.Ui.Win.win_name')
        self.assertFalse(ctor.is_optional)
        self.assertIsInstance(ctor.class_, eolian.Class)
        self.assertEqual(ctor.class_.name, 'Efl.Ui.Win')
        self.assertIsInstance(ctor.function, eolian.Function)
        self.assertEqual(ctor.function.name, 'win_name')


class TestEolianDocumentation(unittest.TestCase):
    def test_documentation(self):
        td = eolian_db.class_by_name_get('Efl.Net.Control.Manager')
        doc = td.documentation
        self.assertIsInstance(doc, eolian.Documentation)
        self.assertIsInstance(doc.summary, str)
        self.assertGreater(len(doc.summary), 10)
        self.assertIsInstance(doc.description, str)
        self.assertGreater(len(doc.description), 20)
        self.assertEqual(doc.since, '1.19')


class TestEolianVariable(unittest.TestCase):
    def test_variable_global(self):
        var = eolian_db.global_by_name_get('Efl.Net.Http.Error.BAD_CONTENT_ENCODING')
        self.assertIsInstance(var, eolian.Variable)
        self.assertEqual(var.name, 'Efl.Net.Http.Error.BAD_CONTENT_ENCODING')
        self.assertEqual(var.short_name, 'BAD_CONTENT_ENCODING')
        self.assertEqual(var.type, eolian.Eolian_Variable_Type.GLOBAL)
        self.assertEqual(var.file, 'efl_net_http_types.eot')
        self.assertFalse(var.is_extern)
        self.assertEqual(list(var.namespaces), ['Efl','Net','Http','Error'])
        self.assertIsInstance(var.documentation, eolian.Documentation)
        self.assertIsInstance(var.base_type, eolian.Type)
        self.assertIsNone(var.value)  # TODO is None correct here? no value?

    def test_variable_constant(self):
        var = eolian_db.constant_by_name_get('Efl.Gfx.Hint_Fill')
        self.assertIsInstance(var, eolian.Variable)
        self.assertEqual(var.name, 'Efl.Gfx.Hint_Fill')
        self.assertEqual(var.short_name, 'Size_Hint_Fill')
        self.assertEqual(var.type, eolian.Eolian_Variable_Type.CONSTANT)
        self.assertEqual(var.file, 'efl_gfx_hint.eo')
        self.assertFalse(var.is_extern)
        self.assertEqual(list(var.namespaces), ['Efl','Gfx'])
        self.assertIsInstance(var.documentation, eolian.Documentation)
        self.assertIsInstance(var.base_type, eolian.Type)
        self.assertIsInstance(var.value, eolian.Expression)
        self.assertEqual(float(var.value.serialize), +1.0)


class TestEolianTypedecl(unittest.TestCase):
    def test_typedecl_enum(self):
        td = eolian_db.enum_by_name_get('Efl.Net.Http.Version')
        self.assertIsInstance(td, eolian.Typedecl)
        self.assertEqual(td.name, 'Efl.Net.Http.Version')
        self.assertEqual(td.short_name, 'Version')
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
        td = eolian_db.enum_by_name_get('Efl.Net.Http.Version')
        field = td.enum_field_get('v1_0')
        self.assertIsInstance(field, eolian.Enum_Type_Field)
        self.assertEqual(field.name, 'v1_0')
        self.assertEqual(field.c_constant, 'EFL_NET_HTTP_VERSION_V1_0')
        self.assertIsInstance(field.documentation, eolian.Documentation)
        self.assertIsInstance(field.value, eolian.Expression)

    def test_typedecl_struct(self):
        td = eolian_db.struct_by_name_get('Efl.Gfx.Color32')
        self.assertIsInstance(td, eolian.Typedecl)
        self.assertEqual(td.name, 'Efl.Gfx.Color32')
        self.assertEqual(td.short_name, 'Color32')
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
        td = eolian_db.struct_by_name_get('Efl.Gfx.Color32')
        field = td.struct_field_get('b')
        self.assertIsInstance(field, eolian.Struct_Type_Field)
        self.assertEqual(field.name, 'b')
        self.assertIsInstance(field.type, eolian.Type)
        self.assertIsInstance(field.documentation, eolian.Documentation)

    def test_typedecl_alias(self):
        alias = eolian_db.alias_by_name_get('Eina.Error')
        self.assertIsInstance(alias, eolian.Typedecl)
        self.assertEqual(alias.type, eolian.Eolian_Typedecl_Type.ALIAS)
        self.assertEqual(alias.name, 'Eina.Error')
        self.assertEqual(alias.short_name, 'Error')
        self.assertIsInstance(alias.aliased_base, eolian.Type)
        self.assertEqual(alias.aliased_base.name, 'int')
        self.assertEqual(alias.c_type, 'typedef int Eina_Error')


class TestEolianType(unittest.TestCase):
    def test_type_regular_builtin(self):
        cls = eolian_db.class_by_name_get('Efl.Loop_Timer')
        func = cls.function_by_name_get('delay')
        param = list(func.parameters)[0]
        t = param.type  # type: double
        self.assertIsInstance(t, eolian.Type)
        self.assertEqual(t.name, 'double')
        self.assertEqual(t.short_name, 'double')
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
        cls = eolian_db.class_by_name_get('Efl.Gfx.Entity')
        func = cls.function_by_name_get('geometry')
        param = list(func.setter_values)[0]
        t = param.type  # type: Eina.Rect
        self.assertIsInstance(t, eolian.Type)
        self.assertEqual(t.name, 'Eina.Rect')
        self.assertEqual(t.short_name, 'Rect')
        self.assertEqual(t.type, eolian.Eolian_Type_Type.REGULAR)
        self.assertEqual(t.builtin_type, eolian.Eolian_Type_Builtin_Type.INVALID)
        self.assertEqual(t.file, 'efl_gfx_entity.eo')  # TODO is this correct ?
        self.assertEqual(list(t.namespaces), ['Eina'])
        self.assertEqual(t.free_func, 'eina_rectangle_free')
        self.assertIsNone(t.class_)
        self.assertEqual(t, t.aliased_base)

        self.assertEqual(t.c_type_default, 'Eina_Rect')  # TODO find a better test
        self.assertEqual(t.c_type_param, 'Eina_Rect')
        self.assertEqual(t.c_type_return, 'Eina_Rect')

        td = t.typedecl
        self.assertIsInstance(td, eolian.Typedecl)
        self.assertEqual(td.name, 'Eina.Rect')

    def test_type_class(self):
        cls = eolian_db.class_by_name_get('Efl.Content')
        func = cls.function_by_name_get('content')
        param = list(func.setter_values)[0]
        t = param.type  # type: Efl.Gfx (class interface)
        self.assertIsInstance(t, eolian.Type)
        self.assertEqual(t.name, 'Efl.Gfx.Entity')
        self.assertEqual(t.short_name, 'Entity')
        self.assertEqual(t.type, eolian.Eolian_Type_Type.CLASS)
        self.assertEqual(t.builtin_type, eolian.Eolian_Type_Builtin_Type.INVALID)
        self.assertEqual(t.file, 'efl_content.eo')  # TODO is this correct ?
        self.assertEqual(list(t.namespaces), ['Efl', 'Gfx'])
        self.assertEqual(t.free_func, 'efl_del')
        self.assertEqual(t, t.aliased_base)

        self.assertEqual(t.c_type_default, 'Efl_Gfx_Entity *')  # TODO find a better test
        self.assertEqual(t.c_type_param, 'Efl_Gfx_Entity *')
        self.assertEqual(t.c_type_return, 'Efl_Gfx_Entity *')

        cls = t.class_
        self.assertIsInstance(cls, eolian.Class)
        self.assertEqual(cls.name, 'Efl.Gfx.Entity')


class TestEolianExpression(unittest.TestCase):
    def test_expression_simple(self):
        td = eolian_db.enum_by_name_get('Efl.Net.Http.Version')
        field = td.enum_field_get('v1_0')
        exp = field.value
        self.assertIsInstance(exp, eolian.Expression)
        self.assertEqual(exp.type, eolian.Eolian_Expression_Type.INT)
        self.assertEqual(exp.serialize, '100')

    def test_expression_unary(self):
        var = eolian_db.constant_by_name_get('Efl.Gfx.Hint_Fill')
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
    eolian_db = eolian.Eolian_State()
    if not isinstance(eolian_db, eolian.Eolian_State):
        raise(RuntimeError('Eolian, failed to create Eolian state'))

    # eolian system scan (BROKEN)
    #  if not eolian_db.system_directory_add():
        #  raise(RuntimeError('Eolian, failed to scan system directories'))

    # eolian source tree scan
    if not eolian_db.directory_add(SCAN_FOLDER):
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
