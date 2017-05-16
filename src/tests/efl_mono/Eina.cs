using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

namespace TestSuite
{

class TestEinaStringshare
{
    // TODO
}

class BaseArray
{
    public static byte[] Values()
    {
        return new byte[3]{0x0,0x2A,0x42};
    }
}

class NativeInheritImpl : test.TestingInherit
{
    public NativeInheritImpl(efl.Object parent = null) : base(parent) {}

    public bool slice_in_flag = false;
    public bool rw_slice_in_flag = false;
    public bool slice_out_flag = false;
    public bool rw_slice_out_flag = false;
    public bool binbuf_in_flag = false;
    public bool binbuf_in_own_flag = false;
    public bool binbuf_out_flag = false;
    public bool binbuf_out_own_flag = false;
    public bool binbuf_return_flag = false;
    public bool binbuf_return_own_flag = false;

    override public bool eina_slice_in(eina.Slice slice)
    {
        slice_in_flag = true;
        return slice.GetBytes().SequenceEqual(BaseArray.Values());
    }

    override public bool eina_rw_slice_in(eina.Rw_Slice slice)
    {
        rw_slice_in_flag = true;
        return slice.GetBytes().SequenceEqual(BaseArray.Values());
    }

    private byte[] slice_out_arr = null;
    private GCHandle slice_out_pinned;
    override public bool eina_slice_out(ref eina.Slice slice)
    {
        slice_out_flag = true;

        slice_out_arr = (byte[]) BaseArray.Values();
        slice_out_pinned = GCHandle.Alloc(slice_out_arr, GCHandleType.Pinned);
        IntPtr ptr = slice_out_pinned.AddrOfPinnedObject();

        slice.Mem = ptr;
        slice.Len = (UIntPtr) slice_out_arr.Length;

        return true;
    }

    private byte[] rw_slice_out_arr = null;
    private GCHandle rw_slice_out_pinned;
    override public bool eina_rw_slice_out(ref eina.Rw_Slice slice)
    {
        rw_slice_out_flag = true;

        rw_slice_out_arr = (byte[]) BaseArray.Values();
        rw_slice_out_pinned = GCHandle.Alloc(rw_slice_out_arr, GCHandleType.Pinned);
        IntPtr ptr = rw_slice_out_pinned.AddrOfPinnedObject();

        slice.Mem = ptr;
        slice.Len = (UIntPtr) rw_slice_out_arr.Length;

        return true;
    }

    // //
    //
    override public bool eina_binbuf_in(eina.Binbuf binbuf)
    {
        binbuf_in_flag = true;

        bool r = binbuf.GetBytes().SequenceEqual(BaseArray.Values());
        r = r && !binbuf.Own;

        binbuf.Insert(42, 0);
        binbuf.Insert(43, 0);
        binbuf.Append(33);

        binbuf.Dispose();

        return r;
    }

    private eina.Binbuf binbuf_in_own_binbuf = null;
    override public bool eina_binbuf_in_own(eina.Binbuf binbuf)
    {
        binbuf_in_own_flag = true;

        bool r = binbuf.GetBytes().SequenceEqual(BaseArray.Values());
        r = r && binbuf.Own;

        binbuf.Insert(42, 0);
        binbuf.Insert(43, 0);
        binbuf.Append(33);

        binbuf_in_own_binbuf = binbuf;

        return r;
    }
    public bool binbuf_in_own_still_usable()
    {
        bool r = binbuf_in_own_binbuf.GetBytes().SequenceEqual(new byte[]{43,42,0x0,0x2A,0x42,33});
        r = r && binbuf_in_own_binbuf.Own;

        binbuf_in_own_binbuf.Dispose();
        binbuf_in_own_binbuf = null;

        return r;
    }

    private eina.Binbuf binbuf_out_binbuf = null;
    override public bool eina_binbuf_out(out eina.Binbuf binbuf)
    {
        binbuf_out_flag = true;

        binbuf = new eina.Binbuf();
        binbuf.Append(33);

        binbuf_out_binbuf = binbuf;

        return true;
    }
    public bool binbuf_out_still_usable()
    {
        bool r = binbuf_out_binbuf.GetBytes().SequenceEqual(BaseArray.Values());
        r = r && binbuf_out_binbuf.Own;

        binbuf_out_binbuf.Dispose();
        binbuf_out_binbuf = null;

        return r;
    }

    private eina.Binbuf binbuf_out_own_binbuf = null;
    override public bool eina_binbuf_out_own(out eina.Binbuf binbuf)
    {
        binbuf_out_own_flag = true;

        binbuf = new eina.Binbuf();
        binbuf.Append(33);

        binbuf_out_own_binbuf = binbuf;

        return true;
    }
    public bool binbuf_out_own_no_longer_own()
    {
        bool r = !binbuf_out_own_binbuf.Own;
        binbuf_out_own_binbuf.Dispose();
        binbuf_out_own_binbuf = null;
        return r;
    }

    private eina.Binbuf binbuf_return_binbuf = null;
    override public eina.Binbuf eina_binbuf_return()
    {
        binbuf_return_flag = true;

        var binbuf = new eina.Binbuf();
        binbuf.Append(33);

        binbuf_return_binbuf = binbuf;

        return binbuf;
    }
    public bool binbuf_return_still_usable()
    {
        bool r = binbuf_return_binbuf.GetBytes().SequenceEqual(BaseArray.Values());
        r = r && binbuf_return_binbuf.Own;

        binbuf_return_binbuf.Dispose();
        binbuf_return_binbuf = null;

        return r;
    }

    private eina.Binbuf binbuf_return_own_binbuf = null;
    override public eina.Binbuf eina_binbuf_return_own()
    {
        binbuf_return_own_flag = true;

        var binbuf = new eina.Binbuf();
        binbuf.Append(33);

        binbuf_return_own_binbuf = binbuf;

        return binbuf;
    }
    public bool binbuf_return_own_no_longer_own()
    {
        bool r = !binbuf_return_own_binbuf.Own;
        binbuf_return_own_binbuf.Dispose();
        binbuf_return_own_binbuf = null;
        return r;
    }
}

class TestEinaBinbuf
{
    private static readonly byte[] test_string = System.Text.Encoding.UTF8.GetBytes("0123456789ABCDEF");

    private static readonly byte[] base_arr = BaseArray.Values();

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
        Test.Assert(binbuf.Own);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{43,42,0x0,0x2A,0x42,33}));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
    }

    public static void test_eina_binbuf_in_own()
    {
        test.Testing t = new test.TestingConcrete();
        var binbuf = new eina.Binbuf(base_arr, (uint)base_arr.Length);
        Test.Assert(t.eina_binbuf_in_own(binbuf));
        Test.Assert(!binbuf.Own);
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
        Test.Assert(!binbuf.Own);
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
        Test.Assert(binbuf.Own);
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
        Test.Assert(!binbuf.Own);
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
        Test.Assert(binbuf.Own);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{33}));
        binbuf.Reset();
        Test.Assert(binbuf.Append(base_arr));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
    }

    // //
    // Inherit
    //
    public static void test_inherit_eina_binbuf_in()
    {
        var t = new NativeInheritImpl();
        var binbuf = new eina.Binbuf(base_arr, (uint)base_arr.Length);
        Test.Assert(NativeInheritImpl.test_testing_eina_binbuf_in(t.raw_handle, binbuf.Handle));
        Test.Assert(t.binbuf_in_flag);
        Test.Assert(binbuf.Own);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{43,42,0x0,0x2A,0x42,33}));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
    }

    public static void test_inherit_eina_binbuf_in_own()
    {
        var t = new NativeInheritImpl();
        var binbuf = new eina.Binbuf(base_arr, (uint)base_arr.Length);
        binbuf.Own = false;
        Test.Assert(NativeInheritImpl.test_testing_eina_binbuf_in_own(t.raw_handle, binbuf.Handle));
        Test.Assert(t.binbuf_in_own_flag);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{43,42,0x0,0x2A,0x42,33}));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
        Test.Assert(t.binbuf_in_own_still_usable());
    }

    public static void test_inherit_eina_binbuf_out()
    {
        var t = new NativeInheritImpl();
        IntPtr bb_hdl;
        Test.Assert(NativeInheritImpl.test_testing_eina_binbuf_out(t.raw_handle, out bb_hdl));
        Test.Assert(t.binbuf_out_flag);
        Test.Assert(bb_hdl != IntPtr.Zero);
        var binbuf = new eina.Binbuf(bb_hdl, false);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{33}));
        binbuf.Reset();
        Test.Assert(binbuf.Append(base_arr));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
        Test.Assert(t.binbuf_out_still_usable());
    }

    public static void test_inherit_eina_binbuf_out_own()
    {
        var t = new NativeInheritImpl();
        IntPtr bb_hdl;
        Test.Assert(NativeInheritImpl.test_testing_eina_binbuf_out_own(t.raw_handle, out bb_hdl));
        Test.Assert(t.binbuf_out_own_flag);
        Test.Assert(bb_hdl != IntPtr.Zero);
        var binbuf = new eina.Binbuf(bb_hdl, true);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{33}));
        binbuf.Reset();
        Test.Assert(binbuf.Append(base_arr));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
        Test.Assert(t.binbuf_out_own_no_longer_own());
    }

    public static void test_inherit_eina_binbuf_return()
    {
        var t = new NativeInheritImpl();
        IntPtr bb_hdl = NativeInheritImpl.test_testing_eina_binbuf_return(t.raw_handle);
        Test.Assert(t.binbuf_return_flag);
        Test.Assert(bb_hdl != IntPtr.Zero);
        var binbuf = new eina.Binbuf(bb_hdl, false);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{33}));
        binbuf.Reset();
        Test.Assert(binbuf.Append(base_arr));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
        Test.Assert(t.binbuf_return_still_usable());
    }

    public static void test_inherit_eina_binbuf_return_own()
    {
        var t = new NativeInheritImpl();
        IntPtr bb_hdl = NativeInheritImpl.test_testing_eina_binbuf_return_own(t.raw_handle);
        Test.Assert(t.binbuf_return_own_flag);
        Test.Assert(bb_hdl != IntPtr.Zero);
        var binbuf = new eina.Binbuf(bb_hdl, true);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{33}));
        binbuf.Reset();
        Test.Assert(binbuf.Append(base_arr));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
        Test.Assert(t.binbuf_return_own_no_longer_own());
    }
}

class TestEinaSlice
{
    private static readonly byte[] base_arr = BaseArray.Values();
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

    public static void test_inherit_eina_slice_in()
    {
        var t = new NativeInheritImpl();
        var slc = new eina.Slice(pinnedPtr, (UIntPtr)3);
        Test.Assert(NativeInheritImpl.test_testing_eina_slice_in(t.raw_handle, slc));
        Test.Assert(t.slice_in_flag);
    }

    public static void test_inherit_eina_rw_slice_in()
    {
        var rw_arr = base_arr.Clone();
        GCHandle pinnedRWData = GCHandle.Alloc(rw_arr, GCHandleType.Pinned);
        IntPtr ptr = pinnedRWData.AddrOfPinnedObject();

        var slc = new eina.Rw_Slice(ptr, (UIntPtr)3);

        var t = new NativeInheritImpl();
        Test.Assert(NativeInheritImpl.test_testing_eina_rw_slice_in(t.raw_handle, slc));

        Test.Assert(t.rw_slice_in_flag);
        Test.Assert(slc.GetBytes().SequenceEqual(base_arr));

        pinnedRWData.Free();
    }

    public static void test_inherit_eina_slice_out()
    {
        var t = new NativeInheritImpl();
        var slc = new eina.Slice();
        Test.Assert(NativeInheritImpl.test_testing_eina_slice_out(t.raw_handle, ref slc));
        Test.Assert(t.slice_out_flag);
        Test.Assert(slc.Mem != IntPtr.Zero);
        Test.Assert(slc.Length == base_arr.Length);
        Test.Assert(slc.GetBytes().SequenceEqual(base_arr));
    }

    public static void test_inherit_eina_rw_slice_out()
    {
        var t = new NativeInheritImpl();
        var slc = new eina.Rw_Slice();
        Test.Assert(NativeInheritImpl.test_testing_eina_rw_slice_out(t.raw_handle, ref slc));
        Test.Assert(t.rw_slice_out_flag);
        Test.Assert(slc.Mem != IntPtr.Zero);
        Test.Assert(slc.Length == base_arr.Length);
        Test.Assert(slc.GetBytes().SequenceEqual(base_arr));
    }
}

class TestEinaArray
{
    private static readonly string test_string = "abcdefghij";

    private static readonly int[] base_arr_int = {0x0,0x2A,0x42};
    private static readonly int[] append_arr_int = {42,43,33};
    private static readonly int[] modified_arr_int = {0x0,0x2A,0x42,42,43,33};

    private static readonly string[] base_arr_str = {"0x0","0x2A","0x42"};
    private static readonly string[] append_arr_str = {"42","43","33"};
    private static readonly string[] modified_arr_str = {"0x0","0x2A","0x42","42","43","33"};

    private static test.Numberwrapper[] BaseArrObj()
    {
        var a = new test.NumberwrapperConcrete();
        var b = new test.NumberwrapperConcrete();
        var c = new test.NumberwrapperConcrete();
        a.number_set(0x0);
        b.number_set(0x2A);
        c.number_set(0x42);
        return new test.NumberwrapperConcrete[]{a,b,c};
    }

    private static test.Numberwrapper[] AppendArrObj()
    {
        var a = new test.NumberwrapperConcrete();
        var b = new test.NumberwrapperConcrete();
        var c = new test.NumberwrapperConcrete();
        a.number_set(42);
        b.number_set(43);
        c.number_set(33);
        return new test.NumberwrapperConcrete[]{a,b,c};
    }

    private static test.Numberwrapper[] ModifiedArrObj()
    {
        var a = new test.NumberwrapperConcrete();
        var b = new test.NumberwrapperConcrete();
        var c = new test.NumberwrapperConcrete();
        var d = new test.NumberwrapperConcrete();
        var e = new test.NumberwrapperConcrete();
        var f = new test.NumberwrapperConcrete();
        a.number_set(0x0);
        b.number_set(0x2A);
        c.number_set(0x42);
        d.number_set(42);
        e.number_set(43);
        f.number_set(33);
        return new test.NumberwrapperConcrete[]{a,b,c,d,e,f};
    }


    public static void eina_array_default()
    {
        var a = new eina.Array<int>();
        Test.Assert(a.Handle != IntPtr.Zero);
    }

    public static void push_int()
    {
        var a = new eina.Array<int>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push(88));
        Test.Assert(a[0] == 88);
    }

    public static void push_string()
    {
        var a = new eina.Array<string>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push(test_string));
        Test.Assert(a[0] == test_string);
    }

    public static void push_obj()
    {
        var a = new eina.Array<test.NumberwrapperConcrete>();
        Test.Assert(a.Handle != IntPtr.Zero);
        var o = new test.NumberwrapperConcrete();
        o.number_set(88);
        Test.Assert(a.Push(o));
        Test.Assert(a[0].raw_handle == o.raw_handle);
        Test.Assert(a[0].number_get() == 88);
    }

    public static void pop_int()
    {
        var a = new eina.Array<int>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push(88));
        Test.Assert(a.Pop() == 88);
        Test.Assert(a.Count() == 0);
    }

    public static void pop_string()
    {
        var a = new eina.Array<string>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push(test_string));
        Test.Assert(a.Pop() == test_string);
        Test.Assert(a.Count() == 0);
    }

    public static void pop_obj()
    {
        var a = new eina.Array<test.NumberwrapperConcrete>();
        Test.Assert(a.Handle != IntPtr.Zero);
        var o = new test.NumberwrapperConcrete();
        o.number_set(88);
        Test.Assert(a.Push(o));
        var p = a.Pop();
        Test.Assert(p.raw_handle == o.raw_handle);
        Test.Assert(p.number_get() == 88);
        Test.Assert(a.Count() == 0);
    }

    public static void data_set_int()
    {
        var a = new eina.Array<int>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push(88));
        Test.Assert(a[0] == 88);
        a.DataSet(0, 44);
        Test.Assert(a[0] == 44);
        a[0] = 22;
        Test.Assert(a[0] == 22);
    }

    public static void data_set_string()
    {
        var a = new eina.Array<string>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push(test_string));
        Test.Assert(a[0] == test_string);
        a.DataSet(0, "other string");
        Test.Assert(a[0] == "other string");
        a[0] = "abc";
        Test.Assert(a[0] == "abc");
    }

    public static void data_set_obj()
    {
        var a = new eina.Array<test.NumberwrapperConcrete>();
        Test.Assert(a.Handle != IntPtr.Zero);

        var o1 = new test.NumberwrapperConcrete();
        o1.number_set(88);

        Test.Assert(a.Push(o1));
        Test.Assert(a[0].raw_handle == o1.raw_handle);
        Test.Assert(a[0].number_get() == 88);

        var o2 = new test.NumberwrapperConcrete();
        o2.number_set(44);

        a.DataSet(0, o2);
        Test.Assert(a[0].raw_handle == o2.raw_handle);
        Test.Assert(a[0].number_get() == 44);

        var o3 = new test.NumberwrapperConcrete();
        o3.number_set(22);

        a[0] = o3;
        Test.Assert(a[0].raw_handle == o3.raw_handle);
        Test.Assert(a[0].number_get() == 22);
    }

    public static void count_int()
    {
        var a = new eina.Array<int>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Count() == 0);
        Test.Assert(a.Push(88));
        Test.Assert(a[0] == 88);
        Test.Assert(a.Count() == 1);
        Test.Assert(a.Push(44));
        Test.Assert(a[1] == 44);
        Test.Assert(a.Count() == 2);
        Test.Assert(a.Push(22));
        Test.Assert(a[2] == 22);
        Test.Assert(a.Count() == 3);
    }

    public static void count_string()
    {
        var a = new eina.Array<string>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Count() == 0);
        Test.Assert(a.Push("a"));
        Test.Assert(a[0] == "a");
        Test.Assert(a.Count() == 1);
        Test.Assert(a.Push("b"));
        Test.Assert(a[1] == "b");
        Test.Assert(a.Count() == 2);
        Test.Assert(a.Push("c"));
        Test.Assert(a[2] == "c");
        Test.Assert(a.Count() == 3);
    }

    public static void count_obj()
    {
        var a = new eina.Array<test.NumberwrapperConcrete>();
        Test.Assert(a.Handle != IntPtr.Zero);

        Test.Assert(a.Count() == 0);

        var o1 = new test.NumberwrapperConcrete();
        o1.number_set(88);
        Test.Assert(a.Push(o1));
        Test.Assert(a[0].raw_handle == o1.raw_handle);
        Test.Assert(a[0].number_get() == 88);
        Test.Assert(a.Count() == 1);

        var o2 = new test.NumberwrapperConcrete();
        o2.number_set(44);
        Test.Assert(a.Push(o2));
        Test.Assert(a[1].raw_handle == o2.raw_handle);
        Test.Assert(a[1].number_get() == 44);
        Test.Assert(a.Count() == 2);

        var o3 = new test.NumberwrapperConcrete();
        o3.number_set(22);
        Test.Assert(a.Push(o3));
        Test.Assert(a[2].raw_handle == o3.raw_handle);
        Test.Assert(a[2].number_get() == 22);
        Test.Assert(a.Count() == 3);
    }

    public static void length_int()
    {
        var a = new eina.Array<int>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Length == 0);
        Test.Assert(a.Push(88));
        Test.Assert(a[0] == 88);
        Test.Assert(a.Length == 1);
        Test.Assert(a.Push(44));
        Test.Assert(a[1] == 44);
        Test.Assert(a.Length == 2);
        Test.Assert(a.Push(22));
        Test.Assert(a[2] == 22);
        Test.Assert(a.Length == 3);
    }

    public static void length_string()
    {
        var a = new eina.Array<string>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Length == 0);
        Test.Assert(a.Push("a"));
        Test.Assert(a[0] == "a");
        Test.Assert(a.Length == 1);
        Test.Assert(a.Push("b"));
        Test.Assert(a[1] == "b");
        Test.Assert(a.Length == 2);
        Test.Assert(a.Push("c"));
        Test.Assert(a[2] == "c");
        Test.Assert(a.Length == 3);
    }

    public static void eina_array_as_ienumerable_int()
    {
        var a = new eina.Array<int>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push(88));
        Test.Assert(a.Push(44));
        Test.Assert(a.Push(22));

        int cmp = 88;
        foreach (int e in a)
        {
            Test.AssertEquals(cmp, e);
            cmp /= 2;
        }
    }

    public static void eina_array_as_ienumerable_string()
    {
        var a = new eina.Array<string>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push("X"));
        Test.Assert(a.Push("XX"));
        Test.Assert(a.Push("XXX"));

        string cmp = "X";
        foreach (string e in a)
        {
            Test.AssertEquals(cmp, e);
            cmp = cmp + "X";
        }
    }

    public static void eina_array_as_ienumerable_obj()
    {
        var a = new test.NumberwrapperConcrete();
        var b = new test.NumberwrapperConcrete();
        var c = new test.NumberwrapperConcrete();
        a.number_set(88);
        b.number_set(44);
        c.number_set(22);
        var cmp = new test.NumberwrapperConcrete[]{a,b,c};

        var arr = new eina.Array<test.NumberwrapperConcrete>();
        Test.Assert(arr.Handle != IntPtr.Zero);
        Test.Assert(arr.Push(a));
        Test.Assert(arr.Push(b));
        Test.Assert(arr.Push(c));

        int i = 0;
        foreach (test.NumberwrapperConcrete e in arr)
        {
            Test.AssertEquals(cmp[i].number_get(), e.number_get());
            Test.Assert(cmp[i].raw_handle == e.raw_handle);
            ++i;
        }
    }

    // //
    // Code Generation
    //

    // Integer //

    public static void test_eina_array_int_in()
    {
        test.Testing t = new test.TestingConcrete();
        var arr = new eina.Array<int>();
        arr.Append(base_arr_int);
        Test.Assert(t.eina_array_int_in(arr));
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(modified_arr_int));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_array_int_in_own()
    {
        test.Testing t = new test.TestingConcrete();
        var arr = new eina.Array<int>();
        arr.Append(base_arr_int);
        Test.Assert(t.eina_array_int_in_own(arr));
        Test.Assert(!arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(modified_arr_int));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.check_eina_array_int_in_own());
    }

    public static void test_eina_array_int_out()
    {
        test.Testing t = new test.TestingConcrete();
        eina.Array<int> arr;
        Test.Assert(t.eina_array_int_out(out arr));
        Test.Assert(!arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_arr_int));
        Test.Assert(arr.Append(append_arr_int));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.check_eina_array_int_out());
    }

    public static void test_eina_array_int_out_own()
    {
        test.Testing t = new test.TestingConcrete();
        eina.Array<int> arr;
        Test.Assert(t.eina_array_int_out_own(out arr));
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_arr_int));
        Test.Assert(arr.Append(append_arr_int));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_array_int_return()
    {
        test.Testing t = new test.TestingConcrete();
        var arr = t.eina_array_int_return();
        Test.Assert(!arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_arr_int));
        Test.Assert(arr.Append(append_arr_int));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.check_eina_array_int_return());
    }

    public static void test_eina_array_int_return_own()
    {
        test.Testing t = new test.TestingConcrete();
        var arr = t.eina_array_int_return_own();
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_arr_int));
        Test.Assert(arr.Append(append_arr_int));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    // String //
    public static void test_eina_array_str_in()
    {
        test.Testing t = new test.TestingConcrete();
        var arr = new eina.Array<string>();
        arr.Append(base_arr_str);
        Test.Assert(t.eina_array_str_in(arr));
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(modified_arr_str));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_array_str_in_own()
    {
        test.Testing t = new test.TestingConcrete();
        var arr = new eina.Array<string>();
        arr.Append(base_arr_str);
        Test.Assert(t.eina_array_str_in_own(arr));
        Test.Assert(!arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(modified_arr_str));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.check_eina_array_str_in_own());
    }

    public static void test_eina_array_str_out()
    {
        test.Testing t = new test.TestingConcrete();
        eina.Array<string> arr;
        Test.Assert(t.eina_array_str_out(out arr));
        Test.Assert(!arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_arr_str));
        Test.Assert(arr.Append(append_arr_str));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.check_eina_array_str_out());
    }

    public static void test_eina_array_str_out_own()
    {
        test.Testing t = new test.TestingConcrete();
        eina.Array<string> arr;
        Test.Assert(t.eina_array_str_out_own(out arr));
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_arr_str));
        Test.Assert(arr.Append(append_arr_str));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_array_str_return()
    {
        test.Testing t = new test.TestingConcrete();
        var arr = t.eina_array_str_return();
        Test.Assert(!arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_arr_str));
        Test.Assert(arr.Append(append_arr_str));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.check_eina_array_str_return());
    }

    public static void test_eina_array_str_return_own()
    {
        test.Testing t = new test.TestingConcrete();
        var arr = t.eina_array_str_return_own();
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_arr_str));
        Test.Assert(arr.Append(append_arr_str));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    // Object //

    private static void NumberwrapperArrayAssertEqual(
        test.Numberwrapper[] a
        , test.Numberwrapper[] b
        , [CallerLineNumber] int line = 0
        , [CallerFilePath] string file = null
        , [CallerMemberName] string member = null)
    {
        Test.Assert(a.Length == b.Length, "Different lenght", line, file, member);
        for (int i = 0; i < a.Length; ++i)
        {
            int av = a[i].number_get();
            int bv = b[i].number_get();
            Test.Assert(av == bv, $"Different values for element [{i}]: {av} == {bv}", line, file, member);
        }
    }


    public static void test_eina_array_obj_in()
    {
        test.Testing t = new test.TestingConcrete();
        var arr = new eina.Array<test.Numberwrapper>();
        arr.Append(BaseArrObj());
        Test.Assert(t.eina_array_obj_in(arr));
        Test.Assert(arr.Own);
        NumberwrapperArrayAssertEqual(arr.ToArray(), ModifiedArrObj());
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_array_obj_in_own()
    {
        test.Testing t = new test.TestingConcrete();
        var arr = new eina.Array<test.Numberwrapper>();
        arr.Append(BaseArrObj());
        Test.Assert(t.eina_array_obj_in_own(arr));
        Test.Assert(!arr.Own);
        NumberwrapperArrayAssertEqual(arr.ToArray(), ModifiedArrObj());
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.check_eina_array_obj_in_own());
    }

    public static void test_eina_array_obj_out()
    {
        test.Testing t = new test.TestingConcrete();
        eina.Array<test.Numberwrapper> arr;
        Test.Assert(t.eina_array_obj_out(out arr));
        Test.Assert(!arr.Own);
        NumberwrapperArrayAssertEqual(arr.ToArray(), BaseArrObj());
        Test.Assert(arr.Append(AppendArrObj()));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.check_eina_array_obj_out());
    }

    public static void test_eina_array_obj_out_own()
    {
        test.Testing t = new test.TestingConcrete();
        eina.Array<test.Numberwrapper> arr;
        Test.Assert(t.eina_array_obj_out_own(out arr));
        Test.Assert(arr.Own);
        NumberwrapperArrayAssertEqual(arr.ToArray(), BaseArrObj());
        Test.Assert(arr.Append(AppendArrObj()));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_array_obj_return()
    {
        test.Testing t = new test.TestingConcrete();
        var arr = t.eina_array_obj_return();
        Test.Assert(!arr.Own);
        NumberwrapperArrayAssertEqual(arr.ToArray(), BaseArrObj());
        Test.Assert(arr.Append(AppendArrObj()));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.check_eina_array_obj_return());
    }

    public static void test_eina_array_obj_return_own()
    {
        test.Testing t = new test.TestingConcrete();
        var arr = t.eina_array_obj_return_own();
        Test.Assert(arr.Own);
        NumberwrapperArrayAssertEqual(arr.ToArray(), BaseArrObj());
        Test.Assert(arr.Append(AppendArrObj()));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_array_obj_return_in_same_id()
    {
        test.Testing t = new test.TestingConcrete();
        var cmp = BaseArrObj();
        var a = new eina.Array<test.Numberwrapper>();
        a.Append(cmp);
        var b = t.eina_array_obj_return_in(a);
        NumberwrapperArrayAssertEqual(a.ToArray(), b.ToArray());
        NumberwrapperArrayAssertEqual(a.ToArray(), BaseArrObj());
        int len = a.Length;
        for (int i=0; i < len; ++i)
        {
            Test.Assert(a[i].raw_handle == b[i].raw_handle);
            Test.Assert(a[i].raw_handle == cmp[i].raw_handle);
        }
    }


    // //
    // Inherit
    //
}

}
