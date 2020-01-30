using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.ComponentModel;
using Eina;

#if EFL_BETA

namespace Efl {

/// <summary>
/// Generic <see cref="Efl.IModel" /> helper class to ease manual implementation of C# models.
///
/// <para>It provides an expanded API like async helpers to get children.</para>
///
/// <para>For MVVM-based models, <see cref="Efl.UserModel&lt;T&gt;" /> provides a simpler API.</para>
/// </summary>
/// <typeparam name="T">The type of the child model. It is the type used when adding/removing/getting items to this
/// model.</typeparam>
public class GenericModel<T> : Efl.Object, Efl.IModel
{
   private Efl.IModel model;

   /// <summary>Creates a new model wrapping <c>model</c>.</summary>
   /// <param name="model">The model to be wrapped.</param>
   /// <param name="parent">The parent of the model.</param>
   public GenericModel (Efl.IModel model, Efl.Object parent = null) : base(parent)
   {
       this.model = model;
   }

   /// <summary>The list of properties available in the wrapped model.</summary>
   public IEnumerable<System.String> Properties
   {
      get { return model.Properties; }
   }

   /// <summary>The number of children in the wrapped model.</summary>
   public  uint ChildrenCount
   {
      get { return model.ChildrenCount; }
   }

   /// <summary>Gets the value of the given property in the wrapped model.</summary>
   /// <param name="property">The property of the model.</param>
   /// <returns>The value of the property.</returns>
   public Eina.Value GetProperty(  System.String property)
   {
       return model.GetProperty(property);
   }

   /// <summary>Sets the value of the given property in the given model.</summary>
   /// <param name="property">The property of the model.</param>
   /// <param name="value">The value of the property.</param>
   /// <returns>An <see cref="Eina.Future" /> that resolves when the property has
   /// been set or reports an error if it could not be set.</returns>
   public Eina.Future SetProperty(  System.String property,   Eina.Value value)
   {
       return model.SetProperty(property, value);
   }

   /// <summary>Returns an <see cref="Eina.Future" /> that will resolve when the property is ready to be read.</summary>
   /// <param name="property">The property of the model.</param>
   /// <returns>An <see cref="Eina.Future" /> that resolves when the property is ready.</returns>
   public Eina.Future GetPropertyReady(  System.String property)
   {
       return model.GetPropertyReady(property);
   }

   /// <summary>Gets a number of children from the wrapped model.</summary>
   /// <param name="start">The start of the range.</param>
   /// <param name="count">The size of the range.</param>
   /// <returns>An <see cref="Eina.Future" />  that resolves to an
   /// <see cref="Eina.Array&lt;T&gt;" /> of children models.</returns>
   public Eina.Future GetChildrenSlice(  uint start,   uint count)
   {
       return model.GetChildrenSlice(start, count);
   }

   /// <summary>Adds a new object to the wrapper model.</summary>
   /// <param name="o">The object to get the properties from.</param>
   public void Add(T o)
   {
      Efl.IModel child = (Efl.IModel)this.AddChild();
      ModelHelper.SetProperties(o, child);
   }

   /// <summary>Adds a new child to the model and returns it.</summary>
   /// <returns>The object to be wrapped.</returns>
   public Efl.Object AddChild()
   {
       return model.AddChild();
   }

   /// <summary>Deletes the given <c>child</c> from the wrapped model.</summary>
   /// <param name="child">The child to be deleted.</param>
   public void DelChild( Efl.Object child)
   {
       model.DelChild(child);
   }

   /// <summary>Gets the element at the specified <c>index</c>.</summary>
   /// <param name="index">The position of the element.</param>
   /// <returns>Token to notify the async operation of external request to cancel.</returns>
   async public System.Threading.Tasks.Task<T> GetAtAsync(uint index)
   {
       using (Eina.Value v = await GetChildrenSliceAsync(index, 1).ConfigureAwait(false))
       {
           if (v.GetValueType().IsContainer())
           {
               var child = (Efl.IModel)v[0];
               T obj = (T)Activator.CreateInstance(typeof(T), Array.Empty<object>());
               ModelHelper.GetProperties(obj, child);
               return obj;
           }
           else
           {
               throw new System.InvalidOperationException("GetChildrenSlice must have returned a container");
           }
       }
   }

   /// <summary>Async wrapper around <see cref="SetProperty(System.String, Eina.Value)" />.</summary>
   /// <param name="property">The property to be added.</param>
   /// <param name="value">The value of the property.</param>
   /// <param name="token">The token for the task's cancellation.</param>
   /// <returns>Task that resolves when the property has been set or could not
   /// be set.</returns>
   public System.Threading.Tasks.Task<Eina.Value> SetPropertyAsync(  System.String property,  Eina.Value value, System.Threading.CancellationToken token=default(System.Threading.CancellationToken))
   {
       return model.SetPropertyAsync(property, value, token);
   }

   /// <summary>Async wrapper around <see cref="GetPropertyReady(System.String)" />.</summary>
   /// <param name="property">The property of the model.</param>
   /// <param name="token">The token for the task's cancellation.</param>
   /// <returns>Task that resolves when the given property is ready to be
   /// read.</returns>
   public System.Threading.Tasks.Task<Eina.Value> GetPropertyReadyAsync(  System.String property, System.Threading.CancellationToken token=default(System.Threading.CancellationToken))
   {
       return model.GetPropertyReadyAsync(property, token);
   }

   /// <summary>Async wrapper around <see cref="GetChildrenSlice(uint, uint)" />.</summary>
   /// <param name="start">The start of the range.</param>
   /// <param name="count">The size of the range.</param>
   /// <param name="token">Token to notify the async operation of external request to cancel.</param>
   /// <returns>Task that resolves when the desired <see cref="Eina.Array&lt;T&gt;" /> of
   /// children models is ready.</returns>
   public System.Threading.Tasks.Task<Eina.Value> GetChildrenSliceAsync(  uint start,  uint count, System.Threading.CancellationToken token=default(System.Threading.CancellationToken))
   {
       return model.GetChildrenSliceAsync(start, count, token);
   }

   /// <summary>Get children as specified by iterator.
   /// 
   /// Provided index have to be between 0 and <see cref="Efl.IModel.ChildrenCount"/>.
   /// 
   /// This function might rely on <see cref="Efl.IModel.GetChildrenSlice"/> as a fallback.<br/>Since EFL 1.23.</summary>
   /// <param  name="indices">Indices of the requested children.</param>
   /// <returns>Array of children</returns>
   public Eina.Future GetChildrenIndex(IEnumerable<uint> indices)
   {
       return model.GetChildrenIndex(indices);
   }

   /// <summary>Async wrapper for <see cref="GetChildrenIndex" />.
   /// </summary>
   /// <param name="indices">Indices of the requested children.</param>
   /// <param name="token">Token to notify the async operation of external request to cancel.</param>
   /// <returns>An async task wrapping the result of the operation.</returns>
   public System.Threading.Tasks.Task<Eina.Value> GetChildrenIndexAsync(IEnumerable<uint> indices, System.Threading.CancellationToken token = default(System.Threading.CancellationToken))
   {
       return model.GetChildrenIndexAsync(indices, token);
   }

   /// <summary>Event triggered when properties on the wrapped model changes.</summary>
   public event EventHandler<Efl.ModelPropertiesChangedEventArgs> PropertiesChangedEvent
   {
      add {
          model.PropertiesChangedEvent += value;
      }
      remove {
          model.PropertiesChangedEvent -= value;
      }
   }

   /// <summary>Event triggered when a child is added from the wrapped model.</summary>
   public event EventHandler<Efl.ModelChildAddedEventArgs> ChildAddedEvent
   {
      add {
          model.ChildAddedEvent += value;
      }
      remove {
          model.ChildAddedEvent -= value;
      }
   }

   /// <summary>Event triggered when a child is removed from the wrapped model.</summary>
   public event EventHandler<Efl.ModelChildRemovedEventArgs> ChildRemovedEvent
   {
      add {
          model.ChildRemovedEvent += value;
      }
      remove {
          model.ChildRemovedEvent -= value;
      }
   }

   /// <summary>Event triggered when the number of children changes.</summary>
   public event EventHandler ChildrenCountChangedEvent
   {
      add {
          model.ChildrenCountChangedEvent += value;
      }
      remove {
          model.ChildrenCountChangedEvent -= value;
      }
   }
}

}

#endif
