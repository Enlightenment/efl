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
    private static byte[] test_string = System.Text.Encoding.UTF8.GetBytes("0123456789ABCDEF");

    public static void eina_binbuf_default()
    {
        var binbuf = new eina.Binbuf();
        Test.Assert(binbuf.Handle() != IntPtr.Zero);
        Test.Assert(binbuf.GetString().SequenceEqual(new byte[0]));
    }

    public static void eina_binbuf_bytes()
    {
        var binbuf = new eina.Binbuf(test_string);
        Test.Assert(binbuf.Handle() != IntPtr.Zero);
        byte[] cmp = binbuf.GetString();
        Test.Assert(cmp != test_string);
        Test.Assert(cmp.SequenceEqual(test_string));
    }

    public static void eina_binbuf_bytes_length()
    {
        var binbuf = new eina.Binbuf(test_string, 7);
        Test.Assert(binbuf.Handle() != IntPtr.Zero);
        byte[] cmp = binbuf.GetString();
        byte[] expected = System.Text.Encoding.UTF8.GetBytes("0123456");
        Test.Assert(cmp != test_string);
        Test.Assert(cmp != expected);
        Test.Assert(cmp.SequenceEqual(expected));
    }

    public static void eina_binbuf_copy_ctor()
    {
        var binbuf = new eina.Binbuf(test_string);
        Test.Assert(binbuf.Handle() != IntPtr.Zero);
        var binbuf2 = new eina.Binbuf(binbuf);
        Test.Assert(binbuf2.Handle() != IntPtr.Zero && binbuf.Handle() != binbuf2.Handle());
        byte[] cmp = binbuf.GetString();
        byte[] cmp2 = binbuf2.GetString();
        Test.Assert(cmp != cmp2);
        Test.Assert(cmp.SequenceEqual(cmp2));
    }

    public static void free_get_null_handle()
    {
        var binbuf = new eina.Binbuf(test_string);
        Test.Assert(binbuf.Handle() != IntPtr.Zero);
        binbuf.Free();
        Test.Assert(binbuf.Handle() == IntPtr.Zero);
    }

    public static void reset_get_empty_string()
    {
        var binbuf = new eina.Binbuf(test_string);
        Test.Assert(binbuf.Handle() != IntPtr.Zero);
        byte[] cmp = binbuf.GetString();
        Test.Assert(cmp != test_string);
        Test.Assert(cmp.SequenceEqual(test_string));
        binbuf.Reset();
        Test.Assert(binbuf.Handle() != IntPtr.Zero);
        Test.Assert(binbuf.GetString().SequenceEqual(new byte[0]));
    }

    public static void append_bytes()
    {
        var binbuf = new eina.Binbuf();
        binbuf.Append(test_string);
        byte[] cmp = binbuf.GetString();
        Test.Assert(cmp != test_string);
        Test.Assert(cmp.SequenceEqual(test_string));
    }

    public static void append_bytes_length()
    {
        var binbuf = new eina.Binbuf();
        binbuf.Append(test_string, 7);
        byte[] cmp = binbuf.GetString();
        byte[] expected = System.Text.Encoding.UTF8.GetBytes("0123456");
        Test.Assert(cmp != expected);
        Test.Assert(cmp.SequenceEqual(expected));
    }

    public static void append_binbuf()
    {
        var binbuf = new eina.Binbuf();
        Test.Assert(binbuf.GetString().SequenceEqual(new byte[0]));
        var binbuf2 = new eina.Binbuf(test_string);
        binbuf.Append(binbuf2);
        byte[] cmp = binbuf.GetString();
        byte[] cmp2 = binbuf2.GetString();
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
        byte[] cmp = binbuf.GetString();
        Test.Assert(cmp.Length == 3);
        Test.Assert(cmp[0] == 0 && cmp[1] == 12 && cmp[2] == 42);
    }

    public static void remove()
    {
        var binbuf = new eina.Binbuf(test_string);
        Test.Assert(binbuf.GetString().SequenceEqual(test_string));
        binbuf.Remove(2, 9);
        Test.Assert(binbuf.Handle() != IntPtr.Zero);
        byte[] expected = System.Text.Encoding.UTF8.GetBytes("019ABCDEF");
        Test.Assert(binbuf.GetString().SequenceEqual(expected));
    }

    public static void binbuf_free_string()
    {
        var binbuf = new eina.Binbuf(test_string);
        Test.Assert(binbuf.GetString().SequenceEqual(test_string));
        binbuf.FreeString();
        Test.Assert(binbuf.Handle() != IntPtr.Zero);
        Test.Assert(binbuf.GetString().SequenceEqual(new byte[0]));
    }

    public static void binbuf_length()
    {
        var binbuf = new eina.Binbuf(test_string, 6);
        Test.Assert(binbuf.Length() == 6);
        Test.Assert(binbuf.GetString().Length == 6);
    }
}

class TestEinaSlice
{
    private static readonly byte[] expected_arr = new byte[3]{0x0,0x2A,0x42};
    private static readonly GCHandle pinnedData = GCHandle.Alloc(expected_arr, GCHandleType.Pinned);
    private static readonly IntPtr pinnedPtr = pinnedData.AddrOfPinnedObject();

    public static void eina_slice_marshalling()
    {
        var binbuf = new eina.Binbuf(expected_arr);
        Test.Assert(binbuf.Handle() != IntPtr.Zero);

        eina.Slice slc = eina.Binbuf.eina_binbuf_slice_get(binbuf.Handle());

        Test.Assert(slc.GetBytes().SequenceEqual(expected_arr));
        Test.Assert(expected_arr.Length == (int)(slc.Len));
    }

    public static void eina_slice_size()
    {
        Test.Assert(Marshal.SizeOf(typeof(eina.Slice)) == Marshal.SizeOf(typeof(UIntPtr)) + Marshal.SizeOf(typeof(IntPtr)));
        Test.Assert(Marshal.SizeOf(typeof(eina.Rw_Slice)) == Marshal.SizeOf(typeof(UIntPtr)) + Marshal.SizeOf(typeof(IntPtr)));
    }

    public static void pinned_data_set()
    {
        var binbuf = new eina.Binbuf();
        binbuf.Append(new eina.Slice().PinnedDataSet(pinnedPtr, 3));
        Test.Assert(binbuf.GetString().SequenceEqual(expected_arr));
    }

    public static void test_eina_slice_in()
    {
        test.Testing t = new test.TestingConcrete();
        var slc = new eina.Slice(pinnedPtr, 3);
        Test.Assert(t.eina_slice_in(slc));
    }

    public static void test_eina_rw_slice_in()
    {
        GCHandle pinnedRWData = GCHandle.Alloc(expected_arr, GCHandleType.Pinned);
        IntPtr ptr = pinnedRWData.AddrOfPinnedObject();

        var slc = new eina.Rw_Slice(ptr, 3);

        test.Testing t = new test.TestingConcrete();
        Test.Assert(t.eina_rw_slice_in(slc));

        pinnedRWData.Free();
    }

    public static void test_eina_slice_out()
    {
        test.Testing t = new test.TestingConcrete();
        var slc = new eina.Slice();
        Test.Assert(t.eina_slice_out(out slc));
        Test.Assert(slc.GetBytes().SequenceEqual(expected_arr));
        Test.Assert(expected_arr.Length == (int)(slc.Len));
    }

    public static void test_eina_rw_slice_out()
    {
        test.Testing t = new test.TestingConcrete();
        var slc = new eina.Rw_Slice();
        Test.Assert(t.eina_rw_slice_out(out slc));
        Test.Assert(slc.GetBytes().SequenceEqual(expected_arr));
        Test.Assert(expected_arr.Length == (int)(slc.Len));
    }

    /*
    public static void test_eina_slice_return()
    {
        test.Testing t = new test.TestingConcrete();
        eina.Slice slc = t.eina_slice_return();
        Test.Assert(slc.GetBytes().SequenceEqual(expected_arr));
        Test.Assert(expected_arr.Length == (int)(slc.Len));
    }

    public static void test_eina_rw_slice_return()
    {
        test.Testing t = new test.TestingConcrete();
        eina.Rw_Slice slc = t.eina_rw_slice_return();
        Test.Assert(slc.GetBytes().SequenceEqual(expected_arr));
        Test.Assert(expected_arr.Length == (int)(slc.Len));
    }
    */
}

}
