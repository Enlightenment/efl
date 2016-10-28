using System;
using System.Runtime.InteropServices;

class TestMain
{
    [DllImport("eo")] static extern void efl_object_init();
    [DllImport("ecore")] static extern void ecore_init();
    [DllImport("evas")] static extern void evas_init();
    static void Main(string[] args)
    {
        efl_object_init();
        ecore_init();
        evas_init();
        
        efl.Loop loop = new efl.LoopConcrete();

        EcoreEvas ecore_evas = new EcoreEvas();
        
        System.Console.WriteLine("Hello World");

        efl.canvas.Object canvas = ecore_evas.canvas;
        canvas.visible_set();
        loop.begin();
    }
}
