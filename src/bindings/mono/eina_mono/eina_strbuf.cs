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
using System.ComponentModel;

using static Eina.EinaNative.StrbufNativeMethods;

namespace Eina
{

namespace EinaNative
{

static internal class StrbufNativeMethods
{
    [DllImport(efl.Libs.Eina)]
    internal static extern IntPtr eina_strbuf_new();

    [DllImport(efl.Libs.Eina)]
    internal static extern void eina_strbuf_free(IntPtr buf);

    [DllImport(efl.Libs.Eina)]
    internal static extern void eina_strbuf_reset(IntPtr buf);

    [DllImport(efl.Libs.Eina)]
    [return: MarshalAsAttribute(UnmanagedType.U1)]
    internal static extern bool eina_strbuf_append(IntPtr buf, string str);

    [DllImport(efl.Libs.Eina)]
    [return: MarshalAsAttribute(UnmanagedType.U1)]
    internal static extern bool eina_strbuf_append_escaped(IntPtr buf, string str);

    [DllImport(efl.Libs.Eina)]
    [return: MarshalAsAttribute(UnmanagedType.U1)]
    internal static extern bool eina_strbuf_append_char(IntPtr buf, char c);

    [DllImport(efl.Libs.Eina, CharSet=CharSet.Ansi)]
    [return:
     MarshalAs(UnmanagedType.CustomMarshaler,
	       MarshalTypeRef=typeof(Efl.Eo.StringPassOwnershipMarshaler))]
    internal static extern string eina_strbuf_string_steal(IntPtr buf);

    [DllImport(efl.Libs.Eina, CharSet=CharSet.Ansi)]
    [return:
     MarshalAs(UnmanagedType.CustomMarshaler,
	       MarshalTypeRef=typeof(Efl.Eo.StringKeepOwnershipMarshaler))]
    internal static extern string eina_strbuf_string_get(IntPtr buf);

    [DllImport(efl.Libs.Eina)]
    internal static extern IntPtr eina_strbuf_length_get(IntPtr buf); // Uses IntPtr as wrapper for size_t
}

} // namespace EinaNative

/// <summary>Native string buffer, similar to the C# StringBuilder class.
///
/// <para>Since EFL 1.23.</para>
/// </summary>
public class Strbuf : IDisposable
{
    ///<summary>Pointer to the underlying native handle.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    [EditorBrowsable(EditorBrowsableState.Never)]
    public IntPtr Handle { get; protected set; }
    private Ownership Ownership;
    private bool Disposed;

    ///<summary>Creates a new Strbuf. By default its lifetime is managed.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public Strbuf(Ownership ownership = Ownership.Managed)
    {
        this.Handle = eina_strbuf_new();
        this.Ownership = ownership;
    }

    ///<summary>Creates a new Strbuf from an existing IntPtr.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    [EditorBrowsable(EditorBrowsableState.Never)]
    public Strbuf(IntPtr ptr, Ownership ownership)
    {
        this.Handle = ptr;
        this.Ownership = ownership;
    }

    /// <summary>Releases the ownership of the underlying value to C.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    [EditorBrowsable(EditorBrowsableState.Never)]
    public void ReleaseOwnership()
    {
        this.Ownership = Ownership.Unmanaged;
    }

    /// <summary>Takes the ownership of the underlying value to the Managed runtime.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    [EditorBrowsable(EditorBrowsableState.Never)]
    public void TakeOwnership()
    {
        this.Ownership = Ownership.Managed;
    }

    ///<summary>Public method to explicitly free the wrapped buffer.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    ///<summary>Actually free the wrapped buffer. Can be called from Dispose() or through the Garbage Collector.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    protected virtual void Dispose(bool disposing)
    {
        if (this.Ownership == Ownership.Unmanaged)
        {
            Disposed = true;
            return;
        }

        if (!Disposed && (Handle != IntPtr.Zero))
        {
            if (disposing)
            {
                eina_strbuf_free(Handle);
            }
            else
            {
                Efl.Eo.Globals.ThreadSafeFreeCbExec(eina_strbuf_free, Handle);
            }

            Handle = IntPtr.Zero;
        }

        Disposed = true;
    }

    ///<summary>Finalizer to be called from the Garbage Collector.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    ~Strbuf()
    {
        Dispose(false);
    }

    ///<summary>Retrieves the length of the buffer contents.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <value>The number of characters in this buffer.</value>
    public int Length
    {
        get
        {
            IntPtr size = eina_strbuf_length_get(Handle);
            return size.ToInt32();
        }
    }

    ///<summary>Resets a string buffer. Its len is set to 0.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public void Reset()
    {
        if (Disposed)
        {
            throw new ObjectDisposedException(base.GetType().Name);
        }

        eina_strbuf_reset(Handle);
    }

    ///<summary>Appends a string to a buffer, reallocating as necessary.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="text">The string to be appended.</param>
    /// <returns><c>true</c> if the append was successful.</returns>
    public bool Append(string text)
    {
        if (Disposed)
        {
            throw new ObjectDisposedException(base.GetType().Name);
        }

        return eina_strbuf_append(Handle, text);
    }

    ///<summary>Appends an escaped string to a buffer, reallocating as necessary.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="text">The string to be escaped and appended.</param>
    /// <returns><c>true</c> if the append was successful.</returns>
    public bool AppendEscaped(string text)
    {
        if (Disposed)
        {
            throw new ObjectDisposedException(base.GetType().Name);
        }

        return eina_strbuf_append_escaped(Handle, text);
    }

    ///<summary>Appends a char to a buffer, reallocating as necessary.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="c">The character to be appended.</param>
    /// <returns><c>true</c> if the append was successful.</returns>
    public bool Append(char c)
    {
        if (Disposed)
        {
            throw new ObjectDisposedException(base.GetType().Name);
        }

        return eina_strbuf_append_char(Handle, c);
    }

    ///<summary>Steals the content of a buffer. This causes the buffer to be re-initialized.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>A string with the contents of this buffer.</returns>
    public string Steal()
    {
        if (Disposed)
        {
            throw new ObjectDisposedException(base.GetType().Name);
        }

        return eina_strbuf_string_steal(this.Handle);
    }

    /// <summary>Copy the content of a buffer.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>A string with the contents of this buffer.</returns>
    public override string ToString()
    {
        return eina_strbuf_string_get(this.Handle);
    }
}
} // namespace eina
