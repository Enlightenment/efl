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

        // TODO Add key events to cycle through color and styles.
        evas.Text text = new evas.TextConcrete(canvas);
        text.style_set(evas.Text_Style_Type.Outline_soft_shadow);

        text.color_set(0, 0, 0, 255);
        text.glow_color_set(255, 0, 0, 255);
        text.glow_color_set(0, 255, 0, 255);
        text.outline_color_set(0, 0, 255, 255);
        text.shadow_color_set(0, 255,255, 255);
        text.font_set("Courier", 30);

        text.text_set("sample text");
        text.size_set((3*WIDTH)/4, HEIGHT / 4);
        text.position_set(WIDTH/8, (3*HEIGHT)/8);
        text.visible_set(true);


        loop.begin();
    }
}


