using System;
using System.Runtime.InteropServices;

using static eina.ElementFunctions;
using static eina.ArrayNativeFunctions;

namespace eina {

// public class TypeConvertions
// {
//     public static T NativeToManaged<T>(IntPtr ele)
//     {
//         Console.WriteLine("DATAGET GENERIC");
//         var w = Marshal.PtrToStructure<eina.ValueConvertWrapper<T> >(ele);
//         return w.Val;
//     }
//
//     public static string NativeToManaged<T>(IntPtr ele)
//     {
//         Console.WriteLine("DATAGET STRING");
//
//         IntPtr ele = eina_array_data_get_custom_export_mono(arr.Handle, idx);
//         if (ele == IntPtr.Zero)
//             return null;
//         return Marshal.PtrToStringAuto(ele);
//     }
// }

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

public class Array<T> : IDisposable
{
    public static uint DefaultStep = 32;

    private eina.ElementType elementType;
    public eina.ElementType GetElementType()
    {
        return elementType;
    }

    public IntPtr Handle {get;set;} = IntPtr.Zero;
    public bool Own {get;set;}
    public bool OwnContent {get;set;}

    public int Length
    {
        get { return Count(); }
    }


    private void InitNew(uint step)
    {
        elementType = GetElementTypeCode(typeof(T));

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
            if (elementType == ElementType.ObjectType)
                eina_array_free_obj_custom_export_mono(h);
            else if (elementType == ElementType.StringType)
                eina_array_free_string_custom_export_mono(h);
            else
                eina_array_free_generic_custom_export_mono(h);
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

    public void Clean()
    {
        eina_array_clean_custom_export_mono(Handle);
    }

    public void Flush()
    {
        eina_array_flush(Handle);
    }

    public int Count()
    {
        return (int) eina_array_count_custom_export_mono(Handle);
    }

//     public void Add(int val)
//     {
//         Push(val);
//     }

//     public T this[int i]
//     {
//         get
//         {
//             return this.DataGet(i);
//         }
//     }

}

}

public static class EinaArraySpecialMethods
{
    public static bool Push<T>(this eina.Array<T> arr, T val)
    {
        IntPtr ele = ManagedToNativeAlloc(val, arr.GetElementType());
        return arr.InternalPush(ele); // TODO: free if false ?
    }

    public static bool Push(this eina.Array<string> arr, string val)
    {
        IntPtr ele = ManagedToNativeAlloc(val);
        return arr.InternalPush(ele); // TODO: free if false ?
    }

    public static T Pop<T>(this eina.Array<T> arr)
    {
        IntPtr ele = arr.InternalPop();
        var r = NativeToManaged<T>(ele, arr.GetElementType());
        if (arr.OwnContent && ele != IntPtr.Zero)
            NativeFree(ele, arr.GetElementType());
        return r;
    }

    public static string Pop(this eina.Array<string> arr)
    {
        IntPtr ele = arr.InternalPop();
        var r = NativeToManagedString(ele);
        if (arr.OwnContent && ele != IntPtr.Zero)
            NativeFreeString(ele);
        return r;
    }

    public static T DataGet<T>(this eina.Array<T> arr, int idx)
    {
        IntPtr ele = arr.InternalDataGet(idx);
        return NativeToManaged<T>(ele, arr.GetElementType());
    }

    public static string DataGet(this eina.Array<string> arr, int idx)
    {
        IntPtr ele = arr.InternalDataGet(idx);
        return NativeToManagedString(ele);
    }

    public static T At<T>(this eina.Array<T> arr, int idx)
    {
        IntPtr ele = arr.InternalDataGet(idx);
        return NativeToManaged<T>(ele, arr.GetElementType());
    }

    public static string At(this eina.Array<string> arr, int idx)
    {
        IntPtr ele = arr.InternalDataGet(idx);
        return NativeToManagedString(ele);
    }

    public static void DataSet<T>(this eina.Array<T> arr, int idx, T val)
    {
        IntPtr ele = arr.InternalDataGet(idx); // TODO: check bondaries ??
        if (arr.OwnContent && ele != IntPtr.Zero)
            NativeFree(ele, arr.GetElementType());
        ele = ManagedToNativeAlloc(val, arr.GetElementType());
        arr.InternalDataSet(idx, ele);
    }

    public static void DataSet(this eina.Array<string> arr, int idx, string val)
    {
        IntPtr ele = arr.InternalDataGet(idx);
        if (arr.OwnContent && ele != IntPtr.Zero)
            NativeFreeString(ele);
        ele = ManagedToNativeAlloc(val);
        arr.InternalDataSet(idx, ele);
    }

    public static T[] ToArray<T>(this eina.Array<T> arr)
    {
        int len = arr.Length;
        var managed = new T[len];
        for(int i = 0; i < len; ++i)
        {
            managed[i] = NativeToManaged<T>(arr.InternalDataGet(i), arr.GetElementType());
        }
        return managed;
    }

    public static string[] ToArray(this eina.Array<string> arr)
    {
        int len = arr.Length;
        var managed = new string[len];
        for(int i = 0; i < len; ++i)
        {
            managed[i] = NativeToManagedString(arr.InternalDataGet(i));
        }
        return managed;
    }
}
