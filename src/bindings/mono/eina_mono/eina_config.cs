using System;
using System.Runtime.InteropServices;

namespace eina {

public class Config {
    [DllImport("eina")] private static extern int eina_init();
    [DllImport("eina")] private static extern int eina_shutdown();

    public static void Init() {
        if (eina_init() == 0)
            throw (new efl.EflException("Failed to initialize Eina"));

        // Initialize the submodules here
        eina.Log.Init();
        eina.Error.Init();
    }

    public static void Shutdown() {
        if (eina_shutdown() != 0)
            throw (new efl.EflException("Failed to shutdown all Eina modules"));
    }

}

/// <summary>
/// Wrapper class for pointers that need some cleanup afterwards
/// like strings.
/// </summary>
public class DisposableIntPtr : IDisposable {

    public IntPtr Handle { get; set; }
    private bool ShouldFree;
    private bool Disposed;

    /// <summary>Wraps a new ptr what will be freed based on the
    /// value of shouldFree</summary>
    public DisposableIntPtr(IntPtr ptr, bool shouldFree=false)
    {
        Handle = ptr;
        ShouldFree = shouldFree;
    }

    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    protected virtual void Dispose(bool disposing)
    {
        if (!Disposed && ShouldFree) {
            Marshal.FreeHGlobal(this.Handle);
        }
        Disposed = true;
    }

    ~DisposableIntPtr()
    {
        Dispose(false);
    }
}
}
