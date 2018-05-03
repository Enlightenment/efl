using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

using EinaTestData;
using static EinaTestData.BaseData;

namespace EinaTestData
{

class BaseSequence
{
    public static byte[] Values()
    {
        return new byte[3]{0x0,0x2A,0x42};
    }
}

public static class BaseData
{
    public static readonly int[] base_seq_int = {0x0,0x2A,0x42};
    public static readonly int[] append_seq_int = {42,43,33};
    public static readonly int[] modified_seq_int = {0x0,0x2A,0x42,42,43,33};

    public static readonly string[] base_seq_str = {"0x0","0x2A","0x42"};
    public static readonly string[] append_seq_str = {"42","43","33"};
    public static readonly string[] modified_seq_str = {"0x0","0x2A","0x42","42","43","33"};

    public static test.INumberwrapper NW(int n)
    {
        var nw = new test.Numberwrapper();
        nw.SetNumber(n);
        return nw;
    }

    public static test.INumberwrapper[] BaseSeqObj()
    {
        var a = new test.Numberwrapper();
        var b = new test.Numberwrapper();
        var c = new test.Numberwrapper();
        a.SetNumber(0x0);
        b.SetNumber(0x2A);
        c.SetNumber(0x42);
        return new test.INumberwrapper[]{a,b,c};
    }

    public static test.Numberwrapper[] AppendSeqObj()
    {
        var a = new test.Numberwrapper();
        var b = new test.Numberwrapper();
        var c = new test.Numberwrapper();
        a.SetNumber(42);
        b.SetNumber(43);
        c.SetNumber(33);
        return new test.Numberwrapper[]{a,b,c};
    }

    public static test.Numberwrapper[] ModifiedSeqObj()
    {
        var a = new test.Numberwrapper();
        var b = new test.Numberwrapper();
        var c = new test.Numberwrapper();
        var d = new test.Numberwrapper();
        var e = new test.Numberwrapper();
        var f = new test.Numberwrapper();
        a.SetNumber(0x0);
        b.SetNumber(0x2A);
        c.SetNumber(0x42);
        d.SetNumber(42);
        e.SetNumber(43);
        f.SetNumber(33);
        return new test.Numberwrapper[]{a,b,c,d,e,f};
    }

    public static void NumberwrapperSequenceAssertEqual(
        test.INumberwrapper[] a
        , test.INumberwrapper[] b
        , [CallerLineNumber] int line = 0
        , [CallerFilePath] string file = null
        , [CallerMemberName] string member = null)
    {
        Test.Assert(a.Length == b.Length, "Different lenght", line, file, member);
        for (int i = 0; i < a.Length; ++i)
        {
            int av = a[i].GetNumber();
            int bv = b[i].GetNumber();
            Test.Assert(av == bv, $"Different values for element [{i}]: {av} == {bv}", line, file, member);
        }
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

    override public bool EinaSliceIn(eina.Slice slice)
    {
        slice_in_flag = true;
        return slice.GetBytes().SequenceEqual(BaseSequence.Values());
    }

    override public bool EinaRwSliceIn(eina.Rw_Slice slice)
    {
        rw_slice_in_flag = true;
        return slice.GetBytes().SequenceEqual(BaseSequence.Values());
    }

    private byte[] slice_out_seq = null;
    private GCHandle slice_out_pinned;
    override public bool EinaSliceOut(ref eina.Slice slice)
    {
        slice_out_flag = true;

        slice_out_seq = (byte[]) BaseSequence.Values();
        slice_out_pinned = GCHandle.Alloc(slice_out_seq, GCHandleType.Pinned);
        IntPtr ptr = slice_out_pinned.AddrOfPinnedObject();

        slice.Mem = ptr;
        slice.Len = (UIntPtr) slice_out_seq.Length;

        return true;
    }

    private byte[] rw_slice_out_seq = null;
    private GCHandle rw_slice_out_pinned;
    override public bool EinaRwSliceOut(ref eina.Rw_Slice slice)
    {
        rw_slice_out_flag = true;

        rw_slice_out_seq = (byte[]) BaseSequence.Values();
        rw_slice_out_pinned = GCHandle.Alloc(rw_slice_out_seq, GCHandleType.Pinned);
        IntPtr ptr = rw_slice_out_pinned.AddrOfPinnedObject();

        slice.Mem = ptr;
        slice.Len = (UIntPtr) rw_slice_out_seq.Length;

        return true;
    }

    // //
    //
    override public bool EinaBinbufIn(eina.Binbuf binbuf)
    {
        binbuf_in_flag = true;

        bool r = binbuf.GetBytes().SequenceEqual(BaseSequence.Values());
        r = r && !binbuf.Own;

        binbuf.Insert(42, 0);
        binbuf.Insert(43, 0);
        binbuf.Append(33);

        binbuf.Dispose();

        return r;
    }

    private eina.Binbuf binbuf_in_own_binbuf = null;
    override public bool EinaBinbufInOwn(eina.Binbuf binbuf)
    {
        binbuf_in_own_flag = true;

        bool r = binbuf.GetBytes().SequenceEqual(BaseSequence.Values());
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
    override public bool EinaBinbufOut(out eina.Binbuf binbuf)
    {
        binbuf_out_flag = true;

        binbuf = new eina.Binbuf();
        binbuf.Append(33);

        binbuf_out_binbuf = binbuf;

        return true;
    }
    public bool binbuf_out_still_usable()
    {
        bool r = binbuf_out_binbuf.GetBytes().SequenceEqual(BaseSequence.Values());
        r = r && binbuf_out_binbuf.Own;

        binbuf_out_binbuf.Dispose();
        binbuf_out_binbuf = null;

        return r;
    }

    private eina.Binbuf binbuf_out_own_binbuf = null;
    override public bool EinaBinbufOutOwn(out eina.Binbuf binbuf)
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
    override public eina.Binbuf EinaBinbufReturn()
    {
        binbuf_return_flag = true;

        var binbuf = new eina.Binbuf();
        binbuf.Append(33);

        binbuf_return_binbuf = binbuf;

        return binbuf;
    }
    public bool binbuf_return_still_usable()
    {
        bool r = binbuf_return_binbuf.GetBytes().SequenceEqual(BaseSequence.Values());
        r = r && binbuf_return_binbuf.Own;

        binbuf_return_binbuf.Dispose();
        binbuf_return_binbuf = null;

        return r;
    }

    private eina.Binbuf binbuf_return_own_binbuf = null;
    override public eina.Binbuf EinaBinbufReturnOwn()
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

} // EinaTestData

namespace TestSuite
{

class TestEinaBinbuf
{
    private static readonly byte[] test_string = System.Text.Encoding.UTF8.GetBytes("0123456789ABCDEF");

    private static readonly byte[] base_seq = BaseSequence.Values();

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
        test.ITesting t = new test.Testing();
        var binbuf = new eina.Binbuf(base_seq, (uint)base_seq.Length);
        Test.Assert(t.EinaBinbufIn(binbuf));
        Test.Assert(binbuf.Own);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{43,42,0x0,0x2A,0x42,33}));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
    }

    public static void test_eina_binbuf_in_own()
    {
        test.ITesting t = new test.Testing();
        var binbuf = new eina.Binbuf(base_seq, (uint)base_seq.Length);
        Test.Assert(t.EinaBinbufInOwn(binbuf));
        Test.Assert(!binbuf.Own);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{43,42,0x0,0x2A,0x42,33}));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
        Test.Assert(t.CheckBinbufInOwn());
    }

    public static void test_eina_binbuf_out()
    {
        test.ITesting t = new test.Testing();
        eina.Binbuf binbuf;
        Test.Assert(t.EinaBinbufOut(out binbuf));
        Test.Assert(!binbuf.Own);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{33}));
        binbuf.Reset();
        Test.Assert(binbuf.Append(base_seq));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
        Test.Assert(t.CheckBinbufOut());
    }

    public static void test_eina_binbuf_out_own()
    {
        test.ITesting t = new test.Testing();
        eina.Binbuf binbuf;
        Test.Assert(t.EinaBinbufOutOwn(out binbuf));
        Test.Assert(binbuf.Own);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{33}));
        binbuf.Reset();
        Test.Assert(binbuf.Append(base_seq));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
    }

    public static void test_eina_binbuf_return()
    {
        test.ITesting t = new test.Testing();
        var binbuf = t.EinaBinbufReturn();
        Test.Assert(!binbuf.Own);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{33}));
        binbuf.Reset();
        Test.Assert(binbuf.Append(base_seq));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
        Test.Assert(t.CheckBinbufReturn());
    }

    public static void test_eina_binbuf_return_own()
    {
        test.ITesting t = new test.Testing();
        var binbuf = t.EinaBinbufReturnOwn();
        Test.Assert(binbuf.Own);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{33}));
        binbuf.Reset();
        Test.Assert(binbuf.Append(base_seq));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
    }

    // //
    // Inherit
    //
    public static void test_inherit_eina_binbuf_in()
    {
        var t = new NativeInheritImpl();
        var binbuf = new eina.Binbuf(base_seq, (uint)base_seq.Length);
        Test.Assert(t.CallEinaBinbufIn(binbuf));
        Test.Assert(t.binbuf_in_flag);
        Test.Assert(binbuf.Own);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{43,42,0x0,0x2A,0x42,33}));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
    }

    public static void test_inherit_eina_binbuf_in_own()
    {
        var t = new NativeInheritImpl();
        var binbuf = new eina.Binbuf(base_seq, (uint)base_seq.Length);
        binbuf.Own = false;
        Test.Assert(t.CallEinaBinbufInOwn(binbuf));
        Test.Assert(t.binbuf_in_own_flag);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{43,42,0x0,0x2A,0x42,33}));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
        Test.Assert(t.binbuf_in_own_still_usable());
    }

    public static void test_inherit_eina_binbuf_out()
    {
        var t = new NativeInheritImpl();
        eina.Binbuf binbuf = t.CallEinaBinbufOut();
        Test.Assert(t.binbuf_out_flag);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{33}));
        binbuf.Reset();
        Test.Assert(binbuf.Append(base_seq));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
        Test.Assert(t.binbuf_out_still_usable());
    }

    public static void test_inherit_eina_binbuf_out_own()
    {
        var t = new NativeInheritImpl();
        eina.Binbuf binbuf = t.CallEinaBinbufOutOwn();
        Test.Assert(t.binbuf_out_own_flag);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{33}));
        binbuf.Reset();
        Test.Assert(binbuf.Append(base_seq));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
        Test.Assert(t.binbuf_out_own_no_longer_own());
    }

    public static void test_inherit_eina_binbuf_return()
    {
        var t = new NativeInheritImpl();
        var binbuf = t.CallEinaBinbufReturn();
        Test.Assert(t.binbuf_return_flag);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{33}));
        binbuf.Reset();
        Test.Assert(binbuf.Append(base_seq));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
        Test.Assert(t.binbuf_return_still_usable());
    }

    public static void test_inherit_eina_binbuf_return_own()
    {
        var t = new NativeInheritImpl();
        var binbuf = t.CallEinaBinbufReturnOwn();
        Test.Assert(t.binbuf_return_own_flag);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{33}));
        binbuf.Reset();
        Test.Assert(binbuf.Append(base_seq));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
        Test.Assert(t.binbuf_return_own_no_longer_own());
    }
}

class TestEinaSlice
{
    private static readonly byte[] base_seq = BaseSequence.Values();
    private static readonly GCHandle pinnedData = GCHandle.Alloc(base_seq, GCHandleType.Pinned);
    private static readonly IntPtr pinnedPtr = pinnedData.AddrOfPinnedObject();

    public static void eina_slice_marshalling()
    {
        var binbuf = new eina.Binbuf(base_seq);
        Test.Assert(binbuf.Handle != IntPtr.Zero);

        eina.Slice slc = eina.Binbuf.eina_binbuf_slice_get(binbuf.Handle);

        Test.Assert(slc.GetBytes().SequenceEqual(base_seq));
        Test.Assert(base_seq.Length == (int)(slc.Len));
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
        Test.Assert(binbuf.GetBytes().SequenceEqual(base_seq));
    }

    public static void test_eina_slice_in()
    {
        test.ITesting t = new test.Testing();
        var slc = new eina.Slice(pinnedPtr, (UIntPtr)3);
        Test.Assert(t.EinaSliceIn(slc));
    }

    public static void test_eina_rw_slice_in()
    {
        var rw_seq = base_seq.Clone();
        GCHandle pinnedRWData = GCHandle.Alloc(rw_seq, GCHandleType.Pinned);
        IntPtr ptr = pinnedRWData.AddrOfPinnedObject();

        var slc = new eina.Rw_Slice(ptr, (UIntPtr)3);

        test.ITesting t = new test.Testing();
        Test.Assert(t.EinaRwSliceIn(slc));

        Test.Assert(slc.GetBytes().SequenceEqual(new byte[3]{0x1,0x2B,0x43}));

        pinnedRWData.Free();
    }

    public static void test_eina_slice_out()
    {
        test.ITesting t = new test.Testing();
        var slc = new eina.Slice();
        Test.Assert(t.EinaSliceOut(ref slc));
        Test.Assert(slc.Mem != IntPtr.Zero);
        Test.Assert(slc.Length == base_seq.Length);
        Test.Assert(slc.GetBytes().SequenceEqual(base_seq));
    }

    public static void test_eina_rw_slice_out()
    {
        test.ITesting t = new test.Testing();
        var slc = new eina.Rw_Slice();
        Test.Assert(t.EinaRwSliceOut(ref slc));
        Test.Assert(slc.Mem != IntPtr.Zero);
        Test.Assert(slc.Length == base_seq.Length);
        Test.Assert(slc.GetBytes().SequenceEqual(base_seq));
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
        Test.Assert(t.EinaSliceIn(slc));
        Test.Assert(t.slice_in_flag);
    }

    public static void test_inherit_eina_rw_slice_in()
    {
        var rw_seq = base_seq.Clone();
        GCHandle pinnedRWData = GCHandle.Alloc(rw_seq, GCHandleType.Pinned);
        IntPtr ptr = pinnedRWData.AddrOfPinnedObject();

        var slc = new eina.Rw_Slice(ptr, (UIntPtr)3);

        var t = new NativeInheritImpl();
        Test.Assert(t.EinaRwSliceIn(slc));

        Test.Assert(t.rw_slice_in_flag);
        Test.Assert(slc.GetBytes().SequenceEqual(base_seq));

        pinnedRWData.Free();
    }

    public static void test_inherit_eina_slice_out()
    {
        var t = new NativeInheritImpl();
        var slc = new eina.Slice();
        Test.Assert(t.EinaSliceOut(ref slc));
        Test.Assert(t.slice_out_flag);
        Test.Assert(slc.Mem != IntPtr.Zero);
        Test.Assert(slc.Length == base_seq.Length);
        Test.Assert(slc.GetBytes().SequenceEqual(base_seq));
    }

    public static void test_inherit_eina_rw_slice_out()
    {
        var t = new NativeInheritImpl();
        var slc = new eina.Rw_Slice();
        Test.Assert(t.EinaRwSliceOut(ref slc));
        Test.Assert(t.rw_slice_out_flag);
        Test.Assert(slc.Mem != IntPtr.Zero);
        Test.Assert(slc.Length == base_seq.Length);
        Test.Assert(slc.GetBytes().SequenceEqual(base_seq));
    }
}

class TestEinaArray
{
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
        Test.Assert(a.Push("test string §éΨبÿツ"));
        Test.AssertEquals("test string §éΨبÿツ", a[0]);
    }

    public static void push_obj()
    {
        var a = new eina.Array<test.INumberwrapper>();
        Test.Assert(a.Handle != IntPtr.Zero);
        var o = new test.Numberwrapper();
        o.SetNumber(88);
        Test.Assert(a.Push(o));
        Test.Assert(a[0].raw_handle == o.raw_handle);
        Test.Assert(a[0].GetNumber() == 88);
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
        Test.Assert(a.Push("test string"));
        Test.Assert(a.Pop() == "test string");
        Test.Assert(a.Count() == 0);
    }

    public static void pop_obj()
    {
        var a = new eina.Array<test.INumberwrapper>();
        Test.Assert(a.Handle != IntPtr.Zero);
        var o = new test.Numberwrapper();
        o.SetNumber(88);
        Test.Assert(a.Push(o));
        var p = a.Pop();
        Test.Assert(p.raw_handle == o.raw_handle);
        Test.Assert(p.GetNumber() == 88);
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
        Test.Assert(a.Push("test string"));
        Test.Assert(a[0] == "test string");
        a.DataSet(0, "other string");
        Test.Assert(a[0] == "other string");
        a[0] = "abc";
        Test.Assert(a[0] == "abc");
    }

    public static void data_set_obj()
    {
        var a = new eina.Array<test.INumberwrapper>();
        Test.Assert(a.Handle != IntPtr.Zero);

        var o1 = new test.Numberwrapper();
        o1.SetNumber(88);

        Test.Assert(a.Push(o1));
        Test.Assert(a[0].raw_handle == o1.raw_handle);
        Test.Assert(a[0].GetNumber() == 88);

        var o2 = new test.Numberwrapper();
        o2.SetNumber(44);

        a.DataSet(0, o2);
        Test.Assert(a[0].raw_handle == o2.raw_handle);
        Test.Assert(a[0].GetNumber() == 44);

        var o3 = new test.Numberwrapper();
        o3.SetNumber(22);

        a[0] = o3;
        Test.Assert(a[0].raw_handle == o3.raw_handle);
        Test.Assert(a[0].GetNumber() == 22);
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
        var a = new eina.Array<test.INumberwrapper>();
        Test.Assert(a.Handle != IntPtr.Zero);

        Test.Assert(a.Count() == 0);

        var o1 = new test.Numberwrapper();
        o1.SetNumber(88);
        Test.Assert(a.Push(o1));
        Test.Assert(a[0].raw_handle == o1.raw_handle);
        Test.Assert(a[0].GetNumber() == 88);
        Test.Assert(a.Count() == 1);

        var o2 = new test.Numberwrapper();
        o2.SetNumber(44);
        Test.Assert(a.Push(o2));
        Test.Assert(a[1].raw_handle == o2.raw_handle);
        Test.Assert(a[1].GetNumber() == 44);
        Test.Assert(a.Count() == 2);

        var o3 = new test.Numberwrapper();
        o3.SetNumber(22);
        Test.Assert(a.Push(o3));
        Test.Assert(a[2].raw_handle == o3.raw_handle);
        Test.Assert(a[2].GetNumber() == 22);
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
        var a = new test.Numberwrapper();
        var b = new test.Numberwrapper();
        var c = new test.Numberwrapper();
        a.SetNumber(88);
        b.SetNumber(44);
        c.SetNumber(22);
        var cmp = new test.Numberwrapper[]{a,b,c};

        var arr = new eina.Array<test.INumberwrapper>();
        Test.Assert(arr.Handle != IntPtr.Zero);
        Test.Assert(arr.Push(a));
        Test.Assert(arr.Push(b));
        Test.Assert(arr.Push(c));

        int i = 0;
        foreach (test.Numberwrapper e in arr)
        {
            Test.AssertEquals(cmp[i].GetNumber(), e.GetNumber());
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
        test.ITesting t = new test.Testing();
        var arr = new eina.Array<int>();
        arr.Append(base_seq_int);
        Test.Assert(t.EinaArrayIntIn(arr));
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(modified_seq_int));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_array_int_in_own()
    {
        test.ITesting t = new test.Testing();
        var arr = new eina.Array<int>();
        arr.Append(base_seq_int);
        Test.Assert(t.EinaArrayIntInOwn(arr));
        Test.Assert(!arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(modified_seq_int));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaArrayIntInOwn());
    }

    public static void test_eina_array_int_out()
    {
        test.ITesting t = new test.Testing();
        eina.Array<int> arr;
        Test.Assert(t.EinaArrayIntOut(out arr));
        Test.Assert(!arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_int));
        Test.Assert(arr.Append(append_seq_int));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaArrayIntOut());
    }

    public static void test_eina_array_int_out_own()
    {
        test.ITesting t = new test.Testing();
        eina.Array<int> arr;
        Test.Assert(t.EinaArrayIntOutOwn(out arr));
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_int));
        Test.Assert(arr.Append(append_seq_int));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_array_int_return()
    {
        test.ITesting t = new test.Testing();
        var arr = t.EinaArrayIntReturn();
        Test.Assert(!arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_int));
        Test.Assert(arr.Append(append_seq_int));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaArrayIntReturn());
    }

    public static void test_eina_array_int_return_own()
    {
        test.ITesting t = new test.Testing();
        var arr = t.EinaArrayIntReturnOwn();
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_int));
        Test.Assert(arr.Append(append_seq_int));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    // String //
    public static void test_eina_array_str_in()
    {
        test.ITesting t = new test.Testing();
        var arr = new eina.Array<string>();
        arr.Append(base_seq_str);
        Test.Assert(t.EinaArrayStrIn(arr));
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(modified_seq_str));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_array_str_in_own()
    {
        test.ITesting t = new test.Testing();
        var arr = new eina.Array<string>();
        arr.Append(base_seq_str);
        Test.Assert(t.EinaArrayStrInOwn(arr));
        Test.Assert(!arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(modified_seq_str));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaArrayStrInOwn());
    }

    public static void test_eina_array_str_out()
    {
        test.ITesting t = new test.Testing();
        eina.Array<string> arr;
        Test.Assert(t.EinaArrayStrOut(out arr));
        Test.Assert(!arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_str));
        Test.Assert(arr.Append(append_seq_str));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaArrayStrOut());
    }

    public static void test_eina_array_str_out_own()
    {
        test.ITesting t = new test.Testing();
        eina.Array<string> arr;
        Test.Assert(t.EinaArrayStrOutOwn(out arr));
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_str));
        Test.Assert(arr.Append(append_seq_str));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_array_str_return()
    {
        test.ITesting t = new test.Testing();
        var arr = t.EinaArrayStrReturn();
        Test.Assert(!arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_str));
        Test.Assert(arr.Append(append_seq_str));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaArrayStrReturn());
    }

    public static void test_eina_array_str_return_own()
    {
        test.ITesting t = new test.Testing();
        var arr = t.EinaArrayStrReturnOwn();
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_str));
        Test.Assert(arr.Append(append_seq_str));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    // Object //

    public static void test_eina_array_obj_in()
    {
        test.ITesting t = new test.Testing();
        var arr = new eina.Array<test.INumberwrapper>();
        arr.Append(BaseSeqObj());
        Test.Assert(t.EinaArrayObjIn(arr));
        Test.Assert(arr.Own);
        NumberwrapperSequenceAssertEqual(arr.ToArray(), ModifiedSeqObj());
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_array_obj_in_own()
    {
        test.ITesting t = new test.Testing();
        var arr = new eina.Array<test.INumberwrapper>();
        arr.Append(BaseSeqObj());
        Test.Assert(t.EinaArrayObjInOwn(arr));
        Test.Assert(!arr.Own);
        NumberwrapperSequenceAssertEqual(arr.ToArray(), ModifiedSeqObj());
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaArrayObjInOwn());
    }

    public static void test_eina_array_obj_out()
    {
        test.ITesting t = new test.Testing();
        eina.Array<test.INumberwrapper> arr;
        Test.Assert(t.EinaArrayObjOut(out arr));
        Test.Assert(!arr.Own);
        NumberwrapperSequenceAssertEqual(arr.ToArray(), BaseSeqObj());
        Test.Assert(arr.Append(AppendSeqObj()));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaArrayObjOut());
    }

    public static void test_eina_array_obj_out_own()
    {
        test.ITesting t = new test.Testing();
        eina.Array<test.INumberwrapper> arr;
        Test.Assert(t.EinaArrayObjOutOwn(out arr));
        Test.Assert(arr.Own);
        NumberwrapperSequenceAssertEqual(arr.ToArray(), BaseSeqObj());
        Test.Assert(arr.Append(AppendSeqObj()));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_array_obj_return()
    {
        test.ITesting t = new test.Testing();
        var arr = t.EinaArrayObjReturn();
        Test.Assert(!arr.Own);
        NumberwrapperSequenceAssertEqual(arr.ToArray(), BaseSeqObj());
        Test.Assert(arr.Append(AppendSeqObj()));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaArrayObjReturn());
    }

    public static void test_eina_array_obj_return_own()
    {
        test.ITesting t = new test.Testing();
        var arr = t.EinaArrayObjReturnOwn();
        Test.Assert(arr.Own);
        NumberwrapperSequenceAssertEqual(arr.ToArray(), BaseSeqObj());
        Test.Assert(arr.Append(AppendSeqObj()));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_array_obj_return_in_same_id()
    {
        test.ITesting t = new test.Testing();
        var cmp = BaseSeqObj();
        var a = new eina.Array<test.INumberwrapper>();
        a.Append(cmp);
        var b = t.EinaArrayObjReturnIn(a);
        NumberwrapperSequenceAssertEqual(a.ToArray(), b.ToArray());
        NumberwrapperSequenceAssertEqual(a.ToArray(), BaseSeqObj());
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

class TestEinaInarray
{
    public static void eina_inarray_default()
    {
        var a = new eina.Inarray<int>();
        Test.Assert(a.Handle != IntPtr.Zero);
        a.Dispose();
    }

    public static void push_int()
    {
        var a = new eina.Inarray<int>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push(88) == 0);
        Test.Assert(a[0] == 88);
        a.Dispose();
    }

    public static void push_string()
    {
        var a = new eina.Inarray<string>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push("test string") == 0);
        Test.Assert(a[0] == "test string");
        a.Dispose();
    }

    public static void push_obj()
    {
        var a = new eina.Inarray<test.INumberwrapper>();
        Test.Assert(a.Handle != IntPtr.Zero);
        var o = new test.Numberwrapper();
        o.SetNumber(88);
        Test.Assert(a.Push(o) == 0);
        Test.Assert(a[0].raw_handle == o.raw_handle);
        Test.Assert(a[0].GetNumber() == 88);
        a.Dispose();
    }

    public static void pop_int()
    {
        var a = new eina.Inarray<int>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push(88) >= 0);
        Test.Assert(a.Pop() == 88);
        Test.Assert(a.Count() == 0);
        a.Dispose();
    }

    public static void pop_string()
    {
        var a = new eina.Inarray<string>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push("test string") >= 0);
        Test.Assert(a.Pop() == "test string");
        Test.Assert(a.Count() == 0);
        a.Dispose();
    }

    public static void pop_obj()
    {
        var a = new eina.Inarray<test.INumberwrapper>();
        Test.Assert(a.Handle != IntPtr.Zero);
        var o = new test.Numberwrapper();
        o.SetNumber(88);
        Test.Assert(a.Push(o) >= 0);
        var p = a.Pop();
        Test.Assert(p.raw_handle == o.raw_handle);
        Test.Assert(p.GetNumber() == 88);
        Test.Assert(a.Count() == 0);
        a.Dispose();
    }

    public static void replace_at_int()
    {
        var a = new eina.Inarray<int>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push(88) >= 0);
        Test.Assert(a[0] == 88);
        a.ReplaceAt(0, 44);
        Test.Assert(a[0] == 44);
        Test.Assert(a.Count() == 1);
        a[0] = 22;
        Test.Assert(a[0] == 22);
        Test.Assert(a.Count() == 1);
        a.Dispose();
    }

    public static void replace_at_string()
    {
        var a = new eina.Inarray<string>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push("test string") >= 0);
        Test.Assert(a[0] == "test string");
        a.ReplaceAt(0, "other string");
        Test.Assert(a[0] == "other string");
        Test.Assert(a.Count() == 1);
        a[0] = "abc";
        Test.Assert(a[0] == "abc");
        Test.Assert(a.Count() == 1);
        a.Dispose();
    }

    public static void replace_at_obj()
    {
        var a = new eina.Inarray<test.INumberwrapper>();
        Test.Assert(a.Handle != IntPtr.Zero);

        var o1 = new test.Numberwrapper();
        o1.SetNumber(88);

        Test.Assert(a.Push(o1) >= 0);
        Test.Assert(a[0].raw_handle == o1.raw_handle);
        Test.Assert(a[0].GetNumber() == 88);

        var o2 = new test.Numberwrapper();
        o2.SetNumber(44);

        a.ReplaceAt(0, o2);
        Test.Assert(a[0].raw_handle == o2.raw_handle);
        Test.Assert(a[0].GetNumber() == 44);
        Test.Assert(a.Count() == 1);

        var o3 = new test.Numberwrapper();
        o3.SetNumber(22);

        a[0] = o3;
        Test.Assert(a[0].raw_handle == o3.raw_handle);
        Test.Assert(a[0].GetNumber() == 22);
        Test.Assert(a.Count() == 1);

        a.Dispose();
    }

    public static void count_int()
    {
        var a = new eina.Inarray<int>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Count() == 0);
        Test.Assert(a.Push(88) == 0);
        Test.Assert(a[0] == 88);
        Test.Assert(a.Count() == 1);
        Test.Assert(a.Push(44) == 1);
        Test.Assert(a[1] == 44);
        Test.Assert(a.Count() == 2);
        Test.Assert(a.Push(22) == 2);
        Test.Assert(a[2] == 22);
        Test.Assert(a.Count() == 3);
        a.Dispose();
    }

    public static void count_string()
    {
        var a = new eina.Inarray<string>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Count() == 0);
        Test.Assert(a.Push("a") == 0);
        Test.Assert(a[0] == "a");
        Test.Assert(a.Count() == 1);
        Test.Assert(a.Push("b") == 1);
        Test.Assert(a[1] == "b");
        Test.Assert(a.Count() == 2);
        Test.Assert(a.Push("c") == 2);
        Test.Assert(a[2] == "c");
        Test.Assert(a.Count() == 3);
        a.Dispose();
    }

    public static void count_obj()
    {
        var a = new eina.Inarray<test.INumberwrapper>();
        Test.Assert(a.Handle != IntPtr.Zero);

        Test.Assert(a.Count() == 0);

        var o1 = new test.Numberwrapper();
        o1.SetNumber(88);
        Test.Assert(a.Push(o1) == 0);
        Test.Assert(a[0].raw_handle == o1.raw_handle);
        Test.Assert(a[0].GetNumber() == 88);
        Test.Assert(a.Count() == 1);

        var o2 = new test.Numberwrapper();
        o2.SetNumber(44);
        Test.Assert(a.Push(o2) == 1);
        Test.Assert(a[1].raw_handle == o2.raw_handle);
        Test.Assert(a[1].GetNumber() == 44);
        Test.Assert(a.Count() == 2);

        var o3 = new test.Numberwrapper();
        o3.SetNumber(22);
        Test.Assert(a.Push(o3) == 2);
        Test.Assert(a[2].raw_handle == o3.raw_handle);
        Test.Assert(a[2].GetNumber() == 22);
        Test.Assert(a.Count() == 3);

        a.Dispose();
    }

    public static void length_int()
    {
        var a = new eina.Inarray<int>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Length == 0);
        Test.Assert(a.Push(88) >= 0);
        Test.Assert(a[0] == 88);
        Test.Assert(a.Length == 1);
        Test.Assert(a.Push(44) >= 0);
        Test.Assert(a[1] == 44);
        Test.Assert(a.Length == 2);
        Test.Assert(a.Push(22) >= 0);
        Test.Assert(a[2] == 22);
        Test.Assert(a.Length == 3);
        a.Dispose();
    }

    public static void length_string()
    {
        var a = new eina.Inarray<string>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Length == 0);
        Test.Assert(a.Push("a") >= 0);
        Test.Assert(a[0] == "a");
        Test.Assert(a.Length == 1);
        Test.Assert(a.Push("b") >= 0);
        Test.Assert(a[1] == "b");
        Test.Assert(a.Length == 2);
        Test.Assert(a.Push("c") >= 0);
        Test.Assert(a[2] == "c");
        Test.Assert(a.Length == 3);
        a.Dispose();
    }

    public static void eina_inarray_as_ienumerable_int()
    {
        var a = new eina.Inarray<int>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push(88) == 0);
        Test.Assert(a.Push(44) == 1);
        Test.Assert(a.Push(22) == 2);

        int cmp = 88;
        foreach (int e in a)
        {
            Test.AssertEquals(cmp, e);
            cmp /= 2;
        }
        a.Dispose();
    }

    public static void eina_inarray_as_ienumerable_string()
    {
        var a = new eina.Inarray<string>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push("X") == 0);
        Test.Assert(a.Push("XX") == 1);
        Test.Assert(a.Push("XXX") == 2);

        string cmp = "X";
        foreach (string e in a)
        {
            Test.AssertEquals(cmp, e);
            cmp = cmp + "X";
        }
        a.Dispose();
    }

    public static void eina_inarray_as_ienumerable_obj()
    {
        var a = new test.Numberwrapper();
        var b = new test.Numberwrapper();
        var c = new test.Numberwrapper();
        a.SetNumber(88);
        b.SetNumber(44);
        c.SetNumber(22);
        var cmp = new test.Numberwrapper[]{a,b,c};

        var arr = new eina.Inarray<test.INumberwrapper>();
        Test.Assert(arr.Handle != IntPtr.Zero);
        Test.Assert(arr.Push(a) == 0);
        Test.Assert(arr.Push(b) == 1);
        Test.Assert(arr.Push(c) == 2);

        int i = 0;
        foreach (test.Numberwrapper e in arr)
        {
            Test.AssertEquals(cmp[i].GetNumber(), e.GetNumber());
            Test.Assert(cmp[i].raw_handle == e.raw_handle);
            ++i;
        }
        arr.Dispose();
    }

    // //
    // Code Generation
    //

    // Integer //

    public static void test_eina_inarray_int_in()
    {
        test.ITesting t = new test.Testing();
        var arr = new eina.Inarray<int>();
        arr.Append(base_seq_int);
        Test.Assert(t.EinaInarrayIntIn(arr));
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(modified_seq_int));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_inarray_int_in_own()
    {
        test.ITesting t = new test.Testing();
        var arr = new eina.Inarray<int>();
        arr.Append(base_seq_int);
        Test.Assert(t.EinaInarrayIntInOwn(arr));
        Test.Assert(!arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(modified_seq_int));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaInarrayIntInOwn());
    }

    public static void test_eina_inarray_int_out()
    {
        test.ITesting t = new test.Testing();
        eina.Inarray<int> arr;
        Test.Assert(t.EinaInarrayIntOut(out arr));
        Test.Assert(!arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_int));
        Test.Assert(arr.Append(append_seq_int));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaInarrayIntOut());
    }

    public static void test_eina_inarray_int_out_own()
    {
        test.ITesting t = new test.Testing();
        eina.Inarray<int> arr;
        Test.Assert(t.EinaInarrayIntOutOwn(out arr));
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_int));
        Test.Assert(arr.Append(append_seq_int));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_inarray_int_return()
    {
        test.ITesting t = new test.Testing();
        var arr = t.EinaInarrayIntReturn();
        Test.Assert(!arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_int));
        Test.Assert(arr.Append(append_seq_int));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaInarrayIntReturn());
    }

    public static void test_eina_inarray_int_return_own()
    {
        test.ITesting t = new test.Testing();
        var arr = t.EinaInarrayIntReturnOwn();
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_int));
        Test.Assert(arr.Append(append_seq_int));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    // String //
    public static void test_eina_inarray_str_in()
    {
        test.ITesting t = new test.Testing();
        var arr = new eina.Inarray<string>();
        arr.Append(base_seq_str);
        Test.Assert(t.EinaInarrayStrIn(arr));
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(modified_seq_str));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_inarray_str_in_own()
    {
        test.ITesting t = new test.Testing();
        var arr = new eina.Inarray<string>();
        arr.Append(base_seq_str);
        Test.Assert(t.EinaInarrayStrInOwn(arr));
        Test.Assert(!arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(modified_seq_str));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaInarrayStrInOwn());
    }

    public static void test_eina_inarray_str_out()
    {
        test.ITesting t = new test.Testing();
        eina.Inarray<string> arr;
        Test.Assert(t.EinaInarrayStrOut(out arr));
        Test.Assert(!arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_str));
        Test.Assert(arr.Append(append_seq_str));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaInarrayStrOut());
    }

    public static void test_eina_inarray_str_out_own()
    {
        test.ITesting t = new test.Testing();
        eina.Inarray<string> arr;
        Test.Assert(t.EinaInarrayStrOutOwn(out arr));
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_str));
        Test.Assert(arr.Append(append_seq_str));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_inarray_str_return()
    {
        test.ITesting t = new test.Testing();
        var arr = t.EinaInarrayStrReturn();
        Test.Assert(!arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_str));
        Test.Assert(arr.Append(append_seq_str));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaInarrayStrReturn());
    }

    public static void test_eina_inarray_str_return_own()
    {
        test.ITesting t = new test.Testing();
        var arr = t.EinaInarrayStrReturnOwn();
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_str));
        Test.Assert(arr.Append(append_seq_str));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    // Object //

    public static void test_eina_inarray_obj_in()
    {
        test.ITesting t = new test.Testing();
        var arr = new eina.Inarray<test.INumberwrapper>();
        arr.Append(BaseSeqObj());
        Test.Assert(t.EinaInarrayObjIn(arr));
        Test.Assert(arr.Own);
        NumberwrapperSequenceAssertEqual(arr.ToArray(), ModifiedSeqObj());
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_inarray_obj_in_own()
    {
        test.ITesting t = new test.Testing();
        var arr = new eina.Inarray<test.INumberwrapper>();
        arr.Append(BaseSeqObj());
        Test.Assert(t.EinaInarrayObjInOwn(arr));
        Test.Assert(!arr.Own);
        NumberwrapperSequenceAssertEqual(arr.ToArray(), ModifiedSeqObj());
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaInarrayObjInOwn());
    }

    public static void test_eina_inarray_obj_out()
    {
        test.ITesting t = new test.Testing();
        eina.Inarray<test.INumberwrapper> arr;
        Test.Assert(t.EinaInarrayObjOut(out arr));
        Test.Assert(!arr.Own);
        NumberwrapperSequenceAssertEqual(arr.ToArray(), BaseSeqObj());
        Test.Assert(arr.Append(AppendSeqObj()));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaInarrayObjOut());
    }

    public static void test_eina_inarray_obj_out_own()
    {
        test.ITesting t = new test.Testing();
        eina.Inarray<test.INumberwrapper> arr;
        Test.Assert(t.EinaInarrayObjOutOwn(out arr));
        Test.Assert(arr.Own);
        NumberwrapperSequenceAssertEqual(arr.ToArray(), BaseSeqObj());
        Test.Assert(arr.Append(AppendSeqObj()));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_inarray_obj_return()
    {
        test.ITesting t = new test.Testing();
        var arr = t.EinaInarrayObjReturn();
        Test.Assert(!arr.Own);
        NumberwrapperSequenceAssertEqual(arr.ToArray(), BaseSeqObj());
        Test.Assert(arr.Append(AppendSeqObj()));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaInarrayObjReturn());
    }

    public static void test_eina_inarray_obj_return_own()
    {
        test.ITesting t = new test.Testing();
        var arr = t.EinaInarrayObjReturnOwn();
        Test.Assert(arr.Own);
        NumberwrapperSequenceAssertEqual(arr.ToArray(), BaseSeqObj());
        Test.Assert(arr.Append(AppendSeqObj()));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_inarray_obj_return_in_same_id()
    {
        test.ITesting t = new test.Testing();
        var cmp = BaseSeqObj();
        var a = new eina.Inarray<test.INumberwrapper>();
        a.Append(cmp);
        var b = t.EinaInarrayObjReturnIn(a);
        NumberwrapperSequenceAssertEqual(a.ToArray(), b.ToArray());
        NumberwrapperSequenceAssertEqual(a.ToArray(), BaseSeqObj());
        int len = a.Length;
        for (int i=0; i < len; ++i)
        {
            Test.Assert(a[i].raw_handle == b[i].raw_handle);
            Test.Assert(a[i].raw_handle == cmp[i].raw_handle);
        }
        a.Dispose();
    }
}

class TestEinaList
{
    public static void data_set_int()
    {
        var lst = new eina.List<int>();
        lst.Append(88);
        Test.Assert(lst[0] == 88);
        lst.DataSet(0, 44);
        Test.Assert(lst[0] == 44);
        lst[0] = 22;
        Test.Assert(lst[0] == 22);
    }

    public static void data_set_string()
    {
        var lst = new eina.List<string>();
        lst.Append("test string");
        Test.Assert(lst[0] == "test string");
        lst.DataSet(0, "other string");
        Test.Assert(lst[0] == "other string");
        lst[0] = "abc";
        Test.Assert(lst[0] == "abc");
    }

    public static void data_set_obj()
    {
        var lst = new eina.List<test.INumberwrapper>();

        var o1 = new test.Numberwrapper();
        o1.SetNumber(88);

        lst.Append(o1);
        Test.Assert(lst[0].raw_handle == o1.raw_handle);
        Test.Assert(lst[0].GetNumber() == 88);

        var o2 = new test.Numberwrapper();
        o2.SetNumber(44);

        lst.DataSet(0, o2);
        Test.Assert(lst[0].raw_handle == o2.raw_handle);
        Test.Assert(lst[0].GetNumber() == 44);

        var o3 = new test.Numberwrapper();
        o3.SetNumber(22);

        lst[0] = o3;
        Test.Assert(lst[0].raw_handle == o3.raw_handle);
        Test.Assert(lst[0].GetNumber() == 22);
    }

    public static void append_count_int()
    {
        var lst = new eina.List<int>();
        Test.Assert(lst.Count() == 0);
        lst.Append(88);
        Test.Assert(lst[0] == 88);
        Test.Assert(lst.Count() == 1);
        lst.Append(44);
        Test.Assert(lst[1] == 44);
        Test.Assert(lst.Count() == 2);
        lst.Append(22);
        Test.Assert(lst[2] == 22);
        Test.Assert(lst.Count() == 3);
    }

    public static void append_count_string()
    {
        var lst = new eina.List<string>();
        Test.Assert(lst.Count() == 0);
        lst.Append("a");
        Test.Assert(lst[0] == "a");
        Test.Assert(lst.Count() == 1);
        lst.Append("b");
        Test.Assert(lst[1] == "b");
        Test.Assert(lst.Count() == 2);
        lst.Append("c");
        Test.Assert(lst[2] == "c");
        Test.Assert(lst.Count() == 3);
    }

    public static void append_count_obj()
    {
        var lst = new eina.List<test.INumberwrapper>();

        Test.Assert(lst.Count() == 0);

        var o1 = new test.Numberwrapper();
        o1.SetNumber(88);
        lst.Append(o1);
        Test.Assert(lst[0].raw_handle == o1.raw_handle);
        Test.Assert(lst[0].GetNumber() == 88);
        Test.Assert(lst.Count() == 1);

        var o2 = new test.Numberwrapper();
        o2.SetNumber(44);
        lst.Append(o2);
        Test.Assert(lst[1].raw_handle == o2.raw_handle);
        Test.Assert(lst[1].GetNumber() == 44);
        Test.Assert(lst.Count() == 2);

        var o3 = new test.Numberwrapper();
        o3.SetNumber(22);
        lst.Append(o3);
        Test.Assert(lst[2].raw_handle == o3.raw_handle);
        Test.Assert(lst[2].GetNumber() == 22);
        Test.Assert(lst.Count() == 3);
    }

    public static void length_int()
    {
        var lst = new eina.List<int>();
        Test.Assert(lst.Length == 0);
        lst.Append(88);
        Test.Assert(lst[0] == 88);
        Test.Assert(lst.Length == 1);
        lst.Append(44);
        Test.Assert(lst[1] == 44);
        Test.Assert(lst.Length == 2);
        lst.Append(22);
        Test.Assert(lst[2] == 22);
        Test.Assert(lst.Length == 3);
    }

    public static void length_string()
    {
        var lst = new eina.List<string>();
        Test.Assert(lst.Length == 0);
        lst.Append("a");
        Test.Assert(lst[0] == "a");
        Test.Assert(lst.Length == 1);
        lst.Append("b");
        Test.Assert(lst[1] == "b");
        Test.Assert(lst.Length == 2);
        lst.Append("c");
        Test.Assert(lst[2] == "c");
        Test.Assert(lst.Length == 3);
    }

    public static void prepend_count_int()
    {
        var lst = new eina.List<int>();
        Test.Assert(lst.Count() == 0);
        lst.Prepend(88);
        Test.Assert(lst[0] == 88);
        Test.Assert(lst.Count() == 1);
        lst.Prepend(44);
        Test.Assert(lst[0] == 44);
        Test.Assert(lst.Count() == 2);
        lst.Prepend(22);
        Test.Assert(lst[0] == 22);
        Test.Assert(lst.Count() == 3);
    }

    public static void prepend_count_string()
    {
        var lst = new eina.List<string>();
        Test.Assert(lst.Count() == 0);
        lst.Prepend("a");
        Test.Assert(lst[0] == "a");
        Test.Assert(lst.Count() == 1);
        lst.Prepend("b");
        Test.Assert(lst[0] == "b");
        Test.Assert(lst.Count() == 2);
        lst.Prepend("c");
        Test.Assert(lst[0] == "c");
        Test.Assert(lst.Count() == 3);
    }

    public static void prepend_count_obj()
    {
        var lst = new eina.List<test.INumberwrapper>();

        Test.Assert(lst.Count() == 0);

        var o1 = new test.Numberwrapper();
        o1.SetNumber(88);
        lst.Prepend(o1);
        Test.Assert(lst[0].raw_handle == o1.raw_handle);
        Test.Assert(lst[0].GetNumber() == 88);
        Test.Assert(lst.Count() == 1);

        var o2 = new test.Numberwrapper();
        o2.SetNumber(44);
        lst.Prepend(o2);
        Test.Assert(lst[0].raw_handle == o2.raw_handle);
        Test.Assert(lst[0].GetNumber() == 44);
        Test.Assert(lst.Count() == 2);

        var o3 = new test.Numberwrapper();
        o3.SetNumber(22);
        lst.Prepend(o3);
        Test.Assert(lst[0].raw_handle == o3.raw_handle);
        Test.Assert(lst[0].GetNumber() == 22);
        Test.Assert(lst.Count() == 3);
    }

    public static void sorted_insert_int()
    {
        var lst = new eina.List<int>();
        lst.SortedInsert(88);
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{88}));
        lst.SortedInsert(22);
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{22,88}));
        lst.SortedInsert(44);
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{22,44,88}));

    }

    public static void sorted_insert_string()
    {
        var lst = new eina.List<string>();
        lst.SortedInsert("c");
        Test.Assert(lst.ToArray().SequenceEqual(new string[]{"c"}));
        lst.SortedInsert("a");
        Test.Assert(lst.ToArray().SequenceEqual(new string[]{"a","c"}));
        lst.SortedInsert("b");
        Test.Assert(lst.ToArray().SequenceEqual(new string[]{"a","b","c"}));

    }

    public static void sort_int()
    {
        var lst = new eina.List<int>();
        lst.Append(88);
        lst.Append(22);
        lst.Append(11);
        lst.Append(44);
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{88,22,11,44}));
        lst.Sort();
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{11,22,44,88}));

    }

    public static void sort_string()
    {
        var lst = new eina.List<string>();
        lst.Append("d");
        lst.Append("b");
        lst.Append("a");
        lst.Append("c");
        Test.Assert(lst.ToArray().SequenceEqual(new string[]{"d","b","a","c"}));
        lst.Sort();
        Test.Assert(lst.ToArray().SequenceEqual(new string[]{"a","b","c","d"}));
    }

    public static void reverse_int()
    {
        var lst = new eina.List<int>();
        lst.Append(22);
        lst.Append(44);
        lst.Append(88);
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{22,44,88}));
        lst.Reverse();
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{88,44,22}));

    }

    public static void reverse_string()
    {
        var lst = new eina.List<string>();
        lst.Append("a");
        lst.Append("b");
        lst.Append("c");
        Test.Assert(lst.ToArray().SequenceEqual(new string[]{"a","b","c"}));
        lst.Reverse();
        Test.Assert(lst.ToArray().SequenceEqual(new string[]{"c","b","a"}));
    }

    public static void eina_list_as_ienumerable_int()
    {
        var lst = new eina.List<int>();
        lst.Append(88);
        lst.Append(44);
        lst.Append(22);

        int cmp = 88;
        foreach (int e in lst)
        {
            Test.AssertEquals(cmp, e);
            cmp /= 2;
        }
    }

    public static void eina_list_as_ienumerable_string()
    {
        var lst = new eina.List<string>();
        lst.Append("X");
        lst.Append("XX");
        lst.Append("XXX");

        string cmp = "X";
        foreach (string e in lst)
        {
            Test.AssertEquals(cmp, e);
            cmp = cmp + "X";
        }
    }

    public static void eina_list_as_ienumerable_obj()
    {
        var a = new test.Numberwrapper();
        var b = new test.Numberwrapper();
        var c = new test.Numberwrapper();
        a.SetNumber(88);
        b.SetNumber(44);
        c.SetNumber(22);
        var cmp = new test.Numberwrapper[]{a,b,c};

        var lst = new eina.List<test.INumberwrapper>();
        lst.Append(a);
        lst.Append(b);
        lst.Append(c);

        int i = 0;
        foreach (test.Numberwrapper e in lst)
        {
            Test.AssertEquals(cmp[i].GetNumber(), e.GetNumber());
            Test.Assert(cmp[i].raw_handle == e.raw_handle);
            ++i;
        }
    }

    // //
    // Code Generation
    //

    // Integer //

    public static void test_eina_list_int_in()
    {
        test.ITesting t = new test.Testing();
        var lst = new eina.List<int>();
        lst.AppendArray(base_seq_int);
        Test.Assert(t.EinaListIntIn(lst));
        Test.Assert(lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_int));
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    public static void test_eina_list_int_in_own()
    {
        test.ITesting t = new test.Testing();
        var lst = new eina.List<int>();
        lst.AppendArray(base_seq_int);
        Test.Assert(t.EinaListIntInOwn(lst));
        Test.Assert(!lst.Own);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaListIntInOwn());
    }

    public static void test_eina_list_int_out()
    {
        test.ITesting t = new test.Testing();
        eina.List<int> lst;
        Test.Assert(t.EinaListIntOut(out lst));
        Test.Assert(!lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_int));
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaListIntOut());
    }

    public static void test_eina_list_int_out_own()
    {
        test.ITesting t = new test.Testing();
        eina.List<int> lst;
        Test.Assert(t.EinaListIntOutOwn(out lst));
        Test.Assert(lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_int));
        lst.AppendArray(append_seq_int);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    public static void test_eina_list_int_return()
    {
        test.ITesting t = new test.Testing();
        var lst = t.EinaListIntReturn();
        Test.Assert(!lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_int));
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaListIntReturn());
    }

    public static void test_eina_list_int_return_own()
    {
        test.ITesting t = new test.Testing();
        var lst = t.EinaListIntReturnOwn();
        Test.Assert(lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_int));
        lst.AppendArray(append_seq_int);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    // String //
    public static void test_eina_list_str_in()
    {
        test.ITesting t = new test.Testing();
        var lst = new eina.List<string>();
        lst.AppendArray(base_seq_str);
        Test.Assert(t.EinaListStrIn(lst));
        Test.Assert(lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_str));
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    public static void test_eina_list_str_in_own()
    {
        test.ITesting t = new test.Testing();
        var lst = new eina.List<string>();
        lst.AppendArray(base_seq_str);
        Test.Assert(t.EinaListStrInOwn(lst));
        Test.Assert(!lst.Own);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaListStrInOwn());
    }

    public static void test_eina_list_str_out()
    {
        test.ITesting t = new test.Testing();
        eina.List<string> lst;
        Test.Assert(t.EinaListStrOut(out lst));
        Test.Assert(!lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_str));
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaListStrOut());
    }

    public static void test_eina_list_str_out_own()
    {
        test.ITesting t = new test.Testing();
        eina.List<string> lst;
        Test.Assert(t.EinaListStrOutOwn(out lst));
        Test.Assert(lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_str));
        lst.AppendArray(append_seq_str);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    public static void test_eina_list_str_return()
    {
        test.ITesting t = new test.Testing();
        var lst = t.EinaListStrReturn();
        Test.Assert(!lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_str));
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaListStrReturn());
    }

    public static void test_eina_list_str_return_own()
    {
        test.ITesting t = new test.Testing();
        var lst = t.EinaListStrReturnOwn();
        Test.Assert(lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_str));
        lst.AppendArray(append_seq_str);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    // Object //

    public static void test_eina_list_obj_in()
    {
        test.ITesting t = new test.Testing();
        var lst = new eina.List<test.INumberwrapper>();
        lst.AppendArray(BaseSeqObj());
        Test.Assert(t.EinaListObjIn(lst));
        Test.Assert(lst.Own);
        NumberwrapperSequenceAssertEqual(lst.ToArray(), BaseSeqObj());
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    public static void test_eina_list_obj_in_own()
    {
        test.ITesting t = new test.Testing();
        var lst = new eina.List<test.INumberwrapper>();
        lst.AppendArray(BaseSeqObj());
        Test.Assert(t.EinaListObjInOwn(lst));
        Test.Assert(!lst.Own);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaListObjInOwn());
    }

    public static void test_eina_list_obj_out()
    {
        test.ITesting t = new test.Testing();
        eina.List<test.INumberwrapper> lst;
        Test.Assert(t.EinaListObjOut(out lst));
        Test.Assert(!lst.Own);
        NumberwrapperSequenceAssertEqual(lst.ToArray(), BaseSeqObj());
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaListObjOut());
    }

    public static void test_eina_list_obj_out_own()
    {
        test.ITesting t = new test.Testing();
        eina.List<test.INumberwrapper> lst;
        Test.Assert(t.EinaListObjOutOwn(out lst));
        Test.Assert(lst.Own);
        NumberwrapperSequenceAssertEqual(lst.ToArray(), BaseSeqObj());
        lst.AppendArray(AppendSeqObj());
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    public static void test_eina_list_obj_return()
    {
        test.ITesting t = new test.Testing();
        var lst = t.EinaListObjReturn();
        Test.Assert(!lst.Own);
        NumberwrapperSequenceAssertEqual(lst.ToArray(), BaseSeqObj());
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaListObjReturn());
    }

    public static void test_eina_list_obj_return_own()
    {
        test.ITesting t = new test.Testing();
        var lst = t.EinaListObjReturnOwn();
        Test.Assert(lst.Own);
        NumberwrapperSequenceAssertEqual(lst.ToArray(), BaseSeqObj());
        lst.AppendArray(AppendSeqObj());
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    public static void test_eina_list_obj_return_in_same_id()
    {
        test.ITesting t = new test.Testing();
        var cmp = BaseSeqObj();
        var a = new eina.List<test.INumberwrapper>();
        a.AppendArray(cmp);
        var b = t.EinaListObjReturnIn(a);
        NumberwrapperSequenceAssertEqual(a.ToArray(), b.ToArray());
        NumberwrapperSequenceAssertEqual(a.ToArray(), BaseSeqObj());
        int len = a.Length;
        for (int i=0; i < len; ++i)
        {
            Test.Assert(a[i].raw_handle == b[i].raw_handle);
            Test.Assert(a[i].raw_handle == cmp[i].raw_handle);
        }
    }
}

class TestEinaInlist
{
    public static void data_set_int()
    {
        var lst = new eina.Inlist<int>();
        lst.Append(88);
        Test.Assert(lst[0] == 88);
        lst.DataSet(0, 44);
        Test.Assert(lst[0] == 44);
        lst[0] = 22;
        Test.Assert(lst[0] == 22);
        Test.Assert(lst.Count() == 1);
    }

    public static void data_set_string()
    {
        var lst = new eina.Inlist<string>();
        lst.Append("test string");
        Test.Assert(lst[0] == "test string");
        lst.DataSet(0, "other string");
        Test.Assert(lst[0] == "other string");
        lst[0] = "abc";
        Test.Assert(lst[0] == "abc");
        Test.Assert(lst.Count() == 1);
    }

    public static void data_set_obj()
    {
        var lst = new eina.Inlist<test.INumberwrapper>();

        var o1 = new test.Numberwrapper();
        o1.SetNumber(88);

        lst.Append(o1);
        Test.Assert(lst[0].raw_handle == o1.raw_handle);
        Test.Assert(lst[0].GetNumber() == 88);

        var o2 = new test.Numberwrapper();
        o2.SetNumber(44);

        lst.DataSet(0, o2);
        Test.Assert(lst[0].raw_handle == o2.raw_handle);
        Test.Assert(lst[0].GetNumber() == 44);

        var o3 = new test.Numberwrapper();
        o3.SetNumber(22);

        lst[0] = o3;
        Test.Assert(lst[0].raw_handle == o3.raw_handle);
        Test.Assert(lst[0].GetNumber() == 22);

        Test.Assert(lst.Count() == 1);
    }

    public static void append_count_int()
    {
        var lst = new eina.Inlist<int>();
        Test.Assert(lst.Count() == 0);
        lst.Append(88);
        Test.Assert(lst[0] == 88);
        Test.Assert(lst.Count() == 1);
        lst.Append(44);
        Test.Assert(lst[1] == 44);
        Test.Assert(lst.Count() == 2);
        lst.Append(22);
        Test.Assert(lst[2] == 22);
        Test.Assert(lst.Count() == 3);
    }

    public static void append_count_string()
    {
        var lst = new eina.Inlist<string>();
        Test.Assert(lst.Count() == 0);
        lst.Append("a");
        Test.Assert(lst[0] == "a");
        Test.Assert(lst.Count() == 1);
        lst.Append("b");
        Test.Assert(lst[1] == "b");
        Test.Assert(lst.Count() == 2);
        lst.Append("c");
        Test.Assert(lst[2] == "c");
        Test.Assert(lst.Count() == 3);
    }

    public static void append_count_obj()
    {
        var lst = new eina.Inlist<test.INumberwrapper>();

        Test.Assert(lst.Count() == 0);

        var o1 = new test.Numberwrapper();
        o1.SetNumber(88);
        lst.Append(o1);
        Test.Assert(lst[0].raw_handle == o1.raw_handle);
        Test.Assert(lst[0].GetNumber() == 88);
        Test.Assert(lst.Count() == 1);

        var o2 = new test.Numberwrapper();
        o2.SetNumber(44);
        lst.Append(o2);
        Test.Assert(lst[1].raw_handle == o2.raw_handle);
        Test.Assert(lst[1].GetNumber() == 44);
        Test.Assert(lst.Count() == 2);

        var o3 = new test.Numberwrapper();
        o3.SetNumber(22);
        lst.Append(o3);
        Test.Assert(lst[2].raw_handle == o3.raw_handle);
        Test.Assert(lst[2].GetNumber() == 22);
        Test.Assert(lst.Count() == 3);
    }

    public static void length_int()
    {
        var lst = new eina.Inlist<int>();
        Test.Assert(lst.Length == 0);
        lst.Append(88);
        Test.Assert(lst[0] == 88);
        Test.Assert(lst.Length == 1);
        lst.Append(44);
        Test.Assert(lst[1] == 44);
        Test.Assert(lst.Length == 2);
        lst.Append(22);
        Test.Assert(lst[2] == 22);
        Test.Assert(lst.Length == 3);
    }

    public static void length_string()
    {
        var lst = new eina.Inlist<string>();
        Test.Assert(lst.Length == 0);
        lst.Append("a");
        Test.Assert(lst[0] == "a");
        Test.Assert(lst.Length == 1);
        lst.Append("b");
        Test.Assert(lst[1] == "b");
        Test.Assert(lst.Length == 2);
        lst.Append("c");
        Test.Assert(lst[2] == "c");
        Test.Assert(lst.Length == 3);
    }

    public static void prepend_count_int()
    {
        var lst = new eina.Inlist<int>();
        Test.Assert(lst.Count() == 0);
        lst.Prepend(88);
        Test.Assert(lst[0] == 88);
        Test.Assert(lst.Count() == 1);
        lst.Prepend(44);
        Test.Assert(lst[0] == 44);
        Test.Assert(lst.Count() == 2);
        lst.Prepend(22);
        Test.Assert(lst[0] == 22);
        Test.Assert(lst.Count() == 3);
    }

    public static void prepend_count_string()
    {
        var lst = new eina.Inlist<string>();
        Test.Assert(lst.Count() == 0);
        lst.Prepend("a");
        Test.Assert(lst[0] == "a");
        Test.Assert(lst.Count() == 1);
        lst.Prepend("b");
        Test.Assert(lst[0] == "b");
        Test.Assert(lst.Count() == 2);
        lst.Prepend("c");
        Test.Assert(lst[0] == "c");
        Test.Assert(lst.Count() == 3);
    }

    public static void prepend_count_obj()
    {
        var lst = new eina.Inlist<test.INumberwrapper>();

        Test.Assert(lst.Count() == 0);

        var o1 = new test.Numberwrapper();
        o1.SetNumber(88);
        lst.Prepend(o1);
        Test.Assert(lst[0].raw_handle == o1.raw_handle);
        Test.Assert(lst[0].GetNumber() == 88);
        Test.Assert(lst.Count() == 1);

        var o2 = new test.Numberwrapper();
        o2.SetNumber(44);
        lst.Prepend(o2);
        Test.Assert(lst[0].raw_handle == o2.raw_handle);
        Test.Assert(lst[0].GetNumber() == 44);
        Test.Assert(lst.Count() == 2);

        var o3 = new test.Numberwrapper();
        o3.SetNumber(22);
        lst.Prepend(o3);
        Test.Assert(lst[0].raw_handle == o3.raw_handle);
        Test.Assert(lst[0].GetNumber() == 22);
        Test.Assert(lst.Count() == 3);
    }

    public static void eina_inlist_as_ienumerable_int()
    {
        var lst = new eina.Inlist<int>();
        lst.Append(88);
        lst.Append(44);
        lst.Append(22);

        int cmp = 88;
        foreach (int e in lst)
        {
            Test.AssertEquals(cmp, e);
            cmp /= 2;
        }
    }

    public static void eina_inlist_as_ienumerable_string()
    {
        var lst = new eina.Inlist<string>();
        lst.Append("X");
        lst.Append("XX");
        lst.Append("XXX");

        string cmp = "X";
        foreach (string e in lst)
        {
            Test.AssertEquals(cmp, e);
            cmp = cmp + "X";
        }
    }

    public static void eina_inlist_as_ienumerable_obj()
    {
        var a = new test.Numberwrapper();
        var b = new test.Numberwrapper();
        var c = new test.Numberwrapper();
        a.SetNumber(88);
        b.SetNumber(44);
        c.SetNumber(22);
        var cmp = new test.Numberwrapper[]{a,b,c};

        var lst = new eina.Inlist<test.INumberwrapper>();
        lst.Append(a);
        lst.Append(b);
        lst.Append(c);

        int i = 0;
        foreach (test.Numberwrapper e in lst)
        {
            Test.AssertEquals(cmp[i].GetNumber(), e.GetNumber());
            Test.Assert(cmp[i].raw_handle == e.raw_handle);
            ++i;
        }
    }

    // //
    // Code Generation
    //

    // Integer //

    public static void test_eina_inlist_int_in()
    {
        test.ITesting t = new test.Testing();
        var lst = new eina.Inlist<int>();
        lst.AppendArray(base_seq_int);
        Test.Assert(t.EinaInlistIntIn(lst));
        Test.Assert(lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_int));
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    public static void test_eina_inlist_int_in_own()
    {
        test.ITesting t = new test.Testing();
        var lst = new eina.Inlist<int>();
        lst.AppendArray(base_seq_int);
        Test.Assert(t.EinaInlistIntInOwn(lst));
        Test.Assert(!lst.Own);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaInlistIntInOwn());
    }

    public static void test_eina_inlist_int_out()
    {
        test.ITesting t = new test.Testing();
        eina.Inlist<int> lst;
        Test.Assert(t.EinaInlistIntOut(out lst));
        Test.Assert(!lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_int));
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaInlistIntOut());
    }

    public static void test_eina_inlist_int_out_own()
    {
        test.ITesting t = new test.Testing();
        eina.Inlist<int> lst;
        Test.Assert(t.EinaInlistIntOutOwn(out lst));
        Test.Assert(lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_int));
        lst.AppendArray(append_seq_int);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    public static void test_eina_inlist_int_return()
    {
        test.ITesting t = new test.Testing();
        var lst = t.EinaInlistIntReturn();
        Test.Assert(!lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_int));
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaInlistIntReturn());
    }

    public static void test_eina_inlist_int_return_own()
    {
        test.ITesting t = new test.Testing();
        var lst = t.EinaInlistIntReturnOwn();
        Test.Assert(lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_int));
        lst.AppendArray(append_seq_int);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    // String //
    public static void test_eina_inlist_str_in()
    {
        test.ITesting t = new test.Testing();
        var lst = new eina.Inlist<string>();
        lst.AppendArray(base_seq_str);
        Test.Assert(t.EinaInlistStrIn(lst));
        Test.Assert(lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_str));
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    public static void test_eina_inlist_str_in_own()
    {
        test.ITesting t = new test.Testing();
        var lst = new eina.Inlist<string>();
        lst.AppendArray(base_seq_str);
        Test.Assert(t.EinaInlistStrInOwn(lst));
        Test.Assert(!lst.Own);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaInlistStrInOwn());
    }

    public static void test_eina_inlist_str_out()
    {
        test.ITesting t = new test.Testing();
        eina.Inlist<string> lst;
        Test.Assert(t.EinaInlistStrOut(out lst));
        Test.Assert(!lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_str));
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaInlistStrOut());
    }

    public static void test_eina_inlist_str_out_own()
    {
        test.ITesting t = new test.Testing();
        eina.Inlist<string> lst;
        Test.Assert(t.EinaInlistStrOutOwn(out lst));
        Test.Assert(lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_str));
        lst.AppendArray(append_seq_str);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    public static void test_eina_inlist_str_return()
    {
        test.ITesting t = new test.Testing();
        var lst = t.EinaInlistStrReturn();
        Test.Assert(!lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_str));
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaInlistStrReturn());
    }

    public static void test_eina_inlist_str_return_own()
    {
        test.ITesting t = new test.Testing();
        var lst = t.EinaInlistStrReturnOwn();
        Test.Assert(lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_str));
        lst.AppendArray(append_seq_str);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    // Object //

    public static void test_eina_inlist_obj_in()
    {
        test.ITesting t = new test.Testing();
        var lst = new eina.Inlist<test.INumberwrapper>();
        lst.AppendArray(BaseSeqObj());
        Test.Assert(t.EinaInlistObjIn(lst));
        Test.Assert(lst.Own);
        NumberwrapperSequenceAssertEqual(lst.ToArray(), BaseSeqObj());
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    public static void test_eina_inlist_obj_in_own()
    {
        test.ITesting t = new test.Testing();
        var lst = new eina.Inlist<test.INumberwrapper>();
        lst.AppendArray(BaseSeqObj());
        Test.Assert(t.EinaInlistObjInOwn(lst));
        Test.Assert(!lst.Own);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaInlistObjInOwn());
    }

    public static void test_eina_inlist_obj_out()
    {
        test.ITesting t = new test.Testing();
        eina.Inlist<test.INumberwrapper> lst;
        Test.Assert(t.EinaInlistObjOut(out lst));
        Test.Assert(!lst.Own);
        NumberwrapperSequenceAssertEqual(lst.ToArray(), BaseSeqObj());
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaInlistObjOut());
    }

    public static void test_eina_inlist_obj_out_own()
    {
        test.ITesting t = new test.Testing();
        eina.Inlist<test.INumberwrapper> lst;
        Test.Assert(t.EinaInlistObjOutOwn(out lst));
        Test.Assert(lst.Own);
        NumberwrapperSequenceAssertEqual(lst.ToArray(), BaseSeqObj());
        lst.AppendArray(AppendSeqObj());
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    public static void test_eina_inlist_obj_return()
    {
        test.ITesting t = new test.Testing();
        var lst = t.EinaInlistObjReturn();
        Test.Assert(!lst.Own);
        NumberwrapperSequenceAssertEqual(lst.ToArray(), BaseSeqObj());
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaInlistObjReturn());
    }

    public static void test_eina_inlist_obj_return_own()
    {
        test.ITesting t = new test.Testing();
        var lst = t.EinaInlistObjReturnOwn();
        Test.Assert(lst.Own);
        NumberwrapperSequenceAssertEqual(lst.ToArray(), BaseSeqObj());
        lst.AppendArray(AppendSeqObj());
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    public static void test_eina_inlist_obj_return_in_same_id()
    {
        test.ITesting t = new test.Testing();
        var cmp = BaseSeqObj();
        var a = new eina.Inlist<test.INumberwrapper>();
        a.AppendArray(cmp);
        var b = t.EinaInlistObjReturnIn(a);
        NumberwrapperSequenceAssertEqual(a.ToArray(), b.ToArray());
        NumberwrapperSequenceAssertEqual(a.ToArray(), BaseSeqObj());
        int len = a.Length;
        for (int i=0; i < len; ++i)
        {
            Test.Assert(a[i].raw_handle == b[i].raw_handle);
            Test.Assert(a[i].raw_handle == cmp[i].raw_handle);
        }
    }
} // < TestEinaInlist


class TestEinaHash
{
    public static void data_set_int()
    {
        var hsh = new eina.Hash<int, int>();
        Test.Assert(hsh.Count == 0);

        hsh[88] = 888;
        Test.Assert(hsh[88] == 888);
        Test.Assert(hsh.Count == 1);

        hsh[44] = 444;
        Test.Assert(hsh[44] == 444);
        Test.Assert(hsh.Count == 2);

        hsh[22] = 222;
        Test.Assert(hsh[22] == 222);
        Test.Assert(hsh.Count == 3);

        hsh.Dispose();
    }

    public static void data_set_str()
    {
        var hsh = new eina.Hash<string, string>();
        Test.Assert(hsh.Count == 0);

        hsh["aa"] = "aaa";
        Test.Assert(hsh["aa"] == "aaa");
        Test.Assert(hsh.Count == 1);

        hsh["bb"] = "bbb";
        Test.Assert(hsh["bb"] == "bbb");
        Test.Assert(hsh.Count == 2);

        hsh["cc"] = "ccc";
        Test.Assert(hsh["cc"] == "ccc");
        Test.Assert(hsh.Count == 3);

        hsh.Dispose();
    }

    public static void data_set_obj()
    {
        var hsh = new eina.Hash<test.INumberwrapper, test.INumberwrapper>();
        Test.Assert(hsh.Count == 0);

        var a = NW(22);
        var aa = NW(222);
        var b = NW(44);
        var bb = NW(444);
        var c = NW(88);
        var cc = NW(888);

        hsh[a] = aa;
        Test.Assert(hsh[a].raw_handle == aa.raw_handle);
        Test.Assert(hsh[a].GetNumber() == aa.GetNumber());
        Test.Assert(hsh.Count == 1);

        hsh[b] = bb;
        Test.Assert(hsh[b].raw_handle == bb.raw_handle);
        Test.Assert(hsh[b].GetNumber() == bb.GetNumber());
        Test.Assert(hsh.Count == 2);

        hsh[c] = cc;
        Test.Assert(hsh[c].raw_handle == cc.raw_handle);
        Test.Assert(hsh[c].GetNumber() == cc.GetNumber());

        Test.Assert(hsh.Count == 3);

        hsh.Dispose();
    }

    public static void eina_hash_as_ienumerable_int()
    {
        var hsh = new eina.Hash<int, int>();
        var dct = new Dictionary<int, int>();

        hsh[88] = 888;
        hsh[44] = 444;
        hsh[22] = 222;

        dct[88] = 888;
        dct[44] = 444;
        dct[22] = 222;

        int count = 0;

        foreach (KeyValuePair<int, int> kvp in hsh)
        {
            Test.Assert(dct[kvp.Key] == kvp.Value);
            dct.Remove(kvp.Key);
            ++count;
        }

        Test.AssertEquals(count, 3);
        Test.AssertEquals(dct.Count, 0);

        hsh.Dispose();
    }

    public static void eina_hash_as_ienumerable_str()
    {
        var hsh = new eina.Hash<string, string>();
        var dct = new Dictionary<string, string>();

        hsh["aa"] = "aaa";
        hsh["bb"] = "bbb";
        hsh["cc"] = "ccc";

        dct["aa"] = "aaa";
        dct["bb"] = "bbb";
        dct["cc"] = "ccc";

        int count = 0;

        foreach (KeyValuePair<string, string> kvp in hsh)
        {
            Test.Assert(dct[kvp.Key] == kvp.Value);
            dct.Remove(kvp.Key);
            ++count;
        }

        Test.AssertEquals(count, 3);
        Test.AssertEquals(dct.Count, 0);

        hsh.Dispose();
    }

    public static void eina_hash_as_ienumerable_obj()
    {
        var hsh = new eina.Hash<test.INumberwrapper, test.INumberwrapper>();
        var dct = new Dictionary<int, test.INumberwrapper>();

        var a = NW(22);
        var aa = NW(222);
        var b = NW(44);
        var bb = NW(444);
        var c = NW(88);
        var cc = NW(888);

        hsh[a] = aa;
        hsh[b] = bb;
        hsh[c] = cc;

        dct[a.GetNumber()] = aa;
        dct[b.GetNumber()] = bb;
        dct[c.GetNumber()] = cc;

        int count = 0;

        foreach (KeyValuePair<test.INumberwrapper, test.INumberwrapper> kvp in hsh)
        {
            Test.Assert(dct[kvp.Key.GetNumber()].raw_handle == kvp.Value.raw_handle);
            Test.Assert(dct[kvp.Key.GetNumber()].GetNumber() == kvp.Value.GetNumber());
            dct.Remove(kvp.Key.GetNumber());
            ++count;
        }

        Test.AssertEquals(count, 3);
        Test.AssertEquals(dct.Count, 0);

        hsh.Dispose();
    }

    // //
    // Code Generation
    //

    // Integer //

    public static void test_eina_hash_int_in()
    {
        test.ITesting t = new test.Testing();
        var hsh = new eina.Hash<int,int>();
        hsh[22] = 222;
        Test.Assert(t.EinaHashIntIn(hsh));
        Test.Assert(hsh.Own);
        Test.Assert(hsh[22] == 222);
        Test.Assert(hsh[44] == 444);
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
    }

    public static void test_eina_hash_int_in_own()
    {
        test.ITesting t = new test.Testing();
        var hsh = new eina.Hash<int,int>();
        hsh[22] = 222;
        Test.Assert(t.EinaHashIntInOwn(hsh));
        Test.Assert(!hsh.Own);
        Test.Assert(hsh[22] == 222);
        Test.Assert(hsh[44] == 444);
        hsh[88] = 888;
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashIntInOwn());
    }

    public static void test_eina_hash_int_out()
    {
        test.ITesting t = new test.Testing();
        eina.Hash<int,int> hsh;
        Test.Assert(t.EinaHashIntOut(out hsh));
        Test.Assert(!hsh.Own);
        Test.Assert(hsh[22] == 222);
        hsh[44] = 444;
        Test.Assert(hsh[44] == 444);
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashIntOut());
    }

    public static void test_eina_hash_int_out_own()
    {
        test.ITesting t = new test.Testing();
        eina.Hash<int,int> hsh;
        Test.Assert(t.EinaHashIntOutOwn(out hsh));
        Test.Assert(hsh.Own);
        Test.Assert(hsh[22] == 222);
        hsh[44] = 444;
        Test.Assert(hsh[44] == 444);
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashIntOutOwn());
    }

    public static void test_eina_hash_int_return()
    {
        test.ITesting t = new test.Testing();
        var hsh = t.EinaHashIntReturn();
        Test.Assert(!hsh.Own);
        Test.Assert(hsh[22] == 222);
        hsh[44] = 444;
        Test.Assert(hsh[44] == 444);
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashIntReturn());
    }

    public static void test_eina_hash_int_return_own()
    {
        test.ITesting t = new test.Testing();
        var hsh = t.EinaHashIntReturnOwn();
        Test.Assert(hsh.Own);
        Test.Assert(hsh[22] == 222);
        hsh[44] = 444;
        Test.Assert(hsh[44] == 444);
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashIntReturnOwn());
    }

    // String //

    public static void test_eina_hash_str_in()
    {
        test.ITesting t = new test.Testing();
        var hsh = new eina.Hash<string,string>();
        hsh["aa"] = "aaa";
        Test.Assert(t.EinaHashStrIn(hsh));
        Test.Assert(hsh.Own);
        Test.Assert(hsh["aa"] == "aaa");
        Test.Assert(hsh["bb"] == "bbb");
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
    }

    public static void test_eina_hash_str_in_own()
    {
        test.ITesting t = new test.Testing();
        var hsh = new eina.Hash<string,string>();
        hsh["aa"] = "aaa";
        Test.Assert(t.EinaHashStrInOwn(hsh));
        Test.Assert(!hsh.Own);
        Test.Assert(hsh["aa"] == "aaa");
        Test.Assert(hsh["bb"] == "bbb");
        hsh["cc"] = "ccc";
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashStrInOwn());
    }

    public static void test_eina_hash_str_out()
    {
        test.ITesting t = new test.Testing();
        eina.Hash<string,string> hsh;
        Test.Assert(t.EinaHashStrOut(out hsh));
        Test.Assert(!hsh.Own);
        Test.Assert(hsh["aa"] == "aaa");
        hsh["bb"] = "bbb";
        Test.Assert(hsh["bb"] == "bbb");
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashStrOut());
    }

    public static void test_eina_hash_str_out_own()
    {
        test.ITesting t = new test.Testing();
        eina.Hash<string,string> hsh;
        Test.Assert(t.EinaHashStrOutOwn(out hsh));
        Test.Assert(hsh.Own);
        Test.Assert(hsh["aa"] == "aaa");
        hsh["bb"] = "bbb";
        Test.Assert(hsh["bb"] == "bbb");
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashStrOutOwn());
    }

    public static void test_eina_hash_str_return()
    {
        test.ITesting t = new test.Testing();
        var hsh = t.EinaHashStrReturn();
        Test.Assert(!hsh.Own);
        Test.Assert(hsh["aa"] == "aaa");
        hsh["bb"] = "bbb";
        Test.Assert(hsh["bb"] == "bbb");
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashStrReturn());
    }

    public static void test_eina_hash_str_return_own()
    {
        test.ITesting t = new test.Testing();
        var hsh = t.EinaHashStrReturnOwn();
        Test.Assert(hsh.Own);
        Test.Assert(hsh["aa"] == "aaa");
        hsh["bb"] = "bbb";
        Test.Assert(hsh["bb"] == "bbb");
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashStrReturnOwn());
    }

    // Object //

    public static void test_eina_hash_obj_in()
    {
        test.ITesting t = new test.Testing();
        var hsh = new eina.Hash<test.INumberwrapper, test.INumberwrapper>();
        var nwk1 = NW(22);
        var nwv1 = NW(222);
        hsh[nwk1] = nwv1;
        test.INumberwrapper nwk2;
        test.INumberwrapper nwv2;
        Test.Assert(t.EinaHashObjIn(hsh, nwk1, nwv1, out nwk2, out nwv2));
        Test.Assert(hsh.Own);
        Test.Assert(hsh[nwk1].raw_handle == nwv1.raw_handle);
        Test.Assert(hsh[nwk1].GetNumber() == nwv1.GetNumber());
        Test.Assert(hsh[nwk1].GetNumber() == 222);
        Test.Assert(hsh[nwk2].raw_handle == nwv2.raw_handle);
        Test.Assert(hsh[nwk2].GetNumber() == nwv2.GetNumber());
        Test.Assert(hsh[nwk2].GetNumber() == 444);
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
    }

    public static void test_eina_hash_obj_in_own()
    {
        test.ITesting t = new test.Testing();
        var hsh = new eina.Hash<test.INumberwrapper, test.INumberwrapper>();
        var nwk1 = NW(22);
        var nwv1 = NW(222);
        hsh[nwk1] = nwv1;
        test.INumberwrapper nwk2;
        test.INumberwrapper nwv2;
        Test.Assert(t.EinaHashObjInOwn(hsh, nwk1, nwv1, out nwk2, out nwv2));
        Test.Assert(!hsh.Own);
        Test.Assert(hsh[nwk1].raw_handle == nwv1.raw_handle);
        Test.Assert(hsh[nwk1].GetNumber() == nwv1.GetNumber());
        Test.Assert(hsh[nwk1].GetNumber() == 222);
        Test.Assert(hsh[nwk2].raw_handle == nwv2.raw_handle);
        Test.Assert(hsh[nwk2].GetNumber() == nwv2.GetNumber());
        Test.Assert(hsh[nwk2].GetNumber() == 444);
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashObjInOwn(nwk1, nwv1, nwk2, nwv2));
    }

    public static void test_eina_hash_obj_out()
    {
        test.ITesting t = new test.Testing();
        eina.Hash<test.INumberwrapper, test.INumberwrapper> hsh;
        test.INumberwrapper nwk1;
        test.INumberwrapper nwv1;
        Test.Assert(t.EinaHashObjOut(out hsh, out nwk1, out nwv1));
        Test.Assert(!hsh.Own);
        Test.Assert(hsh[nwk1].raw_handle == nwv1.raw_handle);
        Test.Assert(hsh[nwk1].GetNumber() == nwv1.GetNumber());
        Test.Assert(hsh[nwk1].GetNumber() == 222);
        test.INumberwrapper nwk2 = NW(44);
        test.INumberwrapper nwv2 = NW(444);
        hsh[nwk2] = nwv2;
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashObjOut(nwk1, nwv1, nwk2, nwv2));
    }

    public static void test_eina_hash_obj_out_own()
    {
        test.ITesting t = new test.Testing();
        eina.Hash<test.INumberwrapper, test.INumberwrapper> hsh;
        test.INumberwrapper nwk1;
        test.INumberwrapper nwv1;
        Test.Assert(t.EinaHashObjOutOwn(out hsh, out nwk1, out nwv1));
        Test.Assert(hsh.Own);
        Test.Assert(hsh[nwk1].raw_handle == nwv1.raw_handle);
        Test.Assert(hsh[nwk1].GetNumber() == nwv1.GetNumber());
        Test.Assert(hsh[nwk1].GetNumber() == 222);
        test.INumberwrapper nwk2 = NW(44);
        test.INumberwrapper nwv2 = NW(444);
        hsh[nwk2] = nwv2;
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashObjOutOwn());
    }

    public static void test_eina_hash_obj_return()
    {
        test.ITesting t = new test.Testing();
        test.INumberwrapper nwk1;
        test.INumberwrapper nwv1;
        var hsh = t.EinaHashObjReturn(out nwk1, out nwv1);
        Test.Assert(!hsh.Own);
        Test.Assert(hsh[nwk1].raw_handle == nwv1.raw_handle);
        Test.Assert(hsh[nwk1].GetNumber() == nwv1.GetNumber());
        Test.Assert(hsh[nwk1].GetNumber() == 222);
        test.INumberwrapper nwk2 = NW(44);
        test.INumberwrapper nwv2 = NW(444);
        hsh[nwk2] = nwv2;
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashObjReturn(nwk1, nwv1, nwk2, nwv2));
    }

    public static void test_eina_hash_obj_return_own()
    {
        test.ITesting t = new test.Testing();
        test.INumberwrapper nwk1;
        test.INumberwrapper nwv1;
        var hsh = t.EinaHashObjReturnOwn(out nwk1, out nwv1);
        Test.Assert(hsh.Own);
        Test.Assert(hsh[nwk1].raw_handle == nwv1.raw_handle);
        Test.Assert(hsh[nwk1].GetNumber() == nwv1.GetNumber());
        Test.Assert(hsh[nwk1].GetNumber() == 222);
        test.INumberwrapper nwk2 = NW(44);
        test.INumberwrapper nwv2 = NW(444);
        hsh[nwk2] = nwv2;
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashObjReturnOwn());
    }
}


class TestEinaIterator
{
    // Array //

    public static void eina_array_int_empty_iterator()
    {
        var arr = new eina.Array<int>();
        var itr = arr.GetIterator();
        int idx = 0;
        foreach (int e in itr)
        {
            ++idx;
        }
        Test.AssertEquals(idx, 0);

        itr.Dispose();
        arr.Dispose();
    }

    public static void eina_array_int_filled_iterator()
    {
        var arr = new eina.Array<int>();
        arr.Append(base_seq_int);
        var itr = arr.GetIterator();
        int idx = 0;
        foreach (int e in itr)
        {
            Test.Assert(e == base_seq_int[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_int.Length);

        itr.Dispose();
        arr.Dispose();
    }

    public static void eina_array_str_empty_iterator()
    {
        var arr = new eina.Array<string>();
        var itr = arr.GetIterator();
        int idx = 0;
        foreach (string e in itr)
        {
            ++idx;
        }
        Test.AssertEquals(idx, 0);

        itr.Dispose();
        arr.Dispose();
    }

    public static void eina_array_str_filled_iterator()
    {
        var arr = new eina.Array<string>();
        arr.Append(base_seq_str);
        var itr = arr.GetIterator();
        int idx = 0;
        foreach (string e in itr)
        {
            Test.Assert(e == base_seq_str[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_str.Length);

        itr.Dispose();
        arr.Dispose();
    }

    public static void eina_array_obj_empty_iterator()
    {
        var arr = new eina.Array<test.INumberwrapper>();
        var itr = arr.GetIterator();
        int idx = 0;
        foreach (test.INumberwrapper e in itr)
        {
            ++idx;
        }
        Test.AssertEquals(idx, 0);

        itr.Dispose();
        arr.Dispose();
    }

    public static void eina_array_obj_filled_iterator()
    {
        var arr = new eina.Array<test.INumberwrapper>();
        var base_objs = BaseSeqObj();
        arr.Append(base_objs);
        var itr = arr.GetIterator();
        int idx = 0;
        foreach (test.INumberwrapper e in itr)
        {
            Test.Assert(e.raw_handle == base_objs[idx].raw_handle);
            Test.Assert(e.GetNumber() == base_objs[idx].GetNumber());
            ++idx;
        }
        Test.AssertEquals(idx, base_objs.Length);

        itr.Dispose();
        arr.Dispose();
    }

    // Inarray

    public static void eina_inarray_int_empty_iterator()
    {
        var arr = new eina.Inarray<int>();
        var itr = arr.GetIterator();
        int idx = 0;
        foreach (int e in itr)
        {
            ++idx;
        }
        Test.AssertEquals(idx, 0);

        itr.Dispose();
        arr.Dispose();
    }

    public static void eina_inarray_int_filled_iterator()
    {
        var arr = new eina.Inarray<int>();
        arr.Append(base_seq_int);
        var itr = arr.GetIterator();
        int idx = 0;
        foreach (int e in itr)
        {
            Test.Assert(e == base_seq_int[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_int.Length);

        itr.Dispose();
        arr.Dispose();
    }

    // List //

    public static void eina_list_int_empty_iterator()
    {
        var lst = new eina.List<int>();
        var itr = lst.GetIterator();
        int idx = 0;
        foreach (int e in itr)
        {
            ++idx;
        }
        Test.AssertEquals(idx, 0);

        itr.Dispose();
        lst.Dispose();
    }

    public static void eina_list_int_filled_iterator()
    {
        var lst = new eina.List<int>();
        lst.AppendArray(base_seq_int);
        var itr = lst.GetIterator();
        int idx = 0;
        foreach (int e in itr)
        {
            Test.Assert(e == base_seq_int[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_int.Length);

        itr.Dispose();
        lst.Dispose();
    }

    public static void eina_list_str_empty_iterator()
    {
        var lst = new eina.List<string>();
        var itr = lst.GetIterator();
        int idx = 0;
        foreach (string e in itr)
        {
            ++idx;
        }
        Test.AssertEquals(idx, 0);

        itr.Dispose();
        lst.Dispose();
    }

    public static void eina_list_str_filled_iterator()
    {
        var lst = new eina.List<string>();
        lst.AppendArray(base_seq_str);
        var itr = lst.GetIterator();
        int idx = 0;
        foreach (string e in itr)
        {
            Test.Assert(e == base_seq_str[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_str.Length);

        itr.Dispose();
        lst.Dispose();
    }

    public static void eina_list_obj_empty_iterator()
    {
        var lst = new eina.List<test.INumberwrapper>();
        var itr = lst.GetIterator();
        int idx = 0;
        foreach (test.INumberwrapper e in itr)
        {
            ++idx;
        }
        Test.AssertEquals(idx, 0);

        itr.Dispose();
        lst.Dispose();
    }

    public static void eina_list_obj_filled_iterator()
    {
        var lst = new eina.List<test.INumberwrapper>();
        var base_objs = BaseSeqObj();
        lst.AppendArray(base_objs);
        var itr = lst.GetIterator();
        int idx = 0;
        foreach (test.INumberwrapper e in itr)
        {
            Test.Assert(e.raw_handle == base_objs[idx].raw_handle);
            Test.Assert(e.GetNumber() == base_objs[idx].GetNumber());
            ++idx;
        }
        Test.AssertEquals(idx, base_objs.Length);

        itr.Dispose();
        lst.Dispose();
    }

    // Inlist //

    public static void eina_inlist_int_empty_iterator()
    {
        var lst = new eina.Inlist<int>();
        var itr = lst.GetIterator();
        int idx = 0;
        foreach (int e in itr)
        {
            ++idx;
        }
        Test.AssertEquals(idx, 0);

        itr.Dispose();
        lst.Dispose();
    }

    public static void eina_inlist_int_filled_iterator()
    {
        var lst = new eina.Inlist<int>();
        lst.AppendArray(base_seq_int);
        var itr = lst.GetIterator();
        int idx = 0;
        foreach (int e in itr)
        {
            Test.Assert(e == base_seq_int[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_int.Length);

        itr.Dispose();
        lst.Dispose();
    }

    // Hash //

    public static void eina_hash_keys_int_empty_iterator()
    {
        var hsh = new eina.Hash<int, int>();
        var itr = hsh.Keys();
        bool entered = false;
        foreach (int e in itr)
        {
            entered = true;
        }
        Test.Assert(!entered);

        itr.Dispose();
        hsh.Dispose();
    }

    public static void eina_hash_values_int_empty_iterator()
    {
        var hsh = new eina.Hash<int, int>();
        var itr = hsh.Values();
        bool entered = false;
        foreach (int e in itr)
        {
            entered = true;
        }
        Test.Assert(!entered);

        itr.Dispose();
        hsh.Dispose();
    }

    public static void eina_hash_keys_int_filled_iterator()
    {
        var hsh = new eina.Hash<int, int>();
        var dct = new Dictionary<int, bool>();
        hsh[22] = 222;
        hsh[44] = 444;
        hsh[88] = 888;
        dct[22] = true;
        dct[44] = true;
        dct[88] = true;

        var itr = hsh.Keys();

        int idx = 0;
        foreach (int e in itr)
        {
            Test.Assert(dct[e]);
            dct.Remove(e);
            ++idx;
        }
        Test.AssertEquals(dct.Count, 0);
        Test.AssertEquals(idx, 3);

        itr.Dispose();
        hsh.Dispose();
    }

    public static void eina_hash_values_int_filled_iterator()
    {
        var hsh = new eina.Hash<int, int>();
        var dct = new Dictionary<int, bool>();
        hsh[22] = 222;
        hsh[44] = 444;
        hsh[88] = 888;
        dct[222] = true;
        dct[444] = true;
        dct[888] = true;

        var itr = hsh.Values();

        int idx = 0;
        foreach (int e in itr)
        {
            Test.Assert(dct[e]);
            dct.Remove(e);
            ++idx;
        }
        Test.AssertEquals(dct.Count, 0);
        Test.AssertEquals(idx, 3);

        itr.Dispose();
        hsh.Dispose();
    }

    public static void eina_hash_keys_str_empty_iterator()
    {
        var hsh = new eina.Hash<string, string>();
        var itr = hsh.Keys();
        bool entered = false;
        foreach (string e in itr)
        {
            entered = true;
        }
        Test.Assert(!entered);

        itr.Dispose();
        hsh.Dispose();
    }

    public static void eina_hash_values_str_empty_iterator()
    {
        var hsh = new eina.Hash<string, string>();
        var itr = hsh.Values();
        bool entered = false;
        foreach (string e in itr)
        {
            entered = true;
        }
        Test.Assert(!entered);

        itr.Dispose();
        hsh.Dispose();
    }

    public static void eina_hash_keys_str_filled_iterator()
    {
        var hsh = new eina.Hash<string, string>();
        var dct = new Dictionary<string, bool>();
        hsh["aa"] = "aaa";
        hsh["bb"] = "bbb";
        hsh["cc"] = "ccc";
        dct["aa"] = true;
        dct["bb"] = true;
        dct["cc"] = true;

        var itr = hsh.Keys();

        int idx = 0;
        foreach (string e in itr)
        {
            Test.Assert(dct[e]);
            dct.Remove(e);
            ++idx;
        }
        Test.AssertEquals(dct.Count, 0);
        Test.AssertEquals(idx, 3);

        itr.Dispose();
        hsh.Dispose();
    }

    public static void eina_hash_values_str_filled_iterator()
    {
        var hsh = new eina.Hash<string, string>();
        var dct = new Dictionary<string, bool>();
        hsh["aa"] = "aaa";
        hsh["bb"] = "bbb";
        hsh["cc"] = "ccc";
        dct["aaa"] = true;
        dct["bbb"] = true;
        dct["ccc"] = true;

        var itr = hsh.Values();

        int idx = 0;
        foreach (string e in itr)
        {
            Test.Assert(dct[e]);
            dct.Remove(e);
            ++idx;
        }
        Test.AssertEquals(dct.Count, 0);
        Test.AssertEquals(idx, 3);

        itr.Dispose();
        hsh.Dispose();
    }

    public static void eina_hash_keys_obj_empty_iterator()
    {
        var hsh = new eina.Hash<test.INumberwrapper, test.INumberwrapper>();
        var itr = hsh.Keys();
        bool entered = false;
        foreach (test.INumberwrapper e in itr)
        {
            entered = true;
        }
        Test.Assert(!entered);

        itr.Dispose();
        hsh.Dispose();
    }

    public static void eina_hash_values_obj_empty_iterator()
    {
        var hsh = new eina.Hash<test.INumberwrapper, test.INumberwrapper>();
        var itr = hsh.Values();
        bool entered = false;
        foreach (test.INumberwrapper e in itr)
        {
            entered = true;
        }
        Test.Assert(!entered);

        itr.Dispose();
        hsh.Dispose();
    }

    public static void eina_hash_keys_obj_filled_iterator()
    {
        var hsh = new eina.Hash<test.INumberwrapper, test.INumberwrapper>();
        var dct = new eina.Hash<int, test.INumberwrapper>();
        var a = NW(22);
        var b = NW(44);
        var c = NW(88);
        var aa = NW(222);
        var bb = NW(444);
        var cc = NW(888);
        hsh[a] = aa;
        hsh[b] = bb;
        hsh[c] = cc;
        dct[a.GetNumber()] = a;
        dct[b.GetNumber()] = b;
        dct[c.GetNumber()] = c;

        var itr = hsh.Keys();

        int idx = 0;
        foreach (test.INumberwrapper e in itr)
        {
            Test.Assert(dct[e.GetNumber()] != null);
            Test.Assert(dct[e.GetNumber()].raw_handle == e.raw_handle);
            Test.Assert(dct[e.GetNumber()].GetNumber() == e.GetNumber());
            dct.Remove(e.GetNumber());
            ++idx;
        }
        Test.AssertEquals(dct.Count, 0);
        Test.AssertEquals(idx, 3);

        itr.Dispose();
        hsh.Dispose();
    }

    public static void eina_hash_values_obj_filled_iterator()
    {
        var hsh = new eina.Hash<test.INumberwrapper, test.INumberwrapper>();
        var dct = new eina.Hash<int, test.INumberwrapper>();
        var a = NW(22);
        var b = NW(44);
        var c = NW(88);
        var aa = NW(222);
        var bb = NW(444);
        var cc = NW(888);
        hsh[a] = aa;
        hsh[b] = bb;
        hsh[c] = cc;
        dct[aa.GetNumber()] = aa;
        dct[bb.GetNumber()] = bb;
        dct[cc.GetNumber()] = cc;

        var itr = hsh.Values();

        int idx = 0;
        foreach (test.INumberwrapper e in itr)
        {
            Test.Assert(dct[e.GetNumber()] != null);
            Test.Assert(dct[e.GetNumber()].raw_handle == e.raw_handle);
            Test.Assert(dct[e.GetNumber()].GetNumber() == e.GetNumber());
            dct.Remove(e.GetNumber());
            ++idx;
        }
        Test.AssertEquals(dct.Count, 0);
        Test.AssertEquals(idx, 3);

        itr.Dispose();
        hsh.Dispose();
    }

    // //
    // Code Generation
    //

    // Integer //

    public static void test_eina_iterator_int_in()
    {
        test.ITesting t = new test.Testing();
        var arr = new eina.Array<int>();
        arr.Append(base_seq_int);
        var itr = arr.GetIterator();

        Test.Assert(itr.Own);
        Test.Assert(!itr.OwnContent);
        Test.Assert(arr.Own);
        Test.Assert(arr.OwnContent);

        Test.Assert(t.EinaIteratorIntIn(itr));

        Test.Assert(itr.Own);
        Test.Assert(!itr.OwnContent);
        Test.Assert(arr.Own);
        Test.Assert(arr.OwnContent);

        itr.Dispose();
        arr.Dispose();
    }

    public static void test_eina_iterator_int_in_own()
    {
        test.ITesting t = new test.Testing();
        var arr = new eina.Array<int>();
        arr.Append(base_seq_int);
        var itr = arr.GetIterator();
        arr.OwnContent = false;
        itr.OwnContent = true;

        Test.Assert(itr.Own);
        Test.Assert(itr.OwnContent);
        Test.Assert(arr.Own);
        Test.Assert(!arr.OwnContent);

        Test.Assert(t.EinaIteratorIntInOwn(itr));

        Test.Assert(!itr.Own);
        Test.Assert(!itr.OwnContent);
        Test.Assert(arr.Own);
        Test.Assert(!arr.OwnContent);

        itr.Dispose();
        arr.Dispose();

        Test.Assert(t.CheckEinaIteratorIntInOwn());
    }

    public static void test_eina_iterator_int_out()
    {
        test.ITesting t = new test.Testing();
        eina.Iterator<int> itr;

        Test.Assert(t.EinaIteratorIntOut(out itr));


        Test.Assert(!itr.Own);
        Test.Assert(!itr.OwnContent);

        int idx = 0;
        foreach (int e in itr)
        {
            Test.AssertEquals(e, base_seq_int[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_int.Length);

        itr.Dispose();

        Test.Assert(t.CheckEinaIteratorIntOut());
    }

    public static void test_eina_iterator_int_out_own()
    {
        test.ITesting t = new test.Testing();
        eina.Iterator<int> itr;

        Test.Assert(t.EinaIteratorIntOutOwn(out itr));

        Test.Assert(itr.Own);
        Test.Assert(itr.OwnContent);

        int idx = 0;
        foreach (int e in itr)
        {
            Test.AssertEquals(e, base_seq_int[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_int.Length);

        itr.Dispose();
    }

    public static void test_eina_iterator_int_return()
    {
        test.ITesting t = new test.Testing();

        var itr = t.EinaIteratorIntReturn();

        Test.Assert(!itr.Own);
        Test.Assert(!itr.OwnContent);

        int idx = 0;
        foreach (int e in itr)
        {
            Test.AssertEquals(e, base_seq_int[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_int.Length);

        itr.Dispose();

        Test.Assert(t.CheckEinaIteratorIntReturn());
    }

    public static void test_eina_iterator_int_return_own()
    {
        test.ITesting t = new test.Testing();

        var itr = t.EinaIteratorIntReturnOwn();

        Test.Assert(itr.Own);
        Test.Assert(itr.OwnContent);

        int idx = 0;
        foreach (int e in itr)
        {
            Test.AssertEquals(e, base_seq_int[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_int.Length);

        itr.Dispose();
    }

    // String //

    public static void test_eina_iterator_str_in()
    {
        test.ITesting t = new test.Testing();
        var arr = new eina.Array<string>();
        arr.Append(base_seq_str);
        var itr = arr.GetIterator();

        Test.Assert(itr.Own);
        Test.Assert(!itr.OwnContent);
        Test.Assert(arr.Own);
        Test.Assert(arr.OwnContent);

        Test.Assert(t.EinaIteratorStrIn(itr));

        Test.Assert(itr.Own);
        Test.Assert(!itr.OwnContent);
        Test.Assert(arr.Own);
        Test.Assert(arr.OwnContent);

        itr.Dispose();
        arr.Dispose();
    }

    public static void test_eina_iterator_str_in_own()
    {
        test.ITesting t = new test.Testing();
        var arr = new eina.Array<string>();
        arr.Append(base_seq_str);
        var itr = arr.GetIterator();
        arr.OwnContent = false;
        itr.OwnContent = true;

        Test.Assert(itr.Own);
        Test.Assert(itr.OwnContent);
        Test.Assert(arr.Own);
        Test.Assert(!arr.OwnContent);

        Test.Assert(t.EinaIteratorStrInOwn(itr));

        Test.Assert(!itr.Own);
        Test.Assert(!itr.OwnContent);
        Test.Assert(!itr.Own);
        Test.Assert(!itr.OwnContent);
        Test.Assert(arr.Own);
        Test.Assert(!arr.OwnContent);
        Test.Assert(arr.Own);
        Test.Assert(!arr.OwnContent);

        itr.Dispose();
        arr.Dispose();

        Test.Assert(t.CheckEinaIteratorStrInOwn());
    }

    public static void test_eina_iterator_str_out()
    {
        test.ITesting t = new test.Testing();
        eina.Iterator<string> itr;

        Test.Assert(t.EinaIteratorStrOut(out itr));

        Test.Assert(!itr.Own);
        Test.Assert(!itr.OwnContent);

        int idx = 0;
        foreach (string e in itr)
        {
            Test.AssertEquals(e, base_seq_str[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_str.Length);

        itr.Dispose();

        Test.Assert(t.CheckEinaIteratorStrOut());
    }

    public static void test_eina_iterator_str_out_own()
    {
        test.ITesting t = new test.Testing();
        eina.Iterator<string> itr;

        Test.Assert(t.EinaIteratorStrOutOwn(out itr));

        Test.Assert(itr.Own);
        Test.Assert(itr.OwnContent);

        int idx = 0;
        foreach (string e in itr)
        {
            Test.AssertEquals(e, base_seq_str[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_str.Length);

        itr.Dispose();
    }

    public static void test_eina_iterator_str_return()
    {
        test.ITesting t = new test.Testing();

        var itr = t.EinaIteratorStrReturn();

        Test.Assert(!itr.Own);
        Test.Assert(!itr.OwnContent);

        int idx = 0;
        foreach (string e in itr)
        {
            Test.AssertEquals(e, base_seq_str[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_str.Length);

        itr.Dispose();

        Test.Assert(t.CheckEinaIteratorStrReturn());
    }

    public static void test_eina_iterator_str_return_own()
    {
        test.ITesting t = new test.Testing();

        var itr = t.EinaIteratorStrReturnOwn();

        Test.Assert(itr.Own);
        Test.Assert(itr.OwnContent);

        int idx = 0;
        foreach (string e in itr)
        {
            Test.AssertEquals(e, base_seq_str[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_str.Length);

        itr.Dispose();
    }

    // Object //

    public static void test_eina_iterator_obj_in()
    {
        test.ITesting t = new test.Testing();
        var arr = new eina.Array<test.INumberwrapper>();
        arr.Append(BaseSeqObj());
        var itr = arr.GetIterator();

        Test.Assert(itr.Own);
        Test.Assert(!itr.OwnContent);
        Test.Assert(arr.Own);
        Test.Assert(arr.OwnContent);

        Test.Assert(t.EinaIteratorObjIn(itr));

        Test.Assert(itr.Own);
        Test.Assert(!itr.OwnContent);
        Test.Assert(arr.Own);
        Test.Assert(arr.OwnContent);

        itr.Dispose();
        arr.Dispose();
    }

    public static void test_eina_iterator_obj_in_own()
    {
        test.ITesting t = new test.Testing();
        var arr = new eina.Array<test.INumberwrapper>();
        arr.Append(BaseSeqObj());
        var itr = arr.GetIterator();
        arr.OwnContent = false;
        itr.OwnContent = true;

        Test.Assert(itr.Own);
        Test.Assert(itr.OwnContent);
        Test.Assert(arr.Own);
        Test.Assert(!arr.OwnContent);

        Test.Assert(t.EinaIteratorObjInOwn(itr));

        Test.Assert(!itr.Own);
        Test.Assert(!itr.OwnContent);
        Test.Assert(arr.Own);
        Test.Assert(!arr.OwnContent);

        itr.Dispose();
        arr.Dispose();

        Test.Assert(t.CheckEinaIteratorObjInOwn());
    }

    public static void test_eina_iterator_obj_out()
    {
        test.ITesting t = new test.Testing();
        eina.Iterator<test.INumberwrapper> itr;

        Test.Assert(t.EinaIteratorObjOut(out itr));

        Test.Assert(!itr.Own);
        Test.Assert(!itr.OwnContent);

        var base_seq_obj = BaseSeqObj();

        int idx = 0;
        foreach (test.INumberwrapper e in itr)
        {
            Test.AssertEquals(e.GetNumber(), base_seq_obj[idx].GetNumber());
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_obj.Length);

        itr.Dispose();

        Test.Assert(t.CheckEinaIteratorObjOut());
    }

    public static void test_eina_iterator_obj_out_own()
    {
        test.ITesting t = new test.Testing();
        eina.Iterator<test.INumberwrapper> itr;

        Test.Assert(t.EinaIteratorObjOutOwn(out itr));

        Test.Assert(itr.Own);
        Test.Assert(itr.OwnContent);

        var base_seq_obj = BaseSeqObj();

        int idx = 0;
        foreach (test.INumberwrapper e in itr)
        {
            Test.AssertEquals(e.GetNumber(), base_seq_obj[idx].GetNumber());
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_obj.Length);

        itr.Dispose();
    }

    public static void test_eina_iterator_obj_return()
    {
        test.ITesting t = new test.Testing();

        var itr = t.EinaIteratorObjReturn();

        Test.Assert(!itr.Own);
        Test.Assert(!itr.OwnContent);

        var base_seq_obj = BaseSeqObj();

        int idx = 0;
        foreach (test.INumberwrapper e in itr)
        {
            Test.AssertEquals(e.GetNumber(), base_seq_obj[idx].GetNumber());
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_obj.Length);

        itr.Dispose();

        Test.Assert(t.CheckEinaIteratorObjReturn());
    }

    public static void test_eina_iterator_obj_return_own()
    {
        test.ITesting t = new test.Testing();

        var itr = t.EinaIteratorObjReturnOwn();

        Test.Assert(itr.Own);
        Test.Assert(itr.OwnContent);

        var base_seq_obj = BaseSeqObj();

        int idx = 0;
        foreach (test.INumberwrapper e in itr)
        {
            Test.AssertEquals(e.GetNumber(), base_seq_obj[idx].GetNumber());
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_obj.Length);

        itr.Dispose();
    }
} // < TestEinaIterator

}
