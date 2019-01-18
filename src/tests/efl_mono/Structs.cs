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

    private static void complex_default_instantiation()
    {
        var complex = new Dummy.StructComplex();
        checkZeroedStructComplex(complex);
    }

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
    }

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

    public static void simple_out()
    {
        var simple = new Dummy.StructSimple();
        var t = new Dummy.TestObject();
        bool r = t.StructSimpleOut(out simple);
        Test.Assert(r, "Function returned false");
        checkStructSimple(simple);
    }

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

    public static void simple_return()
    {
        var t = new Dummy.TestObject();
        var simple = t.StructSimpleReturn();
        checkStructSimple(simple);
    }

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

        public override bool StructSimpleOut(out Dummy.StructSimple simple) {
            called = true;
            simple = new Dummy.StructSimple();
            simple.Fstring = "Virtual Struct Out";
            return true;
        }

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

        public override Dummy.StructSimple StructSimpleReturn()
        {
            called = true;
            var simple = new Dummy.StructSimple();
            simple.Fstring = "Virtual Struct Return";
            return simple;
        }

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
    }

    public static void simple_in_virtual()
    {
        StructReturner t = new StructReturner();
        var simple = structSimpleWithValues();
        simple.Fstring = "Virtual Struct In";

        t.CallStructSimpleIn(simple);
        Test.Assert(t.called);
        Test.AssertEquals(simple.Fstring, t.received.Fstring);
    }

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

    public static void simple_out_virtual()
    {
        StructReturner t = new StructReturner();
        Dummy.StructSimple simple;
        t.CallStructSimpleOut(out simple);
        Test.Assert(t.called, "override was not called");
        Test.AssertEquals("Virtual Struct Out", simple.Fstring);
    }

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

    public static void simple_return_virtual()
    {
        StructReturner t = new StructReturner();
        Dummy.StructSimple simple = t.CallStructSimpleReturn();
        Test.Assert(t.called, "override was not called");
        Test.AssertEquals("Virtual Struct Return", simple.Fstring);
    }

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

    // Complex Structs
    public static void complex_in()
    {
        var complex = structComplexWithValues();
        var t = new Dummy.TestObject();
        bool r = t.StructComplexIn(complex);
        Test.Assert(r, "Function returned false");
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
    }

    // public static void complex_ptr_return()
    // {
    // }

    // public static void complex_ptr_return_own()
    // {
    // }
}

}
