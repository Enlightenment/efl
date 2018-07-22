using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public class MyBox : evas.BoxInherit
{
    public MyBox(efl.IObject parent) : base(parent) {}

    [DllImport("evas")] static extern void evas_obj_box_layout_vertical(IntPtr obj, IntPtr data, IntPtr privdata);
    [DllImport("evas")] static extern void evas_obj_box_layout_horizontal(IntPtr obj, IntPtr data, IntPtr privdata);
    [DllImport("evas")] static extern void evas_object_box_layout_horizontal(IntPtr obj, IntPtr data, IntPtr privdata);
    [DllImport("evas")] static extern IntPtr evas_object_evas_get(IntPtr obj);
    [DllImport("evas")] static extern void evas_event_freeze(IntPtr obj);
    [DllImport("evas")] static extern void evas_event_thaw(IntPtr obj);
    
    override public void CalculateGroup()
    {
        IntPtr evas = evas_object_evas_get(raw_handle);
        evas_event_freeze(evas);
        Console.WriteLine("called group_calculate");
        /* layouting_set(true); */
        evas_obj_box_layout_vertical(raw_handle, IntPtr.Zero, IntPtr.Zero);
        /* layouting_set(false); */
        /* children_changed_set(false); */
        evas_event_thaw(evas);
    }
}

class TestMain
{
    static void Main(string[] args)
    {
        efl.All.Init();

        efl.Loop loop = new efl.Loop();


        EcoreEvas ecore_evas = new EcoreEvas();

        efl.canvas.IObject canvas = ecore_evas.canvas;
        canvas.SetVisible(true);

        efl.IObject parent = canvas.GetParent();
        System.Diagnostics.Debug.Assert(parent.raw_handle != IntPtr.Zero);

        evas.IBox box = new MyBox(canvas);
        eina.Size2D size = new eina.Size2D();

        size.W = 320;
        size.H = 240;

        box.SetSize(size);
        box.SetVisible(true);

        efl.canvas.Rectangle rect = new efl.canvas.Rectangle(canvas);
        rect.SetColor(0, 0, 255, 255);
        size.W = 320;
        size.H = 120;
        rect.SetSize(size);
        rect.SetVisible(true);
        box.Append(rect);

        efl.canvas.Rectangle rect2 = new efl.canvas.Rectangle(canvas);
        rect2.SetColor(0, 255, 0, 255);
        rect2.SetSize(size);
        rect2.SetVisible(true);
        box.Append(rect2);

        loop.Begin();

        efl.All.Shutdown();
    }
}
