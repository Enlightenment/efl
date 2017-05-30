using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;

using eina.Callbacks;
using static eina.HashNativeFunctions;
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

    [DllImport("eflcustomexportsmono")] public static extern IntPtr
        efl_mono_native_ptr_compare_addr_get();
    [DllImport("eflcustomexportsmono")] public static extern IntPtr
        efl_mono_native_str_compare_addr_get();

    [DllImport("eflcustomexportsmono")] public static extern IntPtr
        efl_mono_native_free_addr_get();
    [DllImport("eflcustomexportsmono")] public static extern IntPtr
        efl_mono_native_efl_unref_addr_get();
}

[StructLayout(LayoutKind.Sequential)]
public struct ConvertWrapper<T>
{
    public T Val {get;set;}
}

public interface IBaseElementTraits<T>
{
    IntPtr ManagedToNativeAlloc(T man);
    IntPtr ManagedToNativeAllocRef(T man);
    void NativeFree(IntPtr nat);
    void NativeFreeRef(IntPtr nat);
    T NativeToManaged(IntPtr nat);
    IntPtr EinaCompareCb();
    IntPtr EinaFreeCb();
    IntPtr EinaHashNew();
}

public class StringElementTraits<T> : IBaseElementTraits<T>
{
    public IntPtr ManagedToNativeAlloc(T man)
    {
        return efl_mono_native_strdup((string)(object)man);
    }

    public IntPtr ManagedToNativeAllocRef(T man)
    {
        // Keep alloc on C# ?
        return ManagedToNativeAlloc(man);
    }

    public void NativeFree(IntPtr nat)
    {
        if (nat != IntPtr.Zero)
            efl_mono_native_free(nat);
    }

    public void NativeFreeRef(IntPtr nat)
    {
        NativeFree(nat);
    }

    public T NativeToManaged(IntPtr nat)
    {
        if (nat == IntPtr.Zero)
            return default(T);
        return (T)(object)Marshal.PtrToStringAuto(nat);
    }

    public IntPtr EinaCompareCb()
    {
        return efl_mono_native_str_compare_addr_get();
    }

    public IntPtr EinaFreeCb()
    {
        return efl_mono_native_free_addr_get();
    }

    public IntPtr EinaHashNew()
    {
        return eina_hash_string_superfast_new(IntPtr.Zero);
    }
}

public class EflObjectElementTraits<T> : IBaseElementTraits<T>
{
    private System.Type concreteType = null;
    private static IBaseElementTraits<IntPtr> intPtrTraits = null;

    public EflObjectElementTraits()
    {
        if (intPtrTraits == null)
            intPtrTraits = TraitFunctions.GetTypeTraits<IntPtr>();
    }

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

    public IntPtr ManagedToNativeAllocRef(T man)
    {
        return intPtrTraits.ManagedToNativeAlloc(((efl.eo.IWrapper)man).raw_handle);
    }

    public void NativeFree(IntPtr nat)
    {
        if (nat != IntPtr.Zero)
            efl.eo.Globals.efl_unref(nat);
    }

    public void NativeFreeRef(IntPtr nat)
    {
        intPtrTraits.NativeFree(nat);
    }

    public T NativeToManaged(IntPtr nat)
    {
        if (nat == IntPtr.Zero)
            return default(T);
        return (T) Activator.CreateInstance(concreteType, efl.eo.Globals.efl_ref(nat));
    }

    public IntPtr EinaCompareCb()
    {
        return efl_mono_native_ptr_compare_addr_get();
    }

    public IntPtr EinaFreeCb()
    {
        return efl_mono_native_efl_unref_addr_get();
    }

    public IntPtr EinaHashNew()
    {
        return eina_hash_pointer_new(IntPtr.Zero);
    }
}

public abstract class PrimitiveElementTraits<T>
{
    private Eina_Compare_Cb dlgt = null;

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
        if (nat == IntPtr.Zero)
        {
            eina.Log.Error("Null pointer on primitive/struct container.");
            return default(T);
        }
        var w = Marshal.PtrToStructure<eina.ConvertWrapper<T> >(nat);
        return w.Val;
    }

    private int PrimitiveCompareCb(IntPtr ptr1, IntPtr ptr2)
    {
        var m1 = (IComparable) NativeToManaged(ptr1);
        var m2 = NativeToManaged(ptr2);
        return m1.CompareTo(m2);
    }

    public IntPtr EinaCompareCb()
    {
        if (dlgt == null)
            dlgt = new Eina_Compare_Cb(PrimitiveCompareCb);
        return Marshal.GetFunctionPointerForDelegate(dlgt);
    }

    public IntPtr EinaFreeCb()
    {
        return efl_mono_native_free_addr_get();
    }
}

public class Primitive32ElementTraits<T> : PrimitiveElementTraits<T>, IBaseElementTraits<T>
{
    IBaseElementTraits<Int32> int32Traits = null;

    public IntPtr ManagedToNativeAllocRef(T man)
    {
        if (int32Traits == null)
            int32Traits = TraitFunctions.GetTypeTraits<Int32>();
        return int32Traits.ManagedToNativeAlloc(Convert.ToInt32((object)man));
    }

    public void NativeFreeRef(IntPtr nat)
    {
        if (int32Traits == null)
            int32Traits = TraitFunctions.GetTypeTraits<Int32>();
        int32Traits.NativeFree(nat);
    }

    public IntPtr EinaHashNew()
    {
        return eina_hash_int32_new(IntPtr.Zero);
    }
}

public class Primitive64ElementTraits<T> : PrimitiveElementTraits<T>, IBaseElementTraits<T>
{
    IBaseElementTraits<Int64> int64Traits = null;

    public IntPtr ManagedToNativeAllocRef(T man)
    {
        if (int64Traits == null)
            int64Traits = TraitFunctions.GetTypeTraits<Int64>();
        return int64Traits.ManagedToNativeAlloc(Convert.ToInt64((object)man));
    }

    public void NativeFreeRef(IntPtr nat)
    {
        if (int64Traits == null)
            int64Traits = TraitFunctions.GetTypeTraits<Int64>();
        int64Traits.NativeFree(nat);
    }

    public IntPtr EinaHashNew()
    {
        return eina_hash_int64_new(IntPtr.Zero);
    }
}

public static class TraitFunctions
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
                var c = type.Assembly.GetType(type.FullName + "Concrete");
                if (c != null && type.IsAssignableFrom(c))
                    concrete = c;
            }
            traits = new EflObjectElementTraits<T>(concrete);
        }
        else if (IsString(type))
            traits = new StringElementTraits<T>();
        else if (type.IsValueType)
        {
            if (Marshal.SizeOf<T>() <= 4)
                traits = new Primitive32ElementTraits<T>();
            else
                traits = new Primitive64ElementTraits<T>();
        }
        else
            throw new Exception("No traits registered for this type");

        register[type] = traits;
        return traits;
    }

    public static object RegisterTypeTraits<T>(IBaseElementTraits<T> traits)
    {
        register[typeof(T)] = traits;
        return traits;
    }

    public static IBaseElementTraits<T> GetTypeTraits<T>()
    {
        object traits;
        if (!register.TryGetValue(typeof(T), out traits))
            traits = RegisterTypeTraits<T>();
        return (IBaseElementTraits<T>) traits;
    }

    //                  //
    // Traits functions //
    //                  //

    // Convertion functions //

    public static IntPtr ManagedToNativeAlloc<T>(T man)
    {
        return GetTypeTraits<T>().ManagedToNativeAlloc(man);
    }

    public static IntPtr ManagedToNativeAllocRef<T>(T man)
    {
        return GetTypeTraits<T>().ManagedToNativeAllocRef(man);
    }

    public static void NativeFree<T>(IntPtr nat)
    {
        GetTypeTraits<T>().NativeFree(nat);
    }

    public static void NativeFreeRef<T>(IntPtr nat)
    {
        GetTypeTraits<T>().NativeFreeRef(nat);
    }

    public static T NativeToManaged<T>(IntPtr nat)
    {
        return GetTypeTraits<T>().NativeToManaged(nat);
    }

    // Misc //

    public static IntPtr EinaCompareCb<T>()
    {
        return GetTypeTraits<T>().EinaCompareCb();
    }

    public static IntPtr EinaFreeCb<T>()
    {
        return GetTypeTraits<T>().EinaFreeCb();
    }

    public static IntPtr EinaHashNew<TKey>()
    {
        return GetTypeTraits<TKey>().EinaHashNew();
    }
}

}
