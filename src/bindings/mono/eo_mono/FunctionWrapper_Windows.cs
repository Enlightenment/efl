using System;
using System.Runtime.InteropServices;

namespace Efl { namespace Eo {

public partial class FunctionInterop
{
    [DllImport(efl.Libs.Libdl)]
    public static extern IntPtr GetProcAddress(IntPtr handle, string symbol);

    private static IntPtr LoadFunctionPointer(IntPtr nativeLibraryHandle, string functionName)
        => FunctionInterop.GetProcAddress(nativeLibraryHandle, functionName);
}

} }
