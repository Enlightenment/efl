using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;

using eina.Callbacks;
using static eina.HashNativeFunctions;
using static eina.InarrayNativeFunctions;
using static eina.InlistNativeFunctions;
using static eina.NativeCustomExportFunctions;
using static eina.ContainerCommonData;

namespace eina {

public enum ElementType { NumericType, StringType, ObjectType };


public static class NativeCustomExportFunctions
{
    [DllImport("eflcustomexportsmono")] public static extern void
        efl_mono_native_free(IntPtr ptr);
    [DllImport("eflcustomexportsmono")] public static extern void
        efl_mono_native_free_ref(IntPtr ptr);
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

public static class ContainerCommonData
{
    public static IBaseElementTraits<IntPtr> intPtrTraits = null;
}

[StructLayout(LayoutKind.Sequential)]
public struct ConvertWrapper<T>
{
    public T Val {get;set;}
}

[StructLayout(LayoutKind.Sequential)]
public struct InlistMem
{
    public IntPtr next {get;set;}
    public IntPtr prev {get;set;}
    public IntPtr last {get;set;}
}

[StructLayout(LayoutKind.Sequential)]
public struct InlistNode<T>
{
    public InlistMem __in_list {get;set;}
    public T Val {get;set;}
}


public interface IBaseElementTraits<T>
{
    IntPtr ManagedToNativeAlloc(T man);
    IntPtr ManagedToNativeAllocRef(T man, bool refs);
    IntPtr ManagedToNativeAllocInlistNode(T man);
    IntPtr ManagedToNativeAllocInplace(T man);
    void NativeFree(IntPtr nat);
    void NativeFreeRef(IntPtr nat, bool unrefs);
    void NativeFreeInlistNodeElement(IntPtr nat);
    void NativeFreeInlistNode(IntPtr nat, bool freeElement);
    void NativeFreeInplace(IntPtr nat);
    void ResidueFreeInplace(IntPtr nat);
    T NativeToManaged(IntPtr nat);
    T NativeToManagedRef(IntPtr nat);
    T NativeToManagedInlistNode(IntPtr nat);
    T NativeToManagedInplace(IntPtr nat);
    IntPtr EinaCompareCb();
    IntPtr EinaFreeCb();
    IntPtr EinaHashNew();
    IntPtr EinaInarrayNew(uint step);
}

public class StringElementTraits<T> : IBaseElementTraits<T>
{

    public StringElementTraits()
    {
        if (intPtrTraits == null)
            intPtrTraits = TraitFunctions.GetTypeTraits<IntPtr>();
    }

    public IntPtr ManagedToNativeAlloc(T man)
    {
        return efl_mono_native_strdup((string)(object)man);
    }

    public IntPtr ManagedToNativeAllocRef(T man, bool refs)
    {
        // Keep alloc on C# ?
        return ManagedToNativeAlloc(man);
    }

    public IntPtr ManagedToNativeAllocInlistNode(T man)
    {
        var node = new InlistNode<IntPtr>();
        node.Val = ManagedToNativeAlloc(man);
        GCHandle pinnedData = GCHandle.Alloc(node, GCHandleType.Pinned);
        IntPtr ptr = pinnedData.AddrOfPinnedObject();
        IntPtr nat = efl_mono_native_alloc_copy(ptr, (uint)(Marshal.SizeOf<InlistNode<IntPtr> >()));
        pinnedData.Free();
        return nat;
    }

    public IntPtr ManagedToNativeAllocInplace(T man)
    {
        return intPtrTraits.ManagedToNativeAlloc(ManagedToNativeAlloc(man));
    }

    public void NativeFree(IntPtr nat)
    {
        if (nat != IntPtr.Zero)
            efl_mono_native_free(nat);
    }

    public void NativeFreeRef(IntPtr nat, bool unrefs)
    {
        NativeFree(nat);
    }

    public void NativeFreeInlistNodeElement(IntPtr nat)
    {
        if (nat == IntPtr.Zero)
            return;
        var node = Marshal.PtrToStructure< InlistNode<IntPtr> >(nat);
        NativeFree(node.Val);
    }

    public void NativeFreeInlistNode(IntPtr nat, bool freeElement)
    {
        if (nat == IntPtr.Zero)
            return;
        if (freeElement)
            NativeFreeInlistNodeElement(nat);
        efl_mono_native_free(nat);
    }

    public void NativeFreeInplace(IntPtr nat)
    {
        efl_mono_native_free_ref(nat);
    }

    public void ResidueFreeInplace(IntPtr nat)
    {
        intPtrTraits.NativeFree(nat);
    }

    public T NativeToManaged(IntPtr nat)
    {
        if (nat == IntPtr.Zero)
            return default(T);
        return (T)(object)Marshal.PtrToStringAuto(nat);
    }

    public T NativeToManagedRef(IntPtr nat)
    {
        return NativeToManaged(nat);
    }

    public T NativeToManagedInlistNode(IntPtr nat)
    {
        if (nat == IntPtr.Zero)
        {
            eina.Log.Error("Null pointer for Inlist node.");
            return default(T);
        }
        var w = Marshal.PtrToStructure< InlistNode<IntPtr> >(nat);
        return NativeToManaged(w.Val);
    }

    public T NativeToManagedInplace(IntPtr nat)
    {
        if (nat == IntPtr.Zero)
            return default(T);
        return NativeToManaged(intPtrTraits.NativeToManaged(nat));
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

    public IntPtr EinaInarrayNew(uint step)
    {
        return eina_inarray_new((uint)Marshal.SizeOf<IntPtr>(), step);
    }
}

public class EflObjectElementTraits<T> : IBaseElementTraits<T>
{
    private System.Type concreteType = null;

    public EflObjectElementTraits(System.Type concrete)
    {
        if (intPtrTraits == null)
            intPtrTraits = TraitFunctions.GetTypeTraits<IntPtr>();

        concreteType = concrete;
    }

    public IntPtr ManagedToNativeAlloc(T man)
    {
        IntPtr h = ((efl.eo.IWrapper)man).raw_handle;
        if (h == IntPtr.Zero)
            return h;
        return efl.eo.Globals.efl_ref(h);
    }

    public IntPtr ManagedToNativeAllocRef(T man, bool refs)
    {
        IntPtr h = refs ? ManagedToNativeAlloc(man) : ((efl.eo.IWrapper)man).raw_handle;
        return intPtrTraits.ManagedToNativeAlloc(h);
    }

    public IntPtr ManagedToNativeAllocInlistNode(T man)
    {
        var node = new InlistNode<IntPtr>();
        node.Val = ManagedToNativeAlloc(man);
        GCHandle pinnedData = GCHandle.Alloc(node, GCHandleType.Pinned);
        IntPtr ptr = pinnedData.AddrOfPinnedObject();
        IntPtr nat = efl_mono_native_alloc_copy(ptr, (uint)(Marshal.SizeOf<InlistNode<IntPtr> >()));
        pinnedData.Free();
        return nat;
    }

    public IntPtr ManagedToNativeAllocInplace(T man)
    {
        return intPtrTraits.ManagedToNativeAlloc(ManagedToNativeAlloc(man));
    }

    public void NativeFree(IntPtr nat)
    {
        if (nat != IntPtr.Zero)
            efl.eo.Globals.efl_unref(nat);
    }

    public void NativeFreeRef(IntPtr nat, bool unrefs)
    {
        if (unrefs)
            NativeFree(intPtrTraits.NativeToManaged(nat));
        intPtrTraits.NativeFree(nat);
    }

    public void NativeFreeInlistNodeElement(IntPtr nat)
    {
        if (nat == IntPtr.Zero)
            return;
        var node = Marshal.PtrToStructure< InlistNode<IntPtr> >(nat);
        NativeFree(node.Val);
    }

    public void NativeFreeInlistNode(IntPtr nat, bool freeElement)
    {
        if (nat == IntPtr.Zero)
            return;
        if (freeElement)
            NativeFreeInlistNodeElement(nat);
        efl_mono_native_free(nat);
    }

    public void NativeFreeInplace(IntPtr nat)
    {
        NativeFree(intPtrTraits.NativeToManaged(nat));
    }

    public void ResidueFreeInplace(IntPtr nat)
    {
        intPtrTraits.NativeFree(nat);
    }

    public T NativeToManaged(IntPtr nat)
    {
        if (nat == IntPtr.Zero)
            return default(T);
        return (T) Activator.CreateInstance(concreteType, efl.eo.Globals.efl_ref(nat));
    }

    public T NativeToManagedRef(IntPtr nat)
    {
        if (nat == IntPtr.Zero)
            return default(T);
        return NativeToManaged(intPtrTraits.NativeToManaged(nat));
    }

    public T NativeToManagedInlistNode(IntPtr nat)
    {
        if (nat == IntPtr.Zero)
        {
            eina.Log.Error("Null pointer for Inlist node.");
            return default(T);
        }
        var w = Marshal.PtrToStructure< InlistNode<IntPtr> >(nat);
        return NativeToManaged(w.Val);
    }

    public T NativeToManagedInplace(IntPtr nat)
    {
        if (nat == IntPtr.Zero)
            return default(T);
        return NativeToManaged(intPtrTraits.NativeToManaged(nat));
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

    public IntPtr EinaInarrayNew(uint step)
    {
        return eina_inarray_new((uint)Marshal.SizeOf<IntPtr>(), step);
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

    public IntPtr ManagedToNativeAllocInlistNode(T man)
    {
        var node = new InlistNode<T>();
        node.Val = man;
        GCHandle pinnedData = GCHandle.Alloc(node, GCHandleType.Pinned);
        IntPtr ptr = pinnedData.AddrOfPinnedObject();
        IntPtr nat = efl_mono_native_alloc_copy(ptr, (uint)(Marshal.SizeOf< InlistNode<T> >()));
        pinnedData.Free();
        return nat;
    }

    public IntPtr ManagedToNativeAllocInplace(T man)
    {
        return ManagedToNativeAlloc(man);
    }

    public void NativeFree(IntPtr nat)
    {
        efl_mono_native_free(nat);
    }

    public void NativeFreeInlistNodeElement(IntPtr nat)
    {
        // Do nothing
    }

    public void NativeFreeInlistNode(IntPtr nat, bool freeElement)
    {
        efl_mono_native_free(nat);
    }

    public void NativeFreeInplace(IntPtr nat)
    {
        // Do nothing
    }

    public void ResidueFreeInplace(IntPtr nat)
    {
        NativeFree(nat);
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

    public T NativeToManagedRef(IntPtr nat)
    {
        return NativeToManaged(nat);
    }

    public T NativeToManagedInlistNode(IntPtr nat)
    {
        if (nat == IntPtr.Zero)
        {
            eina.Log.Error("Null pointer for Inlist node.");
            return default(T);
        }
        var w = Marshal.PtrToStructure< InlistNode<T> >(nat);
        return w.Val;
    }

    public T NativeToManagedInplace(IntPtr nat)
    {
        return NativeToManaged(nat);
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

    public IntPtr EinaInarrayNew(uint step)
    {
        return eina_inarray_new((uint)Marshal.SizeOf<T>(), step);
    }
}

public class Primitive32ElementTraits<T> : PrimitiveElementTraits<T>, IBaseElementTraits<T>
{
    private static IBaseElementTraits<Int32> int32Traits = null;

    public Primitive32ElementTraits()
    {
        if (int32Traits == null)
            if (typeof(T) == typeof(Int32)) // avoid infinite recursion
                int32Traits = (IBaseElementTraits<Int32>)this;
            else
                int32Traits = TraitFunctions.GetTypeTraits<Int32>();
    }

    public IntPtr ManagedToNativeAllocRef(T man, bool refs)
    {
        return int32Traits.ManagedToNativeAlloc(Convert.ToInt32((object)man));
    }

    public void NativeFreeRef(IntPtr nat, bool unrefs)
    {
        int32Traits.NativeFree(nat);
    }

    public IntPtr EinaHashNew()
    {
        return eina_hash_int32_new(IntPtr.Zero);
    }
}

public class Primitive64ElementTraits<T> : PrimitiveElementTraits<T>, IBaseElementTraits<T>
{
    private static IBaseElementTraits<Int64> int64Traits = null;

    public Primitive64ElementTraits()
    {
        if (int64Traits == null)
            if (typeof(T) == typeof(Int64)) // avoid infinite recursion
                int64Traits = (IBaseElementTraits<Int64>)this;
            else
                int64Traits = TraitFunctions.GetTypeTraits<Int64>();
    }

    public IntPtr ManagedToNativeAllocRef(T man, bool refs)
    {
        return int64Traits.ManagedToNativeAlloc(Convert.ToInt64((object)man));
    }

    public void NativeFreeRef(IntPtr nat, bool unrefs)
    {
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

    public static IntPtr ManagedToNativeAllocRef<T>(T man, bool refs = false)
    {
        return GetTypeTraits<T>().ManagedToNativeAllocRef(man, refs);
    }

    public static IntPtr ManagedToNativeAllocInplace<T>(T man)
    {
        return GetTypeTraits<T>().ManagedToNativeAllocInplace(man);
    }

    public static IntPtr ManagedToNativeAllocInlistNode<T>(T man)
    {
        return GetTypeTraits<T>().ManagedToNativeAllocInlistNode(man);
    }

    public static void NativeFree<T>(IntPtr nat)
    {
        GetTypeTraits<T>().NativeFree(nat);
    }

    public static void NativeFreeRef<T>(IntPtr nat, bool unrefs = false)
    {
        GetTypeTraits<T>().NativeFreeRef(nat, unrefs);
    }

    public static void NativeFreeInlistNodeElement<T>(IntPtr nat)
    {
        GetTypeTraits<T>().NativeFreeInlistNodeElement(nat);
    }

    public static void NativeFreeInlistNode<T>(IntPtr nat, bool freeElement = true)
    {
        GetTypeTraits<T>().NativeFreeInlistNode(nat, freeElement);
    }

    public static void NativeFreeInplace<T>(IntPtr nat)
    {
        GetTypeTraits<T>().NativeFreeInplace(nat);
    }

    public static void ResidueFreeInplace<T>(IntPtr nat)
    {
        GetTypeTraits<T>().ResidueFreeInplace(nat);
    }

    public static T NativeToManaged<T>(IntPtr nat)
    {
        return GetTypeTraits<T>().NativeToManaged(nat);
    }

    public static T NativeToManagedRef<T>(IntPtr nat)
    {
        return GetTypeTraits<T>().NativeToManagedRef(nat);
    }

    public static T NativeToManagedInlistNode<T>(IntPtr nat)
    {
        return GetTypeTraits<T>().NativeToManagedInlistNode(nat);
    }

    public static T NativeToManagedInplace<T>(IntPtr nat)
    {
        return GetTypeTraits<T>().NativeToManagedInplace(nat);
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

    public static IntPtr EinaInarrayNew<T>(uint step)
    {
        return GetTypeTraits<T>().EinaInarrayNew(step);
    }
}

}
