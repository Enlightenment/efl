using System;

namespace TestSuite
{

class TestEinaError
{
    public static void basic_test()
    {
        eina.Error.Clear();
        Test.AssertNotRaises<efl.EflException>(eina.Error.RaiseIfOccurred);
        eina.Error.Set(eina.Error.ENOENT);
        Test.AssertRaises<efl.EflException>(eina.Error.RaiseIfOccurred);
    }
}

class TestEolianError
{

    public static void global_eina_error()
    {
        test.ITesting obj = new test.Testing();
        Test.AssertRaises<efl.EflException>(() => obj.RaisesEinaError());
    }

    class Child : test.TestingInherit {
    }

    public static void global_eina_error_inherited()
    {
        test.ITesting obj = new Child();
        Test.AssertRaises<efl.EflException>(() => obj.RaisesEinaError());
    }

    class CustomException : Exception {
        public CustomException(string msg): base(msg) {}
    }

    class Overrider : test.TestingInherit {
        public override void ChildrenRaiseError() {
            throw (new CustomException("Children error"));
        }
    }

    public static void exception_raised_from_inherited_virtual()
    {
        test.ITesting obj = new Overrider();

        Test.AssertRaises<efl.EflException>(obj.CallChildrenRaiseError);
    }

    // return eina_error
    public static void eina_error_return()
    {
        test.ITesting obj = new test.Testing();
        eina.Error expected = 42;
        obj.SetErrorRet(expected);
        eina.Error error = obj.ReturnsError();

        Test.AssertEquals(expected, error);

        expected = 0;
        obj.SetErrorRet(expected);
        error = obj.ReturnsError();

        Test.AssertEquals(expected, error);
    }

    class ReturnOverride : test.TestingInherit {
        eina.Error code;
        public override void SetErrorRet(eina.Error err) {
            code = 2 * err;
        }
        public override eina.Error ReturnsError()
        {
            return code;
        }
    }

    public static void eina_error_return_from_inherited_virtual()
    {
        test.ITesting obj = new ReturnOverride();
        eina.Error expected = 42;
        obj.SetErrorRet(expected);
        eina.Error error = obj.ReturnsError();

        Test.AssertEquals(new eina.Error(expected * 2), error);

        expected = 0;
        obj.SetErrorRet(expected);
        error = obj.ReturnsError();

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
        test.ITesting obj = new test.Testing();
        Listener listener = new Listener();
        obj.EvtWithIntEvt += listener.callback;

        Test.AssertRaises<efl.EflException>(() => { obj.EmitEventWithInt(2); });
    }
}
}
