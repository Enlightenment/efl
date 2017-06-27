using System;
using System.Runtime.InteropServices;

using static efl.UnsafeNativeMethods;

namespace efl {

static class UnsafeNativeMethods {
    [DllImport("ecore")] public static extern void ecore_init();
    [DllImport("ecore")] public static extern void ecore_shutdown();
    [DllImport("evas")] public static extern void evas_init();
    [DllImport("evas")] public static extern void evas_shutdown();
}

public static class All {
    public static void Init() {
        eina.Config.Init();
        efl.eo.Config.Init();
        ecore_init();
        evas_init();
    }

    /// <summary>Shutdowns all EFL subsystems.</summary>
    public static void Shutdown() {
        // Try to cleanup everything before actually shutting down.
        System.GC.Collect();
        System.GC.WaitForPendingFinalizers();

        evas_shutdown();
        ecore_shutdown();
        efl.eo.Config.Shutdown();
        eina.Config.Shutdown();
    }
}

}
