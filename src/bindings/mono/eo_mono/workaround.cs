
using System;
using System.Runtime.InteropServices;

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

public struct Evas_Object_Box_Layout {};
public struct Eina_Free_Cb {};
public struct Evas_Object_Box_Option {};

namespace eina {
    
public struct Rw_Slice {}
public struct Slice {}

}

namespace efl { namespace kw_event {

public struct Description {};
        
}

public struct Callback_Priority {};
public struct Event_Cb {};
public struct Callback_Array_Item {};
public struct Dbg_Info {};

}

namespace efl { namespace gfx {

public interface Buffer {}

namespace buffer {
public struct Access_Mode {}
}
        
} }

namespace evas { namespace font {

}

public struct Modifier_Mask {}

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

