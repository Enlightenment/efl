using System;

namespace TestSuite
{

class TestEinaError
{
    public static void basic_efl_error_available()
    {
        Test.AssertEquals(Eina.Error.UNHANDLED_EXCEPTION.Message, "Unhandled C# exception occurred.");
    }
    public static void basic_test()
    {
        Eina.Error.Clear();
        Eina.Error.Set(Eina.Error.ENOENT);
        Eina.Error err = Eina.Error.Get();
        Test.AssertEquals(err, Eina.Error.ENOENT);
    }
}

class TestEolianError
{

    class CustomException : Exception {
        public CustomException(string msg): base(msg) {}
    }

    class Overrider : Dummy.TestObject {
        public override void ChildrenRaiseError() {
            throw (new CustomException("Children error"));
        }
    }

    public static void exception_raised_from_inherited_virtual()
    {
        var obj = new Overrider();

        Test.AssertRaises<Efl.EflException>(obj.CallChildrenRaiseError);
    }

    // return eina_error
    public static void eina_error_return()
    {
        var obj = new Dummy.TestObject();
        Eina.Error expected = 42;
        obj.SetErrorRet(expected);
        Eina.Error error = obj.ReturnsError();

        Test.AssertEquals(expected, error);

        expected = 0;
        obj.SetErrorRet(expected);
        error = obj.ReturnsError();

        Test.AssertEquals(expected, error);
    }

    class ReturnOverride : Dummy.TestObject {
        Eina.Error code;
        public override void SetErrorRet(Eina.Error err) {
            code = 2 * err;
        }
        public override Eina.Error ReturnsError()
        {
            return code;
        }
    }

    public static void eina_error_return_from_inherited_virtual()
    {
        var obj = new ReturnOverride();
        Eina.Error expected = 42;
        obj.SetErrorRet(expected);
        Eina.Error error = obj.ReturnsError();

        Test.AssertEquals(new Eina.Error(expected * 2), error);

        expected = 0;
        obj.SetErrorRet(expected);
        error = obj.ReturnsError();

        Test.AssertEquals(new Eina.Error(expected * 2), error);
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
        var obj = new Dummy.TestObject();
        Listener listener = new Listener();
        obj.EvtWithIntEvt += listener.callback;

        Test.AssertRaises<Efl.EflException>(() => { obj.EmitEventWithInt(2); });
    }
}
}
