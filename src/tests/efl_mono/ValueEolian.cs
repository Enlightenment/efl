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

// Disable warning about missing docs when generating docs
#pragma warning disable 1591

using System;

namespace TestSuite {


public static class TestEinaValueEolian {
    public static void TestEolianEinaValueInReturn()
    {
        var obj = new Dummy.TestObject();

        using (Eina.Value v = new Eina.Value(Eina.ValueType.Int32)) {
            v.Set(42);
            obj.SetValuePtr(v);
            Test.AssertEquals(Eina.Ownership.Managed, v.Ownership);

            Eina.Value v_received = obj.GetValuePtrOwn();
            Test.AssertEquals(Eina.Ownership.Managed, v_received.Ownership);
            Test.AssertEquals(v, v_received);
            v_received.Dispose();
        }
        obj.Dispose();
    }

    public static void TestEolianEinaValueInOwn()
    {
        var obj = new Dummy.TestObject();

        using (Eina.Value v = new Eina.Value(Eina.ValueType.Int32)) {
            v.Set(2001);
            Test.AssertEquals(Eina.Ownership.Managed, v.Ownership);

            obj.SetValuePtrOwn(v);
            Test.AssertEquals(Eina.Ownership.Unmanaged, v.Ownership);

            Eina.Value v_received = obj.GetValuePtr();
            Test.AssertEquals(Eina.Ownership.Unmanaged, v_received.Ownership);

            Test.AssertEquals(v, v_received);

            obj.ClearValue();
        }
        obj.Dispose();
    }

    public static void TestEolianEinaValueOut()
    {
        var obj = new Dummy.TestObject();

        using (Eina.Value v = new Eina.Value(Eina.ValueType.String)) {
            Eina.Value v_out = null;

            v.Set("hello!");
            obj.SetValuePtr(v);
            obj.OutValuePtr(out v_out);

            Test.AssertEquals(v, v_out);
            Test.AssertEquals(Eina.Ownership.Unmanaged, v_out.Ownership);
            v_out.Dispose();
        }
        obj.Dispose();
    }

    public static void TestEolianEinaValueOutOwn()
    {
        var obj = new Dummy.TestObject();

        using (Eina.Value v = new Eina.Value(Eina.ValueType.String)) {
            Eina.Value v_out = null;

            v.Set("hello!");
            obj.SetValuePtr(v);
            obj.OutValuePtrOwn(out v_out);

            Test.AssertEquals(v, v_out);
            Test.AssertEquals(Eina.Ownership.Managed, v_out.Ownership);
            v_out.Dispose();
        }
        obj.Dispose();
    }

    public static void TestEolianEinaValueOutByValue()
    {
        var obj = new Dummy.TestObject();

        using (Eina.Value v = new Eina.Value(Eina.ValueType.String)) {
            Eina.Value v_out = null;

            v.Set("hello!");
            obj.SetValue(v);
            obj.OutValue(out v_out);

            Test.AssertEquals(v, v_out);
            Test.AssertEquals(Eina.Ownership.Managed, v_out.Ownership);
            v_out.Dispose();
        }
        obj.Dispose();
    }

    private class ValueHandler : Dummy.TestObject
    {
        public Eina.Value value;

        public ValueHandler() : base(null)
        {
            value = null;
        }

        public override void SetValue(Eina.Value value)
        {
            this.value = value;
        }
    }

    public static void TestEolianEinaValueByValueConst()
    {
        ValueHandler obj = new ValueHandler();
        using (Eina.Value val = new Eina.Value(Eina.ValueType.String)) {
            obj.CallSetValue(val);
            Test.AssertEquals(val, obj.value);
        }
        obj.Dispose();
    }

    public static void TestEolianEinaValueImplicitOperators()
    {
        var obj = new Dummy.TestObject();
        int payload = 1999;
        obj.SetValue(payload);

        var expected = new Eina.Value(1999);
        var received = new Eina.Value(Eina.ValueType.String);

        received.Dispose();
        obj.OutValue(out received);
        Test.AssertEquals(expected, received);
        Test.AssertEquals(Eina.ValueType.Int32, received.GetValueType());

        int i = received;
        Test.AssertEquals(i, 1999);

        expected.Dispose();
        expected = new Eina.Value("Hallo");
        obj.SetValue("Hallo");

        obj.OutValue(out received);
        Test.AssertEquals(expected, received);
        Test.AssertEquals(Eina.ValueType.String, received.GetValueType());

        string s = received;
        Test.AssertEquals(s, "Hallo");

        // Casting
        expected.Dispose();
        expected = new Eina.Value((double)15);
        obj.SetValue((double)15);

        received.Dispose();
        obj.OutValue(out received);
        Test.AssertEquals(expected, received);
        Test.AssertEquals(Eina.ValueType.Double, received.GetValueType());

        // Check for 0
        // This is a special value, since C# can silently convert it to an enum
        // leading to collisions with Eina.ValueType
        expected.Dispose();
        expected = new Eina.Value(0);
        obj.SetValue(0);
        received.Dispose();
        obj.OutValue(out received);
        Test.AssertEquals(expected, received);
        Test.AssertEquals(Eina.ValueType.Int32, received.GetValueType());
        expected.Dispose();
        received.Dispose();
        obj.Dispose();
    }

// ValueType in eolian context is beta, so not allowed.
// Value does not have this problem as it is used as any_value/any_value_ref
#if EFL_BETA
    public static void TestEolianEinaValueTypeMarshalling()
    {
        var obj = new Dummy.TestObject();

        var values = Enum.GetValues(typeof(Eina.ValueType));
        foreach (Eina.ValueType type in values)
        {
            Test.AssertEquals(type, obj.MirrorValueType(type));
        }
        obj.Dispose();
    }
#endif
}
#pragma warning restore 1591
}

