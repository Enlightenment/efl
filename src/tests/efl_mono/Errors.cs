using System;

namespace TestSuite
{

class TestEinaError
{
    public static void basic_test()
    {
        eina.Error.Clear();
        Test.AssertNotRaises<efl.EflException>(eina.Error.RaiseIfOcurred);
        eina.Error.Set(eina.Error.ENOENT);
        Test.AssertRaises<efl.EflException>(eina.Error.RaiseIfOcurred);
    }
}

class TestEolianError
{

    public static void global_eina_error()
    {
        test.Testing obj = new test.TestingConcrete();
        Test.AssertRaises<efl.EflException>(() => obj.raises_eina_error());
    }

    class Child : test.TestingInherit {
    }

    public static void global_eina_error_inherited()
    {
        test.Testing obj = new Child();
        Test.AssertRaises<efl.EflException>(() => obj.raises_eina_error());
    }

    class CustomException : Exception {
        public CustomException(string msg): base(msg) {}
    }

    class Overrider : test.TestingInherit {
        public override void children_raise_error() {
            throw (new CustomException("Children error"));
        }
    }

    public static void exception_raised_from_inherited_virtual()
    {
        test.Testing obj = new Overrider();

        Test.AssertRaises<efl.EflException>(obj.call_children_raise_error);
    }

    // return eina_error
    public static void eina_error_return()
    {
        test.Testing obj = new test.TestingConcrete();
        eina.Error expected = 42;
        obj.error_ret_set(expected);
        eina.Error error = obj.returns_error();

        Test.AssertEquals(expected, error);

        expected = 0;
        obj.error_ret_set(expected);
        error = obj.returns_error();

        Test.AssertEquals(expected, error);
    }

    class ReturnOverride : test.TestingInherit {
        eina.Error code;
        public override void error_ret_set(eina.Error err) {
            code = 2 * err;
        }
        public override eina.Error returns_error()
        {
            return code;
        }
    }

    public static void eina_error_return_from_inherited_virtual()
    {
        test.Testing obj = new ReturnOverride();
        eina.Error expected = 42;
        obj.error_ret_set(expected);
        eina.Error error = obj.returns_error();

        Test.AssertEquals(new eina.Error(expected * 2), error);

        expected = 0;
        obj.error_ret_set(expected);
        error = obj.returns_error();

        Test.AssertEquals(new eina.Error(expected * 2), error);
    }

    // events
    class Listener
    {
        public bool called = false;
        public void callback(object sender, EventArgs e) {
            throw (new CustomException("Event exception"));
        }
        public void another_callback(object sender, EventArgs e) {}
    }

    public static void eina_error_event_raise_exception()
    {
        // An event whose managed delegate generates an exception
        // must set an eina_error so it can be reported back to
        // the managed code
        efl.Loop loop = new efl.LoopConcrete();
        Listener listener = new Listener();
        loop.CALLBACK_ADD += listener.callback;

        Test.AssertRaises<efl.EflException>(() => loop.IDLE += listener.another_callback);
    }
}
}
