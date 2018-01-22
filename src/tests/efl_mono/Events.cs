using System;

namespace TestSuite
{

class TestEoEvents
{
    public bool called = false;
    public bool correct_sender = false;
    protected void callback(object sender, EventArgs e) {
        called = true;
        efl.Object obj = sender as efl.Object;
        if (obj != null)
        {
            obj.SetName("loop_called");
            correct_sender = true;
        }
    }
    protected void another_callback(object sender, EventArgs e) { }

    public static void callback_add_event()
    {
        efl.Loop loop = new efl.LoopConcrete();
        loop.SetName("loop");
        TestEoEvents listener = new TestEoEvents();
        loop.CALLBACK_ADD += listener.callback;

        Test.Assert(!listener.called);
        Test.Assert(!listener.correct_sender);
        Test.AssertEquals("loop", loop.GetName());
        loop.IDLE += listener.another_callback;
        Test.Assert(listener.called);
        Test.Assert(listener.correct_sender);
        Test.AssertEquals("loop_called", loop.GetName());
    }

    public static void event_with_string_payload()
    {
        test.Testing obj = new test.TestingConcrete();
        string received_string = null;

        obj.EVT_WITH_STRING += (object sender, test.EVT_WITH_STRING_Args e) => {
            received_string = e.arg;
        };

        obj.EmitEventWithString("Some args");

        Test.AssertEquals("Some args", received_string);
    }

    public static void event_with_int_payload()
    {
        test.Testing obj = new test.TestingConcrete();
        int received_int= 0;

        obj.EVT_WITH_INT += (object sender, test.EVT_WITH_INT_Args e) => {
            received_int = e.arg;
        };

        obj.EmitEventWithInt(-1984);

        Test.AssertEquals(-1984, received_int);
    }

    public static void event_with_uint_payload()
    {
        test.Testing obj = new test.TestingConcrete();
        uint received_uint = 0;
        obj.EVT_WITH_UINT += (object sender, test.EVT_WITH_UINT_Args e) => {
            received_uint = e.arg;
        };

        obj.EmitEventWithUint(0xbeef);

        Test.AssertEquals<uint>(0xbeef, received_uint);
    }

    public static void event_with_float_payload()
    {
        test.Testing obj = new test.TestingConcrete();
        float received_float= 0;

        obj.EVT_WITH_FLOAT += (object sender, test.EVT_WITH_FLOAT_Args e) => {
            received_float = e.arg;
        };

        obj.EmitEventWithFloat(3.14f);

        Test.AssertEquals(3.14f, received_float);
    }

    public static void event_with_object_payload()
    {
        test.Testing obj = new test.TestingConcrete();
        test.Testing received_obj = null;

        obj.EVT_WITH_OBJ += (object sender, test.EVT_WITH_OBJ_Args e) => {
            received_obj = e.arg;
        };

        test.Testing sent_obj = new test.TestingConcrete();

        obj.EmitEventWithObj(sent_obj);

        Test.AssertEquals(sent_obj, received_obj);
    }
}
}
