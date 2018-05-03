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
    private efl.canvas.IObject canvas;
    private efl.canvas.IRectangle bg;
    private evas.Text text;
    private evas.Image border;

    public TestMain(String border_file) {
        ecore_evas = new EcoreEvas();
        eina.Size2D size = new eina.Size2D();
        eina.Position2D position = new eina.Position2D();
        canvas = ecore_evas.canvas;
        canvas.SetVisible(true);

        bg = new efl.canvas.Rectangle(canvas);
        bg.SetColor(255, 255, 255, 255);
        position.X = 0;
        position.Y = 0;
        bg.SetPosition(position);
        size.W = WIDTH;
        size.H = HEIGHT;
        bg.SetSize(size);
        bg.SetVisible(true);
        bg.SetKeyFocus(true);

        bg.KeyDownEvt += On_KeyDown;

        text = new evas.Text(canvas);
        text.SetStyle(evas.Text_Style_Type.OutlineSoftShadow);

        text.SetColor(0, 0, 0, 255);
        text.SetGlowColor(255, 0, 0, 255);
        text.SetOutlineColor(0, 0, 255, 255);
        text.SetShadowColor(0, 255,255, 255);
        text.SetFont("Courier", 30);

        text.SetText("sample text");
        size.W = 3*WIDTH / 4;
        size.H = HEIGHT / 4;
        text.SetSize(size);
        position.X = WIDTH / 8;
        position.Y = 3 * HEIGHT / 8;
        text.SetPosition(position);
        text.SetVisible(true);

        efl.font.Size font_size = 0;
        String font = String.Empty;
        text.GetFont(out font, out font_size);
        Console.WriteLine("Adding text object with font {0} and size {1}", font, size);

        // setup border
        border = new evas.Image(canvas);
        border.SetFile(border_file, null);
        border.SetBorder(3, 3, 3, 3);
        border.SetBorderCenterFill(0);

        size.W = 3 * WIDTH / 4 + 3;
        size.H = HEIGHT / 4 + 3;
        border.SetSize(size);
        position.X = WIDTH / 8 - 3;
        position.Y = 3 * HEIGHT / 8 - 3;
        border.SetPosition(position);
        border.SetVisible(true);


    }

    private void On_KeyDown(object sender, efl.input.Interface.KeyDownEvt_Args e)
    {
        var key = e.arg.GetKey();

        if (key == "h") {
            Console.WriteLine(commands);
        } else if (key == "t") {
            evas.Text_Style_Type type = text.GetStyle();
            type = (evas.Text_Style_Type)(((int)type + 1) % 10); // 10 hardcoded from C example
            text.SetStyle(type);
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

        efl.Loop loop = new efl.Loop();
        TestMain t = new TestMain(border_path);

        loop.Begin();

        efl.All.Shutdown();
    }
}


