using System;

public class Example
{

    private static double KMS_PER_MILE = 1.609344;

    private static double KmsToMiles(double kms)
    {
        return kms / KMS_PER_MILE;
    }
    private static double MilesToKms(double miles)
    {
        return miles * KMS_PER_MILE;
    }

    private static void ShowErrorPopup(efl.ui.Win win, string message)
    {
        efl.ui.IPopup_Alert popup = new efl.ui.Popup_Alert(win);
        efl.ui.Text popup_text = new efl.ui.Text(popup);
        popup_text.SetText($"Error: {message}");
        popup.SetContent(popup_text);
        popup.SetVisible(true);
        popup.SetButton(efl.ui.Popup_Alert_Button.Positive, "Ok");
        popup.SetSize(new eina.Size2D(150, 30));
        popup.ButtonClickedEvt += (object sender, efl.ui.Popup_Alert.ButtonClickedEvt_Args e) => {
            popup.SetParent(null);
            popup.Del();
        };
    }

#if WIN32 // Passed to the C# compiler with -define:WIN32
    // Mono on Windows by default uses multi-thread apartments for COM stuff while
    // OLE - used by ecore win32 DnD requires single threading for COM.
    [STAThreadAttribute()]
#endif
    public static void Main() {
        int W = 120;
        int H = 30;
        eina.Size2D size = new eina.Size2D(W, H);

        efl.All.Init(efl.Components.Ui);

        efl.ui.Win win = new efl.ui.Win(null);
        win.SetText("C# Unit Converter");
        win.SetAutohide(true);

        efl.ui.Box_Flow box = new efl.ui.Box_Flow(win);
        box.SetDirection(efl.ui.Dir.Horizontal);

        efl.ui.Box_Flow miles_box = new efl.ui.Box_Flow(box);
        miles_box.SetDirection(efl.ui.Dir.Down);

        box.DoPack(miles_box);

        efl.ui.Text miles_label = new efl.ui.Text(miles_box);
        miles_label.SetText("Miles:");
        miles_label.SetSize(size);
        miles_label.SetVisible(true);

        efl.ui.Text_Editable miles_input = new efl.ui.Text_Editable(miles_box);
        miles_input.SetText("");
        miles_input.SetScrollable(true);
        miles_input.SetSize(size);
        miles_input.SetVisible(true);

        efl.ui.IButton miles_button = new efl.ui.Button(miles_box);
        miles_button.SetText("To Km");
        miles_button.SetSize(size);
        miles_button.SetVisible(true);

        miles_box.DoPack(miles_label);
        miles_box.DoPack(miles_input);
        miles_box.DoPack(miles_button);


        efl.ui.Box_Flow kms_box = new efl.ui.Box_Flow(box);
        kms_box.SetDirection(efl.ui.Dir.Down);

        box.DoPack(kms_box);

        efl.ui.Text kms_label = new efl.ui.Text(kms_box);
        kms_label.SetText("Kilometers:");
        kms_label.SetSize(size);
        kms_label.SetVisible(true);

        efl.ui.Text_Editable kms_input = new efl.ui.Text_Editable(kms_box);
        kms_input.SetText("");
        kms_input.SetScrollable(true);
        kms_input.SetSize(size);
        kms_input.SetVisible(true);

        efl.ui.IButton kms_button = new efl.ui.Button(kms_box);
        kms_button.SetText("To Miles");
        kms_button.SetSize(size);
        kms_button.SetVisible(true);

        kms_box.DoPack(kms_label);
        kms_box.DoPack(kms_input);
        kms_box.DoPack(kms_button);

        kms_button.ClickedEvt += (object sender, EventArgs e) => {
            try
            {
                string text = kms_input.GetText();
                Console.WriteLine("Text is [{0}]", text);
                double val = double.Parse(text);
                miles_input.SetText(String.Format("{0:f3}", KmsToMiles(val)));
                kms_input.SetFocus(true);
            }
            catch (FormatException ex)
            {
                Console.WriteLine("Exception {0} caught", ex);
                ShowErrorPopup(win, "Invalid number");
            }
        };

        miles_button.ClickedEvt += (object sender, EventArgs e) => {
            try
            {
                string text = miles_input.GetText();
                Console.WriteLine("Text is [{0}]", text);
                double val = double.Parse(text);
                kms_input.SetText(String.Format("{0:f3}", MilesToKms(val)));
                miles_input.SetFocus(true);
            }
            catch (FormatException ex)
            {
                Console.WriteLine("Exception {0} cautght", ex);
                ShowErrorPopup(win, "Invalid number");
            }
        };

        kms_box.SetVisible(true);
        miles_box.SetVisible(true);

        box.SetPosition(new eina.Position2D(20, 30));
        box.SetVisible(true);

        win.SetPosition(new eina.Position2D(200, 200));

        win.SetSize(new eina.Size2D(400, 120));
        win.SetVisible(true);

        efl.ui.Config.Run();

        efl.All.Shutdown();
    }

}
