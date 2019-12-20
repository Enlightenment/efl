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

#pragma warning disable 1591

using System.Runtime.InteropServices;
using System.ComponentModel;

using static eldbus.EldbusObjectNativeFunctions;

using IntPtr = System.IntPtr;

namespace eldbus
{

[EditorBrowsable(EditorBrowsableState.Never)]
internal static class EldbusObjectNativeFunctions
{
    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_object_get(IntPtr conn, string bus, string path);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_object_ref(IntPtr obj);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_object_unref(IntPtr obj);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_object_free_cb_add(IntPtr obj, IntPtr cb, IntPtr data);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_object_free_cb_del(IntPtr obj, IntPtr cb, IntPtr data);

// typedef enum
// {
//    ELDBUS_OBJECT_EVENT_IFACE_ADDED = 0, /**< a parent path must have a ObjectManager interface */
//    ELDBUS_OBJECT_EVENT_IFACE_REMOVED, /**< a parent path must have a ObjectManager interface */
//    ELDBUS_OBJECT_EVENT_PROPERTY_CHANGED, /**< a property has changes */
//    ELDBUS_OBJECT_EVENT_PROPERTY_REMOVED, /**< a property was removed */
//    ELDBUS_OBJECT_EVENT_DEL,
//    ELDBUS_OBJECT_EVENT_LAST    /**< sentinel, not a real event type */
// } Eldbus_Object_Event_Type;
//
//     [DllImport(efl.Libs.Eldbus)] public static extern void
//         eldbus_object_event_callback_add(IntPtr obj, Eldbus_Object_Event_Type type, IntPtr cb, IntPtr cb_data);
//
//     [DllImport(efl.Libs.Eldbus)] public static extern void
//         eldbus_object_event_callback_del(IntPtr obj, Eldbus_Object_Event_Type type, IntPtr cb, IntPtr cb_data);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_object_connection_get(IntPtr obj);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_object_bus_name_get(IntPtr obj);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_object_path_get(IntPtr obj);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_object_send(IntPtr obj, IntPtr msg, IntPtr cb, IntPtr cb_data, double timeout);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_object_signal_handler_add(IntPtr obj, string _interface, string member, IntPtr cb, IntPtr cb_data);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_object_method_call_new(IntPtr obj, string _interface, string member);

    // FreeDesktop.Org Methods

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_object_peer_ping(IntPtr obj, IntPtr cb, IntPtr data);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_object_peer_machine_id_get(IntPtr obj, IntPtr cb, IntPtr data);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_object_introspect(IntPtr obj, IntPtr cb, IntPtr data);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_object_managed_objects_get(IntPtr obj, IntPtr cb, IntPtr data);

//     [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
//         eldbus_object_manager_interfaces_added(IntPtr obj, Eldbus_Signal_Cb cb, IntPtr cb_data);
//
//     [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
//         eldbus_object_manager_interfaces_removed(IntPtr obj, Eldbus_Signal_Cb cb, IntPtr cb_data);
}

/// <summary>Represents a DBus object.
/// <para>Since EFL 1.23.</para>
/// </summary>
public class Object : System.IDisposable
{
    [EditorBrowsable(EditorBrowsableState.Never)]
    public IntPtr Handle {get;set;} = IntPtr.Zero;
    /// <summary>Whether this managed list owns the native one.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public bool Own {get;set;} = true;

    [EditorBrowsable(EditorBrowsableState.Never)]
    private void InitNew(IntPtr handle, bool own)
    {
        Handle = handle;
        Own = own;
        CheckHandle();
    }

    private void CheckHandle()
    {
        if (Handle == IntPtr.Zero)
        {
            eldbus.Common.RaiseNullHandle();
        }
    }

    [EditorBrowsable(EditorBrowsableState.Never)]
    public Object(IntPtr handle, bool own)
    {
        InitNew(handle, own);
    }

    /// <summary>
    /// Constructor.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="conn"><see cref="eldbus.Connection" /> where object
    /// belongs.</param>
    /// <param name="bus">The name of the bus or unique id who listens for calls
    /// of this object</param>
    /// <param name="path">The object path of this object.</param>
    public Object(eldbus.Connection conn, string bus, string path)
    {
        if (conn == null)
        {
            throw new System.ArgumentNullException(nameof(conn));
        }

        if (bus == null)
        {
            throw new System.ArgumentNullException(nameof(bus));
        }

        if (path == null)
        {
            throw new System.ArgumentNullException(nameof(path));
        }

        var handle = eldbus_object_get(conn.Handle, bus, path);

        if (handle == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Object' object from eldbus_object_get");
        }

        InitNew(handle, true);
    }

    /// <summary>Finalizes with garbage collector.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    ~Object()
    {
        Dispose(false);
    }


    /// <summary>Disposes of this list.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="disposing">Whether this was called from the finalizer (<c>false</c>) or from the
    /// <see cref="Dispose()"/> method.</param>
    protected virtual void Dispose(bool disposing)
    {
        IntPtr h = Handle;
        Handle = IntPtr.Zero;
        if (h == IntPtr.Zero)
        {
            return;
        }

        if (Own)
        {
            if (disposing)
            {
                eldbus_object_unref(h);
            }
            else
            {
                Efl.Eo.Globals.ThreadSafeFreeCbExec(eldbus_object_unref, h);
            }
        }
    }

    /// <summary>Disposes of this list.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public void Dispose()
    {
        Dispose(true);
        System.GC.SuppressFinalize(this);
    }

    /// <summary>Releases the native resources held by this instance.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public void Free()
    {
        Dispose();
    }

    /// <summary>
    ///   Releases the native handler.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>The native handler.</returns>
    public IntPtr Release()
    {
        IntPtr h = Handle;
        Handle = IntPtr.Zero;
        return h;
    }

    /// <summary>
    ///   Get the <see cref="eldbus.Connection" /> object associated with a
    /// <see cref="eldbus.Object" />
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>The <see cref="eldbus.Connection" /> object</returns>
    public eldbus.Connection GetConnection()
    {
        CheckHandle();
        var conn = eldbus_object_connection_get(Handle);

        if (conn == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Connection' object from eldbus_object_connection_get");
        }

        return new eldbus.Connection(conn, false);
    }

    /// <summary>
    ///   Get the name associated with a <see cref="eldbus.Object" />
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>A string corresponding to the <see cref="eldbus.Object" />
    /// name</returns>
    public string GetBusName()
    {
        CheckHandle();
        var ptr = eldbus_object_bus_name_get(Handle);
        return Eina.StringConversion.NativeUtf8ToManagedString(ptr);
    }

    /// <summary>
    ///   Get the path associated with a <see cref="eldbus.Object" />
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>A string corresponding to the <see cref="eldbus.Object" />
    /// path.</returns>
    public string GetPath()
    {
        CheckHandle();
        var ptr = eldbus_object_path_get(Handle);
        return Eina.StringConversion.NativeUtf8ToManagedString(ptr);
    }

    /// <summary>
    ///   Increse object reference
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public void Ref()
    {
        CheckHandle();
        eldbus_object_ref(Handle);
    }

    /// <summary>
    ///   Decrease object reference.
    /// <para>If reference == 0 object will be freed and all its children.</para>
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public void Unref()
    {
        CheckHandle();
        eldbus_object_unref(Handle);
    }

    /// <summary>
    ///   Send a message
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="msg">The message will be sent in connection to this object.</param>
    /// <param name="dlgt">The function to call when receiving answer.</param>
    /// <param name="timeout">Timeout.</param>
    /// <returns>A <see cref="eldbus.Pending" /></returns>
    public eldbus.Pending Send(eldbus.Message msg, eldbus.MessageDelegate dlgt = null, double timeout = -1)
    {
        CheckHandle();

        if (msg == null)
        {
            throw new System.ArgumentNullException(nameof(msg));
        }

        IntPtr cb_wrapper = dlgt == null ? IntPtr.Zero : eldbus.Common.GetMessageCbWrapperPtr();
        IntPtr cb_data = dlgt == null ? IntPtr.Zero : Marshal.GetFunctionPointerForDelegate(dlgt);

        var pending_hdl = eldbus_object_send(Handle, msg.Handle, cb_wrapper, cb_data, timeout);

        if (pending_hdl == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Pending' object from eldbus_object_send");
        }

        return new eldbus.Pending(pending_hdl, false);
    }

    /// <summary>
    ///   Call a dbus method on the <see cref="eldbus.Object" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="_interface">The interface name.</param>
    /// <param name="member">Name of the method to be called.</param>
    /// <returns>A new <see cref="eldbus.Message" /></returns>
    public eldbus.Message NewMethodCall(string _interface, string member)
    {
        CheckHandle();

        var hdl = eldbus_object_method_call_new(Handle, _interface, member);

        if (hdl == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Message' object from eldbus_object_method_call_new");
        }

        return new eldbus.Message(hdl, false);
    }

    /// <summary>
    ///   Call the method Ping on the eldbus.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="dlgt">The function to call when receiving answer.</param>
    /// <returns>A <see cref="eldbus.Pending" /></returns>
    public eldbus.Pending PeerPing(eldbus.MessageDelegate dlgt = null)
    {
        CheckHandle();

        IntPtr cb_wrapper = dlgt == null ? IntPtr.Zero : eldbus.Common.GetMessageCbWrapperPtr();
        IntPtr cb_data = dlgt == null ? IntPtr.Zero : Marshal.GetFunctionPointerForDelegate(dlgt);

        var pending_hdl = eldbus_object_peer_ping(Handle, cb_wrapper, cb_data);

        if (pending_hdl == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Pending' object from eldbus_object_peer_ping");
        }

        return new eldbus.Pending(pending_hdl, false);
    }

    /// <summary>
    ///   Call the method GetMachineId on the eldbus.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="dlgt">The function to call when receiving answer.</param>
    /// <returns>A <see cref="eldbus.Pending" /></returns>
    public eldbus.Pending GetPeerMachineId(eldbus.MessageDelegate dlgt = null)
    {
        CheckHandle();

        IntPtr cb_wrapper = dlgt == null ? IntPtr.Zero : eldbus.Common.GetMessageCbWrapperPtr();
        IntPtr cb_data = dlgt == null ? IntPtr.Zero : Marshal.GetFunctionPointerForDelegate(dlgt);

        var pending_hdl = eldbus_object_peer_machine_id_get(Handle, cb_wrapper, cb_data);

        if (pending_hdl == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Pending' object from eldbus_object_peer_machine_id_get");
        }

        return new eldbus.Pending(pending_hdl, false);
    }

    /// <summary>
    ///   Call the method Introspect on the eldbus.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="dlgt">The function to call when receiving answer.</param>
    /// <returns>A <see cref="eldbus.Pending" /></returns>
    public eldbus.Pending Introspect(eldbus.MessageDelegate dlgt = null)
    {
        CheckHandle();

        IntPtr cb_wrapper = dlgt == null ? IntPtr.Zero : eldbus.Common.GetMessageCbWrapperPtr();
        IntPtr cb_data = dlgt == null ? IntPtr.Zero : Marshal.GetFunctionPointerForDelegate(dlgt);

        var pending_hdl = eldbus_object_introspect(Handle, cb_wrapper, cb_data);

        if (pending_hdl == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Pending' object from eldbus_object_introspect");
        }

        return new eldbus.Pending(pending_hdl, false);
    }

    /// <summary>
    ///   Call the method GetmanagedObjects on eldbus.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="dlgt">The function to call when receiving answer.</param>
    /// <returns>A <see cref="eldbus.Pending" /></returns>
    public eldbus.Pending GetManagedObjects(eldbus.MessageDelegate dlgt = null)
    {
        CheckHandle();

        IntPtr cb_wrapper = dlgt == null ? IntPtr.Zero : eldbus.Common.GetMessageCbWrapperPtr();
        IntPtr cb_data = dlgt == null ? IntPtr.Zero : Marshal.GetFunctionPointerForDelegate(dlgt);

        var pending_hdl = eldbus_object_managed_objects_get(Handle, cb_wrapper, cb_data);

        if (pending_hdl == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Pending' object from eldbus_object_managed_objects_get");
        }

        return new eldbus.Pending(pending_hdl, false);
    }
}

}
