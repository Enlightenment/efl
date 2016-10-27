using System;
using System.Runtime.InteropServices;

class TestMain
{
    [DllImport("eo")] static extern void efl_object_init();
    [DllImport("ecore")] static extern void ecore_init();
    static void Main(string[] args)
    {
        efl_object_init();
        ecore_init();
        efl.Loop loop = new efl.LoopConcrete();
        
        System.Console.WriteLine("Hello World");
    }
}
