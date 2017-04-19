using System;
using System.Runtime.InteropServices;

using static eina.NativeCustomExportFunctions;

namespace eina {

public static class NativeCustomExportFunctions
{
    [DllImport("eflcustomexportsmono")] public static extern void
        efl_mono_native_free(IntPtr ptr);
    [DllImport("eflcustomexportsmono")] public static extern IntPtr
        efl_mono_native_alloc_copy(IntPtr val, uint size);
    [DllImport("eflcustomexportsmono")] public static extern IntPtr
        efl_mono_native_strdup(string str);
}

[StructLayout(LayoutKind.Sequential)]
public struct ConvertWrapper<T>
{
    public T Val {get;set;}
}

public class TypeTag<T> {}

public static class ElementConvert
{
    // //
    // Native alloc
    //
    public static IntPtr ManagedToNativeAlloc<T>(T man)
    {
        GCHandle pinnedData = GCHandle.Alloc(man, GCHandleType.Pinned);
        IntPtr ptr = pinnedData.AddrOfPinnedObject();
        IntPtr nat = efl_mono_native_alloc_copy(ptr, (uint)(Marshal.SizeOf<T>()));
        pinnedData.Free();
        return nat;
    }

    public static IntPtr ManagedToNativeAlloc(string man)
    {
        return efl_mono_native_strdup(man);
    }

    // //
    // Native free
    //
    public static void NativeFree<T>(IntPtr nat)
    {
        efl_mono_native_free(nat);
    }

    // //
    // Managed convert
    //
    public static T NativeToManaged<T>(IntPtr nat, eina.TypeTag<T> t)
    {
        var w = Marshal.PtrToStructure<eina.ConvertWrapper<T> >(nat);
        return w.Val;
    }

    public static string NativeToManaged(IntPtr nat, eina.TypeTag<string> t)
    {
        if (nat == IntPtr.Zero)
            return null;
        return Marshal.PtrToStringAuto(nat);
    }
}

}
