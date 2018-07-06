using System;

public class Example
{
    public static efl.ui.IButton CreateButton(efl.IObject parent,
                                             string text,
                                             int w, int h,
                                             EventHandler callback) {
        efl.ui.IButton button = new efl.ui.Button(parent);
        button.SetText(text);
        button.SetSize(new eina.Size2D(w, h));

        button.ClickedEvt += callback;

        return button;
    }

    public static void Formatter(eina.Strbuf buf, eina.Value val){
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

        efl.All.Init(efl.Components.Ui);

        efl.ui.Win win = new efl.ui.Win(null);
        win.SetText("Hello, C#!!");
        win.SetAutohide(true);

        efl.ui.Box_Flow box = new efl.ui.Box_Flow(win);

        efl.ui.IButton button = CreateButton(box, "Click to exit", 120, 30,
                (object sender, EventArgs e) => {
                    efl.ui.Config.Exit();
                });

        box.DoPack(button);

        efl.ui.Progressbar bar = new efl.ui.Progressbar(box);
        bar.SetSize(new eina.Size2D(W, H));
        bar.SetFormatCb(Formatter);

        efl.ui.ISlider slider = new efl.ui.Slider(box);
        slider.SetSize(new eina.Size2D(W, H));

        slider.ChangedEvt += (object sender, EventArgs e) => {
            bar.SetRangeValue(slider.GetRangeValue());
        };

        box.DoPack(bar);
        box.DoPack(slider);

        button.SetVisible(true);
        box.SetVisible(true);

        win.SetSize(new eina.Size2D(W, 3 * H));
        win.SetVisible(true);

        efl.ui.Config.Run();

        efl.All.Shutdown();
    }

}
