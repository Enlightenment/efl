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
#pragma warning disable 1591

using System;
using System.Runtime.InteropServices;
using System.Threading;

using static Efl.UnsafeNativeMethods;

namespace Efl
{

static class UnsafeNativeMethods
{
    private delegate void init_func_delegate();
    [DllImport(efl.Libs.Ecore)] internal static extern void ecore_init();
    [DllImport(efl.Libs.Ecore)] internal static extern void ecore_init_ex(int argc, IntPtr argv);
    [DllImport(efl.Libs.Ecore)] internal static extern void ecore_shutdown();
    // dotnet loads libraries from DllImport with RTLD_LOCAL. Due to the
    // way evas modules are built with meson, currently they do not link directly
    // with libevas, leading to symbol not found errors when trying to open them.
    // The call to FunctionWrapper makes sure evas is loaded with RTLD_GLOBAL,
    // allowing the symbols to remain visible for the modules until the build
    // is sorted out.
    private static Efl.Eo.FunctionWrapper<init_func_delegate> _evas_init;
    [DllImport(efl.Libs.Evas)] internal static extern void evas_shutdown();
    [DllImport(efl.Libs.Elementary)] internal static extern int elm_init(int argc, IntPtr argv);
    [DllImport(efl.Libs.Elementary)] internal static extern void elm_shutdown();
    [DllImport(efl.Libs.Elementary)] internal static extern void elm_run();
    [DllImport(efl.Libs.Elementary)] internal static extern void elm_exit();

    static UnsafeNativeMethods()
    {
        _evas_init = new Efl.Eo.FunctionWrapper<init_func_delegate>(efl.Libs.Evas, "evas_init");
    }

    internal static void evas_init()
    {
        _evas_init.Value.Delegate();
    }
}

/// <summary>Wrapper around the initialization functions of all modules.
/// <para>Since EFL 1.23.</para>
/// </summary>
public static class All
{
    private static Efl.Csharp.Components initComponents = Efl.Csharp.Components.Basic;

    /// <summary>
    ///   If the main loop was initialized.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public static bool MainLoopInitialized {
        get;
        private set;
    }

    internal static readonly object InitLock = new object();

    /// <summary>
    ///   Initialize the Efl.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="components">The <see cref="Efl.Csharp.Components" /> that initialize the Efl.</param>
    public static void Init(Efl.Csharp.Components components = Efl.Csharp.Components.Basic)
    {
        if (components == Efl.Csharp.Components.None)
        {
            return;
        }

        initComponents = components;

        if ((initComponents & Efl.Csharp.Components.Basic)
            == Efl.Csharp.Components.Basic)
        {
            Eina.Config.Init();
            Efl.Eo.Config.Init();
            ecore_init();
            ecore_init_ex(0, IntPtr.Zero);
            evas_init();
            eldbus.Config.Init();
        }

        if ((initComponents & Efl.Csharp.Components.Ui)
            == Efl.Csharp.Components.Ui)
        {
            Efl.Ui.Config.Init();
        }
        Monitor.Enter(InitLock);
        MainLoopInitialized = true;
        Monitor.Exit(InitLock);
    }

    /// <summary>Shutdowns all EFL subsystems.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public static void Shutdown()
    {
        // Try to cleanup everything before actually shutting down.
        Eina.Log.Debug("Calling GC before shutdown");
        for (int i = 0; i < 3; i++)
        {
            System.GC.Collect();
            System.GC.WaitForPendingFinalizers();
            Efl.App.AppMain.Iterate();
        }

        Monitor.Enter(InitLock);
        MainLoopInitialized = false;
        Monitor.Exit(InitLock);

        if (initComponents == Efl.Csharp.Components.None)
        {
            return;
        }

        if ((initComponents & Efl.Csharp.Components.Ui)
            == Efl.Csharp.Components.Ui)
        {
            Eina.Log.Debug("Shutting down Elementary");
            Efl.Ui.Config.Shutdown();
        }

        if ((initComponents & Efl.Csharp.Components.Basic)
            == Efl.Csharp.Components.Basic)
        {
            Eina.Log.Debug("Shutting down Eldbus");
            eldbus.Config.Shutdown();
            Eina.Log.Debug("Shutting down Evas");
            evas_shutdown();
            Eina.Log.Debug("Shutting down Ecore");
            ecore_shutdown();
            Eina.Log.Debug("Shutting down Eo");
            Efl.Eo.Config.Shutdown();
            Eina.Log.Debug("Shutting down Eina");
            Eina.Config.Shutdown();
        }
    }
}

// Placeholder. Will move to elm_config.cs later
namespace Ui
{

/// <summary>Initialization and shutdown of the UI libraries.
/// <para>Since EFL 1.23.</para>
/// </summary>
public static class Config
{
    /// <summary>
    /// Initialize the configuration of Elm.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public static void Init()
    {
        // TODO Support elm command line arguments
#if WIN32 // Not a native define, we define it in our build system
        // Ecore_Win32 uses OleInitialize, which requires single thread apartments
        if (System.Threading.Thread.CurrentThread.GetApartmentState() != ApartmentState.STA)
        {
            throw new InvalidOperationException("UI Applications require STAThreadAttribute in Main()");
        }
#endif
        elm_init(0, IntPtr.Zero);

        Efl.Ui.Win.ExitOnAllWindowsClosed = new Eina.Value(0);
    }

    /// <summary>
    ///   Shutdown Elm systems.
    /// <para>Since EFL 1.24.</para>
    /// </summary>
    public static void Shutdown()
    {
        elm_shutdown();
    }

    /// <summary>
    ///   Run Elm system.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public static void Run()
    {
        elm_run();
    }

    /// <summary>
    ///   Exit Elm.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public static void Exit()
    {
        elm_exit();
    }
}

}

}
