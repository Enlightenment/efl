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

    public class CustomException : Exception {
        public CustomException() {}
        public CustomException(string msg): base(msg) {}
        public CustomException(string msg, Exception inner) : base(msg, inner) {}
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
        obj.Dispose();
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
        obj.Dispose();
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
        obj.Dispose();
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
        var  obj = new Dummy.TestObject();
        Listener listener = new Listener();
        obj.EvtWithIntEvent += listener.callback;

        Test.AssertRaises<Efl.EflException>(() =>
        { obj.EmitEventWithInt(2); });
        obj.Dispose();
    }
}
}
