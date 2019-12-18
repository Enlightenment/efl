using System;
using System.Runtime.InteropServices;

namespace Eina
{

/// <summary>
/// Class to deal with native Environment variables.
///
/// <para>To be used in place of <see cref="System.Environment" /> methods when
/// accessing the native environment directly.</para>
/// <para>Since EFL 1.24.</para>
/// </summary>
internal static class Environment
{
    /// <summary>
    /// Returns the value of the environment variable named <c>name</c>.
    ///
    /// <para>Since EFL 1.24.</para>
    /// </summary>
    /// <param name="name">The name of the variable to be retrieved</param>
    /// <returns>The value of the variable. <c>null</c> if not set.</returns>
    public static string GetEnv(string name)
    {
        return Eina.NativeCustomExportFunctions.efl_mono_native_getenv(name);
    }

    /// <summary>
    /// Sets a native environment variable.
    ///
    /// <para>Since EFL 1.24.</para>
    /// </summary>
    /// <param name="name">The name of the variable</param>
    /// <param name="value">The value to be set.</param>
    /// <param name="overwrite"><c>true</c> if an existing variable must be overwritten.</param>
    public static void SetEnv(string name, string value, bool overwrite=true)
    {
        Eina.Error error = Eina.NativeCustomExportFunctions.efl_mono_native_setenv(name, value, overwrite ? 1 : 0);
        Eina.Error.Raise(error);
    }
}

internal static partial class NativeCustomExportFunctions
{
    [DllImport(efl.Libs.CustomExports, CharSet=CharSet.Ansi)]
    [return: MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringKeepOwnershipMarshaler))]
    public static extern string efl_mono_native_getenv(string name);

    [DllImport(efl.Libs.CustomExports, CharSet=CharSet.Ansi)]
    public static extern Eina.Error efl_mono_native_setenv(string name, string value, int overwrite);
}

}
