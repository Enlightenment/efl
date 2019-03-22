using System;
using System.Runtime.InteropServices;
using System.Threading;

using static Efl.Csharp.Ui.UnsafeNativeMethods;

namespace Efl
{

namespace Csharp
{

namespace Ui
{

static class UnsafeNativeMethods
{
    [DllImport(efl.Libs.Elementary)] internal static extern int elm_init(int argc, IntPtr argv);
    [DllImport(efl.Libs.Elementary)] internal static extern int elm_init(int argc, [In] string[] argv);
    [DllImport(efl.Libs.Elementary)] internal static extern void elm_policy_set(int policy, int policy_detail);
    [DllImport(efl.Libs.Elementary)] internal static extern void elm_shutdown();
    [DllImport(efl.Libs.Elementary)] internal static extern void elm_run();
    [DllImport(efl.Libs.Elementary)] internal static extern void elm_exit();
}

/// <summary>Configuration of Elementary EFL UI Library.</summary>
internal static class Config
{

    /// <summary>Initializes the library.
    ///
    /// Also sets the default policy to <see cref="Elm.PolicyQuit.LastWindowHidden" />.</summary>
    internal static void Init(string[] commandLineArgs = null)
    {
#if WIN32 // Not a native define, we define it in our build system
        // Ecore_Win32 uses OleInitialize, which requires single thread apartments
        if (System.Threading.Thread.CurrentThread.GetApartmentState() != ApartmentState.STA)
        {
            throw new InvalidOperationException("UI Applications require STAThreadAttribute in Main()");
        }

#endif
        if (commandLineArgs == null)
            elm_init(0, IntPtr.Zero);
        else
            elm_init(commandLineArgs.Length, commandLineArgs);

        elm_policy_set((int)Elm.Policy.Quit, (int)Elm.PolicyQuit.LastWindowHidden);
    }

    /// <summary>Shuts down the library.</summary>
    internal static void Shutdown()
    {
        elm_shutdown();
    }

    /// <summary>Starts the Elementary main loop.</summary>
    internal static void Run()
    {
        elm_run();
    }

    /// <summary>Signals the main loop to exit.</summary>
    internal static void Exit()
    {
        elm_exit();
    }
}

}

}

}
