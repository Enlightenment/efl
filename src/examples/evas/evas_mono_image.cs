using System;
using System.Runtime.CompilerServices;

class TestMain
{
    static int WIDTH = 320;
    static int HEIGHT = 240;

    Evas.Image image;


    static string ImagePath([CallerFilePath] string folder="")
    {
        return System.IO.Path.GetDirectoryName(folder);
    }

    public TestMain(Evas.Image image)
    {
        this.image = image;
    }

    static void Main(string[] args)
    {
        Efl.All.Init();

        Efl.Loop loop = new Efl.Loop();

        EcoreEvas ecore_evas = new EcoreEvas();
        Eina.Size2D size = new Eina.Size2D();
        
        Efl.Canvas.Object canvas = ecore_evas.canvas;
        canvas.SetVisible(true);

        Efl.Object parent = canvas.GetParent();
        System.Diagnostics.Debug.Assert(parent.NativeHandle != IntPtr.Zero);
        
        Efl.Canvas.Rectangle bg = new Efl.Canvas.Rectangle(canvas);
        bg.SetColor(255, 255, 255, 255);
        size.W = WIDTH;
        size.H = HEIGHT;
        bg.SetSize(size);
        bg.SetVisible(true);

        string valid_path = args[0];
        Evas.Image image = new Evas.Image(canvas);
        image.SetFile(valid_path, null);

        /* FIXME evas-image uses error handling code from
         * evas_object_image_load_error_get, which seems to be not available
         * Efl.image.load.State state = image.load_error_get(); */

        // FIXME missing move
        Eina.Rect rect = new Eina.Rect();

        rect.X = 0;
        rect.Y = 0;
        rect.W = WIDTH / 2;
        rect.H = HEIGHT / 2;
        image.SetFill(rect);

        size.W = WIDTH / 2;
        size.H = HEIGHT / 2;
        image.SetSize(size);
        image.SetVisible(true);

        rect = image.GetFill();
        rect.Y -= 50;
        rect.W += 100;
        image.SetFill(rect);

        TestMain listener = new TestMain(image);

        // TODO handle key events in order to alter the image like the C
        // example. Meanwhile, just set some w fill
        /* EventListener callback = new EventListener(); */

        /* bg.key_focus_set(true); */
        /* bg.event_callback_priority_add(Evas.Callback_Type.Key_down, */
        /*         Efl.Callback_Priority.Default, */
        /*         callback, null); */

        loop.Begin();

        Efl.All.Shutdown();
    }

    public void on_key_down(object sender, EventArgs e)
    {
        Console.WriteLine("on_key_down called");
    }
}


