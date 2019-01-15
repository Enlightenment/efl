using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

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

    public static Dummy.Numberwrapper NW(int n)
    {
        var nw = new Dummy.Numberwrapper();
        nw.SetNumber(n);
        return nw;
    }

    public static Dummy.Numberwrapper[] BaseSeqObj()
    {
        var a = new Dummy.Numberwrapper();
        var b = new Dummy.Numberwrapper();
        var c = new Dummy.Numberwrapper();
        a.SetNumber(0x0);
        b.SetNumber(0x2A);
        c.SetNumber(0x42);
        return new Dummy.Numberwrapper[]{a,b,c};
    }

    public static Dummy.Numberwrapper[] AppendSeqObj()
    {
        var a = new Dummy.Numberwrapper();
        var b = new Dummy.Numberwrapper();
        var c = new Dummy.Numberwrapper();
        a.SetNumber(42);
        b.SetNumber(43);
        c.SetNumber(33);
        return new Dummy.Numberwrapper[]{a,b,c};
    }

    public static Dummy.Numberwrapper[] ModifiedSeqObj()
    {
        var a = new Dummy.Numberwrapper();
        var b = new Dummy.Numberwrapper();
        var c = new Dummy.Numberwrapper();
        var d = new Dummy.Numberwrapper();
        var e = new Dummy.Numberwrapper();
        var f = new Dummy.Numberwrapper();
        a.SetNumber(0x0);
        b.SetNumber(0x2A);
        c.SetNumber(0x42);
        d.SetNumber(42);
        e.SetNumber(43);
        f.SetNumber(33);
        return new Dummy.Numberwrapper[]{a,b,c,d,e,f};
    }

    public static void NumberwrapperSequenceAssertEqual(
        Dummy.Numberwrapper[] a
        , Dummy.Numberwrapper[] b
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

class NativeInheritImpl : Dummy.TestObject
{
    public NativeInheritImpl(Efl.Object parent = null) : base(parent) {}

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

    override public bool EinaSliceIn(Eina.Slice slice)
    {
        slice_in_flag = true;
        return slice.GetBytes().SequenceEqual(BaseSequence.Values());
    }

    override public bool EinaRwSliceIn(Eina.RwSlice slice)
    {
        rw_slice_in_flag = true;
        return slice.GetBytes().SequenceEqual(BaseSequence.Values());
    }

    private byte[] slice_out_seq = null;
    private GCHandle slice_out_pinned;
    override public bool EinaSliceOut(ref Eina.Slice slice)
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
    override public bool EinaRwSliceOut(ref Eina.RwSlice slice)
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
    override public bool EinaBinbufIn(Eina.Binbuf binbuf)
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

    private Eina.Binbuf binbuf_in_own_binbuf = null;
    override public bool EinaBinbufInOwn(Eina.Binbuf binbuf)
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

    private Eina.Binbuf binbuf_out_binbuf = null;
    override public bool EinaBinbufOut(out Eina.Binbuf binbuf)
    {
        binbuf_out_flag = true;

        binbuf = new Eina.Binbuf();
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

    private Eina.Binbuf binbuf_out_own_binbuf = null;
    override public bool EinaBinbufOutOwn(out Eina.Binbuf binbuf)
    {
        binbuf_out_own_flag = true;

        binbuf = new Eina.Binbuf();
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

    private Eina.Binbuf binbuf_return_binbuf = null;
    override public Eina.Binbuf EinaBinbufReturn()
    {
        binbuf_return_flag = true;

        var binbuf = new Eina.Binbuf();
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

    private Eina.Binbuf binbuf_return_own_binbuf = null;
    override public Eina.Binbuf EinaBinbufReturnOwn()
    {
        binbuf_return_own_flag = true;

        var binbuf = new Eina.Binbuf();
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


