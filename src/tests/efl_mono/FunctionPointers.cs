using System;
using System.Runtime.InteropServices;

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
        var obj = new Dummy.TestObject();
        obj.SetCallback(twice);

        Test.Assert(called == false, "set_callback should not call the callback");

        int x = obj.CallCallback(42);

        Test.Assert(called, "call_callback must call a callback");
        Test.AssertEquals(42 * 2, x);
    }

    public static void set_callback_with_lambda()
    {
        setup();

        var obj = new Dummy.TestObject();
        obj.SetCallback(y => {
                    called = true;
                    return y + 4;
                });

        Test.Assert(called == false, "set_callback should not call the callback");

        int x = obj.CallCallback(37);

        Test.Assert(called, "call_callback must call a callback");
        Test.AssertEquals(37 + 4, x);
    }

    public static void replace_callback()
    {
        setup();

        var obj = new Dummy.TestObject();
        obj.SetCallback(twice);
        Test.Assert(called == false, "set_callback should not call the callback");

        int x = obj.CallCallback(42);
        Test.Assert(called, "call_callback must call a callback");
        Test.AssertEquals(42 * 2, x);

        bool new_called = false;
        obj.SetCallback(y => {
                    new_called = true;
                    return y * y;
                });

        Test.Assert(new_called == false, "set_callback should not call the callback");

        x = obj.CallCallback(42);
        Test.Assert(new_called, "call_callback must call a callback");
        Test.AssertEquals(42 * 42, x);
    }

    class NoOverride : Dummy.TestObject {
    }
    public static void set_callback_inherited_no_override()
    {
        setup();
        NoOverride obj = new NoOverride();
        obj.SetCallback(thrice);

        Test.Assert(!called, "set_callback in virtual should not call the callback");

        int x = obj.CallCallback(42);

        Test.Assert(called, "call_callback must call a callback");
        Test.AssertEquals(42 * 3, x);
    }

    class WithOverride : Dummy.TestObject {
        public bool set_called = false;
        public bool invoke_called = false;
        public Dummy.SimpleCb cb = null;

        public WithOverride() : base() {

        }
        public override void SetCallback(Dummy.SimpleCb cb) {
            set_called = true;
            this.cb = cb;
        }
        public override int CallCallback(int a) {
            invoke_called = true;
            if (cb != null)
                return cb(a);
            Eina.Log.Error("No callback set upon call_callback invocation");
            return -1;
        }
    }
    public static void set_callback_inherited_with_override()
    {
        setup();
        WithOverride obj = new WithOverride();
        obj.SetCallback(thrice);

        Test.Assert(obj.set_called, "set_callback override must have been called");
        Test.Assert(!obj.invoke_called, "invoke_callback must not have been called");

        obj.set_called = false;
        int x = obj.CallCallback(42);

        Test.Assert(!obj.set_called, "set_callback override must not have been called");
        Test.Assert(obj.invoke_called, "set_callback in virtual should not call the callback");

        Test.Assert(called, "call_callback must call a callback");
        Test.AssertEquals(42 * 3, x);
    }

    // These are needed due to issues calling methods on obj from the GC thread (where the
    // free function is actually called)
    [System.Runtime.InteropServices.DllImport("efl_mono_native_test")] static extern bool free_called_get();
    [System.Runtime.InteropServices.DllImport("efl_mono_native_test")] static extern bool free_called_set(bool val);

     public static void set_callback_inherited_called_from_c()
     {
        setup();
        WithOverride obj = new WithOverride();
        free_called_set(false);
        obj.CallSetCallback();

        Test.Assert(obj.set_called, "set_callback override must have been called");
        Test.Assert(!obj.invoke_called, "invoke_callback must not have been called");
        Test.Assert(!free_called_get(), "call_set_callback must not call the free callback");

        obj.set_called = false;
        int x = obj.CallCallback(42);

        Test.Assert(!obj.set_called, "set_callback override must not have been called");
        Test.Assert(obj.invoke_called, "set_callback in virtual should not call the callback");
        Test.Assert(!free_called_get(), "call_callback must not call the free callback");

        Test.AssertEquals(42 * 3, x);

        setup();
        obj.set_called = false;
        obj.invoke_called = false;
        free_called_set(false);

        // Should release the handle to the wrapper allocated when calling set_callback from C.
        obj.SetCallback(twice);

        GC.Collect();
        GC.WaitForPendingFinalizers();

        Test.Assert(obj.set_called, "set_callback override must have been called");
        Test.Assert(!obj.invoke_called, "invoke_callback must not have been called");
        Test.Assert(free_called_get(), "free callback must have been called");

        obj.set_called = false;
        free_called_set(false);
        x = obj.CallCallback(42);

        Test.Assert(!obj.set_called, "set_callback override must not have been called");
        Test.Assert(obj.invoke_called, "set_callback in virtual should not call the callback");
        Test.Assert(!free_called_get(), "must not call old free_callback on new callback");

        Test.Assert(called, "call_callback must call a callback");
        Test.AssertEquals(42 * 2, x);

    }
}

}

