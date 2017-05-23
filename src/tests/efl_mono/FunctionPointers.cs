using System;

namespace TestSuite
{

class TestFunctionPointers
{

    static bool called = false;

    static int twice(int a)
    {
        called = true;
        return a * 2;
    }

    static int thrice(int a)
    {
        called = true;
        return a * 3;
    }

    static void setup()
    {
        called = false;
    }

    public static void set_callback_basic()
    {
        setup();
        test.Testing obj = new test.TestingConcrete();
        obj.set_callback(twice);

        Test.Assert(called == false, "set_callback should not call the callback");

        int x = obj.call_callback(42);

        Test.Assert(called, "call_callback must call a callback");
        Test.AssertEquals(42 * 2, x);
    }

    public static void set_callback_with_lambda()
    {
        setup();

        test.Testing obj = new test.TestingConcrete();
        obj.set_callback(y => {
                    called = true;
                    return y + 4;
                });

        Test.Assert(called == false, "set_callback should not call the callback");

        int x = obj.call_callback(37);

        Test.Assert(called, "call_callback must call a callback");
        Test.AssertEquals(37 + 4, x);
    }

    public static void replace_callback()
    {
        setup();

        test.Testing obj = new test.TestingConcrete();
        obj.set_callback(twice);
        Test.Assert(called == false, "set_callback should not call the callback");

        int x = obj.call_callback(42);
        Test.Assert(called, "call_callback must call a callback");
        Test.AssertEquals(42 * 2, x);

        bool new_called = false;
        obj.set_callback(y => {
                    new_called = true;
                    return y * y;
                });

        Test.Assert(new_called == false, "set_callback should not call the callback");

        x = obj.call_callback(42);
        Test.Assert(new_called, "call_callback must call a callback");
        Test.AssertEquals(42 * 42, x);
    }

    class NoOverride : test.TestingInherit {
    }
    public static void set_callback_inherited_no_override()
    {
        setup();
        NoOverride obj = new NoOverride();
        obj.set_callback(thrice);

        Test.Assert(!called, "set_callback in virtual should not call the callback");

        int x = obj.call_callback(42);

        Test.Assert(called, "call_callback must call a callback");
        Test.AssertEquals(42 * 3, x);
    }

    class WithOverride : test.TestingInherit {
        public bool set_called = false;
        public bool invoke_called = false;
        public SimpleCb cb = null;

        public WithOverride() : base() {

        }
        public override void set_callback(SimpleCb cb) {
            set_called = true;
            this.cb = cb;
        }
        public override int call_callback(int a) {
            invoke_called = true;
            if (cb != null)
                return cb(a);
            eina.Log.Error("No callback set upon call_callback invocation");
            return -1;
        }
    }
    public static void set_callback_inherited_with_override()
    {
        setup();
        WithOverride obj = new WithOverride();
        obj.set_callback(thrice);

        Test.Assert(obj.set_called, "set_callback override must have been called");
        Test.Assert(!obj.invoke_called, "invoke_callback must not have been called");

        obj.set_called = false;
        int x = obj.call_callback(42);

        Test.Assert(!obj.set_called, "set_callback override must not have been called");
        Test.Assert(obj.invoke_called, "set_callback in virtual should not call the callback");

        Test.Assert(called, "call_callback must call a callback");
        Test.AssertEquals(42 * 3, x);
    }
}

}

