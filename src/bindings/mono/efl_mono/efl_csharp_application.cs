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
using System.Runtime.InteropServices;
using System.Threading;
using System.Diagnostics.CodeAnalysis;
using System.Collections.Generic;

using static Efl.UnsafeNativeMethods;

namespace Efl.Csharp
{

/// <summary>The components to be initialized.
/// <para>Since EFL 1.23.</para>
/// </summary>
[Flags]
public enum Components : Int32
{
    // Base flags.
    /// <summary>
    ///   Initialize nothing.
    /// <para>Since EFL 1.24.</para>
    /// </summary>
    None = 0x0,
    ///<summary>Basic components: Eina, Eo, Ecore, Evas and DBus.
    /// <para>Since EFL 1.24.</para>
    /// </summary>
    Basic = 0x1,
    /// <summary>
    ///   Elementary Widget toolkit: Elm.
    /// <para>it's depend of <see cref="Efl.Csharp.Components.Basic" />.</para>
    /// <para>Since EFL 1.24.</para>
    /// </summary>
    Ui = Basic | 0x2,

    // Combined flags.
    /// <summary>
    ///   Enable all components, equals to <see cref="Efl.Csharp.Components.Ui" />.
    /// <para>Since EFL 1.24.</para>
    /// </summary>
    All = int.MaxValue,
}

/// <summary>
/// This represents the entry point for the EFL framework
/// You can use this class to implement the 4 abstract methods which will then be called accordingly
/// All subsystems of efl are booted up correctly when the abstract methods of this class are called.
/// <para>Since EFL 1.23.</para>
/// </summary>
/// <remarks>
/// Calls to efl outside those efl-callbacks or outside the mainloop are not allowed and will lead to issues
/// </remarks>
///
/// <example>
/// UserApp is the class that implements the Application abstract
/// <code>
/// public static void Main()
/// {
///     UserApp editor = new UserApp();
///     editor.Launch(editor);
/// }
/// </code>
/// </example>
public abstract class Application
{
    //the initializied components
    private static Components initComponents = Components.All;

    //what follows are 3 private functions to boot up the internals of efl
    private static void Init(Efl.Csharp.Components components = Components.All)
    {
        if (components == Components.None)
        {
            return;
        }

        initComponents = components;

        if ((initComponents & Components.Basic) == Components.Basic)
        {
            Eina.Config.Init();
            Efl.Eo.Config.Init();
            ecore_init();
            ecore_init_ex(0, IntPtr.Zero);
            evas_init();
            eldbus.Config.Init();
        }

        if ((initComponents & Components.Ui) == Components.Ui)
        {
#if WIN32
            // TODO Support elm command line arguments
            // Not a native define, we define it in our build system
            // Ecore_Win32 uses OleInitialize, which requires single thread apartments
            if (System.Threading.Thread.CurrentThread.GetApartmentState()
                != ApartmentState.STA)
            {
                throw new InvalidOperationException("UI Applications require STAThreadAttribute in Main()");
            }
#endif
            elm_init(0, IntPtr.Zero);

            Efl.Ui.Win.ExitOnAllWindowsClosed = new Eina.Value(0);
        }
    }

    private static void Shutdown()
    {
        // Try to cleanup everything before actually shutting down.
        System.GC.Collect();
        System.GC.WaitForPendingFinalizers();

        if (initComponents == Components.None)
        {
            return;
        }

        if ((initComponents & Components.Ui) == Components.Ui)
        {
            elm_shutdown();
        }

        if ((initComponents & Components.Basic) == Components.Basic)
        {
            eldbus.Config.Shutdown();
            evas_shutdown();
            ecore_shutdown();
            Efl.Eo.Config.Shutdown();
            Eina.Config.Shutdown();
        }
    }

    /// <summary>
    /// Called when the application is started. Arguments from the command line are passed here.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="args">Arguments passed from command line.</param>
    protected abstract void OnInitialize(string[] args);

    /// <summary>
    /// Arguments are passed here, Additional calls to this function may occure,
    /// but then the initialization flag is set to false.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <remarks>
    /// When Initialize is true then OnInitialize is also called
    /// </remarks>
    /// <param name="args">Argsuments passed from command line</param>
    protected virtual void OnArguments(Efl.LoopArguments args)
    {
    }

    /// <summary>
    /// Called when the application is not going to be displayed, or is not used by a user for some time.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    protected virtual void OnPause()
    {
    }

    /// <summary>
    /// Called before an application is used again after a call to OnPause().
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    protected virtual void OnResume()
    {
    }

    /// <summary>
    /// Called before starting the shutdown of the application.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    protected virtual void OnTerminate()
    {
    }

    /// <summary>
    /// This function initializices everything in EFL and runs your application.
    /// This call will result in a call to OnInitialize(), which you application should override.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="components">The <see cref="Efl.Csharp.Components" /> to run the application.</param>
    public void Launch(Efl.Csharp.Components components = Components.All)
    {
        Init(components);
        Efl.App app = Efl.App.AppMain;
#if EFL_BETA
        var command_line = new List<Eina.Stringshare>();
        //command_line.Add(List.ConvertAll(Environment.GetCommandLineArgs(), s => (Eina.Stringshare)s));
        //command_line.AddRange(Environment.GetCommandLineArgs());
        app.SetCommandArray(command_line);
#endif
        app.ArgumentsEvent += (object sender, LoopArgumentsEventArgs evt) =>
        {
            if (evt.Arg.Initialization)
            {
                var evtArgv = evt.Arg.Argv;
                int n = evtArgv.Count;
                var argv = new string[n];
                for (int i = 0; i < n; ++i)
                {
                    argv[i] = evtArgv[i];
                }

                OnInitialize(argv);
            }

            OnArguments(evt.Arg);
        };
        app.PauseEvent += (object sender, EventArgs e) =>
        {
            OnPause();
        };
        app.ResumeEvent += (object sender, EventArgs e) =>
        {
            OnResume();
        };
        app.TerminateEvent += (object sender, EventArgs e) =>
        {
            OnTerminate();
        };
        app.Begin();
        Shutdown();
    }
}

}
