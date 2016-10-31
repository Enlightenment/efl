
using System;
using System.Runtime.InteropServices;

namespace efl { namespace eo {

public class Globals {
    [DllImport("eo")] public static extern IntPtr
        _efl_add_internal_start([MarshalAs(UnmanagedType.LPStr)] String file, int line,
                                IntPtr klass, IntPtr parent, byte is_ref, byte is_fallback);
    [DllImport("eo")] public static extern IntPtr
        _efl_add_end(IntPtr eo, byte is_ref, byte is_fallback);
    [DllImport("eo")] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0);
    [DllImport("eo")] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0, IntPtr base1);
    [DllImport("eo")] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0, IntPtr base1, IntPtr base2);
    [DllImport("eo")] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0, IntPtr base1, IntPtr base2, IntPtr base3);
    [DllImport("eo")] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0, IntPtr base1, IntPtr base2, IntPtr base3, IntPtr base4);
    [DllImport("eo")] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0, IntPtr base1, IntPtr base2, IntPtr base3, IntPtr base4, IntPtr base5);
    [DllImport("eo")] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0, IntPtr base1, IntPtr base2, IntPtr base3, IntPtr base4, IntPtr base5, IntPtr base6);
    [DllImport("eo")] public static extern IntPtr
        efl_class_new(IntPtr class_description, IntPtr base0, IntPtr base1, IntPtr base2, IntPtr base3, IntPtr base4, IntPtr base5, IntPtr base6, IntPtr base7);
    [DllImport("eo")] public static extern byte efl_class_functions_set(IntPtr klass_id, IntPtr object_ops, IntPtr class_ops);
    [DllImport("eo")] public static extern IntPtr efl_data_scope_get(IntPtr obj, IntPtr klass);
    [DllImport("eo")] public static extern IntPtr efl_super(IntPtr obj, IntPtr klass);
}        
        
public interface IWrapper
{
    IntPtr raw_handle
    {
        get;
    }
}

public class MarshalTest : ICustomMarshaler
{
    public static ICustomMarshaler GetInstance(string cookie)
    {
        return new MarshalTest();
    }
    public void CleanUpManagedData(object ManagedObj)
    {
        //throw new NotImplementedException();
    }

    public void CleanUpNativeData(IntPtr pNativeData)
    {
        //throw new NotImplementedException();
    }

    public int GetNativeDataSize()
    {
        Console.WriteLine("GetNativeDataSize");
        return 0;
        //return 8;
    }

    public IntPtr MarshalManagedToNative(object ManagedObj)
    {
        Console.WriteLine("MarshallManagedToNative");
        return ((IWrapper)ManagedObj).raw_handle;
    }

    public object MarshalNativeToManaged(IntPtr pNativeData)
    {
        return null;
    }
}

} }
