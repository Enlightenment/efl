using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public class MyBox : evas.BoxInherit
{
    public MyBox(efl.Object parent) : base(parent) {}

    [DllImport("evas")] static extern void evas_obj_box_layout_vertical(IntPtr obj, IntPtr data, IntPtr privdata);
    [DllImport("evas")] static extern void evas_obj_box_layout_horizontal(IntPtr obj, IntPtr data, IntPtr privdata);
    [DllImport("evas")] static extern void evas_object_box_layout_horizontal(IntPtr obj, IntPtr data, IntPtr privdata);
    [DllImport("evas")] static extern IntPtr evas_object_evas_get(IntPtr obj);
    [DllImport("evas")] static extern void evas_event_freeze(IntPtr obj);
    [DllImport("evas")] static extern void evas_event_thaw(IntPtr obj);
    
    override public void group_calculate()
    {
        IntPtr evas = evas_object_evas_get(raw_handle);
        evas_event_freeze(evas);
        Console.WriteLine("called group_calculate");
        layouting_set(true);
        evas_obj_box_layout_vertical(raw_handle, IntPtr.Zero, IntPtr.Zero);
        layouting_set(false);
        children_changed_set(false);
        evas_event_thaw(evas);
    }
}

class TestMain
{
    static void Main(string[] args)
    {
        efl.All.Init();

        efl.Loop loop = new efl.LoopConcrete();


        EcoreEvas ecore_evas = new EcoreEvas();

        efl.canvas.Object canvas = ecore_evas.canvas;
        canvas.visible_set(true);

        efl.Object parent = canvas.parent_get();
        System.Diagnostics.Debug.Assert(parent.raw_handle != IntPtr.Zero);

        evas.Box box = new MyBox(canvas);
        box.size_set(320, 240);
        box.visible_set(true);

        efl.canvas.Rectangle rect = new efl.canvas.RectangleConcrete(canvas);
        rect.color_set(0, 0, 255, 255);
        rect.size_set(320, 120);
        rect.visible_set(true);
        box.append(rect);

        efl.canvas.Rectangle rect2 = new efl.canvas.RectangleConcrete(canvas);
        rect2.color_set(0, 255, 0, 255);
        rect2.size_set(320, 120);
        rect2.visible_set(true);
        box.append(rect2);

        loop.begin();

        efl.All.Shutdown();
    }
}
