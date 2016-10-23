
namespace efl {

using System;
    
public class Object : efl.eo.IWrapper
{
    IntPtr handle;
    public Object()
    {
        handle = (IntPtr)0;
    }
    public Object(IntPtr raw)
    {
        handle = raw;
    }
}
    
}
