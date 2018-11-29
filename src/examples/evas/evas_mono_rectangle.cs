using System;

class TestMain
{
    private static int[,] colors = new int[,] {
        {255, 0, 0},
        {0, 255, 0},
        {0, 0, 255}
    };

    static void Main(string[] args)
    {
        int color_index = 0;

        Efl.All.Init();

        Efl.Loop loop = new Efl.Loop();
        EcoreEvas ecore_evas = new EcoreEvas();
        Efl.Canvas.Object canvas = ecore_evas.canvas;
        canvas.SetVisible(true);

        Efl.Object parent = canvas.GetParent();
        System.Diagnostics.Debug.Assert(parent.NativeHandle != IntPtr.Zero);

        Efl.Canvas.Rectangle rect = new Efl.Canvas.Rectangle(canvas);
        rect.SetColor(colors[0, 0], colors[0, 1], colors[0, 2], 255);
        Eina.Size2D size = new Eina.Size2D();
        size.W = 640;
        size.H = 480;
        rect.SetSize(size);
        rect.SetVisible(true);

        canvas.KeyDownEvt += (object sender, Efl.Input.InterfaceKeyDownEvt_Args e) => {
            color_index = (color_index + 1) % 3;
            Console.WriteLine("Key Down");
            Console.WriteLine("Got key obj at {0}", e.arg.NativeHandle.ToString("X"));
            Console.WriteLine("Got key_get() == [{0}]", e.arg.GetKey());
            rect.SetColor(colors[color_index, 0],
                          colors[color_index, 1],
                          colors[color_index, 2], 255);
        };

        loop.Begin();

        Efl.All.Shutdown();
    }
}
