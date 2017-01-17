using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

class TestMain
{
    [DllImport("eo")] static extern void efl_object_init();
    [DllImport("ecore")] static extern void ecore_init();

    /* private efl.Loop loop; */

    public TestMain(efl.Loop loop)
    {
        /* this.loop = loop; */
    }

    static void Main(string[] args)
    {
        efl_object_init();
        ecore_init();

        efl.Loop loop = new efl.LoopConcrete();

        TestMain listener = new TestMain(loop);

        loop.CALLBACK_ADD += listener.callback_added_cb;

        loop.CALLBACK_ADD += listener.on_idle_enter;
        loop.CALLBACK_ADD -= listener.on_idle_enter;

        loop.IDLE += listener.on_idle_enter; // Will trigger CALLBACK_ADD
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


