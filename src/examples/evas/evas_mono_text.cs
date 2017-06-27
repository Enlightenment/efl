using System;
using System.Linq;
using System.Collections.Generic;

static class Extensions
{
    public static IEnumerable<T> Circle<T>(this IEnumerable<T> list, int index=0)
    {
        var count = list.Count();
        index = index % count;

        while (true) {
            yield return list.ElementAt(index);
            index = (index + 1) % count;
        }
    }
}

class TestMain
{
    static int WIDTH = 320;
    static int HEIGHT = 240;

    private EcoreEvas ecore_evas;
    private efl.canvas.Object canvas;
    private efl.canvas.Rectangle bg;
    private evas.Text text;
    private evas.Image border;

    public TestMain(String border_file) {
        ecore_evas = new EcoreEvas();
        canvas = ecore_evas.canvas;
        canvas.visible_set(true);

        bg = new efl.canvas.RectangleConcrete(canvas);
        bg.color_set(255, 255, 255, 255);
        bg.position_set(0, 0);
        bg.size_set(WIDTH, HEIGHT);
        bg.visible_set(true);
        bg.key_focus_set(true);

        bg.KEY_DOWN += On_KeyDown;

        text = new evas.TextConcrete(canvas);
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

        Efl_Font_Size size = 0;
        String font = String.Empty;
        text.font_get(out font, out size);
        Console.WriteLine("Adding text object with font {0} and size {1}", font, size);

        // setup border
        border = new evas.ImageConcrete(canvas);
        border.file_set(border_file, null);
        border.border_set(3, 3, 3, 3);
        border.border_center_fill_set(0);

        border.size_set((3 * WIDTH) / 4 + 3, (HEIGHT / 4) + 3);
        border.position_set((WIDTH / 8)-3, ((3 * HEIGHT) / 8) - 3);
        border.visible_set(true);


    }

    private void On_KeyDown(object sender, efl.input.KEY_DOWN_Args e)
    {
        var key = e.arg.key_get();

        if (key == "h") {
            Console.WriteLine(commands);
        } else if (key == "t") {
            evas.Text_Style_Type type = text.style_get();
            type = (evas.Text_Style_Type)(((int)type + 1) % 10); // 10 hardcoded from C example
            text.style_set(type);
        }
    }

    static string commands = @"commands are:
          t - change text's current style
          h - print help";


    static void Main(string[] args)
    {
        efl.All.Init();

        String border_path = "./src/examples/evas/resources/images/red.png";

        if (args.Length >= 1)
            border_path = args[0];

        efl.Loop loop = new efl.LoopConcrete();
        TestMain t = new TestMain(border_path);

        loop.begin();

        efl.All.Shutdown();
    }
}


