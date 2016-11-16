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


    static void Main(string[] args)
    {
        efl_object_init();
        ecore_init();
        evas_init();

        efl.Loop loop = new efl.LoopConcrete();

        EcoreEvas ecore_evas = new EcoreEvas();
        
        efl.canvas.Object canvas = ecore_evas.canvas;
        canvas.visible_set(true);

        efl.canvas.Rectangle bg = new efl.canvas.RectangleConcrete(canvas);
        bg.color_set(255, 255, 255, 255);
        bg.position_set(0, 0);
        bg.size_set(WIDTH, HEIGHT);
        bg.visible_set(true);

        string path = args[0];
        evas.Image logo = new evas.ImageConcrete(canvas);
        logo.fill_auto_set(true);

        // TODO add preloaded support (depends on events)

        logo.file_set(path, null);
        logo.size_set(WIDTH / 2, HEIGHT / 2);

        // TODO add a bunch of key/mouse handlers

        logo.visible_set(true);

        int[] pixels = new int[(WIDTH/4) * (HEIGHT / 4)];
        System.Random generator = new System.Random();
        for (int i = 0; i < pixels.Length; i++) {
            pixels[i] = generator.Next();
        }

        evas.Image noise_img = new evas.ImageConcrete(canvas);
        noise_img.size_set(WIDTH/4, HEIGHT / 4);
        // FIXME Add a way to set the pixels.
        // noise_img.data_set(pixels);
        noise_img.fill_auto_set(true);
        noise_img.position_set(WIDTH * 5 / 8, HEIGHT / 8);
        noise_img.visible_set(true);
        Console.WriteLine("Creating noise image with sizez %d, %d", WIDTH/4, HEIGHT/4);

        efl.canvas.Proxy proxy_img = new efl.canvas.ProxyConcrete(canvas);
        proxy_img.source_set(noise_img);
        proxy_img.position_set(WIDTH/2, HEIGHT/2);
        proxy_img.size_set(WIDTH/2, HEIGHT/2);
        proxy_img.visible_set(true);

        loop.begin();
    }
}


