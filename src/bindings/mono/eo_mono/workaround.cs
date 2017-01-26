
using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;

[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi)]
public struct ClassDescription
{
    public uint version;
    [MarshalAs(UnmanagedType.LPStr)] public String name;
    public int class_type;
    public UIntPtr data_size;
    public IntPtr class_initializer;
    public IntPtr class_constructor;
    public IntPtr class_destructor;
}

[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi)]
public struct Efl_Op_Description
{
    public IntPtr api_func;
    public IntPtr func;
}

[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi)]
public struct Efl_Object_Ops
{
    public IntPtr descs; /**< The op descriptions array of size count. */
    public UIntPtr count; /**< Number of op descriptions. */
};

[StructLayout(LayoutKind.Sequential)]
public struct EolianPD
{
    public IntPtr pointer;
}

#pragma warning disable 0169
public struct Evas_Object_Box_Layout
{
    IntPtr o;
    IntPtr priv;
    IntPtr user_data;
};
public struct Eina_Free_Cb
{
    IntPtr data;
};
public struct Evas_Object_Box_Option {
    IntPtr obj;
    [MarshalAsAttribute(UnmanagedType.U1)] bool max_reached;
    [MarshalAsAttribute(UnmanagedType.U1)] bool min_reached;
    evas.Coord alloc_size;
};
#pragma warning restore 0169

namespace eina {
    
public struct Rw_Slice {}
public struct Slice {}

}

namespace efl { namespace kw_event {

[StructLayout(LayoutKind.Sequential)]
public struct Description {
    IntPtr pointer; // Opaque type, just pass the pointer. What about hot/freeze/etc?

    private static Dictionary<string, IntPtr> descriptions = new Dictionary<string, IntPtr>();

    public Description(string name)
    {
        if (!descriptions.ContainsKey(name))
        {
            // FIXME Other libraries....
            Console.WriteLine("Loading {name}");
            IntPtr library = efl.eo.Globals.dlopen("/opt/efl-mono/lib/libecore.so", efl.eo.Globals.RTLD_NOW);
            IntPtr data = efl.eo.Globals.dlsym(library, name);
            descriptions.Add(name, data);
        }

        this.pointer = descriptions[name];
    }
};

}


public delegate void Event_Cb(System.IntPtr data, System.IntPtr evt);
#pragma warning disable 0169
public struct Callback_Array_Item {
    IntPtr desc;
    IntPtr func;
};
public struct Dbg_Info {
    IntPtr name;
    IntPtr value;
};
#pragma warning restore 0169

public struct Event {


};

}

namespace efl { namespace gfx {

public interface Buffer {}

namespace buffer {
public struct Access_Mode {}
}
        
} }

namespace evas { namespace font {

}


// C# does not allow typedefs, so we use these implicit conversions.
public struct Modifier_Mask {
    private ulong mask;

    public static implicit operator Modifier_Mask(ulong x)
    {
        return new Modifier_Mask{mask=x};
    }
    public static implicit operator ulong(Modifier_Mask x)
    {
        return x.mask;
    }
}

public struct Coord {
    int val;

    public Coord(int value) { val = value; }
    static public implicit operator Coord(int val) {
        return new Coord(val);
    }
    static public implicit operator int(Coord coord) {
        return coord.val;
    }
}


}

public struct Efl_Font_Size {
    int val;

    public Efl_Font_Size(int value) { val = value; }
    static public implicit operator Efl_Font_Size(int val) {
        return new Efl_Font_Size(val);
    }
    static public implicit operator int(Efl_Font_Size coord) {
        return coord.val;
    }
}

