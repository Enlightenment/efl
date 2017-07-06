#define CODE_ANALYSIS

#pragma warning disable 1591

using System;
using System.Diagnostics.CodeAnalysis;

namespace TestSuite {


[SuppressMessage("Gendarme.Rules.Portability", "DoNotHardcodePathsRule")]
public static class TestEinaValue {
    public static void TestIntSimple()
    {
        using (eina.Value v = new eina.Value(eina.ValueType.Int32)) {
            Test.Assert(v.Set(32));
            int x;
            Test.Assert(v.Get(out x));
            Test.AssertEquals(32, x);
        }
    }

    public static void TestUIntSimple()
    {
        using (eina.Value v = new eina.Value(eina.ValueType.Int32)) {
            Test.Assert(v.Set(0xdeadbeef));
            uint x = 0;
            Test.Assert(v.Get(out x));
            Test.AssertEquals(0xdeadbeef, x);
        }
    }

    public static void TestStringSimple()
    {
        using (eina.Value v = new eina.Value(eina.ValueType.String)) {
            string expected_str = "Hello";
            Test.Assert(v.Set(expected_str));
            string str = null;
            Test.Assert(v.Get(out str));
            Test.AssertEquals(expected_str, str);
        }
    }

    public static void TestSetWrongType()
    {
        using (eina.Value v = new eina.Value(eina.ValueType.String)) {
            Test.AssertRaises<ArgumentException>(() => v.Set(42));
            Test.AssertNotRaises<ArgumentException>(() => v.Set("Wumpus"));
            Test.Assert(v.Setup(eina.ValueType.Int32));
            Test.AssertRaises<ArgumentException>(() => v.Set("Wat?"));
            Test.AssertNotRaises<ArgumentException>(() => v.Set(1984));
        }
    }

    public static void TestValueSetup()
    {
        using (eina.Value v = new eina.Value(eina.ValueType.Int32)) {
            Test.Assert(v.Set(44));
            int x = 0;
            Test.Assert(v.Get(out x));
            Test.AssertEquals(44, x);
            v.Setup(eina.ValueType.String);

            string str = "Hello";
            Test.Assert(v.Get(out str));
            Test.AssertNull(str);
        }
    }

    public static void TestValueFlush()
    {
        using (eina.Value v = new eina.Value(eina.ValueType.Int32)) {
            Test.Assert(v.Set(44));
            v.Flush();

            int x;
            Test.AssertRaises<eina.ValueFlushedException>(() => v.Get(out x));
            x = 42;
            Test.AssertRaises<eina.ValueFlushedException>(() => v.Set(x));

            v.Setup(eina.ValueType.String);
            Test.AssertNotRaises<eina.ValueFlushedException>(() => v.Set("Hello, EFL"));

            string y = String.Empty;
            Test.AssertNotRaises<eina.ValueFlushedException>(() => v.Get(out y));
            v.Flush();
            Test.AssertRaises<eina.ValueFlushedException>(() => v.Get(out y));

            v.Setup(eina.ValueType.Array, eina.ValueType.UInt32);

            Test.AssertNotRaises<eina.ValueFlushedException>(() =>
                    v.Append(42));
            v.Flush();
            Test.AssertRaises<eina.ValueFlushedException>(() =>
                    v.Append(42));

            Test.AssertRaises<eina.ValueFlushedException>(() => v.GetValueSubType());

        }
    }

    public static void TestValueCompareInts()
    {
        using (eina.Value a = new eina.Value(eina.ValueType.Int32))
        using (eina.Value b = new eina.Value(eina.ValueType.Int32)) {
            Test.Assert(a.Set(123));
            Test.Assert(b.Set(123));
            Test.AssertEquals(0, a.CompareTo(b));

            Test.Assert(a.Set(-10));
            Test.AssertLessThan(a, b);

            Test.Assert(a.Set(123));
            Test.Assert(b.Set(10));
            Test.AssertGreaterThan(a, b);
        }
    }

    public static void TestValueComparisonEquals()
    {
        using (eina.Value a = new eina.Value(eina.ValueType.Int32))
        using (eina.Value b = new eina.Value(eina.ValueType.Int32))
        using (eina.Value c = new eina.Value(eina.ValueType.Int32)) {
            Test.Assert(a.Set(1));
            Test.Assert(b.Set(1));
            Test.Assert(c.Set(1));

            Test.Assert(a.Equals(a), "A equals A");
            Test.Assert(a.Equals(b) == b.Equals(a), "A equals B == B equals A");
            Test.Assert(a.Equals(b) == b.Equals(c) == a.Equals(c));

            Test.Assert(b.Set(0));
            Test.Assert(a.Equals(b) == b.Equals(a), "A equals B == B equals A");

            Test.Assert(a.Equals(null) == false, "A == null");
        }
    }

    public static void TestValueComparisonOverloadEquals()
    {
        using (eina.Value a = new eina.Value(eina.ValueType.Int32))
        using (eina.Value b = new eina.Value(eina.ValueType.Int32)) {
            Test.Assert(a.Set(1));
            Test.Assert(b.Set(1));

            Test.Assert(a == b);
            Test.Assert(!(a != b));
            Test.Assert(b == a);
            Test.Assert(!(b != a));

            Test.Assert(b.Set(42));

            Test.Assert(a != b);
            Test.Assert(!(a == b));
            Test.Assert(b != a);
            Test.Assert(!(b == a));

            Test.Assert(b.Set(42));

        }
    }

    public static void TestValueComparisonOverloadLessMore()
    {
        using (eina.Value a = new eina.Value(eina.ValueType.Int32))
        using (eina.Value b = new eina.Value(eina.ValueType.Int32)) {
            Test.Assert(a.Set(1));
            Test.Assert(b.Set(0));

            Test.Assert(a > b);
            Test.Assert(!(a < b));
            Test.Assert(b < a);
            Test.Assert(!(b > a));
        }
    }

    public static void TestValueCompareStrings()
    {
        using (eina.Value a = new eina.Value(eina.ValueType.String))
        using (eina.Value b = new eina.Value(eina.ValueType.String)) {
            Test.Assert(a.Set("aaa"));
            Test.Assert(b.Set("aaa"));
            Test.AssertEquals(0, a.CompareTo(b));

            Test.Assert(a.Set("abc"));
            Test.Assert(b.Set("acd"));
            Test.AssertLessThan(a, b);

            Test.Assert(a.Set("acd"));
            Test.Assert(b.Set("abc"));
            Test.AssertGreaterThan(a, b);
        }
    }

    public static void TestValueCompareArray()
    {
        using(eina.Value a = new eina.Value(eina.ValueType.Array, eina.ValueType.Int32))
        using(eina.Value b = new eina.Value(eina.ValueType.Array, eina.ValueType.Int32)) {

            Test.AssertEquals(a, b);

            Test.Assert(a.Append(0));
            Test.Assert(a.Append(1));
            Test.Assert(a.Append(5));
            Test.Assert(a.Append(42));

            Test.Assert(b.Append(0));
            Test.Assert(b.Append(1));
            Test.Assert(b.Append(5));
            Test.Assert(b.Append(42));

            Test.AssertEquals(a, b);

            a[0] = -1;
            Test.Assert(!a.Equals(b));
            Test.AssertLessThan(a, b);

            a[0] = 10;
            Test.AssertGreaterThan(a, b);

            a[0] = 0;
            Test.AssertEquals(a, b);

            // bigger arrays are greater
            Test.Assert(b.Append(0));
            Test.AssertLessThan(a, b);

            Test.Assert(a.Append(0));
            Test.Assert(a.Append(0));
            Test.AssertGreaterThan(a, b);

            // bigger arrays are greater, unless an element says other wise
            b[0] = 10;
            Test.AssertGreaterThan(b, a);
        }
    }

    /* public static void TestValueCompareList() */
    /* { */
    /*     Test.Assert(false, "Implement me."); */
    /* } */

    /* public static void TestValueCompareHash() */
    /* { */
    /*     Test.Assert(false, "Implement me."); */
    /* } */

    public static void TestValueToString()
    {
        using(eina.Value a = new eina.Value(eina.ValueType.Int32)) {
            int i = -12345;
            string x = $"{i}";
            Test.Assert(a.Set(i));
            Test.AssertEquals(x, a.ToString());

            uint u = 0xdeadbeef;
            x = $"{u}";
            Test.Assert(a.Setup(eina.ValueType.UInt32));
            Test.Assert(a.Set(u));
            Test.AssertEquals(x, a.ToString());

            string s = "Hello, Johnny!";
            x = s;
            Test.Assert(a.Setup(eina.ValueType.String));
            Test.Assert(a.Set(s));
            Test.AssertEquals(x, a.ToString());
        }
    }

    public static void TestValueConvertInt()
    {
        using(eina.Value from = new eina.Value(eina.ValueType.Int32))
        using(eina.Value to = new eina.Value(eina.ValueType.UInt32)) {
            int source = 0x7FFFFFFF;
            uint target_uint;
            int target_int;
            string target_str;
            string source_str = $"{source}";

            Test.Assert(from.Set(source));
            Test.Assert(from.ConvertTo(to));
            Test.Assert(to.Get(out target_uint));
            Test.AssertEquals(target_uint, (uint)source);

            Test.Assert(to.Setup(eina.ValueType.Int32));
            Test.Assert(from.ConvertTo(to));
            Test.Assert(to.Get(out target_int));
            Test.AssertEquals(target_int, source);

            Test.Assert(to.Setup(eina.ValueType.String));
            Test.Assert(from.ConvertTo(to));
            Test.Assert(to.Get(out target_str));
            Test.AssertEquals(target_str, source_str);

            // FIXME Add tests for failing ConvertTo() calls when downcasting
            // to smaller types
        }
    }

    public static void TestValueConvertUInt()
    {
        using(eina.Value from = new eina.Value(eina.ValueType.UInt32))
        using(eina.Value to = new eina.Value(eina.ValueType.UInt32)) {
            uint source = 0xFFFFFFFF;
            uint target_uint;
            string target_str;
            string source_str = $"{source}";

            Test.Assert(from.Set(source));
            Test.Assert(from.ConvertTo(to));
            Test.Assert(to.Get(out target_uint));
            Test.AssertEquals(target_uint, source);

            Test.Assert(to.Setup(eina.ValueType.Int32));
            Test.Assert(!from.ConvertTo(to));

            Test.Assert(to.Setup(eina.ValueType.String));
            Test.Assert(from.ConvertTo(to));
            Test.Assert(to.Get(out target_str));
            Test.AssertEquals(target_str, source_str);

            // FIXME Add tests for failing ConvertTo() calls when downcasting
            // to smaller types
        }
    }

    public static void TestValueArray() {
        using(eina.Value array = new eina.Value(eina.ValueType.Array, eina.ValueType.Int32)) {
            Test.Assert(array.Append(0));
            Test.Assert(array.Append(1));
            Test.Assert(array.Append(5));
            Test.Assert(array.Append(42));


            Test.AssertEquals((int)array[0], 0);
            Test.AssertEquals((int)array[1], 1);
            Test.AssertEquals((int)array[2], 5);
            Test.AssertEquals((int)array[3], 42);
        }

        Test.AssertRaises<ArgumentException>(() => {
            using(eina.Value array = new eina.Value(eina.ValueType.String, eina.ValueType.String)) { }
        });
    }

    public static void TestArrayOutOfBounds() {
        using(eina.Value array = new eina.Value(eina.ValueType.Array, eina.ValueType.Int32)) {
            Test.AssertRaises<System.ArgumentOutOfRangeException>(() => array[0] = 1);
            Test.Assert(array.Append(0));
            Test.AssertNotRaises<System.ArgumentOutOfRangeException>(() => array[0] = 1);
            Test.AssertRaises<System.ArgumentOutOfRangeException>(() => array[1] = 1);
            Test.Assert(array.Append(0));
            Test.AssertNotRaises<System.ArgumentOutOfRangeException>(() => array[1] = 1);

        }
    }

    public static void TestValueArraySubType() {
        using(eina.Value array = new eina.Value(eina.ValueType.Array, eina.ValueType.Int32))
            Test.AssertEquals(eina.ValueType.Int32, array.GetValueSubType());

        using(eina.Value array = new eina.Value(eina.ValueType.Array, eina.ValueType.UInt32))
            Test.AssertEquals(eina.ValueType.UInt32, array.GetValueSubType());
    }

    /* public static void TestValueList() { */
    /*     Test.Assert(false, "Implement me."); */
    /* } */

    /* public static void TestValueHash() { */
    /*     Test.Assert(false, "Implement me."); */
    /* } */

    /* public static void TestValueTimeVal() { */
    /*     Test.Assert(false, "Implement me."); */
    /* } */

    /* public static void TestValueBlob() { */
    /*     Test.Assert(false, "Implement me."); */
    /* } */

    /* public static void TestValueStruct() { */
    /*     Test.Assert(false, "Implement me."); */
    /* } */

    /* public static void TestValueArrayOfStructs() { */
    /*     Test.Assert(false, "Implement me."); */
    /* } */

    /* public static void TestValueOptionalInt() { */
    /*     Test.Assert(false, "Implement me."); */
    /* } */

    /* public static void TestValueOptionalString() { */
    /*     Test.Assert(false, "Implement me."); */
    /* } */

    /* public static void TestValueOptionalStructMembers() { */
    /*     Test.Assert(false, "Implement me."); */
    /* } */
}
#pragma warning restore 1591
}
