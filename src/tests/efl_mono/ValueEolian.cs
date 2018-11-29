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
        }
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
        }
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
        }
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
    }
}
#pragma warning restore 1591
}

