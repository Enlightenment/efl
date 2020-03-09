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
using System;
using System.Runtime.InteropServices;
using System.Linq;

using static EinaTestData.BaseData;
using static TestSuite.StructHelpers;

namespace TestSuite
{

internal class TestStructs
{
    // Test cases //

    // Default initialization (C# side)

    private static void simple_default_instantiation()
    {
        var simple = new Dummy.StructSimple();
        checkZeroedStructSimple(simple);
    }

#if EFL_BETA
    private static void complex_default_instantiation()
    {
        var complex = new Dummy.StructComplex();
        checkZeroedStructComplex(complex);
    }
#endif

    public static void parameter_initialization()
    {
        var simple = new Dummy.StructSimple(0x1, 0x2, (char)0x3, 0x4, 0x5);
        Test.AssertEquals(0x1, simple.Fbyte);
        Test.AssertEquals(0x2, simple.Fubyte);
        Test.AssertEquals(0x3, simple.Fchar);
        Test.AssertEquals(0x4, simple.Fshort);
        Test.AssertEquals(0x5, simple.Fushort);
        Test.AssertEquals(0, simple.Fint);
    }

    // As parameters

    public static void simple_in()
    {
        var simple = structSimpleWithValues();
        var t = new Dummy.TestObject();
        bool r = t.StructSimpleIn(simple);
        Test.Assert(r, "Function returned false");
        t.Dispose();
    }

    /*
    public static void simple_ptr_in()
    {
        var simple = structSimpleWithValues();
        int original = simple.Fint;
        simple.Fmstring = "Struct Ptr In";
        var t = new Dummy.TestObject();
        Test.Assert(t.StructSimplePtrIn(ref simple));
        Test.AssertEquals(-original, simple.Fint);
        Test.AssertEquals("nI rtP tcurtS", simple.Fmstring);
    }

    public static void simple_ptr_in_own()
    {
        var simple = structSimpleWithValues();
        int original = simple.Fint;
        simple.Fmstring = "Struct Ptr In Own";
        var t = new Dummy.TestObject();
        Dummy.StructSimple result = t.StructSimplePtrInOwn(ref simple);
        Test.AssertEquals(-original, result.Fint);
        Test.AssertEquals("nwO nI rtP tcurtS", result.Fmstring);
    }
    */

    public static void simple_out()
    {
        var simple = new Dummy.StructSimple();
        var t = new Dummy.TestObject();
        bool r = t.StructSimpleOut(out simple);
        Test.Assert(r, "Function returned false");
        checkStructSimple(simple);
        t.Dispose();
    }

    /*
    public static void simple_ptr_out()
    {
        Dummy.StructSimple simple;
        var t = new Dummy.TestObject();
        Dummy.StructSimple result = t.StructSimplePtrOut(out simple);
        Test.AssertEquals(result.Fint, simple.Fint);
        Test.AssertEquals(result.Fstring, simple.Fstring);
    }

    public static void simple_ptr_out_own()
    {
        Dummy.StructSimple simple;
        var t = new Dummy.TestObject();
        Dummy.StructSimple result = t.StructSimplePtrOutOwn(out simple);
        Test.AssertEquals(result.Fint, simple.Fint);
        Test.AssertEquals(simple.Fstring, "Ptr Out Own");
    }
    */

    public static void simple_return()
    {
        var t = new Dummy.TestObject();
        var simple = t.StructSimpleReturn();
        checkStructSimple(simple);
        t.Dispose();
    }

    /*
    public static void simple_ptr_return()
    {
        var t = new Dummy.TestObject();
        var simple = t.StructSimplePtrReturn();
        Test.AssertEquals(simple.Fstring, "Ret Ptr");
    }

    public static void simple_ptr_return_own()
    {
        var t = new Dummy.TestObject();
        var simple = t.StructSimplePtrReturnOwn();
        Test.AssertEquals(simple.Fstring, "Ret Ptr Own");
    }
    */

    public class StructReturner : Dummy.TestObject
    {
        public Dummy.StructSimple received;
        public bool called;

        public StructReturner() : base(null)
        {
            called = false;
            received = default(Dummy.StructSimple);
        }

        public override bool StructSimpleIn(Dummy.StructSimple simple)
        {
            called = true;
            received = simple;

            return true;
        }

        /*
        public override bool StructSimplePtrIn(ref Dummy.StructSimple simple)
        {
            called = true;
            simple.Fstring = "Virtual Struct Ptr In";
            return true;
        }

        public override Dummy.StructSimple StructSimplePtrInOwn(ref Dummy.StructSimple simple)
        {
            called = true;
            received = simple;
            return received;
        }
        */

        public override bool StructSimpleOut(out Dummy.StructSimple simple) {
            called = true;
            simple = new Dummy.StructSimple(fstring: "Virtual Struct Out");
            return true;
        }

        /*
        public override Dummy.StructSimple StructSimplePtrOut(out Dummy.StructSimple simple) {
            called = true;
            // No way to explicitly define the ownership of the parameter.
            simple = new Dummy.StructSimple();
            simple.Fstring = "Virtual Struct Ptr Out";
            return simple;
        }

        public override Dummy.StructSimple StructSimplePtrOutOwn(out Dummy.StructSimple simple) {
            called = true;
            // No way to explicitly define the ownership of the parameter.
            simple = new Dummy.StructSimple();
            simple.Fstring = "Virtual Struct Ptr Out Own";
            return simple;
        }
        */

        public override Dummy.StructSimple StructSimpleReturn()
        {
            called = true;
            var simple = new Dummy.StructSimple(fstring: "Virtual Struct Return");
            return simple;
        }

        /*
        public override Dummy.StructSimple StructSimplePtrReturn()
        {
            called = true;
            var simple = new Dummy.StructSimple();
            simple.Fstring = "Virtual Struct Ptr Return";
            return simple;
        }

        public override Dummy.StructSimple StructSimplePtrReturnOwn()
        {
            called = true;
            var simple = new Dummy.StructSimple();
            simple.Fstring = "Virtual Struct Ptr Return Own";
            return simple;
        }
        */
    }

    public static void simple_in_virtual()
    {
        StructReturner t = new StructReturner();
        var simple = structSimpleWithValues();

        t.CallStructSimpleIn(simple);
        Test.Assert(t.called);
        Test.AssertEquals(simple.Fstring, t.received.Fstring);
        t.Dispose();
    }

    /*
    public static void simple_ptr_in_virtual()
    {
        StructReturner t = new StructReturner();
        var simple = structSimpleWithValues();
        string reference = "Virtual Struct Ptr In";

        t.CallStructSimplePtrIn(ref simple);
        Test.Assert(t.called);
        Test.AssertEquals(simple.Fstring, reference);
    }

    public static void simple_ptr_in_own_virtual()
    {
        StructReturner t = new StructReturner();
        var simple = structSimpleWithValues();
        simple.Fstring = "Virtual Struct Ptr In Own";

        t.CallStructSimplePtrInOwn(ref simple);
        Test.Assert(t.called);
        Test.AssertEquals(t.received.Fstring, simple.Fstring);
    }
    */

    public static void simple_out_virtual()
    {
        StructReturner t = new StructReturner();
        Dummy.StructSimple simple;
        t.CallStructSimpleOut(out simple);
        Test.Assert(t.called, "override was not called");
        Test.AssertEquals("Virtual Struct Out", simple.Fstring);
        t.Dispose();
    }

    /*
    public static void simple_ptr_out_virtual()
    {
        StructReturner t = new StructReturner();
        Dummy.StructSimple simple;
        t.CallStructSimplePtrOut(out simple);
        Test.Assert(t.called, "override was not called");
        Test.AssertEquals("Virtual Struct Ptr Out", simple.Fstring);
    }

    public static void simple_ptr_out_own_virtual()
    {
        StructReturner t = new StructReturner();
        Dummy.StructSimple simple;
        t.CallStructSimplePtrOutOwn(out simple);
        Test.Assert(t.called, "override was not called");
        Test.AssertEquals("Virtual Struct Ptr Out Own", simple.Fstring);
    }
    */

    public static void simple_return_virtual()
    {
        StructReturner t = new StructReturner();
        Dummy.StructSimple simple = t.CallStructSimpleReturn();
        Test.Assert(t.called, "override was not called");
        Test.AssertEquals("Virtual Struct Return", simple.Fstring);
        t.Dispose();
    }

    /*
    public static void simple_ptr_return_virtual()
    {
        StructReturner t = new StructReturner();
        Dummy.StructSimple simple = t.CallStructSimplePtrReturn();
        Test.Assert(t.called, "override was not called");
        Test.AssertEquals("Virtual Struct Ptr Return", simple.Fstring);
    }

    public static void simple_ptr_return_own_virtual()
    {
        StructReturner t = new StructReturner();
        Dummy.StructSimple simple = t.CallStructSimplePtrReturnOwn();
        Test.Assert(t.called, "override was not called");
        Test.AssertEquals("Virtual Struct Ptr Return Own", simple.Fstring);
    }
    */

#if EFL_BETA
    // Complex Structs
    public static void complex_in()
    {
        var complex = structComplexWithValues();
        var t = new Dummy.TestObject();
        bool r = t.StructComplexIn(complex);
        Test.Assert(r, "Function returned false");
        t.Dispose();
    }

    // public static void complex_ptr_in()
    // {
    // }

    // public static void complex_ptr_in_own()
    // {
    // }

    public static void complex_out()
    {
        var complex = new Dummy.StructComplex();
        var t = new Dummy.TestObject();
        bool r = t.StructComplexOut(out complex);
        Test.Assert(r, "Function returned false");
        checkStructComplex(complex);
        t.Dispose();
    }

    public static void complex_iterator_retrieves_list_correctly()
    {
        var complex = structComplexWithValues();

        var i = 0;
        foreach (var elm in complex.Fiterator) {
            Test.AssertEquals(elm, complex.Flist[i]);
            i++;
        }
    }

    public static void complex_iterator_retrieves_array_correctly()
    {
        var complex = structComplexWithValues();

        var i = 0;
        foreach (var elm in complex.Fiterator) {
            Test.AssertEquals(elm, complex.Farray[i]);
            i++;
        }
    }

    public static void complex_accessor_retrieves_list_correctly()
    {
        var complex = structComplexWithValues();

        var i = 0;
        foreach (var elm in complex.Faccessor) {
            Test.AssertEquals(elm, complex.Flist[i]);
            i++;
        }
    }

    public static void complex_accessor_retrieves_array_correctly()
    {
        var complex = structComplexWithValues();

        var i = 0;
        foreach (var elm in complex.Faccessor) {
            Test.AssertEquals(elm, complex.Farray[i]);
            i++;
        }
    }

    // public static void complex_ptr_out()
    // {
    // }

    // public static void complex_ptr_out_own()
    // {
    // }

    public static void complex_return()
    {
        var t = new Dummy.TestObject();
        var complex = t.StructComplexReturn();
        checkStructComplex(complex);
        t.Dispose();
    }
#endif
    // public static void complex_ptr_return()
    // {
    // }

    // public static void complex_ptr_return_own()
    // {
    // }
}

internal class TestStructEquality
{
    static Dummy.StructSimple a = new Dummy.StructSimple(1, 2, (char)3, 4, fstring: "", fmstring: "", fstringshare: "");
    static Dummy.StructSimple b = new Dummy.StructSimple(1, 2, (char)3, 4, fstring: "", fmstring: "", fstringshare: "");

    static Dummy.StructSimple c = new Dummy.StructSimple(4, 3, (char)2, 1, fstring: "", fmstring: "", fstringshare: "");

    // to check if we differ on a single struct field
    static Dummy.StructSimple singleDifferentField = new Dummy.StructSimple(1, 2, (char)3, 5, fstring: "", fmstring: "", fstringshare: "");

    public static void test_equals()
    {
        Test.AssertEquals(a, b);
        Test.AssertNotEquals(a, c);
        Test.AssertNotEquals(a, singleDifferentField);
    }

    public static void test_equals_different_types()
    {
        Test.Assert(!(a.Equals(new Object())));
    }

    public static void test_equatable()
    {
        Test.Assert(((IEquatable<Dummy.StructSimple>)a).Equals(b));
        Test.Assert(!((IEquatable<Dummy.StructSimple>)a).Equals(c));
        Test.Assert(!((IEquatable<Dummy.StructSimple>)a).Equals(singleDifferentField));
    }

    public static void test_equality_operators()
    {
        Test.Assert(a == b);
        Test.Assert(a != c);
        Test.Assert(a != singleDifferentField);
    }

    public static void test_hash_code()
    {
        Test.AssertEquals(a.GetHashCode(), b.GetHashCode());
        Test.AssertNotEquals(a.GetHashCode(), c.GetHashCode());
        Test.AssertNotEquals(a.GetHashCode(), singleDifferentField.GetHashCode());
    }

#if !MONO
    public static void test_deconstruct() {
        var p = new Eina.Position2D(1, 2);
        var (x, y) = p;

        Test.AssertEquals(x, 1);
        Test.AssertEquals(y, 2);
    }
#endif
}

internal class TestStructTuples
{
    private static Eina.Position2D simulate_position_usage(Eina.Position2D p) {
        return p;
    }

    public static void test_same_type_fields_assign_conversion() {
        Eina.Position2D p = (1, 2);
        Test.AssertEquals(p.X, 1);
        Test.AssertEquals(p.Y, 2);
    }

    public static void test_same_type_fields_call_conversion() {
        var p = simulate_position_usage((1, 2));
        Test.AssertEquals(p.X, 1);
        Test.AssertEquals(p.Y, 2);
    }

    public static void test_different_type_fields_assign_conversion() {
        Efl.Ui.FormatValue v = (1, "Format");
        Test.AssertEquals(v.Value, 1);
        Test.AssertEquals(v.Text, "Format");
    }

#if EFL_BETA
    public static void test_complex_fields_assign_conversion() {
        var pos = new Eina.Position2D(1, 2);
        uint seat = 3;
        var types = new System.String[] {"text", "markup"};

        Efl.Ui.DropEvent attr = (pos, seat, types);
        Test.AssertEquals(attr.Position, pos);
        Test.AssertEquals(attr.Seat, seat);
        Test.AssertEquals(attr.AvailableTypes.ElementAt(0), types[0]);
        Test.AssertEquals(attr.AvailableTypes.ElementAt(1), types[1]);
    }
#endif
}

}
