using System;

public class Example
{
#if EFL_BETA
    public static Efl.Ui.Button CreateButton(Efl.Object parent,
                                             string text,
                                             int w, int h,
                                             EventHandler callback) {
        Efl.Ui.Button button = new Efl.Ui.Button(parent);
        button.SetText(text);
        button.SetSize(new Eina.Size2D(w, h));

        ((Efl.Ui.Clickable)button).ClickedEvt += callback;

        return button;
    }

    public static void Formatter(Eina.Strbuf buf, Eina.Value val){
        double ratio;
        if (val.Get(out ratio)) {
            buf.Append($"{(int)(ratio*100)}%");
        } else {
            buf.Append("Error");
        }
    }


#if WIN32 // Passed to the C# compiler with -define:WIN32
    // Mono on Windows by default uses multi-thread apartments for COM stuff while
    // OLE - used by ecore win32 DnD requires single threading for COM.
    [STAThreadAttribute()]
#endif
    public static void Main() {
        int W = 120;
        int H = 30;

        Efl.All.Init(Efl.Components.Ui);

        Efl.Ui.Win win = new Efl.Ui.Win(null);
        win.SetText("Hello, C#!!");
        win.SetAutohide(true);

        Efl.Ui.BoxFlow box = new Efl.Ui.BoxFlow(win);

        Efl.Ui.Button button = CreateButton(box, "Click to exit", 120, 30,
                (object sender, EventArgs e) => {
                    Efl.Ui.Config.Exit();
                });

        box.DoPack(button);

        Efl.Ui.Progressbar bar = new Efl.Ui.Progressbar(box);
        bar.SetSize(new Eina.Size2D(W, H));
        bar.SetFormatCb(Formatter);

        Efl.Ui.Slider slider = new Efl.Ui.Slider(box);
        slider.SetSize(new Eina.Size2D(W, H));

        slider.ChangedEvt += (object sender, EventArgs e) => {
            bar.SetRangeValue(slider.GetRangeValue());
        };

        box.DoPack(bar);
        box.DoPack(slider);

        button.SetVisible(true);
        box.SetVisible(true);

        win.SetSize(new Eina.Size2D(W, 3 * H));
        win.SetVisible(true);

        Efl.Ui.Config.Run();

        Efl.All.Shutdown();
    }
#else
    public static void Main()
    {
    }
#endif

}
