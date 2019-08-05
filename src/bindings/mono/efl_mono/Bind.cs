#if EFL_BETA

using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.ComponentModel;

namespace Efl {

/// <summary>Representas a bindable property as used by <see cref="Efl.Ui.ItemFactory&lt;T&gt;" /> instances.
///
/// <para>It is internally instantiated and returned by generated extension methods.</para>
/// </summary>
public class Bindable<T>
{
    /// <summary>Creates a new bindable property with the source name <c>name</c>.</summary>
    public Bindable(string name, Efl.Ui.IPropertyBind binder)
    {
        this.name = name;
        this.binder = binder;
    }

    /// <summary>Binds the model property <c>model_property</c> to the property <c>name</c> set in the constructor.</summary>
    public void Bind(string model_property)
    {
        binder.PropertyBind(name, model_property);
    }

    string name;
    Efl.Ui.IPropertyBind binder;
}

}

#endif
