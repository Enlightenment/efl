/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define CODE_ANALYSIS

#pragma warning disable 1591

using System;
using System.Diagnostics.CodeAnalysis;
using Efl.Ui;

namespace TestSuite {

#if EFL_BETA

[SuppressMessage("Gendarme.Rules.Portability", "DoNotHardcodePathsRule")]
public static class TestParts
{
    public static void basic_part_test()
    {
        var t = new Dummy.PartHolder();
        do_part_test(t);
        t.Dispose();
    }

    private class Child : Dummy.PartHolder
    {
        public Child() : base(null) {}
    }

    public static void inherited_part_test() {
        var t = new Child();
        do_part_test(t);
        t.Dispose();
    }

    private static void do_part_test(Dummy.PartHolder t)
    {
        var p1 = t.OnePart;
        var p2 = t.TwoPart;
        Test.Assert(p1 is Dummy.TestObject);
        Test.AssertEquals("part_one", p1.Name);
        Test.Assert(p2 is Dummy.TestObject);
        Test.AssertEquals("part_two", p2.Name);
    }
}

public static class TestMVVMParts
{
    public static void mvvm_dynamic_parts()
    {
        var parent = new Efl.Ui.Win(null, "", "");
        parent.Visible = false;
        var factory = new Efl.Ui.ItemFactory<Efl.Ui.ListDefaultItem>(parent);

        var bindablePart = factory.TextPart();
        var error = bindablePart.Markup().Bind("name");

        Test.AssertEquals(error, Eina.Error.NO_ERROR);
        factory.Dispose();
        parent.Dispose();
    }

    public static void mvvm_factory_properties()
    {
        var parent = new Efl.Ui.Win(null, "", "");
        parent.Visible = false;
        var factory = new Efl.Ui.ItemFactory<Efl.Ui.ListDefaultItem>(parent);
        var iconFactory = new Efl.Ui.ImageFactory(null);
        iconFactory.BindProperty("filename", "modelProperty");
        var error = factory.IconPart().BindFactory(iconFactory);

        Test.AssertEquals(error, Eina.Error.NO_ERROR);
        iconFactory.Dispose();
        factory.Dispose();
        parent.Dispose();
    }
}

public static class TestNamedParts
{
    public static void named_parts()
    {
        var obj = new Dummy.PartHolder();
        var p1 = obj.GetPart("one");
        var p2 = obj.GetPart("two");
        Test.Assert(p1 is Dummy.TestObject);
        Test.AssertEquals("part_one", p1.Name);
        Test.Assert(p2 is Dummy.TestObject);
        Test.AssertEquals("part_two", p2.Name);
        obj.Dispose();
    }
}

#endif

}
