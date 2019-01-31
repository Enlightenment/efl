using System;
using System.Runtime.InteropServices;

namespace Efl { namespace Eo {

public partial class NativeModule
{
    public const int RTLD_NOW = 0x002;

    [DllImport(efl.Libs.Libdl)]
    public static extern IntPtr dlopen(string fileName, int flag);
    [DllImport(efl.Libs.Libdl)]
    public static extern int dlclose(IntPtr handle);

    public static void UnloadLibrary(IntPtr handle)
    {
        dlclose(handle);
    }
    
    public static IntPtr LoadLibrary(string filename)
    {
        Console.WriteLine("Loading library {0}", filename);
        var r = dlopen(filename, RTLD_NOW);
        if (r == IntPtr.Zero)
        {
            r = dlopen("lib" + filename, RTLD_NOW);
            if (r == IntPtr.Zero)
            {
                r = dlopen(filename + ".so", RTLD_NOW);
                if (r == IntPtr.Zero)
                {
                    r = dlopen("lib" + filename + ".so", RTLD_NOW);
                }
            }
        }
        return r;
    }
}




} }
