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
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

using Eina;
using EinaTestData;
using static EinaTestData.BaseData;

namespace TestSuite
{

#if EFL_BETA

class TestEinaBinbuf
{
    private static readonly byte[] test_string = System.Text.Encoding.UTF8.GetBytes("0123456789ABCDEF");

    private static readonly byte[] base_seq = BaseSequence.Values();

    public static void eina_binbuf_default()
    {
        var binbuf = new Eina.Binbuf();
        Test.Assert(binbuf.Handle != IntPtr.Zero);
        Test.Assert(binbuf.GetBytes().SequenceEqual(Array.Empty<byte>()));
        binbuf.Dispose();
    }

    public static void eina_binbuf_bytes()
    {
        var binbuf = new Eina.Binbuf(test_string);
        Test.Assert(binbuf.Handle != IntPtr.Zero);
        byte[] cmp = binbuf.GetBytes();
        Test.Assert(cmp != test_string);
        Test.Assert(cmp.SequenceEqual(test_string));
        binbuf.Dispose();
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
        binbuf.Dispose();
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
        binbuf2.Dispose();
        binbuf.Dispose();
    }

    public static void free_get_null_handle()
    {
        var binbuf = new Eina.Binbuf(test_string);
        Test.Assert(binbuf.Handle != IntPtr.Zero);
        binbuf.Free();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
        binbuf.Dispose();
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
        Test.Assert(binbuf.GetBytes().SequenceEqual(Array.Empty<byte>()));
        binbuf.Dispose();
    }

    public static void append_bytes()
    {
        var binbuf = new Eina.Binbuf();
        binbuf.Append(test_string);
        byte[] cmp = binbuf.GetBytes();
        Test.Assert(cmp != test_string);
        Test.Assert(cmp.SequenceEqual(test_string));
        binbuf.Dispose();
    }

    public static void append_bytes_length()
    {
        var binbuf = new Eina.Binbuf();
        binbuf.Append(test_string, 7);
        byte[] cmp = binbuf.GetBytes();
        byte[] expected = System.Text.Encoding.UTF8.GetBytes("0123456");
        Test.Assert(cmp != expected);
        Test.Assert(cmp.SequenceEqual(expected));
        binbuf.Dispose();
    }

    public static void append_binbuf()
    {
        var binbuf = new Eina.Binbuf();
        Test.Assert(binbuf.GetBytes().SequenceEqual(Array.Empty<byte>()));
        var binbuf2 = new Eina.Binbuf(test_string);
        binbuf.Append(binbuf2);
        byte[] cmp = binbuf.GetBytes();
        byte[] cmp2 = binbuf2.GetBytes();
        Test.Assert(cmp != cmp2);
        Test.Assert(cmp2.SequenceEqual(cmp));
        Test.Assert(cmp2.SequenceEqual(test_string));
        binbuf2.Dispose();
        binbuf.Dispose();
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
        binbuf.Dispose();
    }

    public static void remove()
    {
        var binbuf = new Eina.Binbuf(test_string);
        Test.Assert(binbuf.GetBytes().SequenceEqual(test_string));
        binbuf.Remove(2, 9);
        Test.Assert(binbuf.Handle != IntPtr.Zero);
        byte[] expected = System.Text.Encoding.UTF8.GetBytes("019ABCDEF");
        Test.Assert(binbuf.GetBytes().SequenceEqual(expected));
        binbuf.Dispose();
    }

    public static void get_string_native()
    {
        var binbuf = new Eina.Binbuf(test_string);
        Test.Assert(binbuf.GetBytes().SequenceEqual(test_string));
        Test.Assert(binbuf.GetStringNative() != IntPtr.Zero);
        binbuf.Dispose();
    }

    public static void binbuf_free_string()
    {
        var binbuf = new Eina.Binbuf(test_string);
        Test.Assert(binbuf.GetBytes().SequenceEqual(test_string));
        binbuf.FreeString();
        Test.Assert(binbuf.Handle != IntPtr.Zero);
        Test.Assert(binbuf.GetBytes().SequenceEqual(Array.Empty<byte>()));
        binbuf.Dispose();
    }

    public static void binbuf_length()
    {
        var binbuf = new Eina.Binbuf(test_string, 6);
        Test.Assert(binbuf.Length == 6);
        Test.Assert(binbuf.GetBytes().Length == 6);
        binbuf.Dispose();
    }

    public static void test_eina_binbuf_in()
    {
        var t = new Dummy.TestObject();
        var binbuf = new Eina.Binbuf(base_seq, (uint)base_seq.Length);
        Test.Assert(t.EinaBinbufIn(binbuf));
        Test.Assert(binbuf.Own);
        Test.Assert(binbuf.GetBytes().SequenceEqual(
            new byte[]{43, 42, 0x0, 0x2A, 0x42, 33}));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
        t.Dispose();
    }

    public static void test_eina_binbuf_in_own()
    {
        var t = new Dummy.TestObject();
        var binbuf = new Eina.Binbuf(base_seq, (uint)base_seq.Length);
        Test.Assert(t.EinaBinbufInOwn(binbuf));
        Test.Assert(!binbuf.Own);
        Test.Assert(binbuf.GetBytes().SequenceEqual(
            new byte[]{43, 42, 0x0, 0x2A, 0x42, 33}));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
        Test.Assert(t.CheckBinbufInOwn());
        t.Dispose();
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
        t.Dispose();
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
        t.Dispose();
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
        t.Dispose();
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
        t.Dispose();
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
        Test.Assert(binbuf.GetBytes().SequenceEqual(
            new byte[]{43, 42, 0x0, 0x2A, 0x42, 33}));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
        t.Dispose();
    }

    public static void test_inherit_eina_binbuf_in_own()
    {
        var t = new NativeInheritImpl();
        var binbuf = new Eina.Binbuf(base_seq, (uint)base_seq.Length);
        binbuf.Own = false;
        Test.Assert(t.CallEinaBinbufInOwn(binbuf));
        Test.Assert(t.binbuf_in_own_flag);
        Test.Assert(binbuf.GetBytes().SequenceEqual(
            new byte[]{43, 42, 0x0, 0x2A, 0x42, 33}));
        binbuf.Dispose();
        Test.Assert(binbuf.Handle == IntPtr.Zero);
        Test.Assert(t.binbuf_in_own_still_usable());
        t.Dispose();
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
        t.Dispose();
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
        t.Dispose();
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
        t.Dispose();
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
        t.Dispose();
    }
}

#endif

class TestEinaSlice
{
    private static readonly byte[] base_seq = BaseSequence.Values();
    private static readonly GCHandle pinnedData = GCHandle.Alloc(base_seq, GCHandleType.Pinned);
    private static readonly IntPtr pinnedPtr = pinnedData.AddrOfPinnedObject();

#if EFL_BETA
    public static void eina_slice_marshalling()
    {
        var binbuf = new Eina.Binbuf(base_seq);
        Test.Assert(binbuf.Handle != IntPtr.Zero);

        Eina.Slice slc = binbuf.GetSlice();

        Test.Assert(slc.GetBytes().SequenceEqual(base_seq));
        Test.Assert(base_seq.Length == (int)(slc.Len));
        binbuf.Dispose();
    }
#endif

    public static void eina_slice_size()
    {
        Test.Assert(Marshal.SizeOf(typeof(Eina.Slice)) == Marshal.SizeOf(typeof(UIntPtr)) + Marshal.SizeOf(typeof(IntPtr)));
        Test.Assert(Marshal.SizeOf(typeof(Eina.RwSlice)) == Marshal.SizeOf(typeof(UIntPtr)) + Marshal.SizeOf(typeof(IntPtr)));
    }

#if EFL_BETA
    public static void pinned_data_set()
    {
        var binbuf = new Eina.Binbuf();
        binbuf.Append(new Eina.Slice(pinnedPtr, (UIntPtr)3));
        Test.Assert(binbuf.GetBytes().SequenceEqual(base_seq));
        binbuf.Dispose();
    }
#endif

    public static void test_eina_slice_in()
    {
        var t = new Dummy.TestObject();
        var slc = new Eina.Slice(pinnedPtr, (UIntPtr)3);
        Test.Assert(t.EinaSliceIn(slc));
        t.Dispose();
    }

    public static void test_eina_rw_slice_in()
    {
        var rw_seq = base_seq.Clone();
        GCHandle pinnedRWData = GCHandle.Alloc(rw_seq, GCHandleType.Pinned);
        IntPtr ptr = pinnedRWData.AddrOfPinnedObject();

        var slc = new Eina.RwSlice(ptr, (UIntPtr)3);

        var t = new Dummy.TestObject();
        Test.Assert(t.EinaRwSliceIn(slc));

        Test.Assert(slc.GetBytes().SequenceEqual(new byte[3]{0x1, 0x2B, 0x43}));

        pinnedRWData.Free();
        t.Dispose();
    }

    public static void test_eina_slice_out()
    {
        var t = new Dummy.TestObject();
        var slc = new Eina.Slice();
        Test.Assert(t.EinaSliceOut(ref slc));
        Test.Assert(slc.Mem != IntPtr.Zero);
        Test.Assert(slc.Length == base_seq.Length);
        Test.Assert(slc.GetBytes().SequenceEqual(base_seq));
        t.Dispose();
    }

    public static void test_eina_rw_slice_out()
    {
        var t = new Dummy.TestObject();
        var slc = new Eina.RwSlice();
        Test.Assert(t.EinaRwSliceOut(ref slc));
        Test.Assert(slc.Mem != IntPtr.Zero);
        Test.Assert(slc.Length == base_seq.Length);
        Test.Assert(slc.GetBytes().SequenceEqual(base_seq));
        t.Dispose();
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
        t.Dispose();
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
        t.Dispose();
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
        t.Dispose();
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
        t.Dispose();
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
        t.Dispose();
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
        a.Dispose();
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
        a.Dispose();
    }

    public static void push_string()
    {
        var a = new Eina.Array<string>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push("test string §éΨبÿツ"));
        Test.AssertEquals("test string §éΨبÿツ", a[0]);
        a.Dispose();
    }

    public static void push_stringshare()
    {
        var a = new Eina.Array<Eina.Stringshare>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push("test string §éΨبÿツ"));
        Test.AssertEquals("test string §éΨبÿツ", a[0].Str);
        a.Dispose();
    }

    public static void push_obj()
    {
        var a = new Eina.Array<Dummy.Numberwrapper>();
        Test.Assert(a.Handle != IntPtr.Zero);
        var o = new Dummy.Numberwrapper();
        o.Number = 88;
        Test.Assert(a.Push(o));
        Test.Assert(a[0].NativeHandle == o.NativeHandle);
        Test.Assert(a[0].Number == 88);
        o.Dispose();
        a.Dispose();
    }

    public static void pop_int()
    {
        var a = new Eina.Array<int>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push(88));
        Test.Assert(a.Pop() == 88);
        Test.Assert(a.Count() == 0);
        a.Dispose();
    }

    public static void pop_string()
    {
        var a = new Eina.Array<string>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push("test string"));
        Test.Assert(a.Pop() == "test string");
        Test.Assert(a.Count() == 0);
        a.Dispose();
    }

    public static void pop_stringshare()
    {
        var a = new Eina.Array<Eina.Stringshare>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push("test string"));
        Test.Assert(a.Pop() == "test string");
        Test.Assert(a.Count() == 0);
        a.Dispose();
    }

    public static void pop_obj()
    {
        var a = new Eina.Array<Dummy.Numberwrapper>();
        Test.Assert(a.Handle != IntPtr.Zero);
        var o = new Dummy.Numberwrapper();
        o.Number = 88;
        Test.Assert(a.Push(o));
        var p = a.Pop();
        Test.Assert(p.NativeHandle == o.NativeHandle);
        Test.Assert(p.Number == 88);
        Test.Assert(a.Count() == 0);
        o.Dispose();
        a.Dispose();
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
        a.Dispose();
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
        a.Dispose();
    }

    public static void data_set_stringshare()
    {
        var a = new Eina.Array<Eina.Stringshare>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Push("test string"));
        Test.Assert(a[0] == "test string");
        a.DataSet(0, "other string");
        Test.Assert(a[0] == "other string");
        a[0] = "abc";
        Test.Assert(a[0] == "abc");
        a.Dispose();
    }

    public static void data_set_obj()
    {
        var a = new Eina.Array<Dummy.Numberwrapper>();
        Test.Assert(a.Handle != IntPtr.Zero);

        var o1 = new Dummy.Numberwrapper();
        o1.Number = 88;

        Test.Assert(a.Push(o1));
        Test.Assert(a[0].NativeHandle == o1.NativeHandle);
        Test.Assert(a[0].Number == 88);

        var o2 = new Dummy.Numberwrapper();
        o2.Number = 44;

        a.DataSet(0, o2);
        Test.Assert(a[0].NativeHandle == o2.NativeHandle);
        Test.Assert(a[0].Number == 44);

        var o3 = new Dummy.Numberwrapper();
        o3.Number = 22;

        a[0] = o3;
        Test.Assert(a[0].NativeHandle == o3.NativeHandle);
        Test.Assert(a[0].Number == 22);
        o3.Dispose();
        o2.Dispose();
        o1.Dispose();
        a.Dispose();
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
        a.Dispose();
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
        a.Dispose();
    }

    public static void count_stringshare()
    {
        var a = new Eina.Array<Eina.Stringshare>();
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
        a.Dispose();
    }

    public static void count_obj()
    {
        var a = new Eina.Array<Dummy.Numberwrapper>();
        Test.Assert(a.Handle != IntPtr.Zero);

        Test.Assert(a.Count() == 0);

        var o1 = new Dummy.Numberwrapper();
        o1.Number = 88;
        Test.Assert(a.Push(o1));
        Test.Assert(a[0].NativeHandle == o1.NativeHandle);
        Test.Assert(a[0].Number == 88);
        Test.Assert(a.Count() == 1);

        var o2 = new Dummy.Numberwrapper();
        o2.Number = 44;
        Test.Assert(a.Push(o2));
        Test.Assert(a[1].NativeHandle == o2.NativeHandle);
        Test.Assert(a[1].Number == 44);
        Test.Assert(a.Count() == 2);

        var o3 = new Dummy.Numberwrapper();
        o3.Number = 22;
        Test.Assert(a.Push(o3));
        Test.Assert(a[2].NativeHandle == o3.NativeHandle);
        Test.Assert(a[2].Number == 22);
        Test.Assert(a.Count() == 3);

        o3.Dispose();
        o2.Dispose();
        o1.Dispose();
        a.Dispose();
    }

    public static void length_int()
    {
        var a = new Eina.Array<int>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Count == 0);
        Test.Assert(a.Push(88));
        Test.Assert(a[0] == 88);
        Test.Assert(a.Count == 1);
        Test.Assert(a.Push(44));
        Test.Assert(a[1] == 44);
        Test.Assert(a.Count == 2);
        Test.Assert(a.Push(22));
        Test.Assert(a[2] == 22);
        Test.Assert(a.Count == 3);
        a.Dispose();
    }

    public static void length_string()
    {
        var a = new Eina.Array<string>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Count == 0);
        Test.Assert(a.Push("a"));
        Test.Assert(a[0] == "a");
        Test.Assert(a.Count == 1);
        Test.Assert(a.Push("b"));
        Test.Assert(a[1] == "b");
        Test.Assert(a.Count == 2);
        Test.Assert(a.Push("c"));
        Test.Assert(a[2] == "c");
        Test.Assert(a.Count == 3);
        a.Dispose();
    }

    public static void length_stringshare()
    {
        var a = new Eina.Array<Eina.Stringshare>();
        Test.Assert(a.Handle != IntPtr.Zero);
        Test.Assert(a.Count == 0);
        Test.Assert(a.Push("a"));
        Test.Assert(a[0] == "a");
        Test.Assert(a.Count == 1);
        Test.Assert(a.Push("b"));
        Test.Assert(a[1] == "b");
        Test.Assert(a.Count == 2);
        Test.Assert(a.Push("c"));
        Test.Assert(a[2] == "c");
        Test.Assert(a.Count == 3);
        a.Dispose();
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
        a.Dispose();
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
        a.Dispose();
    }

    public static void eina_array_as_ienumerable_stringshare()
    {
        var a = new Eina.Array<Eina.Stringshare>();
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
        a.Dispose();
    }

    public static void eina_array_as_ienumerable_obj()
    {
        var a = new Dummy.Numberwrapper();
        var b = new Dummy.Numberwrapper();
        var c = new Dummy.Numberwrapper();
        a.Number = 88;
        b.Number = 44;
        c.Number = 22;
        var cmp = new Dummy.Numberwrapper[]{a, b, c};

        var arr = new Eina.Array<Dummy.Numberwrapper>();
        Test.Assert(arr.Handle != IntPtr.Zero);
        Test.Assert(arr.Push(a));
        Test.Assert(arr.Push(b));
        Test.Assert(arr.Push(c));

        int i = 0;
        foreach (Dummy.Numberwrapper e in arr)
        {
            Test.AssertEquals(cmp[i].Number, e.Number);
            Test.Assert(cmp[i].NativeHandle == e.NativeHandle);
            ++i;
        }
        arr.Dispose();
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
        t.Dispose();
    }

    public static void test_eina_array_int_in_own()
    {
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<int>();
        arr.Append(base_seq_int);
        Test.Assert(t.EinaArrayIntInOwn(arr));
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(modified_seq_int));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaArrayIntInOwn());
        t.Dispose();
    }

    public static void test_eina_array_int_out()
    {
        var t = new Dummy.TestObject();
        IList<int> arr;
        Test.Assert(t.EinaArrayIntOut(out arr));
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_int));
        Test.Assert(arr.Concat(append_seq_int) != null);
        Test.Assert(t.CheckEinaArrayIntOut());
        t.Dispose();
    }

    public static void test_eina_array_int_out_own()
    {
        var t = new Dummy.TestObject();
        IList<int> arr;
        Test.Assert(t.EinaArrayIntOutOwn(out arr));
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_int));
        Test.Assert(arr.Concat(append_seq_int) != null);
        t.Dispose();
    }

    public static void test_eina_array_int_return()
    {
        var t = new Dummy.TestObject();
        var arr = t.EinaArrayIntReturn();
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_int));
        Test.Assert(arr.Concat(append_seq_int) != null);
        Test.Assert(t.CheckEinaArrayIntReturn());
        t.Dispose();
    }

    public static void test_eina_array_int_return_own()
    {
        var t = new Dummy.TestObject();
        var arr = t.EinaArrayIntReturnOwn();
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_int));
        Test.Assert(arr.Concat(append_seq_int) != null);
        t.Dispose();
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
        t.Dispose();
    }

    public static void test_eina_array_str_in_own()
    {
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<string>();
        arr.Append(base_seq_str);
        Test.Assert(t.EinaArrayStrInOwn(arr));
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(modified_seq_str));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaArrayStrInOwn());
        t.Dispose();
    }

    public static void test_eina_array_str_out()
    {
        var t = new Dummy.TestObject();
        IList<string> arr;
        Test.Assert(t.EinaArrayStrOut(out arr));
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_str));
        Test.Assert(arr.Concat(append_seq_str) != null);
        Test.Assert(t.CheckEinaArrayStrOut());
        t.Dispose();
    }

    public static void test_eina_array_str_out_own()
    {
        var t = new Dummy.TestObject();
        IList<string> arr;
        Test.Assert(t.EinaArrayStrOutOwn(out arr));
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_str));
        Test.Assert(arr.Concat(append_seq_str) != null);
        t.Dispose();
    }

    public static void test_eina_array_str_return()
    {
        var t = new Dummy.TestObject();
        var arr = t.EinaArrayStrReturn();
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_str));
        Test.Assert(arr.Concat(append_seq_str) != null);
        Test.Assert(t.CheckEinaArrayStrReturn());
        t.Dispose();
    }

    public static void test_eina_array_str_return_own()
    {
        var t = new Dummy.TestObject();
        var arr = t.EinaArrayStrReturnOwn();
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_str));
        Test.Assert(arr.Concat(append_seq_str) != null);
        t.Dispose();
    }

    // Eina.Stringshare //
    public static void test_eina_array_strshare_in()
    {
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<Eina.Stringshare>();
        arr.Append(base_seq_strshare);
        Test.Assert(t.EinaArrayStrshareIn(arr));
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(modified_seq_strshare));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        t.Dispose();
    }

    public static void test_eina_array_strshare_in_own()
    {
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<Eina.Stringshare>();
        arr.Append(base_seq_strshare);
        Test.Assert(t.EinaArrayStrshareInOwn(arr));
        Test.Assert(arr.Own);
        Test.Assert(arr.ToArray().SequenceEqual(modified_seq_strshare));
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaArrayStrshareInOwn());
        t.Dispose();
    }

    public static void test_eina_array_strshare_out()
    {
        var t = new Dummy.TestObject();
        IList<Eina.Stringshare> arr;
        Test.Assert(t.EinaArrayStrshareOut(out arr));
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_strshare));
        Test.Assert(arr.Concat(append_seq_strshare) != null);
        Test.Assert(t.CheckEinaArrayStrshareOut());
        t.Dispose();
    }

    public static void test_eina_array_strshare_out_own()
    {
        var t = new Dummy.TestObject();
        IList<Eina.Stringshare> arr;
        Test.Assert(t.EinaArrayStrshareOutOwn(out arr));
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_strshare));
        Test.Assert(arr.Concat(append_seq_strshare) != null);
        t.Dispose();
    }

    public static void test_eina_array_strshare_return()
    {
        var t = new Dummy.TestObject();
        var arr = t.EinaArrayStrshareReturn();
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_strshare));
        Test.Assert(arr.Concat(append_seq_strshare) != null);
        Test.Assert(t.CheckEinaArrayStrshareReturn());
        t.Dispose();
    }

    public static void test_eina_array_strshare_return_own()
    {
        var t = new Dummy.TestObject();
        var arr = t.EinaArrayStrshareReturnOwn();
        Test.Assert(arr.ToArray().SequenceEqual(base_seq_strshare));
        Test.Assert(arr.Concat(append_seq_strshare) != null);
        t.Dispose();
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
        t.Dispose();
    }

    public static void test_eina_array_obj_in_own()
    {
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<Dummy.Numberwrapper>();
        arr.Append(BaseSeqObj());
        Test.Assert(t.EinaArrayObjInOwn(arr));
        Test.Assert(arr.Own);
        NumberwrapperSequenceAssertEqual(arr.ToArray(), ModifiedSeqObj());
        arr.Dispose();
        Test.Assert(arr.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaArrayObjInOwn());
        t.Dispose();
    }

    public static void test_eina_array_obj_out()
    {
        var t = new Dummy.TestObject();
        IList<Dummy.Numberwrapper> arr;
        Test.Assert(t.EinaArrayObjOut(out arr));
        NumberwrapperSequenceAssertEqual(arr.ToArray(), BaseSeqObj());
        Test.Assert(arr.Concat(AppendSeqObj()) != null);
        Test.Assert(t.CheckEinaArrayObjOut());
        t.Dispose();
    }

    public static void test_eina_array_obj_out_own()
    {
        var t = new Dummy.TestObject();
        IList<Dummy.Numberwrapper> arr;
        Test.Assert(t.EinaArrayObjOutOwn(out arr));
        NumberwrapperSequenceAssertEqual(arr.ToArray(), BaseSeqObj());
        Test.Assert(arr.Concat(AppendSeqObj()) != null);
        t.Dispose();
    }

    public static void test_eina_array_obj_return()
    {
        var t = new Dummy.TestObject();
        var arr = t.EinaArrayObjReturn();
        NumberwrapperSequenceAssertEqual(arr.ToArray(), BaseSeqObj());
        Test.Assert(arr.Concat(AppendSeqObj()) != null);
        Test.Assert(t.CheckEinaArrayObjReturn());
        t.Dispose();
    }

    public static void test_eina_array_obj_return_own()
    {
        var t = new Dummy.TestObject();
        var arr = t.EinaArrayObjReturnOwn();
        NumberwrapperSequenceAssertEqual(arr.ToArray(), BaseSeqObj());
        Test.Assert(arr.Concat(AppendSeqObj()) != null);
        t.Dispose();
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
        int len = a.Count;
        for (int i=0; i < len; ++i)
        {
            Test.Assert(a[i].NativeHandle == b[i].NativeHandle);
            Test.Assert(a[i].NativeHandle == cmp[i].NativeHandle);
        }
        a.Dispose();
        t.Dispose();
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
        o.Number = 88;
        Test.Assert(a.Push(o) == 0);
        Test.Assert(a[0].NativeHandle == o.NativeHandle);
        Test.Assert(a[0].Number == 88);
        o.Dispose();
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
        o.Number = 88;
        Test.Assert(a.Push(o) >= 0);
        var p = a.Pop();
        Test.Assert(p.NativeHandle == o.NativeHandle);
        Test.Assert(p.Number == 88);
        Test.Assert(a.Count() == 0);
        o.Dispose();
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
        o1.Number = 88;

        Test.Assert(a.Push(o1) >= 0);
        Test.Assert(a[0].NativeHandle == o1.NativeHandle);
        Test.Assert(a[0].Number == 88);

        var o2 = new Dummy.Numberwrapper();
        o2.Number = 44;

        a.ReplaceAt(0, o2);
        Test.Assert(a[0].NativeHandle == o2.NativeHandle);
        Test.Assert(a[0].Number == 44);
        Test.Assert(a.Count() == 1);

        var o3 = new Dummy.Numberwrapper();
        o3.Number = 22;

        a[0] = o3;
        Test.Assert(a[0].NativeHandle == o3.NativeHandle);
        Test.Assert(a[0].Number == 22);
        Test.Assert(a.Count() == 1);

        o3.Dispose();
        o2.Dispose();
        o1.Dispose();
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
        o1.Number = 88;
        Test.Assert(a.Push(o1) == 0);
        Test.Assert(a[0].NativeHandle == o1.NativeHandle);
        Test.Assert(a[0].Number == 88);
        Test.Assert(a.Count() == 1);

        var o2 = new Dummy.Numberwrapper();
        o2.Number = 44;
        Test.Assert(a.Push(o2) == 1);
        Test.Assert(a[1].NativeHandle == o2.NativeHandle);
        Test.Assert(a[1].Number == 44);
        Test.Assert(a.Count() == 2);

        var o3 = new Dummy.Numberwrapper();
        o3.Number = 22;
        Test.Assert(a.Push(o3) == 2);
        Test.Assert(a[2].NativeHandle == o3.NativeHandle);
        Test.Assert(a[2].Number == 22);
        Test.Assert(a.Count() == 3);

        o3.Dispose();
        o2.Dispose();
        o1.Dispose();
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
        a.Number = 88;
        b.Number = 44;
        c.Number = 22;
        var cmp = new Dummy.Numberwrapper[]{a, b, c};

        var arr = new Eina.Inarray<Dummy.Numberwrapper>();
        Test.Assert(arr.Handle != IntPtr.Zero);
        Test.Assert(arr.Push(a) == 0);
        Test.Assert(arr.Push(b) == 1);
        Test.Assert(arr.Push(c) == 2);

        int i = 0;
        foreach (Dummy.Numberwrapper e in arr)
        {
            Test.AssertEquals(cmp[i].Number, e.Number);
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
        lst.Dispose();
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
        lst.Dispose();
    }

    public static void data_set_stringshare()
    {
        var lst = new Eina.List<Eina.Stringshare>();
        lst.Append("test string");
        Test.Assert(lst[0] == "test string");
        lst.DataSet(0, "other string");
        Test.Assert(lst[0] == "other string");
        lst[0] = "abc";
        Test.Assert(lst[0] == "abc");
        lst.Dispose();
    }

    public static void data_set_obj()
    {
        var lst = new Eina.List<Dummy.Numberwrapper>();

        var o1 = new Dummy.Numberwrapper();
        o1.Number = 88;

        lst.Append(o1);
        Test.Assert(lst[0].NativeHandle == o1.NativeHandle);
        Test.Assert(lst[0].Number == 88);

        var o2 = new Dummy.Numberwrapper();
        o2.Number = 44;

        lst.DataSet(0, o2);
        Test.Assert(lst[0].NativeHandle == o2.NativeHandle);
        Test.Assert(lst[0].Number == 44);

        var o3 = new Dummy.Numberwrapper();
        o3.Number = 22;

        lst[0] = o3;
        Test.Assert(lst[0].NativeHandle == o3.NativeHandle);
        Test.Assert(lst[0].Number == 22);
        o3.Dispose();
        o2.Dispose();
        o1.Dispose();
        lst.Dispose();
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
        lst.Dispose();
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
        lst.Dispose();
    }

    public static void append_count_stringshare()
    {
        var lst = new Eina.List<Eina.Stringshare>();
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
        lst.Dispose();
    }

    public static void append_count_obj()
    {
        var lst = new Eina.List<Dummy.Numberwrapper>();

        Test.Assert(lst.Count() == 0);

        var o1 = new Dummy.Numberwrapper();
        o1.Number = 88;
        lst.Append(o1);
        Test.Assert(lst[0].NativeHandle == o1.NativeHandle);
        Test.Assert(lst[0].Number == 88);
        Test.Assert(lst.Count() == 1);

        var o2 = new Dummy.Numberwrapper();
        o2.Number = 44;
        lst.Append(o2);
        Test.Assert(lst[1].NativeHandle == o2.NativeHandle);
        Test.Assert(lst[1].Number == 44);
        Test.Assert(lst.Count() == 2);

        var o3 = new Dummy.Numberwrapper();
        o3.Number = 22;
        lst.Append(o3);
        Test.Assert(lst[2].NativeHandle == o3.NativeHandle);
        Test.Assert(lst[2].Number == 22);
        Test.Assert(lst.Count() == 3);
        o3.Dispose();
        o2.Dispose();
        o1.Dispose();
        lst.Dispose();
    }

    public static void length_int()
    {
        var lst = new Eina.List<int>();
        Test.Assert(lst.Count == 0);
        lst.Append(88);
        Test.Assert(lst[0] == 88);
        Test.Assert(lst.Count == 1);
        lst.Append(44);
        Test.Assert(lst[1] == 44);
        Test.Assert(lst.Count == 2);
        lst.Append(22);
        Test.Assert(lst[2] == 22);
        Test.Assert(lst.Count == 3);
        lst.Dispose();
    }

    public static void length_string()
    {
        var lst = new Eina.List<string>();
        Test.Assert(lst.Count == 0);
        lst.Append("a");
        Test.Assert(lst[0] == "a");
        Test.Assert(lst.Count == 1);
        lst.Append("b");
        Test.Assert(lst[1] == "b");
        Test.Assert(lst.Count == 2);
        lst.Append("c");
        Test.Assert(lst[2] == "c");
        Test.Assert(lst.Count == 3);
        lst.Dispose();
    }

    public static void length_stringshare()
    {
        var lst = new Eina.List<Eina.Stringshare>();
        Test.Assert(lst.Count == 0);
        lst.Append("a");
        Test.Assert(lst[0] == "a");
        Test.Assert(lst.Count == 1);
        lst.Append("b");
        Test.Assert(lst[1] == "b");
        Test.Assert(lst.Count == 2);
        lst.Append("c");
        Test.Assert(lst[2] == "c");
        Test.Assert(lst.Count == 3);
        lst.Dispose();
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
        lst.Dispose();
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
        lst.Dispose();
    }

    public static void prepend_count_stringshare()
    {
        var lst = new Eina.List<Eina.Stringshare>();
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
        lst.Dispose();
    }

    public static void prepend_count_obj()
    {
        var lst = new Eina.List<Dummy.Numberwrapper>();

        Test.Assert(lst.Count() == 0);

        var o1 = new Dummy.Numberwrapper();
        o1.Number = 88;
        lst.Prepend(o1);
        Test.Assert(lst[0].NativeHandle == o1.NativeHandle);
        Test.Assert(lst[0].Number == 88);
        Test.Assert(lst.Count() == 1);

        var o2 = new Dummy.Numberwrapper();
        o2.Number = 44;
        lst.Prepend(o2);
        Test.Assert(lst[0].NativeHandle == o2.NativeHandle);
        Test.Assert(lst[0].Number == 44);
        Test.Assert(lst.Count() == 2);

        var o3 = new Dummy.Numberwrapper();
        o3.Number = 22;
        lst.Prepend(o3);
        Test.Assert(lst[0].NativeHandle == o3.NativeHandle);
        Test.Assert(lst[0].Number == 22);
        Test.Assert(lst.Count() == 3);

        o3.Dispose();
        o2.Dispose();
        o1.Dispose();
        lst.Dispose();
    }

    public static void sorted_insert_int()
    {
        var lst = new Eina.List<int>();
        lst.SortedInsert(88);
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{88}));
        lst.SortedInsert(22);
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{22, 88}));
        lst.SortedInsert(44);
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{22, 44, 88}));
        lst.Dispose();
    }

    public static void sorted_insert_string()
    {
        var lst = new Eina.List<string>();
        lst.SortedInsert("c");
        Test.Assert(lst.ToArray().SequenceEqual(new string[]{"c"}));
        lst.SortedInsert("a");
        Test.Assert(lst.ToArray().SequenceEqual(new string[]{"a", "c"}));
        lst.SortedInsert("b");
        Test.Assert(lst.ToArray().SequenceEqual(new string[]{"a", "b", "c"}));
        lst.Dispose();
    }

    public static void sorted_insert_stringshare()
    {
        var lst = new Eina.List<Eina.Stringshare>();
        lst.SortedInsert("c");
        Test.Assert(lst.ToArray().SequenceEqual(new Eina.Stringshare[]{"c"}));
        lst.SortedInsert("a");
        Test.Assert(
            lst.ToArray().SequenceEqual(new Eina.Stringshare[]{"a", "c"}));
        lst.SortedInsert("b");
        Test.Assert(
            lst.ToArray().SequenceEqual(new Eina.Stringshare[]{"a", "b", "c"}));
        lst.Dispose();
    }

    public static void sorted_insert_custom_comparer_natural()
    {
        var lst = new Eina.List<int>();
        Eina.List<int>.Compare comparator = (int a, int b) => a - b;

        lst.SortedInsert(comparator, 1);
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{1}));
        lst.SortedInsert(comparator, 2);
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{1, 2}));
        lst.SortedInsert(comparator, 3);
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{1, 2, 3}));
        lst.SortedInsert(comparator, -1);
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{-1, 1, 2, 3}));
        lst.Dispose();
    }

    public static void sorted_insert_custom_comparer_reversed()
    {
        var lst = new Eina.List<int>();
        Eina.List<int>.Compare comparator = (int a, int b) => b - a;

        lst.SortedInsert(comparator, 1);
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{1}));
        lst.SortedInsert(comparator, 2);
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{2, 1}));
        lst.SortedInsert(comparator, 3);
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{3, 2, 1}));
        lst.SortedInsert(comparator, -1);
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{3, 2, 1, -1}));
        lst.Dispose();
    }

    public static void sorted_insert_custom_comparer_string()
    {
        var lst = new Eina.List<string>();
        Eina.List<string>.Compare comparator = (string a, string b) => b.Length - a.Length;
        lst.SortedInsert(comparator, "The");
        Test.Assert(lst.ToArray().SequenceEqual(new string[]{"The"}));
        lst.SortedInsert(comparator, "Quick");
        Test.Assert(lst.ToArray().SequenceEqual(new string[]{"Quick", "The"}));
        lst.SortedInsert(comparator, "Brown");
        Test.Assert(lst.ToArray().SequenceEqual(new string[]{"Brown", "Quick", "The"}));
        lst.SortedInsert(comparator, "Jumped");
        Test.Assert(lst.ToArray().SequenceEqual(
            new string[]{"Jumped", "Brown", "Quick", "The"}));
        lst.Dispose();
    }

    public static void sort_int()
    {
        var lst = new Eina.List<int>();
        lst.Append(88);
        lst.Append(22);
        lst.Append(11);
        lst.Append(44);
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{88, 22, 11, 44}));
        lst.Sort();
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{11, 22, 44, 88}));
        lst.Dispose();
    }

    public static void sort_string()
    {
        var lst = new Eina.List<string>();
        lst.Append("d");
        lst.Append("b");
        lst.Append("a");
        lst.Append("c");
        Test.Assert(
            lst.ToArray().SequenceEqual(new string[]{"d", "b", "a", "c"}));
        lst.Sort();
        Test.Assert(
            lst.ToArray().SequenceEqual(new string[]{"a", "b", "c", "d"}));
        lst.Dispose();
    }

    public static void sort_stringshare()
    {
        var lst = new Eina.List<Eina.Stringshare>();
        lst.Append("d");
        lst.Append("b");
        lst.Append("a");
        lst.Append("c");
        Test.Assert(lst.ToArray().SequenceEqual(
            new Eina.Stringshare[]{"d", "b", "a", "c"}));
        lst.Sort();
        Test.Assert(lst.ToArray().SequenceEqual(
            new Eina.Stringshare[]{"a", "b", "c", "d"}));
        lst.Dispose();
    }

    public static void reverse_int()
    {
        var lst = new Eina.List<int>();
        lst.Append(22);
        lst.Append(44);
        lst.Append(88);
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{22, 44, 88}));
        lst.Reverse();
        Test.Assert(lst.ToArray().SequenceEqual(new int[]{88, 44, 22}));
        lst.Dispose();
    }

    public static void reverse_string()
    {
        var lst = new Eina.List<string>();
        lst.Append("a");
        lst.Append("b");
        lst.Append("c");
        Test.Assert(lst.ToArray().SequenceEqual(new string[]{"a", "b", "c"}));
        lst.Reverse();
        Test.Assert(lst.ToArray().SequenceEqual(new string[]{"c", "b", "a"}));
        lst.Dispose();
    }

    public static void reverse_stringshare()
    {
        var lst = new Eina.List<Eina.Stringshare>();
        lst.Append("a");
        lst.Append("b");
        lst.Append("c");
        Test.Assert(
            lst.ToArray().SequenceEqual(new Eina.Stringshare[]{"a", "b", "c"}));
        lst.Reverse();
        Test.Assert(
            lst.ToArray().SequenceEqual(new Eina.Stringshare[]{"c", "b", "a"}));
        lst.Dispose();
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
        lst.Dispose();
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
        lst.Dispose();
    }

    public static void eina_list_as_ienumerable_stringshare()
    {
        var lst = new Eina.List<Eina.Stringshare>();
        lst.Append("X");
        lst.Append("XX");
        lst.Append("XXX");

        string cmp = "X";
        foreach (string e in lst)
        {
            Test.AssertEquals(cmp, e);
            cmp = cmp + "X";
        }
        lst.Dispose();
    }

    public static void eina_list_as_ienumerable_obj()
    {
        var a = new Dummy.Numberwrapper();
        var b = new Dummy.Numberwrapper();
        var c = new Dummy.Numberwrapper();
        a.Number = 88;
        b.Number = 44;
        c.Number = 22;
        var cmp = new Dummy.Numberwrapper[]{a, b, c};

        var lst = new Eina.List<Dummy.Numberwrapper>();
        lst.Append(a);
        lst.Append(b);
        lst.Append(c);

        int i = 0;
        foreach (Dummy.Numberwrapper e in lst)
        {
            Test.AssertEquals(cmp[i].Number, e.Number);
            Test.Assert(cmp[i].NativeHandle == e.NativeHandle);
            ++i;
        }
        lst.Dispose();
    }

    // //
    // Code Generation
    //

    // Integer //
#if EFL_BETA
    public static void test_eina_list_int_in()
    {
        var t = new Dummy.TestObject();
        var lst = new Eina.List<int>();
        lst.Append(base_seq_int);
        Test.Assert(t.EinaListIntIn(lst));
        Test.Assert(lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_int));
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        t.Dispose();
    }

    public static void test_eina_list_int_in_own()
    {
        var t = new Dummy.TestObject();
        var lst = new Eina.List<int>();
        lst.Append(base_seq_int);
        Test.Assert(t.EinaListIntInOwn(lst));
        Test.Assert(lst.Own);
        Test.Assert(lst.OwnContent);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaListIntInOwn());
        t.Dispose();
    }

    public static void test_eina_list_int_out()
    {
        var t = new Dummy.TestObject();
        IList<int> lst;
        Test.Assert(t.EinaListIntOut(out lst));
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_int));
        Test.Assert(t.CheckEinaListIntOut());
        t.Dispose();
    }

    public static void test_eina_list_int_out_own()
    {
        var t = new Dummy.TestObject();
        IList<int> lst;
        Test.Assert(t.EinaListIntOutOwn(out lst));
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_int));
        lst.Concat(append_seq_int);
        t.Dispose();
    }

    public static void test_eina_list_int_return()
    {
        var t = new Dummy.TestObject();
        var lst = t.EinaListIntReturn();
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_int));
        Test.Assert(t.CheckEinaListIntReturn());
        t.Dispose();
    }

    public static void test_eina_list_int_return_own()
    {
        var t = new Dummy.TestObject();
        var lst = t.EinaListIntReturnOwn();
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_int));
        lst.Concat(append_seq_int);
        t.Dispose();
    }

    // String //
    public static void test_eina_list_str_in()
    {
        var t = new Dummy.TestObject();
        var lst = new Eina.List<string>();
        lst.Append(base_seq_str);
        Test.Assert(t.EinaListStrIn(lst));
        Test.Assert(lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_str));
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        t.Dispose();
    }

    public static void test_eina_list_str_in_own()
    {
        var t = new Dummy.TestObject();
        var lst = new Eina.List<string>();
        lst.Append(base_seq_str);
        Test.Assert(t.EinaListStrInOwn(lst));
        Test.Assert(lst.Own);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaListStrInOwn());
        t.Dispose();
    }

    public static void test_eina_list_str_out()
    {
        var t = new Dummy.TestObject();
        IList<string> lst;
        Test.Assert(t.EinaListStrOut(out lst));
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_str));
        Test.Assert(t.CheckEinaListStrOut());
        t.Dispose();
    }

    public static void test_eina_list_str_out_own()
    {
        var t = new Dummy.TestObject();
        IList<string> lst;
        Test.Assert(t.EinaListStrOutOwn(out lst));
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_str));
        lst.Concat(append_seq_str);
        t.Dispose();
    }

    public static void test_eina_list_str_return()
    {
        var t = new Dummy.TestObject();
        var lst = t.EinaListStrReturn();
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_str));
        Test.Assert(t.CheckEinaListStrReturn());
        t.Dispose();
    }

    public static void test_eina_list_str_return_own()
    {
        var t = new Dummy.TestObject();
        var lst = t.EinaListStrReturnOwn();
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_str));
        lst.Concat(append_seq_str);
        t.Dispose();
    }

    // Eina.Stringshare //
    public static void test_eina_list_strshare_in()
    {
        var t = new Dummy.TestObject();
        var lst = new Eina.List<Eina.Stringshare>();
        lst.Append(base_seq_strshare);
        Test.Assert(t.EinaListStrshareIn(lst));
        Test.Assert(lst.Own);
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_strshare));
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        t.Dispose();
    }

    public static void test_eina_list_strshare_in_own()
    {
        var t = new Dummy.TestObject();
        var lst = new Eina.List<Eina.Stringshare>();
        lst.Append(base_seq_strshare);
        Test.Assert(t.EinaListStrshareInOwn(lst));
        Test.Assert(lst.Own);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaListStrshareInOwn());
        t.Dispose();
    }

    public static void test_eina_list_strshare_out()
    {
        var t = new Dummy.TestObject();
        IList<Eina.Stringshare> lst;
        Test.Assert(t.EinaListStrshareOut(out lst));
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_strshare));
        Test.Assert(t.CheckEinaListStrshareOut());
        t.Dispose();
    }

    public static void test_eina_list_strshare_out_own()
    {
        var t = new Dummy.TestObject();
        IList<Eina.Stringshare> lst;
        Test.Assert(t.EinaListStrshareOutOwn(out lst));
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_strshare));
        lst.Concat(append_seq_strshare);
        t.Dispose();
    }

    public static void test_eina_list_strshare_return()
    {
        var t = new Dummy.TestObject();
        var lst = t.EinaListStrshareReturn();
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_strshare));
        Test.Assert(t.CheckEinaListStrshareReturn());
        t.Dispose();
    }

    public static void test_eina_list_strshare_return_own()
    {
        var t = new Dummy.TestObject();
        var lst = t.EinaListStrshareReturnOwn();
        Test.Assert(lst.ToArray().SequenceEqual(base_seq_strshare));
        lst.Concat(append_seq_strshare);
        t.Dispose();
    }

    // Object //

    public static void test_eina_list_obj_in()
    {
        var t = new Dummy.TestObject();
        var lst = new Eina.List<Dummy.Numberwrapper>();
        lst.Append(BaseSeqObj());
        Test.Assert(t.EinaListObjIn(lst));
        Test.Assert(lst.Own);
        NumberwrapperSequenceAssertEqual(lst.ToArray(), BaseSeqObj());
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        t.Dispose();
    }

    public static void test_eina_list_obj_in_own()
    {
        var t = new Dummy.TestObject();
        var lst = new Eina.List<Dummy.Numberwrapper>();
        lst.Append(BaseSeqObj());
        Test.Assert(t.EinaListObjInOwn(lst));
        Test.Assert(lst.Own);
        lst.Dispose();
        Test.Assert(lst.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaListObjInOwn());
        t.Dispose();
    }

    public static void test_eina_list_obj_out()
    {
        var t = new Dummy.TestObject();
        IList<Dummy.Numberwrapper> lst;
        Test.Assert(t.EinaListObjOut(out lst));
        NumberwrapperSequenceAssertEqual(lst.ToArray(), BaseSeqObj());
        Test.Assert(t.CheckEinaListObjOut());
        t.Dispose();
    }

    public static void test_eina_list_obj_out_own()
    {
        var t = new Dummy.TestObject();
        IList<Dummy.Numberwrapper> lst;
        Test.Assert(t.EinaListObjOutOwn(out lst));
        NumberwrapperSequenceAssertEqual(lst.ToArray(), BaseSeqObj());
        lst.Concat(AppendSeqObj());
        t.Dispose();
    }

    public static void test_eina_list_obj_return()
    {
        var t = new Dummy.TestObject();
        var lst = t.EinaListObjReturn();
        NumberwrapperSequenceAssertEqual(lst.ToArray(), BaseSeqObj());
        Test.Assert(t.CheckEinaListObjReturn());
        t.Dispose();
    }

    public static void test_eina_list_obj_return_own()
    {
        var t = new Dummy.TestObject();
        var lst = t.EinaListObjReturnOwn();
        NumberwrapperSequenceAssertEqual(lst.ToArray(), BaseSeqObj());
        lst.Concat(AppendSeqObj());
        t.Dispose();
    }

    public static void test_eina_list_obj_return_in_same_id()
    {
        var t = new Dummy.TestObject();
        var cmp = BaseSeqObj();
        var a = new Eina.List<Dummy.Numberwrapper>();
        a.Append(cmp);
        var b = t.EinaListObjReturnIn(a);
        NumberwrapperSequenceAssertEqual(a.ToArray(), b.ToArray());
        NumberwrapperSequenceAssertEqual(a.ToArray(), BaseSeqObj());
        int len = a.Count;
        for (int i=0; i < len; ++i)
        {
            Test.Assert(a[i].NativeHandle == b[i].NativeHandle);
            Test.Assert(a[i].NativeHandle == cmp[i].NativeHandle);
        }
        a.Dispose();
        t.Dispose();
    }
#endif
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
        lst.Dispose();
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
        lst.Dispose();
    }

    public static void data_set_obj()
    {
        var lst = new Eina.Inlist<Dummy.Numberwrapper>();

        var o1 = new Dummy.Numberwrapper();
        o1.Number = 88;

        lst.Append(o1);
        Test.Assert(lst[0].NativeHandle == o1.NativeHandle);
        Test.Assert(lst[0].Number == 88);

        var o2 = new Dummy.Numberwrapper();
        o2.Number = 44;

        lst.DataSet(0, o2);
        Test.Assert(lst[0].NativeHandle == o2.NativeHandle);
        Test.Assert(lst[0].Number == 44);

        var o3 = new Dummy.Numberwrapper();
        o3.Number = 22;

        lst[0] = o3;
        Test.Assert(lst[0].NativeHandle == o3.NativeHandle);
        Test.Assert(lst[0].Number == 22);

        Test.Assert(lst.Count() == 1);
        o3.Dispose();
        o2.Dispose();
        o1.Dispose();
        lst.Dispose();
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
        lst.Dispose();
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
        lst.Dispose();
    }

    public static void append_count_obj()
    {
        var lst = new Eina.Inlist<Dummy.Numberwrapper>();

        Test.Assert(lst.Count() == 0);

        var o1 = new Dummy.Numberwrapper();
        o1.Number = 88;
        lst.Append(o1);
        Test.Assert(lst[0].NativeHandle == o1.NativeHandle);
        Test.Assert(lst[0].Number == 88);
        Test.Assert(lst.Count() == 1);

        var o2 = new Dummy.Numberwrapper();
        o2.Number = 44;
        lst.Append(o2);
        Test.Assert(lst[1].NativeHandle == o2.NativeHandle);
        Test.Assert(lst[1].Number == 44);
        Test.Assert(lst.Count() == 2);

        var o3 = new Dummy.Numberwrapper();
        o3.Number = 22;
        lst.Append(o3);
        Test.Assert(lst[2].NativeHandle == o3.NativeHandle);
        Test.Assert(lst[2].Number == 22);
        Test.Assert(lst.Count() == 3);
        o3.Dispose();
        o2.Dispose();
        o1.Dispose();
        lst.Dispose();
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
        lst.Dispose();
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
        lst.Dispose();
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
        lst.Dispose();
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
        lst.Dispose();
    }

    public static void prepend_count_obj()
    {
        var lst = new Eina.Inlist<Dummy.Numberwrapper>();

        Test.Assert(lst.Count() == 0);

        var o1 = new Dummy.Numberwrapper();
        o1.Number = 88;
        lst.Prepend(o1);
        Test.Assert(lst[0].NativeHandle == o1.NativeHandle);
        Test.Assert(lst[0].Number == 88);
        Test.Assert(lst.Count() == 1);

        var o2 = new Dummy.Numberwrapper();
        o2.Number = 44;
        lst.Prepend(o2);
        Test.Assert(lst[0].NativeHandle == o2.NativeHandle);
        Test.Assert(lst[0].Number == 44);
        Test.Assert(lst.Count() == 2);

        var o3 = new Dummy.Numberwrapper();
        o3.Number = 22;
        lst.Prepend(o3);
        Test.Assert(lst[0].NativeHandle == o3.NativeHandle);
        Test.Assert(lst[0].Number == 22);
        Test.Assert(lst.Count() == 3);
        o3.Dispose();
        o2.Dispose();
        o1.Dispose();
        lst.Dispose();
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
        lst.Dispose();
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
        lst.Dispose();
    }

    public static void eina_inlist_as_ienumerable_obj()
    {
        var a = new Dummy.Numberwrapper();
        var b = new Dummy.Numberwrapper();
        var c = new Dummy.Numberwrapper();
        a.Number = 88;
        b.Number = 44;
        c.Number = 22;
        var cmp = new Dummy.Numberwrapper[]{a, b, c};

        var lst = new Eina.Inlist<Dummy.Numberwrapper>();
        lst.Append(a);
        lst.Append(b);
        lst.Append(c);

        int i = 0;
        foreach (Dummy.Numberwrapper e in lst)
        {
            Test.AssertEquals(cmp[i].Number, e.Number);
            Test.Assert(cmp[i].NativeHandle == e.NativeHandle);
            ++i;
        }
        lst.Dispose();
    }

} // < TestEinaInlist

#if EFL_BETA

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

    public static void data_set_strshare()
    {
        var hsh = new Eina.Hash<Eina.Stringshare, Eina.Stringshare>();
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
        Test.Assert(hsh[a].Number == aa.Number);
        Test.Assert(hsh.Count == 1);

        hsh[b] = bb;
        Test.Assert(hsh[b].NativeHandle == bb.NativeHandle);
        Test.Assert(hsh[b].Number == bb.Number);
        Test.Assert(hsh.Count == 2);

        hsh[c] = cc;
        Test.Assert(hsh[c].NativeHandle == cc.NativeHandle);
        Test.Assert(hsh[c].Number == cc.Number);

        Test.Assert(hsh.Count == 3);

        c.Dispose();
        b.Dispose();
        a.Dispose();
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

    public static void eina_hash_as_ienumerable_strshare()
    {
        var hsh = new Eina.Hash<Eina.Stringshare, Eina.Stringshare>();
        var dct = new Dictionary<Eina.Stringshare, Eina.Stringshare>();

        hsh["aa"] = "aaa";
        hsh["bb"] = "bbb";
        hsh["cc"] = "ccc";

        dct["aa"] = "aaa";
        dct["bb"] = "bbb";
        dct["cc"] = "ccc";

        int count = 0;

        foreach (KeyValuePair<Eina.Stringshare, Eina.Stringshare> kvp in hsh)
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

        dct[a.Number] = aa;
        dct[b.Number] = bb;
        dct[c.Number] = cc;

        int count = 0;

        foreach (KeyValuePair<Dummy.Numberwrapper, Dummy.Numberwrapper> kvp in hsh)
        {
            Test.Assert(dct[kvp.Key.Number].NativeHandle == kvp.Value.NativeHandle);
            Test.Assert(dct[kvp.Key.Number].Number == kvp.Value.Number);
            dct.Remove(kvp.Key.Number);
            ++count;
        }

        Test.AssertEquals(count, 3);
        Test.AssertEquals(dct.Count, 0);

        a.Dispose();
        b.Dispose();
        c.Dispose();
        hsh.Dispose();
    }

    // //
    // Code Generation
    //

    // Integer //

    public static void test_eina_hash_int_in()
    {
        var t = new Dummy.TestObject();
        var hsh = new Eina.Hash<int, int>();
        hsh[22] = 222;
        Test.Assert(t.EinaHashIntIn(hsh));
        Test.Assert(hsh.Own);
        Test.Assert(hsh[22] == 222);
        Test.Assert(hsh[44] == 444);
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        t.Dispose();
    }

    /*
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
        t.Dispose();
    }
    */

    public static void test_eina_hash_int_out()
    {
        var t = new Dummy.TestObject();
        Eina.Hash<int, int>hsh;
        Test.Assert(t.EinaHashIntOut(out hsh));
        Test.Assert(!hsh.Own);
        Test.Assert(hsh[22] == 222);
        hsh[44] = 444;
        Test.Assert(hsh[44] == 444);
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashIntOut());
        t.Dispose();
    }

    /*
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
        t.Dispose();
    }
    */

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
        t.Dispose();
    }

    /*
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
        t.Dispose();
    }
    */

    // String //

    public static void test_eina_hash_str_in()
    {
        var t = new Dummy.TestObject();
        var hsh = new Eina.Hash<string, string>();
        hsh["aa"] = "aaa";
        Test.Assert(t.EinaHashStrIn(hsh));
        Test.Assert(hsh.Own);
        Test.Assert(hsh["aa"] == "aaa");
        Test.Assert(hsh["bb"] == "bbb");
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        t.Dispose();
    }

    public static void test_eina_hash_str_in_own()
    {
        var t = new Dummy.TestObject();
        var hsh = new Eina.Hash<string, string>();
        hsh["aa"] = "aaa";
        Test.Assert(t.EinaHashStrInOwn(hsh));
        Test.Assert(!hsh.Own);
        Test.Assert(hsh["aa"] == "aaa");
        Test.Assert(hsh["bb"] == "bbb");
        hsh["cc"] = "ccc";
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashStrInOwn());
        t.Dispose();
    }

    public static void test_eina_hash_str_out()
    {
        var t = new Dummy.TestObject();
        Eina.Hash<string, string>hsh;
        Test.Assert(t.EinaHashStrOut(out hsh));
        Test.Assert(!hsh.Own);
        Test.Assert(hsh["aa"] == "aaa");
        hsh["bb"] = "bbb";
        Test.Assert(hsh["bb"] == "bbb");
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashStrOut());
        t.Dispose();
    }

    public static void test_eina_hash_str_out_own()
    {
        var t = new Dummy.TestObject();
        Eina.Hash<string, string>hsh;
        Test.Assert(t.EinaHashStrOutOwn(out hsh));
        Test.Assert(hsh.Own);
        Test.Assert(hsh["aa"] == "aaa");
        hsh["bb"] = "bbb";
        Test.Assert(hsh["bb"] == "bbb");
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashStrOutOwn());
        t.Dispose();
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
        t.Dispose();
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
        t.Dispose();
    }

    // Eina.Stringshare //

    /*public static void test_eina_hash_strshare_in()
    {
        var t = new Dummy.TestObject();
        var hsh = new Eina.Hash<Eina.Stringshare, Eina.Stringshare>();
        hsh["aa"] = "aaa";
        Test.Assert(t.EinaHashStrshareIn(hsh));
        Test.Assert(hsh.Own);
        Test.Assert(hsh["aa"] == "aaa");
        Test.Assert(hsh["bb"] == "bbb");
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
    }

    public static void test_eina_hash_strshare_in_own()
    {
        var t = new Dummy.TestObject();
        var hsh = new Eina.Hash<Eina.Stringshare, Eina.Stringshare>();
        hsh["aa"] = "aaa";
        Test.Assert(t.EinaHashStrshareInOwn(hsh));
        Test.Assert(!hsh.Own);
        Test.Assert(hsh["aa"] == "aaa");
        Test.Assert(hsh["bb"] == "bbb");
        hsh["cc"] = "ccc";
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashStrshareInOwn());
    }

    public static void test_eina_hash_strshare_out()
    {
        var t = new Dummy.TestObject();
        Eina.Hash<string,string> hsh;
        Test.Assert(t.EinaHashStrshareOut(out hsh));
        Test.Assert(!hsh.Own);
        Test.Assert(hsh["aa"] == "aaa");
        hsh["bb"] = "bbb";
        Test.Assert(hsh["bb"] == "bbb");
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashStrshareOut());
    }

    public static void test_eina_hash_strshare_out_own()
    {
        var t = new Dummy.TestObject();
        Eina.Hash<string,string> hsh;
        Test.Assert(t.EinaHashStrshareOutOwn(out hsh));
        Test.Assert(hsh.Own);
        Test.Assert(hsh["aa"] == "aaa");
        hsh["bb"] = "bbb";
        Test.Assert(hsh["bb"] == "bbb");
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashStrshareOutOwn());
    }

    public static void test_eina_hash_strshare_return()
    {
        var t = new Dummy.TestObject();
        var hsh = t.EinaHashStrshareReturn();
        Test.Assert(!hsh.Own);
        Test.Assert(hsh["aa"] == "aaa");
        hsh["bb"] = "bbb";
        Test.Assert(hsh["bb"] == "bbb");
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashStrshareReturn());
    }

    public static void test_eina_hash_strshare_return_own()
    {
        var t = new Dummy.TestObject();
        var hsh = t.EinaHashStrshareReturnOwn();
        Test.Assert(hsh.Own);
        Test.Assert(hsh["aa"] == "aaa");
        hsh["bb"] = "bbb";
        Test.Assert(hsh["bb"] == "bbb");
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashStrshareReturnOwn());
    }*/

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
        Test.Assert(hsh[nwk1].Number == nwv1.Number);
        Test.Assert(hsh[nwk1].Number == 222);
        Test.Assert(hsh[nwk2].NativeHandle == nwv2.NativeHandle);
        Test.Assert(hsh[nwk2].Number == nwv2.Number);
        Test.Assert(hsh[nwk2].Number == 444);
        nwk1.Dispose();
        nwk2.Dispose();
        nwv1.Dispose();
        nwv2.Dispose();
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        t.Dispose();
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
        Test.Assert(hsh[nwk1].Number == nwv1.Number);
        Test.Assert(hsh[nwk1].Number == 222);
        Test.Assert(hsh[nwk2].NativeHandle == nwv2.NativeHandle);
        Test.Assert(hsh[nwk2].Number == nwv2.Number);
        Test.Assert(hsh[nwk2].Number == 444);
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashObjInOwn(nwk1, nwv1, nwk2, nwv2));
        nwk1.Dispose();
        nwk2.Dispose();
        nwv1.Dispose();
        nwv2.Dispose();
        t.Dispose();
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
        Test.Assert(hsh[nwk1].Number == nwv1.Number);
        Test.Assert(hsh[nwk1].Number == 222);
        Dummy.Numberwrapper nwk2 = NW(44);
        Dummy.Numberwrapper nwv2 = NW(444);
        hsh[nwk2] = nwv2;
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashObjOut(nwk1, nwv1, nwk2, nwv2));
        nwk1.Dispose();
        nwk2.Dispose();
        nwv1.Dispose();
        nwv2.Dispose();
        t.Dispose();
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
        Test.Assert(hsh[nwk1].Number == nwv1.Number);
        Test.Assert(hsh[nwk1].Number == 222);
        Dummy.Numberwrapper nwk2 = NW(44);
        Dummy.Numberwrapper nwv2 = NW(444);
        hsh[nwk2] = nwv2;
        nwk1.Dispose();
        nwk2.Dispose();
        nwv1.Dispose();
        nwv2.Dispose();
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashObjOutOwn());
        t.Dispose();
    }

    public static void test_eina_hash_obj_return()
    {
        var t = new Dummy.TestObject();
        Dummy.Numberwrapper nwk1;
        Dummy.Numberwrapper nwv1;
        var hsh = t.EinaHashObjReturn(out nwk1, out nwv1);
        Test.Assert(!hsh.Own);
        Test.Assert(hsh[nwk1].NativeHandle == nwv1.NativeHandle);
        Test.Assert(hsh[nwk1].Number == nwv1.Number);
        Test.Assert(hsh[nwk1].Number == 222);
        Dummy.Numberwrapper nwk2 = NW(44);
        Dummy.Numberwrapper nwv2 = NW(444);
        hsh[nwk2] = nwv2;
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashObjReturn(nwk1, nwv1, nwk2, nwv2));
        nwk1.Dispose();
        nwk2.Dispose();
        nwv1.Dispose();
        nwv2.Dispose();
        t.Dispose();
    }

    public static void test_eina_hash_obj_return_own()
    {
        var t = new Dummy.TestObject();
        Dummy.Numberwrapper nwk1;
        Dummy.Numberwrapper nwv1;
        var hsh = t.EinaHashObjReturnOwn(out nwk1, out nwv1);
        Test.Assert(hsh.Own);
        Test.Assert(hsh[nwk1].NativeHandle == nwv1.NativeHandle);
        Test.Assert(hsh[nwk1].Number == nwv1.Number);
        Test.Assert(hsh[nwk1].Number == 222);
        Dummy.Numberwrapper nwk2 = NW(44);
        Dummy.Numberwrapper nwv2 = NW(444);
        hsh[nwk2] = nwv2;
        nwk1.Dispose();
        nwk2.Dispose();
        nwv1.Dispose();
        nwv2.Dispose();
        hsh.Dispose();
        Test.Assert(hsh.Handle == IntPtr.Zero);
        Test.Assert(t.CheckEinaHashObjReturnOwn());
        t.Dispose();
    }
}

#endif // EFL_BETA


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

    public static void eina_array_strshare_empty_iterator()
    {
        var arr = new Eina.Array<Eina.Stringshare>();
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

    public static void eina_array_strshare_filled_iterator()
    {
        var arr = new Eina.Array<Eina.Stringshare>();
        arr.Append(base_seq_strshare);
        var itr = arr.GetIterator();
        int idx = 0;
        foreach (string e in itr)
        {
            Test.Assert(e == base_seq_strshare[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_strshare.Length);

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
            Test.Assert(e.Number == base_objs[idx].Number);
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
        lst.Append(base_seq_int);
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
        lst.Append(base_seq_str);
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

    public static void eina_list_strshare_empty_iterator()
    {
        var lst = new Eina.List<Eina.Stringshare>();
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

    public static void eina_list_strshare_filled_iterator()
    {
        var lst = new Eina.List<Eina.Stringshare>();
        lst.Append(base_seq_strshare);
        var itr = lst.GetIterator();
        int idx = 0;
        foreach (string e in itr)
        {
            Test.Assert(e == base_seq_strshare[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_strshare.Length);

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
        lst.Append(base_objs);
        var itr = lst.GetIterator();
        int idx = 0;
        foreach (Dummy.Numberwrapper e in itr)
        {
            Test.Assert(e.NativeHandle == base_objs[idx].NativeHandle);
            Test.Assert(e.Number == base_objs[idx].Number);
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

/*
    public static void eina_hash_keys_strshare_empty_iterator()
    {
        var hsh = new Eina.Hash<Eina.Stringshare, Eina.Stringshare>();
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

    public static void eina_hash_values_strshare_empty_iterator()
    {
        var hsh = new Eina.Hash<Eina.Stringshare, Eina.Stringshare>();
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

    public static void eina_hash_keys_strshare_filled_iterator()
    {
        var hsh = new Eina.Hash<Eina.Stringshare, Eina.Stringshare>();
        var dct = new Dictionary<Eina.Stringshare, bool>();
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

    public static void eina_hash_values_strshare_filled_iterator()
    {
        var hsh = new Eina.Hash<Eina.Stringshare, Eina.Stringshare>();
        var dct = new Dictionary<Eina.Stringshare, bool>();
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
*/

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
        dct[a.Number] = a;
        dct[b.Number] = b;
        dct[c.Number] = c;

        var itr = hsh.Keys();

        int idx = 0;
        foreach (Dummy.Numberwrapper e in itr)
        {
            Test.Assert(dct[e.Number] != null);
            Test.Assert(dct[e.Number].NativeHandle == e.NativeHandle);
            Test.Assert(dct[e.Number].Number == e.Number);
            dct.Remove(e.Number);
            ++idx;
        }
        Test.AssertEquals(dct.Count, 0);
        Test.AssertEquals(idx, 3);

        itr.Dispose();
        dct.Dispose();
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
        dct[aa.Number] = aa;
        dct[bb.Number] = bb;
        dct[cc.Number] = cc;

        var itr = hsh.Values();

        int idx = 0;
        foreach (Dummy.Numberwrapper e in itr)
        {
            Test.Assert(dct[e.Number] != null);
            Test.Assert(dct[e.Number].NativeHandle == e.NativeHandle);
            Test.Assert(dct[e.Number].Number == e.Number);
            dct.Remove(e.Number);
            ++idx;
        }
        Test.AssertEquals(dct.Count, 0);
        Test.AssertEquals(idx, 3);

        itr.Dispose();
        a.Dispose();
        b.Dispose();
        c.Dispose();
        dct.Dispose();
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
        Test.Assert(arr.Own);
        Test.Assert(arr.OwnContent);

        Test.Assert(t.EinaIteratorIntIn(itr));

        Test.Assert(itr.Own);
        Test.Assert(arr.Own);
        Test.Assert(arr.OwnContent);

        itr.Dispose();
        arr.Dispose();
        t.Dispose();
    }

    public static void test_eina_iterator_int_in_own()
    {
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<int>();
        arr.Append(base_seq_int);
        var itr = arr.GetIterator();

        Test.Assert(itr.Own);
        Test.Assert(arr.Own);
        Test.Assert(arr.OwnContent);

        // Will copy the Iterator, owning the copy.
        Test.Assert(t.EinaIteratorIntInOwn(itr));

        Test.Assert(itr.Own);
        Test.Assert(arr.Own);
        // Content must continue to be owned by the array
        Test.Assert(arr.OwnContent);

        itr.Dispose();
        arr.Dispose();

        Test.Assert(t.CheckEinaIteratorIntInOwn());
        t.Dispose();
    }

    public static void test_eina_iterator_int_out()
    {
        var t = new Dummy.TestObject();
        IEnumerable<int> itr;

        Test.Assert(t.EinaIteratorIntOut(out itr));

        int idx = 0;
        foreach (int e in itr)
        {
            Test.AssertEquals(e, base_seq_int[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_int.Length);

        Test.Assert(t.CheckEinaIteratorIntOut());
        t.Dispose();
    }

    public static void test_eina_iterator_int_out_own()
    {
        var t = new Dummy.TestObject();
        IEnumerable<int> itr;

        Test.Assert(t.EinaIteratorIntOutOwn(out itr));

        int idx = 0;
        foreach (int e in itr)
        {
            Test.AssertEquals(e, base_seq_int[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_int.Length);

        t.Dispose();
    }

    public static void test_eina_iterator_int_return()
    {
        var t = new Dummy.TestObject();

        var itr = t.EinaIteratorIntReturn();

        int idx = 0;
        foreach (int e in itr)
        {
            Test.AssertEquals(e, base_seq_int[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_int.Length);

        Test.Assert(t.CheckEinaIteratorIntReturn());
        t.Dispose();
    }

    public static void test_eina_iterator_int_return_own()
    {
        var t = new Dummy.TestObject();

        var itr = t.EinaIteratorIntReturnOwn();

        int idx = 0;
        foreach (int e in itr)
        {
            Test.AssertEquals(e, base_seq_int[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_int.Length);

        t.Dispose();
    }

    // String //

    public static void test_eina_iterator_str_in()
    {
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<string>();
        arr.Append(base_seq_str);
        var itr = arr.GetIterator();

        Test.Assert(itr.Own);
        Test.Assert(arr.Own);
        Test.Assert(arr.OwnContent);

        Test.Assert(t.EinaIteratorStrIn(itr));

        Test.Assert(itr.Own);
        Test.Assert(arr.Own);
        Test.Assert(arr.OwnContent);

        itr.Dispose();
        arr.Dispose();
        t.Dispose();
    }

    public static void test_eina_iterator_str_in_own()
    {
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<string>();
        arr.Append(base_seq_str);
        var itr = arr.GetIterator();

        Test.Assert(itr.Own);
        Test.Assert(arr.Own);
        Test.Assert(arr.OwnContent);

        Test.Assert(t.EinaIteratorStrInOwn(itr));

        Test.Assert(itr.Own);
        Test.Assert(arr.Own);
        Test.Assert(arr.OwnContent);

        itr.Dispose();
        arr.Dispose();

        Test.Assert(t.CheckEinaIteratorStrInOwn());
        t.Dispose();
    }

    public static void test_eina_iterator_str_out()
    {
        var t = new Dummy.TestObject();
        IEnumerable<string> itr;

        Test.Assert(t.EinaIteratorStrOut(out itr));

        int idx = 0;
        foreach (string e in itr)
        {
            Test.AssertEquals(e, base_seq_str[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_str.Length);

        Test.Assert(t.CheckEinaIteratorStrOut());
        t.Dispose();
    }

    public static void test_eina_iterator_str_out_own()
    {
        var t = new Dummy.TestObject();
        IEnumerable<string> itr;

        Test.Assert(t.EinaIteratorStrOutOwn(out itr));

        int idx = 0;
        foreach (string e in itr)
        {
            Test.AssertEquals(e, base_seq_str[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_str.Length);

        t.Dispose();
    }

    public static void test_eina_iterator_str_return()
    {
        var t = new Dummy.TestObject();

        var itr = t.EinaIteratorStrReturn();

        int idx = 0;
        foreach (string e in itr)
        {
            Test.AssertEquals(e, base_seq_str[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_str.Length);

        Test.Assert(t.CheckEinaIteratorStrReturn());
        t.Dispose();
    }

    public static void test_eina_iterator_str_return_own()
    {
        var t = new Dummy.TestObject();

        var itr = t.EinaIteratorStrReturnOwn();

        int idx = 0;
        foreach (string e in itr)
        {
            Test.AssertEquals(e, base_seq_str[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_str.Length);

        t.Dispose();
    }

    // Eina.Stringshare //

    public static void test_eina_iterator_strshare_in()
    {
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<Eina.Stringshare>();
        arr.Append(base_seq_strshare);
        var itr = arr.GetIterator();

        Test.Assert(itr.Own);
        Test.Assert(arr.Own);
        Test.Assert(arr.OwnContent);

        Test.Assert(t.EinaIteratorStrshareIn(itr));

        Test.Assert(itr.Own);
        Test.Assert(arr.Own);
        Test.Assert(arr.OwnContent);

        itr.Dispose();
        arr.Dispose();
        t.Dispose();
    }

    public static void test_eina_iterator_strshare_in_own()
    {
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<Eina.Stringshare>();
        arr.Append(base_seq_strshare);
        var itr = arr.GetIterator();

        Test.Assert(itr.Own);
        Test.Assert(arr.Own);
        Test.Assert(arr.OwnContent);

        Test.Assert(t.EinaIteratorStrshareInOwn(itr));

        // Moving collections currently copy them, should not reflect on managed objects.
        Test.Assert(itr.Own);
        Test.Assert(arr.Own);
        Test.Assert(arr.OwnContent);

        itr.Dispose();
        arr.Dispose();

        Test.Assert(t.CheckEinaIteratorStrshareInOwn());
        t.Dispose();
    }

    public static void test_eina_iterator_strshare_out()
    {
        var t = new Dummy.TestObject();
        IEnumerable<Eina.Stringshare> itr;

        Test.Assert(t.EinaIteratorStrshareOut(out itr));

        int idx = 0;
        foreach (Eina.Stringshare e in itr)
        {
            Test.AssertEquals(e, base_seq_strshare[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_strshare.Length);

        Test.Assert(t.CheckEinaIteratorStrshareOut());
        t.Dispose();
    }

    public static void test_eina_iterator_strshare_out_own()
    {
        var t = new Dummy.TestObject();
        IEnumerable<Eina.Stringshare> itr;

        Test.Assert(t.EinaIteratorStrshareOutOwn(out itr));

        int idx = 0;
        foreach (Eina.Stringshare e in itr)
        {
            Test.AssertEquals(e, base_seq_strshare[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_strshare.Length);

        t.Dispose();
    }

    public static void test_eina_iterator_strshare_return()
    {
        var t = new Dummy.TestObject();

        var itr = t.EinaIteratorStrshareReturn();

        int idx = 0;
        foreach (Eina.Stringshare e in itr)
        {
            Test.AssertEquals(e, base_seq_strshare[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_strshare.Length);

        Test.Assert(t.CheckEinaIteratorStrshareReturn());
        t.Dispose();
    }

    public static void test_eina_iterator_strshare_return_own()
    {
        var t = new Dummy.TestObject();

        var itr = t.EinaIteratorStrshareReturnOwn();

        int idx = 0;
        foreach (Eina.Stringshare e in itr)
        {
            Test.AssertEquals(e, base_seq_strshare[idx]);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_strshare.Length);

        t.Dispose();
    }

    // Object //

    public static void test_eina_iterator_obj_in()
    {
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<Dummy.Numberwrapper>();
        arr.Append(BaseSeqObj());
        var itr = arr.GetIterator();

        Test.Assert(itr.Own);
        Test.Assert(arr.Own);
        Test.Assert(arr.OwnContent);

        Test.Assert(t.EinaIteratorObjIn(itr));

        Test.Assert(itr.Own);
        Test.Assert(arr.Own);
        Test.Assert(arr.OwnContent);

        itr.Dispose();
        arr.Dispose();
        t.Dispose();
    }

    public static void test_eina_iterator_obj_in_own()
    {
        var t = new Dummy.TestObject();
        var arr = new Eina.Array<Dummy.Numberwrapper>();
        arr.Append(BaseSeqObj());
        var itr = arr.GetIterator();

        Test.Assert(itr.Own);
        Test.Assert(arr.Own);
        Test.Assert(arr.OwnContent);

        Test.Assert(t.EinaIteratorObjInOwn(itr));

        Test.Assert(itr.Own);
        Test.Assert(arr.Own);
        Test.Assert(arr.OwnContent);

        itr.Dispose();
        arr.Dispose();

        Test.Assert(t.CheckEinaIteratorObjInOwn());
        t.Dispose();
    }

    public static void test_eina_iterator_obj_out()
    {
        var t = new Dummy.TestObject();
        IEnumerable<Dummy.Numberwrapper> itr;

        Test.Assert(t.EinaIteratorObjOut(out itr));

        var base_seq_obj = BaseSeqObj();

        int idx = 0;
        foreach (Dummy.Numberwrapper e in itr)
        {
            Test.AssertEquals(e.Number, base_seq_obj[idx].Number);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_obj.Length);

        Test.Assert(t.CheckEinaIteratorObjOut());
        t.Dispose();
    }

    public static void test_eina_iterator_obj_out_own()
    {
        var t = new Dummy.TestObject();
        IEnumerable<Dummy.Numberwrapper> itr;

        Test.Assert(t.EinaIteratorObjOutOwn(out itr));

        var base_seq_obj = BaseSeqObj();

        int idx = 0;
        foreach (Dummy.Numberwrapper e in itr)
        {
            Test.AssertEquals(e.Number, base_seq_obj[idx].Number);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_obj.Length);

        t.Dispose();
    }

    public static void test_eina_iterator_obj_return()
    {
        var t = new Dummy.TestObject();

        var itr = t.EinaIteratorObjReturn();

        var base_seq_obj = BaseSeqObj();

        int idx = 0;
        foreach (Dummy.Numberwrapper e in itr)
        {
            Test.AssertEquals(e.Number, base_seq_obj[idx].Number);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_obj.Length);

        Test.Assert(t.CheckEinaIteratorObjReturn());
        t.Dispose();
    }

    public static void test_eina_iterator_obj_return_own()
    {
        var t = new Dummy.TestObject();

        var itr = t.EinaIteratorObjReturnOwn();

        var base_seq_obj = BaseSeqObj();

        int idx = 0;
        foreach (Dummy.Numberwrapper e in itr)
        {
            Test.AssertEquals(e.Number, base_seq_obj[idx].Number);
            ++idx;
        }
        Test.AssertEquals(idx, base_seq_obj.Length);

        t.Dispose();
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

        foreach (Tuple<int, int> pair in zipped)
        {
            Test.AssertEquals(pair.Item1, pair.Item2);
        }

        IEnumerable<int> seq = accessor;
        Test.AssertEquals(seq.Count(), 4);

        lst.Dispose();
    }

    public static void basic_accessor_array()
    {
        var arr = new Eina.Array<string>();
        arr.Append(base_seq_str);

        Eina.Accessor<string> accessor = arr.GetAccessor();

        var zipped = accessor.Zip(arr, (first, second) => new Tuple<string, string>(first, second));

        foreach (Tuple<string, string> pair in zipped)
        {
            Test.AssertEquals(pair.Item1, pair.Item2);
        }

        arr.Dispose();
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

        foreach (Tuple<int, int> pair in zipped)
        {
            Test.AssertEquals(pair.Item1, pair.Item2);
        }
        lst.Dispose();
    }

    public static void basic_accessor_inarray()
    {
        var arr = new Eina.Inarray<int>();
        arr.Append(base_seq_int);

        Eina.Accessor<int> accessor = arr.GetAccessor();

        var zipped = accessor.Zip(arr, (first, second) => new Tuple<int, int>(first, second));

        foreach (Tuple<int, int> pair in zipped)
        {
            Test.AssertEquals(pair.Item1, pair.Item2);
        }
        arr.Dispose();
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
