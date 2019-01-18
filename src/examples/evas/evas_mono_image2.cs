using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

class TestMain
{
    static int WIDTH = 320;
    static int HEIGHT = 240;


    static void Main(string[] args)
    {
        Efl.All.Init();

        Efl.Loop loop = new Efl.Loop();

        EcoreEvas ecore_evas = new EcoreEvas();

        Eina.Size2D size = new Eina.Size2D();
        Eina.Position2D pos = new Eina.Position2D();
        
        Efl.Canvas.Object canvas = ecore_evas.canvas;
        canvas.SetVisible(true);

        Efl.Canvas.Rectangle bg = new Efl.Canvas.Rectangle(canvas);
        bg.SetColor(255, 255, 255, 255);
        pos.X = 0;
        pos.Y = 0;
        bg.SetPosition(pos);
        size.W = WIDTH;
        size.H = HEIGHT;
        bg.SetSize(size);
        bg.SetVisible(true);

        string path = args[0];
        Evas.Image logo = new Evas.Image(canvas);
        logo.SetFillAuto(true);

        // TODO add preloaded support (depends on events)

        logo.SetFile(path, null);
        size.W = WIDTH / 2;
        size.H = HEIGHT / 2;
        logo.SetSize(size);

        // TODO add a bunch of key/mouse handlers

        logo.SetVisible(true);

        int[] pixels = new int[(WIDTH/4) * (HEIGHT / 4)];
        System.Random generator = new System.Random();
        for (int i = 0; i < pixels.Length; i++) {
            pixels[i] = generator.Next();
        }

        Evas.Image noise_img = new Evas.Image(canvas);
        size.W = WIDTH / 4;
        size.H = HEIGHT / 4;
        noise_img.SetSize(size);
        // FIXME Add a way to set the pixels.
        // noise_img.data_set(pixels);
        noise_img.SetFillAuto(true);
        pos.X = WIDTH * 5 / 8;
        pos.Y = HEIGHT / 8;
        noise_img.SetPosition(pos);
        noise_img.SetVisible(true);
        Console.WriteLine("Creating noise image with sizez %d, %d", WIDTH/4, HEIGHT/4);

        Efl.Canvas.Proxy proxy_img = new Efl.Canvas.Proxy(canvas);
        proxy_img.SetSource(noise_img);
        pos.X = WIDTH / 2;
        pos.Y = HEIGHT / 2;
        proxy_img.SetPosition(pos);
        size.W = WIDTH / 2;
        size.H = HEIGHT / 2;
        proxy_img.SetSize(size);
        proxy_img.SetVisible(true);

        loop.Begin();
    }
}


