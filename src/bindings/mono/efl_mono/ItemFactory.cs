#if EFL_BETA

using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.ComponentModel;

namespace Efl.Ui
{

/// <summary>Helper factory class. Makes use of C# extension methods for easier property binding.
///
/// <code>
/// var factory = Efl.Ui.Factory&lt;Efl.Ui.Button&gt;();
/// factory.Style().Bind("Name"); // The factory Style property is bound to the Name property for the given model.
/// </code>
/// </summary>
public class ItemFactory<T> : Efl.Ui.LayoutFactory
{
    /// <summary>Creates a new factory.
    /// </summary>
    /// <param name="parent">The parent of the factory.</param>
    public ItemFactory(Efl.Object parent = null)
        : base (parent, typeof(T))
    {
    }
}

}

#endif
