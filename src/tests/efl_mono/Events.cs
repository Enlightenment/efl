using System;

namespace TestSuite
{

class TestEoEvents
{
    public bool called = false;
    protected void callback(object sender, EventArgs e) {
        called = true;
    }
    protected void another_callback(object sender, EventArgs e) { }

    public static void callback_add_event()
    {
        efl.Loop loop = new efl.LoopConcrete();
        TestEoEvents listener = new TestEoEvents();
        loop.CALLBACK_ADD += listener.callback;

        Test.Assert(!listener.called);
        loop.IDLE += listener.another_callback;
        Test.Assert(listener.called);
    }
}

}
