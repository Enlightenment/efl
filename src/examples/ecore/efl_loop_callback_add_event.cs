using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

class TestMain
{
    /* private efl.Loop loop; */

    public TestMain(efl.Loop loop)
    {
        /* this.loop = loop; */
    }

    static void Main(string[] args)
    {
        efl.All.Init();

        efl.Loop loop = new efl.LoopConcrete();

        TestMain listener = new TestMain(loop);

        loop.CALLBACK_ADD += listener.callback_added_cb;

        loop.CALLBACK_ADD += listener.on_idle_enter;
        loop.CALLBACK_ADD -= listener.on_idle_enter;

        loop.IDLE += listener.on_idle_enter; // Will trigger CALLBACK_ADD

        efl.All.Shutdown();
    }

    public void on_idle_enter(object sender, EventArgs e)
    {
        Console.WriteLine("I should not be called while the loop is not running...");
    }

    public void callback_added_cb(object sender, EventArgs e)
    {
        Console.WriteLine("Looks like we added a new callback.");
    }
}


