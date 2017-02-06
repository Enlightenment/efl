using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

class TestMain
{
    [DllImport("eo")] static extern void efl_object_init();
    [DllImport("ecore")] static extern void ecore_init();
    [DllImport("evas")] static extern void evas_init();

    static void Main(string[] args)
    {
        eina.Log.Init();
        efl_object_init();
        ecore_init();
        evas_init();

        efl.Loop loop = new efl.LoopConcrete();
        EcoreEvas ecore_evas = new EcoreEvas();
        efl.canvas.Object canvas = ecore_evas.canvas;
        canvas.visible_set(true);

        efl.Object parent = canvas.parent_get();
        System.Diagnostics.Debug.Assert(parent.raw_handle != IntPtr.Zero);

        efl.canvas.Rectangle rect = new efl.canvas.RectangleConcrete(canvas);
        rect.color_set(255, 0, 0, 255);
        rect.size_set(640, 480);
        rect.visible_set(true);

        canvas.KEY_DOWN += (object sender, efl.input.KEY_DOWN_Args e) => {
            Console.WriteLine("Key Down");
            Console.WriteLine("Got key obj at {0}", e.arg.raw_handle.ToString("X"));
            Console.WriteLine("Got key_get() == [{0}]", e.arg.key_get());
            /* Console.WriteLine("Got key code [{0}] at {1}", e.arg.key_code_get(), e.arg.raw_handle); */
        };

        loop.begin();
    }
}
