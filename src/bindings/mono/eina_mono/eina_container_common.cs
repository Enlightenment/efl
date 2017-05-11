using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;

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

public interface BaseElementTraits<T>
{
    IntPtr ManagedToNativeAlloc(T man);
    void NativeFree(IntPtr nat);
    T NativeToManaged(IntPtr nat);
}

public class StringElementTraits<T> : BaseElementTraits<T>
{
    public IntPtr ManagedToNativeAlloc(T man)
    {
        return efl_mono_native_strdup((string)(object)man);
    }

    public void NativeFree(IntPtr nat)
    {
        efl_mono_native_free(nat);
    }

    public T NativeToManaged(IntPtr nat)
    {
        if (nat == IntPtr.Zero)
            return default(T);
        return (T)(object)Marshal.PtrToStringAuto(nat);
    }
}

public class EflObjectElementTraits<T> : BaseElementTraits<T>
{
    private System.Type concreteType;

    public EflObjectElementTraits(System.Type concrete)
    {
        concreteType = concrete;
    }

    public IntPtr ManagedToNativeAlloc(T man)
    {
        IntPtr h = ((efl.eo.IWrapper)man).raw_handle;
        if (h == IntPtr.Zero)
            return h;
        return efl.eo.Globals.efl_ref(h);
    }

    public void NativeFree(IntPtr nat)
    {
        efl.eo.Globals.efl_unref(nat);
    }

    public T NativeToManaged(IntPtr nat)
    {
        return (T) Activator.CreateInstance(concreteType, efl.eo.Globals.efl_ref(nat));
    }
}

public class GeneralElementTraits<T> : BaseElementTraits<T>
{
    public IntPtr ManagedToNativeAlloc(T man)
    {
        GCHandle pinnedData = GCHandle.Alloc(man, GCHandleType.Pinned);
        IntPtr ptr = pinnedData.AddrOfPinnedObject();
        IntPtr nat = efl_mono_native_alloc_copy(ptr, (uint)(Marshal.SizeOf<T>()));
        pinnedData.Free();
        return nat;
    }

    public void NativeFree(IntPtr nat)
    {
        efl_mono_native_free(nat);
    }

    public T NativeToManaged(IntPtr nat)
    {
        var w = Marshal.PtrToStructure<eina.ConvertWrapper<T> >(nat);
        return w.Val;
    }
}

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
        else if (IsString(type))
            return ElementType.StringType;
        else
            return ElementType.NumericType;
    }

    private static IDictionary<System.Type, object> register = new Dictionary<System.Type, object>();

    public static object RegisterTypeTraits<T>()
    {
        object traits;
        var type = typeof(T);
        if (IsEflObject(type))
        {
            System.Type concrete = type;
            if (!type.Name.EndsWith("Concrete"))
            {
                string fn = type.FullName;
                string aqn = type.AssemblyQualifiedName;
                aqn = aqn.Remove(0, fn.Length).Insert(0, fn + "Concrete");
                var c = System.Type.GetType(aqn);
                if (c != null && type.IsAssignableFrom(c))
                    concrete = c;
            }
            traits = new EflObjectElementTraits<T>(concrete);
        }
        else if (IsString(type))
            traits = new StringElementTraits<T>();
        else
            traits = new GeneralElementTraits<T>();

        register[type] = traits;
        return traits;
    }

    public static object RegisterTypeTraits<T>(BaseElementTraits<T> traits)
    {
        register[typeof(T)] = traits;
        return traits;
    }

    public static BaseElementTraits<T> GetTypeTraits<T>()
    {
        object traits;
        if (!register.TryGetValue(typeof(T), out traits))
            traits = RegisterTypeTraits<T>();
        return (BaseElementTraits<T>) traits;
    }

    // Convertion functions //

    public static IntPtr ManagedToNativeAlloc<T>(T man)
    {
        return GetTypeTraits<T>().ManagedToNativeAlloc(man);
    }

    public static void NativeFree<T>(IntPtr nat)
    {
        GetTypeTraits<T>().NativeFree(nat);
    }

    public static T NativeToManaged<T>(IntPtr nat)
    {
        return GetTypeTraits<T>().NativeToManaged(nat);
    }

/*
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
*/
}

}
