
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
    [DllImport("eo")] public static extern IntPtr efl_class_get(IntPtr obj);
    [DllImport("dl")] public static extern IntPtr dlsym
       (IntPtr handle, [MarshalAs(UnmanagedType.LPStr)] String name);

    public delegate byte class_initializer(IntPtr klass);
    
    public static IntPtr register_class(class_initializer initializer, IntPtr base_klass)
    {
        ClassDescription description;
        description.version = 2; // EO_VERSION
        description.name = "BoxInherit";
        description.class_type = 0; // REGULAR
        description.data_size = (UIntPtr)8;
        description.class_initializer = IntPtr.Zero;
        description.class_constructor = IntPtr.Zero;
        description.class_destructor = IntPtr.Zero;

        if(initializer != null)
            description.class_initializer = Marshal.GetFunctionPointerForDelegate(initializer);

        IntPtr description_ptr = Marshal.AllocHGlobal(Marshal.SizeOf(description));
        Marshal.StructureToPtr(description, description_ptr, false);
      
        Console.WriteLine("Going to register!");
        IntPtr klass = efl.eo.Globals.efl_class_new(description_ptr, base_klass, IntPtr.Zero);
        if(klass == IntPtr.Zero)
            Console.WriteLine("klass was not registed");
        Console.WriteLine("Registered?");
        return klass;
    }
    public static IntPtr instantiate(IntPtr klass, efl.Object parent)
    {
        Console.WriteLine("Instantiating");
        System.IntPtr parent_ptr = System.IntPtr.Zero;
        if(parent != null)
            parent_ptr = parent.raw_handle;

        System.IntPtr eo = efl.eo.Globals._efl_add_internal_start("file", 0, klass, parent_ptr, 0, 0);
        Console.WriteLine("efl_add_internal_start returned");
        eo = efl.eo.Globals._efl_add_end(eo, 0, 0);
        Console.WriteLine("efl_add_end returned");
        return eo;
    }
    public static void data_set(efl.eo.IWrapper obj)
    {
      IntPtr pd = efl.eo.Globals.efl_data_scope_get(obj.raw_handle, obj.raw_klass);
      {
          GCHandle gch = GCHandle.Alloc(obj);
          EolianPD epd;
          epd.pointer = GCHandle.ToIntPtr(gch);
          Marshal.StructureToPtr(epd, pd, false);
      }
    }
    public static efl.eo.IWrapper data_get(IntPtr pd)
    {
        EolianPD epd = (EolianPD)Marshal.PtrToStructure(pd, typeof(EolianPD));
        if(epd.pointer != IntPtr.Zero)
        {
            GCHandle gch = GCHandle.FromIntPtr(epd.pointer);
            return (efl.eo.IWrapper)gch.Target;
        }
        else
            return null;
    }
}        
        
public interface IWrapper
{
    IntPtr raw_handle
    {
        get;
    }
    IntPtr raw_klass
    {
        get;
    }
}

public class MarshalTest<T> : ICustomMarshaler
{
    public static ICustomMarshaler GetInstance(string cookie)
    {
        Console.WriteLine("MarshalTest.GetInstace cookie " + cookie);
        return new MarshalTest<T>();
    }
    public void CleanUpManagedData(object ManagedObj)
    {
        Console.WriteLine("MarshalTest.CleanUpManagedData");
        //throw new NotImplementedException();
    }

    public void CleanUpNativeData(IntPtr pNativeData)
    {
        Console.WriteLine("MarshalTest.CleanUpNativeData");
        //throw new NotImplementedException();
    }

    public int GetNativeDataSize()
    {
        Console.WriteLine("MarshalTest.GetNativeDataSize");
        return 0;
        //return 8;
    }

    public IntPtr MarshalManagedToNative(object ManagedObj)
    {
        Console.WriteLine("MarshalTest.MarshallManagedToNative");
        return ((IWrapper)ManagedObj).raw_handle;
    }

    public object MarshalNativeToManaged(IntPtr pNativeData)
    {
        Console.WriteLine("MarshalTest.MarshalNativeToManaged");
        return Activator.CreateInstance(typeof(T), new System.Object[] {pNativeData});
//        return null;
    }
}

} }
