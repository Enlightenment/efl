#if EFL_BETA

using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.ComponentModel;

namespace Efl {

/// <summary>Represents a bindable property as used by <see cref="Efl.Ui.ItemFactory&lt;T&gt;" /> instances.
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

    /// <summary>Binds the model property <c>modelProperty</c> to the property <c>name</c> set in the constructor.</summary>
    public void Bind(string modelProperty)
    {
        binder.PropertyBind(name, modelProperty);
    }

    string name;
    Efl.Ui.IPropertyBind binder;
}

/// <summary>Represents bindable parts as used by <see cref="Efl.Ui.ItemFactory&lt;T&gt;" /> instances.
///
/// <para>It is internally instantiated and returned by generated extension methods.</para>
/// </summary>
public class BindablePart<T>
{
    /// <summary>Creates a new bindable property with the binder <c>binder</c>.</summary>
    public BindablePart(Efl.Ui.IPropertyBind binder)
    {
        this.binder = binder;
    }

    /// <summary>Binds the model property <c>modelProperty</c> to the part property <c>name</c> set in the constructor.</summary>
    public void Bind(string partProperty, string modelProperty)
    {
        binder.PropertyBind(partProperty, modelProperty);
    }

    private Efl.Ui.IPropertyBind binder;
}

}

#endif
