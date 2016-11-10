using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

class TestMain
{
    [DllImport("eo")] static extern void efl_object_init();
    [DllImport("ecore")] static extern void ecore_init();
    [DllImport("evas")] static extern void evas_init();

    static int WIDTH = 100;
    static int HEIGHT = 150;

    static void Main(string[] args)
    {
        efl_object_init();
        ecore_init();
        evas_init();

        efl.Loop loop = new efl.LoopConcrete();

        EcoreEvas ecore_evas = new EcoreEvas();
        
        efl.canvas.Object canvas = ecore_evas.canvas;
        canvas.visible_set(true);

        efl.Object parent = canvas.parent_get();
        System.Diagnostics.Debug.Assert(parent.raw_handle != IntPtr.Zero);
        
        efl.canvas.Rectangle bg = new efl.canvas.RectangleConcrete(canvas);
        bg.color_set(255, 255, 255, 255);
        bg.size_set(WIDTH, HEIGHT);
        bg.visible_set(true);


        evas.Table table = new evas.TableConcrete(canvas);
        table.homogeneous_set(evas.object_table.Homogeneous_Mode.None);
        table.padding_set(0, 0);
        table.size_set(WIDTH, HEIGHT);
        table.visible_set(true);

        efl.canvas.Rectangle rect = new efl.canvas.RectangleConcrete(canvas);
        rect.color_set(255, 0, 0, 255);
        rect.hint_min_set(100, 50);
        rect.visible_set(true);
        table.pack(rect, 1, 1, 2, 1);
        
        rect = new efl.canvas.RectangleConcrete(canvas);
        rect.color_set(0, 255, 0, 255);
        rect.hint_min_set(50, 100);
        rect.visible_set(true);
        table.pack(rect, 1, 2, 1, 2);

        rect = new efl.canvas.RectangleConcrete(canvas);
        rect.color_set(0, 0, 255, 255);
        rect.hint_min_set(50, 50);
        rect.visible_set(true);
        table.pack(rect, 2, 2, 1, 1);

        rect = new efl.canvas.RectangleConcrete(canvas);
        rect.color_set(255, 255, 0, 255);
        rect.hint_min_set(50, 50);
        rect.visible_set(true);
        table.pack(rect, 2, 3, 1, 1);
        
        loop.begin();
    }
}

