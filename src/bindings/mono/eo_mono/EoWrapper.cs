/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Threading;
using System.Reflection;
using System.Collections;

namespace Efl
{

namespace Eo
{

/// <summary>
/// Abstract class that delivers base level binding to Efl Objects.
///
/// Most of it is protected functionalities to serve the generated
/// binding classes that inherit from it.
///
/// <para>Since EFL 1.23.</para>
/// </summary>
public abstract class EoWrapper : IWrapper, IDisposable
{
    /// <summary>Object used to synchronize access to EFL events.</summary>
    private readonly object eflBindingEventLock = new object();
    private bool generated = true;
    private System.IntPtr handle = IntPtr.Zero;

    private static Efl.EventCb ownershipUniqueDelegate = new Efl.EventCb(OwnershipUniqueCallback);
    private static Efl.EventCb ownershipSharedDelegate = new Efl.EventCb(OwnershipSharedCallback);

    private Hashtable keyValueHash = null;

    /// <summary>Constructor to be used when objects are expected to be constructed from native code.
    /// <para>For a class that inherited from an EFL# class to be properly constructed from native code
    /// one must create a constructor with this signature and calls this base constructor from it.
    /// This constructor will take care of calling base constructors of the native classes and
    /// perform additional setup so objects are ready to use.</para>
    /// <para>It is advisable to check for the <see cref="NativeHandle"/> property in the top level
    /// constructor and signal an error when it has a value of IntPtr.Zero after this
    /// constructor completion.</para>
    /// <para>Warning: Do not use this constructor directly from a `new` statement.</para>
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    ///
    /// <param name="ch">Tag struct storing the native handle of the object being constructed.</param>
    protected EoWrapper(ConstructingHandle ch)
    {
        generated = false;
        handle = Efl.Eo.Globals.efl_constructor(Efl.Eo.Globals.efl_super(ch.NativeHandle, Efl.Eo.Globals.efl_class_get(ch.NativeHandle)));
        if (handle == IntPtr.Zero)
        {
            Eina.Log.Warning("Natice constructor returned NULL");
            return;
        }

        AddWrapperSupervisor();
        // Make an additional reference to C#
        // - Will also call EVENT_OWNERSHIP_SHARED
        Efl.Eo.Globals.efl_ref(handle);
    }

    /// <summary>Initializes a new instance of the <see cref="Object"/> class.
    /// <para>Internal usage: Constructs an instance from a native pointer. This is used when interacting with C code and should not be used directly.</para>
    /// <para>Do not implement this constructor.</para>
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="wh">The native pointer to be wrapped.</param>
    internal EoWrapper(Efl.Eo.WrappingHandle wh)
    {
        handle = wh.NativeHandle;
        AddWrapperSupervisor();
    }

    /// <summary>Initializes a new instance of the <see cref="Object"/> class.
    /// <para>Internal usage: Constructor to actually call the native library constructors. C# subclasses
    /// must use the public constructor only.</para>
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="baseKlass">The pointer to the base native Eo class.</param>
    /// <param name="parent">The Efl.Object parent of this instance.</param>
    /// <param name="file">Name of the file from where the constructor is called.</param>
    /// <param name="line">Number of the line from where the constructor is called.</param>
    protected EoWrapper(IntPtr baseKlass, Efl.Object parent,
                        [CallerFilePath] string file = null,
                        [CallerLineNumber] int line = 0)
    {
        generated = Efl.Eo.BindingEntityAttribute.IsBindingEntity(((object)this).GetType());
        IntPtr actual_klass = baseKlass;
        if (!generated)
        {
            actual_klass = Efl.Eo.ClassRegister.GetInheritKlassOrRegister(baseKlass, ((object)this).GetType());
        }

        // Creation of the unfinalized Eo handle
        Eina.Log.Debug($"Instantiating from klass 0x{actual_klass.ToInt64():x}");
        System.IntPtr parent_ptr = System.IntPtr.Zero;
        if (parent != null)
        {
            parent_ptr = parent.NativeHandle;
        }

        if (generated)
        {
            handle = Efl.Eo.Globals._efl_add_internal_start(file, line, actual_klass, parent_ptr, 1, 0);
        }
        else
        {
            handle = Efl.Eo.Globals._efl_add_internal_start_bindings(file, line, actual_klass, parent_ptr, 1, 0,
                                                                     Efl.Eo.Globals.efl_mono_avoid_top_level_constructor_callback_addr_get(),
                                                                     IntPtr.Zero);
        }

        if (handle == System.IntPtr.Zero)
        {
            throw new Exception("Instantiation failed");
        }

        Eina.Log.Debug($"Eo instance right after internal_start 0x{handle.ToInt64():x} with refcount {Efl.Eo.Globals.efl_ref_count(handle)}");
        Eina.Log.Debug($"Parent was 0x{parent_ptr.ToInt64()}");

        // Creation of wrapper supervisor
        AddWrapperSupervisor();
    }

    /// <summary>Destructor.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    ~EoWrapper()
    {
        Dispose(false);
    }

    /// <summary>Pointer to the native instance.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public System.IntPtr NativeHandle
    {
        get { return handle; }
    }

    /// <summary>Pointer to the native class description.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public abstract System.IntPtr NativeClass
    {
        get;
    }

    /// <summary>
    /// Whether this object type is one of the generated binding classes or a custom
    /// class defined by the user and that inherit from one of the generated ones.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>
    /// True if this object type is one of the generated binding classes,
    /// false if it is class that is manually defined and that inherits from
    /// one of the generated ones, including user defined classes.
    /// </returns>
    protected bool IsGeneratedBindingClass
    {
        get { return generated; }
    }

    /// <summary>Releases the underlying native instance.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    protected virtual void Dispose(bool disposing)
    {
        if (disposing && handle != System.IntPtr.Zero)
        {
            IntPtr h = handle;
            handle = IntPtr.Zero;
            Efl.Eo.Globals.efl_mono_native_dispose(h);
        }
        else
        {
            Monitor.Enter(Efl.All.InitLock);
            if (Efl.All.MainLoopInitialized)
            {
                Efl.Eo.Globals.efl_mono_thread_safe_native_dispose(handle);
            }

            Monitor.Exit(Efl.All.InitLock);
        }
    }

    /// <summary>Turns the native pointer into a string representation.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>A string with the type and the native pointer for this object.</returns>
    public override String ToString()
    {
        return $"{this.GetType().Name}@[0x{(UInt64)handle:x}]";
    }

    /// <summary>Releases the underlying native instance.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    /// <summary>Releases the underlying Eo object.
    ///
    /// This method is a C# counterpart to the C `efl_del` function. It removes the parent of the object
    /// and releases the Eo reference it was holding.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public void Del()
    {
        // FIXME Implement this
        ((Efl.Object)this).Parent = null;
        Dispose();
    }

    /// <summary>Finishes instantiating this object.
    /// <para>Internal usage by generated code.</para>
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    protected void FinishInstantiation()
    {
        Eina.Log.Debug("calling efl_add_internal_end");
        var h = Efl.Eo.Globals._efl_add_end(handle, 1, 0);
        Eina.Log.Debug($"efl_add_end returned eo 0x{handle.ToInt64():x}");

        // if (h == IntPtr.Zero) // TODO
        // {
        // }

        handle = h;
    }

    /// <summary>Adds a new event handler, registering it to the native event.
    /// <para>For internal use only.</para>
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="lib">The name of the native library definining the event.</param>
    /// <param name="key">The name of the native event.</param>
    /// <param name="evtCaller">Delegate to be called by native code on event raising.</param>
    /// <param name="evtDelegate">Managed delegate that will be called by evtCaller on event raising.</param>
    internal void AddNativeEventHandler(string lib, string key, Efl.EventCb evtCaller, object evtDelegate)
    {
        lock (eflBindingEventLock)
        {
            IntPtr desc = Efl.EventDescription.GetNative(lib, key);
            if (desc == IntPtr.Zero)
            {
                Eina.Log.Error($"Failed to get native event {key}");
                return;
            }

            var wsPtr = Efl.Eo.Globals.efl_mono_wrapper_supervisor_get(handle);
            var ws = Efl.Eo.Globals.WrapperSupervisorPtrToManaged(wsPtr);
            if (ws.EoEvents.ContainsKey((desc, evtDelegate)))
            {
                Eina.Log.Warning($"Event proxy for event {key} already registered!");
                return;
            }

            IntPtr evtCallerPtr = Marshal.GetFunctionPointerForDelegate(evtCaller);
            if (!Efl.Eo.Globals.efl_event_callback_priority_add(handle, desc, 0, evtCallerPtr, wsPtr))
            {
                Eina.Log.Error($"Failed to add event proxy for event {key}");
                return;
            }

            ws.EoEvents[(desc, evtDelegate)] = (evtCallerPtr, evtCaller);
            Eina.Error.RaiseIfUnhandledException();
        }
    }

    /// <summary>Removes the given event handler for the given event.
    /// <para>For internal use only.</para>
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="lib">The name of the native library definining the event.</param>
    /// <param name="key">The name of the native event.</param>
    /// <param name="evtDelegate">The delegate to be removed.</param>
    internal void RemoveNativeEventHandler(string lib, string key, object evtDelegate)
    {
        lock (eflBindingEventLock)
        {
            IntPtr desc = Efl.EventDescription.GetNative(lib, key);
            if (desc == IntPtr.Zero)
            {
                Eina.Log.Error($"Failed to get native event {key}");
                return;
            }

            var wsPtr = Efl.Eo.Globals.efl_mono_wrapper_supervisor_get(handle);
            var ws = Efl.Eo.Globals.WrapperSupervisorPtrToManaged(wsPtr);
            var evtPair = (desc, evtDelegate);
            if (ws.EoEvents.TryGetValue(evtPair, out var caller))
            {
                if (!Efl.Eo.Globals.efl_event_callback_del(handle, desc, caller.evtCallerPtr, wsPtr))
                {
                    Eina.Log.Error($"Failed to remove event proxy for event {key}");
                    return;
                }

                ws.EoEvents.Remove(evtPair);
                Eina.Error.RaiseIfUnhandledException();
            }
            else
            {
                Eina.Log.Error($"Trying to remove proxy for event {key} when it is not registered.");
            }
        }
    }

    internal Efl.EventCb GetInternalEventCallback<T>(EventHandler<T> handler, Func<IntPtr, T> createArgsInstance) where T:EventArgs
    {
        return (IntPtr data, ref Efl.Event evt) =>
        {
           var obj = Efl.Eo.Globals.WrapperSupervisorPtrToManaged(data).Target;
           if (obj != null)
           {
              try
              {
                 handler?.Invoke(obj, createArgsInstance(evt.Info));
              }
              catch (Exception e)
              {
                 Eina.Log.Error(e.ToString());
                 Eina.Error.Set(Eina.Error.UNHANDLED_EXCEPTION);
              }
           }
        };
    }

    internal Efl.EventCb GetInternalEventCallback(EventHandler handler)
    {
        return (IntPtr data, ref Efl.Event evt) =>
        {
           var obj = Efl.Eo.Globals.WrapperSupervisorPtrToManaged(data).Target;
           if (obj != null)
           {
              try
              {
                 handler?.Invoke(obj, EventArgs.Empty);
              }
              catch (Exception e)
              {
                 Eina.Log.Error(e.ToString());
                 Eina.Error.Set(Eina.Error.UNHANDLED_EXCEPTION);
              }
           }
        };
    }

    internal void CallNativeEventCallback(string lib, string key, IntPtr eventInfo, Action<IntPtr> freeAction)
    {
        try
        {
            IntPtr desc = Efl.EventDescription.GetNative(lib, key);
            if (desc == IntPtr.Zero)
                throw new ArgumentException($"Failed to get native event {key}", "key");

            Efl.Eo.Globals.CallEventCallback(NativeHandle, desc, eventInfo);
        }
        finally
        {
            if (freeAction != null)
               freeAction(eventInfo);
        }
    }

    private static void OwnershipUniqueCallback(IntPtr data, ref Efl.Event evt)
    {
        var ws = Efl.Eo.Globals.WrapperSupervisorPtrToManaged(data);
        ws.MakeUnique();
    }

    private static void OwnershipSharedCallback(IntPtr data, ref Efl.Event evt)
    {
        var ws = Efl.Eo.Globals.WrapperSupervisorPtrToManaged(data);
        ws.MakeShared();
    }

    /// <summary>Create and set to the internal native state a C# supervisor for this Eo wrapper. For internal use only.</summary>
    private void AddWrapperSupervisor()
    {
        var ws = new Efl.Eo.WrapperSupervisor(this);
        Efl.Eo.Globals.SetWrapperSupervisor(handle, ws);
        if (Efl.Eo.Globals.efl_ref_count(handle) > 1)
        {
            ws.MakeShared();
        }

        AddOwnershipEventHandlers();
    }

    /// <summary>Register handlers to ownership events, in order to control the object lifetime. For internal use only.</summary>
    private void AddOwnershipEventHandlers()
    {
        AddNativeEventHandler("eo", "_EFL_EVENT_INVALIDATE", ownershipUniqueDelegate, ownershipUniqueDelegate);
        AddNativeEventHandler("eo", "_EFL_EVENT_OWNERSHIP_UNIQUE", ownershipUniqueDelegate, ownershipUniqueDelegate);
        AddNativeEventHandler("eo", "_EFL_EVENT_OWNERSHIP_SHARED", ownershipSharedDelegate, ownershipSharedDelegate);
        Eina.Error.RaiseIfUnhandledException();
    }

    /// <summary>
    /// Struct to be used when constructing objects from native code.
    /// <para>For internal use by generated code only.</para>
    /// <para>Wraps the pointer handle to the native object instance.</para>
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    protected struct ConstructingHandle : IEquatable<ConstructingHandle>
    {
        /// <summary>Constructor for wrapping the native handle.
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        public ConstructingHandle(IntPtr h)
        {
            NativeHandle = h;
        }

        /// <summary>Pointer to the native instance.
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        public IntPtr NativeHandle { get; private set; }

        /// <summary>
        ///   Gets a hash for <see cref="ConstructingHandle" />.
        /// <para>Since EFL 1.24.</para>
        /// </summary>
        /// <returns>A hash code.</returns>
        public override int GetHashCode() => NativeHandle.GetHashCode();

        /// <summary>Returns whether this <see cref="ConstructingHandle" />
        /// is equal to the given <see cref="object" />.
        /// <para>Since EFL 1.24.</para>
        /// </summary>
        /// <param name="other">The <see cref="object" /> to be compared to.</param>
        /// <returns><c>true</c> if is equal to <c>other</c>.</returns>
        public override bool Equals(object other)
            => (!(other is ConstructingHandle))
            ? false : Equals((ConstructingHandle)other);

        /// <summary>Returns whether this <see cref="ConstructingHandle" /> is equal
        /// to the given <see cref="ConstructingHandle" />.
        /// <para>Since EFL 1.24.</para>
        /// </summary>
        /// <param name="other">The <see cref="ConstructingHandle" /> to be compared to.</param>
        /// <returns><c>true</c> if is equal to <c>other</c>.</returns>
        public bool Equals(ConstructingHandle other)
            => NativeHandle == other.NativeHandle;

        /// <summary>Returns whether <c>lhs</c> is equal to <c>rhs</c>.
        /// <para>Since EFL 1.24.</para>
        /// </summary>
        /// <param name="lhs">The left hand side of the operator.</param>
        /// <param name="rhs">The right hand side of the operator.</param>
        /// <returns><c>true</c> if <c>lhs</c> is equal
        /// to <c>rhs</c>.</returns>
        public static bool operator==(ConstructingHandle lhs, ConstructingHandle rhs)
            => lhs.Equals(rhs);

        /// <summary>Returns whether <c>lhs</c> is not equal to <c>rhs</c>.
        /// <para>Since EFL 1.24.</para>
        /// </summary>
        /// <param name="lhs">The left hand side of the operator.</param>
        /// <param name="rhs">The right hand side of the operator.</param>
        /// <returns><c>true</c> if <c>lhs</c> is not equal
        /// to <c>rhs</c>.</returns>
        public static bool operator!=(ConstructingHandle lhs, ConstructingHandle rhs)
            => !(lhs == rhs);
    }

    /// <summary>
    /// Set a value object associated with a key object.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public void SetKeyValue(object key, object val)
    {
        if (keyValueHash == null)
            keyValueHash = new Hashtable();

        keyValueHash.Add(key, val);
    }

    /// <summary>
    /// Get a value object associated with a key object.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public object GetKeyValue(object key)
    {
        if (keyValueHash == null)
            return null;

        return keyValueHash[key];
    }

    /// <summary>Wrapper for native methods and virtual method delegates.
    /// <para>For internal use by generated code only.</para>
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    internal abstract class NativeMethods : Efl.Eo.NativeClass
    {
        private static EflConstructorDelegate csharpEflConstructorStaticDelegate = new EflConstructorDelegate(Constructor);
        private static Efl.Eo.NativeModule EoModule = new Efl.Eo.NativeModule("eo");

        private delegate IntPtr EflConstructorDelegate(IntPtr obj, IntPtr pd);

        /// <summary>Gets the list of Eo operations to override.
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        /// <returns>The list of Eo operations to be overload.</returns>
        internal override System.Collections.Generic.List<EflOpDescription> GetEoOps(Type type, bool includeInherited)
        {
            var descs = new System.Collections.Generic.List<EflOpDescription>();

            descs.Add(new EflOpDescription()
            {
                api_func = Efl.Eo.FunctionInterop.LoadFunctionPointer(EoModule.Module, "efl_constructor"),
                func = Marshal.GetFunctionPointerForDelegate(csharpEflConstructorStaticDelegate)
            });

            return descs;
        }

        private static IntPtr Constructor(IntPtr obj, IntPtr pd)
        {
            try
            {
                var eoKlass = Efl.Eo.Globals.efl_class_get(obj);
                var managedType = ClassRegister.GetManagedType(eoKlass);
                if (managedType == null)
                {
                    IntPtr nativeName = Efl.Eo.Globals.efl_class_name_get(eoKlass);
                    var name = Eina.StringConversion.NativeUtf8ToManagedString(nativeName);
                    Eina.Log.Warning($"Can't get Managed class for object handle 0x{(UInt64)obj:x} with native class [{name}]");
                    return IntPtr.Zero;
                }

                var flags = BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic;
                ConstructorInfo constructor = managedType.GetConstructor(flags, null, new Type[1] { typeof(ConstructingHandle) }, null);
                if (constructor == null)
                {
                    Eina.Log.Error($"Type {managedType.FullName} lacks a constructor that receives a ConstructingHandle. It can not be constructed from native code.");
                    return IntPtr.Zero;
                }

                var eoWrapper = (Efl.Eo.IWrapper) constructor.Invoke(new object[1] { new ConstructingHandle(obj) });
                if (eoWrapper == null)
                {
                    Eina.Log.Warning("Constructor was unable to create a new object");
                    return IntPtr.Zero;
                }

                return eoWrapper.NativeHandle;
            }
            catch (Exception e)
            {
                Eina.Log.Warning($"Inherited constructor error: {e.ToString()}");
                Eina.Error.Set(Eina.Error.UNHANDLED_EXCEPTION);
                return IntPtr.Zero;
            }
        }
    }
}

} // namespace Eo

/// <summary>Concrete realization of Efl.Object.
///
/// Some legacy classes (like Evas.Canvas) may be returned by some methods. As these classes are not bound, we
/// allow minimal interaction with them through <see cref="Efl.Object" />.
///
/// But as <see cref="Efl.Object" /> is abstract, whis realized class will allow us to create C# instances of it.</summary>
[Efl.Object.NativeMethods]
internal class ObjectRealized : Efl.Object
{
    internal ObjectRealized(Efl.Eo.WrappingHandle ch) : base(ch) { }
}

} // namespace Efl
