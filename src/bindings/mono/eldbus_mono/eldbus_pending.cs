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

using static eldbus.EldbusPendingNativeFunctions;

namespace eldbus
{

[EditorBrowsable(EditorBrowsableState.Never)]
internal static class EldbusPendingNativeFunctions
{
    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_pending_data_set(IntPtr pending, string key, IntPtr data);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_pending_data_get(IntPtr pending, string key);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_pending_data_del(IntPtr pending, string key);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_pending_cancel(IntPtr pending);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_pending_destination_get(IntPtr pending);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_pending_path_get(IntPtr pending);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_pending_interface_get(IntPtr pending);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_pending_method_get(IntPtr pending);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_pending_free_cb_add(IntPtr pending, IntPtr cb, IntPtr data);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_pending_free_cb_del(IntPtr pending, IntPtr cb, IntPtr data);
}

/// <summary>Represents a DBus pending.
/// <para>Since EFL 1.23.</para>
/// </summary>
public class Pending
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
    public Pending(IntPtr handle, bool own)
    {
        InitNew(handle, own);
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
    ///   Cancel the pending message.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public void Cancel()
    {
        CheckHandle();
        eldbus_pending_cancel(Handle);
    }

    /// <summary>
    ///   Get the destination of the pending message.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>A string corresponding to the destination of the
    /// message</returns>
    public string GetDestination()
    {
        CheckHandle();
        var ptr = eldbus_pending_destination_get(Handle);
        return Eina.StringConversion.NativeUtf8ToManagedString(ptr);
    }

    /// <summary>
    ///   Get the path of the pending message.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>A string corresponding to the path of the message.</returns>
    public string GetPath()
    {
        CheckHandle();
        var ptr = eldbus_pending_path_get(Handle);
        return Eina.StringConversion.NativeUtf8ToManagedString(ptr);
    }

    /// <summary>
    ///   Get the interface of the pending message.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>A string corresponding to the interface of the
    /// message.</returns>
    public string GetInterface()
    {
        CheckHandle();
        var ptr = eldbus_pending_interface_get(Handle);
        return Eina.StringConversion.NativeUtf8ToManagedString(ptr);
    }

    /// <summary>
    ///   Get the method of the pending message.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>A string corresponding to the method of the message.</returns>
    public string GetMethod()
    {
        CheckHandle();
        var ptr = eldbus_pending_method_get(Handle);
        return Eina.StringConversion.NativeUtf8ToManagedString(ptr);
    }
}

}
