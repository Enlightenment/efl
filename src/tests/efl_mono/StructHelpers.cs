using System;
using System.Linq;
using System.Runtime.InteropServices;

using static EinaTestData.BaseData;

namespace TestSuite
{

internal class StructHelpers
{
    // Auxiliary function //

    internal static Dummy.StructSimple structSimpleWithValues()
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

    internal static void checkStructSimple(Dummy.StructSimple simple)
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

    internal static void checkZeroedStructSimple(Dummy.StructSimple simple)
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

    internal static Dummy.StructComplex structComplexWithValues()
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

    internal static void checkStructComplex(Dummy.StructComplex complex)
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


    internal static void checkZeroedStructComplex(Dummy.StructComplex complex)
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


}

}
