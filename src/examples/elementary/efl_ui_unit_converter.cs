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
        efl.ui.Popup_Alert popup = new efl.ui.Popup_AlertConcrete(win);
        efl.ui.Text popup_text = new efl.ui.TextConcrete(popup);
        popup_text.SetText($"Error: {message}");
        popup.SetContent(popup_text);
        popup.SetVisible(true);
        popup.SetButton(efl.ui.popup_alert.Button.Positive, "Ok");
        popup.SetSize(150, 30);
        popup.BUTTON_CLICKED += (object sender, efl.ui.BUTTON_CLICKED_Args e) => {
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
        efl.All.Init(efl.Components.Ui);

        efl.ui.Win win = new efl.ui.WinConcrete(null);
        win.SetText("C# Unit Converter");
        win.SetAutohide(true);

        efl.ui.Box_Flow box = new efl.ui.Box_FlowConcrete(win);
        box.SetDirection(efl.ui.Dir.Horizontal);

        efl.ui.Box_Flow miles_box = new efl.ui.Box_FlowConcrete(box);
        miles_box.SetDirection(efl.ui.Dir.Down);

        box.Pack(miles_box);

        efl.ui.Text miles_label = new efl.ui.TextConcrete(miles_box);
        miles_label.SetText("Miles:");
        miles_label.SetSize(120, 30);
        miles_label.SetVisible(true);

        efl.ui.text.Editable miles_input = new efl.ui.text.EditableConcrete(miles_box);
        miles_input.SetText("");
        miles_input.SetScrollable(true);
        miles_input.SetSize(120, 30);
        miles_input.SetVisible(true);

        efl.ui.Button miles_button = new efl.ui.ButtonConcrete(miles_box);
        miles_button.SetText("To Km");
        miles_button.SetSize(120, 30);
        miles_button.SetVisible(true);

        miles_box.Pack(miles_label);
        miles_box.Pack(miles_input);
        miles_box.Pack(miles_button);


        efl.ui.Box_Flow kms_box = new efl.ui.Box_FlowConcrete(box);
        kms_box.SetDirection(efl.ui.Dir.Down);

        box.Pack(kms_box);

        efl.ui.Text kms_label = new efl.ui.TextConcrete(kms_box);
        kms_label.SetText("Kilometers:");
        kms_label.SetSize(120, 30);
        kms_label.SetVisible(true);

        efl.ui.text.Editable kms_input = new efl.ui.text.EditableConcrete(kms_box);
        kms_input.SetText("");
        kms_input.SetScrollable(true);
        kms_input.SetSize(120, 30);
        kms_input.SetVisible(true);

        efl.ui.Button kms_button = new efl.ui.ButtonConcrete(kms_box);
        kms_button.SetText("To Miles");
        kms_button.SetSize(120, 30);
        kms_button.SetVisible(true);

        kms_box.Pack(kms_label);
        kms_box.Pack(kms_input);
        kms_box.Pack(kms_button);

        kms_button.CLICKED += (object sender, EventArgs e) => {
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

        miles_button.CLICKED += (object sender, EventArgs e) => {
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

        eina.Position2D pos;

        pos.X = 20;
        pos.Y = 30;
        box.SetPosition(pos);
        box.SetVisible(true);

        pos.X = 200;
        pos.Y = 200;
        win.SetPosition(pos);

        win.SetSize(400,120);
        win.SetVisible(true);

        efl.ui.Config.Run();

        efl.All.Shutdown();
    }

}
