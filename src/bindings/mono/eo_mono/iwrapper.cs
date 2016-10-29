
using System;
using System.Runtime.InteropServices;

namespace efl { namespace eo {

public interface IWrapper
{
    System.IntPtr raw_handle
    {
        get;
    }
}

class MarshalTest : ICustomMarshaler
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
        System.Console.WriteLine("GetNativeDataSize");
        return 0;
        //return 8;
    }

    public IntPtr MarshalManagedToNative(object ManagedObj)
    {
        System.Console.WriteLine("MarshallManagedToNative");
        return ((IWrapper)ManagedObj).raw_handle;
    }

    public object MarshalNativeToManaged(IntPtr pNativeData)
    {
        return null;
    }
}

} }
