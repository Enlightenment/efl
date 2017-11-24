using System;

public class Example
{
    public static efl.ui.Button CreateButton(efl.Object parent,
                                             string text,
                                             int w, int h,
                                             EventHandler callback) {
        efl.ui.Button button = new efl.ui.ButtonConcrete(parent);
        button.SetText(text);
        button.SetSize(w, h);

        button.CLICKED += callback;

        return button;
    }

#if WIN32 // Passed to the C# compiler with -define:WIN32
    // Mono on Windows by default uses multi-thread apartments for COM stuff while
    // OLE - used by ecore win32 DnD requires single threading for COM.
    [STAThreadAttribute()]
#endif
    public static void Main() {
        efl.All.Init(efl.Components.Ui);

        efl.ui.Win win = new efl.ui.WinConcrete(null);
        win.SetText("Hello, C#!!");
        win.SetAutohide(true);

        efl.ui.Box_Flow box = new efl.ui.Box_FlowConcrete(win);

        efl.ui.Button button = CreateButton(box, "Click to exit", 120, 30,
                (object sender, EventArgs e) => {
                    efl.ui.Config.Exit();
                });

        box.Pack(button);

        efl.ui.Progressbar bar = new efl.ui.ProgressbarConcrete(box);
        bar.SetSize(120, 30);

        efl.ui.Slider slider = new efl.ui.SliderConcrete(box);
        slider.SetSize(120, 30);

        slider.CHANGED += (object sender, EventArgs e) => {
            bar.SetRangeValue(slider.GetRangeValue());
        };

        box.Pack(bar);
        box.Pack(slider);

        button.SetVisible(true);
        box.SetVisible(true);

        win.SetSize(120, 90);
        win.SetVisible(true);

        efl.ui.Config.Run();

        efl.All.Shutdown();
    }

}
