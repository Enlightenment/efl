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
using System.Diagnostics.Contracts;

using static eldbus.EldbusProxyNativeFunctions;

namespace eldbus
{

[EditorBrowsable(EditorBrowsableState.Never)]
internal static class EldbusProxyNativeFunctions
{
    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_proxy_get(IntPtr obj, string _interface);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_proxy_ref(IntPtr proxy);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_proxy_unref(IntPtr proxy);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_proxy_object_get(IntPtr proxy);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_proxy_interface_get(IntPtr proxy);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_proxy_data_set(IntPtr proxy, string key, IntPtr data);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_proxy_data_get(IntPtr proxy, string key);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_proxy_data_del(IntPtr proxy, string key);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_proxy_free_cb_add(IntPtr proxy, IntPtr cb, IntPtr data);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_proxy_free_cb_del(IntPtr proxy, IntPtr cb, IntPtr data);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_proxy_method_call_new(IntPtr proxy, string member);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_proxy_send(IntPtr proxy, IntPtr msg, IntPtr cb, IntPtr cb_data, double timeout);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_proxy_send_and_block(IntPtr proxy, IntPtr msg, double timeout);

//     [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
//         eldbus_proxy_call(IntPtr proxy, string member, IntPtr cb, IntPtr cb_data, double timeout, string signature, ...);
//
//     [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
//         eldbus_proxy_vcall(IntPtr proxy, string member, IntPtr cb, IntPtr cb_data, double timeout, string signature, va_list ap);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_proxy_signal_handler_add(IntPtr proxy, string member, IntPtr cb, IntPtr cb_data);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_proxy_event_callback_add(IntPtr proxy, int type, IntPtr cb, IntPtr cb_data);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_proxy_event_callback_del(IntPtr proxy, int type, IntPtr cb, IntPtr cb_data);
}

/// <summary>Represents a DBus proxy object.
/// <para>Since EFL 1.23.</para>
/// </summary>
public class Proxy : IDisposable
{
    [EditorBrowsable(EditorBrowsableState.Never)]
    public IntPtr Handle {get;set;} = IntPtr.Zero;
    /// <summary>Whether this managed list owns the native one.
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
    public Proxy(IntPtr handle, bool own)
    {
        InitNew(handle, own);
    }

    /// <summary>
    ///   Constructor
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="obj">The <see cref="eldbus.Object" />.</param>
    /// <param name="_interface">The interface name.</param>
    public Proxy(eldbus.Object obj, string _interface)
    {
        Contract.Requires(obj != null, nameof(obj));
        InitNew(eldbus_proxy_get(obj.Handle, _interface), true);
    }

    /// <summary>Finalizes with garbage collector.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    ~Proxy()
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
                eldbus_proxy_unref(h);
            }
            else
            {
                Efl.Eo.Globals.ThreadSafeFreeCbExec(eldbus_proxy_unref, h);
            }
        }
    }

    /// <summary>Disposes of this list.
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

    eldbus.Object GetObject()
    {
        CheckHandle();
        var ptr = eldbus_proxy_object_get(Handle);
        if (ptr == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Object' object from eldbus_proxy_object_get");
        }

        return new eldbus.Object(ptr, false);
    }

    string GetInterface()
    {
        CheckHandle();
        var ptr = eldbus_proxy_interface_get(Handle);
        return Eina.StringConversion.NativeUtf8ToManagedString(ptr);
    }

    eldbus.Message NewMethodCall(string member)
    {
        CheckHandle();

        if (member == null)
        {
            throw new ArgumentNullException(nameof(member));
        }

        var ptr = eldbus_proxy_method_call_new(Handle, member);
        if (ptr == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Message' object from eldbus_proxy_method_call_new");
        }

        return new eldbus.Message(ptr, false);
    }

    eldbus.Pending Send(eldbus.Message msg, eldbus.MessageDelegate dlgt = null, double timeout = -1)
    {
        CheckHandle();

        if (msg == null)
        {
            throw new ArgumentNullException(nameof(msg));
        }

        // Native send() takes ownership of the message. We ref here to keep the IDisposable
        // behavior simpler and keeping the original object alive in case the user wants.
        msg.Ref();

        IntPtr cb_wrapper = dlgt == null ? IntPtr.Zero : eldbus.Common.GetMessageCbWrapperPtr();
        IntPtr cb_data = dlgt == null ? IntPtr.Zero : Marshal.GetFunctionPointerForDelegate(dlgt);

        var pending_hdl = eldbus_proxy_send(Handle, msg.Handle, cb_wrapper, cb_data, timeout);

        if (pending_hdl == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Pending' object from eldbus_proxy_send");
        }

        return new eldbus.Pending(pending_hdl, false);
    }

    eldbus.Message SendAndBlock(eldbus.Message msg, double timeout = -1)
    {
        CheckHandle();
        var ptr = eldbus_proxy_send_and_block(Handle, msg.Handle, timeout);
        if (ptr == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Message' object from eldbus_proxy_send_and_block");
        }

        return new eldbus.Message(ptr, true);
    }

    eldbus.Pending Call(string member, eldbus.MessageDelegate dlgt, double timeout, params BasicMessageArgument[] args)
    {
        CheckHandle();

        using (var msg = NewMethodCall(member))
        {
            foreach (BasicMessageArgument arg in args)
            {
                arg.AppendTo(msg);
            }

            return Send(msg, dlgt, timeout);
        }
    }

    eldbus.Pending Call(string member, params BasicMessageArgument[] args)
    {
        return Call(member, null, -1.0, args);
    }
}

}
