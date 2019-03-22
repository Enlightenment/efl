#pragma warning disable 1591

using System;
using System.Runtime.InteropServices;

using static Efl.UnsafeNativeMethods;

namespace Efl {

static class UnsafeNativeMethods {

    private delegate void init_func_delegate();
    [DllImport(efl.Libs.Ecore)] internal static extern void ecore_init();
    [DllImport(efl.Libs.Ecore)] internal static extern void ecore_shutdown();
    // dotnet loads libraries from DllImport with RTLD_LOCAL. Due to the
    // way evas modules are built with meson, currently they do not link directly
    // with libevas, leading to symbol not found errors when trying to open them.
    // The call to FunctionWrapper makes sure evas is loaded with RTLD_GLOBAL,
    // allowing the symbols to remain visible for the modules until the build
    // is sorted out.
    private static Efl.Eo.FunctionWrapper<init_func_delegate> _evas_init;
    [DllImport(efl.Libs.Evas)] internal static extern void evas_shutdown();
    static UnsafeNativeMethods() {
        _evas_init = new Efl.Eo.FunctionWrapper<init_func_delegate>("evas", "evas_init");
    }
    internal static void evas_init()
    {
        _evas_init.Value.Delegate();
    }
}

public static class All {
    private static bool InitializedUi = false;

    public static void Init(Efl.Csharp.Components components=Efl.Csharp.Components.Basic) {
        Eina.Config.Init();
        Efl.Eo.Config.Init();
        ecore_init();
        evas_init();
        eldbus.Config.Init();

        if (components == Efl.Csharp.Components.Ui) {
            Efl.Csharp.Ui.Config.Init();
            InitializedUi = true;
        }
    }

    /// <summary>Shutdowns all EFL subsystems.</summary>
    public static void Shutdown() {
        // Try to cleanup everything before actually shutting down.
        System.GC.Collect();
        System.GC.WaitForPendingFinalizers();

        if (InitializedUi)
            Efl.Csharp.Ui.Config.Shutdown();
        eldbus.Config.Shutdown();
        evas_shutdown();
        ecore_shutdown();
        Efl.Eo.Config.Shutdown();
        Eina.Config.Shutdown();
    }
}

}
