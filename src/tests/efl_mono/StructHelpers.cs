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
using System.Linq;
using System.Runtime.InteropServices;
using System.Collections.Generic;

using Eina;
using static EinaTestData.BaseData;

namespace TestSuite
{

internal class StructHelpers
{
    // Auxiliary function //

    internal static Dummy.StructSimple structSimpleWithValues()
    {
        return  new Dummy.StructSimple(
           fbyte: (sbyte)-126,
           fubyte: (byte) 254u,
           fchar: '~',
           fshort: (short) -32766,
           fushort: (ushort) 65534u,
           fint: -32766,
           fuint: 65534u,
           flong: -2147483646,
           fulong: 4294967294u,
           fllong: -9223372036854775806,
           fullong: 18446744073709551614u,
           fint8: (sbyte) -126,
           fuint8: (byte) 254u,
           fint16: (short) -32766,
           fuint16: (ushort) 65534u,
           fint32: -2147483646,
           fuint32: 4294967294u,
           fint64: -9223372036854775806,
           fuint64: 18446744073709551614u,
           fssize: -2147483646,
           fsize: 4294967294u,
           fintptr: (IntPtr) 0xFE,
           fptrdiff: -2147483646,
           ffloat: -16777216.0f,
           fdouble: -9007199254740992.0,
           fbool: true,
           fenum: Dummy.SampleEnum.V2,
           fstring: "test/string",
           fmstring: "test/mstring",
           fstringshare: "test/stringshare"
        );
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
        Test.Assert(simple.Fenum == Dummy.SampleEnum.V0);
        Test.Assert(simple.Fstring == null);
        Test.Assert(simple.Fmstring == null);
        Test.Assert(simple.Fstringshare == null);
    }

#if EFL_BETA
    internal static Dummy.StructComplex structComplexWithValues()
    {
        var Farray = new Eina.Array<string>();
        Farray.Add("0x0");
        Farray.Add("0x2A");
        Farray.Add("0x42");

        var Flist = new Eina.List<string>();
        Flist.Add("0x0");
        Flist.Add("0x2A");
        Flist.Add("0x42");

        var Fhash = new Eina.Hash<string, string>();
        Fhash["aa"] = "aaa";
        Fhash["bb"] = "bbb";
        Fhash["cc"] = "ccc";

        var Fiterator = ((Eina.Array<string>)Farray).GetIterator();
        var Faccessor = ((Eina.Array<string>)Farray).GetAccessor();

        var Fany_value = new Eina.Value(Eina.ValueType.Double);
        Fany_value.Set(-9007199254740992.0);

        var Fany_value_ref = new Eina.Value(Eina.ValueType.String);
        Fany_value_ref.Set("abc");

        var Fbinbuf = new Eina.Binbuf();
        Fbinbuf.Append(126);

        var Fslice = new Eina.Slice(Eina.MemoryNative.Alloc(1), (UIntPtr)1);
        Marshal.WriteByte(Fslice.Mem, 125);

        var Fobj = new Dummy.Numberwrapper();
        Fobj.Number = 42;

        return new Dummy.StructComplex(
            farray: Farray,
            flist: Flist,
            fhash: Fhash,
            fiterator: Fiterator,
            faccessor: Faccessor,
            fanyValue:Fany_value,
            fanyValueRef: Fany_value_ref,
            fbinbuf: Fbinbuf,
            fslice:Fslice,
            fobj: Fobj
        );
    }

    internal static void checkStructComplex(Dummy.StructComplex complex)
    {
        Test.Assert(complex.Farray.ToArray().SequenceEqual(base_seq_str));

        Test.Assert(complex.Flist.ToArray().SequenceEqual(base_seq_str));

        Test.Assert(complex.Fhash["aa"] == "aaa");
        Test.Assert(complex.Fhash["bb"] == "bbb");
        Test.Assert(complex.Fhash["cc"] == "ccc");

        int idx = 0;
        foreach (string e in complex.Fiterator)
        {
            Test.Assert(e == base_seq_str[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_str.Length);

        double double_val = 0;
        Test.Assert(complex.FanyValue.Get(out double_val));
        Test.Assert(double_val == -9007199254740992.0);

        string str_val = null;
        Test.Assert(complex.FanyValueRef.Get(out str_val));
        Test.Assert(str_val == "abc");

        Test.Assert(complex.Fbinbuf.Length == 1);
        Test.Assert(complex.Fbinbuf.GetBytes()[0] == 126);

        Test.Assert(complex.Fslice.Length == 1);
        Test.Assert(complex.Fslice.GetBytes()[0] == 125);

        Test.Assert(complex.Fobj != null);
        Test.Assert(complex.Fobj.Number == 42);
    }


    internal static void checkZeroedStructComplex(Dummy.StructComplex complex)
    {
        Test.Assert(complex.Farray == null);
        Test.Assert(complex.Flist == null);
        Test.Assert(complex.Fhash == null);
        Test.Assert(complex.Fiterator == null);
        Test.Assert(complex.FanyValue == null);
        Test.Assert(complex.FanyValueRef == null);
        Test.Assert(complex.Fbinbuf == null);

        Test.Assert(complex.Fslice.Length == 0);
        Test.Assert(complex.Fslice.Mem == IntPtr.Zero);

        Test.Assert(complex.Fobj == null);
    }

#endif

}

}
