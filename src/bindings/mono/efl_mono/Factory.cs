using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.ComponentModel;

namespace Efl { namespace Ui {

public class ItemFactory<T> : Efl.Ui.CachingFactory, IDisposable
{
    public ItemFactory(Efl.Object parent = null)
        : base (parent, Efl.Eo.Globals.get_efl_klass_from_klass<T>())
    {
    }

    
}

} }
