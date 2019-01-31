using System;
using System.Runtime.InteropServices;

namespace Efl { namespace Eo {

public partial class FunctionInterop
{
    [DllImport(efl.Libs.Libdl)]
    public static extern IntPtr dlsym(IntPtr handle, string symbol);
        
    public static IntPtr LoadFunctionPointer(IntPtr nativeLibraryHandle, string functionName)
    {
        Console.WriteLine("searching {0} in library {1}", nativeLibraryHandle, functionName);
        var s = FunctionInterop.dlsym(nativeLibraryHandle, functionName);
        Console.WriteLine("searching {0} in library {1}, result {2}", nativeLibraryHandle, functionName, s);
        return s;
    }
}


} }
