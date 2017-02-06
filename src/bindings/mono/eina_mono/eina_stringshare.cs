
using System;
using System.Runtime.InteropServices;


namespace eina {

public class Stringshare {
    [DllImport("eina")] private static extern System.IntPtr
        eina_stringshare_add_length(string str, System.UInt32 slen);
    [DllImport("eina")] private static extern System.IntPtr
        eina_stringshare_add(string str);


    System.IntPtr handle = System.IntPtr.Zero;

    public Stringshare() {
        handle = eina_stringshare_add("");
    }

}

}

