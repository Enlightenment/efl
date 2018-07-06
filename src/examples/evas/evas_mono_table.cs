using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

class TestMain
{
    static int WIDTH = 100;
    static int HEIGHT = 150;

    static void Main(string[] args)
    {
        efl.All.Init();

        efl.Loop loop = new efl.Loop();

        EcoreEvas ecore_evas = new EcoreEvas();
        
        eina.Size2D size = new eina.Size2D();
        size.W = WIDTH;
        size.H = HEIGHT;

        eina.Size2D hint = new eina.Size2D();

        efl.canvas.IObject canvas = ecore_evas.canvas;
        canvas.SetVisible(true);

        efl.IObject parent = canvas.GetParent();
        System.Diagnostics.Debug.Assert(parent.raw_handle != IntPtr.Zero);
        
        efl.canvas.Rectangle bg = new efl.canvas.Rectangle(canvas);
        bg.SetColor(255, 255, 255, 255);
        bg.SetSize(size);
        bg.SetVisible(true);


        evas.Table table = new evas.Table(canvas);
        table.SetHomogeneous(evas.object_table.Homogeneous_Mode.None);
        table.SetPadding(0, 0);
        table.SetSize(size);
        table.SetVisible(true);

        efl.canvas.Rectangle rect = new efl.canvas.Rectangle(canvas);
        rect.SetColor(255, 0, 0, 255);
        hint.W = 100;
        hint.H = 50;
        rect.SetHintMin(hint);
        rect.SetVisible(true);
        table.Pack(rect, 1, 1, 2, 1);
        
        rect = new efl.canvas.Rectangle(canvas);
        rect.SetColor(0, 255, 0, 255);
        hint.W = 50;
        hint.H = 100;
        rect.SetHintMin(hint);
        rect.SetVisible(true);
        table.Pack(rect, 1, 2, 1, 2);

        rect = new efl.canvas.Rectangle(canvas);
        rect.SetColor(0, 0, 255, 255);
        hint.W = 50;
        hint.H = 50;
        rect.SetHintMin(hint);
        rect.SetVisible(true);
        table.Pack(rect, 2, 2, 1, 1);

        rect = new efl.canvas.Rectangle(canvas);
        rect.SetColor(255, 255, 0, 255);
        rect.SetHintMin(hint);
        rect.SetVisible(true);
        table.Pack(rect, 2, 3, 1, 1);
        
        loop.Begin();

        efl.All.Shutdown();
    }
}

