using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.ComponentModel;

namespace Efl {

public class Bindable<T>
{
    public Bindable(string name, Efl.Ui.PropertyBind binder)
    {
        this.name = name;
        this.binder = binder;
    }

    public void Bind(string model_property)
    {
        binder.PropertyBind(name, model_property);
    }

    public void Set(T value)
    {
        // set somehow
    }

    string name;
    Efl.Ui.PropertyBind binder;
}

}
