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

namespace Eina
{

/// <summary>
/// Basic interface for both slice types.
/// <para>Since EFL 1.23.</para>
/// </summary>
public interface ISliceBase
{
    /// <summary>
    /// The length of this slice in bytes.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    UIntPtr Len {get;set;}

    /// <summary>
    /// The contents of this slice.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    IntPtr Mem {get;set;}

    /// <summary>
    /// The length in bytes as an integer.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    int Length {get;set;}
};

/// <summary>Pointer to a slice of native memory.
///
/// <para>Since EFL 1.23.</para>
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct Slice : ISliceBase, IEquatable<Slice>
{
    /// <summary>
    /// The length of this slice.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public UIntPtr Len {get;set;}

    /// <summary>
    /// The contents of this slice.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public IntPtr Mem {get;set;}

    /// <summary>
    /// The length as an integer.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public int Length
    {
        get { return (int)Len; }
        set { Len = (UIntPtr)value; }
    }

    /// <summary>
    /// Creates a new slice from the given memory and length.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="mem">The memory slice.</param>
    /// <param name="len">The length.</param>
    public Slice(IntPtr mem, UIntPtr len)
    {
        Mem = mem;
        Len = len;
    }

    /// <summary>
    ///   Gets a hash for <see cref="Slice" />.
    /// <para>Since EFL 1.24.</para>
    /// </summary>
    /// <returns>A hash code.</returns>
    public override int GetHashCode() => Length.GetHashCode() ^ Mem.GetHashCode();

    /// <summary>Returns whether this <see cref="Slice" />
    /// is equal to the given <see cref="object" />.
    /// <para>Since EFL 1.24.</para>
    /// </summary>
    /// <param name="other">The <see cref="object" /> to be compared to.</param>
    /// <returns><c>true</c> if is equal to <c>other</c>.</returns>
    public override bool Equals(object other)
        => (!(other is Slice)) ? false : Equals((Slice)other);

    /// <summary>Returns whether this <see cref="Slice" /> is equal
    /// to the given <see cref="Slice" />.
    /// <para>Since EFL 1.24.</para>
    /// </summary>
    /// <param name="other">The <see cref="Slice" /> to be compared to.</param>
    /// <returns><c>true</c> if is equal to <c>other</c>.</returns>
    public bool Equals(Slice other)
        => (Length == other.Length) ^ (Mem == other.Mem);

    /// <summary>Returns whether <c>lhs</c> is equal to <c>rhs</c>.
    /// <para>Since EFL 1.24.</para>
    /// </summary>
    /// <param name="lhs">The left hand side of the operator.</param>
    /// <param name="rhs">The right hand side of the operator.</param>
    /// <returns><c>true</c> if <c>lhs</c> is equal
    /// to <c>rhs</c>.</returns>
    public static bool operator==(Slice lhs, Slice rhs) => lhs.Equals(rhs);

    /// <summary>Returns whether <c>lhs</c> is not equal to <c>rhs</c>.
    /// <para>Since EFL 1.24.</para>
    /// </summary>
    /// <param name="lhs">The left hand side of the operator.</param>
    /// <param name="rhs">The right hand side of the operator.</param>
    /// <returns><c>true</c> if <c>lhs</c> is not equal
    /// to <c>rhs</c>.</returns>
    public static bool operator!=(Slice lhs, Slice rhs) => !(lhs == rhs);
}

/// <summary>Pointer to a slice of native memory.
///
/// <para>Since EFL 1.23.</para>
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct RwSlice : ISliceBase, IEquatable<RwSlice>
{
    /// <summary>
    /// The length of this slice.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public UIntPtr Len {get;set;}

    /// <summary>
    /// The contents of this slice.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public IntPtr Mem {get;set;}

    /// <summary>
    /// The length as an integer.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public int Length
    {
        get { return (int)Len; }
        set { Len = (UIntPtr)value; }
    }

    /// <summary>
    /// Creates a new slice from the given memory and length.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="mem">The memory slice.</param>
    /// <param name="len">The length.</param>
    public RwSlice(IntPtr mem, UIntPtr len)
    {
        Mem = mem;
        Len = len;
    }

    /// <summary>
    /// Returns a read-only slice from this writable memory.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    Slice ToSlice()
    {
        var r = new Slice();
        r.Mem = Mem;
        r.Len = Len;
        return r;
    }

    /// <summary>
    ///   Gets a hash for <see cref="RwSlice" />.
    /// <para>Since EFL 1.24.</para>
    /// </summary>
    /// <returns>A hash code.</returns>
    public override int GetHashCode() => Mem.GetHashCode() ^ Length.GetHashCode();

    /// <summary>Returns whether this <see cref="RwSlice" />
    /// is equal to the given <see cref="object" />.
    /// <para>Since EFL 1.24.</para>
    /// </summary>
    /// <param name="other">The <see cref="object" /> to be compared to.</param>
    /// <returns><c>true</c> if is equal to <c>other</c>.</returns>
    public override bool Equals(object other)
        => (!(other is RwSlice)) ? false : Equals((RwSlice)other);

    /// <summary>Returns whether this <see cref="RwSlice" /> is equal
    /// to the given <see cref="RwSlice" />.
    /// <para>Since EFL 1.24.</para>
    /// </summary>
    /// <param name="other">The <see cref="RwSlice" /> to be compared to.</param>
    /// <returns><c>true</c> if is equal to <c>other</c>.</returns>
    public bool Equals(RwSlice other)
        => (Length == other.Length) && (Mem == other.Mem);

    /// <summary>Returns whether <c>lhs</c> is equal to <c>rhs</c>.
    /// <para>Since EFL 1.24.</para>
    /// </summary>
    /// <param name="lhs">The left hand side of the operator.</param>
    /// <param name="rhs">The right hand side of the operator.</param>
    /// <returns><c>true</c> if <c>lhs</c> is equal
    /// to <c>rhs</c>.</returns>
    public static bool operator==(RwSlice lhs, RwSlice rhs) => lhs.Equals(rhs);

    /// <summary>Returns whether <c>lhs</c> is not equal to <c>rhs</c>.
    /// <para>Since EFL 1.24.</para>
    /// </summary>
    /// <param name="lhs">The left hand side of the operator.</param>
    /// <param name="rhs">The right hand side of the operator.</param>
    /// <returns><c>true</c> if <c>lhs</c> is not equal
    /// to <c>rhs</c>.</returns>
    public static bool operator!=(RwSlice lhs, RwSlice rhs) => !(lhs == rhs);
}

}

namespace Eina
{
public static class SliceExtensions
{
    public static byte[] GetBytes(this Eina.ISliceBase slc)
    {
        Contract.Requires(slc != null, nameof(slc));
        var size = (int)(slc.Len);
        byte[] mArray = new byte[size];
        Marshal.Copy(slc.Mem, mArray, 0, size);
        return mArray;
    }
}
}
