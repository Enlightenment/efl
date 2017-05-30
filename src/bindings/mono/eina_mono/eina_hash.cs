using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;

using static eina.TraitFunctions;
using static eina.HashNativeFunctions;
using eina.Callbacks;

namespace eina
{

public static class HashNativeFunctions
{
    [DllImport("eina")] public static extern IntPtr
        eina_hash_new(IntPtr key_length_cb, IntPtr key_cmp_cb, IntPtr key_hash_cb, IntPtr data_free_cb, int buckets_power_size);

    [DllImport("eina")] public static extern void
        eina_hash_free_cb_set(IntPtr hash, IntPtr data_free_cb);

    [DllImport("eina")] public static extern IntPtr
        eina_hash_string_djb2_new(IntPtr data_free_cb);

    [DllImport("eina")] public static extern IntPtr
        eina_hash_string_superfast_new(IntPtr data_free_cb);

    [DllImport("eina")] public static extern IntPtr
        eina_hash_string_small_new(IntPtr data_free_cb);

    [DllImport("eina")] public static extern IntPtr
        eina_hash_int32_new(IntPtr data_free_cb);

    [DllImport("eina")] public static extern IntPtr
        eina_hash_int64_new(IntPtr data_free_cb);

    [DllImport("eina")] public static extern IntPtr
        eina_hash_pointer_new(IntPtr data_free_cb);

    [DllImport("eina")] public static extern IntPtr
        eina_hash_stringshared_new(IntPtr data_free_cb);

    [DllImport("eina")] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eina_hash_add(IntPtr hash, IntPtr key, IntPtr data);

    [DllImport("eina")] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eina_hash_direct_add(IntPtr hash, IntPtr key, IntPtr data);

    [DllImport("eina")] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eina_hash_del(IntPtr hash, IntPtr key, IntPtr data);

    [DllImport("eina")] public static extern IntPtr
        eina_hash_find(IntPtr hash, IntPtr key);

    [DllImport("eina")] public static extern IntPtr
        eina_hash_modify(IntPtr hash, IntPtr key, IntPtr data);

    [DllImport("eina")] public static extern IntPtr
        eina_hash_set(IntPtr hash, IntPtr key, IntPtr data);

    [DllImport("eina")] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eina_hash_move(IntPtr hash, IntPtr old_key, IntPtr new_key);

    [DllImport("eina")] public static extern void
        eina_hash_free(IntPtr hash);

    [DllImport("eina")] public static extern void
        eina_hash_free_buckets(IntPtr hash);

    [DllImport("eina")] public static extern int
        eina_hash_population(IntPtr hash);

    [DllImport("eina")] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eina_hash_add_by_hash(IntPtr hash, IntPtr key, int key_length, int key_hash, IntPtr data);

    [DllImport("eina")] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eina_hash_direct_add_by_hash(IntPtr hash, IntPtr key, int key_length, int key_hash, IntPtr data);

    [DllImport("eina")] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eina_hash_del_by_key_hash(IntPtr hash, IntPtr key, int key_length, int key_hash);

    [DllImport("eina")] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eina_hash_del_by_key(IntPtr hash, IntPtr key);

    [DllImport("eina")] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eina_hash_del_by_data(IntPtr hash, IntPtr data);

    [DllImport("eina")] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eina_hash_del_by_hash(IntPtr hash, IntPtr key, int key_length, int key_hash, IntPtr data);

    [DllImport("eina")] public static extern IntPtr
        eina_hash_find_by_hash(IntPtr hash, IntPtr key, int key_length, int key_hash);

    [DllImport("eina")] public static extern IntPtr
        eina_hash_modify_by_hash(IntPtr hash, IntPtr key, int key_length, int key_hash, IntPtr data);

    [DllImport("eina")] public static extern void
        eina_hash_foreach(IntPtr hash, IntPtr func, IntPtr fdata);


    [DllImport("eina")] public static extern void
        eina_hash_list_append(IntPtr hash, IntPtr key, IntPtr data);
    [DllImport("eina")] public static extern void
        eina_hash_list_prepend(IntPtr hash, IntPtr key, IntPtr data);
    [DllImport("eina")] public static extern void
        eina_hash_list_remove(IntPtr hash, IntPtr key, IntPtr data);

    [DllImport("eina")] public static extern int
        eina_hash_superfast(string key, int len);
}

public class Hash<TKey, TValue> : IDisposable
{
    public IntPtr Handle {get; set;} = IntPtr.Zero;
    public bool Own {get; set;}
    public bool OwnContent {get; set;}

    public int Count {
        get {
            return Population();
        }
    }


    private void InitNew()
    {
        Handle = EinaHashNew<TKey>();
        SetOwn(true);
        SetOwnContent(true);
    }

    public Hash()
    {
        InitNew();
    }

    public Hash(IntPtr handle, bool own)
    {
        Handle = handle;
        SetOwn(own);
        SetOwnContent(own);
    }

    public Hash(IntPtr handle, bool own, bool ownContent)
    {
        Handle = handle;
        SetOwn(own);
        SetOwnContent(ownContent);
    }

    ~Hash()
    {
        Dispose(false);
    }

    protected virtual void Dispose(bool disposing)
    {
        IntPtr h = Handle;
        Handle = IntPtr.Zero;
        if (h == IntPtr.Zero)
            return;

        if (Own)
            eina_hash_free(h);
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

    public void SetOwn(bool own)
    {
        Own = own;
    }

    public void SetOwnContent(bool ownContent)
    {
        OwnContent = ownContent;

        if (ownContent)
            eina_hash_free_cb_set(Handle, EinaFreeCb<TValue>());
    }

    public void SetOwnership(bool ownAll)
    {
        SetOwn(ownAll);
        SetOwnContent(ownAll);
    }

    public void SetOwnership(bool own, bool ownContent)
    {
        SetOwn(own);
        SetOwnContent(ownContent);
    }

    public void UnSetFreeCb()
    {
        eina_hash_free_cb_set(Handle, IntPtr.Zero);
    }

    public bool AddNew(TKey key, TValue val)
    {
        var nk = ManagedToNativeAllocRef(key);
        var nv = ManagedToNativeAlloc(val);
        var r = eina_hash_add(Handle, nk, nv);
        NativeFreeRef<TKey>(nk);
        return r;
    }

    public void Add(TKey key, TValue val)
    {
        Set(key, val);
    }

    public bool DelByKey(TKey key)
    {
        var nk = ManagedToNativeAllocRef(key);
        var r = eina_hash_del_by_key(Handle, nk);
        NativeFreeRef<TKey>(nk);
        return r;
    }

    public bool DelByValue(TValue val)
    {
        var nv = ManagedToNativeAlloc(val);
        var r = eina_hash_del_by_data(Handle, nv);
        NativeFree<TValue>(nv);
        return r;
    }

    public void Remove(TKey key)
    {
        DelByKey(key);
    }

    public TValue Find(TKey key)
    {
        var nk = ManagedToNativeAllocRef(key);
        var found = eina_hash_find(Handle, nk);
        NativeFreeRef<TKey>(nk);
        if (found == IntPtr.Zero)
            throw new KeyNotFoundException();
        return NativeToManaged<TValue>(found);
    }

    public bool TryGetValue(TKey key, out TValue val)
    {
        var nk = ManagedToNativeAllocRef(key);
        var found = eina_hash_find(Handle, nk);
        NativeFreeRef<TKey>(nk);
        if (found == IntPtr.Zero)
        {
            val = default(TValue);
            return false;
        }
        val = NativeToManaged<TValue>(found);
        return true;
    }

    public bool ContainsKey(TKey key)
    {
        var nk = ManagedToNativeAllocRef(key);
        var found = eina_hash_find(Handle, nk);
        NativeFreeRef<TKey>(nk);
        return found != IntPtr.Zero;
    }

    public bool Modify(TKey key, TValue val)
    {
        var nk = ManagedToNativeAllocRef(key);
        var nv = ManagedToNativeAlloc(val);
        var old = eina_hash_modify(Handle, nk, nv);
        NativeFreeRef<TKey>(nk);
        if (old == IntPtr.Zero)
        {
            NativeFree<TValue>(nv);
            return false;
        }
        if (OwnContent)
            NativeFree<TValue>(old);
        return true;
    }

    public void Set(TKey key, TValue val)
    {
        var nk = ManagedToNativeAllocRef(key);
        var nv = ManagedToNativeAlloc(val);
        var old = eina_hash_set(Handle, nk, nv);
        NativeFreeRef<TKey>(nk);
        if (OwnContent && old != IntPtr.Zero)
            NativeFree<TValue>(old);
    }

    public TValue this[TKey key]
    {
        get
        {
            return Find(key);
        }
        set
        {
            Set(key, value);
        }
    }

    public bool Move(TKey key_old, TKey key_new)
    {
        var nk_old = ManagedToNativeAllocRef(key_old);
        var nk_new = ManagedToNativeAllocRef(key_new);
        var r = eina_hash_move(Handle, nk_old, nk_new);
        NativeFreeRef<TKey>(nk_old);
        NativeFreeRef<TKey>(nk_new);
        return r;
    }

    void FreeBuckets()
    {
        eina_hash_free_buckets(Handle);
    }

    int Population()
    {
        return eina_hash_population(Handle);
    }

// TODO: implement when iterator is ready
//     public IEnumerator<KeyValuePair<TKey, TValue> > GetEnumerator()
//     {
//         for (IntPtr curr = Handle; curr != IntPtr.Zero; curr = InternalNext(curr))
//         {
//             yield return NativeToManaged<T>(InternalDataGet(curr));
//         }
//     }
//
//     System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
//     {
//         return this.GetEnumerator();
//     }
}

}

