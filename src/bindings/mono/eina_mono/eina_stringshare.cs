
using System;
using System.Runtime.InteropServices;


namespace eina {

public class Stringshare {
    [DllImport("eina")] public static extern System.IntPtr
        eina_stringshare_add_length(string str, System.UInt32 slen);
    [DllImport("eina")] public static extern System.IntPtr
        eina_stringshare_add(string str);
    [DllImport("eina")] public static extern void
        eina_stringshare_del(System.IntPtr str);
}

}

