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
}
}
