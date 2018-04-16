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
        loop.CallbackAddEvt += listener.callback;

        Test.Assert(!listener.called);
        Test.Assert(!listener.correct_sender);
        Test.AssertEquals("loop", loop.GetName());
        loop.IdleEvt += listener.another_callback;
        Test.Assert(listener.called);
        Test.Assert(listener.correct_sender);
        Test.AssertEquals("loop_called", loop.GetName());
    }

    public static void event_with_string_payload()
    {
        test.Testing obj = new test.TestingConcrete();
        string received_string = null;

        obj.EvtWithStringEvt += (object sender, test.EvtWithStringEvt_Args e) => {
            received_string = e.arg;
        };

        obj.EmitEventWithString("Some args");

        Test.AssertEquals("Some args", received_string);
    }

    public static void event_with_int_payload()
    {
        test.Testing obj = new test.TestingConcrete();
        int received_int= 0;

        obj.EvtWithIntEvt += (object sender, test.EvtWithIntEvt_Args e) => {
            received_int = e.arg;
        };

        obj.EmitEventWithInt(-1984);

        Test.AssertEquals(-1984, received_int);
    }

    public static void event_with_uint_payload()
    {
        test.Testing obj = new test.TestingConcrete();
        uint received_uint = 0;
        obj.EvtWithUintEvt += (object sender, test.EvtWithUintEvt_Args e) => {
            received_uint = e.arg;
        };

        obj.EmitEventWithUint(0xbeef);

        Test.AssertEquals<uint>(0xbeef, received_uint);
    }

    public static void event_with_float_payload()
    {
        test.Testing obj = new test.TestingConcrete();
        float received_float= 0;

        obj.EvtWithFloatEvt += (object sender, test.EvtWithFloatEvt_Args e) => {
            received_float = e.arg;
        };

        obj.EmitEventWithFloat(3.14f);

        Test.AssertEquals(3.14f, received_float);
    }

    public static void event_with_object_payload()
    {
        test.Testing obj = new test.TestingConcrete();
        test.Testing received_obj = null;

        obj.EvtWithObjEvt += (object sender, test.EvtWithObjEvt_Args e) => {
            received_obj = e.arg;
        };

        test.Testing sent_obj = new test.TestingConcrete();

        obj.EmitEventWithObj(sent_obj);

        Test.AssertEquals(sent_obj, received_obj);
    }
}
}
