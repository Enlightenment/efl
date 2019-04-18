using System;
using System.Runtime.InteropServices;

namespace Efl
{

namespace Eo
{

public class partial NativeModule
{
   [DllImport(efl.Libs.Kernel32, CharSet = CharSet.Unicode, SetLastError = true)]
   public static extern IntPtr LoadLibrary(string libFilename);
}

}

}
