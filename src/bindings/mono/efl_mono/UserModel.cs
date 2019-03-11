using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Linq;
using System.ComponentModel;

namespace Efl {

internal class ModelHelper
{
    static internal void SetProperties<T>(T o, Efl.Model child)
    {
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
         Console.WriteLine ("Setting property with value {0}", v.ToString());
         child.SetProperty(prop.Name, v);
         //v.Dispose();
      }
    }

    static internal void GetProperties<T>(T o, Efl.Model child)
    {
      var properties = typeof(T).GetProperties();
      foreach(var prop in properties)
      {
         Console.WriteLine("Reading property of name {0}", prop.Name);
         using (var v = child.GetProperty(prop.Name))
         {
            Console.WriteLine("Read property");
            if (prop.PropertyType == typeof(int))
            {
               int x;
               v.Get(out x);
               Console.WriteLine("Type is int, value is {0}", x);
               prop.SetValue(o, x);
            }
            else if (prop.PropertyType == typeof(string))
            {
               string x;
               v.Get(out x);
               Console.WriteLine("Type is string, value is {0}", x);
               prop.SetValue(o, x);
            }
            else
            {
               Console.WriteLine("Type is unknown");
               throw new Exception("Type unknown " + prop.PropertyType.Name);
            }
         }
      }
    }
}
    
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
       // if (obj == null)
       //     Console.WriteLine("Object from AddChild is null");
       //Debug.Assert(obj != null);
       Console.WriteLine ("static casting"); Console.Out.Flush();
       Efl.Model child = Efl.ModelConcrete.static_cast(obj);
       //Debug.Assert(child != null);

       ModelHelper.SetProperties(o, child);
   }
}

}
