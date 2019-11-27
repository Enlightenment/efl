#!/usr/bin/env python3
# encoding: utf-8
"""
Pyolian test suite
==================

to run all the tests:
> pytest test_eolian.py

"""
import os
import sys

import pytest

from . import eolian


# Use .eo files from the source tree (not the installed ones)
script_path = os.path.dirname(os.path.realpath(__file__))
root_path = os.path.abspath(os.path.join(script_path, '..', '..', '..'))
SCAN_FOLDER = os.path.join(root_path, 'src', 'lib')


# the main Eolian unit state
eolian_db = None


@pytest.fixture(scope='module')
def eolian_db():
    db = eolian.Eolian_State()
    if not isinstance(db, eolian.Eolian_State):
        raise (RuntimeError('Eolian, failed to create Eolian state'))

    # eolian system scan (BROKEN)
    #  if not eolian_db.system_directory_add():
    #  raise(RuntimeError('Eolian, failed to scan system directories'))

    # eolian source tree scan
    if not db.directory_add(SCAN_FOLDER):
        raise (RuntimeError('Eolian, failed to scan source directory'))

    # Parse all known eo files
    if not db.all_eot_files_parse():
        raise (RuntimeError('Eolian, failed to parse all EOT files'))

    if not db.all_eo_files_parse():
        raise (RuntimeError('Eolian, failed to parse all EO files'))

    # tests are executed here
    yield db

    # cleanup eolian
    del db


class TestEolian(object):
    def test_file_format(self):
        v = eolian.file_format_version
        assert isinstance(v, int)
        assert v >= 1


class TestBaseObject(object):
    def test_base_object_equality(self, eolian_db):
        cls1 = eolian_db.class_by_name_get('Efl.Loop_Timer')
        cls2 = eolian_db.class_by_file_get('efl_loop_timer.eo')
        assert isinstance(cls1, eolian.Class)
        assert isinstance(cls2, eolian.Class)
        assert cls1 == cls2
        assert cls1 == 'Efl.Loop_Timer'
        assert cls2 == 'Efl.Loop_Timer'
        assert cls1 != 'another string'
        assert cls1 != 1234
        assert cls1 is not None
        assert cls1 != 0

        enum1 = eolian_db.enum_by_name_get('Efl.Ui.Focus.Direction')
        enum2 = eolian_db.enum_by_name_get('Efl.Ui.Focus.Direction')
        assert isinstance(enum1, eolian.Typedecl)
        assert isinstance(enum2, eolian.Typedecl)
        assert enum1 == enum2
        assert enum1 == 'Efl.Ui.Focus.Direction'
        assert enum2 == 'Efl.Ui.Focus.Direction'
        assert enum1 != 'another string'
        assert enum1 != 1234
        assert enum1 is not None
        assert enum1 != 0

        assert cls1 != enum1


class TestEolianState(object):
    def test_unit_getters(self, eolian_db):
        count = 0
        for unit in eolian_db.units:
            assert isinstance(unit, eolian.Eolian_Unit)
            assert unit.file.endswith(('.eo', '.eot'))
            count += 1
        assert count > 400

        unit = eolian_db.unit_by_file_get('efl_ui_win.eo')
        assert isinstance(unit, eolian.Eolian_Unit)
        assert unit.file == 'efl_ui_win.eo'

    def test_object_getters(self, eolian_db):
        obj = eolian_db.object_by_name_get('Efl.Ui.Frame')
        assert isinstance(obj, eolian.Object)
        assert type(obj) != eolian.Object
        assert obj.name == 'Efl.Ui.Frame'

        count = 0
        for obj in eolian_db.objects:
            assert isinstance(obj, eolian.Object)
            assert type(obj) != eolian.Object
            count += 1
        assert count > 800

        count = 0
        for obj in eolian_db.objects_by_file_get('efl_loop.eo'):
            assert isinstance(obj, eolian.Object)
            assert type(obj) != eolian.Object
            count += 1
        assert count > 1

    @pytest.mark.skip(reason='Should this return True sooner or later?')
    def test_integrity(self, eolian_db):
        assert eolian_db.state_check() is True


class TestEolianUnit(object):
    def test_unit_get(self, eolian_db):
        unit = eolian_db.unit_by_file_get('efl_ui_win.eo')
        assert isinstance(unit.state, eolian.Eolian_State)
        assert unit.state == eolian_db
        assert isinstance(unit.version, int)
        assert unit.version >= 1

    def test_file_get(self, eolian_db):
        unit = eolian_db.unit_by_file_get('efl_ui_win.eo')
        assert isinstance(unit, eolian.Eolian_Unit)
        assert unit.file == 'efl_ui_win.eo'
        assert unit.file_path.endswith('efl_ui_win.eo')  # full path can change

    @pytest.mark.skip(reason='Skipped until unit/state support is fixed')
    def test_children_listing(self, eolian_db):
        li = list(eolian_db.children)
        assert len(li) > 500
        assert isinstance(li[0], eolian.Eolian_Unit)

    def test_file_listing(self, eolian_db):
        li = list(eolian_db.eo_file_paths)
        assert len(li) > 400
        assert li[0].endswith('.eo')

        li = list(eolian_db.eo_files)
        assert len(li) > 400
        assert li[0].endswith('.eo')

        li = list(eolian_db.eot_file_paths)
        assert len(li) > 10
        assert li[0].endswith('.eot')

        li = list(eolian_db.eot_files)
        assert len(li) > 10
        assert li[0].endswith('.eot')

    def test_object_listing(self, eolian_db):
        unit = eolian_db.unit_by_file_get('efl_ui_win.eo')
        assert unit.object_by_name_get('Efl.Ui.Frame') is None

        obj = unit.object_by_name_get('Efl.Ui.Win')
        assert isinstance(obj, eolian.Object)
        assert type(obj) != eolian.Object
        assert obj.name == 'Efl.Ui.Win'

        count = 0
        for obj in unit.objects:
            assert isinstance(obj, eolian.Object)
            assert type(obj) != eolian.Object
            count += 1
        assert count > 5

    def test_enum_listing(self, eolian_db):
        li = list(eolian_db.enums_by_file_get('efl_ui_win.eo'))
        assert len(li) > 5
        assert isinstance(li[0], eolian.Typedecl)
        assert li[0].type == eolian.Eolian_Typedecl_Type.ENUM

        all_count = 0
        for enum in eolian_db.enums:
            assert isinstance(enum, eolian.Typedecl)
            assert enum.type == eolian.Eolian_Typedecl_Type.ENUM
            all_count += 1
        assert all_count > 50

    def test_struct_listing(self, eolian_db):
        li = list(eolian_db.structs_by_file_get('eina_types.eot'))
        assert len(li) > 10
        assert isinstance(li[0], eolian.Typedecl)
        assert li[0].type in (eolian.Eolian_Typedecl_Type.STRUCT,
                              eolian.Eolian_Typedecl_Type.STRUCT_OPAQUE)

        all_count = 0
        for struct in eolian_db.structs:
            assert isinstance(struct, eolian.Typedecl)
            assert struct.type in (eolian.Eolian_Typedecl_Type.STRUCT,
                                   eolian.Eolian_Typedecl_Type.STRUCT_OPAQUE)
            all_count += 1
        assert all_count > 50

    def test_alias_listing(self, eolian_db):
        li = list(eolian_db.aliases_by_file_get('eina_types.eot'))
        assert len(li) > 2
        assert isinstance(li[0], eolian.Typedecl)

        all_count = 0
        for alias in eolian_db.aliases:
            assert isinstance(alias, eolian.Typedecl)
            assert alias.type in (eolian.Eolian_Typedecl_Type.ALIAS,
                                  eolian.Eolian_Typedecl_Type.FUNCTION_POINTER)  # TODO is this correct ??
            all_count += 1
        assert all_count > 10

    def test_constant_listing(self, eolian_db):
        li = list(eolian_db.constants)
        assert len(li) > 2
        assert isinstance(li[0], eolian.Constant)

        li = list(eolian_db.constants_by_file_get('efl_gfx_stack.eo'))
        assert len(li) > 1
        assert isinstance(li[0], eolian.Constant)

    def test_class_listing(self, eolian_db):
        all_count = 0
        for cls in eolian_db.classes:
            assert isinstance(cls, eolian.Class)
            all_count += 1
        assert all_count > 400


class TestEolianNamespace(object):
    def test_all_namespace(self, eolian_db):
        count = 0
        for ns in eolian_db.all_namespaces:
            assert isinstance(ns, eolian.Namespace)
            count += 1
        assert count > 50

    def test_namespace_vs_class_collision(self, eolian_db):
        colliding_classes = ['Ecore.Audio',
                             'Ecore.Audio.In',
                             'Ecore.Audio.Out',
                             'Ecore.Event.Message',
                             'Ector.Buffer',
                             'Ector.Renderer',
                             'Ector.Renderer.Cairo',
                             'Ector.Renderer.GL',
                             'Ector.Renderer.Gradient',
                             'Ector.Renderer.Software',
                             'Ector.Software.Buffer',
                             'Eio.Sentry',
                             'Eldbus.Model',
                             ]
        for ns in eolian_db.all_namespaces:
            cls = eolian_db.class_by_name_get(ns.name)
            # Some legacy classes are parsed and still make this fail.
            if cls:
                assert cls.name in colliding_classes
            else:
                assert cls is None

    def test_namespace_equality(self, eolian_db):
        ns1 = eolian.Namespace(eolian_db, 'Efl.Io')
        ns2 = eolian.Namespace(eolian_db, 'Efl.Net')
        assert isinstance(ns1, eolian.Namespace)
        assert isinstance(ns2, eolian.Namespace)
        assert ns1 != ns2
        assert ns1 == eolian.Namespace(eolian_db, 'Efl.Io')
        assert ns2 == eolian.Namespace(eolian_db, 'Efl.Net')

    def test_namespace_sorting(self, eolian_db):
        nspaces = eolian_db.all_namespaces
        nspaces.sort(reverse=True)
        assert nspaces[0] > nspaces[-1]
        assert nspaces[1] < nspaces[0]

    def test_namespace_by_name(self, eolian_db):
        ns = eolian.Namespace(eolian_db, 'Efl.Net')
        assert isinstance(ns, eolian.Namespace)
        assert ns.name == 'Efl.Net'
        assert ns.namespaces == ['Efl', 'Net']

        ns = eolian_db.namespace_get_by_name('Efl')
        assert isinstance(ns, eolian.Namespace)
        assert ns.name == 'Efl'

        assert len(ns.classes) > 30
        for cls in ns.classes:
            assert isinstance(cls, eolian.Class)
        assert len(ns.regulars) > 4
        for cls in ns.regulars:
            assert isinstance(cls, eolian.Class)
            assert cls.type == eolian.Eolian_Class_Type.REGULAR
        assert len(ns.mixins) > 0
        for cls in ns.mixins:
            assert isinstance(cls, eolian.Class)
            assert cls.type == eolian.Eolian_Class_Type.MIXIN
        assert len(ns.interfaces) > 15
        for cls in ns.interfaces:
            assert isinstance(cls, eolian.Class)
            assert cls.type == eolian.Eolian_Class_Type.INTERFACE

        assert len(ns.enums) > 1
        for td in ns.enums:
            assert isinstance(td, eolian.Typedecl)
            assert td.type == eolian.Eolian_Typedecl_Type.ENUM
        assert len(ns.aliases) > 0
        for td in ns.aliases:
            assert isinstance(td, eolian.Typedecl)
            # TODO eolian_typedecl_all_aliases_get also return FUNCTION_POINTER
            # is this correct? or an eolian bug ?
            #  assert Equal(td.type, eolian.Eolian_Typedecl_Type.ALIAS)
        assert len(ns.structs) > 2
        for td in ns.structs:
            assert isinstance(td, eolian.Typedecl)
            assert td.type == eolian.Eolian_Typedecl_Type.STRUCT


class TestEolianObject(object):
    def test_object_instance(self, eolian_db):
        obj = eolian_db.object_by_name_get('Efl.Ui.Frame')
        assert isinstance(obj, eolian.Class)
        assert obj.name == 'Efl.Ui.Frame'

    def test_unit(self, eolian_db):
        obj = eolian_db.object_by_name_get('Efl.Ui.Frame')
        assert isinstance(obj.unit, eolian.Eolian_Unit)
        assert obj.unit.file == 'efl_ui_frame.eo'

    def test_name(self, eolian_db):
        obj = eolian_db.object_by_name_get('Efl.Ui.Frame')
        assert obj.name == 'Efl.Ui.Frame'
        assert obj.c_name == 'Efl_Ui_Frame'

    def test_short_name(self, eolian_db):
        obj = eolian_db.object_by_name_get('Efl.Ui.Frame')
        assert obj.short_name == 'Frame'

    def test_file(self, eolian_db):
        obj = eolian_db.object_by_name_get('Efl.Ui.Frame')
        assert obj.file == 'efl_ui_frame.eo'

    def test_line(self, eolian_db):
        obj = eolian_db.object_by_name_get('Efl.Ui.Frame')
        assert isinstance(obj.line, int)
        assert obj.line > 0

    def test_column(self, eolian_db):
        obj = eolian_db.object_by_name_get('Efl.Ui.Frame')
        assert isinstance(obj.column, int)
        assert obj.column > 0


class TestEolianClass(object):
    def test_class(self, eolian_db):
        cls = eolian_db.class_by_file_get('efl_loop_timer.eo')
        assert isinstance(cls, eolian.Class)

        cls = eolian_db.class_by_name_get('Efl.Loop_Timer')
        assert isinstance(cls, eolian.Class)

        assert cls.name == 'Efl.Loop_Timer'
        assert cls.short_name == 'Loop_Timer'
        assert cls.file == 'efl_loop_timer.eo'
        assert list(cls.namespaces) == ['Efl']
        assert cls.type == eolian.Eolian_Class_Type.REGULAR
        assert isinstance(cls.documentation, eolian.Documentation)
        assert cls.c_prefix is None  # TODO fin a class with a value
        assert cls.event_c_prefix is None  # TODO same as above
        assert cls.data_type is None  # TODO same as above
        assert cls.parent.name == 'Efl.Loop_Consumer'
        assert [c.name for c in cls.extensions] == []
        assert [c.name for c in cls.hierarchy] == ['Efl.Loop_Consumer', 'Efl.Object']
        assert cls.ctor_enable is False
        assert cls.dtor_enable is False
        assert cls.c_get_function_name == 'efl_loop_timer_class_get'
        assert cls.c_macro == 'EFL_LOOP_TIMER_CLASS'
        assert cls.c_data_type == 'Efl_Loop_Timer_Data'
        assert [f.name for f in cls.methods] == ['timer_reset', 'timer_loop_reset', 'timer_delay']
        assert [f.name for f in cls.properties] == ['timer_interval', 'time_pending']
        assert len(list(cls.implements)) > 5
        assert isinstance(list(cls.implements)[0], eolian.Implement)

    def test_mixins_requires(self, eolian_db):
        cls = eolian_db.class_by_name_get('Efl.Loop_Timer')
        assert len(list(cls.requires)) == 0
        cls = eolian_db.class_by_name_get('Efl.File')
        assert len(list(cls.requires)) == 1
        assert list(cls.requires)[0].name == 'Efl.Object'


class TestEolianFunction(object):
    def test_function(self, eolian_db):
        cls = eolian_db.class_by_name_get('Efl.Loop_Timer')
        f = cls.function_by_name_get('timer_delay')
        assert isinstance(f, eolian.Function)
        assert f.name == 'timer_delay'
        assert f.type == eolian.Eolian_Function_Type.METHOD
        assert f.method_scope == eolian.Eolian_Object_Scope.PUBLIC
        assert f.getter_scope == eolian.Eolian_Object_Scope.UNKNOWN  # TODO correct ?
        assert f.setter_scope == eolian.Eolian_Object_Scope.UNKNOWN  # TODO correct ?
        assert f.full_c_method_name == 'efl_loop_timer_delay'
        assert f.full_c_getter_name == 'efl_loop_timer_delay_get'
        assert f.full_c_setter_name == 'efl_loop_timer_delay_set'
        assert f.method_return_type is None  # TODO correct ?
        assert f.setter_return_type is None  # TODO correct ?
        assert f.getter_return_type is None  # TODO correct ?
        assert f.is_static is False
        assert f.is_beta is False
        assert f.is_constructor(cls) is False
        #  # #assert f.is_function_pointer == False  # TODO broken somehow
        assert len(list(f.getter_values)) == 1
        assert len(list(f.getter_values)) == 1
        assert len(list(f.parameters)) == 1
        assert f.return_allow_unused(eolian.Eolian_Function_Type.PROP_GET) is True
        assert f.return_is_by_ref(eolian.Eolian_Function_Type.PROP_GET) is False
        assert f.return_is_move(eolian.Eolian_Function_Type.PROP_GET) is False
        assert f.object_is_const is False
        assert f.class_.name == 'Efl.Loop_Timer'
        assert isinstance(f.implement, eolian.Implement)

    def test_function_parameter(self, eolian_db):
        cls = eolian_db.class_by_name_get('Efl.Loop_Timer')
        f = cls.function_by_name_get('timer_delay')
        p = list(f.parameters)[0]
        assert p.direction == eolian.Eolian_Parameter_Direction.IN
        assert p.name == 'add'
        assert p.default_value is None
        assert p.is_optional is False
        assert p.is_by_ref is False
        assert p.is_move is False
        assert p.type.name == 'double'
        assert p.c_type_get(False) == 'double'
        assert p.c_type_get(True) == 'double'
        assert isinstance(p.documentation, eolian.Documentation)


class TestEolianImplement(object):
    def test_implement(self, eolian_db):
        cls = eolian_db.class_by_name_get('Efl.Loop_Timer')
        f = cls.function_by_name_get('timer_delay')
        im = f.implement
        assert isinstance(im, eolian.Implement)
        assert im.name == 'Efl.Loop_Timer.timer_delay'
        assert isinstance(im.class_, eolian.Class)
        assert isinstance(im.function, eolian.Function)
        assert isinstance(im.implementing_class, eolian.Class)
        assert im.implementing_class.name == 'Efl.Loop_Timer'
        assert isinstance(im.documentation_get(), eolian.Documentation)  # TODO is UNRESOLVED correct ?
        assert im.is_auto() is False
        assert im.is_empty() is False
        assert im.is_pure_virtual() is False
        assert im.is_prop_set is False
        assert im.is_prop_get is False
        assert im.is_property is False
        assert im.is_method is True
        assert im.parent is None

    def test_implement_parent(self, eolian_db):
        cls = eolian_db.class_by_name_get('Efl.Ui.Button')
        im = [im for im in cls.implements if im.short_name == 'content_unset'][0]
        assert isinstance(im, eolian.Implement)
        assert im.name == 'Efl.Content.content_unset'  # TODO is this right??
        assert im.parent.name == 'Efl.Content.content_unset'


class TestEolianEvent(object):
    def test_event(self, eolian_db):
        cls = eolian_db.class_by_name_get('Efl.Loop_Timer')
        assert [e.name for e in cls.events] == ['timer,tick']
        ev = cls.event_by_name_get('timer,tick')
        assert isinstance(ev, eolian.Event)
        assert ev.name == 'timer,tick'
        assert ev.c_macro == 'EFL_LOOP_TIMER_EVENT_TIMER_TICK'
        assert ev.type is None  # TODO is this correct
        assert isinstance(ev.documentation, eolian.Documentation)
        assert ev.scope == eolian.Eolian_Object_Scope.PUBLIC
        assert ev.is_beta is False
        assert ev.is_hot is False
        assert ev.is_restart is False
        assert ev.class_.name == cls.name


class TestEolianPart(object):
    def test_part(self, eolian_db):
        cls = eolian_db.class_by_name_get('Efl.Ui.Popup')
        parts = list(cls.parts)
        assert len(parts) > 0

        part = parts[0]
        assert part.name == 'backwall'
        assert isinstance(part.class_, eolian.Class)
        assert part.class_.name == 'Efl.Ui.Popup_Part_Backwall'
        assert isinstance(part.documentation, eolian.Documentation)


class TestEolianConstructor(object):
    def test_constructor(self, eolian_db):
        cls = eolian_db.class_by_name_get('Efl.Ui.Win')
        ctors = list(cls.constructors)
        assert len(ctors) > 0
        ctor = ctors[0]
        assert isinstance(ctor, eolian.Constructor)
        assert ctor.name == 'Efl.Ui.Win.win_name'
        assert ctor.is_optional is True
        assert isinstance(ctor.class_, eolian.Class)
        assert ctor.class_.name == 'Efl.Ui.Win'
        assert isinstance(ctor.function, eolian.Function)
        assert ctor.function.name == 'win_name'


class TestEolianDocumentation(object):
    def test_documentation(self, eolian_db):
        td = eolian_db.class_by_name_get('Efl.Object')
        doc = td.documentation
        assert isinstance(doc, eolian.Documentation)
        assert isinstance(doc.summary, str)
        assert len(doc.summary) > 10
        assert isinstance(doc.description, str)
        assert len(doc.description) > 20
        assert doc.since == '1.22'


class TestEolianConstant(object):
    def test_constant(self, eolian_db):
        var = eolian_db.constant_by_name_get('Efl.Gfx.Hint_Expand')
        assert isinstance(var, eolian.Constant)
        assert var.name == 'Efl.Gfx.Hint_Expand'
        assert var.short_name == 'Hint_Expand'
        assert var.file == 'efl_gfx_hint.eo'
        assert var.is_extern is False
        assert list(var.namespaces) == ['Efl', 'Gfx']
        assert isinstance(var.documentation, eolian.Documentation)
        assert isinstance(var.type, eolian.Type)
        assert isinstance(var.value, eolian.Expression)
        assert float(var.value.serialize) == +1.0


class TestEolianTypedecl(object):
    def test_typedecl_enum(self, eolian_db):
        td = eolian_db.enum_by_name_get('Efl.Net.Http.Version')
        assert isinstance(td, eolian.Typedecl)
        assert td.name == 'Efl.Net.Http.Version'
        assert td.short_name == 'Version'
        assert td.file == 'efl_net_http_types.eot'
        assert list(td.namespaces) == ['Efl', 'Net', 'Http']
        assert isinstance(td.documentation, eolian.Documentation)
        assert td.base_type is None  # TODO find a better test
        assert td.free_func is None  # TODO find a better test
        assert td.function_pointer is None  # TODO find a better test
        assert td.is_extern is False
        assert len(list(td.enum_fields)) == 3
        assert td.c_type == 'enum Efl_Net_Http_Version { v1_0 = 100, v1_1 = 101, v2_0 = 200 }'

    def test_typedecl_enum_field(self, eolian_db):
        td = eolian_db.enum_by_name_get('Efl.Net.Http.Version')
        field = td.enum_field_get('v1_0')
        assert isinstance(field, eolian.Enum_Type_Field)
        assert field.name == 'v1_0'
        assert field.c_constant == 'EFL_NET_HTTP_VERSION_V1_0'
        assert isinstance(field.documentation, eolian.Documentation)
        assert isinstance(field.value, eolian.Expression)

    def test_typedecl_struct(self, eolian_db):
        td = eolian_db.struct_by_name_get('Efl.Gfx.Color32')
        assert isinstance(td, eolian.Typedecl)
        assert td.name == 'Efl.Gfx.Color32'
        assert td.short_name == 'Color32'
        assert td.file == 'efl_canvas_filter_internal.eo'
        assert list(td.namespaces) == ['Efl', 'Gfx']
        assert isinstance(td.documentation, eolian.Documentation)
        assert td.base_type is None  # TODO find a better test
        assert td.free_func is None  # TODO find a better test
        assert td.function_pointer is None  # TODO find a better test
        assert td.is_extern is False
        assert len(list(td.struct_fields)) == 4
        assert td.c_type == 'struct Efl_Gfx_Color32 { uint8_t r; uint8_t g; uint8_t b; uint8_t a; }'

    def test_typedecl_struct_field(self, eolian_db):
        td = eolian_db.struct_by_name_get('Efl.Gfx.Color32')
        field = td.struct_field_get('b')
        assert isinstance(field, eolian.Struct_Type_Field)
        assert field.name == 'b'
        assert isinstance(field.type, eolian.Type)
        assert isinstance(field.documentation, eolian.Documentation)
        assert field.c_type == "uint8_t"
        assert field.is_by_ref is False
        assert field.is_move is False

    def test_typedecl_alias(self, eolian_db):
        alias = eolian_db.alias_by_name_get('Eina.Error')
        assert isinstance(alias, eolian.Typedecl)
        assert alias.type == eolian.Eolian_Typedecl_Type.ALIAS
        assert alias.name == 'Eina.Error'
        assert alias.short_name == 'Error'
        assert isinstance(alias.aliased_base, eolian.Type)
        assert alias.aliased_base.name == 'int'
        assert alias.c_type == 'typedef int Eina_Error'


class TestEolianType(object):
    def test_type_regular_builtin(self, eolian_db):
        cls = eolian_db.class_by_name_get('Efl.Loop_Timer')
        func = cls.function_by_name_get('timer_delay')
        param = list(func.parameters)[0]
        t = param.type  # type double
        assert isinstance(t, eolian.Type)
        assert t.name == 'double'
        assert t.short_name == 'double'
        assert t.type == eolian.Eolian_Type_Type.REGULAR
        assert t.builtin_type == eolian.Eolian_Type_Builtin_Type.DOUBLE
        assert t.file == 'efl_loop_timer.eo'  # TODO is this correct ?
        assert t.base_type is None  # TODO find a better test
        assert t.next_type is None  # TODO find a better test
        assert t.is_const is False
        assert t.is_ptr is False
        assert t.is_move is False
        assert list(t.namespaces) == []
        assert t.class_ is None
        assert t == t.aliased_base  # TODO find a better test

    def test_type_regular(self, eolian_db):
        cls = eolian_db.class_by_name_get('Efl.Gfx.Entity')
        func = cls.function_by_name_get('geometry')
        param = list(func.setter_values)[0]
        t = param.type  # type Eina.Rect
        assert isinstance(t, eolian.Type)
        assert t.name == 'Eina.Rect'
        assert t.short_name == 'Rect'
        assert t.type == eolian.Eolian_Type_Type.REGULAR
        assert t.builtin_type == eolian.Eolian_Type_Builtin_Type.INVALID
        assert t.file == 'efl_gfx_entity.eo'  # TODO is this correct ?
        assert t.is_const is False
        assert t.is_ptr is False
        assert t.is_move is False
        assert list(t.namespaces) == ['Eina']
        assert t.class_ is None
        assert t == t.aliased_base

        td = t.typedecl
        assert isinstance(td, eolian.Typedecl)
        assert td.name == 'Eina.Rect'

    def test_type_class(self, eolian_db):
        cls = eolian_db.class_by_name_get('Efl.Content')
        func = cls.function_by_name_get('content')
        param = list(func.setter_values)[0]
        t = param.type  # type Efl.Gfx (class interface)
        assert isinstance(t, eolian.Type)
        assert t.name == 'Efl.Gfx.Entity'
        assert t.short_name == 'Entity'
        assert t.type == eolian.Eolian_Type_Type.CLASS
        assert t.builtin_type == eolian.Eolian_Type_Builtin_Type.INVALID
        assert t.file == 'efl_content.eo'  # TODO is this correct ?
        assert t.is_const is False
        assert t.is_ptr is False
        assert t.is_move is False
        assert list(t.namespaces) == ['Efl', 'Gfx']
        assert t == t.aliased_base

        cls = t.class_
        assert isinstance(cls, eolian.Class)
        assert cls.name == 'Efl.Gfx.Entity'


class TestEolianExpression(object):
    def test_expression_simple(self, eolian_db):
        td = eolian_db.enum_by_name_get('Efl.Net.Http.Version')
        field = td.enum_field_get('v1_0')
        exp = field.value
        assert isinstance(exp, eolian.Expression)
        assert exp.type == eolian.Eolian_Expression_Type.INT
        assert exp.serialize == '100'

    def test_expression_unary(self, eolian_db):
        var = eolian_db.constant_by_name_get('Efl.Gfx.Stack_Layer_Min')
        exp = var.value
        assert isinstance(exp, eolian.Expression)
        assert exp.type == eolian.Eolian_Expression_Type.UNARY
        assert float(exp.serialize) == 32768.0  # TODO is this a bug? isn't -1.0 =
        assert exp.unary_operator == eolian.Eolian_Unary_Operator.UNM  # Invalid -> NO
        unary = exp.unary_expression
        assert isinstance(unary, eolian.Expression)
        assert unary.type == eolian.Eolian_Expression_Type.INT
        assert float(exp.serialize) == 32768.0  # Bug too ?

        # TODO test_expression_binary
        #  exp.binary_operator # TODO find a better test (only works for BINARY expr)
        #  exp.binary_lhs # TODO find a better test (only works for BINARY expr)
        #  exp.binary_rhs # TODO find a better test (only works for BINARY expr)

class TestEolianInherits(object):
    def test_inherits_full(self, eolian_db):
        cls = eolian_db.class_by_name_get('Efl.Ui.Widget')
        assert 'Efl.Object' in cls.inherits_full

    def test_extensions_hierarchy(self, eolian_db):
        cls = eolian_db.class_by_name_get('Efl.Ui.Widget')

        # inherited extension
        assert any(x.name == 'Efl.Gfx.Stack' for x in cls.extensions_hierarchy)
        # direct extension
        assert any(x.name == 'Efl.Access.Object' for x in cls.extensions_hierarchy)
