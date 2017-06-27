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
}
