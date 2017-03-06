using System;
using System.Runtime.InteropServices;

namespace eina {

public class Binbuf : IDisposable
{
    [DllImport("eina")] public static extern IntPtr
        eina_binbuf_new();
    [DllImport("eina")] public static extern void
        eina_binbuf_free(IntPtr buf);
    [DllImport("eina")] public static extern void
        eina_binbuf_reset(IntPtr buf);
    [DllImport("eina")] public static extern byte
        eina_binbuf_append_length(IntPtr buf, byte[] str, UIntPtr length);
    [DllImport("eina")] public static extern byte
        eina_binbuf_append_buffer(IntPtr buf, IntPtr data);
    [DllImport("eina")] public static extern byte
        eina_binbuf_append_char(IntPtr buf, byte c);
    [DllImport("eina")] public static extern byte
        eina_binbuf_insert_length(IntPtr buf, byte[] str, UIntPtr length, UIntPtr pos);
    [DllImport("eina")] public static extern byte
        eina_binbuf_insert_char(IntPtr buf, byte c, UIntPtr pos);
    [DllImport("eina")] public static extern byte
        eina_binbuf_remove(IntPtr buf, UIntPtr start, UIntPtr end);
    [DllImport("eina")] public static extern IntPtr
        eina_binbuf_string_get(IntPtr buf);
    [DllImport("eina")] public static extern void
        eina_binbuf_string_free(IntPtr buf);
    [DllImport("eina")] public static extern UIntPtr
        eina_binbuf_length_get(IntPtr buf);

    private IntPtr handle = IntPtr.Zero;

    public IntPtr Handle()
    {
        return handle;
    }

    private void InitNew()
    {
        handle = eina_binbuf_new();
        if (handle == IntPtr.Zero)
            throw new SEHException("Could not alloc binbuf");
    }

    public Binbuf()
    {
        InitNew();
    }

    public Binbuf(byte[] str, uint? length = null)
    {
        InitNew();

        if (str != null)
        {
            if (!Append(str, (length != null ? length.Value : (uint)(str.Length))))
            {
                Free();
                throw new SEHException("Could not append on binbuf");
            }
        }
    }

    public Binbuf(Binbuf bb)
    {
        InitNew();

        if (bb != null)
        {
            Append(bb);
        }
    }

    ~Binbuf()
    {
        Dispose(false);
    }

    protected virtual void Dispose(bool disposing)
    {
        if (handle != IntPtr.Zero) {
            Free();
        }
    }

    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    public void Free()
    {
        eina_binbuf_free(handle);
        handle = IntPtr.Zero;
    }

    public void Reset()
    {
        eina_binbuf_reset(handle);
    }

    public bool Append(byte[] str)
    {
        return 0 != eina_binbuf_append_length(handle, str, (UIntPtr)(str.Length));
    }

    public bool Append(byte[] str, uint length)
    {
        return 0 != eina_binbuf_append_length(handle, str, (UIntPtr)length);
    }

    public bool Append(Binbuf bb)
    {
        return 0 != eina_binbuf_append_buffer(handle, bb.Handle());
    }

    public bool Append(byte c)
    {
        return 0 != eina_binbuf_append_char(handle, c);
    }

    public bool Insert(byte[] str, uint pos)
    {
        return 0 != eina_binbuf_insert_length(handle, str, (UIntPtr)(str.Length), (UIntPtr)pos);
    }

    public bool Insert(byte[] str, uint length, uint pos)
    {
        return 0 != eina_binbuf_insert_length(handle, str, (UIntPtr)length, (UIntPtr)pos);
    }

    public bool Insert(byte c, uint pos)
    {
        return 0 != eina_binbuf_insert_char(handle, c, (UIntPtr)pos);
    }

    public bool Remove(uint start, uint end)
    {
        return 0 != eina_binbuf_remove(handle, (UIntPtr)start, (UIntPtr)end);
    }

    public byte[] GetString()
    {
        var ptr = eina_binbuf_string_get(handle);
        if (ptr == IntPtr.Zero)
            return null;

        int size = this.Length();
        byte[] mArray = new byte[size];
        Marshal.Copy(ptr, mArray, 0, size);
        return mArray;
    }

    public IntPtr GetStringUnsafe()
    {
        return eina_binbuf_string_get(handle);
    }

    public void FreeString()
    {
        eina_binbuf_string_free(handle);
    }

    public int Length()
    {
        return (int) eina_binbuf_length_get(handle);
    }
}

}
