using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.ComponentModel;

namespace Efl {

public class UserModel<T> : Efl.MonoModelInternal, IDisposable
{
   ///<summary>Pointer to the native class description.</summary>
   public override System.IntPtr NativeClass {
      get {
         if (((object)this).GetType() == typeof (UserModel<T>))
            return Efl.MonoModelInternalNativeInherit.GetEflClassStatic();
         else
            return Efl.Eo.Globals.klasses[((object)this).GetType()];
      }
   }
   public UserModel (Efl.Object parent = null) : base("MonoModelInternal", Efl.MonoModelInternal.efl_mono_model_internal_class_get(), typeof(MonoModelInternal), parent, true)
   {
     inherited = false;
     var properties = typeof(T).GetProperties();
     foreach(var prop in properties)
     {
        AddProperty(prop.Name, System.IntPtr.Zero);
     }
         
     FinishInstantiation();
   }
    
   ~UserModel()
   {
       Dispose(false);
   }
   public void Add (T o)
   {
       Efl.Object obj = this.AddChild();
       if (obj == null)
           Console.WriteLine("Object from AddChild is null");
       //Debug.Assert(obj != null);
       Efl.Model child = Efl.ModelConcrete.static_cast(obj);
       //Debug.Assert(child != null);

       var properties = typeof(T).GetProperties();
       foreach(var prop in properties)
       {
           Eina.Value v;
           if (prop.PropertyType == typeof(int))
           {
               v = new Eina.Value(Eina.ValueType.Int32);
               v.Set((int)prop.GetValue(o));
           }
           else if (prop.PropertyType == typeof(string))
           {
               v = new Eina.Value(Eina.ValueType.String);
               v.Set((string)prop.GetValue(o));
           }
           else
               throw new Exception("Type unknown " + prop.PropertyType.Name);
           child.SetProperty(prop.Name, v);
       }
   }
}

}
