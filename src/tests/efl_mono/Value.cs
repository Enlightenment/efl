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
using System.Linq;
using System.Diagnostics.CodeAnalysis;
using System.Collections.Generic;

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

    public static void TestObjectSimple()
    {
        using (Eina.Value v = new Eina.Value(Eina.ValueType.Object))
        {
            var obj = new Dummy.TestObject();
            Test.Assert(v.Set(obj));
            Efl.Object target;
            Test.Assert(v.Get(out target));
            Test.AssertEquals(target, obj);
            target.Dispose();
            obj.Dispose();
        }
    }

    // Efl.Object conversions are made explicit to avoid ambiguity between
    // Set(Efl.Object) and Set(Value) when dealing with classes derived from
    // Efl.Object.
    public static void TestObjectImplicit()
    {
        var obj = new Dummy.TestObject();
        var v = (Eina.Value)obj;
        Test.AssertEquals(v.GetValueType(), Eina.ValueType.Object);
        Efl.Object target = (Efl.Object)v;

        Test.AssertEquals(target, obj);
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

    public static void TestValueOptionalObject()
    {
        using (Eina.Value a = new Eina.Value(Eina.ValueType.Object)) {
            Test.Assert(!a.Optional);
            BoolRet dummy = () => a.OptionalEmpty;
            Test.AssertRaises<Eina.InvalidValueTypeException>(() => dummy());
        }

        using (Eina.Value a = new Eina.Value(Eina.ValueType.Optional)) {
            Test.Assert(a.Optional);
            Test.Assert(a.OptionalEmpty); // By default, optional values are empty

            // Sets expectation
            Efl.Object expected = new Dummy.TestObject();
            Test.Assert(a.Set(expected));
            Test.Assert(a.Optional);
            Test.Assert(!a.OptionalEmpty);

            Test.Assert(a.Reset());
            Test.Assert(a.OptionalEmpty);

            Test.Assert(a.Set(expected));
            Test.Assert(!a.OptionalEmpty);

            Efl.Object received = null;
            Test.Assert(a.Get(out received));
            Test.AssertEquals(expected, received);
            received.Dispose();
            expected.Dispose();
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
            actual.Dispose();
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
            actual.Dispose();
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

    public static void TestValueComparisonOverloadLessGreater()
    {
        using (Eina.Value a = new Eina.Value(Eina.ValueType.Int32))
        using (Eina.Value b = new Eina.Value(Eina.ValueType.Int32)) {
            Test.Assert(a.Set(1));
            Test.Assert(b.Set(0));

            Test.Assert(a > b);
            Test.Assert(!(a <= b));
            Test.Assert(!(a < b));
            Test.Assert(a >= b);
            Test.Assert(b < a);
            Test.Assert(!(b >= a));
            Test.Assert(!(b > a));
            Test.Assert(b <= a);

            Test.AssertEquals(a > b, !(a <= b));
            Test.AssertEquals(!(a < b), a >= b);
            Test.AssertEquals(b < a, !(b >= a));
            Test.AssertEquals(!(b > a), b <= a);
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
        using (Eina.Value a = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.Int32))
        using (Eina.Value b = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.Int32)) {

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
        using (Eina.Value a = new Eina.Value(Eina.ValueType.List, Eina.ValueType.Int32))
        using (Eina.Value b = new Eina.Value(Eina.ValueType.List, Eina.ValueType.Int32)) {

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
        using (Eina.Value a = new Eina.Value(Eina.ValueType.Int32)) {
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
        using (Eina.Value from = new Eina.Value(Eina.ValueType.Int32))
        using (Eina.Value to = new Eina.Value(Eina.ValueType.UInt32)) {
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
        using (Eina.Value from = new Eina.Value(Eina.ValueType.UInt32))
        using (Eina.Value to = new Eina.Value(Eina.ValueType.UInt32)) {
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
            using (Eina.Value array = new Eina.Value(Eina.ValueType.String, Eina.ValueType.String)) { }
        });
    }

    public static void TestValueContainerWithNonContainerAccess()
    {
        using (Eina.Value array = new Eina.Value(Eina.ValueType.Int32)) {
            Test.AssertRaises<Eina.InvalidValueTypeException>(() => array[0] = 1);
            object val = null;
            Test.AssertRaises<Eina.InvalidValueTypeException>(() => val = array[0]);
        }
    }

    public static void TestValueArrayOfSByte()
    {
        using (Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.SByte)) {
            Test.AssertEquals(0, array.Count());
            Test.Assert(array.Append(0));
            Test.AssertEquals(1, array.Count());
            Test.Assert(array.Append(1));
            Test.AssertEquals(2, array.Count());
            Test.Assert(array.Append(5));
            Test.AssertEquals(3, array.Count());
            Test.Assert(array.Append(42));
            Test.AssertEquals(4, array.Count());


            Test.AssertEquals((sbyte)array[0], 0);
            Test.AssertEquals((sbyte)array[1], 1);
            Test.AssertEquals((sbyte)array[2], 5);
            Test.AssertEquals((sbyte)array[3], 42);

            array[0] = 120;
            array[1] = -42;
            Test.AssertEquals(4, array.Count());

            Test.AssertEquals((sbyte)array[0], 120);
            Test.AssertEquals((sbyte)array[1], -42);
            Test.AssertEquals((sbyte)array[2], 5);
            Test.AssertEquals((sbyte)array[3], 42);

            Test.AssertEquals("[120, -42, 5, 42]", array.ToString());
        }
    }

    public static void TestValueArrayOfByte()
    {
        using (Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.Byte)) {
            Test.AssertEquals(0, array.Count());
            Test.Assert(array.Append(0));
            Test.AssertEquals(1, array.Count());
            Test.Assert(array.Append(1));
            Test.AssertEquals(2, array.Count());
            Test.Assert(array.Append(5));
            Test.AssertEquals(3, array.Count());
            Test.Assert(array.Append(42));
            Test.AssertEquals(4, array.Count());


            Test.AssertEquals((byte)array[0], 0);
            Test.AssertEquals((byte)array[1], 1);
            Test.AssertEquals((byte)array[2], 5);
            Test.AssertEquals((byte)array[3], 42);

            array[0] = 155;
            array[1] = 42;
            Test.AssertEquals(4, array.Count());

            Test.AssertEquals((byte)array[0], 155);
            Test.AssertEquals((byte)array[1], 42);
            Test.AssertEquals((byte)array[2], 5);
            Test.AssertEquals((byte)array[3], 42);

            Test.AssertEquals("[155, 42, 5, 42]", array.ToString());

            Test.AssertRaises<OverflowException>(() => array[0] = 123214);
        }
    }

    public static void TestValueArrayOfInts()
    {
        using (Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.Int32)) {
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
    }

    public static void TestValueArrayOfInt64s()
    {
        using (Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.Int64)) {
            Test.AssertEquals(0, array.Count());
            Test.Assert(array.Append(0));
            Test.AssertEquals(1, array.Count());
            Test.Assert(array.Append(10000000000));
            Test.AssertEquals(2, array.Count());
            Test.Assert(array.Append(5));
            Test.AssertEquals(3, array.Count());
            Test.Assert(array.Append(42));
            Test.AssertEquals(4, array.Count());


            Test.AssertEquals((long)array[0], 0);
            Test.AssertEquals((long)array[1], 10000000000);
            Test.AssertEquals((long)array[2], 5);
            Test.AssertEquals((long)array[3], 42);

            array[0] = 1984;
            array[1] = -42;
            Test.AssertEquals(4, array.Count());

            Test.AssertEquals((long)array[0], 1984);
            Test.AssertEquals((long)array[1], -42);
            Test.AssertEquals((long)array[2], 5);
            Test.AssertEquals((long)array[3], 42);

            Test.AssertEquals("[1984, -42, 5, 42]", array.ToString());
        }
    }

    public static void TestValueArrayOfUInts()
    {

        using (Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.UInt32)) {
            Test.Assert(array.Append(2));
            Test.AssertEquals((uint)array[0], (uint)2);
            Test.AssertRaises<OverflowException>(() => array[0] = -1);
        }
    }

    public static void TestValueArrayOfStrings()
    {

        using (Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.String)) {

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

    public static void TestValueArrayOfObjects()
    {

        using (Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.Object)) {

            var a = new Dummy.TestObject();
            var b = new Dummy.TestObject();

            Test.Assert(array.Append(a));
            Test.Assert(array.Append(b));

            Test.AssertEquals((Efl.Object)array[0], a);
            Test.AssertEquals((Efl.Object)array[1], b);

            var c = new Dummy.TestObject();
            array[0] = c;
            array[1] = b;

            Test.AssertEquals((Efl.Object)array[0], c);
            Test.AssertEquals((Efl.Object)array[1], b);
            c.Dispose();
            b.Dispose();
            a.Dispose();
        }
    }

    public static void TestInsertValueArrayOfSByte()
    {
        using (Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.SByte)) {
            Test.AssertEquals(0, array.Count());
            Test.Assert(!array.Insert(0, 3));
            Test.Assert(!array.Insert(0, 1));
            Test.Assert(!array.Insert(-123, 12));
            Test.AssertEquals(0, array.Count());

            Test.Assert(array.Insert(0, 0));
            Test.AssertEquals(1, array.Count());
            Test.Assert(array.Insert(6, 1));
            Test.AssertEquals(2, array.Count());
            Test.Assert(array.Insert(-122, 0));
            Test.AssertEquals(3, array.Count());
            Test.Assert(array.Insert(30, 1));
            Test.AssertEquals(4, array.Count());

            Test.AssertEquals((sbyte)array[0], (sbyte)-122);
            Test.AssertEquals((sbyte)array[1], (sbyte)30);
            Test.AssertEquals((sbyte)array[2], (sbyte)0);
            Test.AssertEquals((sbyte)array[3], (sbyte)6);

            array[0] = 120;
            array[1] = -42;
            Test.AssertEquals(4, array.Count());

            Test.AssertEquals((sbyte)array[0], (sbyte)120);
            Test.AssertEquals((sbyte)array[1], (sbyte)-42);
            Test.AssertEquals((sbyte)array[2], (sbyte)0);
            Test.AssertEquals((sbyte)array[3], (sbyte)6);

            Test.AssertEquals("[120, -42, 0, 6]", array.ToString());
        }
    }

    public static void TestInsertValueArrayOfByte()
    {
        using (Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.Byte)) {
            Test.AssertEquals(0, array.Count());
            Test.Assert(!array.Insert(0, 3));
            Test.Assert(!array.Insert(0, 1));
            Test.Assert(!array.Insert(123, 12));
            Test.AssertEquals(0, array.Count());

            Test.Assert(array.Insert(0, 0));
            Test.AssertEquals(1, array.Count());
            Test.Assert(array.Insert(6, 1));
            Test.AssertEquals(2, array.Count());
            Test.Assert(array.Insert(122, 0));
            Test.AssertEquals(3, array.Count());
            Test.Assert(array.Insert(30, 1));
            Test.AssertEquals(4, array.Count());

            Test.AssertEquals((byte)array[0], (byte)122);
            Test.AssertEquals((byte)array[1], (byte)30);
            Test.AssertEquals((byte)array[2], (byte)0);
            Test.AssertEquals((byte)array[3], (byte)6);

            array[0] = 120;
            array[1] = 42;
            Test.AssertEquals(4, array.Count());

            Test.AssertEquals((byte)array[0], (byte)120);
            Test.AssertEquals((byte)array[1], (byte)42);
            Test.AssertEquals((byte)array[2], (byte)0);
            Test.AssertEquals((byte)array[3], (byte)6);

            Test.AssertEquals("[120, 42, 0, 6]", array.ToString());
        }
    }

    public static void TestInsertValueArrayOfInts()
    {
        using (Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.Int32)) {
            Test.AssertEquals(0, array.Count());
            Test.Assert(!array.Insert(0, 3));
            Test.Assert(!array.Insert(0, 1));
            Test.Assert(!array.Insert(123, 12));
            Test.AssertEquals(0, array.Count());

            Test.Assert(array.Insert(0, 0));
            Test.AssertEquals(1, array.Count());
            Test.Assert(array.Insert(1, 1));
            Test.AssertEquals(2, array.Count());
            Test.Assert(array.Insert(5, 0));
            Test.AssertEquals(3, array.Count());
            Test.Assert(array.Insert(42, 1));
            Test.AssertEquals(4, array.Count());

            Test.AssertEquals((int)array[0], (int)5);
            Test.AssertEquals((int)array[1], (int)42);
            Test.AssertEquals((int)array[2], (int)0);
            Test.AssertEquals((int)array[3], (int)1);

            array[0] = 1984;
            array[1] = -42;
            Test.AssertEquals(4, array.Count());

            Test.AssertEquals((int)array[0], (int)1984);
            Test.AssertEquals((int)array[1], (int)-42);
            Test.AssertEquals((int)array[2], (int)0);
            Test.AssertEquals((int)array[3], (int)1);

            Test.AssertEquals("[1984, -42, 0, 1]", array.ToString());
        }
    }

    public static void TestInsertValueArrayOfInt64s()
    {
        using (Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.Int64)) {
            Test.AssertEquals(0, array.Count());
            Test.Assert(!array.Insert(0, 3));
            Test.Assert(!array.Insert(0, 1));
            Test.Assert(!array.Insert(123, 12));
            Test.AssertEquals(0, array.Count());

            Test.Assert(array.Insert(0,0));
            Test.AssertEquals(1, array.Count());
            Test.Assert(array.Insert(10000000000,1));
            Test.AssertEquals(2, array.Count());
            Test.Assert(array.Insert(5,0));
            Test.AssertEquals(3, array.Count());
            Test.Assert(array.Insert(42,1));
            Test.AssertEquals(4, array.Count());

            Test.AssertEquals((long)array[0], (long)5);
            Test.AssertEquals((long)array[1], (long)42);
            Test.AssertEquals((long)array[2], (long)0);
            Test.AssertEquals((long)array[3], (long)10000000000);

            array[0] = 1984;
            array[1] = -42;
            Test.AssertEquals(4, array.Count());

            Test.AssertEquals((long)array[0], (long)1984);
            Test.AssertEquals((long)array[1], (long)-42);
            Test.AssertEquals((long)array[2], (long)0);
            Test.AssertEquals((long)array[3], (long)10000000000);

            Test.AssertEquals("[1984, -42, 0, 10000000000]", array.ToString());
        }
    }

    public static void TestInsertValueArrayOfUInts()
    {

        using (Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.UInt32)) {
            Test.AssertEquals(0, array.Count());
            Test.Assert(!array.Insert(0, 3));
            Test.Assert(!array.Insert(0, 1));
            Test.Assert(!array.Insert(123, 12));
            Test.AssertEquals(0, array.Count());

            Test.Assert(array.Insert(0,0));
            Test.AssertEquals(1, array.Count());
            Test.Assert(array.Insert(7,1));
            Test.AssertEquals(2, array.Count());
            Test.Assert(array.Insert(5,0));
            Test.AssertEquals(3, array.Count());
            Test.Assert(array.Insert(42,1));
            Test.AssertEquals(4, array.Count());

            Test.AssertEquals((uint)array[0], (uint)5);
            Test.AssertEquals((uint)array[1], (uint)42);
            Test.AssertEquals((uint)array[2], (uint)0);
            Test.AssertEquals((uint)array[3], (uint)7);

            array[0] = 1984;
            array[1] = 100;
            Test.AssertEquals(4, array.Count());

            Test.AssertEquals((uint)array[0], (uint)1984);
            Test.AssertEquals((uint)array[1], (uint)100);
            Test.AssertEquals((uint)array[2], (uint)0);
            Test.AssertEquals((uint)array[3], (uint)7);

            Test.AssertEquals("[1984, 100, 0, 7]", array.ToString());
        }
    }

    public static void TestInsertValueArrayOfStrings()
    {

        using (Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.String)) {

            Test.AssertEquals(0, array.Count());
            Test.Assert(!array.Insert("hello",3));
            Test.Assert(!array.Insert("world",12));
            Test.AssertEquals(0, array.Count());

            Test.Assert(array.Insert("hello",0));
            Test.Assert(array.Insert("world",1));
            Test.Assert(array.Insert("first",0));
            Test.Assert(array.Insert("sec",1));

            Test.AssertEquals((string)array[0], (string)"first");
            Test.AssertEquals((string)array[1], (string)"sec");
            Test.AssertEquals((string)array[2], (string)"hello");
            Test.AssertEquals((string)array[3], (string)"world");

            array[0] = "efl";
            array[1] = "rocks";
            Test.AssertEquals(4, array.Count());

            Test.AssertEquals((string)array[0], (string)"efl");
            Test.AssertEquals((string)array[1], (string)"rocks");
            Test.AssertEquals((string)array[2], (string)"hello");
            Test.AssertEquals((string)array[3], (string)"world");
        }
    }

    public static void TestInsertValueArrayOfObjects()
    {

        using (Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.Object)) {

            var a = new Dummy.TestObject();
            var b = new Dummy.TestObject();
            var c = new Dummy.TestObject();
            var d = new Dummy.TestObject();

            Test.AssertEquals(0, array.Count());
            Test.Assert(!array.Insert(a,12));
            Test.Assert(!array.Insert(b,3));
            Test.AssertEquals(0, array.Count());

            Test.Assert(array.Insert(a,0));
            Test.Assert(array.Insert(b,1));
            Test.Assert(array.Insert(c,0));
            Test.Assert(array.Insert(d,1));

            Test.AssertEquals((Efl.Object)array[0], c);
            Test.AssertEquals((Efl.Object)array[1], d);
            Test.AssertEquals((Efl.Object)array[2], a);
            Test.AssertEquals((Efl.Object)array[3], b);

            array[0] = a;
            array[1] = b;

            Test.AssertEquals((Efl.Object)array[0], a);
            Test.AssertEquals((Efl.Object)array[1], b);
            Test.AssertEquals((Efl.Object)array[2], a);
            Test.AssertEquals((Efl.Object)array[3], b);

            d.Dispose();
            c.Dispose();
            b.Dispose();
            a.Dispose();
        }
    }

    public static void TestInsertValueListOfSByte()
    {
        using (Eina.Value list = new Eina.Value(Eina.ValueType.List, Eina.ValueType.SByte)) {
            Test.AssertEquals(0, list.Count());
            Test.Assert(!list.Insert(0, 3));
            Test.Assert(!list.Insert(0, 1));
            Test.Assert(!list.Insert(-123, 12));
            Test.AssertEquals(0, list.Count());

            Test.Assert(list.Insert(0, 0));
            Test.AssertEquals(1, list.Count());
            Test.Assert(list.Insert(6, 1));
            Test.AssertEquals(2, list.Count());
            Test.Assert(list.Insert(-122, 0));
            Test.AssertEquals(3, list.Count());
            Test.Assert(list.Insert(30, 1));
            Test.AssertEquals(4, list.Count());

            Test.AssertEquals((sbyte)list[0], (sbyte)-122);
            Test.AssertEquals((sbyte)list[1], (sbyte)30);
            Test.AssertEquals((sbyte)list[2], (sbyte)0);
            Test.AssertEquals((sbyte)list[3], (sbyte)6);

            list[0] = 120;
            list[1] = -42;
            Test.AssertEquals(4, list.Count());

            Test.AssertEquals((sbyte)list[0], (sbyte)120);
            Test.AssertEquals((sbyte)list[1], (sbyte)-42);
            Test.AssertEquals((sbyte)list[2], (sbyte)0);
            Test.AssertEquals((sbyte)list[3], (sbyte)6);

            Test.AssertEquals("[120, -42, 0, 6]", list.ToString());
        }
    }

    public static void TestInsertValueListOfByte()
    {
        using (Eina.Value list = new Eina.Value(Eina.ValueType.List, Eina.ValueType.Byte)) {
            Test.AssertEquals(0, list.Count());
            Test.Assert(!list.Insert(0, 3));
            Test.Assert(!list.Insert(0, 1));
            Test.Assert(!list.Insert(123, 12));
            Test.AssertEquals(0, list.Count());

            Test.Assert(list.Insert(0, 0));
            Test.AssertEquals(1, list.Count());
            Test.Assert(list.Insert(6, 1));
            Test.AssertEquals(2, list.Count());
            Test.Assert(list.Insert(122, 0));
            Test.AssertEquals(3, list.Count());
            Test.Assert(list.Insert(30, 1));
            Test.AssertEquals(4, list.Count());

            Test.AssertEquals((byte)list[0], (byte)122);
            Test.AssertEquals((byte)list[1], (byte)30);
            Test.AssertEquals((byte)list[2], (byte)0);
            Test.AssertEquals((byte)list[3], (byte)6);

            list[0] = 120;
            list[1] = 42;
            Test.AssertEquals(4, list.Count());

            Test.AssertEquals((byte)list[0], (byte)120);
            Test.AssertEquals((byte)list[1], (byte)42);
            Test.AssertEquals((byte)list[2], (byte)0);
            Test.AssertEquals((byte)list[3], (byte)6);

            Test.AssertEquals("[120, 42, 0, 6]", list.ToString());
        }
    }

    public static void TestInsertValueListOfInts()
    {
        using (Eina.Value list = new Eina.Value(Eina.ValueType.List, Eina.ValueType.Int32)) {
            Test.AssertEquals(0, list.Count());
            Test.Assert(!list.Insert(0, 3));
            Test.Assert(!list.Insert(0, 1));
            Test.Assert(!list.Insert(123, 12));
            Test.AssertEquals(0, list.Count());

            Test.Assert(list.Insert(0, 0));
            Test.AssertEquals(1, list.Count());
            Test.Assert(list.Insert(1, 1));
            Test.AssertEquals(2, list.Count());
            Test.Assert(list.Insert(5, 0));
            Test.AssertEquals(3, list.Count());
            Test.Assert(list.Insert(42, 1));
            Test.AssertEquals(4, list.Count());

            Test.AssertEquals((int)list[0], (int)5);
            Test.AssertEquals((int)list[1], (int)42);
            Test.AssertEquals((int)list[2], (int)0);
            Test.AssertEquals((int)list[3], (int)1);

            list[0] = 1984;
            list[1] = -42;
            Test.AssertEquals(4, list.Count());

            Test.AssertEquals((int)list[0], (int)1984);
            Test.AssertEquals((int)list[1], (int)-42);
            Test.AssertEquals((int)list[2], (int)0);
            Test.AssertEquals((int)list[3], (int)1);

            Test.AssertEquals("[1984, -42, 0, 1]", list.ToString());
        }
    }

    public static void TestInsertValueListOfInt64s()
    {
        using (Eina.Value list = new Eina.Value(Eina.ValueType.List, Eina.ValueType.Int64)) {
            Test.AssertEquals(0, list.Count());
            Test.Assert(!list.Insert(0, 3));
            Test.Assert(!list.Insert(0, 1));
            Test.Assert(!list.Insert(123, 12));
            Test.AssertEquals(0, list.Count());

            Test.Assert(list.Insert(0,0));
            Test.AssertEquals(1, list.Count());
            Test.Assert(list.Insert(10000000000,1));
            Test.AssertEquals(2, list.Count());
            Test.Assert(list.Insert(5,0));
            Test.AssertEquals(3, list.Count());
            Test.Assert(list.Insert(42,1));
            Test.AssertEquals(4, list.Count());

            Test.AssertEquals((long)list[0], (long)5);
            Test.AssertEquals((long)list[1], (long)42);
            Test.AssertEquals((long)list[2], (long)0);
            Test.AssertEquals((long)list[3], (long)10000000000);

            list[0] = 1984;
            list[1] = -42;
            Test.AssertEquals(4, list.Count());

            Test.AssertEquals((long)list[0], (long)1984);
            Test.AssertEquals((long)list[1], (long)-42);
            Test.AssertEquals((long)list[2], (long)0);
            Test.AssertEquals((long)list[3], (long)10000000000);

            Test.AssertEquals("[1984, -42, 0, 10000000000]", list.ToString());
        }
    }

    public static void TestInsertValueListOfUInts()
    {

        using (Eina.Value list = new Eina.Value(Eina.ValueType.List, Eina.ValueType.UInt32)) {
            Test.AssertEquals(0, list.Count());
            Test.Assert(!list.Insert(0, 3));
            Test.Assert(!list.Insert(0, 1));
            Test.Assert(!list.Insert(123, 12));
            Test.AssertEquals(0, list.Count());

            Test.Assert(list.Insert(0,0));
            Test.AssertEquals(1, list.Count());
            Test.Assert(list.Insert(7,1));
            Test.AssertEquals(2, list.Count());
            Test.Assert(list.Insert(5,0));
            Test.AssertEquals(3, list.Count());
            Test.Assert(list.Insert(42,1));
            Test.AssertEquals(4, list.Count());

            Test.AssertEquals((uint)list[0], (uint)5);
            Test.AssertEquals((uint)list[1], (uint)42);
            Test.AssertEquals((uint)list[2], (uint)0);
            Test.AssertEquals((uint)list[3], (uint)7);

            list[0] = 1984;
            list[1] = 100;
            Test.AssertEquals(4, list.Count());

            Test.AssertEquals((uint)list[0], (uint)1984);
            Test.AssertEquals((uint)list[1], (uint)100);
            Test.AssertEquals((uint)list[2], (uint)0);
            Test.AssertEquals((uint)list[3], (uint)7);

            Test.AssertEquals("[1984, 100, 0, 7]", list.ToString());
        }
    }

    public static void TestInsertValueListOfStrings()
    {

        using (Eina.Value list = new Eina.Value(Eina.ValueType.List, Eina.ValueType.String)) {

            Test.AssertEquals(0, list.Count());
            Test.Assert(!list.Insert("Nice",2));
            Test.Assert(!list.Insert("efl",5));
            Test.AssertEquals(0, list.Count());

            Test.Assert(list.Insert("hello",0));
            Test.Assert(list.Insert("world",1));
            Test.Assert(list.Insert("first",0));
            Test.Assert(list.Insert("sec",1));

            Test.AssertEquals((string)list[0], (string)"first");
            Test.AssertEquals((string)list[1], (string)"sec");
            Test.AssertEquals((string)list[2], (string)"hello");
            Test.AssertEquals((string)list[3], (string)"world");

            list[0] = "efl";
            list[1] = "rocks";
            Test.AssertEquals(4, list.Count());

            Test.AssertEquals((string)list[0], (string)"efl");
            Test.AssertEquals((string)list[1], (string)"rocks");
            Test.AssertEquals((string)list[2], (string)"hello");
            Test.AssertEquals((string)list[3], (string)"world");
        }
    }

    public static void TestInsertValueListOfObjects()
    {

        using (Eina.Value list = new Eina.Value(Eina.ValueType.List, Eina.ValueType.Object)) {

            var a = new Dummy.TestObject();
            var b = new Dummy.TestObject();
            var c = new Dummy.TestObject();
            var d = new Dummy.TestObject();

            Test.AssertEquals(0, list.Count());
            Test.Assert(!list.Insert(a,6));
            Test.Assert(!list.Insert(b,1));
            Test.Assert(!list.Insert(c,7));
            Test.AssertEquals(0, list.Count());

            Test.Assert(list.Insert(a,0));
            Test.Assert(list.Insert(b,1));
            Test.Assert(list.Insert(c,0));
            Test.Assert(list.Insert(d,1));

            Test.AssertEquals((Efl.Object)list[0], c);
            Test.AssertEquals((Efl.Object)list[1], d);
            Test.AssertEquals((Efl.Object)list[2], a);
            Test.AssertEquals((Efl.Object)list[3], b);

            list[0] = a;
            list[1] = b;

            Test.AssertEquals((Efl.Object)list[0], a);
            Test.AssertEquals((Efl.Object)list[1], b);
            Test.AssertEquals((Efl.Object)list[2], a);
            Test.AssertEquals((Efl.Object)list[3], b);

            d.Dispose();
            c.Dispose();
            b.Dispose();
            a.Dispose();
        }
    }

    public static void TestArrayOutOfBounds() {
        using (Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.Int32)) {
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
        using (Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.Int32))
            Test.AssertEquals(Eina.ValueType.Int32, array.GetValueSubType());

        using (Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.UInt32))
            Test.AssertEquals(Eina.ValueType.UInt32, array.GetValueSubType());
    }

    public static void TestValueArrayConvert() {
        using (Eina.Value array = new Eina.Value(Eina.ValueType.Array, Eina.ValueType.Int32))
        using (Eina.Value other = new Eina.Value(Eina.ValueType.Int32)) {
            other.Set(100);
            other.ConvertTo(array);
            Test.AssertEquals(100, (int)array[0]);
            Test.AssertEquals("[100]", array.ToString());
        }
    }

    public static void TestValueList() {
        using (Eina.Value list = new Eina.Value(Eina.ValueType.List, Eina.ValueType.Int32)) {
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

        using (Eina.Value list = new Eina.Value(Eina.ValueType.List, Eina.ValueType.UInt32)) {
            Test.Assert(list.Append(2));
            Test.AssertEquals((uint)list[0], (uint)2);
            Test.AssertRaises<OverflowException>(() => list[0] = -1);
        }

        using (Eina.Value list = new Eina.Value(Eina.ValueType.List, Eina.ValueType.String)) {

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
        using (Eina.Value list = new Eina.Value(Eina.ValueType.List, Eina.ValueType.Int32)) {
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
        using (Eina.Value list = new Eina.Value(Eina.ValueType.List, Eina.ValueType.Int32))
            Test.AssertEquals(Eina.ValueType.Int32, list.GetValueSubType());

        using (Eina.Value list = new Eina.Value(Eina.ValueType.List, Eina.ValueType.UInt32))
            Test.AssertEquals(Eina.ValueType.UInt32, list.GetValueSubType());
    }

    public static void TestValueListConvert() {
        using (Eina.Value list = new Eina.Value(Eina.ValueType.List, Eina.ValueType.Int32))
        using (Eina.Value other = new Eina.Value(Eina.ValueType.Int32)) {
            other.Set(100);
            other.ConvertTo(list);
            Test.AssertEquals(100, (int)list[0]);
            Test.AssertEquals("[100]", list.ToString());
        }
    }

    public static void TestStringThroughValue() {
        // Check if Value_Native->Value doesn't try to free the pointed string.
        using (Eina.Value value_ptr = new Eina.Value(Eina.ValueType.String)) {
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
        v2.Dispose();
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

public static class TestValueFromObject
{

    private class Holder
    {
        public int Number { get; set; }
        public double Factor { get; set; }
        public string Name { get; set; }
        public Efl.Object Obj { get; set; }
    }

    public static void TestConversionFromToObject()
    {
        var source = new Holder {
            Number = 1984,
            Factor = 3.14,
            Name = "Orwell",
            Obj = new Dummy.TestObject(),
        };

        {
            var prop = source.GetType().GetProperty("Name");
            var v = new Eina.Value(prop.GetValue(source));

            Test.AssertEquals(v.GetValueType(), Eina.ValueType.String);
            Test.AssertEquals((string)v, prop.GetValue(source));

            Test.Assert(v.Set("New value"));
            prop.SetValue(source, v.Unwrap());
            Test.AssertEquals(prop.GetValue(source), "New value");
        }

        {
            var prop = source.GetType().GetProperty("Factor");
            var v = new Eina.Value(prop.GetValue(source));

            Test.AssertEquals(v.GetValueType(), Eina.ValueType.Double);
            Test.AssertAlmostEquals((double)v, (double)prop.GetValue(source));

            Test.Assert(v.Set(2.78));
            prop.SetValue(source, v.Unwrap());
            Test.AssertEquals(prop.GetValue(source), 2.78);
        }

        {
            var prop = source.GetType().GetProperty("Number");
            var v = new Eina.Value(prop.GetValue(source));

            Test.AssertEquals(v.GetValueType(), Eina.ValueType.Int32);
            Test.AssertEquals((int)v, prop.GetValue(source));

            Test.Assert(v.Set(2012));
            prop.SetValue(source, v.Unwrap());
            Test.AssertEquals(prop.GetValue(source), 2012);
        }

        {
            var prop = source.GetType().GetProperty("Obj");
            var v = new Eina.Value(prop.GetValue(source));

            Test.AssertEquals(v.GetValueType(), Eina.ValueType.Object);
            Test.AssertEquals((Efl.Object)v, prop.GetValue(source));

            var newObj = new Dummy.TestObject();
            Test.Assert(v.Set(newObj));
            prop.SetValue(source, v.Unwrap());
            Test.AssertEquals(prop.GetValue(source), newObj);
            newObj.Dispose();
        }
    }

    private class ComplexHolder
    {
        public IEnumerable<int> Bag { get; set; }
        public IEnumerable<Efl.Object> BagOfObjects { get; set; }
    }

    public static void TestContainerFromToObject()
    {
        var initialBag = new Eina.Array<int>();
        initialBag.Push(2);
        initialBag.Push(4);
        initialBag.Push(6);

        var source = new ComplexHolder { Bag = initialBag };
        var prop = source.GetType().GetProperty("Bag");
        var v = new Eina.Value(prop.GetValue(source));
        Test.AssertEquals(prop.GetValue(source), initialBag);

        Test.AssertEquals(v.GetValueType(), Eina.ValueType.Array);
        Test.AssertEquals(v.GetValueSubType(), Eina.ValueType.Int32);

        Test.AssertEquals(v[0], initialBag[0]);
        Test.AssertEquals(v[1], initialBag[1]);
        Test.AssertEquals(v[2], initialBag[2]);

        v[0] = 100;
        v[1] = 200;
        v[2] = 300;

        prop.SetValue(source, v.Unwrap());

        IEnumerable<int> newVal = prop.GetValue(source) as IEnumerable<int>;
        var toCheck = newVal.ToList();

        Test.AssertEquals(toCheck[0], 100);
        Test.AssertEquals(toCheck[1], 200);
        Test.AssertEquals(toCheck[2], 300);
        v.Dispose();
    }

    public static void TestObjectContainerFromToObject()
    {
        var initialBag = new Eina.Array<Efl.Object>();
        var tmp1 = new Dummy.TestObject();
        var tmp2 = new Dummy.TestObject();
        var tmp3 = new Dummy.TestObject();
        initialBag.Push(tmp1);
        initialBag.Push(tmp2);
        initialBag.Push(tmp3);

        var source = new ComplexHolder { BagOfObjects = initialBag };
        var prop = source.GetType().GetProperty("BagOfObjects");
        var v = new Eina.Value(prop.GetValue(source));
        Test.AssertEquals(prop.GetValue(source), initialBag);

        Test.AssertEquals(v.GetValueType(), Eina.ValueType.Array);
        Test.AssertEquals(v.GetValueSubType(), Eina.ValueType.Object);

        Test.AssertEquals(v[0], initialBag[0]);
        Test.AssertEquals(v[1], initialBag[1]);
        Test.AssertEquals(v[2], initialBag[2]);

        var first = new Dummy.TestObject();
        var second = new Dummy.TestObject();
        var third = new Dummy.TestObject();
        v[0] = first;
        v[1] = second;
        v[2] = third;

        prop.SetValue(source, v.Unwrap());

        IEnumerable<Efl.Object> newVal = prop.GetValue(source) as IEnumerable<Efl.Object>;
        var toCheck = newVal.ToList();

        Test.AssertEquals(toCheck[0], first);
        Test.AssertEquals(toCheck[1], second);
        Test.AssertEquals(toCheck[2], third);
        tmp3.Dispose();
        tmp2.Dispose();
        tmp1.Dispose();
        v.Dispose();
    }
}
#pragma warning restore 1591
}
