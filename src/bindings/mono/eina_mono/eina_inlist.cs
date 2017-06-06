using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;

using static eina.TraitFunctions;
using static eina.InlistNativeFunctions;
using eina.Callbacks;

namespace eina {

public static class InlistNativeFunctions
{
    [DllImport("eina")] public static extern IntPtr
        eina_inlist_append(IntPtr in_list, IntPtr in_item);

    [DllImport("eina")] public static extern IntPtr
        eina_inlist_prepend(IntPtr in_list, IntPtr in_item);

    [DllImport("eina")] public static extern IntPtr
        eina_inlist_append_relative(IntPtr in_list, IntPtr in_item, IntPtr in_relative);

    [DllImport("eina")] public static extern IntPtr
        eina_inlist_prepend_relative(IntPtr in_list, IntPtr in_item, IntPtr in_relative);

    [DllImport("eina")] public static extern IntPtr
        eina_inlist_remove(IntPtr in_list, IntPtr in_item);

    [DllImport("eina")] public static extern IntPtr
        eina_inlist_find(IntPtr in_list, IntPtr in_item);

    [DllImport("eina")] public static extern IntPtr
        eina_inlist_promote(IntPtr list, IntPtr item);

    [DllImport("eina")] public static extern IntPtr
        eina_inlist_demote(IntPtr list, IntPtr item);

    [DllImport("eina")] public static extern uint
        eina_inlist_count(IntPtr list);

    [DllImport("eina")] public static extern IntPtr
        eina_inlist_iterator_new(IntPtr in_list);

    [DllImport("eina")] public static extern IntPtr
        eina_inlist_accessor_new(IntPtr in_list);

    [DllImport("eina")] public static extern IntPtr
        eina_inlist_sorted_insert(IntPtr list, IntPtr item, IntPtr func);

    [DllImport("eina")] public static extern IntPtr
        eina_inlist_sorted_state_new(void);

    [DllImport("eina")] public static extern int
        eina_inlist_sorted_state_init(IntPtr state, IntPtr list);

    [DllImport("eina")] public static extern void
        eina_inlist_sorted_state_free(IntPtr state);

    [DllImport("eina")] public static extern IntPtr
        eina_inlist_sorted_state_insert(IntPtr list, IntPtr item, IntPtr func, IntPtr state);

    [DllImport("eina")] public static extern IntPtr
        eina_inlist_sort(IntPtr head, IntPtr func);

// static inline IntPtr eina_inlist_first(IntPtr list);
// static inline IntPtr eina_inlist_last(IntPtr list);
}

public class Inlist<T> : IEnumerable<T>, IDisposable
{
    public IntPtr Handle {get;set;} = IntPtr.Zero;
    public bool Own {get;set;}
    public bool OwnContent {get;set;}

    public int Length
    {
        get { return Count(); }
    }


    private void InitNew()
    {
        Handle = IntPtr.Zero;
        Own = true;
        OwnContent = true;
    }

    private IntPtr InternalLast()
    {
        return eina_inlist_last_custom_export_mono(Handle);
    }

    private static IntPtr InternalNext(IntPtr inlist)
    {
        return eina_inlist_next_custom_export_mono(inlist);
    }

    private static IntPtr InternalPrev(IntPtr inlist)
    {
        return eina_inlist_prev_custom_export_mono(inlist);
    }

    private static IntPtr InternalDataGet(IntPtr inlist)
    {
        return eina_inlist_data_get_custom_export_mono(inlist);
    }

    private static IntPtr InternalDataSet(IntPtr inlist, IntPtr data)
    {
        return eina_inlist_data_set_custom_export_mono(inlist, data);
    }


    public Inlist()
    {
        InitNew();
    }

    public Inlist(IntPtr handle, bool own)
    {
        Handle = handle;
        Own = own;
        OwnContent = own;
    }

    public Inlist(IntPtr handle, bool own, bool ownContent)
    {
        Handle = handle;
        Own = own;
        OwnContent = ownContent;
    }

    ~Inlist()
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
            for(IntPtr curr = h; curr != IntPtr.Zero; curr = InternalNext(curr))
            {
                NativeFree<T>(InternalDataGet(curr));
            }
        }

        if (Own)
            eina_inlist_free(h);
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

    public int Count()
    {
        return (int) eina_inlist_count_custom_export_mono(Handle);
    }

    public void Append(T val)
    {
        IntPtr ele = ManagedToNativeAlloc(val);
        Handle = eina_inlist_append(Handle, ele);
    }

    public void Prepend(T val)
    {
        IntPtr ele = ManagedToNativeAlloc(val);
        Handle = eina_inlist_prepend(Handle, ele);
    }

    public void SortedInsert(T val)
    {
        IntPtr ele = ManagedToNativeAlloc(val);
        Handle = eina_inlist_sorted_insert(Handle, EinaCompareCb<T>(), ele);
    }

    public void SortedInsert(Eina_Compare_Cb compareCb, T val)
    {
        IntPtr ele = ManagedToNativeAlloc(val);
        Handle = eina_inlist_sorted_insert(Handle, Marshal.GetFunctionPointerForDelegate(compareCb), ele);
    }

    public void Sort(int limit = 0)
    {
        Handle = eina_inlist_sort(Handle, (uint)limit, EinaCompareCb<T>());
    }

    public void Sort(Eina_Compare_Cb compareCb)
    {
        Handle = eina_inlist_sort(Handle, 0, Marshal.GetFunctionPointerForDelegate(compareCb));
    }

    public void Sort(int limit, Eina_Compare_Cb compareCb)
    {
        Handle = eina_inlist_sort(Handle, (uint)limit, Marshal.GetFunctionPointerForDelegate(compareCb));
    }

    public T Nth(int n)
    {
        // TODO: check bounds ???
        IntPtr ele = eina_inlist_nth(Handle, (uint)n);
        return NativeToManaged<T>(ele);
    }

    public void DataSet(int idx, T val)
    {
        IntPtr pos = eina_inlist_nth_inlist(Handle, (uint)idx);
        if (pos == IntPtr.Zero)
            throw new IndexOutOfRangeException();
        if (OwnContent)
            NativeFree<T>(InternalDataGet(pos));
        IntPtr ele = ManagedToNativeAlloc(val);
        InternalDataSet(pos, ele);
    }

    public T this[int idx]
    {
        get
        {
            return Nth(idx);
        }
        set
        {
            DataSet(idx, value);
        }
    }

    public T LastDataGet()
    {
        IntPtr ele = eina_inlist_last_data_get_custom_export_mono(Handle);
        return NativeToManaged<T>(ele);
    }

    public void Reverse()
    {
        Handle = eina_inlist_reverse(Handle);
    }

    public void Shuffle()
    {
        Handle = eina_inlist_shuffle(Handle, IntPtr.Zero);
    }

    public T[] ToArray()
    {
        var managed = new T[Count()];
        int i = 0;
        for(IntPtr curr = Handle; curr != IntPtr.Zero; curr = InternalNext(curr), ++i)
        {
            managed[i] = NativeToManaged<T>(InternalDataGet(curr));
        }
        return managed;
    }

    public void AppendArray(T[] values)
    {
        foreach (T v in values)
            Append(v);
    }

    public IEnumerator<T> GetEnumerator()
    {
        for(IntPtr curr = Handle; curr != IntPtr.Zero; curr = InternalNext(curr))
        {
            yield return NativeToManaged<T>(InternalDataGet(curr));
        }
    }

    System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
    {
        return this.GetEnumerator();
    }
}

}
