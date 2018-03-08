using System;
using System.Runtime.InteropServices;
using System.Linq;

using static EinaTestData.BaseData;

namespace TestSuite
{

class TestStructs
{
    // Auxiliary function //

    private static test.StructSimple structSimpleWithValues()
    {
        var simple = new test.StructSimple();

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
        simple.Fenum = test.SampleEnum.V2;
        simple.Fstring = "test/string";
        simple.Fmstring = "test/mstring";
        simple.Fstringshare = "test/stringshare";

        return simple;
    }

    private static void checkStructSimple(test.StructSimple simple)
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
        Test.Assert(simple.Fenum == test.SampleEnum.V2);
        Test.Assert(simple.Fstring == "test/string");
        Test.Assert(simple.Fmstring == "test/mstring");
        Test.Assert(simple.Fstringshare == "test/stringshare");
    }

    private static void checkZeroedStructSimple(test.StructSimple simple)
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
        Test.Assert(simple.Fenum == test.SampleEnum.V0);
        Test.Assert(simple.Fstring == null);
        Test.Assert(simple.Fmstring == null);
        Test.Assert(simple.Fstringshare == null);
    }

    private static test.StructComplex structComplexWithValues()
    {
        var complex = new test.StructComplex();

        complex.Farray = new eina.Array<int>();
        complex.Farray.Push(0x0);
        complex.Farray.Push(0x2A);
        complex.Farray.Push(0x42);

        complex.Finarray = new eina.Inarray<int>();
        complex.Finarray.Push(0x0);
        complex.Finarray.Push(0x2A);
        complex.Finarray.Push(0x42);


        complex.Flist = new eina.List<string>();
        complex.Flist.Append("0x0");
        complex.Flist.Append("0x2A");
        complex.Flist.Append("0x42");

        complex.Finlist = new eina.Inlist<int>();
        complex.Finlist.Append(0x0);
        complex.Finlist.Append(0x2A);
        complex.Finlist.Append(0x42);

        complex.Fhash = new eina.Hash<string, string>();
        complex.Fhash["aa"] = "aaa";
        complex.Fhash["bb"] = "bbb";
        complex.Fhash["cc"] = "ccc";

        complex.Fiterator = complex.Farray.GetIterator();

        complex.Fany_value = new eina.Value(eina.ValueType.Double);
        complex.Fany_value.Set(-9007199254740992.0);

        complex.Fany_value_ptr = new eina.Value(eina.ValueType.String);
        complex.Fany_value_ptr.Set("abc");

        complex.Fbinbuf = new eina.Binbuf();
        complex.Fbinbuf.Append(126);

        complex.Fslice.Length = 1;
        complex.Fslice.Mem = eina.MemoryNative.Alloc(1);
        Marshal.WriteByte(complex.Fslice.Mem, 125);

        complex.Fobj = new test.NumberwrapperConcrete();
        complex.Fobj.SetNumber(42);

        return complex;
    }

    private static void checkStructComplex(test.StructComplex complex)
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


    private static void checkZeroedStructComplex(test.StructComplex complex)
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
        var simple = new test.StructSimple();
        checkZeroedStructSimple(simple);
    }

    private static void complex_default_instantiation()
    {
        var complex = new test.StructComplex();
        checkZeroedStructComplex(complex);
    }

    public static void parameter_initialization()
    {
        var simple = new test.StructSimple(0x1, 0x2, (char)0x3, 0x4, 0x5);
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
        test.Testing t = new test.TestingConcrete();
        bool r = t.StructSimpleIn(simple);
        Test.Assert(r, "Function returned false");
    }

    // public static void simple_ptr_in()
    // {
    //     var simple = structSimpleWithValues();
    //     test.Testing t = new test.TestingConcrete();
    //     bool r = t.struct_simple_ptr_in(simple);
    //     Test.Assert(r, "Function returned false");
    // }

    // public static void simple_ptr_in_own()
    // {
    //     var simple = structSimpleWithValues();
    //     test.Testing t = new test.TestingConcrete();
    //     bool r = t.struct_simple_ptr_in_own(simple);
    //     Test.Assert(r, "Function returned false");
    // }

    public static void simple_out()
    {
        var simple = new test.StructSimple();
        test.Testing t = new test.TestingConcrete();
        bool r = t.StructSimpleOut(ref simple);
        Test.Assert(r, "Function returned false");
        checkStructSimple(simple);
    }

    // public static void simple_ptr_out()
    // {
    // }

    // public static void simple_ptr_out_own()
    // {
    // }

    public static void simple_return()
    {
        test.Testing t = new test.TestingConcrete();
        var simple = t.StructSimpleReturn();
        checkStructSimple(simple);
    }

    // public static void simple_ptr_return()
    // {
    // }

    // public static void simple_ptr_return_own()
    // {
    // }

    public static void complex_in()
    {
        var complex = structComplexWithValues();
        test.Testing t = new test.TestingConcrete();
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
        var complex = new test.StructComplex();
        test.Testing t = new test.TestingConcrete();
        bool r = t.StructComplexOut(ref complex);
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
        test.Testing t = new test.TestingConcrete();
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
