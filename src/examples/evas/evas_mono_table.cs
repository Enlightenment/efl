using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

class TestMain
{
    static int WIDTH = 100;
    static int HEIGHT = 150;

    static void Main(string[] args)
    {
        Efl.All.Init();

        Efl.Loop loop = new Efl.Loop();

        EcoreEvas ecore_evas = new EcoreEvas();
        
        Eina.Size2D size = new Eina.Size2D();
        size.W = WIDTH;
        size.H = HEIGHT;

        Eina.Size2D hint = new Eina.Size2D();

        Efl.Canvas.Object canvas = ecore_evas.canvas;
        canvas.SetVisible(true);

        Efl.Object parent = canvas.GetParent();
        System.Diagnostics.Debug.Assert(parent.NativeHandle != IntPtr.Zero);
        
        Efl.Canvas.Rectangle bg = new Efl.Canvas.Rectangle(canvas);
        bg.SetColor(255, 255, 255, 255);
        bg.SetSize(size);
        bg.SetVisible(true);


        Evas.Table table = new Evas.Table(canvas);
        table.SetHomogeneous(Evas.ObjectTable.HomogeneousMode.None);
        table.SetPadding(0, 0);
        table.SetSize(size);
        table.SetVisible(true);

        Efl.Canvas.Rectangle rect = new Efl.Canvas.Rectangle(canvas);
        rect.SetColor(255, 0, 0, 255);
        hint.W = 100;
        hint.H = 50;
        rect.SetHintMin(hint);
        rect.SetVisible(true);
        table.Pack(rect, 1, 1, 2, 1);
        
        rect = new Efl.Canvas.Rectangle(canvas);
        rect.SetColor(0, 255, 0, 255);
        hint.W = 50;
        hint.H = 100;
        rect.SetHintMin(hint);
        rect.SetVisible(true);
        table.Pack(rect, 1, 2, 1, 2);

        rect = new Efl.Canvas.Rectangle(canvas);
        rect.SetColor(0, 0, 255, 255);
        hint.W = 50;
        hint.H = 50;
        rect.SetHintMin(hint);
        rect.SetVisible(true);
        table.Pack(rect, 2, 2, 1, 1);

        rect = new Efl.Canvas.Rectangle(canvas);
        rect.SetColor(255, 255, 0, 255);
        rect.SetHintMin(hint);
        rect.SetVisible(true);
        table.Pack(rect, 2, 3, 1, 1);
        
        loop.Begin();

        Efl.All.Shutdown();
    }
}

