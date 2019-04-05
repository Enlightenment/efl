using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

using EinaTestData;
using static EinaTestData.BaseData;

namespace TestSuite
{

class TestEinaBinbuf
{
    private static readonly byte[] test_string = System.Text.Encoding.UTF8.GetBytes("0123456789ABCDEF");

    private static readonly byte[] base_seq = BaseSequence.Values();

    public static void eina_binbuf_default()
    {
        var binbuf = new Eina.Binbuf();
        Test.Assert(binbuf.Handle != IntPtr.Zero);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[0]));
    }

    public static void eina_binbuf_bytes()
    {
        var binbuf = new Eina.Binbuf(test_string);
        Test.Assert(binbuf.Handle != IntPtr.Zero);
        byte[] cmp = binbuf.GetBytes();
        Test.Assert(cmp != test_string);
        Test.Assert(cmp.SequenceEqual(test_string));
    }

    public static void eina_binbuf_bytes_length()
    {
        var binbuf = new Eina.Binbuf(test_string, 7);
        Test.Assert(binbuf.Handle != IntPtr.Zero);
        byte[] cmp = binbuf.GetBytes();
        byte[] expected = System.Text.Encoding.UTF8.GetBytes("0123456");
        Test.Assert(cmp != test_string);
        Test.Assert(cmp != expected);
        Test.Assert(cmp.SequenceEqual(expected));
    }

    public static void eina_binbuf_copy_ctor()
    {
        var binbuf = new Eina.Binbuf(test_string);
        Test.Assert(binbuf.Handle != IntPtr.Zero);
        var binbuf2 = new Eina.Binbuf(binbuf);
        Test.Assert(binbuf2.Handle != IntPtr.Zero && binbuf.Handle != binbuf2.Handle);
        byte[] cmp = binbuf.GetBytes();
        byte[] cmp2 = binbuf2.GetBytes();
        Test.Assert(cmp != cmp2);
        Test.Assert(cmp.SequenceEqual(cmp2));
    }

    public static void free_get_null_handle()
    {
        var binbuf = new Eina.Binbuf(test_string);
        Test.Assert(binbuf.Handle != IntPtr.Zero);
        binbuf.Free();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
    }

    public static void reset_get_empty_string()
    {
        var binbuf = new Eina.Binbuf(test_string);
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
        var binbuf = new Eina.Binbuf();
        binbuf.Append(test_string);
        byte[] cmp = binbuf.GetBytes();
        Test.Assert(cmp != test_string);
        Test.Assert(cmp.SequenceEqual(test_string));
    }

    public static void append_bytes_length()
    {
        var binbuf = new Eina.Binbuf();
        binbuf.Append(test_string, 7);
        byte[] cmp = binbuf.GetBytes();
        byte[] expected = System.Text.Encoding.UTF8.GetBytes("0123456");
        Test.Assert(cmp != expected);
        Test.Assert(cmp.SequenceEqual(expected));
    }

    public static void append_binbuf()
    {
        var binbuf = new Eina.Binbuf();
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[0]));
        var binbuf2 = new Eina.Binbuf(test_string);
        binbuf.Append(binbuf2);
        byte[] cmp = binbuf.GetBytes();
        byte[] cmp2 = binbuf2.GetBytes();
        Test.Assert(cmp != cmp2);
        Test.Assert(cmp2.SequenceEqual(cmp));
        Test.Assert(cmp2.SequenceEqual(test_string));
    }

    public static void append_char()
    {
        var binbuf = new Eina.Binbuf();
        binbuf.Append((byte) 0);
        binbuf.Append((byte) 12);
        binbuf.Append((byte) 42);
        byte[] cmp = binbuf.GetBytes();
        Test.Assert(cmp.Length == 3);
        Test.Assert(cmp[0] == 0 && cmp[1] == 12 && cmp[2] == 42);
    }

    public static void remove()
    {
        var binbuf = new Eina.Binbuf(test_string);
        Test.Assert(binbuf.GetBytes().SequenceEqual(test_string));
        binbuf.Remove(2, 9);
        Test.Assert(binbuf.Handle != IntPtr.Zero);
        byte[] expected = System.Text.Encoding.UTF8.GetBytes("019ABCDEF");
        Test.Assert(binbuf.GetBytes().SequenceEqual(expected));
    }

    public static void get_string_native()
    {
        var binbuf = new Eina.Binbuf(test_string);
        Test.Assert(binbuf.GetBytes().SequenceEqual(test_string));
        Test.Assert(binbuf.GetStringNative() != IntPtr.Zero);
    }

    public static void binbuf_free_string()
    {
        var binbuf = new Eina.Binbuf(test_string);
        Test.Assert(binbuf.GetBytes().SequenceEqual(test_string));
        binbuf.FreeString();
        Test.Assert(binbuf.Handle != IntPtr.Zero);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[0]));
    }

    public static void binbuf_length()
    {
        var binbuf = new Eina.Binbuf(test_string, 6);
        Test.Assert(binbuf.Length == 6);
        Test.Assert(binbuf.GetBytes().Length == 6);
    }

    public static void test_eina_binbuf_in()
    {
        var t = new Dummy.TestObject();
        var binbuf = new Eina.Binbuf(base_seq, (uint)base_seq.Length);
        Test.Assert(t.EinaBinbufIn(binbuf));
        Test.Assert(binbuf.Own);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{43,42,0x0,0x2A,0x42,33}));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
    }

    public static void test_eina_binbuf_in_own()
    {
        var t = new Dummy.TestObject();
        var binbuf = new Eina.Binbuf(base_seq, (uint)base_seq.Length);
        Test.Assert(t.EinaBinbufInOwn(binbuf));
        Test.Assert(!binbuf.Own);
        Test.Assert(binbuf.GetBytes().SequenceEqual(new byte[]{43,42,0x0,0x2A,0x42,33}));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
        Test.Assert(t.CheckBinbufInOwn());
    }

    public static void test_eina_binbuf_out()
    {
        var t = new Dummy.TestObject();
        Eina.Binbuf binbuf;
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
        var t = new Dummy.TestObject();
        Eina.Binbuf binbuf;
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
        var t = new Dummy.TestObject();
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
        var t = new Dummy.TestObject();
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
        var binbuf = new Eina.Binbuf(base_seq, (uint)base_seq.Length);
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
        var binbuf = new Eina.Binbuf(base_seq, (uint)base_seq.Length);
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
        Eina.Binbuf binbuf = t.CallEinaBinbufOut();
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
        Eina.Binbuf binbuf = t.CallEinaBinbufOutOwn();
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
        var binbuf = new Eina.Binbuf(base_seq);
        Test.Assert(binbuf.Handle != IntPtr.Zero);

        Eina.Slice slc = Eina.Binbuf.eina_binbuf_slice_get(binbuf.Handle);

        Test.Assert(slc.GetBytes().SequenceEqual(base_seq));
        Test.Assert(base_seq.Length == (int)(slc.Len));
    }

    public static void eina_slice_size()
    {
        Test.Assert(Marshal.SizeOf(typeof(Eina.Slice)) == Marshal.SizeOf(typeof(UIntPtr)) + Marshal.SizeOf(typeof(IntPtr)));
        Test.Assert(Marshal.SizeOf(typeof(Eina.RwSlice)) == Marshal.SizeOf(typeof(UIntPtr)) + Marshal.SizeOf(typeof(IntPtr)));
    }

    public static void pinned_data_set()
    {
        var binbuf = new Eina.Binbuf();
        binbuf.Append(new Eina.Slice().PinnedDataSet(pinnedPtr, (UIntPtr)3));
        Test.Assert(binbuf.GetBytes().SequenceEqual(base_seq));
    }

    public static void test_eina_slice_in()
    {
        var t = new Dummy.TestObject();
        var slc = new Eina.Slice(pinnedPtr, (UIntPtr)3);
        Test.Assert(t.EinaSliceIn(slc));
    }

    public static void test_eina_rw_slice_in()
    {
        var rw_seq = base_seq.Clone();
        GCHandle pinnedRWData = GCHandle.Alloc(rw_seq, GCHandleType.Pinned);
        IntPtr ptr = pinnedRWData.AddrOfPinnedObject();

        var slc = new Eina.RwSlice(ptr, (UIntPtr)3);

        var t = new Dummy.TestObject();
        Test.Assert(t.EinaRwSliceIn(slc));

        Test.Assert(slc.GetBytes().SequenceEqual(new byte[3]{0x1,0x2B,0x43}));

        pinnedRWData.Free();
    }

    public static void test_eina_slice_out()
    {
        var t = new Dummy.TestObject();
        var slc = new Eina.Slice();
        Test.Assert(t.EinaSliceOut(ref slc));
        Test.Assert(slc.Mem != IntPtr.Zero);
        Test.Assert(slc.Length == base_seq.Length);
        Test.Assert(slc.GetBytes().SequenceEqual(base_seq));
    }

    public static void test_eina_rw_slice_out()
    {
        var t = new Dummy.TestObject();
        var slc = new Eina.RwSlice();
        Test.Assert(t.EinaRwSliceOut(ref slc));
        Test.Assert(slc.Mem != IntPtr.Zero);
        Test.Assert(slc.Length == base_seq.Length);
        Test.Assert(slc.GetBytes().SequenceEqual(base_seq));
    }

    public static void test_eina_rw_slice_inout()
    {
        var t = new Dummy.TestObject();
        var rw_seq = new byte[4]{0xA, 0xA, 0xA, 0xA};
        var expected_seq = new byte[4]{0xA, 0xB, 0xC, 0xD};
        var pinnedRWData = GCHandle.Alloc(rw_seq, GCHandleType.Pinned);
        IntPtr ptr = pinnedRWData.AddrOfPinnedObject();
        var slc = new Eina.RwSlice(ptr, (UIntPtr)4);
        Test.Assert(t.EinaRwSliceInout(ref slc));
        Test.Assert(slc.Mem != IntPtr.Zero);
        Test.Assert(slc.Length == rw_seq.Length);
        Test.Assert(slc.GetBytes().SequenceEqual(expected_seq));
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
        var slc = new Eina.Slice(pinnedPtr, (UIntPtr)3);
        Test.Assert(t.EinaSliceIn(slc));
        Test.Assert(t.slice_in_flag);
    }

    public static void test_inherit_eina_rw_slice_in()
    {
        var rw_seq = base_seq.Clone();
        GCHandle pinnedRWData = GCHandle.Alloc(rw_seq, GCHandleType.Pinned);
        IntPtr ptr = pinnedRWData.AddrOfPinnedObject();

        var slc = new Eina.RwSlice(ptr, (UIntPtr)3);

        var t = new NativeInheritImpl();
        Test.Assert(t.EinaRwSliceIn(slc));

        Test.Assert(t.rw_slice_in_flag);
        Test.Assert(slc.GetBytes().SequenceEqual(base_seq));

        pinnedRWData.Free();
    }

    public static void test_inherit_eina_slice_out()
    {
        var t = new NativeInheritImpl();
        var slc = new Eina.Slice();
        Test.Assert(t.EinaSliceOut(ref slc));
        Test.Assert(t.slice_out_flag);
        Test.Assert(slc.Mem != IntPtr.Zero);
        Test.Assert(slc.Length == base_seq.Length);
        Test.Assert(slc.GetBytes().SequenceEqual(base_seq));
    }

    public static void test_inherit_eina_rw_slice_out()
    {
        var t = new NativeInheritImpl();
        var slc = new Eina.RwSlice();
        Test.Assert(t.EinaRwSliceOut(ref slc));
        Test.Assert(t.rw_slice_out_flag);
        Test.Assert(slc.Mem != IntPtr.Zero);
        Test.Assert(slc.Length == base_seq.Length);
        Test.Assert(slc.GetBytes().SequenceEqual(base_seq));
    }
}

class TestEinaArray
{
    public static void SetUp()
    {
        Dummy.TestObject.CreateCmpArrayObjects();
    }

    public static void TearDown()
    {
        Dummy.TestObject.DestroyCmpArrayObjects();
    }

    public static void eina_array_default()
    {
        var a = new Eina.Array<int>();
        Test.Assert(a.Handle != IntPtr.Zero);
    }

    public static void create_array_from_null()
    {
        Test.AssertRaises<ArgumentNullException>(() => new Eina.Array<int>(IntPtr.Zero, false));
        Test.AssertRaises<ArgumentNullException>(() => new Eina.Array<int>(IntPtr.Zero, false, false));
    }

    public static void push_int()
    {
        var a = new Eina.Array<int>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push(88));
        Test.Assert(a[0] == 88);
    }

    public static void push_string()
    {
        var a = new Eina.Array<string>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push("test string §éΨبÿツ"));
        Test.AssertEquals("test string §éΨبÿツ", a[0]);
    }

    public static void push_obj()
    {
        var a = new Eina.Array<Dummy.Numberwrapper>();
        Test.Assert(a.Handle != IntPtr.Zero);
        var o = new Dummy.Numberwrapper();
        o.SetNumber(88);
        Test.Assert(a.Push(o));
        Test.Assert(a[0].NativeHandle == o.NativeHandle);
        Test.Assert(a[0].GetNumber() == 88);
    }

    public static void pop_int()
    {
        var a = new Eina.Array<int>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push(88));
        Test.Assert(a.Pop() == 88);
        Test.Assert(a.Count() == 0);
    }

    public static void pop_string()
    {
        var a = new Eina.Array<string>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push("test string"));
        Test.Assert(a.Pop() == "test string");
        Test.Assert(a.Count() == 0);
    }

    public static void pop_obj()
    {
        var a = new Eina.Array<Dummy.Numberwrapper>();
        Test.Assert(a.Handle != IntPtr.Zero);
        var o = new Dummy.Numberwrapper();
        o.SetNumber(88);
        Test.Assert(a.Push(o));
        var p = a.Pop();
        Test.Assert(p.NativeHandle == o.NativeHandle);
        Test.Assert(p.GetNumber() == 88);
        Test.Assert(a.Count() == 0);
    }

    public static void data_set_int()
    {
        var a = new Eina.Array<int>();
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
        var a = new Eina.Array<string>();
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
        var a = new Eina.Array<Dummy.Numberwrapper>();
        Test.Assert(a.Handle != IntPtr.Zero);

        var o1 = new Dummy.Numberwrapper();
        o1.SetNumber(88);

        Test.Assert(a.Push(o1));
        Test.Assert(a[0].NativeHandle == o1.NativeHandle);
        Test.Assert(a[0].GetNumber() == 88);

        var o2 = new Dummy.Numberwrapper();
        o2.SetNumber(44);

        a.DataSet(0, o2);
        Test.Assert(a[0].NativeHandle == o2.NativeHandle);
        Test.Assert(a[0].GetNumber() == 44);

        var o3 = new Dummy.Numberwrapper();
        o3.SetNumber(22);

        a[0] = o3;
        Test.Assert(a[0].NativeHandle == o3.NativeHandle);
        Test.Assert(a[0].GetNumber() == 22);
    }

    public static void count_int()
    {
        var a = new Eina.Array<int>();
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
        var a = new Eina.Array<string>();
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
        var a = new Eina.Array<Dummy.Numberwrapper>();
        Test.Assert(a.Handle != IntPtr.Zero);

        Test.Assert(a.Count() == 0);

        var o1 = new Dummy.Numberwrapper();
        o1.SetNumber(88);
        Test.Assert(a.Push(o1));
        Test.Assert(a[0].NativeHandle == o1.NativeHandle);
        Test.Assert(a[0].GetNumber() == 88);
        Test.Assert(a.Count() == 1);

        var o2 = new Dummy.Numberwrapper();
        o2.SetNumber(44);
        Test.Assert(a.Push(o2));
        Test.Assert(a[1].NativeHandle == o2.NativeHandle);
        Test.Assert(a[1].GetNumber() == 44);
        Test.Assert(a.Count() == 2);

        var o3 = new Dummy.Numberwrapper();
        o3.SetNumber(22);
        Test.Assert(a.Push(o3));
        Test.Assert(a[2].NativeHandle == o3.NativeHandle);
        Test.Assert(a[2].GetNumber() == 22);
        Test.Assert(a.Count() == 3);
    }

    public static void length_int()
    {
        var a = new Eina.Array<int>();
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
        var a = new Eina.Array<string>();
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
        var a = new Eina.Array<int>();
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
        var a = new Eina.Array<string>();
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
        var a = new Dummy.Numberwrapper();
        var b = new Dummy.Numberwrapper();
        var c = new Dummy.Numberwrapper();
        a.SetNumber(88);
        b.SetNumber(44);
        c.SetNumber(22);
        var cmp = new Dummy.Numberwrapper[]{a,b,c};

        var arr = new Eina.Array<Dummy.Numberwrapper>();
        Test.Assert(arr.Handle != IntPtr.Zero);
        Test.Assert(arr.Push(a));
        Test.Assert(arr.Push(b));
        Test.Assert(arr.Push(c));

        int i = 0;
        foreach (Dummy.Numberwrapper e in arr)
        {
            Test.AssertEquals(cmp[i].GetNumber(), e.GetNumber());
            Test.Assert(cmp[i].NativeHandle == e.NativeHandle);
            ++i;
        }
    }

    // //
    // Code Generation
    //

    // Integer //

    public static void test_eina_array_int_in()
    {
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<int>();
        arr.Append(base_seq_int);
        Test.Assert(t.EinaArrayIntIn(arr));
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(modified_seq_int));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_array_int_in_own()
    {
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<int>();
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
        var t = new Dummy.TestObject();
        Eina.Array<int> arr;
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
        var t = new Dummy.TestObject();
        Eina.Array<int> arr;
        Test.Assert(t.EinaArrayIntOutOwn(out arr));
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_int));
        Test.Assert(arr.Append(append_seq_int));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_array_int_return()
    {
        var t = new Dummy.TestObject();
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
        var t = new Dummy.TestObject();
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
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<string>();
        arr.Append(base_seq_str);
        Test.Assert(t.EinaArrayStrIn(arr));
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(modified_seq_str));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_array_str_in_own()
    {
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<string>();
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
        var t = new Dummy.TestObject();
        Eina.Array<string> arr;
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
        var t = new Dummy.TestObject();
        Eina.Array<string> arr;
        Test.Assert(t.EinaArrayStrOutOwn(out arr));
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_str));
        Test.Assert(arr.Append(append_seq_str));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_array_str_return()
    {
        var t = new Dummy.TestObject();
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
        var t = new Dummy.TestObject();
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
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<Dummy.Numberwrapper>();
        arr.Append(BaseSeqObj());
        Test.Assert(t.EinaArrayObjIn(arr));
        Test.Assert(arr.Own);
        NumberwrapperSequenceAssertEqual(arr.ToArray(), ModifiedSeqObj());
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_array_obj_in_own()
    {
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<Dummy.Numberwrapper>();
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
        var t = new Dummy.TestObject();
        Eina.Array<Dummy.Numberwrapper> arr;
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
        var t = new Dummy.TestObject();
        Eina.Array<Dummy.Numberwrapper> arr;
        Test.Assert(t.EinaArrayObjOutOwn(out arr));
        Test.Assert(arr.Own);
        NumberwrapperSequenceAssertEqual(arr.ToArray(), BaseSeqObj());
        Test.Assert(arr.Append(AppendSeqObj()));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_array_obj_return()
    {
        var t = new Dummy.TestObject();
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
        var t = new Dummy.TestObject();
        var arr = t.EinaArrayObjReturnOwn();
        Test.Assert(arr.Own);
        NumberwrapperSequenceAssertEqual(arr.ToArray(), BaseSeqObj());
        Test.Assert(arr.Append(AppendSeqObj()));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
    }

    public static void test_eina_array_obj_return_in_same_id()
    {
        var t = new Dummy.TestObject();
        var cmp = BaseSeqObj();
        var a = new Eina.Array<Dummy.Numberwrapper>();
        a.Append(cmp);
        var b = t.EinaArrayObjReturnIn(a);
        NumberwrapperSequenceAssertEqual(a.ToArray(), b.ToArray());
        NumberwrapperSequenceAssertEqual(a.ToArray(), BaseSeqObj());
        int len = a.Length;
        for (int i=0; i < len; ++i)
        {
            Test.Assert(a[i].NativeHandle == b[i].NativeHandle);
            Test.Assert(a[i].NativeHandle == cmp[i].NativeHandle);
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
        var a = new Eina.Inarray<int>();
        Test.Assert(a.Handle != IntPtr.Zero);
        a.Dispose();
    }

    public static void push_int()
    {
        var a = new Eina.Inarray<int>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push(88) == 0);
        Test.Assert(a[0] == 88);
        a.Dispose();
    }

    public static void push_string()
    {
        var a = new Eina.Inarray<string>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push("test string") == 0);
        Test.Assert(a[0] == "test string");
        a.Dispose();
    }

    public static void push_obj()
    {
        var a = new Eina.Inarray<Dummy.Numberwrapper>();
        Test.Assert(a.Handle != IntPtr.Zero);
        var o = new Dummy.Numberwrapper();
        o.SetNumber(88);
        Test.Assert(a.Push(o) == 0);
        Test.Assert(a[0].NativeHandle == o.NativeHandle);
        Test.Assert(a[0].GetNumber() == 88);
        a.Dispose();
    }

    public static void pop_int()
    {
        var a = new Eina.Inarray<int>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push(88) >= 0);
        Test.Assert(a.Pop() == 88);
        Test.Assert(a.Count() == 0);
        a.Dispose();
    }

    public static void pop_string()
    {
        var a = new Eina.Inarray<string>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push("test string") >= 0);
        Test.Assert(a.Pop() == "test string");
        Test.Assert(a.Count() == 0);
        a.Dispose();
    }

    public static void pop_obj()
    {
        var a = new Eina.Inarray<Dummy.Numberwrapper>();
        Test.Assert(a.Handle != IntPtr.Zero);
        var o = new Dummy.Numberwrapper();
        o.SetNumber(88);
        Test.Assert(a.Push(o) >= 0);
        var p = a.Pop();
        Test.Assert(p.NativeHandle == o.NativeHandle);
        Test.Assert(p.GetNumber() == 88);
        Test.Assert(a.Count() == 0);
        a.Dispose();
    }

    public static void replace_at_int()
    {
        var a = new Eina.Inarray<int>();
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
        var a = new Eina.Inarray<string>();
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
        var a = new Eina.Inarray<Dummy.Numberwrapper>();
        Test.Assert(a.Handle != IntPtr.Zero);

        var o1 = new Dummy.Numberwrapper();
        o1.SetNumber(88);

        Test.Assert(a.Push(o1) >= 0);
        Test.Assert(a[0].NativeHandle == o1.NativeHandle);
        Test.Assert(a[0].GetNumber() == 88);

        var o2 = new Dummy.Numberwrapper();
        o2.SetNumber(44);

        a.ReplaceAt(0, o2);
        Test.Assert(a[0].NativeHandle == o2.NativeHandle);
        Test.Assert(a[0].GetNumber() == 44);
        Test.Assert(a.Count() == 1);

        var o3 = new Dummy.Numberwrapper();
        o3.SetNumber(22);

        a[0] = o3;
        Test.Assert(a[0].NativeHandle == o3.NativeHandle);
        Test.Assert(a[0].GetNumber() == 22);
        Test.Assert(a.Count() == 1);

        a.Dispose();
    }

    public static void count_int()
    {
        var a = new Eina.Inarray<int>();
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
        var a = new Eina.Inarray<string>();
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
        var a = new Eina.Inarray<Dummy.Numberwrapper>();
        Test.Assert(a.Handle != IntPtr.Zero);

        Test.Assert(a.Count() == 0);

        var o1 = new Dummy.Numberwrapper();
        o1.SetNumber(88);
        Test.Assert(a.Push(o1) == 0);
        Test.Assert(a[0].NativeHandle == o1.NativeHandle);
        Test.Assert(a[0].GetNumber() == 88);
        Test.Assert(a.Count() == 1);

        var o2 = new Dummy.Numberwrapper();
        o2.SetNumber(44);
        Test.Assert(a.Push(o2) == 1);
        Test.Assert(a[1].NativeHandle == o2.NativeHandle);
        Test.Assert(a[1].GetNumber() == 44);
        Test.Assert(a.Count() == 2);

        var o3 = new Dummy.Numberwrapper();
        o3.SetNumber(22);
        Test.Assert(a.Push(o3) == 2);
        Test.Assert(a[2].NativeHandle == o3.NativeHandle);
        Test.Assert(a[2].GetNumber() == 22);
        Test.Assert(a.Count() == 3);

        a.Dispose();
    }

    public static void length_int()
    {
        var a = new Eina.Inarray<int>();
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
        var a = new Eina.Inarray<string>();
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
        var a = new Eina.Inarray<int>();
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
        var a = new Eina.Inarray<string>();
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
        var a = new Dummy.Numberwrapper();
        var b = new Dummy.Numberwrapper();
        var c = new Dummy.Numberwrapper();
        a.SetNumber(88);
        b.SetNumber(44);
        c.SetNumber(22);
        var cmp = new Dummy.Numberwrapper[]{a,b,c};

        var arr = new Eina.Inarray<Dummy.Numberwrapper>();
        Test.Assert(arr.Handle != IntPtr.Zero);
        Test.Assert(arr.Push(a) == 0);
        Test.Assert(arr.Push(b) == 1);
        Test.Assert(arr.Push(c) == 2);

        int i = 0;
        foreach (Dummy.Numberwrapper e in arr)
        {
            Test.AssertEquals(cmp[i].GetNumber(), e.GetNumber());
            Test.Assert(cmp[i].NativeHandle == e.NativeHandle);
            ++i;
        }
        arr.Dispose();
    }

}

class TestEinaList
{
    public static void SetUp()
    {
        Dummy.TestObject.CreateCmpArrayObjects();
    }

    public static void TearDown()
    {
        Dummy.TestObject.DestroyCmpArrayObjects();
    }

    public static void data_set_int()
    {
        var lst = new Eina.List<int>();
        lst.Append(88);
        Test.Assert(lst[0] == 88);
        lst.DataSet(0, 44);
        Test.Assert(lst[0] == 44);
        lst[0] = 22;
        Test.Assert(lst[0] == 22);
    }

    public static void data_set_string()
    {
        var lst = new Eina.List<string>();
        lst.Append("test string");
        Test.Assert(lst[0] == "test string");
        lst.DataSet(0, "other string");
        Test.Assert(lst[0] == "other string");
        lst[0] = "abc";
        Test.Assert(lst[0] == "abc");
    }

    public static void data_set_obj()
    {
        var lst = new Eina.List<Dummy.Numberwrapper>();

        var o1 = new Dummy.Numberwrapper();
        o1.SetNumber(88);

        lst.Append(o1);
        Test.Assert(lst[0].NativeHandle == o1.NativeHandle);
        Test.Assert(lst[0].GetNumber() == 88);

        var o2 = new Dummy.Numberwrapper();
        o2.SetNumber(44);

        lst.DataSet(0, o2);
        Test.Assert(lst[0].NativeHandle == o2.NativeHandle);
        Test.Assert(lst[0].GetNumber() == 44);

        var o3 = new Dummy.Numberwrapper();
        o3.SetNumber(22);

        lst[0] = o3;
        Test.Assert(lst[0].NativeHandle == o3.NativeHandle);
        Test.Assert(lst[0].GetNumber() == 22);
    }

    public static void append_count_int()
    {
        var lst = new Eina.List<int>();
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
        var lst = new Eina.List<string>();
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
        var lst = new Eina.List<Dummy.Numberwrapper>();

        Test.Assert(lst.Count() == 0);

        var o1 = new Dummy.Numberwrapper();
        o1.SetNumber(88);
        lst.Append(o1);
        Test.Assert(lst[0].NativeHandle == o1.NativeHandle);
        Test.Assert(lst[0].GetNumber() == 88);
        Test.Assert(lst.Count() == 1);

        var o2 = new Dummy.Numberwrapper();
        o2.SetNumber(44);
        lst.Append(o2);
        Test.Assert(lst[1].NativeHandle == o2.NativeHandle);
        Test.Assert(lst[1].GetNumber() == 44);
        Test.Assert(lst.Count() == 2);

        var o3 = new Dummy.Numberwrapper();
        o3.SetNumber(22);
        lst.Append(o3);
        Test.Assert(lst[2].NativeHandle == o3.NativeHandle);
        Test.Assert(lst[2].GetNumber() == 22);
        Test.Assert(lst.Count() == 3);
    }

    public static void length_int()
    {
        var lst = new Eina.List<int>();
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
        var lst = new Eina.List<string>();
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
        var lst = new Eina.List<int>();
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
        var lst = new Eina.List<string>();
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
        var lst = new Eina.List<Dummy.Numberwrapper>();

        Test.Assert(lst.Count() == 0);

        var o1 = new Dummy.Numberwrapper();
        o1.SetNumber(88);
        lst.Prepend(o1);
        Test.Assert(lst[0].NativeHandle == o1.NativeHandle);
        Test.Assert(lst[0].GetNumber() == 88);
        Test.Assert(lst.Count() == 1);

        var o2 = new Dummy.Numberwrapper();
        o2.SetNumber(44);
        lst.Prepend(o2);
        Test.Assert(lst[0].NativeHandle == o2.NativeHandle);
        Test.Assert(lst[0].GetNumber() == 44);
        Test.Assert(lst.Count() == 2);

        var o3 = new Dummy.Numberwrapper();
        o3.SetNumber(22);
        lst.Prepend(o3);
        Test.Assert(lst[0].NativeHandle == o3.NativeHandle);
        Test.Assert(lst[0].GetNumber() == 22);
        Test.Assert(lst.Count() == 3);
    }

    public static void sorted_insert_int()
    {
        var lst = new Eina.List<int>();
        lst.SortedInsert(88);
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{88}));
        lst.SortedInsert(22);
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{22,88}));
        lst.SortedInsert(44);
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{22,44,88}));

    }

    public static void sorted_insert_string()
    {
        var lst = new Eina.List<string>();
        lst.SortedInsert("c");
        Test.Assert(lst.ToArray().SequenceEqual(new string[]{"c"}));
        lst.SortedInsert("a");
        Test.Assert(lst.ToArray().SequenceEqual(new string[]{"a","c"}));
        lst.SortedInsert("b");
        Test.Assert(lst.ToArray().SequenceEqual(new string[]{"a","b","c"}));

    }

    public static void sort_int()
    {
        var lst = new Eina.List<int>();
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
        var lst = new Eina.List<string>();
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
        var lst = new Eina.List<int>();
        lst.Append(22);
        lst.Append(44);
        lst.Append(88);
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{22,44,88}));
        lst.Reverse();
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{88,44,22}));

    }

    public static void reverse_string()
    {
        var lst = new Eina.List<string>();
        lst.Append("a");
        lst.Append("b");
        lst.Append("c");
        Test.Assert(lst.ToArray().SequenceEqual(new string[]{"a","b","c"}));
        lst.Reverse();
        Test.Assert(lst.ToArray().SequenceEqual(new string[]{"c","b","a"}));
    }

    public static void eina_list_as_ienumerable_int()
    {
        var lst = new Eina.List<int>();
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
        var lst = new Eina.List<string>();
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
        var a = new Dummy.Numberwrapper();
        var b = new Dummy.Numberwrapper();
        var c = new Dummy.Numberwrapper();
        a.SetNumber(88);
        b.SetNumber(44);
        c.SetNumber(22);
        var cmp = new Dummy.Numberwrapper[]{a,b,c};

        var lst = new Eina.List<Dummy.Numberwrapper>();
        lst.Append(a);
        lst.Append(b);
        lst.Append(c);

        int i = 0;
        foreach (Dummy.Numberwrapper e in lst)
        {
            Test.AssertEquals(cmp[i].GetNumber(), e.GetNumber());
            Test.Assert(cmp[i].NativeHandle == e.NativeHandle);
            ++i;
        }
    }

    // //
    // Code Generation
    //

    // Integer //

    public static void test_eina_list_int_in()
    {
        var t = new Dummy.TestObject();
        var lst = new Eina.List<int>();
        lst.AppendArray(base_seq_int);
        Test.Assert(t.EinaListIntIn(lst));
        Test.Assert(lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_int));
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    public static void test_eina_list_int_in_own()
    {
        var t = new Dummy.TestObject();
        var lst = new Eina.List<int>();
        lst.AppendArray(base_seq_int);
        Test.Assert(t.EinaListIntInOwn(lst));
        Test.Assert(!lst.Own);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaListIntInOwn());
    }

    public static void test_eina_list_int_out()
    {
        var t = new Dummy.TestObject();
        Eina.List<int> lst;
        Test.Assert(t.EinaListIntOut(out lst));
        Test.Assert(!lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_int));
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaListIntOut());
    }

    public static void test_eina_list_int_out_own()
    {
        var t = new Dummy.TestObject();
        Eina.List<int> lst;
        Test.Assert(t.EinaListIntOutOwn(out lst));
        Test.Assert(lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_int));
        lst.AppendArray(append_seq_int);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    public static void test_eina_list_int_return()
    {
        var t = new Dummy.TestObject();
        var lst = t.EinaListIntReturn();
        Test.Assert(!lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_int));
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaListIntReturn());
    }

    public static void test_eina_list_int_return_own()
    {
        var t = new Dummy.TestObject();
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
        var t = new Dummy.TestObject();
        var lst = new Eina.List<string>();
        lst.AppendArray(base_seq_str);
        Test.Assert(t.EinaListStrIn(lst));
        Test.Assert(lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_str));
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    public static void test_eina_list_str_in_own()
    {
        var t = new Dummy.TestObject();
        var lst = new Eina.List<string>();
        lst.AppendArray(base_seq_str);
        Test.Assert(t.EinaListStrInOwn(lst));
        Test.Assert(!lst.Own);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaListStrInOwn());
    }

    public static void test_eina_list_str_out()
    {
        var t = new Dummy.TestObject();
        Eina.List<string> lst;
        Test.Assert(t.EinaListStrOut(out lst));
        Test.Assert(!lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_str));
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaListStrOut());
    }

    public static void test_eina_list_str_out_own()
    {
        var t = new Dummy.TestObject();
        Eina.List<string> lst;
        Test.Assert(t.EinaListStrOutOwn(out lst));
        Test.Assert(lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_str));
        lst.AppendArray(append_seq_str);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    public static void test_eina_list_str_return()
    {
        var t = new Dummy.TestObject();
        var lst = t.EinaListStrReturn();
        Test.Assert(!lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_str));
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaListStrReturn());
    }

    public static void test_eina_list_str_return_own()
    {
        var t = new Dummy.TestObject();
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
        var t = new Dummy.TestObject();
        var lst = new Eina.List<Dummy.Numberwrapper>();
        lst.AppendArray(BaseSeqObj());
        Test.Assert(t.EinaListObjIn(lst));
        Test.Assert(lst.Own);
        NumberwrapperSequenceAssertEqual(lst.ToArray(), BaseSeqObj());
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    public static void test_eina_list_obj_in_own()
    {
        var t = new Dummy.TestObject();
        var lst = new Eina.List<Dummy.Numberwrapper>();
        lst.AppendArray(BaseSeqObj());
        Test.Assert(t.EinaListObjInOwn(lst));
        Test.Assert(!lst.Own);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaListObjInOwn());
    }

    public static void test_eina_list_obj_out()
    {
        var t = new Dummy.TestObject();
        Eina.List<Dummy.Numberwrapper> lst;
        Test.Assert(t.EinaListObjOut(out lst));
        Test.Assert(!lst.Own);
        NumberwrapperSequenceAssertEqual(lst.ToArray(), BaseSeqObj());
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaListObjOut());
    }

    public static void test_eina_list_obj_out_own()
    {
        var t = new Dummy.TestObject();
        Eina.List<Dummy.Numberwrapper> lst;
        Test.Assert(t.EinaListObjOutOwn(out lst));
        Test.Assert(lst.Own);
        NumberwrapperSequenceAssertEqual(lst.ToArray(), BaseSeqObj());
        lst.AppendArray(AppendSeqObj());
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    public static void test_eina_list_obj_return()
    {
        var t = new Dummy.TestObject();
        var lst = t.EinaListObjReturn();
        Test.Assert(!lst.Own);
        NumberwrapperSequenceAssertEqual(lst.ToArray(), BaseSeqObj());
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaListObjReturn());
    }

    public static void test_eina_list_obj_return_own()
    {
        var t = new Dummy.TestObject();
        var lst = t.EinaListObjReturnOwn();
        Test.Assert(lst.Own);
        NumberwrapperSequenceAssertEqual(lst.ToArray(), BaseSeqObj());
        lst.AppendArray(AppendSeqObj());
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
    }

    public static void test_eina_list_obj_return_in_same_id()
    {
        var t = new Dummy.TestObject();
        var cmp = BaseSeqObj();
        var a = new Eina.List<Dummy.Numberwrapper>();
        a.AppendArray(cmp);
        var b = t.EinaListObjReturnIn(a);
        NumberwrapperSequenceAssertEqual(a.ToArray(), b.ToArray());
        NumberwrapperSequenceAssertEqual(a.ToArray(), BaseSeqObj());
        int len = a.Length;
        for (int i=0; i < len; ++i)
        {
            Test.Assert(a[i].NativeHandle == b[i].NativeHandle);
            Test.Assert(a[i].NativeHandle == cmp[i].NativeHandle);
        }
    }
}

class TestEinaInlist
{
    public static void data_set_int()
    {
        var lst = new Eina.Inlist<int>();
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
        var lst = new Eina.Inlist<string>();
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
        var lst = new Eina.Inlist<Dummy.Numberwrapper>();

        var o1 = new Dummy.Numberwrapper();
        o1.SetNumber(88);

        lst.Append(o1);
        Test.Assert(lst[0].NativeHandle == o1.NativeHandle);
        Test.Assert(lst[0].GetNumber() == 88);

        var o2 = new Dummy.Numberwrapper();
        o2.SetNumber(44);

        lst.DataSet(0, o2);
        Test.Assert(lst[0].NativeHandle == o2.NativeHandle);
        Test.Assert(lst[0].GetNumber() == 44);

        var o3 = new Dummy.Numberwrapper();
        o3.SetNumber(22);

        lst[0] = o3;
        Test.Assert(lst[0].NativeHandle == o3.NativeHandle);
        Test.Assert(lst[0].GetNumber() == 22);

        Test.Assert(lst.Count() == 1);
    }

    public static void append_count_int()
    {
        var lst = new Eina.Inlist<int>();
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
        var lst = new Eina.Inlist<string>();
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
        var lst = new Eina.Inlist<Dummy.Numberwrapper>();

        Test.Assert(lst.Count() == 0);

        var o1 = new Dummy.Numberwrapper();
        o1.SetNumber(88);
        lst.Append(o1);
        Test.Assert(lst[0].NativeHandle == o1.NativeHandle);
        Test.Assert(lst[0].GetNumber() == 88);
        Test.Assert(lst.Count() == 1);

        var o2 = new Dummy.Numberwrapper();
        o2.SetNumber(44);
        lst.Append(o2);
        Test.Assert(lst[1].NativeHandle == o2.NativeHandle);
        Test.Assert(lst[1].GetNumber() == 44);
        Test.Assert(lst.Count() == 2);

        var o3 = new Dummy.Numberwrapper();
        o3.SetNumber(22);
        lst.Append(o3);
        Test.Assert(lst[2].NativeHandle == o3.NativeHandle);
        Test.Assert(lst[2].GetNumber() == 22);
        Test.Assert(lst.Count() == 3);
    }

    public static void length_int()
    {
        var lst = new Eina.Inlist<int>();
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
        var lst = new Eina.Inlist<string>();
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
        var lst = new Eina.Inlist<int>();
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
        var lst = new Eina.Inlist<string>();
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
        var lst = new Eina.Inlist<Dummy.Numberwrapper>();

        Test.Assert(lst.Count() == 0);

        var o1 = new Dummy.Numberwrapper();
        o1.SetNumber(88);
        lst.Prepend(o1);
        Test.Assert(lst[0].NativeHandle == o1.NativeHandle);
        Test.Assert(lst[0].GetNumber() == 88);
        Test.Assert(lst.Count() == 1);

        var o2 = new Dummy.Numberwrapper();
        o2.SetNumber(44);
        lst.Prepend(o2);
        Test.Assert(lst[0].NativeHandle == o2.NativeHandle);
        Test.Assert(lst[0].GetNumber() == 44);
        Test.Assert(lst.Count() == 2);

        var o3 = new Dummy.Numberwrapper();
        o3.SetNumber(22);
        lst.Prepend(o3);
        Test.Assert(lst[0].NativeHandle == o3.NativeHandle);
        Test.Assert(lst[0].GetNumber() == 22);
        Test.Assert(lst.Count() == 3);
    }

    public static void eina_inlist_as_ienumerable_int()
    {
        var lst = new Eina.Inlist<int>();
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
        var lst = new Eina.Inlist<string>();
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
        var a = new Dummy.Numberwrapper();
        var b = new Dummy.Numberwrapper();
        var c = new Dummy.Numberwrapper();
        a.SetNumber(88);
        b.SetNumber(44);
        c.SetNumber(22);
        var cmp = new Dummy.Numberwrapper[]{a,b,c};

        var lst = new Eina.Inlist<Dummy.Numberwrapper>();
        lst.Append(a);
        lst.Append(b);
        lst.Append(c);

        int i = 0;
        foreach (Dummy.Numberwrapper e in lst)
        {
            Test.AssertEquals(cmp[i].GetNumber(), e.GetNumber());
            Test.Assert(cmp[i].NativeHandle == e.NativeHandle);
            ++i;
        }
    }

} // < TestEinaInlist


class TestEinaHash
{
    public static void data_set_int()
    {
        var hsh = new Eina.Hash<int, int>();
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
        var hsh = new Eina.Hash<string, string>();
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
        var hsh = new Eina.Hash<Dummy.Numberwrapper, Dummy.Numberwrapper>();
        Test.Assert(hsh.Count == 0);

        var a = NW(22);
        var aa = NW(222);
        var b = NW(44);
        var bb = NW(444);
        var c = NW(88);
        var cc = NW(888);

        hsh[a] = aa;
        Test.Assert(hsh[a].NativeHandle == aa.NativeHandle);
        Test.Assert(hsh[a].GetNumber() == aa.GetNumber());
        Test.Assert(hsh.Count == 1);

        hsh[b] = bb;
        Test.Assert(hsh[b].NativeHandle == bb.NativeHandle);
        Test.Assert(hsh[b].GetNumber() == bb.GetNumber());
        Test.Assert(hsh.Count == 2);

        hsh[c] = cc;
        Test.Assert(hsh[c].NativeHandle == cc.NativeHandle);
        Test.Assert(hsh[c].GetNumber() == cc.GetNumber());

        Test.Assert(hsh.Count == 3);

        hsh.Dispose();
    }

    public static void eina_hash_as_ienumerable_int()
    {
        var hsh = new Eina.Hash<int, int>();
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
        var hsh = new Eina.Hash<string, string>();
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
        var hsh = new Eina.Hash<Dummy.Numberwrapper, Dummy.Numberwrapper>();
        var dct = new Dictionary<int, Dummy.Numberwrapper>();

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

        foreach (KeyValuePair<Dummy.Numberwrapper, Dummy.Numberwrapper> kvp in hsh)
        {
            Test.Assert(dct[kvp.Key.GetNumber()].NativeHandle == kvp.Value.NativeHandle);
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
        var t = new Dummy.TestObject();
        var hsh = new Eina.Hash<int,int>();
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
        var t = new Dummy.TestObject();
        var hsh = new Eina.Hash<int,int>();
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
        var t = new Dummy.TestObject();
        Eina.Hash<int,int> hsh;
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
        var t = new Dummy.TestObject();
        Eina.Hash<int,int> hsh;
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
        var t = new Dummy.TestObject();
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
        var t = new Dummy.TestObject();
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
        var t = new Dummy.TestObject();
        var hsh = new Eina.Hash<string,string>();
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
        var t = new Dummy.TestObject();
        var hsh = new Eina.Hash<string,string>();
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
        var t = new Dummy.TestObject();
        Eina.Hash<string,string> hsh;
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
        var t = new Dummy.TestObject();
        Eina.Hash<string,string> hsh;
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
        var t = new Dummy.TestObject();
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
        var t = new Dummy.TestObject();
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
        var t = new Dummy.TestObject();
        var hsh = new Eina.Hash<Dummy.Numberwrapper, Dummy.Numberwrapper>();
        var nwk1 = NW(22);
        var nwv1 = NW(222);
        hsh[nwk1] = nwv1;
        Dummy.Numberwrapper nwk2;
        Dummy.Numberwrapper nwv2;
        Test.Assert(t.EinaHashObjIn(hsh, nwk1, nwv1, out nwk2, out nwv2));
        Test.Assert(hsh.Own);
        Test.Assert(hsh[nwk1].NativeHandle == nwv1.NativeHandle);
        Test.Assert(hsh[nwk1].GetNumber() == nwv1.GetNumber());
        Test.Assert(hsh[nwk1].GetNumber() == 222);
        Test.Assert(hsh[nwk2].NativeHandle == nwv2.NativeHandle);
        Test.Assert(hsh[nwk2].GetNumber() == nwv2.GetNumber());
        Test.Assert(hsh[nwk2].GetNumber() == 444);
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
    }

    public static void test_eina_hash_obj_in_own()
    {
        var t = new Dummy.TestObject();
        var hsh = new Eina.Hash<Dummy.Numberwrapper, Dummy.Numberwrapper>();
        var nwk1 = NW(22);
        var nwv1 = NW(222);
        hsh[nwk1] = nwv1;
        Dummy.Numberwrapper nwk2;
        Dummy.Numberwrapper nwv2;
        Test.Assert(t.EinaHashObjInOwn(hsh, nwk1, nwv1, out nwk2, out nwv2));
        Test.Assert(!hsh.Own);
        Test.Assert(hsh[nwk1].NativeHandle == nwv1.NativeHandle);
        Test.Assert(hsh[nwk1].GetNumber() == nwv1.GetNumber());
        Test.Assert(hsh[nwk1].GetNumber() == 222);
        Test.Assert(hsh[nwk2].NativeHandle == nwv2.NativeHandle);
        Test.Assert(hsh[nwk2].GetNumber() == nwv2.GetNumber());
        Test.Assert(hsh[nwk2].GetNumber() == 444);
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashObjInOwn(nwk1, nwv1, nwk2, nwv2));
    }

    public static void test_eina_hash_obj_out()
    {
        var t = new Dummy.TestObject();
        Eina.Hash<Dummy.Numberwrapper, Dummy.Numberwrapper> hsh;
        Dummy.Numberwrapper nwk1;
        Dummy.Numberwrapper nwv1;
        Test.Assert(t.EinaHashObjOut(out hsh, out nwk1, out nwv1));
        Test.Assert(!hsh.Own);
        Test.Assert(hsh[nwk1].NativeHandle == nwv1.NativeHandle);
        Test.Assert(hsh[nwk1].GetNumber() == nwv1.GetNumber());
        Test.Assert(hsh[nwk1].GetNumber() == 222);
        Dummy.Numberwrapper nwk2 = NW(44);
        Dummy.Numberwrapper nwv2 = NW(444);
        hsh[nwk2] = nwv2;
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashObjOut(nwk1, nwv1, nwk2, nwv2));
    }

    public static void test_eina_hash_obj_out_own()
    {
        var t = new Dummy.TestObject();
        Eina.Hash<Dummy.Numberwrapper, Dummy.Numberwrapper> hsh;
        Dummy.Numberwrapper nwk1;
        Dummy.Numberwrapper nwv1;
        Test.Assert(t.EinaHashObjOutOwn(out hsh, out nwk1, out nwv1));
        Test.Assert(hsh.Own);
        Test.Assert(hsh[nwk1].NativeHandle == nwv1.NativeHandle);
        Test.Assert(hsh[nwk1].GetNumber() == nwv1.GetNumber());
        Test.Assert(hsh[nwk1].GetNumber() == 222);
        Dummy.Numberwrapper nwk2 = NW(44);
        Dummy.Numberwrapper nwv2 = NW(444);
        hsh[nwk2] = nwv2;
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashObjOutOwn());
    }

    public static void test_eina_hash_obj_return()
    {
        var t = new Dummy.TestObject();
        Dummy.Numberwrapper nwk1;
        Dummy.Numberwrapper nwv1;
        var hsh = t.EinaHashObjReturn(out nwk1, out nwv1);
        Test.Assert(!hsh.Own);
        Test.Assert(hsh[nwk1].NativeHandle == nwv1.NativeHandle);
        Test.Assert(hsh[nwk1].GetNumber() == nwv1.GetNumber());
        Test.Assert(hsh[nwk1].GetNumber() == 222);
        Dummy.Numberwrapper nwk2 = NW(44);
        Dummy.Numberwrapper nwv2 = NW(444);
        hsh[nwk2] = nwv2;
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashObjReturn(nwk1, nwv1, nwk2, nwv2));
    }

    public static void test_eina_hash_obj_return_own()
    {
        var t = new Dummy.TestObject();
        Dummy.Numberwrapper nwk1;
        Dummy.Numberwrapper nwv1;
        var hsh = t.EinaHashObjReturnOwn(out nwk1, out nwv1);
        Test.Assert(hsh.Own);
        Test.Assert(hsh[nwk1].NativeHandle == nwv1.NativeHandle);
        Test.Assert(hsh[nwk1].GetNumber() == nwv1.GetNumber());
        Test.Assert(hsh[nwk1].GetNumber() == 222);
        Dummy.Numberwrapper nwk2 = NW(44);
        Dummy.Numberwrapper nwv2 = NW(444);
        hsh[nwk2] = nwv2;
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashObjReturnOwn());
    }
}


class TestEinaIterator
{
    public static void SetUp()
    {
        Dummy.TestObject.CreateCmpArrayObjects();
    }

    public static void TearDown()
    {
        Dummy.TestObject.DestroyCmpArrayObjects();
    }

    // Array //

    public static void eina_array_int_empty_iterator()
    {
        var arr = new Eina.Array<int>();
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
        var arr = new Eina.Array<int>();
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
        var arr = new Eina.Array<string>();
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
        var arr = new Eina.Array<string>();
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
        var arr = new Eina.Array<Dummy.Numberwrapper>();
        var itr = arr.GetIterator();
        int idx = 0;
        foreach (Dummy.Numberwrapper e in itr)
        {
            ++idx;
        }
        Test.AssertEquals(idx, 0);

        itr.Dispose();
        arr.Dispose();
    }

    public static void eina_array_obj_filled_iterator()
    {
        var arr = new Eina.Array<Dummy.Numberwrapper>();
        var base_objs = BaseSeqObj();
        arr.Append(base_objs);
        var itr = arr.GetIterator();
        int idx = 0;
        foreach (Dummy.Numberwrapper e in itr)
        {
            Test.Assert(e.NativeHandle == base_objs[idx].NativeHandle);
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
        var arr = new Eina.Inarray<int>();
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
        var arr = new Eina.Inarray<int>();
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
        var lst = new Eina.List<int>();
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
        var lst = new Eina.List<int>();
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
        var lst = new Eina.List<string>();
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
        var lst = new Eina.List<string>();
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
        var lst = new Eina.List<Dummy.Numberwrapper>();
        var itr = lst.GetIterator();
        int idx = 0;
        foreach (Dummy.Numberwrapper e in itr)
        {
            ++idx;
        }
        Test.AssertEquals(idx, 0);

        itr.Dispose();
        lst.Dispose();
    }

    public static void eina_list_obj_filled_iterator()
    {
        var lst = new Eina.List<Dummy.Numberwrapper>();
        var base_objs = BaseSeqObj();
        lst.AppendArray(base_objs);
        var itr = lst.GetIterator();
        int idx = 0;
        foreach (Dummy.Numberwrapper e in itr)
        {
            Test.Assert(e.NativeHandle == base_objs[idx].NativeHandle);
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
        var lst = new Eina.Inlist<int>();
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
        var lst = new Eina.Inlist<int>();
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
        var hsh = new Eina.Hash<int, int>();
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
        var hsh = new Eina.Hash<int, int>();
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
        var hsh = new Eina.Hash<int, int>();
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
        var hsh = new Eina.Hash<int, int>();
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
        var hsh = new Eina.Hash<string, string>();
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
        var hsh = new Eina.Hash<string, string>();
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
        var hsh = new Eina.Hash<string, string>();
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
        var hsh = new Eina.Hash<string, string>();
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
        var hsh = new Eina.Hash<Dummy.Numberwrapper, Dummy.Numberwrapper>();
        var itr = hsh.Keys();
        bool entered = false;
        foreach (Dummy.Numberwrapper e in itr)
        {
            entered = true;
        }
        Test.Assert(!entered);

        itr.Dispose();
        hsh.Dispose();
    }

    public static void eina_hash_values_obj_empty_iterator()
    {
        var hsh = new Eina.Hash<Dummy.Numberwrapper, Dummy.Numberwrapper>();
        var itr = hsh.Values();
        bool entered = false;
        foreach (Dummy.Numberwrapper e in itr)
        {
            entered = true;
        }
        Test.Assert(!entered);

        itr.Dispose();
        hsh.Dispose();
    }

    public static void eina_hash_keys_obj_filled_iterator()
    {
        var hsh = new Eina.Hash<Dummy.Numberwrapper, Dummy.Numberwrapper>();
        var dct = new Eina.Hash<int, Dummy.Numberwrapper>();
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
        foreach (Dummy.Numberwrapper e in itr)
        {
            Test.Assert(dct[e.GetNumber()] != null);
            Test.Assert(dct[e.GetNumber()].NativeHandle == e.NativeHandle);
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
        var hsh = new Eina.Hash<Dummy.Numberwrapper, Dummy.Numberwrapper>();
        var dct = new Eina.Hash<int, Dummy.Numberwrapper>();
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
        foreach (Dummy.Numberwrapper e in itr)
        {
            Test.Assert(dct[e.GetNumber()] != null);
            Test.Assert(dct[e.GetNumber()].NativeHandle == e.NativeHandle);
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
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<int>();
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
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<int>();
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
        var t = new Dummy.TestObject();
        Eina.Iterator<int> itr;

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
        var t = new Dummy.TestObject();
        Eina.Iterator<int> itr;

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
        var t = new Dummy.TestObject();

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
        var t = new Dummy.TestObject();

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
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<string>();
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
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<string>();
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
        var t = new Dummy.TestObject();
        Eina.Iterator<string> itr;

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
        var t = new Dummy.TestObject();
        Eina.Iterator<string> itr;

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
        var t = new Dummy.TestObject();

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
        var t = new Dummy.TestObject();

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
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<Dummy.Numberwrapper>();
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
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<Dummy.Numberwrapper>();
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
        var t = new Dummy.TestObject();
        Eina.Iterator<Dummy.Numberwrapper> itr;

        Test.Assert(t.EinaIteratorObjOut(out itr));

        Test.Assert(!itr.Own);
        Test.Assert(!itr.OwnContent);

        var base_seq_obj = BaseSeqObj();

        int idx = 0;
        foreach (Dummy.Numberwrapper e in itr)
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
        var t = new Dummy.TestObject();
        Eina.Iterator<Dummy.Numberwrapper> itr;

        Test.Assert(t.EinaIteratorObjOutOwn(out itr));

        Test.Assert(itr.Own);
        Test.Assert(itr.OwnContent);

        var base_seq_obj = BaseSeqObj();

        int idx = 0;
        foreach (Dummy.Numberwrapper e in itr)
        {
            Test.AssertEquals(e.GetNumber(), base_seq_obj[idx].GetNumber());
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_obj.Length);

        itr.Dispose();
    }

    public static void test_eina_iterator_obj_return()
    {
        var t = new Dummy.TestObject();

        var itr = t.EinaIteratorObjReturn();

        Test.Assert(!itr.Own);
        Test.Assert(!itr.OwnContent);

        var base_seq_obj = BaseSeqObj();

        int idx = 0;
        foreach (Dummy.Numberwrapper e in itr)
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
        var t = new Dummy.TestObject();

        var itr = t.EinaIteratorObjReturnOwn();

        Test.Assert(itr.Own);
        Test.Assert(itr.OwnContent);

        var base_seq_obj = BaseSeqObj();

        int idx = 0;
        foreach (Dummy.Numberwrapper e in itr)
        {
            Test.AssertEquals(e.GetNumber(), base_seq_obj[idx].GetNumber());
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_obj.Length);

        itr.Dispose();
    }
} // < TestEinaIterator


class TestEinaAccessor
{
    public static void basic_accessor_list()
    {
        var lst = new Eina.List<int>();
        lst.Append(1);
        lst.Append(2);
        lst.Append(3);
        lst.Append(4);

        Eina.Accessor<int> accessor = lst.GetAccessor();

        var zipped = accessor.Zip(lst, (first, second) => new Tuple<int, int>(first, second));

        foreach(Tuple<int, int> pair in zipped)
        {
            Test.AssertEquals(pair.Item1, pair.Item2);
        }
    }

    public static void basic_accessor_array()
    {
        var arr = new Eina.Array<string>();
        arr.Append(base_seq_str);

        Eina.Accessor<string> accessor = arr.GetAccessor();

        var zipped = accessor.Zip(arr, (first, second) => new Tuple<string, string>(first, second));

        foreach(Tuple<string, string> pair in zipped)
        {
            Test.AssertEquals(pair.Item1, pair.Item2);
        }
    }

    public static void basic_accessor_inlist()
    {
        var lst = new Eina.Inlist<int>();
        lst.Append(1);
        lst.Append(2);
        lst.Append(3);
        lst.Append(4);

        Eina.Accessor<int> accessor = lst.GetAccessor();

        var zipped = accessor.Zip(lst, (first, second) => new Tuple<int, int>(first, second));

        foreach(Tuple<int, int> pair in zipped)
        {
            Test.AssertEquals(pair.Item1, pair.Item2);
        }
    }

    public static void basic_accessor_inarray()
    {
        var arr = new Eina.Inarray<int>();
        arr.Append(base_seq_int);

        Eina.Accessor<int> accessor = arr.GetAccessor();

        var zipped = accessor.Zip(arr, (first, second) => new Tuple<int, int>(first, second));

        foreach(Tuple<int, int> pair in zipped)
        {
            Test.AssertEquals(pair.Item1, pair.Item2);
        }
    }
}

class TestEinaLog
{
    public static void basic_log_usage()
    {
        Eina.Log.Error("This should work");
    }
}

}
