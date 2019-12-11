/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
using System;

public class Example
{
#if EFL_BETA
    private static double KMS_PER_MILE = 1.609344;

    private static double KmsToMiles(double kms)
    {
        return kms / KMS_PER_MILE;
    }
    private static double MilesToKms(double miles)
    {
        return miles * KMS_PER_MILE;
    }

    private static void ShowErrorPopup(Efl.Ui.Win win, string message)
    {
        Efl.Ui.AlertPopup popup = new Efl.Ui.AlertPopup(win);
        Efl.Ui.Textbox popup_text = new Efl.Ui.Textbox(popup);
        popup_text.SetText($"Error: {message}");
        popup.SetContent(popup_text);
        popup.SetVisible(true);
        popup.SetButton(Efl.Ui.AlertPopupButton.Positive, "Ok", null);
        popup.SetSize(new Eina.Size2D(150, 30));
        popup.ButtonClickedEvent += (object sender, Efl.Ui.AlertPopupButtonClickedEventArgs e) => {
            popup.SetParent(null);
            popup.Invalidate();
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
        Eina.Size2D size = new Eina.Size2D(W, H);

        Efl.All.Init(Efl.Components.Ui);

        Efl.Ui.Win win = new Efl.Ui.Win(null);
        win.SetText("C# Unit Converter");
        win.SetAutohide(true);

        Efl.Ui.BoxFlow box = new Efl.Ui.BoxFlow(win);
        box.SetDirection(Efl.Ui.Dir.Horizontal);

        Efl.Ui.BoxFlow miles_box = new Efl.Ui.BoxFlow(box);
        miles_box.SetDirection(Efl.Ui.Dir.Down);

        box.DoPack(miles_box);

        Efl.Ui.Textbox miles_label = new Efl.Ui.Textbox(miles_box);
        miles_label.SetText("Miles:");
        miles_label.SetSize(size);
        miles_label.SetVisible(true);

        Efl.Ui.Textbox miles_input = new Efl.Ui.Textbox(miles_box);
        miles_input.SetText("");
        miles_input.SetScrollable(true);
        miles_input.SetEditable(true);
        miles_input.SetSize(size);
        miles_input.SetVisible(true);

        Efl.Ui.Button miles_button = new Efl.Ui.Button(miles_box);
        miles_button.SetText("To Km");
        miles_button.SetSize(size);
        miles_button.SetVisible(true);

        miles_box.DoPack(miles_label);
        miles_box.DoPack(miles_input);
        miles_box.DoPack(miles_button);


        Efl.Ui.BoxFlow kms_box = new Efl.Ui.BoxFlow(box);
        kms_box.SetDirection(Efl.Ui.Dir.Down);

        box.DoPack(kms_box);

        Efl.Ui.Textbox kms_label = new Efl.Ui.Textbox(kms_box);
        kms_label.SetText("Kilometers:");
        kms_label.SetSize(size);
        kms_label.SetVisible(true);

        Efl.Ui.TextboxEditable kms_input = new Efl.Ui.TextboxEditable(kms_box);
        kms_input.SetText("");
        kms_input.SetScrollable(true);
        kms_input.SetSize(size);
        kms_input.SetVisible(true);

        Efl.Ui.Button kms_button = new Efl.Ui.Button(kms_box);
        kms_button.SetText("To Miles");
        kms_button.SetSize(size);
        kms_button.SetVisible(true);

        kms_box.DoPack(kms_label);
        kms_box.DoPack(kms_input);
        kms_box.DoPack(kms_button);

        ((Efl.Ui.Clickable)kms_button).ClickedEvent += (object sender, EventArgs e) => {
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

        ((Efl.Ui.Clickable)miles_button).ClickedEvent += (object sender, EventArgs e) => {
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

        box.SetPosition(new Eina.Position2D(20, 30));
        box.SetVisible(true);

        win.SetPosition(new Eina.Position2D(200, 200));

        win.SetSize(new Eina.Size2D(400, 120));
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
