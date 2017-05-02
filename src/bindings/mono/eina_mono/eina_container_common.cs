using System;
using System.Runtime.InteropServices;

using static eina.NativeCustomExportFunctions;

namespace eina {

public enum ElementType { NumericType, StringType, ObjectType };


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

public static class ElementFunctions
{
    public static bool IsEflObject(System.Type type)
    {
        return typeof(efl.eo.IWrapper).IsAssignableFrom(type);
    }

    public static bool IsString(System.Type type)
    {
        return type == typeof(string);
    }

    public static eina.ElementType GetElementTypeCode(System.Type type)
    {
        if (IsEflObject(type))
            return ElementType.ObjectType;
        else if (type == typeof(string))
            return ElementType.StringType;
        else
            return ElementType.NumericType;
    }

    // //
    // Native alloc
    //
    public static IntPtr ManagedToNativeAllocEflObject<T>(T man)
    {
        IntPtr h = ((efl.eo.IWrapper)man).raw_handle;
        if (h == IntPtr.Zero)
            return h;
        return efl.eo.Globals.efl_ref(h);
    }

    public static IntPtr ManagedToNativeAllocOther<T>(T man)
    {
        GCHandle pinnedData = GCHandle.Alloc(man, GCHandleType.Pinned);
        IntPtr ptr = pinnedData.AddrOfPinnedObject();
        IntPtr nat = efl_mono_native_alloc_copy(ptr, (uint)(Marshal.SizeOf<T>()));
        pinnedData.Free();
        return nat;
    }

    public static IntPtr ManagedToNativeAllocString(string man)
    {
        return efl_mono_native_strdup(man);
    }

    public static IntPtr ManagedToNativeAlloc<T>(T man)
    {
        if (IsEflObject(typeof(T)))
            return ManagedToNativeAllocEflObject(man);
        else
            return ManagedToNativeAllocOther(man);
    }

    public static IntPtr ManagedToNativeAlloc(efl.eo.IWrapper man)
    {
        return ManagedToNativeAllocEflObject(man);
    }

    public static IntPtr ManagedToNativeAlloc(string man)
    {
        return ManagedToNativeAllocString(man);
    }

    public static IntPtr ManagedToNativeAlloc<T>(T man, ElementType t)
    {
        if (t == ElementType.ObjectType)
            return ManagedToNativeAllocEflObject(man);
        else
            return ManagedToNativeAllocOther(man);
    }

    // //
    // Native free
    //
    public static void NativeFreeString(IntPtr nat)
    {
        efl_mono_native_free(nat);
    }

    public static void NativeFree<T>(IntPtr nat)
    {
        if (IsEflObject(typeof(T)))
            efl.eo.Globals.efl_unref(nat);
        else
            efl_mono_native_free(nat);
    }

    public static void NativeFree(IntPtr nat, ElementType t)
    {
        if (t == ElementType.ObjectType)
            efl.eo.Globals.efl_unref(nat);
        else
            efl_mono_native_free(nat);
    }

    // //
    // Managed convert
    //
    public static T NativeToManagedEflObject<T>(IntPtr nat)
    {
        return (T) Activator.CreateInstance(typeof(T), nat, false); // TODO: implement ownership!!!
    }

    public static T NativeToManagedOther<T>(IntPtr nat)
    {
        var w = Marshal.PtrToStructure<eina.ConvertWrapper<T> >(nat);
        return w.Val;
    }

    public static string NativeToManagedString(IntPtr nat)
    {
        if (nat == IntPtr.Zero)
            return null;
        return Marshal.PtrToStringAuto(nat);
    }

    public static T NativeToManaged<T>(IntPtr nat)
    {
        if (IsEflObject(typeof(T)))
            return NativeToManagedEflObject<T>(nat);
        else
            return NativeToManagedOther<T>(nat);
    }

    public static T NativeToManaged<T>(IntPtr nat, ElementType t)
    {
        if (t == ElementType.ObjectType)
            return NativeToManagedEflObject<T>(nat);
        else
            return NativeToManagedOther<T>(nat);
    }
}

}
