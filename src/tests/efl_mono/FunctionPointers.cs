using System;

namespace TestSuite
{

class TestFunctionPointers
{

    static bool called = false;
    static bool same_data = false;
    static IntPtr expected_data = IntPtr.Zero;

    static int twice(IntPtr data, int a)
    {
        called = true;
        same_data = data == expected_data;
        return a * 2;
    }

    static int thrice(IntPtr data, int a)
    {
        called = true;
        same_data = data == expected_data;
        return a * 3;
    }

    static void free_it(IntPtr data)
    {
        called = true;
        same_data = data == expected_data;
    }

    static void setup() { setup(IntPtr.Zero); }
    static void setup(IntPtr data)
    {
        called = false;
        same_data = false;
        expected_data = data;
    }

    public static void set_callback_without_data()
    {
        setup(IntPtr.Zero);

        test.Testing obj = new test.TestingConcrete();
        obj.set_callback(twice, expected_data, null);

        Test.Assert(called == false, "set_callback should not call the callback");
        Test.Assert(same_data == false, "set_callback should not call the callback");

        int x = obj.call_callback(42);

        Test.Assert(called, "call_callback must call a callback");
        Test.Assert(same_data, "call_callback must call with expected data");
        Test.Assert(x == 42 * 2, "call_callback must call the right callback");
    }

    public static void set_callback_with_data()
    {
        setup();
        Test.Assert(false, "Implement me");
    }

    public static void replace_callback()
    {
        setup();
        Test.Assert(false, "Implement me");
    }

    public static void set_callback_in_inherited_class()
    {
        setup();
        Test.Assert(false, "Implement me");
    }
}

}

