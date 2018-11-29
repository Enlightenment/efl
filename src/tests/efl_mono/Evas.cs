using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

public class MyBox : Evas.Box
{
    public MyBox(Efl.Object parent) : base(parent) {}

    [DllImport("evas")] static extern void evas_obj_box_layout_vertical(IntPtr obj, IntPtr data, IntPtr privdata);
    [DllImport("evas")] static extern void evas_obj_box_layout_horizontal(IntPtr obj, IntPtr data, IntPtr privdata);
    [DllImport("evas")] static extern void evas_object_box_layout_horizontal(IntPtr obj, IntPtr data, IntPtr privdata);
    [DllImport("evas")] static extern IntPtr evas_object_evas_get(IntPtr obj);
    [DllImport("evas")] static extern void evas_event_freeze(IntPtr obj);
    [DllImport("evas")] static extern void evas_event_thaw(IntPtr obj);

    override public void CalculateGroup()
    {
        IntPtr evas = evas_object_evas_get(NativeHandle);
        evas_event_freeze(evas);
        Eina.Log.Debug("called group_calculate");
        // slayouting_set(true);
        evas_obj_box_layout_vertical(NativeHandle, IntPtr.Zero, IntPtr.Zero);
        // layouting_set(false);
        // children_changed_set(false);
        evas_event_thaw(evas);
    }
}

namespace TestSuite
{

class TestEvas
{
    /* private static string ImagePath([CallerFilePath] string folder="") */
    /* { */
    /*     return System.IO.Path.GetDirectoryName(folder); */
    /* } */

    /* public static void simple_layout() */
    /* { */
/* //         efl.Loop loop = new efl.LoopConcrete(); */

    /*     EcoreEvas ecore_evas = new EcoreEvas(); */

    /*     efl.canvas.Object canvas = ecore_evas.canvas; */
    /*     canvas.visible_set(true); */

    /*     efl.Object parent = canvas.parent_get(); */
    /*     Test.Assert(parent.NativeHandle != IntPtr.Zero); */

    /*     efl.canvas.Rectangle rect = new efl.canvas.RectangleConcrete(canvas); */
    /*     rect.color_set(255, 255, 255, 255); */
    /*     rect.size_set(640, 480); */
    /*     rect.visible_set(true); */

    /*     evas.Box box = new MyBox(canvas); */
    /*     rect.size_set(320, 240); */
    /*     box.visible_set(true); */

    /*     efl.canvas.Image image1 = new efl.canvas.ImageConcrete(canvas); */
    /*     image1.file_set(ImagePath() + "/../../examples/elementary/sphere_hunter/score.jpg", ""); */
    /*     image1.hint_min_set(160, 240); */
    /*     image1.visible_set(true); */

    /*     efl.canvas.Image image2 = new efl.canvas.ImageConcrete(canvas); */
    /*     image2.file_set(ImagePath() + "/../../examples/evas/shooter/assets/images/bricks.jpg", ""); */
    /*     image2.hint_min_set(160, 120); */
    /*     image2.visible_set(true); */

    /*     box.append(image1); */
    /*     box.append(image2); */

/* //         loop.begin(); */
    /* } */
}

}
