using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.ComponentModel;
using System.Reflection;

namespace Efl {

#if EFL_BETA

internal class ModelHelper
{
    /// FIXME Move this to eina_value.cs and be a static method of Value?
    static internal Eina.Value ValueFromProperty<T>(T source, PropertyInfo property)
    {
        return new Eina.Value(property.GetValue(source));
    }

    static internal void SetPropertyFromValue<T>(T target, PropertyInfo property, Eina.Value v)
    {
        property.SetValue(target, v.Unwrap());
    }

    /// <summary>Sets the properties of the <paramref name="child"/> from the properties of the given object
    /// <paramref name="o"/>.</summary>
    static internal void SetProperties<T>(T o, Efl.IModel child)
    {
        var properties = typeof(T).GetProperties();
        foreach (var prop in properties)
        {
            using (var tmp = ValueFromProperty(o, prop))
            {
                child.SetProperty(prop.Name, tmp);
            }
        }
    }

    /// <summary>Sets the properties of <paramref name="o"/> from the properties of <paramref name="child"/>.</summary>
    static internal void GetProperties<T>(T o, Efl.IModel child)
    {
        var properties = typeof(T).GetProperties();
        foreach (var prop in properties)
        {
            using (var v = child.GetProperty(prop.Name))
            {
                SetPropertyFromValue(o, prop, v);
            }
        }
    }
}

/// <summary>Helper class to simplify the creation of MVVM Models based on <see cref="Efl.IModel" />.
///
/// <para>This class enables usage of simple model classes to define the data to be stored in the native model
/// and presented in a friendly manner to the C# developer. Example:</para>
///
/// <code>
/// public class PersonModel
/// {
///     public string Name { get; set; }
///     public int Age { get; set; }
/// }
/// // Instantiating the model
/// var modelData = new Efl.UserModel&lt;PersonModel&gt;(parent);
/// modelData.Add(new PersonModel { Name = "John", Age = 30 };
/// var model = new Efl.GenericModel&lt;PersonModel&gt;(modelData, parent);
/// PersonModel p = await model.GetAtAsync(0);
/// </code>
/// </summary>
/// <typeparam name="T">The enclosed C# model class with the properties to be added to the native model.</typeparam>
[Efl.Eo.BindingEntity]
public class UserModel<T> : Efl.MonoModelInternal
{
   /// <summary>
   /// Creates a new root model.
   ///
   /// <para>The properties of <code>T</code> will be added to the underlying native model as properties the user can get/set on children
   /// of this model.</para>
   /// </summary>
   /// <param name="parent">The parent of the model.</param>
   public UserModel (Efl.Object parent = null) : base(Efl.MonoModelInternal.efl_mono_model_internal_class_get(), parent)
   {
     var properties = typeof(T).GetProperties();
     foreach (var prop in properties)
     {
        AddProperty(prop.Name, Eina.ValueTypeBridge.GetManaged(prop.PropertyType));
     }
   }

   /// <summary>Adds a new child to the model wrapping the properties of <c>o</c>
   ///
   /// <para>Reflection is used to instantiate a new <see cref="Efl.IModel" />-based class for this child and
   /// set the mirroring properties correctly.</para>
   /// </summary>
   ///
   /// <param name="o">The user model instance to be added to this model.</param>
   public void Add (T o)
   {
       Efl.IModel child = (Efl.IModel) this.AddChild();
       ModelHelper.SetProperties(o, child);
   }
}

#endif

}
