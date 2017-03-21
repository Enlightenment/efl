using System;
using System.Linq;
using System.Runtime.InteropServices;

namespace TestSuite
{

class TestEinaStringshare
{
    public static void eina_stringshare_default()
    {
        var str = new eina.Stringshare();
    }
}

class TestEinaBinbuf
{
    private static readonly byte[] test_string = System.Text.Encoding.UTF8.GetBytes("0123456789ABCDEF");

    private static readonly byte[] base_arr = new byte[3]{0x0,0x2A,0x42};

    public static void eina_binbuf_default()
    {
        var binbuf = new eina.Binbuf();
        Test.Assert(binbuf.Handle != IntPtr.Zero);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[0]));
    }

    public static void eina_binbuf_bytes()
    {
        var binbuf = new eina.Binbuf(test_string);
        Test.Assert(binbuf.Handle != IntPtr.Zero);
        byte[] cmp = binbuf.GetBytes();
        Test.Assert(cmp != test_string);
        Test.Assert(cmp.SequenceEqual(test_string));
    }

    public static void eina_binbuf_bytes_length()
    {
        var binbuf = new eina.Binbuf(test_string, 7);
        Test.Assert(binbuf.Handle != IntPtr.Zero);
        byte[] cmp = binbuf.GetBytes();
        byte[] expected = System.Text.Encoding.UTF8.GetBytes("0123456");
        Test.Assert(cmp != test_string);
        Test.Assert(cmp != expected);
        Test.Assert(cmp.SequenceEqual(expected));
    }

    public static void eina_binbuf_copy_ctor()
    {
        var binbuf = new eina.Binbuf(test_string);
        Test.Assert(binbuf.Handle != IntPtr.Zero);
        var binbuf2 = new eina.Binbuf(binbuf);
        Test.Assert(binbuf2.Handle != IntPtr.Zero && binbuf.Handle != binbuf2.Handle);
        byte[] cmp = binbuf.GetBytes();
        byte[] cmp2 = binbuf2.GetBytes();
        Test.Assert(cmp != cmp2);
        Test.Assert(cmp.SequenceEqual(cmp2));
    }

    public static void free_get_null_handle()
    {
        var binbuf = new eina.Binbuf(test_string);
        Test.Assert(binbuf.Handle != IntPtr.Zero);
        binbuf.Free();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
    }

    public static void reset_get_empty_string()
    {
        var binbuf = new eina.Binbuf(test_string);
        Test.Assert(binbuf.Handle != IntPtr.Zero);
        byte[] cmp = binbuf.GetBytes();
        Test.Assert(cmp != test_string);
        Test.Assert(cmp.SequenceEqual(test_string));
        binbuf.Reset();
        Test.Assert(binbuf.Handle != IntPtr.Zero);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[0]));
    }

    public static void append_bytes()
    {
        var binbuf = new eina.Binbuf();
        binbuf.Append(test_string);
        byte[] cmp = binbuf.GetBytes();
        Test.Assert(cmp != test_string);
        Test.Assert(cmp.SequenceEqual(test_string));
    }

    public static void append_bytes_length()
    {
        var binbuf = new eina.Binbuf();
        binbuf.Append(test_string, 7);
        byte[] cmp = binbuf.GetBytes();
        byte[] expected = System.Text.Encoding.UTF8.GetBytes("0123456");
        Test.Assert(cmp != expected);
        Test.Assert(cmp.SequenceEqual(expected));
    }

    public static void append_binbuf()
    {
        var binbuf = new eina.Binbuf();
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[0]));
        var binbuf2 = new eina.Binbuf(test_string);
        binbuf.Append(binbuf2);
        byte[] cmp = binbuf.GetBytes();
        byte[] cmp2 = binbuf2.GetBytes();
        Test.Assert(cmp != cmp2);
        Test.Assert(cmp2.SequenceEqual(cmp));
        Test.Assert(cmp2.SequenceEqual(test_string));
    }

    public static void append_char()
    {
        var binbuf = new eina.Binbuf();
        binbuf.Append((byte) 0);
        binbuf.Append((byte) 12);
        binbuf.Append((byte) 42);
        byte[] cmp = binbuf.GetBytes();
        Test.Assert(cmp.Length == 3);
        Test.Assert(cmp[0] == 0 && cmp[1] == 12 && cmp[2] == 42);
    }

    public static void remove()
    {
        var binbuf = new eina.Binbuf(test_string);
        Test.Assert(binbuf.GetBytes().SequenceEqual(test_string));
        binbuf.Remove(2, 9);
        Test.Assert(binbuf.Handle != IntPtr.Zero);
        byte[] expected = System.Text.Encoding.UTF8.GetBytes("019ABCDEF");
        Test.Assert(binbuf.GetBytes().SequenceEqual(expected));
    }

    public static void get_string_native()
    {
        var binbuf = new eina.Binbuf(test_string);
        Test.Assert(binbuf.GetBytes().SequenceEqual(test_string));
        Test.Assert(binbuf.GetStringNative() != IntPtr.Zero);
    }

    public static void binbuf_free_string()
    {
        var binbuf = new eina.Binbuf(test_string);
        Test.Assert(binbuf.GetBytes().SequenceEqual(test_string));
        binbuf.FreeString();
        Test.Assert(binbuf.Handle != IntPtr.Zero);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[0]));
    }

    public static void binbuf_length()
    {
        var binbuf = new eina.Binbuf(test_string, 6);
        Test.Assert(binbuf.Length == 6);
        Test.Assert(binbuf.GetBytes().Length == 6);
    }

    public static void test_eina_binbuf_in()
    {
        test.Testing t = new test.TestingConcrete();
        var binbuf = new eina.Binbuf(base_arr, (uint)base_arr.Length);
        Test.Assert(t.eina_binbuf_in(binbuf));
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{43,42,0x0,0x2A,0x42,33}));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
    }

    public static void test_eina_binbuf_in_own()
    {
        test.Testing t = new test.TestingConcrete();
        var binbuf = new eina.Binbuf(base_arr, (uint)base_arr.Length);
        Test.Assert(t.eina_binbuf_in_own(binbuf));
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{43,42,0x0,0x2A,0x42,33}));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
        Test.Assert(t.check_binbuf_in_own());
    }

    public static void test_eina_binbuf_out()
    {
        test.Testing t = new test.TestingConcrete();
        eina.Binbuf binbuf;
        Test.Assert(t.eina_binbuf_out(out binbuf));
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{33}));
        binbuf.Reset();
        Test.Assert(binbuf.Append(base_arr));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
        Test.Assert(t.check_binbuf_out());
    }

    public static void test_eina_binbuf_out_own()
    {
        test.Testing t = new test.TestingConcrete();
        eina.Binbuf binbuf;
        Test.Assert(t.eina_binbuf_out_own(out binbuf));
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{33}));
        binbuf.Reset();
        Test.Assert(binbuf.Append(base_arr));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
    }

    public static void test_eina_binbuf_return()
    {
        test.Testing t = new test.TestingConcrete();
        var binbuf = t.eina_binbuf_return();
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{33}));
        binbuf.Reset();
        Test.Assert(binbuf.Append(base_arr));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
        Test.Assert(t.check_binbuf_return());
    }

    public static void test_eina_binbuf_return_own()
    {
        test.Testing t = new test.TestingConcrete();
        var binbuf = t.eina_binbuf_return_own();
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{33}));
        binbuf.Reset();
        Test.Assert(binbuf.Append(base_arr));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
    }
}

class TestEinaSlice
{
    private static readonly byte[] base_arr = new byte[3]{0x0,0x2A,0x42};
    private static readonly GCHandle pinnedData = GCHandle.Alloc(base_arr, GCHandleType.Pinned);
    private static readonly IntPtr pinnedPtr = pinnedData.AddrOfPinnedObject();

    public static void eina_slice_marshalling()
    {
        var binbuf = new eina.Binbuf(base_arr);
        Test.Assert(binbuf.Handle != IntPtr.Zero);

        eina.Slice slc = eina.Binbuf.eina_binbuf_slice_get(binbuf.Handle);

        Test.Assert(slc.GetBytes().SequenceEqual(base_arr));
        Test.Assert(base_arr.Length == (int)(slc.Len));
    }

    public static void eina_slice_size()
    {
        Test.Assert(Marshal.SizeOf(typeof(eina.Slice)) == Marshal.SizeOf(typeof(UIntPtr)) + Marshal.SizeOf(typeof(IntPtr)));
        Test.Assert(Marshal.SizeOf(typeof(eina.Rw_Slice)) == Marshal.SizeOf(typeof(UIntPtr)) + Marshal.SizeOf(typeof(IntPtr)));
    }

    public static void pinned_data_set()
    {
        var binbuf = new eina.Binbuf();
        binbuf.Append(new eina.Slice().PinnedDataSet(pinnedPtr, (UIntPtr)3));
        Test.Assert(binbuf.GetBytes().SequenceEqual(base_arr));
    }

    public static void test_eina_slice_in()
    {
        test.Testing t = new test.TestingConcrete();
        var slc = new eina.Slice(pinnedPtr, (UIntPtr)3);
        Test.Assert(t.eina_slice_in(slc));
    }

    public static void test_eina_rw_slice_in()
    {
        var rw_arr = base_arr.Clone();
        GCHandle pinnedRWData = GCHandle.Alloc(rw_arr, GCHandleType.Pinned);
        IntPtr ptr = pinnedRWData.AddrOfPinnedObject();

        var slc = new eina.Rw_Slice(ptr, (UIntPtr)3);

        test.Testing t = new test.TestingConcrete();
        Test.Assert(t.eina_rw_slice_in(slc));

        Test.Assert(slc.GetBytes().SequenceEqual(new byte[3]{0x1,0x2B,0x43}));

        pinnedRWData.Free();
    }

    public static void test_eina_slice_out()
    {
        test.Testing t = new test.TestingConcrete();
        var slc = new eina.Slice();
        Test.Assert(t.eina_slice_out(ref slc));
        Test.Assert(slc.Mem != IntPtr.Zero);
        Test.Assert(slc.Length == base_arr.Length);
        Test.Assert(slc.GetBytes().SequenceEqual(base_arr));
    }

    public static void test_eina_rw_slice_out()
    {
        test.Testing t = new test.TestingConcrete();
        var slc = new eina.Rw_Slice();
        Test.Assert(t.eina_rw_slice_out(ref slc));
        Test.Assert(slc.Mem != IntPtr.Zero);
        Test.Assert(slc.Length == base_arr.Length);
        Test.Assert(slc.GetBytes().SequenceEqual(base_arr));
    }

    /*
    public static void test_eina_slice_return()
    {
    }

    public static void test_eina_rw_slice_return()
    {
    }
    */
}

}
