using System;
using System.Reflection;

namespace Efl
{

namespace Eo
{

public class WrapperGetter
{
    System.WeakReference reference;

    public WrapperGetter (Efl.Eo.IWrapper obj)
    {
        reference = new WeakReference (obj);
    }
    
    public Efl.Eo.IWrapper Target
    {
        get
        {
            return (Efl.Eo.IWrapper)reference.Target;
        }
        set {}
        
    }
}

}
}

