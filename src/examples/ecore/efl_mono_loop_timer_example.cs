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
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

class TestMain
{
#if EFL_BETA
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

        timer.TimerTickEvent += listener.on_tick;
        timer.TimerTickEvent += listener.another_callback;
        timer.TimerTickEvent -= listener.another_callback;

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
#else
    public static void Main()
    {
    }
#endif
}


