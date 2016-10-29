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
        canvas.visible_set(true);

        efl.canvas.Rectangle rect = new efl.canvas.RectangleConcrete(canvas);
        rect.color_set(255, 255, 255, 255);
        rect.size_set(640, 480);
        rect.visible_set(true);

        evas.Box box = new evas.BoxConcrete(canvas);
        // rect.position_set(160, 120);
        rect.size_set(320, 240);
        box.visible_set(true);
        
        efl.canvas.Image image1 = new efl.canvas.ImageConcrete(canvas);
        image1.file_set("/home/felipe/dev/samsung/upstream/efl/src/examples/elementary/sphere_hunter/score.jpg", "");
        image1.hint_min_set(160, 120);
        image1.visible_set(true);

        efl.canvas.Image image2 = new efl.canvas.ImageConcrete(canvas);
        image2.file_set("/home/felipe/dev/samsung/upstream/efl/src/examples/evas/shooter/assets/images/bricks.jpg", "");
        image2.hint_min_set(160, 120);
        image2.visible_set(true);
        
        box.append(image1);
        box.append(image2);
        
        loop.begin();
    }
}
