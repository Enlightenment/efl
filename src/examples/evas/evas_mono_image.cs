using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

class TestMain
{
    [DllImport("eo")] static extern void efl_object_init();
    [DllImport("ecore")] static extern void ecore_init();
    [DllImport("evas")] static extern void evas_init();

    static int WIDTH = 320;
    static int HEIGHT = 240;


    static string ImagePath([CallerFilePath] string folder="")
    {
        return System.IO.Path.GetDirectoryName(folder);
    }

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

        string valid_path = args[0];
        evas.Image image = new evas.ImageConcrete(canvas);
        image.file_set(valid_path, null);

        /* FIXME evas-image uses error handling code from
         * evas_object_image_load_error_get, which seems to be not available
         * efl.image.load.State state = image.load_error_get(); */

        // FIXME missing move
        image.fill_set(0, 0, WIDTH / 2, HEIGHT / 2);
        image.size_set(WIDTH / 2, HEIGHT / 2);
        image.visible_set(true);

        int x, y, w, h;
        image.fill_get(out x, out y, out w, out h);
        y -= 50;
        w += 100;
        image.fill_set(x, y, w, h);

        // TODO handle key events in order to alter the image like the C
        // example. Meanwhile, just set some w fill
        /* EventListener callback = new EventListener(); */

        /* bg.key_focus_set(true); */
        /* bg.event_callback_priority_add(evas.Callback_Type.Key_down, */ 
        /*         efl.Callback_Priority.Default, */
        /*         callback, null); */

        loop.begin();
    }
}


