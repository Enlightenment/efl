using System;
using System.Runtime.InteropServices;

namespace eina {

public interface ISliceBase
{
    UIntPtr Len {get;set;}
    IntPtr Mem {get;set;}
};

[StructLayout(LayoutKind.Sequential)]
public struct Slice : ISliceBase
{
    public UIntPtr Len {get;set;}
    public IntPtr Mem {get;set;}

    public Slice(IntPtr mem, uint len)
    {
        Mem = mem;
        Len = (UIntPtr)len;
    }

    public Slice PinnedDataSet(IntPtr mem, uint len)
    {
        Mem = mem;
        Len = (UIntPtr)len;
        return this;
    }

    Rw_Slice ToRW()
    {
        return new Rw_Slice(Mem, (uint)Len);
    }
}

[StructLayout(LayoutKind.Sequential)]
public struct Rw_Slice : ISliceBase
{
    public UIntPtr Len {get;set;}
    public IntPtr Mem {get;set;}

    public Rw_Slice(IntPtr mem, uint len)
    {
        Mem = mem;
        Len = (UIntPtr)len;
    }

    public Rw_Slice PinnedDataSet(IntPtr mem, uint len)
    {
        Mem = mem;
        Len = (UIntPtr)len;
        return this;
    }
}

}

public static class Eina_SliceUtils
{
    public static byte[] GetBytes(this eina.ISliceBase slc)
    {
        var size = (int)(slc.Len);
        byte[] mArray = new byte[size];
        Marshal.Copy(slc.Mem, mArray, 0, size);
        return mArray;
    }
}
