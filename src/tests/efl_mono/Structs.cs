using System;
using System.Runtime.InteropServices;
using System.Linq;

using static EinaTestData.BaseData;

namespace TestSuite
{

class TestStructs
{
    // Auxiliary function //

    private static Dummy.StructSimple structSimpleWithValues()
    {
        var simple = new Dummy.StructSimple();

        simple.Fbyte = (sbyte)-126;
        simple.Fubyte = (byte) 254u;
        simple.Fchar = '~';
        simple.Fshort = (short) -32766;
        simple.Fushort = (ushort) 65534u;
        simple.Fint = -32766;
        simple.Fuint = 65534u;
        simple.Flong = -2147483646;
        simple.Fulong = 4294967294u;
        simple.Fllong = -9223372036854775806;
        simple.Fullong = 18446744073709551614u;
        simple.Fint8 = (sbyte) -126;
        simple.Fuint8 = (byte) 254u;
        simple.Fint16 = (short) -32766;
        simple.Fuint16 = (ushort) 65534u;
        simple.Fint32 = -2147483646;
        simple.Fuint32 = 4294967294u;
        simple.Fint64 = -9223372036854775806;
        simple.Fuint64 = 18446744073709551614u;
        simple.Fssize = -2147483646;
        simple.Fsize = 4294967294u;
        simple.Fintptr = (IntPtr) 0xFE;
        simple.Fptrdiff = -2147483646;
        simple.Ffloat = -16777216.0f;
        simple.Fdouble = -9007199254740992.0;
        simple.Fbool = true;
        simple.Fvoid_ptr = (IntPtr) 0xFE;
        simple.Fenum = Dummy.SampleEnum.V2;
        simple.Fstring = "test/string";
        simple.Fmstring = "test/mstring";
        simple.Fstringshare = "test/stringshare";

        return simple;
    }

    private static void checkStructSimple(Dummy.StructSimple simple)
    {
        Test.Assert(simple.Fbyte == (sbyte) -126);
        Test.Assert(simple.Fubyte == (byte) 254u);
        Test.Assert(simple.Fchar == '~');
        Test.Assert(simple.Fshort == (short) -32766);
        Test.Assert(simple.Fushort == (ushort) 65534u);
        Test.Assert(simple.Fint == -32766);
        Test.Assert(simple.Fuint == 65534u);
        Test.Assert(simple.Flong == -2147483646);
        Test.Assert(simple.Fulong == 4294967294u);
        Test.Assert(simple.Fllong == -9223372036854775806);
        Test.Assert(simple.Fullong == 18446744073709551614u);
        Test.Assert(simple.Fint8 == (sbyte) -126);
        Test.Assert(simple.Fuint8 == (byte) 254u);
        Test.Assert(simple.Fint16 == (short) -32766);
        Test.Assert(simple.Fuint16 == (ushort) 65534u);
        Test.Assert(simple.Fint32 == -2147483646);
        Test.Assert(simple.Fuint32 == 4294967294u);
        Test.Assert(simple.Fint64 == -9223372036854775806);
        Test.Assert(simple.Fuint64 == 18446744073709551614u);
        Test.Assert(simple.Fssize == -2147483646);
        Test.Assert(simple.Fsize == 4294967294u);
        Test.Assert(simple.Fintptr == (IntPtr) 0xFE);
        Test.Assert(simple.Fptrdiff == -2147483646);
        Test.Assert(simple.Ffloat == -16777216.0f);
        Test.Assert(simple.Fdouble == -9007199254740992.0);
        Test.Assert(simple.Fbool == true);
        Test.Assert(simple.Fvoid_ptr == (IntPtr) 0xFE);
        Test.Assert(simple.Fenum == Dummy.SampleEnum.V2);
        Test.Assert(simple.Fstring == "test/string");
        Test.Assert(simple.Fmstring == "test/mstring");
        Test.Assert(simple.Fstringshare == "test/stringshare");
    }

    private static void checkZeroedStructSimple(Dummy.StructSimple simple)
    {
        Test.Assert(simple.Fbyte == 0);
        Test.Assert(simple.Fubyte == 0);
        Test.Assert(simple.Fchar == '\0');
        Test.Assert(simple.Fshort == 0);
        Test.Assert(simple.Fushort == 0);
        Test.Assert(simple.Fint == 0);
        Test.Assert(simple.Fuint == 0);
        Test.Assert(simple.Flong == 0);
        Test.Assert(simple.Fulong == 0);
        Test.Assert(simple.Fllong == 0);
        Test.Assert(simple.Fullong == 0);
        Test.Assert(simple.Fint8 == 0);
        Test.Assert(simple.Fuint8 == 0);
        Test.Assert(simple.Fint16 == 0);
        Test.Assert(simple.Fuint16 == 0);
        Test.Assert(simple.Fint32 == 0);
        Test.Assert(simple.Fuint32 == 0);
        Test.Assert(simple.Fint64 == 0);
        Test.Assert(simple.Fuint64 == 0);
        Test.Assert(simple.Fssize == 0);
        Test.Assert(simple.Fsize == 0);
        Test.Assert(simple.Fintptr == IntPtr.Zero);
        Test.Assert(simple.Fptrdiff == 0);
        Test.Assert(simple.Ffloat == 0);
        Test.Assert(simple.Fdouble == 0);
        Test.Assert(simple.Fbool == false);
        Test.Assert(simple.Fvoid_ptr == IntPtr.Zero);
        Test.Assert(simple.Fenum == Dummy.SampleEnum.V0);
        Test.Assert(simple.Fstring == null);
        Test.Assert(simple.Fmstring == null);
        Test.Assert(simple.Fstringshare == null);
    }

    private static Dummy.StructComplex structComplexWithValues()
    {
        var complex = new Dummy.StructComplex();

        complex.Farray = new Eina.Array<int>();
        complex.Farray.Push(0x0);
        complex.Farray.Push(0x2A);
        complex.Farray.Push(0x42);

        complex.Finarray = new Eina.Inarray<int>();
        complex.Finarray.Push(0x0);
        complex.Finarray.Push(0x2A);
        complex.Finarray.Push(0x42);


        complex.Flist = new Eina.List<string>();
        complex.Flist.Append("0x0");
        complex.Flist.Append("0x2A");
        complex.Flist.Append("0x42");

        complex.Finlist = new Eina.Inlist<int>();
        complex.Finlist.Append(0x0);
        complex.Finlist.Append(0x2A);
        complex.Finlist.Append(0x42);

        complex.Fhash = new Eina.Hash<string, string>();
        complex.Fhash["aa"] = "aaa";
        complex.Fhash["bb"] = "bbb";
        complex.Fhash["cc"] = "ccc";

        complex.Fiterator = complex.Farray.GetIterator();

        complex.Fany_value = new Eina.Value(Eina.ValueType.Double);
        complex.Fany_value.Set(-9007199254740992.0);

        complex.Fany_value_ptr = new Eina.Value(Eina.ValueType.String);
        complex.Fany_value_ptr.Set("abc");

        complex.Fbinbuf = new Eina.Binbuf();
        complex.Fbinbuf.Append(126);

        complex.Fslice.Length = 1;
        complex.Fslice.Mem = Eina.MemoryNative.Alloc(1);
        Marshal.WriteByte(complex.Fslice.Mem, 125);

        complex.Fobj = new Dummy.Numberwrapper();
        complex.Fobj.SetNumber(42);

        return complex;
    }

    private static void checkStructComplex(Dummy.StructComplex complex)
    {
        Test.Assert(complex.Farray.ToArray().SequenceEqual(base_seq_int));

        Test.Assert(complex.Finarray.ToArray().SequenceEqual(base_seq_int));

        Test.Assert(complex.Flist.ToArray().SequenceEqual(base_seq_str));

        Test.Assert(complex.Finlist.ToArray().SequenceEqual(base_seq_int));

        Test.Assert(complex.Fhash["aa"] == "aaa");
        Test.Assert(complex.Fhash["bb"] == "bbb");
        Test.Assert(complex.Fhash["cc"] == "ccc");

        int idx = 0;
        foreach (int e in complex.Fiterator)
        {
            Test.Assert(e == base_seq_int[idx]);
            ++idx;
        }

        double double_val = 0;
        Test.Assert(complex.Fany_value.Get(out double_val));
        Test.Assert(double_val == -9007199254740992.0);

        string str_val = null;
        Test.Assert(complex.Fany_value_ptr.Get(out str_val));
        Test.Assert(str_val == "abc");

        Test.Assert(complex.Fbinbuf.Length == 1);
        Test.Assert(complex.Fbinbuf.GetBytes()[0] == 126);

        Test.Assert(complex.Fslice.Length == 1);
        Test.Assert(complex.Fslice.GetBytes()[0] == 125);

        Test.Assert(complex.Fobj != null);
        Test.Assert(complex.Fobj.GetNumber() == 42);
    }


    private static void checkZeroedStructComplex(Dummy.StructComplex complex)
    {
        Test.Assert(complex.Farray == null);
        Test.Assert(complex.Finarray == null);
        Test.Assert(complex.Flist == null);
        Test.Assert(complex.Finlist == null);
        Test.Assert(complex.Fhash == null);
        Test.Assert(complex.Fiterator == null);
        Test.Assert(complex.Fany_value == null);
        Test.Assert(complex.Fany_value_ptr == null);
        Test.Assert(complex.Fbinbuf == null);

        Test.Assert(complex.Fslice.Length == 0);
        Test.Assert(complex.Fslice.Mem == IntPtr.Zero);

        Test.Assert(complex.Fobj == null);
    }

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
