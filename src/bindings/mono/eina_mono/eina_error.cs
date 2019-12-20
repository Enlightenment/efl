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
using System.Diagnostics.CodeAnalysis;

namespace Eina
{

/// <summary>Error codes from native Eina methods.
/// <para>Since EFL 1.23.</para>
/// </summary>
public struct Error : IComparable<Error>, IEquatable<Error>
{
    int code;

    /// <summary>
    /// The error's message.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public string Message
    {
        get { return MsgGet(this); }
    }

    /// <summary>
    /// Unhandled Exception error identifier.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public static readonly Error UNHANDLED_EXCEPTION = eina_error_msg_register("Unhandled C# exception occurred.");

    /// <summary>
    /// No error identifier.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public static readonly Error NO_ERROR = new Error(0);
    /// <summary>
    /// Permission error identifier.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public static readonly Error EPERM = new Error(1);
    /// <summary>
    /// No entity error identifier.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public static readonly Error ENOENT = new Error(2);
    /// <summary>
    /// Cancelled error identifier.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public static readonly Error ECANCELED = new Error(125);

    /// <summary>
    /// Constructor.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="value">The value of the error.</param>
    public Error(int value)
    {
        code = value;
    }

    /// <summary>
    ///   Error identifier conversion from int.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">Value to be converted to Error</param>
    public static implicit operator Error(int val) => FromInt32(val);

    /// <summary>
    ///   Converts a <see cref="int" /> to a <see cref="Error" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="val">The <see cref="int" /> to be converted.</param>
    public static Error FromInt32(int val) => new Error(val);

    /// <summary>
    ///   Int conversion from Error.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="error">Error identifier to be converted to int</param>
    public static implicit operator int(Error error) => ToInt32(error);

    /// <summary>
    ///   Converts a <see cref="Error" /> to a <see cref="int" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="error">The <see cref="Error" /> to be converted.</param>
    public static int ToInt32(Error error) => error.code;

    /// <summary>
    ///   Transform the object to a string representing the object.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>The string representing the value of this.</returns>
    public override string ToString()
    {
        return "Eina.Error(" + code + ")";
    }

    [DllImport(efl.Libs.Eina)] static extern Error eina_error_msg_register(string msg);
    [DllImport(efl.Libs.Eina)] static extern Error eina_error_get();
    [DllImport(efl.Libs.Eina)] static extern void eina_error_set(Error error);
    [DllImport(efl.Libs.Eina)] static extern IntPtr eina_error_msg_get(Error error);

    /// <summary>
    ///   Sets the last error.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="error">The error identifier.</param>
    public static void Set(Error error)
    {
        eina_error_set(error);
    }

    /// <summary>
    ///   Returns the last set error.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>The last error or NO_ERROR identifier.</returns>
    public static Error Get()
    {
        return eina_error_get();
    }

    /// <summary>
    ///   Returns the description of the given error identifier.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="error">Error identifier.</param>
    /// <returns>The description of the error.</returns>
    public static String MsgGet(Error error)
    {
        IntPtr cstr = eina_error_msg_get(error);
        return Eina.StringConversion.NativeUtf8ToManagedString(cstr);
    }

    /// <summary>Raises an exception if an unhandled exception occurred before switching
    /// back to the native code. For example, in an event handler.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    [SuppressMessage("Microsoft.Design", "CA1030:UseEventsWhereAppropriate",
                     Justification = "It's not an event.")]
    public static void RaiseIfUnhandledException()
    {
        Error e = Get();
        if (e == UNHANDLED_EXCEPTION)
        {
            Clear();
            Raise(e);
        }
    }

    /// <summary>
    ///   Raises an exception.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    [SuppressMessage("Microsoft.Design", "CA1030:UseEventsWhereAppropriate",
                     Justification = "It's not an event.")]
    public static void Raise(Error e)
    {
        if (e != 0)
        {
            throw (new Efl.EflException(MsgGet(e)));
        }
    }

    /// <summary>
    ///   Set identifier to a NO_ERROR.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public static void Clear()
    {
        Set(0);
    }

    /// <summary>
    ///   Registers a new error type.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="msg"> The description of the error.</param>
    /// <returns>The unique number identifier for this error.</returns>
    public static Error Register(string msg)
    {
        return eina_error_msg_register(msg);
    }

    /// <summary>
    ///   Gets a hash for <see cref="Eina.Error" />.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>A hash code.</returns>
    public override int GetHashCode()
        => code.GetHashCode() + Message.GetHashCode(StringComparison.Ordinal);

    /// <summary>
    ///   Compare to a given error.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="err">Error to be compared with.</param>
    /// <returns>-1, 0 or 1 if -1 if Error is less, equal or greater than err.</returns>
    public int CompareTo(Error err) => code.CompareTo(err.code);

    /// <summary>
    ///   Check if is equal to obj.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="obj">The object to be checked.</param>
    /// <returns>false if obj is null or not equals, true otherwise.</returns>
    public override bool Equals(object obj)
    {
        if (object.ReferenceEquals(obj, null))
            return false;

        return this.Equals((Error)obj);
    }

    /// <summary>
    ///   Check if is equal to err.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="err">The object to be checked.</param>
    /// <returns>false if obj is null or not equals, true otherwise.</returns>
    public bool Equals(Error err) => this.CompareTo(err) == 0;

    /// <summary>
    ///   Check if lhs is equals to rhs.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="lhs">The left hand side of the operator.</param>
    /// <param name="rhs">The right hand side of the operator.</param>
    /// <returns>true if lhs is equals to rhs, false otherwise.</returns>
    public static bool operator==(Error lhs, Error rhs) => lhs.Equals(rhs);

    /// <summary>
    ///   Check if lhs is not equals to rhs.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="lhs">The left hand side of the operator.</param>
    /// <param name="rhs">The right hand side of the operator.</param>
    /// <returns>true if lhs is not equals to rhs, false otherwise.</returns>
    public static bool operator!=(Error lhs, Error rhs) => !(lhs == rhs);

    /// <summary>
    ///   Check if lhs is less than rhs.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="lhs">The left hand side of the operator.</param>
    /// <param name="rhs">The right hand side of the operator.</param>
    /// <returns>true if lhs is less than rhs, false otherwise.</returns>
    public static bool operator<(Error lhs, Error rhs) => (lhs.CompareTo(rhs) < 0);

    /// <summary>
    ///   Check if lhs is greater to rhs.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="lhs">The left hand side of the operator.</param>
    /// <param name="rhs">The right hand side of the operator.</param>
    /// <returns>true if lhs is greater than rhs, false otherwise.</returns>
    public static bool operator>(Error lhs, Error rhs) => rhs < lhs;

    /// <summary>
    ///   Check if lhs is equals and less than rhs.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="lhs">The left hand side of the operator.</param>
    /// <param name="rhs">The right hand side of the operator.</param>
    /// <returns>true if lhs is equals and less than rhs, false otherwise.</returns>
    public static bool operator<=(Error lhs, Error rhs) => !(lhs > rhs);

    /// <summary>
    ///   Check if lhs is equals and greater than rhs.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="lhs">The left hand side of the operator.</param>
    /// <param name="rhs">The right hand side of the operator.</param>
    /// <returns>true if lhs is equals and greater than rhs, false otherwise.</returns>
    public static bool operator>=(Error lhs, Error rhs) => !(lhs < rhs);

}
}
