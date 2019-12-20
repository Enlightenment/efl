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

using System;
using System.Runtime.InteropServices;
using System.ComponentModel;

using static eldbus.EldbusConnectionNativeFunctions;

namespace eldbus
{

[EditorBrowsable(EditorBrowsableState.Never)]
internal static class EldbusConnectionNativeFunctions
{
    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_connection_get(eldbus.Connection.Type type);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_private_connection_get(eldbus.Connection.Type type);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_address_connection_get(string address);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_private_address_connection_get(string address);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_connection_ref(IntPtr conn);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_connection_unref(IntPtr conn);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_connection_free_cb_add(IntPtr conn, IntPtr cb, IntPtr data);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_connection_free_cb_del(IntPtr conn, IntPtr cb, IntPtr data);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_connection_data_set(IntPtr conn, IntPtr key, IntPtr data);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_connection_data_get(IntPtr conn, IntPtr key);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_connection_data_del(IntPtr conn, IntPtr key);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_connection_event_callback_add(IntPtr conn, int type, IntPtr cb, IntPtr cb_data);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_connection_event_callback_del(IntPtr conn, int type, IntPtr cb, IntPtr cb_data);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_connection_send(IntPtr conn, IntPtr msg, IntPtr cb, IntPtr cb_data, double timeout);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_connection_unique_name_get(IntPtr conn);

    // FreeDesktop.Org Methods

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_name_request(IntPtr conn, string bus, uint flags, IntPtr cb, IntPtr cb_data);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_name_release(IntPtr conn, string bus, IntPtr cb, IntPtr cb_data);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_name_owner_get(IntPtr conn, string bus, IntPtr cb, IntPtr cb_data);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_name_owner_has(IntPtr conn, string bus, IntPtr cb, IntPtr cb_data);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_names_list(IntPtr conn, IntPtr cb, IntPtr cb_data);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_names_activatable_list(IntPtr conn, IntPtr cb, IntPtr cb_data);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_hello(IntPtr conn, IntPtr cb, IntPtr cb_data);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_name_start(IntPtr conn, string bus, uint flags, IntPtr cb, IntPtr cb_data);

//     typedef void (*Eldbus_Name_Owner_Changed_Cb)(void *data, const char *bus, const char *old_id, const char *new_id);
//     [DllImport(efl.Libs.Eldbus)] public static extern void
//         eldbus_name_owner_changed_callback_add(IntPtr conn, string bus, Eldbus_Name_Owner_Changed_Cb cb, IntPtr cb_data, [MarshalAs(UnmanagedType.U1)] bool allow_initial_call);
//     [DllImport(efl.Libs.Eldbus)] public static extern void
//         eldbus_name_owner_changed_callback_del(IntPtr conn, string bus, Eldbus_Name_Owner_Changed_Cb cb, IntPtr cb_data);
}

/// <summary>Represents a DBus connection.
/// <para>Since EFL 1.23.</para>
/// </summary>
public class Connection : IDisposable
{
    /// <summary>
    /// The type of the Connection.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public enum Type
    {
        /// <summary>
        /// Unknown type.
        /// <para>It's a sentinel.</para>
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        Unknown = 0, // sentinel, not a real type
        /// <summary>
        /// Session type.
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        Session,
        /// <summary>
        /// System type.
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        System,
        /// <summary>
        /// Starter type.
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        Starter,
        /// <summary>
        /// Address type.
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        Address,
        /// <summary>
        /// Last type.
        /// <para>It's a sentinel.</para>
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        Last         // sentinel, not a real type
    };

    [EditorBrowsable(EditorBrowsableState.Never)]
    public IntPtr Handle {get;set;} = IntPtr.Zero;
    /// <summary>Whether this wrapper owns the native handle.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public bool Own {get;set;} = true;

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
    public Connection(IntPtr handle, bool own)
    {
        InitNew(handle, own);
    }

    /// <summary>
    /// Constructor.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="type">The type of the connection.</param>
    public Connection(eldbus.Connection.Type type)
    {
        InitNew(eldbus_connection_get(type), true);
    }

    /// <summary>
    /// Constructor.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="address">The address of the connection.</param>
    public Connection(string address)
    {
        InitNew(eldbus_address_connection_get(address), true);
    }

    /// <summary>
    ///  Gets a Connection with a type.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="type"></param>
    /// <returns>A Connection with the type.</returns>
    public static eldbus.Connection GetPrivate(eldbus.Connection.Type type)
    {
        return new eldbus.Connection(eldbus_private_connection_get(type), true);
    }

    /// <summary>
    /// Gets a Connection with a address.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="address">The address of the connection.</param>
    public static eldbus.Connection GetPrivate(string address)
    {
        return new eldbus.Connection(eldbus_private_address_connection_get(address), true);
    }

    /// <summary>
    ///   Finalizer to be called from the Garbage Collector.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    ~Connection()
    {
        Dispose(false);
    }

    /// <summary>Disposes of this wrapper, releasing the native array if owned.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="disposing">True if this was called from <see cref="Dispose()"/> public method. False if
    /// called from the C# finalizer.</param>
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
                eldbus_connection_unref(h);
            }
            else
            {
                Efl.Eo.Globals.ThreadSafeFreeCbExec(eldbus_connection_unref, h);
            }
        }
    }

    /// <summary>Releases the native resources held by this instance.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
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
    /// Send a message.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="msg">The message that will be sent.</param>
    /// <param name="dlgt">The function that is executed when a response arrives..</param>
    /// <param name="timeout">The timeout of the message.</param>
    /// <returns>A <see cref="eldbus.Pending" /></returns>
    public eldbus.Pending Send(eldbus.Message msg, eldbus.MessageDelegate dlgt = null, double timeout = -1)
    {
        CheckHandle();

        if (msg == null)
        {
            throw new ArgumentNullException(nameof(msg));
        }

        IntPtr cb_wrapper = (dlgt == null ? IntPtr.Zero : eldbus.Common.GetMessageCbWrapperPtr());
        IntPtr cb_data = (dlgt == null ? IntPtr.Zero : Marshal.GetFunctionPointerForDelegate(dlgt));

        var pending_hdl = eldbus_connection_send(Handle, msg.Handle, cb_wrapper, cb_data, timeout);

        if (pending_hdl == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Pending' object from eldbus_connection_send");
        }

        msg.Ref();

        return new eldbus.Pending(pending_hdl, false);
    }

    /// <summary>
    /// Gets a unique name assigned by the message bus.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>The unique name string.</returns>
    public string GetUniqueName()
    {
        CheckHandle();
        var ptr = eldbus_connection_unique_name_get(Handle);
        if (ptr == IntPtr.Zero)
        {
            return null;
        }

        return Eina.StringConversion.NativeUtf8ToManagedString(ptr);
    }

    /// <summary>
    ///  Send a RequestName method.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="bus">The name of the bus.</param>
    /// <param name="flags">Parameter of the RequestName method.</param>
    /// <param name="dlgt">The function to call when receiving answer.</param>
    /// <returns>A <see cref="eldbus.Pending" /></returns>
    public eldbus.Pending NameRequest(string bus, uint flags, eldbus.MessageDelegate dlgt = null)
    {
        CheckHandle();

        if (bus == null)
        {
            throw new ArgumentNullException(nameof(bus));
        }

        IntPtr cb_wrapper = (dlgt == null ? IntPtr.Zero : eldbus.Common.GetMessageCbWrapperPtr());
        IntPtr cb_data = (dlgt == null ? IntPtr.Zero : Marshal.GetFunctionPointerForDelegate(dlgt));

        var pending_hdl = eldbus_name_request(Handle, bus, flags, cb_wrapper, cb_data);

        if (pending_hdl == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Pending' object from eldbus_name_request");
        }

        return new eldbus.Pending(pending_hdl, false);
    }

    /// <summary>
    /// Send a ReleaseName method.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="bus">The name of the bus.</param>
    /// <param name="dlgt">The function to call when receiving answer.</param>
    /// <returns>A <see cref="eldbus.Pending" /></returns>
    public eldbus.Pending NameRelease(string bus, eldbus.MessageDelegate dlgt = null)
    {
        CheckHandle();

        if (bus == null)
        {
            throw new ArgumentNullException(nameof(bus));
        }

        IntPtr cb_wrapper = (dlgt == null ? IntPtr.Zero : eldbus.Common.GetMessageCbWrapperPtr());
        IntPtr cb_data = (dlgt == null ? IntPtr.Zero : Marshal.GetFunctionPointerForDelegate(dlgt));

        var pending_hdl = eldbus_name_release(Handle, bus, cb_wrapper, cb_data);

        if (pending_hdl == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Pending' object from eldbus_name_release");
        }

        return new eldbus.Pending(pending_hdl, false);
    }

    /// <summary>
    /// Send a GetNameOwner method.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="bus">The name of the bus.</param>
    /// <param name="dlgt">The function to call when receiving answer.</param>
    /// <returns>A <see cref="eldbus.Pending" /></returns>
    public eldbus.Pending GetNameOwner(string bus, eldbus.MessageDelegate dlgt = null)
    {
        CheckHandle();

        if (bus == null)
        {
            throw new ArgumentNullException(nameof(bus));
        }

        IntPtr cb_wrapper = (dlgt == null ? IntPtr.Zero : eldbus.Common.GetMessageCbWrapperPtr());
        IntPtr cb_data = (dlgt == null ? IntPtr.Zero : Marshal.GetFunctionPointerForDelegate(dlgt));

        var pending_hdl = eldbus_name_owner_get(Handle, bus, cb_wrapper, cb_data);

        if (pending_hdl == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Pending' object from eldbus_name_owner_get");
        }

        return new eldbus.Pending(pending_hdl, false);
    }

    /// <summary>
    /// Send NameHasOwner method.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="bus">The name of the bus.</param>
    /// <param name="dlgt">The function to call when receiving the answer.</param>
    /// <returns>A <see cref="eldbus.Pending" /></returns>
    public eldbus.Pending HasNameOwner(string bus, eldbus.MessageDelegate dlgt = null)
    {
        CheckHandle();

        if (bus == null)
        {
            throw new ArgumentNullException(nameof(bus));
        }

        IntPtr cb_wrapper = (dlgt == null ? IntPtr.Zero : eldbus.Common.GetMessageCbWrapperPtr());
        IntPtr cb_data = (dlgt == null ? IntPtr.Zero : Marshal.GetFunctionPointerForDelegate(dlgt));

        var pending_hdl = eldbus_name_owner_has(Handle, bus, cb_wrapper, cb_data);

        if (pending_hdl == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Pending' object from eldbus_name_owner_has");
        }

        return new eldbus.Pending(pending_hdl, false);
    }

    /// <summary>
    /// Send ListNames method.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="dlgt">The function to call when receiving answer.</param>
    /// <returns>A <see cref="eldbus.Pending" /></returns>
    public eldbus.Pending NameList(eldbus.MessageDelegate dlgt = null)
    {
        CheckHandle();

        IntPtr cb_wrapper = (dlgt == null ? IntPtr.Zero : eldbus.Common.GetMessageCbWrapperPtr());
        IntPtr cb_data = (dlgt == null ? IntPtr.Zero : Marshal.GetFunctionPointerForDelegate(dlgt));

        var pending_hdl = eldbus_names_list(Handle, cb_wrapper, cb_data);

        if (pending_hdl == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Pending' object from eldbus_names_list");
        }

        return new eldbus.Pending(pending_hdl, false);
    }

    /// <summary>
    /// Send ListActivatableNames method.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="dlgt">The function to call when receiving a method.</param>
    /// <returns>A <see cref="eldbus.Pending" /></returns>
    public eldbus.Pending ActivatableList(eldbus.MessageDelegate dlgt = null)
    {
        CheckHandle();

        IntPtr cb_wrapper = (dlgt == null ? IntPtr.Zero : eldbus.Common.GetMessageCbWrapperPtr());
        IntPtr cb_data = (dlgt == null ? IntPtr.Zero : Marshal.GetFunctionPointerForDelegate(dlgt));

        var pending_hdl = eldbus_names_activatable_list(Handle, cb_wrapper, cb_data);

        if (pending_hdl == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Pending' object from eldbus_names_activatable_list");
        }

        return new eldbus.Pending(pending_hdl, false);
    }

    /// <summary>
    /// Send Hello method.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="dlgt">The function to call when receiving answer.</param>
    /// <returns>A <see cref="eldbus.Pending" /></returns>
    public eldbus.Pending Hello(eldbus.MessageDelegate dlgt = null)
    {
        CheckHandle();

        IntPtr cb_wrapper = (dlgt == null ? IntPtr.Zero : eldbus.Common.GetMessageCbWrapperPtr());
        IntPtr cb_data = (dlgt == null ? IntPtr.Zero : Marshal.GetFunctionPointerForDelegate(dlgt));

        var pending_hdl = eldbus_hello(Handle, cb_wrapper, cb_data);

        if (pending_hdl == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Pending' object from eldbus_hello");
        }

        return new eldbus.Pending(pending_hdl, false);
    }

    /// <summary>
    /// Send StartServiceByName method.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="bus">The name of the bus.</param>
    /// <param name="flags">Parameter of the StartServiceByName.</param>
    /// <param name="dlgt">The function to call when receiving answer.</param>
    /// <returns>A <see cref="eldbus.Pending" /></returns>
    public eldbus.Pending NameStart(string bus, uint flags, eldbus.MessageDelegate dlgt = null)
    {
        CheckHandle();

        if (bus == null)
        {
            throw new ArgumentNullException(nameof(bus));
        }

        IntPtr cb_wrapper = (dlgt == null ? IntPtr.Zero : eldbus.Common.GetMessageCbWrapperPtr());
        IntPtr cb_data = (dlgt == null ? IntPtr.Zero : Marshal.GetFunctionPointerForDelegate(dlgt));

        var pending_hdl = eldbus_name_start(Handle, bus, flags, cb_wrapper, cb_data);

        if (pending_hdl == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Pending' object from eldbus_name_start");
        }

        return new eldbus.Pending(pending_hdl, false);
    }

}

}
