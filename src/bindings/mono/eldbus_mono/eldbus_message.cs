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

using static eldbus.EldbusMessageNativeFunctions;

namespace eldbus
{

[EditorBrowsable(EditorBrowsableState.Never)]
internal static class EldbusMessageNativeFunctions
{
    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_message_ref(IntPtr msg);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_message_unref(IntPtr msg);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_message_path_get(IntPtr msg);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_message_interface_get(IntPtr msg);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_message_member_get(IntPtr msg);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_message_destination_get(IntPtr msg);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_message_sender_get(IntPtr msg);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_message_signature_get(IntPtr msg);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_message_method_call_new(string dest, string path, string iface, string method);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_message_signal_new(string path, string _interface, string name);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_message_error_new(IntPtr msg, string error_name, string error_msg);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_message_method_return_new(IntPtr msg);

    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_error_get(IntPtr msg, out IntPtr name, out IntPtr text);

    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_arguments_get(IntPtr msg, string signature, out byte value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_arguments_get(IntPtr msg, string signature, out Int16 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_arguments_get(IntPtr msg, string signature, out UInt16 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_arguments_get(IntPtr msg, string signature, out Int32 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_arguments_get(IntPtr msg, string signature, out UInt32 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_arguments_get(IntPtr msg, string signature, out Int64 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_arguments_get(IntPtr msg, string signature, out UInt64 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_arguments_get(IntPtr msg, string signature, out double value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_arguments_get(IntPtr msg, string signature, out IntPtr value);

//     [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
//         eldbus_message_arguments_vget(IntPtr msg, string signature, va_list ap);

    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_arguments_append(IntPtr msg, string signature, byte value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_arguments_append(IntPtr msg, string signature, Int16 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_arguments_append(IntPtr msg, string signature, UInt16 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_arguments_append(IntPtr msg, string signature, Int32 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_arguments_append(IntPtr msg, string signature, UInt32 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_arguments_append(IntPtr msg, string signature, Int64 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_arguments_append(IntPtr msg, string signature, UInt64 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_arguments_append(IntPtr msg, string signature, double value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_arguments_append(IntPtr msg, string signature, string value);

//     [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
//         eldbus_message_arguments_vappend(IntPtr msg, string signature, va_list ap);


    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_message_iter_container_new(IntPtr iter, int type, string contained_signature);

    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_basic_append(IntPtr iter, int type, byte value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_basic_append(IntPtr iter, int type, Int16 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_basic_append(IntPtr iter, int type, UInt16 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_basic_append(IntPtr iter, int type, Int32 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_basic_append(IntPtr iter, int type, UInt32 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_basic_append(IntPtr iter, int type, Int64 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_basic_append(IntPtr iter, int type, UInt64 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_basic_append(IntPtr iter, int type, double value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_basic_append(IntPtr iter, int type, string value);

    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_arguments_append(IntPtr iter, string signature, out IntPtr value);

//     [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
//         eldbus_message_iter_arguments_vappend(IntPtr iter, string signature, va_list ap);

    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_fixed_array_append(IntPtr iter, int type, IntPtr array, uint size);

    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_container_close(IntPtr iter, IntPtr sub);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_message_iter_get(IntPtr msg);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_message_iter_basic_get(IntPtr iter, out byte value);
    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_message_iter_basic_get(IntPtr iter, out Int16 value);
    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_message_iter_basic_get(IntPtr iter, out UInt16 value);
    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_message_iter_basic_get(IntPtr iter, out Int32 value);
    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_message_iter_basic_get(IntPtr iter, out UInt32 value);
    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_message_iter_basic_get(IntPtr iter, out Int64 value);
    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_message_iter_basic_get(IntPtr iter, out UInt64 value);
    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_message_iter_basic_get(IntPtr iter, out double value);
    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_message_iter_basic_get(IntPtr iter, out IntPtr value);

    [DllImport(efl.Libs.Eldbus)] public static extern string
        eldbus_message_iter_signature_get(IntPtr iter);

    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_next(IntPtr iter);

    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_get_and_next(IntPtr iter, char signature, out byte value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_get_and_next(IntPtr iter, char signature, out Int16 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_get_and_next(IntPtr iter, char signature, out UInt16 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_get_and_next(IntPtr iter, char signature, out Int32 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_get_and_next(IntPtr iter, char signature, out UInt32 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_get_and_next(IntPtr iter, char signature, out Int64 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_get_and_next(IntPtr iter, char signature, out UInt64 value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_get_and_next(IntPtr iter, char signature, out double value);
    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_get_and_next(IntPtr iter, char signature, out IntPtr value);

    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_fixed_array_get(IntPtr iter, int signature, out IntPtr value, out int n_elements);

    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_message_iter_arguments_get(IntPtr iter, string signature, out IntPtr value);

//     [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
//         eldbus_message_iter_arguments_vget(IntPtr iter, string signature, va_list ap);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_message_iter_del(IntPtr iter);
}


/// <summary>Represents a DBus message.
/// <para>Since EFL 1.23.</para>
/// </summary>
public class Message : IDisposable
{
    [EditorBrowsable(EditorBrowsableState.Never)]
    public IntPtr Handle {get;set;} = IntPtr.Zero;
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
    public Message(IntPtr handle, bool own)
    {
        InitNew(handle, own);
    }

    /// <summary>Finalizes with garbage collector.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    ~Message()
    {
        Dispose(false);
    }


    /// <summary>Disposes of this wrapper, releasing the native if owned.
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
                eldbus_message_unref(h);
            }
            else
            {
                Efl.Eo.Globals.ThreadSafeFreeCbExec(eldbus_message_unref, h);
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
    ///   Create a new message to invoke a method on a remote object.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="dest">The bus name or unique id of the remote application.</param>
    /// <param name="path">The object path.</param>
    /// <param name="iface">The interface name.</param>
    /// <param name="method">The name of the method to be called.</param>
    /// <returns>A new <see cref="eldbus.Message" />.</returns>
    public static eldbus.Message NewMethodCall(string dest, string path, string iface, string method)
    {
        var ptr = eldbus_message_method_call_new(dest, path, iface, method);
        if (ptr == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Message' object from eldbus_message_method_call_new");
        }

        return new eldbus.Message(ptr, true);
    }

    /// <summary>
    ///   Create a new signal message.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="path">The object path.</param>
    /// <param name="_interface">The interface name.</param>
    /// <param name="name">The name of the signal to be broadcasted.</param>
    /// <returns>A new <see cref="eldbus.Message" />.</returns>
    public static eldbus.Message NewSignal(string path, string _interface, string name)
    {
        var ptr = eldbus_message_signal_new(path, _interface, name);
        if (ptr == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Message' object from eldbus_message_signal_new");
        }

        return new eldbus.Message(ptr, true);
    }

    /// <summary>
    ///   Increase message reference.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public void Ref()
    {
        CheckHandle();
        eldbus_message_ref(Handle);
    }

    /// <summary>
    ///   Decrease message reference.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public void Unref()
    {
        CheckHandle();
        eldbus_message_unref(Handle);
    }

    /// <summary>
    ///   Get the eldbus message path.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>A string containing the dbus message path.</returns>
    public string GetPath()
    {
        CheckHandle();
        var ptr = eldbus_message_path_get(Handle);
        return Eina.StringConversion.NativeUtf8ToManagedString(ptr);
    }

    /// <summary>
    ///   The eldbus message interface.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>A string containing the dbus message interface.</returns>
    public string GetInterface()
    {
        CheckHandle();
        var ptr = eldbus_message_interface_get(Handle);
        return Eina.StringConversion.NativeUtf8ToManagedString(ptr);
    }

    /// <summary>
    ///   Get the eldbus message member.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>A string containing the dbus message destination.</returns>
    public string GetMember()
    {
        CheckHandle();
        var ptr = eldbus_message_member_get(Handle);
        return Eina.StringConversion.NativeUtf8ToManagedString(ptr);
    }

    /// <summary>
    ///   Get the eldbus message destination.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>A string containing the dbus message destination.</returns>
    public string GetDestination()
    {
        CheckHandle();
        var ptr = eldbus_message_destination_get(Handle);
        return Eina.StringConversion.NativeUtf8ToManagedString(ptr);
    }

    /// <summary>
    ///   Get the eldbus message sender.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>A string containing the dbus message sender.</returns>
    public string GetSender()
    {
        CheckHandle();
        var ptr = eldbus_message_sender_get(Handle);
        return Eina.StringConversion.NativeUtf8ToManagedString(ptr);
    }

    /// <summary>
    ///   Get the eldbus message signature.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>A string containing the dbus message signature.</returns>
    public string GetSignature()
    {
        CheckHandle();
        var ptr = eldbus_message_signature_get(Handle);
        return Eina.StringConversion.NativeUtf8ToManagedString(ptr);
    }

    /// <summary>
    ///   Create a new message that is an error reply to another message.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="error_name">The error name.</param>
    /// <param name="error_msg">The error message string.</param>
    /// <returns>A new <see cref="eldbus.Message" />.</returns>
    public eldbus.Message NewError(string error_name, string error_msg)
    {
        CheckHandle();
        var ptr = eldbus_message_error_new(Handle, error_name, error_msg);
        if (ptr == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Message' object from eldbus_message_error_new");
        }

        return new eldbus.Message(ptr, false);
    }

    /// <summary>
    ///   Create a message that is a reply to a method call.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>A new <see cref="eldbus.Message" />.</returns>
    public eldbus.Message NewMethodReturn()
    {
        CheckHandle();
        var ptr = eldbus_message_method_return_new(Handle);
        if (ptr == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `Message' object from eldbus_message_method_return_new");
        }

        return new eldbus.Message(ptr, false);
    }

    /// <summary>
    ///   Get the error text and name from a <see cref="eldbus.Message" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="name">Store the error name.</param>
    /// <param name="text">Store the error text..</param>
    /// <returns>true on success, false otherwise.</returns>
    public bool GetError(out string name, out string text)
    {
        CheckHandle();
        IntPtr name_ptr;
        IntPtr text_ptr;
        bool r = eldbus_message_error_get(Handle, out name_ptr, out text_ptr);
        name = Eina.StringConversion.NativeUtf8ToManagedString(name_ptr);
        text = Eina.StringConversion.NativeUtf8ToManagedString(text_ptr);
        return r;
    }

    /// <summary>
    ///   Get the arguments from an <see cref="eldbus.Message" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A byte that store the message arguments.</param>
    /// <returns>true if the arguments were read successfully.</returns>
    public bool Get(out byte val)
    {
        CheckHandle();
        return eldbus_message_arguments_get(Handle, Argument.ByteType.Signature, out val);
    }

    /// <summary>
    ///   Get the arguments from an <see cref="eldbus.Message" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A bool that store the message arguments.</param>
    /// <returns>true if the arguments were read successfully.</returns>
    public bool Get(out bool val)
    {
        CheckHandle();
        Int32 aux;
        var r = eldbus_message_arguments_get(Handle, Argument.BooleanType.Signature, out aux);
        val = (aux != 0);
        return r;
    }

    /// <summary>
    ///   Get the arguments from an <see cref="eldbus.Message" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A int16 that store the message arguments.</param>
    /// <returns>true if the arguments were read successfully.</returns>
    public bool Get(out Int16 val)
    {
        CheckHandle();
        return eldbus_message_arguments_get(Handle, Argument.Int16Type.Signature, out val);
    }

    /// <summary>
    ///   Get the arguments from an <see cref="eldbus.Message" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A unsigned int16 that store the message arguments.</param>
    /// <returns>true if the arguments were read successfully.</returns>
    public bool Get(out UInt16 val)
    {
        CheckHandle();
        return eldbus_message_arguments_get(Handle, Argument.UInt16Type.Signature, out val);
    }

    /// <summary>
    ///   Get the arguments from an <see cref="eldbus.Message" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A int32 that store the message arguments.</param>
    /// <returns>true if the arguments were read successfully.</returns>
    public bool Get(out Int32 val)
    {
        CheckHandle();
        return eldbus_message_arguments_get(Handle, Argument.Int32Type.Signature, out val);
    }

    /// <summary>
    ///   Get the arguments from an <see cref="eldbus.Message" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A unsigned int32 that store the message arguments.</param>
    /// <returns>true if the arguments were read successfully.</returns>
    public bool Get(out UInt32 val)
    {
        CheckHandle();
        return eldbus_message_arguments_get(Handle, Argument.UInt32Type.Signature, out val);
    }

    /// <summary>
    ///   Get the arguments from an <see cref="eldbus.Message" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A int64 that store the message arguments.</param>
    /// <returns>true if the arguments were read successfully.</returns>
    public bool Get(out Int64 val)
    {
        CheckHandle();
        return eldbus_message_arguments_get(Handle, Argument.Int64Type.Signature, out val);
    }

    /// <summary>
    ///   Get the arguments from an <see cref="eldbus.Message" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A unsigned int64 that store the message arguments.</param>
    /// <returns>true if the arguments were read successfully.</returns>
    public bool Get(out UInt64 val)
    {
        CheckHandle();
        return eldbus_message_arguments_get(Handle, Argument.UInt64Type.Signature, out val);
    }

    /// <summary>
    ///   Get the arguments from an <see cref="eldbus.Message" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A double that store the message arguments.</param>
    /// <returns>true if the arguments were read successfully.</returns>
    public bool Get(out double val)
    {
        CheckHandle();
        return eldbus_message_arguments_get(Handle, Argument.DoubleType.Signature, out val);
    }

    /// <summary>
    ///   Get the arguments from an <see cref="eldbus.Message" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A string that store the message arguments.</param>
    /// <returns>true if the arguments were read successfully.</returns>
    public bool Get(out string val)
    {
        CheckHandle();
        IntPtr aux;
        var r = eldbus_message_arguments_get(Handle, Argument.StringType.Signature, out aux);
        val = Eina.StringConversion.NativeUtf8ToManagedString(aux);
        return r;
    }

    /// <summary>
    ///   Get the arguments from an <see cref="eldbus.Message" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A <see cref="eldbus.ObjectPath" /> that store the message arguments.</param>
    /// <returns>true if the arguments were read successfully.</returns>
    public bool Get(out eldbus.ObjectPath val)
    {
        CheckHandle();
        IntPtr aux;
        var r = eldbus_message_arguments_get(Handle, Argument.ObjectPathType.Signature, out aux);
        val = Eina.StringConversion.NativeUtf8ToManagedString(aux);
        return r;
    }

    /// <summary>
    ///   Get the arguments from an <see cref="eldbus.Message" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A <see cref="eldbus.SignatureString" /> that store the message arguments.</param>
    /// <returns>true if the arguments were read successfully.</returns>
    public bool Get(out eldbus.SignatureString val)
    {
        CheckHandle();
        IntPtr aux;
        var r = eldbus_message_arguments_get(Handle, Argument.SignatureType.Signature, out aux);
        val = Eina.StringConversion.NativeUtf8ToManagedString(aux);
        return r;
    }

    /// <summary>
    ///   Get the arguments from an <see cref="eldbus.Message" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A <see cref="eldbus.UnixFd" /> that store the message arguments.</param>
    /// <returns>true if the arguments were read successfully.</returns>
    public bool Get(out eldbus.UnixFd val)
    {
        CheckHandle();
        Int32 aux;
        var r = eldbus_message_arguments_get(Handle, Argument.UnixFdType.Signature, out aux);
        val = aux;
        return r;
    }

    /// <summary>
    ///   Appends the arguments.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="args">The arguments to be appended.</param>
    public void Append(params BasicMessageArgument[] args)
    {
        CheckHandle();
        foreach (BasicMessageArgument arg in args)
        {
            arg.AppendTo(this);
        }
    }

    /// <summary>
    ///  Create and append a typed iterator to another iterator.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="signature">The signature to be appended.</param>
    /// <returns>A <see cref="eldbus.MessageIterator" />.</returns>
    public eldbus.MessageIterator AppendOpenContainer(string signature)
    {
        var iter = GetMessageIterator();
        return iter.AppendOpenContainer(signature);
    }

    /// <summary>
    ///   Get the main <see cref="eldbus.MessageIterator" /> from the <see cref="eldbus.Message" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>A <see cref="eldbus.MessageIterator" /></returns>
    public eldbus.MessageIterator GetMessageIterator()
    {
        CheckHandle();
        var ptr = eldbus_message_iter_get(Handle);
        if (ptr == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `MessageIterator' object from eldbus_message_iter_get");
        }

        return new eldbus.MessageIterator(ptr, IntPtr.Zero);
    }
}

/// <summary>
///   Iterator to a <see cref="eldbus.Message" />.
/// <para>Since EFL 1.23.</para>
/// </summary>
public class MessageIterator
{
    [EditorBrowsable(EditorBrowsableState.Never)]
    public IntPtr Handle {get;set;} = IntPtr.Zero;

    /// <summary>
    ///   The parent of the iterator.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public IntPtr Parent {get;set;} = IntPtr.Zero;

    private void InitNew(IntPtr handle, IntPtr parent)
    {
        Handle = handle;
        Parent = parent;
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
    public MessageIterator(IntPtr handle, IntPtr parent)
    {
        InitNew(handle, parent);
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
        Parent = IntPtr.Zero;
        return h;
    }

    /// <summary>
    ///   Appends the arguments.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="args">The arguments to be appended.</param>
    public void Append(params BasicMessageArgument[] args)
    {
        CheckHandle();

        foreach (BasicMessageArgument arg in args)
        {
            arg.AppendTo(this);
        }
    }

    /// <summary>
    ///   Create and append a typed iterator to another iterator.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="signature">The signature to be appended.</param>
    /// <returns>A <see cref="eldbus.MessageIterator" />.</returns>
    public eldbus.MessageIterator AppendOpenContainer(string signature)
    {
        Contract.Requires(signature != null, nameof(signature));
        CheckHandle();

        IntPtr new_iter = IntPtr.Zero;

        if (signature[0] == 'v')
        {
            new_iter = eldbus_message_iter_container_new(Handle, 'v', signature.Substring(1));
        }
        else if (!eldbus_message_iter_arguments_append(Handle, signature, out new_iter))
        {
            throw new SEHException("Eldbus: could not append container type");
        }

        if (new_iter == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `MessageIterator' object from eldbus_message_iter_arguments_append");
        }

        return new eldbus.MessageIterator(new_iter, Handle);
    }

    /// <summary>
    ///   Appends a signature to a container.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="type">The type of the iterator.</param>
    /// <param name="contained_signature">The signature to be appended.</param>
    /// <returns>A <see cref="eldbus.MessageIterator" />.</returns>
    public eldbus.MessageIterator AppendOpenContainer(char type, string contained_signature)
    {
        CheckHandle();

        IntPtr new_iter = eldbus_message_iter_container_new(Handle, type, contained_signature);

        if (new_iter == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get `MessageIterator' object from eldbus_message_iter_container_new");
        }

        return new eldbus.MessageIterator(new_iter, Handle);
    }

    /// <summary>
    ///   Closes a container-typed value appended to the message.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public void CloseContainer()
    {
        CheckHandle();

        if (Parent == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: can not close MessageIterator open container without a parent");
        }

        if (!eldbus_message_iter_container_close(Parent, Handle))
        {
            throw new SEHException("Eldbus: could not close MessageIterator");
        }

        Handle = IntPtr.Zero;
        Parent = IntPtr.Zero;
    }

    /// <summary>
    ///   Returns the current signature of a message iterator.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>A string containing the message iterator signature.</returns>
    public string GetSignature()
    {
        return eldbus_message_iter_signature_get(Handle);
    }

    /// <summary>
    ///   Get a complete type from <see cref="eldbus.MessageIterator" /> if is
    /// not at the end of iterator and move to next field.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A byte that store the data.</param>
    /// <returns>if iterator was reach to end or if the type different of the
    /// type that iterator pointes return false.</returns>
    public bool GetAndNext(out byte val)
    {
        CheckHandle();
        return eldbus_message_iter_get_and_next(Handle, Argument.ByteType.Code, out val);
    }

    /// <summary>
    ///   Get a complete type from <see cref="eldbus.MessageIterator" /> if is
    /// not at the end of iterator and move to next field.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A bool that store the data.</param>
    /// <returns>if iterator was reach to end or if the type different of the
    /// type that iterator pointes return false.</returns>
    public bool GetAndNext(out bool val)
    {
        CheckHandle();
        Int32 aux;
        bool r = eldbus_message_iter_get_and_next(Handle, Argument.BooleanType.Code, out aux);
        val = (aux != 0);
        return r;
    }

    /// <summary>
    ///   Get a complete type from <see cref="eldbus.MessageIterator" /> if is
    /// not at the end of iterator and move to next field.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A int16 that store the data.</param>
    /// <returns>if iterator was reach to end or if the type different of the
    /// type that iterator pointes return false.</returns>
    public bool GetAndNext(out Int16 val)
    {
        CheckHandle();
        return eldbus_message_iter_get_and_next(Handle, Argument.Int16Type.Code, out val);
    }

    /// <summary>
    ///   Get a complete type from <see cref="eldbus.MessageIterator" /> if is
    /// not at the end of iterator and move to next field.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A unsigned int16 that store the data.</param>
    /// <returns>if iterator was reach to end or if the type different of the
    /// type that iterator pointes return false.</returns>
    public bool GetAndNext(out UInt16 val)
    {
        CheckHandle();
        return eldbus_message_iter_get_and_next(Handle, Argument.UInt16Type.Code, out val);
    }

    /// <summary>
    ///   Get a complete type from <see cref="eldbus.MessageIterator" /> if is
    /// not at the end of iterator and move to next field.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A int32 that store the data.</param>
    /// <returns>if iterator was reach to end or if the type different of the
    /// type that iterator pointes return false.</returns>
    public bool GetAndNext(out Int32 val)
    {
        CheckHandle();
        return eldbus_message_iter_get_and_next(Handle, Argument.Int32Type.Code, out val);
    }

    /// <summary>
    ///   Get a complete type from <see cref="eldbus.MessageIterator" /> if is
    /// not at the end of iterator and move to next field.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A unsigned int32 that store the data.</param>
    /// <returns>if iterator was reach to end or if the type different of the
    /// type that iterator pointes return false.</returns>
    public bool GetAndNext(out UInt32 val)
    {
        CheckHandle();
        return eldbus_message_iter_get_and_next(Handle, Argument.UInt32Type.Code, out val);
    }

    /// <summary>
    ///   Get a complete type from <see cref="eldbus.MessageIterator" /> if is
    /// not at the end of iterator and move to next field.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A int64 that store the data.</param>
    /// <returns>if iterator was reach to end or if the type different of the
    /// type that iterator pointes return false.</returns>
    public bool GetAndNext(out Int64 val)
    {
        CheckHandle();
        return eldbus_message_iter_get_and_next(Handle, Argument.Int64Type.Code, out val);
    }

    /// <summary>
    ///   Get a complete type from <see cref="eldbus.MessageIterator" /> if is
    /// not at the end of iterator and move to next field.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A unsigned int64 that store the data.</param>
    /// <returns>if iterator was reach to end or if the type different of the
    /// type that iterator pointes return false.</returns>
    public bool GetAndNext(out UInt64 val)
    {
        CheckHandle();
        return eldbus_message_iter_get_and_next(Handle, Argument.UInt64Type.Code, out val);
    }

    /// <summary>
    ///   Get a complete type from <see cref="eldbus.MessageIterator" /> if is
    /// not at the end of iterator and move to next field.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A double that store the data.</param>
    /// <returns>if iterator was reach to end or if the type different of the
    /// type that iterator pointes return false.</returns>
    public bool GetAndNext(out double val)
    {
        CheckHandle();
        return eldbus_message_iter_get_and_next(Handle, Argument.DoubleType.Code, out val);
    }

    /// <summary>
    ///   Get a complete type from <see cref="eldbus.MessageIterator" /> if is
    /// not at the end of iterator and move to next field.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A string that store the data.</param>
    /// <returns>if iterator was reach to end or if the type different of the
    /// type that iterator pointes return false.</returns>
    public bool GetAndNext(out string val)
    {
        CheckHandle();
        IntPtr aux;
        bool r = eldbus_message_iter_get_and_next(Handle, Argument.StringType.Code, out aux);
        val = Eina.StringConversion.NativeUtf8ToManagedString(aux);
        return r;
    }

    /// <summary>
    ///   Get a complete type from <see cref="eldbus.MessageIterator" /> if is
    /// not at the end of iterator and move to next field.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A <see cref="eldbus.ObjectPath" /> that store the data.</param>
    /// <returns>if iterator was reach to end or if the type different of the
    /// type that iterator pointes return false.</returns>
    public bool GetAndNext(out eldbus.ObjectPath val)
    {
        CheckHandle();
        IntPtr aux;
        bool r = eldbus_message_iter_get_and_next(Handle, Argument.ObjectPathType.Code, out aux);
        val = Eina.StringConversion.NativeUtf8ToManagedString(aux);
        return r;
    }

    /// <summary>
    ///   Get a complete type from <see cref="eldbus.MessageIterator" /> if is
    /// not at the end of iterator and move to next field.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A <see cref="eldbus.SignatureString" /> that store the data.</param>
    /// <returns>if iterator was reach to end or if the type different of the
    /// type that iterator pointes return false.</returns>
    public bool GetAndNext(out eldbus.SignatureString val)
    {
        CheckHandle();
        IntPtr aux;
        bool r = eldbus_message_iter_get_and_next(Handle, Argument.SignatureType.Code, out aux);
        val = Eina.StringConversion.NativeUtf8ToManagedString(aux);
        return r;
    }

    /// <summary>
    ///   Get a complete type from <see cref="eldbus.MessageIterator" /> if is
    /// not at the end of iterator and move to next field.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">A <see cref="eldbus.UnixFd" /> that store the data.</param>
    /// <returns>if iterator was reach to end or if the type different of the
    /// type that iterator pointes return false.</returns>
    public bool GetAndNext(out eldbus.UnixFd val)
    {
        CheckHandle();
        Int32 aux;
        bool r = eldbus_message_iter_get_and_next(Handle, Argument.UnixFdType.Code, out aux);
        val = aux;
        return r;
    }

    /// <summary>
    ///   Get a complete type from <see cref="eldbus.MessageIterator" /> if is
    /// not at the end of iterator and move to next field.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="iter">A <see cref="eldbus.MessageIterator" /> that store
    /// the data.</param>
    /// <param name="typecode">The type of the
    /// <see cref="eldbus.MessageIterator" />.</param>
    /// <returns>if iterator was reach to end or if the type different of the
    /// type that iterator pointes return false.</returns>
    public bool GetAndNext(out eldbus.MessageIterator iter, char typecode)
    {
        CheckHandle();
        IntPtr hdl = IntPtr.Zero;
        bool r = eldbus_message_iter_get_and_next(Handle, typecode, out hdl);
        if (hdl == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get argument");
        }

        iter = new eldbus.MessageIterator(hdl, Handle);

        return r;
    }

    /// <summary>
    ///   Get a complete type from <see cref="eldbus.MessageIterator" /> if is
    /// not at the end of iterator and move to next field.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="iter">A <see cref="eldbus.MessageIterator" /> that store
    /// the data.</param>
    /// <param name="signatue">The signatue of the
    /// <see cref="eldbus.MessageIterator" />.</param>
    /// <returns>if iterator was reach to end or if the type different of the
    /// type that iterator pointes return false.</returns>
    public bool GetAndNext(out eldbus.MessageIterator iter, string signatue)
    {
        CheckHandle();
        IntPtr hdl = IntPtr.Zero;
        if (!eldbus_message_iter_arguments_get(Handle, signatue, out hdl) || hdl == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get argument");
        }

        iter = new eldbus.MessageIterator(hdl, Handle);

        return Next();
    }

    /// <summary>
    ///   Get a basic type from <see cref="eldbus.MessageIterator" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">The basic type of the iterator.</param>
    public void Get(out byte val)
    {
        CheckHandle();
        eldbus_message_iter_basic_get(Handle, out val);
    }

    /// <summary>
    ///   Get a basic type from <see cref="eldbus.MessageIterator" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">The basic type of the iterator.</param>
    public void Get(out bool val)
    {
        CheckHandle();
        Int32 aux;
        eldbus_message_iter_basic_get(Handle, out aux);
        val = (aux != 0);
    }

    /// <summary>
    ///   Get a basic type from <see cref="eldbus.MessageIterator" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">The basic type of the iterator.</param>
    public void Get(out Int16 val)
    {
        CheckHandle();
        eldbus_message_iter_basic_get(Handle, out val);
    }

    /// <summary>
    ///   Get a basic type from <see cref="eldbus.MessageIterator" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">The basic type of the iterator.</param>
    public void Get(out UInt16 val)
    {
        CheckHandle();
        eldbus_message_iter_basic_get(Handle, out val);
    }

    /// <summary>
    ///   Get a basic type from <see cref="eldbus.MessageIterator" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">The basic type of the iterator.</param>
    public void Get(out Int32 val)
    {
        CheckHandle();
        eldbus_message_iter_basic_get(Handle, out val);
    }

    /// <summary>
    ///   Get a basic type from <see cref="eldbus.MessageIterator" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">The basic type of the iterator.</param>
    public void Get(out UInt32 val)
    {
        CheckHandle();
        eldbus_message_iter_basic_get(Handle, out val);
    }

    /// <summary>
    ///   Get a basic type from <see cref="eldbus.MessageIterator" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">The basic type of the iterator.</param>
    public void Get(out Int64 val)
    {
        CheckHandle();
        eldbus_message_iter_basic_get(Handle, out val);
    }

    /// <summary>
    ///   Get a basic type from <see cref="eldbus.MessageIterator" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">The basic type of the iterator.</param>
    public void Get(out UInt64 val)
    {
        CheckHandle();
        eldbus_message_iter_basic_get(Handle, out val);
    }

    /// <summary>
    ///   Get a basic type from <see cref="eldbus.MessageIterator" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">The basic type of the iterator.</param>
    public void Get(out double val)
    {
        CheckHandle();
        eldbus_message_iter_basic_get(Handle, out val);
    }

    /// <summary>
    ///   Get a basic type from <see cref="eldbus.MessageIterator" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">The basic type of the iterator.</param>
    public void Get(out string val)
    {
        CheckHandle();
        IntPtr aux;
        eldbus_message_iter_basic_get(Handle, out aux);
        val = Eina.StringConversion.NativeUtf8ToManagedString(aux);
    }

    /// <summary>
    ///   Get a basic type from <see cref="eldbus.MessageIterator" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">The basic type of the iterator.</param>
    public void Get(out eldbus.ObjectPath val)
    {
        CheckHandle();
        IntPtr aux;
        eldbus_message_iter_basic_get(Handle, out aux);
        val = Eina.StringConversion.NativeUtf8ToManagedString(aux);
    }

    /// <summary>
    ///   Get a basic type from <see cref="eldbus.MessageIterator" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">The basic type of the iterator.</param>
    public void Get(out eldbus.SignatureString val)
    {
        CheckHandle();
        IntPtr aux;
        eldbus_message_iter_basic_get(Handle, out aux);
        val = Eina.StringConversion.NativeUtf8ToManagedString(aux);
    }

    /// <summary>
    ///   Get a basic type from <see cref="eldbus.MessageIterator" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">The basic type of the iterator.</param>
    public void Get(out eldbus.UnixFd val)
    {
        CheckHandle();
        Int32 aux;
        eldbus_message_iter_basic_get(Handle, out aux);
        val = aux;
    }

    /// <summary>
    ///   Get a basic type from <see cref="eldbus.MessageIterator" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="iter">The basic type of the iterator.</param>
    /// <param name="signatue">The signatue of the <see cref="eldbus.MessageIterator" />.</param>
    public void Get(out eldbus.MessageIterator iter, string signatue)
    {
        CheckHandle();
        IntPtr hdl = IntPtr.Zero;
        if (!eldbus_message_iter_arguments_get(Handle, signatue, out hdl) || hdl == IntPtr.Zero)
        {
            throw new SEHException("Eldbus: could not get argument");
        }

        iter = new eldbus.MessageIterator(hdl, Handle);
    }

    /// <summary>
    ///   Moves the iterator to the next field, if any.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>If iterator was reach to end return false.</returns>
    public bool Next()
    {
        CheckHandle();
        return eldbus_message_iter_next(Handle);
    }

    /// <summary>
    ///   Manually delete the iterator.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public void Del()
    {
        CheckHandle();

        eldbus_message_iter_del(Handle);

        Handle = IntPtr.Zero;
        Parent = IntPtr.Zero;
    }

    private void GetFixedArrayInternal(int type_code, out IntPtr value, out int n_elements)
    {
        CheckHandle();

        if (!eldbus_message_iter_fixed_array_get(Handle, type_code, out value, out n_elements))
        {
            throw new SEHException("Eldbus: could not get fixed array");
        }
    }

    /// <summary>
    ///   Copy the iterator to a given array.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="array">The array to receive the copy.</param>
    public void GetFixedArray(out byte[] array)
    {
        IntPtr value;
        int n_elements;
        GetFixedArrayInternal(Argument.ByteType.Code, out value, out n_elements);
        array = new byte[n_elements];
        Marshal.Copy(value, array, 0, n_elements);
    }

    /// <summary>
    ///   Copy the iterator to a given array.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="array">The array to receive the copy.</param>
    public void GetFixedArray(out bool[] array)
    {
        IntPtr value;
        int n_elements;
        GetFixedArrayInternal(Argument.BooleanType.Code, out value, out n_elements);
        var aux = new Int32[n_elements];
        Marshal.Copy(value, aux, 0, n_elements);

        // array = aux.Select(Convert.ToBoolean).ToArray();
        array = Array.ConvertAll(aux, Convert.ToBoolean);
    }

    /// <summary>
    ///   Copy the iterator to a given array.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="array">The array to receive the copy.</param>
    public void GetFixedArray(out Int16[] array)
    {
        IntPtr value;
        int n_elements;
        GetFixedArrayInternal(Argument.Int16Type.Code, out value, out n_elements);
        array = new Int16[n_elements];
        Marshal.Copy(value, array, 0, n_elements);
    }

//     public void GetFixedArray(out UInt16[] array)
//     {
//         IntPtr value;
//         int n_elements;
//         GetFixedArrayInternal(Argument.UInt16Type.Code, out value, out n_elements);
//         array = new UInt16[n_elements];
//         Marshal.Copy(value, array, 0, n_elements);
//     }

    /// <summary>
    ///   Copy the iterator to a given array.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="array">The array to receive the copy.</param>
    public void GetFixedArray(out Int32[] array)
    {
        IntPtr value;
        int n_elements;
        GetFixedArrayInternal(Argument.Int32Type.Code, out value, out n_elements);
        array = new Int32[n_elements];
        Marshal.Copy(value, array, 0, n_elements);
    }

//     public void GetFixedArray(out UInt32[] array)
//     {
//         IntPtr value;
//         int n_elements;
//         GetFixedArrayInternal(Argument.UInt32Type.Code, out value, out n_elements);
//         array = new UInt32[n_elements];
//         Marshal.Copy(value, array, 0, n_elements);
//     }

    /// <summary>
    ///   Copy the iterator to a given array.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="array">The array to receive the copy.</param>
    public void GetFixedArray(out Int64[] array)
    {
        IntPtr value;
        int n_elements;
        GetFixedArrayInternal(Argument.Int64Type.Code, out value, out n_elements);
        array = new Int64[n_elements];
        Marshal.Copy(value, array, 0, n_elements);
    }

//     public void GetFixedArray(out UInt64[] array)
//     {
//         IntPtr value;
//         int n_elements;
//         GetFixedArrayInternal(Argument.UInt64Type.Code, out value, out n_elements);
//         array = new UInt64[n_elements];
//         Marshal.Copy(value, array, 0, n_elements);
//     }

    /// <summary>
    ///   Copy the iterator to a given array.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="array">The array to receive the copy.</param>
    public void GetFixedArray(out eldbus.UnixFd[] array)
    {
        IntPtr value;
        int n_elements;
        GetFixedArrayInternal(Argument.DoubleType.Code, out value, out n_elements);
        var aux = new Int32[n_elements];
        Marshal.Copy(value, aux, 0, n_elements);

        array = Array.ConvertAll(aux, e => new UnixFd(e));
    }
}

}
