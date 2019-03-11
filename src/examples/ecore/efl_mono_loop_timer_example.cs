using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

class TestMain
{
    private Efl.Loop loop;
    private int count;

    public TestMain(Efl.Loop loop)
    {
        this.loop = loop;
        this.count = 0;
    }

    static void Main(string[] args)
    {
        Efl.All.Init();

        var loop = new Efl.Loop();
        var timer = new Efl.LoopTimer(loop, 1.0);

        TestMain listener = new TestMain(loop);

        Console.WriteLine("Starting MainLoop");

        timer.TimerTickEvt += listener.on_tick;
        timer.TimerTickEvt += listener.another_callback;
        timer.TimerTickEvt -= listener.another_callback;

        loop.Begin();

        Efl.All.Shutdown();
    }

    public void on_tick(object sender, EventArgs e)
    {
        Console.WriteLine("on_tick called on listener");

        if (count++ == 5) {
            Eina.Value v = new Eina.Value(Eina.ValueType.Int32);
            v.Set(0);
            loop.Quit(v);
        }
    }

    public void another_callback(object sender, EventArgs e)
    {
        Console.WriteLine("Ooops. Should not have been called...");
    }
}


