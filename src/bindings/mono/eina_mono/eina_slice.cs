#pragma warning disable 1591

using System;
using System.Runtime.InteropServices;
using System.ComponentModel;

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
/// Since EFL 1.23.
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct Slice : ISliceBase
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
}

/// <summary>Pointer to a slice of native memory.
///
/// Since EFL 1.23.
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public struct RwSlice : ISliceBase
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
}

}

public static class Eina_SliceUtils
{
    public static byte[] GetBytes(this Eina.ISliceBase slc)
    {
        var size = (int)(slc.Len);
        byte[] mArray = new byte[size];
        Marshal.Copy(slc.Mem, mArray, 0, size);
        return mArray;
    }
}
