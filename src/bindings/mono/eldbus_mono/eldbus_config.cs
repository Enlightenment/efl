#pragma warning disable 1591

using System;
using System.Runtime.InteropServices;

namespace eldbus
{

/// <summary>Initializes Eldbus.
/// <para>Since EFL 1.23.</para>
/// </summary>
public static class Config
{
    [DllImport(efl.Libs.Eldbus)] private static extern int eldbus_init();
    [DllImport(efl.Libs.Eldbus)] private static extern int eldbus_shutdown();

    /// <summary>
    /// Initialization of the eldbus.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public static void Init()
    {
        if (eldbus_init() == 0)
        {
            throw new Efl.EflException("Failed to initialize Eldbus");
        }
    }

    /// <summary>
    /// Shutdown the eldbus.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public static void Shutdown()
    {
        eldbus_shutdown();
    }

}

}
