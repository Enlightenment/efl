#pragma warning disable 1591

using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;

using static Eina.NativeCustomExportFunctions;

namespace Efl { namespace Eo {

public class Globals {
    [DllImport(efl.Libs.Eo)] public static extern void efl_object_init();
    [DllImport(efl.Libs.Eo)] public static extern void efl_object_shutdown();
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        _efl_add_internal_start([MarshalAs(UnmanagedType.LPStr)] String file, int line,
                                IntPtr klass, IntPtr parent, byte is_ref, byte is_fallback);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        _efl_add_end(IntPtr eo, byte is_ref, byte is_fallback);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        efl_ref(IntPtr eo);
    [DllImport(efl.Libs.CustomExports)] public static extern void
        efl_unref(IntPtr eo);
    [DllImport(efl.Libs.Eo)] public static extern int
        efl_ref_count(IntPtr eo);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0, IntPtr base1);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0, IntPtr base1, IntPtr base2);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0, IntPtr base1, IntPtr base2, IntPtr base3);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0, IntPtr base1, IntPtr base2, IntPtr base3, IntPtr base4);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0, IntPtr base1, IntPtr base2, IntPtr base3, IntPtr base4, IntPtr base5);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0, IntPtr base1, IntPtr base2, IntPtr base3, IntPtr base4, IntPtr base5, IntPtr base6);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0, IntPtr base1, IntPtr base2, IntPtr base3, IntPtr base4, IntPtr base5, IntPtr base6, IntPtr base7);
    [DllImport(efl.Libs.Eo)] public static extern byte efl_class_functions_set(IntPtr klass_id, IntPtr object_ops, IntPtr class_ops);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr efl_data_scope_get(IntPtr obj, IntPtr klass);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr efl_super(IntPtr obj, IntPtr klass);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr efl_class_get(IntPtr obj);
#if WIN32
    public static IntPtr RTLD_DEFAULT = new IntPtr(1);
#else
    public static IntPtr RTLD_DEFAULT = new IntPtr(0);
#endif
    [DllImport(efl.Libs.Evil)] public static extern IntPtr dlerror();
    [DllImport(efl.Libs.Evil)] public static extern IntPtr dlsym
       (IntPtr handle, [MarshalAs(UnmanagedType.LPStr)] String name);

   [DllImport(efl.Libs.Eo)] public static extern bool efl_event_callback_priority_add(
              System.IntPtr obj,
              IntPtr desc,
              short priority,
              Efl.EventCb cb,
              System.IntPtr data);
   [DllImport(efl.Libs.Eo)] public static extern bool efl_event_callback_del(
              System.IntPtr obj,
              IntPtr desc,
              Efl.EventCb cb,
              System.IntPtr data);
    [DllImport(efl.Libs.Eo)] public static extern IntPtr
        efl_object_legacy_only_event_description_get([MarshalAs(UnmanagedType.LPStr)] String name);

    public const int RTLD_NOW = 2;

    public delegate byte class_initializer(IntPtr klass);
    
    public static IntPtr register_class(class_initializer initializer, String class_name, IntPtr base_klass)
    {
        ClassDescription description;
        description.version = 2; // EO_VERSION
        description.name = class_name;
        description.class_type = 0; // REGULAR
        description.data_size = (UIntPtr)8;
        description.class_initializer = IntPtr.Zero;
        description.class_constructor = IntPtr.Zero;
        description.class_destructor = IntPtr.Zero;

        if(initializer != null)
            description.class_initializer = Marshal.GetFunctionPointerForDelegate(initializer);

        IntPtr description_ptr = Eina.MemoryNative.Alloc(Marshal.SizeOf(description));
        Marshal.StructureToPtr(description, description_ptr, false);
      
        Eina.Log.Debug("Going to register!");
        IntPtr klass = Efl.Eo.Globals.efl_class_new(description_ptr, base_klass, IntPtr.Zero);
        if(klass == IntPtr.Zero)
            Eina.Log.Error("klass was not registered");
        else
            Eina.Log.Debug("Registered class successfully");
        return klass;
    }
    public static IntPtr instantiate_start(IntPtr klass, Efl.Object parent)
    {
        Eina.Log.Debug($"Instantiating from klass 0x{klass.ToInt64():x}");
        System.IntPtr parent_ptr = System.IntPtr.Zero;
        if(parent != null)
            parent_ptr = parent.NativeHandle;

        System.IntPtr eo = Efl.Eo.Globals._efl_add_internal_start("file", 0, klass, parent_ptr, 1, 0);
        Console.WriteLine($"Eo instance right after internal_start 0x{eo.ToInt64():x} with refcount {Efl.Eo.Globals.efl_ref_count(eo)}");
        Console.WriteLine($"Parent was 0x{parent_ptr.ToInt64()}");
        return eo;
    }

    public static IntPtr instantiate_end(IntPtr eo) {
        Eina.Log.Debug("calling efl_add_internal_end");
        eo = Efl.Eo.Globals._efl_add_end(eo, 1, 0);
        Eina.Log.Debug($"efl_add_end returned eo 0x{eo.ToInt64():x}");
        return eo;
    }
    public static void data_set(Efl.Eo.IWrapper obj)
    {
      Eina.Log.Debug($"Calling data_scope_get with obj {obj.NativeHandle.ToInt64():x} and klass {obj.NativeClass.ToInt64():x}");
      IntPtr pd = Efl.Eo.Globals.efl_data_scope_get(obj.NativeHandle, obj.NativeClass);
      {
          GCHandle gch = GCHandle.Alloc(obj);
          EolianPD epd;
          epd.pointer = GCHandle.ToIntPtr(gch);
          Marshal.StructureToPtr(epd, pd, false);
      }
    }
    public static Efl.Eo.IWrapper data_get(IntPtr pd)
    {
        EolianPD epd = (EolianPD)Marshal.PtrToStructure(pd, typeof(EolianPD));
        if(epd.pointer != IntPtr.Zero)
        {
            GCHandle gch = GCHandle.FromIntPtr(epd.pointer);
            return (Efl.Eo.IWrapper)gch.Target;
        }
        else
            return null;
    }

    public static IntPtr cached_string_to_intptr(Dictionary<String, IntPtr> dict, String str)
    {
        IntPtr ptr = IntPtr.Zero;

        if (str == null)
            return ptr;

        if (!dict.TryGetValue(str, out ptr))
        {
            ptr = Eina.StringConversion.ManagedStringToNativeUtf8Alloc(str);
            dict[str] = ptr;
        }

        return ptr;
    }

    public static IntPtr cached_stringshare_to_intptr(Dictionary<String, IntPtr> dict, String str)
    {
        IntPtr ptr = IntPtr.Zero;

        if (str == null)
            return ptr;

        if (!dict.TryGetValue(str, out ptr))
        {
            ptr = Eina.Stringshare.eina_stringshare_add(str);
            dict[str] = ptr;
        }

        return ptr;
    }

    public static void free_dict_values(Dictionary<String, IntPtr> dict)
    {
        foreach(IntPtr ptr in dict.Values)
        {
            Eina.MemoryNative.Free(ptr);
        }
    }

    public static void free_stringshare_values(Dictionary<String, IntPtr> dict)
    {
        foreach(IntPtr ptr in dict.Values)
        {
            Eina.Stringshare.eina_stringshare_del(ptr);
        }
    }

    public static void free_gchandle(IntPtr ptr)
    {
        GCHandle handle = GCHandle.FromIntPtr(ptr);
        handle.Free();
    }

    public static System.Threading.Tasks.Task<Eina.Value> WrapAsync(Eina.Future future, CancellationToken token)
    {
        // Creates a task that will wait for SetResult for completion.
        // TaskCompletionSource is used to create tasks for 'external' Task sources.
        var tcs = new System.Threading.Tasks.TaskCompletionSource<Eina.Value>();

        // Flag to be passed to the cancell callback
        bool fulfilled = false;

        future.Then((Eina.Value received) => {
                lock (future)
                {
                    // Convert an failed Future to a failed Task.
                    if (received.GetValueType() == Eina.ValueType.Error)
                    {
                        Eina.Error err;
                        received.Get(out err);
                        if (err == Eina.Error.ECANCELED)
                            tcs.SetCanceled();
                        else
                            tcs.TrySetException(new Efl.FutureException(received));
                    }
                    else
                    {
                        // Will mark the returned task below as completed.
                        tcs.SetResult(received);
                    }
                    fulfilled = true;
                    return received;
                }
        });
        // Callback to be called when the token is cancelled.
        token.Register(() => {
                lock (future)
                {
                    // Will trigger the Then callback above with an Eina.Error
                    if (!fulfilled)
                        future.Cancel();
                }
        });

        return tcs.Task;
    }
} // Globals

public static class Config
{
    public static void Init()
    {
        Globals.efl_object_init();
    }

    public static void Shutdown()
    {
        Globals.efl_object_shutdown();
    }
}

public interface IWrapper
{
    /// <summary>Pointer to internal Eo instance.</summary>
    IntPtr NativeHandle
    {
        get;
    }
    /// <summary>Pointer to internal Eo class.</summary>
    IntPtr NativeClass 
    {
        get;
    }
}

public interface IOwnershipTag
{
}

public class OwnTag : IOwnershipTag
{
}

public class NonOwnTag : IOwnershipTag
{
}

public class MarshalTest<T, U> : ICustomMarshaler
    where U : IOwnershipTag
{
    public static ICustomMarshaler GetInstance(string cookie)
    {
        Eina.Log.Debug("MarshalTest.GetInstace cookie " + cookie);
        return new MarshalTest<T, U>();
    }
    public void CleanUpManagedData(object ManagedObj)
    {
        //Eina.Log.Warning("MarshalTest.CleanUpManagedData not implemented");
        //throw new NotImplementedException();
    }

    public void CleanUpNativeData(IntPtr pNativeData)
    {
        //Eina.Log.Warning("MarshalTest.CleanUpNativeData not implemented");
        //throw new NotImplementedException();
    }

    public int GetNativeDataSize()
    {
        Eina.Log.Debug("MarshalTest.GetNativeDataSize");
        return 0;
        //return 8;
    }

    public IntPtr MarshalManagedToNative(object ManagedObj)
    {
        Eina.Log.Debug("MarshalTest.MarshallManagedToNative");
        var r = ((IWrapper)ManagedObj).NativeHandle;
        if (typeof(U) == typeof(OwnTag))
            Efl.Eo.Globals.efl_ref(r);
        return r;
    }

    public object MarshalNativeToManaged(IntPtr pNativeData)
    {
        Eina.Log.Debug("MarshalTest.MarshalNativeToManaged");
        if (typeof(U) != typeof(OwnTag))
            Efl.Eo.Globals.efl_ref(pNativeData);
        return Activator.CreateInstance(typeof(T), new System.Object[] {pNativeData});
//        return null;
    }
}

public class StringPassOwnershipMarshaler : ICustomMarshaler {
    public object MarshalNativeToManaged(IntPtr pNativeData) {
        var ret = Eina.StringConversion.NativeUtf8ToManagedString(pNativeData);
        Eina.MemoryNative.Free(pNativeData);
        return ret;
    }

    public IntPtr MarshalManagedToNative(object managedObj) {
        return Eina.MemoryNative.StrDup((string)managedObj);
    }

    public void CleanUpNativeData(IntPtr pNativeData) {
        // No need to cleanup. C will take care of it.
    }

    public void CleanUpManagedData(object managedObj) {
    }

    public int GetNativeDataSize() {
        return -1;
    }

    public static ICustomMarshaler GetInstance(string cookie) {
        if (marshaler == null) {
            marshaler = new StringPassOwnershipMarshaler();
        }
        return marshaler;
    }
    static private StringPassOwnershipMarshaler marshaler;
}

public class StringKeepOwnershipMarshaler: ICustomMarshaler {
    public object MarshalNativeToManaged(IntPtr pNativeData) {
        return Eina.StringConversion.NativeUtf8ToManagedString(pNativeData);
    }

    public IntPtr MarshalManagedToNative(object managedObj) {
        return Eina.StringConversion.ManagedStringToNativeUtf8Alloc((string)managedObj);
    }

    public void CleanUpNativeData(IntPtr pNativeData) {
        // No need to free. The Native side will keep the ownership.
    }

    public void CleanUpManagedData(object managedObj) {
    }

    public int GetNativeDataSize() {
        return -1;
    }

    public static ICustomMarshaler GetInstance(string cookie) {
        if (marshaler == null) {
            marshaler = new StringKeepOwnershipMarshaler();
        }
        return marshaler;
    }
    static private StringKeepOwnershipMarshaler marshaler;
}

public class StringsharePassOwnershipMarshaler : ICustomMarshaler {
    public object MarshalNativeToManaged(IntPtr pNativeData) {
        var ret = Eina.StringConversion.NativeUtf8ToManagedString(pNativeData);
        Eina.Stringshare.eina_stringshare_del(pNativeData);
        return ret;
    }

    public IntPtr MarshalManagedToNative(object managedObj) {
        return Eina.Stringshare.eina_stringshare_add((string)managedObj);
    }

    public void CleanUpNativeData(IntPtr pNativeData) {
        // No need to free as it's for own() parameters.
    }

    public void CleanUpManagedData(object managedObj) {
    }

    public int GetNativeDataSize() {
        return -1;
    }

    public static ICustomMarshaler GetInstance(string cookie) {
        if (marshaler == null) {
            marshaler = new StringsharePassOwnershipMarshaler();
        }
        return marshaler;
    }
    static private StringsharePassOwnershipMarshaler marshaler;
}

public class StringshareKeepOwnershipMarshaler : ICustomMarshaler {
    public object MarshalNativeToManaged(IntPtr pNativeData) {
        return Eina.StringConversion.NativeUtf8ToManagedString(pNativeData);
    }

    public IntPtr MarshalManagedToNative(object managedObj) {
        return Eina.Stringshare.eina_stringshare_add((string)managedObj);
    }

    public void CleanUpNativeData(IntPtr pNativeData) {
        // No need to free, as the native side will keep ownership.
    }

    public void CleanUpManagedData(object managedObj) {
    }

    public int GetNativeDataSize() {
        return -1;
    }

    public static ICustomMarshaler GetInstance(string cookie) {
        if (marshaler == null) {
            marshaler = new StringshareKeepOwnershipMarshaler();
        }
        return marshaler;
    }
    static private StringshareKeepOwnershipMarshaler marshaler;
}

public class StrbufPassOwnershipMarshaler : ICustomMarshaler {
    public object MarshalNativeToManaged(IntPtr pNativeData) {
        return new Eina.Strbuf(pNativeData, Eina.Ownership.Managed);
    }

    public IntPtr MarshalManagedToNative(object managedObj) {
        Eina.Strbuf buf = managedObj as Eina.Strbuf;
        buf.ReleaseOwnership();
        return buf.Handle;
    }

    public void CleanUpNativeData(IntPtr pNativeData) {
        // No need to cleanup. C will take care of it.
    }

    public void CleanUpManagedData(object managedObj) {
    }

    public int GetNativeDataSize() {
        return -1;
    }

    public static ICustomMarshaler GetInstance(string cookie) {
        if (marshaler == null) {
            marshaler = new StrbufPassOwnershipMarshaler();
        }
        return marshaler;
    }
    static private StrbufPassOwnershipMarshaler marshaler;
}

public class StrbufKeepOwnershipMarshaler: ICustomMarshaler {
    public object MarshalNativeToManaged(IntPtr pNativeData) {
        return new Eina.Strbuf(pNativeData, Eina.Ownership.Unmanaged);
    }

    public IntPtr MarshalManagedToNative(object managedObj) {
        Eina.Strbuf buf = managedObj as Eina.Strbuf;
        return buf.Handle;
    }

    public void CleanUpNativeData(IntPtr pNativeData) {
        // No need to free. The Native side will keep the ownership.
    }

    public void CleanUpManagedData(object managedObj) {
    }

    public int GetNativeDataSize() {
        return -1;
    }

    public static ICustomMarshaler GetInstance(string cookie) {
        if (marshaler == null) {
            marshaler = new StrbufKeepOwnershipMarshaler();
        }
        return marshaler;
    }
    static private StrbufKeepOwnershipMarshaler marshaler;
}



} // namespace eo

/// <summary>General exception for errors inside the binding.</summary>
public class EflException : Exception
{
    /// <summary>Create a new EflException with the given message.</summary>
    public EflException(string message) : base(message)
    {
    }
}

/// <summary>Exception to be raised when a Task fails due to a failed Eina.Future.</summary>
public class FutureException : EflException
{
    /// <summary>The error code returned by the failed Eina.Future.</summary>
    public Eina.Error Error { get; private set; }

    /// <summary>Construct a new exception from the Eina.Error stored in the given Eina.Value.</summary>
    public FutureException(Eina.Value value) : base("Future failed.")
    {
        if (value.GetValueType() != Eina.ValueType.Error)
            throw new ArgumentException("FutureException must receive an Eina.Value with Eina.Error.");
        Eina.Error err;
        value.Get(out err);
        Error = err;
    }
}

} // namespace efl
