
using System;
using System.Runtime.InteropServices;

public class EcoreEvas
{
    [DllImport("ecore_evas")] static extern void ecore_evas_init();
    // [DllImport("ecore_evas")] static extern IntPtr ecore_evas_new([MarshalAs(UnmanagedType.LPStr)] String engine_name, int x, int y, int w, int h
    //                                                               , [MarshalAs(UnmanagedType.LPStr)] String extra_options);
    [DllImport("ecore_evas")] static extern IntPtr ecore_evas_new(IntPtr engine_name, int x, int y, int w, int h
                                                                  , IntPtr extra_options);
    [DllImport("ecore_evas")] static extern IntPtr ecore_evas_get(IntPtr ecore_evas);
    [DllImport("ecore_evas")] static extern IntPtr ecore_evas_show(IntPtr ecore_evas);

    IntPtr handle;
    public EcoreEvas()
    {
        ecore_evas_init();
        handle = ecore_evas_new(IntPtr.Zero, 0, 0, 640, 480, IntPtr.Zero);
        if(handle == IntPtr.Zero)
            System.Console.WriteLine("Couldn't create a ecore evas");
        ecore_evas_show(handle);
    }

    public efl.canvas.Object canvas
    {
        get { return new efl.canvas.ObjectConcrete(ecore_evas_get(handle)); }
    }
    
}

