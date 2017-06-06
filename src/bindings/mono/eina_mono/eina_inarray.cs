using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;

using static eina.TraitFunctions;
using static eina.InarrayNativeFunctions;

namespace eina {

public static class InarrayNativeFunctions
{
    [DllImport("eina")] public static extern IntPtr
        eina_inarray_new(uint member_size, uint step);
    [DllImport("eina")] public static extern void
        eina_inarray_free(IntPtr array);
    [DllImport("eina")] public static extern void
        eina_inarray_step_set(IntPtr array, uint sizeof_eina_inarray, uint member_size, uint step);
    [DllImport("eina")] public static extern void
        eina_inarray_flush(IntPtr array);
    [DllImport("eina")] public static extern int
        eina_inarray_push(IntPtr array, IntPtr data);
    [DllImport("eina")] public static extern IntPtr
        eina_inarray_grow(IntPtr array, uint size);
    [DllImport("eina")] public static extern int
        eina_inarray_insert(IntPtr array, IntPtr data, IntPtr compare);
    [DllImport("eina")] public static extern int
        eina_inarray_insert_sorted(IntPtr array, IntPtr data, IntPtr compare);
    [DllImport("eina")] public static extern int
        eina_inarray_remove(IntPtr array, IntPtr data);
    [DllImport("eina")] public static extern IntPtr
        eina_inarray_pop(IntPtr array);
    [DllImport("eina")] public static extern IntPtr
        eina_inarray_nth(IntPtr array, uint position);
    [DllImport("eina")] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eina_inarray_insert_at(IntPtr array, uint position, IntPtr data);
    [DllImport("eina")] public static extern IntPtr
        eina_inarray_alloc_at(IntPtr array, uint position, uint member_count);
    [DllImport("eina")] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eina_inarray_replace_at(IntPtr array, uint position, IntPtr data);
    [DllImport("eina")] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eina_inarray_remove_at(IntPtr array, uint position);
    [DllImport("eina")] public static extern void
        eina_inarray_reverse(IntPtr array);
    [DllImport("eina")] public static extern void
        eina_inarray_sort(IntPtr array, IntPtr compare);
    [DllImport("eina")] public static extern int
        eina_inarray_search(IntPtr array, IntPtr data, IntPtr compare);
    [DllImport("eina")] public static extern int
        eina_inarray_search_sorted(IntPtr array, IntPtr data, IntPtr compare);
    [DllImport("eina")] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eina_inarray_foreach(IntPtr array, IntPtr function, IntPtr user_data);
    [DllImport("eina")] public static extern int
        eina_inarray_foreach_remove(IntPtr array, IntPtr match, IntPtr user_data);
    [DllImport("eina")] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eina_inarray_resize(IntPtr array, uint new_size);
    [DllImport("eina")] public static extern uint
        eina_inarray_count(IntPtr array);

    [DllImport("eina")] public static extern IntPtr
        eina_inarray_iterator_new(IntPtr array);
    [DllImport("eina")] public static extern IntPtr
        eina_inarray_iterator_reversed_new(IntPtr array);
    [DllImport("eina")] public static extern IntPtr
        eina_inarray_accessor_new(IntPtr array);
}

public class Inarray<T> : IEnumerable<T>, IDisposable
{
    public static uint DefaultStep = 0;

    public IntPtr Handle {get;set;} = IntPtr.Zero;
    public bool Own {get;set;}
    public bool OwnContent {get;set;}

    public int Length
    {
        get { return Count(); }
    }


    private void InitNew(uint step)
    {
        Handle = EinaInarrayNew<T>(step);
        Own = true;
        OwnContent = true;
        if (Handle == IntPtr.Zero)
            throw new SEHException("Could not alloc inarray");
    }

    public Inarray()
    {
        InitNew(DefaultStep);
    }

    public Inarray(uint step)
    {
        InitNew(step);
    }

    public Inarray(IntPtr handle, bool own)
    {
        Handle = handle;
        Own = own;
        OwnContent = own;
    }

    public Inarray(IntPtr handle, bool own, bool ownContent)
    {
        Handle = handle;
        Own = own;
        OwnContent = ownContent;
    }

    ~Inarray()
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
            uint len = eina_inarray_count(h);
            for(uint i = 0; i < len; ++i)
            {
                NativeFreeInplace<T>(eina_inarray_nth(h, i));
            }
        }

        if (Own)
            eina_inarray_free(h);
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
            for (int i = 0; i < len; ++i)
            {
                NativeFreeInplace<T>(eina_inarray_nth(h, i));
            }
        }
    }

    public void Clean()
    {
        FreeElementsIfOwned();
        eina_inarray_clean_custom_export_mono(Handle);
    }

    public void Flush()
    {
        FreeElementsIfOwned();
        eina_inarray_flush(Handle);
    }

    public int Count()
    {
        return (int) eina_inarray_count_custom_export_mono(Handle);
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

// TODO ???
//     public void Add(T val)
//     {
//         if (!Push(val))
//           throw;
//     }

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
