#define CODE_ANALYSIS

#pragma warning disable 1591

using System;
using System.Diagnostics.CodeAnalysis;

namespace TestSuite {


[SuppressMessage("Gendarme.Rules.Portability", "DoNotHardcodePathsRule")]
public static class TestEinaValue {

    public static void TestByteSimple()
    {
        using (Eina.Value v = new Eina.Value(Eina.ValueType.Byte)) {
            byte val = 0xff;
            Test.Assert(v.Set(val));
            byte x;
            Test.Assert(v.Get(out x));
            Test.AssertEquals(val, x);
        }
    }

    public static void TestSByteSimple()
    {
        using (Eina.Value v = new Eina.Value(Eina.ValueType.SByte)) {
            sbyte val = -45;
            Test.Assert(v.Set(val));
            sbyte x;
            Test.Assert(v.Get(out x));
            Test.AssertEquals(val, x);
        }
    }

    public static void TestShortSimple()
    {
        using (Eina.Value v = new Eina.Value(Eina.ValueType.Short)) {
            short val = -128;
            Test.Assert(v.Set(val));
            short x;
            Test.Assert(v.Get(out x));
            Test.AssertEquals(val, x);
        }
    }

    public static void TestUShortSimple()
    {
        using (Eina.Value v = new Eina.Value(Eina.ValueType.UShort)) {
            ushort val = 0xff55;
            Test.Assert(v.Set(val));
            ushort x;
            Test.Assert(v.Get(out x));
            Test.AssertEquals(val, x);
        }
    }

    public static void TestLongSimple()
    {
        using (Eina.Value v = new Eina.Value(Eina.ValueType.Long)) {
            long val = 0xdeadbeef;
            Test.Assert(v.Set(val));
            long x;
            Test.Assert(v.Get(out x));
            Test.AssertEquals(val, x);
        }
    }

    public static void TestULongSimple()
    {
        using (Eina.Value v = new Eina.Value(Eina.ValueType.ULong)) {
            ulong val = 0xdeadbeef;
            Test.Assert(v.Set(val));
            ulong x;
            Test.Assert(v.Get(out x));
            Test.AssertEquals(val, x);
        }
    }

    public static void TestFloatSimple()
    {
        using (Eina.Value v = new Eina.Value(Eina.ValueType.Float)) {
            float val = 1.609344f;
            Test.Assert(v.Set(val));
            float x;
            Test.Assert(v.Get(out x));
            Test.AssertAlmostEquals(val, x);
        }
    }

    public static void TestDoubleSimple()
    {
        using (Eina.Value v = new Eina.Value(Eina.ValueType.Double)) {
            double val = 1.609344;
            Test.Assert(v.Set(val));
            double  x;
            Test.Assert(v.Get(out x));
            Test.AssertAlmostEquals(val, x);
        }
    }


    public static void TestIntSimple()
    {
        using (Eina.Value v = new Eina.Value(Eina.ValueType.Int32)) {
            Test.Assert(v.Set(32));
            int x;
            Test.Assert(v.Get(out x));
            Test.AssertEquals(32, x);

            Test.Assert(v.Set(-45));
            Test.Assert(v.Get(out x));
            Test.AssertEquals(-45, x);
        }
    }

    public static void TestUIntSimple()
    {
        using (Eina.Value v = new Eina.Value(Eina.ValueType.Int32)) {
            Test.Assert(v.Set(0xdeadbeef));
            uint x = 0;
            Test.Assert(v.Get(out x));
            Test.AssertEquals(0xdeadbeef, x);
        }
    }

    public static void TestStringSimple()
    {
        using (Eina.Value v = new Eina.Value(Eina.ValueType.String)) {
            string expected_str = "Hello";
            Test.Assert(v.Set(expected_str));
            string str = null;
            Test.Assert(v.Get(out str));
            Test.AssertEquals(expected_str, str);
        }
    }

    public static void TestErrorSimple()
    {
        using (Eina.Value v = new Eina.Value(Eina.ValueType.Error)) {
            Eina.Error error = new Eina.Error(Eina.Error.NO_ERROR);
            Test.Assert(v.Set(error));
            Eina.Error x;
            Test.Assert(v.Get(out x));
            Test.AssertEquals(error, x);
        }
    }

    public static void TestSetWrongType()
    {
        using (Eina.Value v = new Eina.Value(Eina.ValueType.String)) {
            Test.AssertRaises<ArgumentException>(() => v.Set(42));
            Test.AssertNotRaises<ArgumentException>(() => v.Set("Wumpus"));
            Test.Assert(v.Setup(Eina.ValueType.Int32));
            Test.AssertRaises<ArgumentException>(() => v.Set("Wat?"));
            Test.AssertNotRaises<ArgumentException>(() => v.Set(1984));
        }
    }

    public static void TestValueSetup()
    {
        using (Eina.Value v = new Eina.Value(Eina.ValueType.Int32)) {
            Test.Assert(v.Set(44));
            int x = 0;
            Test.Assert(v.Get(out x));
            Test.AssertEquals(44, x);
            v.Setup(Eina.ValueType.String);

            string str = "Hello";
            Test.Assert(v.Get(out str));
            Test.AssertNull(str);
        }
    }

    public static void TestValueDispose()
    {
        Eina.Value v = new Eina.Value(Eina.ValueType.Int32);
        v.Dispose();
        Test.AssertRaises<ObjectDisposedException>(() => v.ToString());
        Test.AssertRaises<ObjectDisposedException>(() => v.Set(24));
    }

    private delegate bool BoolRet();
    public static void TestValueOptionalInt()
    {
        using (Eina.Value a = new Eina.Value(Eina.ValueType.Optional)) {
            Test.Assert(a.Optional);
            Test.Assert(a.OptionalEmpty); // By default, optional values are empty

            // Sets expectation
            int expected = 1984;
            Test.Assert(a.Set(expected));
            Test.Assert(a.Optional);
            Test.Assert(!a.OptionalEmpty);

            Test.Assert(a.Reset());
            Test.Assert(a.OptionalEmpty);

            expected = -4891;
            Test.Assert(a.Set(expected)); // Set() automatically infers the subtype from the argument.
            Test.Assert(!a.OptionalEmpty);

            int actual = 0;
            Test.Assert(a.Get(out actual));
            Test.AssertEquals(expected, actual);
        }
    }
    public static void TestValueOptionalUint()
    {
        using (Eina.Value a = new Eina.Value(Eina.ValueType.Optional)) {
            Test.Assert(a.Optional);
            Test.Assert(a.OptionalEmpty); // By default, optional values are empty

            // Sets expectation
            uint expected = 1984;
            Test.Assert(a.Set(expected));
            Test.Assert(a.Optional);
            Test.Assert(!a.OptionalEmpty);

            Test.Assert(a.Reset());
            Test.Assert(a.OptionalEmpty);

            expected = 0xdeadbeef;
            Test.Assert(a.Set(expected));
            Test.Assert(!a.OptionalEmpty);

            uint actual = 0;
            Test.Assert(a.Get(out actual));
            Test.AssertEquals(expected, actual);
        }
    }
    public static void TestValueOptionalString()
    {
        using (Eina.Value a = new Eina.Value(Eina.ValueType.Int32)) {
            Test.Assert(!a.Optional);
            BoolRet dummy = () => a.OptionalEmpty;
            Test.AssertRaises<Eina.InvalidValueTypeException>(() => dummy());
        }

        using (Eina.Value a = new Eina.Value(Eina.ValueType.Optional)) {
            Test.Assert(a.Optional);
            Test.Assert(a.OptionalEmpty); // By default, optional values are empty

            // Sets expectation
            string expected = "Hello, world!";
            Test.Assert(a.Set(expected));
            Test.Assert(a.Optional);
            Test.Assert(!a.OptionalEmpty);

            Test.Assert(a.Reset());
            Test.Assert(a.OptionalEmpty);

            expected = "!dlrow olleH";
            Test.Assert(a.Set(expected));
            Test.Assert(!a.OptionalEmpty);

            string actual = String.Empty;
            Test.Assert(a.Get(out actual));
            Test.AssertEquals(expected, actual);
        }
    }
    public static void TestValueOptionalArrays()
    {
        using (Eina.Value a = new Eina.Value(Eina.ValueType.Optional))
        using (Eina.Value expected = new Eina.Value(Eina.ValueType.Array,
                                                 Eina.ValueType.Int32))
        {

            Test.Assert(a.Optional);
            Test.Assert(a.OptionalEmpty); // By default, optional values are empty

            // Sets expectation
            Test.Assert(expected.Append(-1));
            Test.Assert(expected.Append(0));
            Test.Assert(expected.Append(2));

            Test.Assert(a.Set(expected));
            Test.Assert(a.Optional);
            Test.Assert(!a.OptionalEmpty);

            Test.Assert(a.Reset());
            Test.Assert(a.OptionalEmpty);

            expected.Append(-42);
            Test.Assert(a.Set(expected));
            Test.Assert(!a.OptionalEmpty);

            Eina.Value actual = null;
            Test.Assert(a.Get(out actual));
            Test.AssertEquals(expected, actual);

            Test.Assert(a.Reset());
            Test.Assert(a.Set(expected));
        }
    }
    public static void TestValueOptionalLists()
    {
        using (Eina.Value a = new Eina.Value(Eina.ValueType.Optional))
        using (Eina.Value expected = new Eina.Value(Eina.ValueType.List,
                                                 Eina.ValueType.Int32))
        {

            Test.Assert(a.Optional);
            Test.Assert(a.OptionalEmpty); // By default, optional values are empty

            // Sets expectation
            Test.Assert(expected.Append(-1));
            Test.Assert(expected.Append(0));
            Test.Assert(expected.Append(2));

            Test.Assert(a.Set(expected));
            Test.Assert(a.Optional);
            Test.Assert(!a.OptionalEmpty);

            Test.Assert(a.Reset());
            Test.Assert(a.OptionalEmpty);

            expected.Append(-42);
            Test.Assert(a.Set(expected));
            Test.Assert(!a.OptionalEmpty);

            Eina.Value actual = null;
            Test.Assert(a.Get(out actual));
            Test.AssertEquals(expected, actual);
        }
    }

    public static void TestValueCompareInts()
    {
        using (Eina.Value a = new Eina.Value(Eina.ValueType.Int32))
        using (Eina.Value b = new Eina.Value(Eina.ValueType.Int32)) {
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
        using (Eina.Value a = new Eina.Value(Eina.ValueType.Int32))
        using (Eina.Value b = new Eina.Value(Eina.ValueType.Int32))
        using (Eina.Value c = new Eina.Value(Eina.ValueType.Int32)) {
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
        using (Eina.Value a = new Eina.Value(Eina.ValueType.Int32))
        using (Eina.Value b = new Eina.Value(Eina.ValueType.Int32)) {
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
        using (Eina.Value a = new Eina.Value(Eina.ValueType.Int32))
        using (Eina.Value b = new Eina.Value(Eina.ValueType.Int32)) {
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
        using (Eina.Value a = new Eina.Value(Eina.ValueType.String))
        using (Eina.Value b = new Eina.Value(Eina.ValueType.String)) {
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
        using(Eina.Value a = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.Int32))
        using(Eina.Value b = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.Int32)) {

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

    public static void TestValueCompareList()
    {
        using(Eina.Value a = new Eina.Value(Eina.ValueType.List, Eina.ValueType.Int32))
        using(Eina.Value b = new Eina.Value(Eina.ValueType.List, Eina.ValueType.Int32)) {

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

    /* public static void TestValueCompareHash() */
    /* { */
    /*     Test.Assert(false, "Implement me."); */
    /* } */

    public static void TestValueToString()
    {
        using(Eina.Value a = new Eina.Value(Eina.ValueType.Int32)) {
            int i = -12345;
            string x = $"{i}";
            Test.Assert(a.Set(i));
            Test.AssertEquals(x, a.ToString());

            uint u = 0xdeadbeef;
            x = $"{u}";
            Test.Assert(a.Setup(Eina.ValueType.UInt32));
            Test.Assert(a.Set(u));
            Test.AssertEquals(x, a.ToString());

            string s = "Hello, Johnny!";
            x = s;
            Test.Assert(a.Setup(Eina.ValueType.String));
            Test.Assert(a.Set(s));
            Test.AssertEquals(x, a.ToString());
        }
    }

    public static void TestValueConvertInt()
    {
        using(Eina.Value from = new Eina.Value(Eina.ValueType.Int32))
        using(Eina.Value to = new Eina.Value(Eina.ValueType.UInt32)) {
            int source = 0x7FFFFFFF;
            uint target_uint;
            int target_int;
            string target_str;
            string source_str = $"{source}";

            Test.Assert(from.Set(source));
            Test.Assert(from.ConvertTo(to));
            Test.Assert(to.Get(out target_uint));
            Test.AssertEquals(target_uint, (uint)source);

            Test.Assert(to.Setup(Eina.ValueType.Int32));
            Test.Assert(from.ConvertTo(to));
            Test.Assert(to.Get(out target_int));
            Test.AssertEquals(target_int, source);

            Test.Assert(to.Setup(Eina.ValueType.String));
            Test.Assert(from.ConvertTo(to));
            Test.Assert(to.Get(out target_str));
            Test.AssertEquals(target_str, source_str);

            // FIXME Add tests for failing ConvertTo() calls when downcasting
            // to smaller types
        }
    }

    public static void TestValueConvertUInt()
    {
        using(Eina.Value from = new Eina.Value(Eina.ValueType.UInt32))
        using(Eina.Value to = new Eina.Value(Eina.ValueType.UInt32)) {
            uint source = 0xFFFFFFFF;
            uint target_uint;
            string target_str;
            string source_str = $"{source}";

            Test.Assert(from.Set(source));
            Test.Assert(from.ConvertTo(to));
            Test.Assert(to.Get(out target_uint));
            Test.AssertEquals(target_uint, source);

            Test.Assert(to.Setup(Eina.ValueType.Int32));
            Test.Assert(!from.ConvertTo(to));

            Test.Assert(to.Setup(Eina.ValueType.String));
            Test.Assert(from.ConvertTo(to));
            Test.Assert(to.Get(out target_str));
            Test.AssertEquals(target_str, source_str);

            // FIXME Add tests for failing ConvertTo() calls when downcasting
            // to smaller types
        }
    }

    public static void TestValueContainerConstructorWrongArgs()
    {
        Test.AssertRaises<ArgumentException>(() => {
            using(Eina.Value array = new Eina.Value(Eina.ValueType.String, Eina.ValueType.String)) { }
        });
    }

    public static void TestValueContainerWithNonContainerAccess()
    {
        using(Eina.Value array = new Eina.Value(Eina.ValueType.Int32)) {
            Test.AssertRaises<Eina.InvalidValueTypeException>(() => array[0] = 1);
            object val = null;
            Test.AssertRaises<Eina.InvalidValueTypeException>(() => val = array[0]);
        }
    }

    public static void TestValueArray() {
        using(Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.Int32)) {
            Test.AssertEquals(0, array.Count());
            Test.Assert(array.Append(0));
            Test.AssertEquals(1, array.Count());
            Test.Assert(array.Append(1));
            Test.AssertEquals(2, array.Count());
            Test.Assert(array.Append(5));
            Test.AssertEquals(3, array.Count());
            Test.Assert(array.Append(42));
            Test.AssertEquals(4, array.Count());


            Test.AssertEquals((int)array[0], 0);
            Test.AssertEquals((int)array[1], 1);
            Test.AssertEquals((int)array[2], 5);
            Test.AssertEquals((int)array[3], 42);

            array[0] = 1984;
            array[1] = -42;
            Test.AssertEquals(4, array.Count());

            Test.AssertEquals((int)array[0], 1984);
            Test.AssertEquals((int)array[1], -42);
            Test.AssertEquals((int)array[2], 5);
            Test.AssertEquals((int)array[3], 42);

            Test.AssertEquals("[1984, -42, 5, 42]", array.ToString());
        }

        using(Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.UInt32)) {
            Test.Assert(array.Append(2));
            Test.AssertEquals((uint)array[0], (uint)2);
            Test.AssertRaises<OverflowException>(() => array[0] = -1);
        }

        using(Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.String)) {

            Test.Assert(array.Append("hello"));
            Test.Assert(array.Append("world"));

            Test.AssertEquals((string)array[0], "hello");
            Test.AssertEquals((string)array[1], "world");

            array[0] = "efl";
            array[1] = "rocks";

            Test.AssertEquals((string)array[0], "efl");
            Test.AssertEquals((string)array[1], "rocks");
        }
    }

    public static void TestArrayOutOfBounds() {
        using(Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.Int32)) {
            object placeholder = null;
            Test.AssertRaises<System.ArgumentOutOfRangeException>(() => array[0] = 1);
            Test.AssertRaises<System.ArgumentOutOfRangeException>(() => placeholder = array[0]);
            Test.Assert(array.Append(0));
            Test.AssertNotRaises<System.ArgumentOutOfRangeException>(() => array[0] = 1);
            Test.AssertNotRaises<System.ArgumentOutOfRangeException>(() => placeholder = array[0]);
            Test.AssertRaises<System.ArgumentOutOfRangeException>(() => array[1] = 1);
            Test.AssertRaises<System.ArgumentOutOfRangeException>(() => placeholder = array[1]);
            Test.Assert(array.Append(0));
            Test.AssertNotRaises<System.ArgumentOutOfRangeException>(() => array[1] = 1);
            Test.AssertNotRaises<System.ArgumentOutOfRangeException>(() => placeholder = array[1]);
        }
    }

    public static void TestValueArraySubType() {
        using(Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.Int32))
            Test.AssertEquals(Eina.ValueType.Int32, array.GetValueSubType());

        using(Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.UInt32))
            Test.AssertEquals(Eina.ValueType.UInt32, array.GetValueSubType());
    }

    public static void TestValueArrayConvert() {
        using(Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.Int32))
        using(Eina.Value other = new Eina.Value(Eina.ValueType.Int32)) {
            other.Set(100);
            other.ConvertTo(array);
            Test.AssertEquals(100, (int)array[0]);
            Test.AssertEquals("[100]", array.ToString());
        }
    }

    public static void TestValueList() {
        using(Eina.Value list = new Eina.Value(Eina.ValueType.List, Eina.ValueType.Int32)) {
            Test.AssertEquals(0, list.Count());
            Test.Assert(list.Append(0));
            Test.AssertEquals(1, list.Count());
            Test.Assert(list.Append(1));
            Test.AssertEquals(2, list.Count());
            Test.Assert(list.Append(5));
            Test.AssertEquals(3, list.Count());
            Test.Assert(list.Append(42));
            Test.AssertEquals(4, list.Count());


            Test.AssertEquals((int)list[0], 0);
            Test.AssertEquals((int)list[1], 1);
            Test.AssertEquals((int)list[2], 5);
            Test.AssertEquals((int)list[3], 42);

            list[0] = 1984;
            list[1] = -42;
            Test.AssertEquals(4, list.Count());

            Test.AssertEquals((int)list[0], 1984);
            Test.AssertEquals((int)list[1], -42);
            Test.AssertEquals((int)list[2], 5);
            Test.AssertEquals((int)list[3], 42);

            Test.AssertEquals("[1984, -42, 5, 42]", list.ToString());
        }

        using(Eina.Value list = new Eina.Value(Eina.ValueType.List, Eina.ValueType.UInt32)) {
            Test.Assert(list.Append(2));
            Test.AssertEquals((uint)list[0], (uint)2);
            Test.AssertRaises<OverflowException>(() => list[0] = -1);
        }

        using(Eina.Value list = new Eina.Value(Eina.ValueType.List, Eina.ValueType.String)) {

            Test.Assert(list.Append("hello"));
            Test.Assert(list.Append("world"));

            Test.AssertEquals((string)list[0], "hello");
            Test.AssertEquals((string)list[1], "world");

            list[0] = "efl";
            list[1] = "rocks";

            Test.AssertEquals((string)list[0], "efl");
            Test.AssertEquals((string)list[1], "rocks");
        }
    }

    public static void TestListOutOfBounds() {
        using(Eina.Value list = new Eina.Value(Eina.ValueType.List, Eina.ValueType.Int32)) {
            object placeholder = null;
            Test.AssertRaises<System.ArgumentOutOfRangeException>(() => list[0] = 1);
            Test.AssertRaises<System.ArgumentOutOfRangeException>(() => placeholder = list[0]);
            Test.Assert(list.Append(0));
            Test.AssertNotRaises<System.ArgumentOutOfRangeException>(() => list[0] = 1);
            Test.AssertNotRaises<System.ArgumentOutOfRangeException>(() => placeholder = list[0]);
            Test.AssertRaises<System.ArgumentOutOfRangeException>(() => list[1] = 1);
            Test.AssertRaises<System.ArgumentOutOfRangeException>(() => placeholder = list[1]);
            Test.Assert(list.Append(0));
            Test.AssertNotRaises<System.ArgumentOutOfRangeException>(() => list[1] = 1);
            Test.AssertNotRaises<System.ArgumentOutOfRangeException>(() => placeholder = list[1]);
        }
    }

    public static void TestValueListSubType() {
        using(Eina.Value list = new Eina.Value(Eina.ValueType.List, Eina.ValueType.Int32))
            Test.AssertEquals(Eina.ValueType.Int32, list.GetValueSubType());

        using(Eina.Value list = new Eina.Value(Eina.ValueType.List, Eina.ValueType.UInt32))
            Test.AssertEquals(Eina.ValueType.UInt32, list.GetValueSubType());
    }

    public static void TestValueListConvert() {
        using(Eina.Value list = new Eina.Value(Eina.ValueType.List, Eina.ValueType.Int32))
        using(Eina.Value other = new Eina.Value(Eina.ValueType.Int32)) {
            other.Set(100);
            other.ConvertTo(list);
            Test.AssertEquals(100, (int)list[0]);
            Test.AssertEquals("[100]", list.ToString());
        }
    }

    public static void TestStringThroughValue() {
        // Check if Value_Native->Value doesn't try to free the pointed string.
        using(Eina.Value value_ptr = new Eina.Value(Eina.ValueType.String)) {
            string payload = "Something";
            value_ptr.Set(payload);
            Eina.ValueNative byvalue = value_ptr;
            Eina.Value another_value_ptr = byvalue;
            Test.AssertEquals(value_ptr, another_value_ptr);
        }
    }

    public static void TestValueEmpty() {
        using (Eina.Value empty = new Eina.Value(Eina.ValueType.Empty)) {
            Test.Assert(empty.Empty, "Value must be empty");

            empty.Setup(Eina.ValueType.Int32);

            // Values already set-up are not empty. For this kind of empty, use Optional
            Test.Assert(!empty.Empty, "Values already set-up must not be empty.");

            empty.Set(42);
            Test.Assert(!empty.Empty, "Values with payload must not be empty.");
        }
    }

    public static void TestValueCopy() {
        Eina.Value v2 = null;
        int raw_val = 42;

        using (Eina.Value v = new Eina.Value(Eina.ValueType.Int32)) {
            Test.Assert(v.Set(raw_val));

            v2 = new Eina.Value(v);
        }

        int rec_val;
        Test.Assert(v2.Get(out rec_val));
        Test.AssertEquals(raw_val, rec_val);
    }

    // FIXME Add remaining list tests

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

    /* public static void TestValueOptionalStructMembers() { */
    /*     Test.Assert(false, "Implement me."); */
    /* } */
}
#pragma warning restore 1591
}
