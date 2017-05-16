using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;

using static eina.ElementFunctions;
using static eina.ArrayNativeFunctions;

namespace eina {

public static class ArrayNativeFunctions
{
    [DllImport("eina")] public static extern IntPtr
        eina_array_new(uint step);
    [DllImport("eina")] public static extern void
        eina_array_free(IntPtr array);
    [DllImport("eina")] public static extern void
        eina_array_flush(IntPtr array);
    [DllImport("eina")] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eina_array_remove(IntPtr array, IntPtr keep, IntPtr gdata);
    [DllImport("eina")] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eina_array_push(IntPtr array, IntPtr data);

    [DllImport("eflcustomexportsmono")] public static extern void
        eina_array_free_generic_custom_export_mono(IntPtr array);
    [DllImport("eflcustomexportsmono")] public static extern void
        eina_array_free_string_custom_export_mono(IntPtr array);
    [DllImport("eflcustomexportsmono")] public static extern void
        eina_array_free_obj_custom_export_mono(IntPtr array);
    [DllImport("eflcustomexportsmono")] public static extern void
        eina_array_clean_custom_export_mono(IntPtr array);
    [DllImport("eflcustomexportsmono")] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eina_array_push_custom_export_mono(IntPtr array, IntPtr data);
    [DllImport("eflcustomexportsmono")] public static extern IntPtr
        eina_array_pop_custom_export_mono(IntPtr array);
    [DllImport("eflcustomexportsmono")] public static extern IntPtr
        eina_array_data_get_custom_export_mono(IntPtr array, uint idx);
    [DllImport("eflcustomexportsmono")] public static extern void
        eina_array_data_set_custom_export_mono(IntPtr array, uint idx, IntPtr data);
    [DllImport("eflcustomexportsmono")] public static extern uint
        eina_array_count_custom_export_mono(IntPtr array);

    [DllImport("eflcustomexportsmono")] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eina_array_foreach_custom_export_mono(IntPtr array, IntPtr cb, IntPtr fdata);
}

public class Array<T> : IEnumerable<T>, IDisposable
{
    public static uint DefaultStep = 32;

    public IntPtr Handle {get;set;} = IntPtr.Zero;
    public bool Own {get;set;}
    public bool OwnContent {get;set;}

    public int Length
    {
        get { return Count(); }
    }


    private void InitNew(uint step)
    {
        Handle = eina_array_new(step);
        Own = true;
        OwnContent = true;
        if (Handle == IntPtr.Zero)
            throw new SEHException("Could not alloc array");
    }

    internal bool InternalPush(IntPtr ele)
    {
        return eina_array_push_custom_export_mono(Handle, ele);
    }

    internal IntPtr InternalPop()
    {
        return eina_array_pop_custom_export_mono(Handle);
    }

    internal IntPtr InternalDataGet(int idx)
    {
        return eina_array_data_get_custom_export_mono(Handle, (uint)idx); // TODO: Check bounds ???
    }

    internal void InternalDataSet(int idx, IntPtr ele)
    {
        eina_array_data_set_custom_export_mono(Handle, (uint)idx, ele); // TODO: Check bounds ???
    }

    public Array()
    {
        InitNew(DefaultStep);
    }

    public Array(uint step)
    {
        InitNew(step);
    }

    public Array(IntPtr handle, bool own)
    {
        Handle = handle;
        Own = own;
        OwnContent = own;
    }

    public Array(IntPtr handle, bool own, bool ownContent)
    {
        Handle = handle;
        Own = own;
        OwnContent = ownContent;
    }

    ~Array()
    {
        Dispose(false);
    }

    protected virtual void Dispose(bool disposing)
    {
        IntPtr h = Handle;
        Handle = IntPtr.Zero;
        if (h == IntPtr.Zero)
            return;

        if (OwnContent)
        {
            int len = (int)eina_array_count_custom_export_mono(h);
            for(int i = 0; i < len; ++i)
            {
                NativeFree<T>(eina_array_data_get_custom_export_mono(h, (uint)i));
            }
        }

        if (Own)
            eina_array_free(h);
    }

    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    public void Free()
    {
        Dispose();
    }

    public IntPtr Release()
    {
        IntPtr h = Handle;
        Handle = IntPtr.Zero;
        return h;
    }

    private void FreeElementsIfOwned()
    {
        if (OwnContent)
        {
            int len = Length;
            for(int i = 0; i < len; ++i)
            {
                NativeFree<T>(InternalDataGet(i));
            }
        }
    }

    public void Clean()
    {
        FreeElementsIfOwned();
        eina_array_clean_custom_export_mono(Handle);
    }

    public void Flush()
    {
        FreeElementsIfOwned();
        eina_array_flush(Handle);
    }

    public int Count()
    {
        return (int) eina_array_count_custom_export_mono(Handle);
    }

    public void SetOwnership(bool ownAll)
    {
        Own = ownAll;
        OwnContent = ownAll;
    }

    public void SetOwnership(bool own, bool ownContent)
    {
        Own = own;
        OwnContent = ownContent;
    }

    public bool Push(T val)
    {
        IntPtr ele = ManagedToNativeAlloc(val);
        var r = InternalPush(ele);
        if (!r)
            NativeFree<T>(ele);
        return r;
    }

    public void Add(T val)
    {
        Push(val);
    }

    public T Pop()
    {
        IntPtr ele = InternalPop();
        var r = NativeToManaged<T>(ele);
        if (OwnContent && ele != IntPtr.Zero)
            NativeFree<T>(ele);
        return r;
    }

    public T DataGet(int idx)
    {
        IntPtr ele = InternalDataGet(idx);
        return NativeToManaged<T>(ele);
    }

    public T At(int idx)
    {
        return DataGet(idx);
    }

    public void DataSet(int idx, T val)
    {
        IntPtr ele = InternalDataGet(idx); // TODO: check bondaries ??
        if (OwnContent && ele != IntPtr.Zero)
            NativeFree<T>(ele);
        ele = ManagedToNativeAlloc(val);
        InternalDataSet(idx, ele);
    }

    public T this[int idx]
    {
        get
        {
            return DataGet(idx);
        }
        set
        {
            DataSet(idx, value);
        }
    }

    public T[] ToArray()
    {
        int len = Length;
        var managed = new T[len];
        for(int i = 0; i < len; ++i)
        {
            managed[i] = DataGet(i);
        }
        return managed;
    }

    public bool Append(T[] values)
    {
        foreach(T v in values)
            if (!Push(v))
                return false;
        return true;
    }

    public IEnumerator<T> GetEnumerator()
    {
        int len = Length;
        for(int i = 0; i < len; ++i)
        {
            yield return DataGet(i);
        }
    }

    System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
    {
        return this.GetEnumerator();
    }
}

}
