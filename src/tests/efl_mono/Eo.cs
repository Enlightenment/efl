/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
using System;
using System.Linq;
using System.Collections.Generic;
using System.Reflection;
using System.Diagnostics.CodeAnalysis;

namespace TestSuite
{

class TestEo
{
    public static void return_null_object()
    {
        var testing = new Dummy.TestObject();
        var o1 = testing.ReturnNullObject();
        Test.Assert(o1 == null);
        testing.Dispose();
    }

    //
    // Test cases:
    //
    public static void return_same_object()
    {
        var testing = new Dummy.TestObject();
        var o1 = testing.ReturnObject();
        Test.Assert(o1.NativeHandle != IntPtr.Zero);
        Test.Assert(o1.NativeHandle == testing.NativeHandle);
        var o2 = o1.ReturnObject();
        Test.Assert(o2.NativeHandle != IntPtr.Zero);
        Test.Assert(o2.NativeHandle == o1.NativeHandle);
        testing.Dispose();
    }
    /* Commented out as adding the event listener seems to prevent it from being GC'd.
    public static void destructor_really_frees()
    {
       bool delEventCalled = false;
       {
           var obj = new Dummy.TestObject();
           obj.DEL += (object sender, EventArgs e) => { delEventCalled = true; };
       }

       System.GC.WaitForPendingFinalizers();
       System.GC.Collect();
       System.GC.WaitForPendingFinalizers();
       System.GC.Collect();
       System.GC.WaitForPendingFinalizers();

       Test.Assert(delEventCalled, "DEL event not called");
    } */

    /* Commented until we figure out a new way to test disposing
    public static void dispose_really_frees()
    {
       bool delEventCalled = false;
       {
           var obj = new Dummy.TestObject();
           Eina.Log.Error($"Created object 0x{obj.NativeHandle.ToInt64():x}");
           obj.DelEvent += (object sender, EventArgs e) => { delEventCalled = true; };
           Eina.Log.Error($"Will dispose object 0x{obj.NativeHandle.ToInt64():x}");
           ((IDisposable)obj).Dispose();
       }

       Test.Assert(delEventCalled, "DEL event not called");
    }
    */

    /* Commented out as adding the event listener seems to prevent it from being GC'd.
    public static void derived_destructor_really_frees()
    {
       bool delEventCalled = false;
       {
           var obj = new Derived();
           obj.DEL += (object sender, EventArgs e) => { delEventCalled = true; };
       }

       System.GC.WaitForPendingFinalizers();
       System.GC.Collect();
       System.GC.WaitForPendingFinalizers();
       System.GC.Collect();
       System.GC.WaitForPendingFinalizers();

       Test.Assert(delEventCalled, "DEL event not called");
    }

    public static void derived_dispose_really_frees()
    {
       bool delEventCalled = false;
       {
           var obj = new Derived();
           obj.DEL += (object sender, EventArgs e) => { delEventCalled = true; };
           ((IDisposable)obj).Dispose();
       }

       Test.Assert(delEventCalled, "DEL event not called");
    }
    */
}


class MyObject : Efl.Object
{
    public MyObject() : base(null) { }
}

class TestEoInherit
{
    public static void instantiate_inherited()
    {
        Efl.Object loop = new MyObject();
        Test.Assert(loop.NativeHandle != System.IntPtr.Zero);
        loop.Dispose();
    }

    [SuppressMessage("Microsoft.Reliability", "CA2000:DisposeObjectBeforeLosingScope", Justification = "It is expected to lose scope.")]
    private static WeakReference CreateCollectableInherited()
    {
        return new WeakReference(new MyObject());
    }

    public static void inherited_collected()
    {
        var wref = CreateCollectableInherited();
        Test.CollectAndIterate(300, 10);

        Test.AssertNull(wref.Target);
    }
}

class TestEoNames
{
    public static void name_getset()
    {
        var obj = new Dummy.TestObject();

        string name = "Dummy";
        obj.Name = name;
        Test.AssertEquals(name, obj.Name);
        obj.Dispose();
    }
}

class TestEoParent
{
    public static void basic_parent()
    {
        var parent = new Dummy.TestObject(null);
        var child = new Dummy.TestObject(parent);

        Test.AssertEquals(parent, child.Parent);

        var parent_retrieved = child.Parent as Dummy.TestObject;
        Test.AssertEquals(parent, parent_retrieved);
        child.Dispose();
        parent.Dispose();
    }

    public static void parent_inherited_class()
    {
        Dummy.Numberwrapper parent = new Dummy.Numberwrapper(null);
        var child = new Dummy.TestObject(parent);

        Test.AssertEquals(parent, child.Parent);

        Dummy.Numberwrapper parent_retrieved = child.Parent as Dummy.Numberwrapper;
        Test.AssertEquals(parent, parent_retrieved);
        child.Dispose();
        parent.Dispose();
    }

    private class Derived : Dummy.TestObject
    {
        public Derived(Dummy.TestObject parent = null) : base (parent)
        {
        }
    }

    public static void basic_parent_managed_inherit()
    {
        var parent = new Derived(null);
        var child = new Derived(parent);

        Test.AssertEquals(parent, child.Parent);

        var parent_from_cast = child.Parent as Derived;
        Test.AssertEquals(parent, parent_from_cast);
        child.Dispose();
        parent.Dispose();
    }
}

class TestKlassMethods
{
    public static void basic_class_method()
    {
        int reference = 0xbeef;
        Dummy.TestObject.KlassProp = reference;
        Test.AssertEquals(reference, Dummy.TestObject.KlassProp);
    }

    public static void inherited_class_method()
    {
        int reference = 0xdead;
        Dummy.Child.KlassProp = reference;
        Test.AssertEquals(reference, Dummy.Child.KlassProp);
    }
}

class TestTypedefs
{
    public static void basic_typedef_test()
    {
        var obj = new Dummy.TestObject();
        Dummy.MyInt input = 1900;
        Dummy.MyInt receiver;

        int ret = obj.BypassTypedef(input, out receiver);

        Test.AssertEquals((Dummy.MyInt)ret, input);
        Test.AssertEquals(receiver, input);
        obj.Dispose();
    }
}

class TestVariables
{
    public static void test_constant_variables()
    {
        Test.AssertEquals(Dummy.Constants.ConstvarBool, true);
        Test.AssertEquals(Dummy.Constants.ConstvarInt, -32766);
        Test.AssertEquals(Dummy.Constants.ConstvarUInt, 65533U);
        Test.AssertEquals(Dummy.Constants.ConstvarLong, -2147483644L);
        Test.AssertEquals(Dummy.Constants.ConstvarULong, 4294967288UL);
        Test.AssertEquals(Dummy.Constants.ConstvarLLong, -9223372036854775800);
        Test.AssertEquals(Dummy.Constants.ConstvarULLong, 18446744073709551615);
        Test.AssertEquals(Dummy.Constants.ConstvarFloat, 16777211.0f);
        Test.AssertEquals(Dummy.Constants.ConstvarDouble, 9007199254740988.0);
        Test.AssertEquals(Dummy.Constants.ConstvarChar, '!');
        Test.AssertEquals(Dummy.Constants.ConstvarString, "test_str");
    }
}

class TestEoAccessors
{
    private static void do_eo_accessors(IEnumerable<int> accessor, bool shouldMove=false)
    {
        var obj = new Dummy.TestObject();

        IEnumerable<int> source = shouldMove ? accessor.ToList() : accessor;

        IEnumerable<int> acc = shouldMove ? obj.CloneAccessorOwn(accessor) : obj.CloneAccessor(accessor);

        var zipped = acc.Zip(source, (first, second) => new Tuple<int, int>(first, second));

        foreach (Tuple<int, int> pair in zipped)
        {
            Test.AssertEquals(pair.Item1, pair.Item2);
        }

        obj.Dispose();
    }

    public static void eina_eo_accessors()
    {
        Eina.List<int> lst = new Eina.List<int>();
        lst.Append(4);
        lst.Append(3);
        lst.Append(2);
        lst.Append(5);

        do_eo_accessors(lst.GetAccessor());

        lst.Dispose();
    }
    public static void eina_eo_accessors_own()
    {
        Eina.List<int> lst = new Eina.List<int>();
        lst.Append(4);
        lst.Append(3);
        lst.Append(2);
        lst.Append(1);
        Eina.Accessor<int> acc = lst.GetAccessor();
        do_eo_accessors(acc, shouldMove : true);

        Test.Assert(acc.Own);

    }

    public static void managed_eo_accessors()
    {
        List<int> lst = new List<int>();
        lst.Add(-1);
        lst.Add(1);
        lst.Add(4);
        lst.Add(42);

        do_eo_accessors(lst);
    }

    public static void managed_eo_accessors_own()
    {
        List<int> lst = new List<int>();
        lst.Add(-1);
        lst.Add(1);
        lst.Add(4);
        lst.Add(42);

        do_eo_accessors(lst, shouldMove : true);
    }
}

class TestEoFinalize
{
    public sealed class Inherit : Efl.Object
    {
        public bool finalizeCalled = false;
        public override Efl.Object FinalizeAdd()
        {
            finalizeCalled = true;
            return this;
        }
    }


    public static void finalize_call()
    {
        Inherit inherit = new Inherit();
        Test.Assert(inherit.finalizeCalled);
        inherit.Dispose();
    }
}

class TestEoMultipleChildClasses
{

    public sealed class FirstChild : Efl.Object
    {
        public int receivedValue = 0;
        public override Efl.Object FinalizeAdd()
        {
            receivedValue = 1;
            return this;
        }
    }

    public sealed class SecondChild : Efl.Object
    {
        public int receivedValue = 0;
        public override Efl.Object FinalizeAdd()
        {
            receivedValue = 2;
            return this;
        }
    }

    public static void test_multiple_child_classes()
    {
        FirstChild obj = new FirstChild();
        Test.AssertEquals(1, obj.receivedValue);
        SecondChild obj2 = new SecondChild();
        Test.AssertEquals(2, obj2.receivedValue);

        obj.Dispose();
        obj = new FirstChild();
        Test.AssertEquals(1, obj.receivedValue);
        obj2.Dispose();
        obj.Dispose();
    }
}

class TestCsharpProperties
{

    private class MyObject : Dummy.TestObject
    {
        public MyObject(Efl.Object parent = null) : base(parent)
        {
        }
        private MyObject(ConstructingHandle ch) : base(ch)
        {
        }
    }
    public static void test_csharp_properties()
    {
        var obj = new Dummy.TestObject();
        var name = "My Name";
        obj.Name = name;

        Test.AssertEquals(name, obj.Name);
        obj.Dispose();
    }

    public static void test_getter_only()
    {
        var obj = new Dummy.TestObject();
        Test.Assert(!obj.Invalidating);
        obj.Dispose();
    }

    public static void test_setter_only()
    {
        var obj = new Dummy.TestObject();
        int val = -1984;

        obj.SetSetterOnly(val);
        Test.AssertEquals(val, obj.GetSetterOnly());
        obj.Dispose();
    }

    public static void test_class_property()
    {
        int val = -42;
        Dummy.TestObject.KlassProp = val;
        Test.AssertEquals(val, Dummy.TestObject.KlassProp);
    }

    public static void test_iface_property()
    {
        int val = -33;
        Dummy.ITestIface iface = new Dummy.TestObject();
        iface.IfaceProp = val;
        Test.AssertEquals(val, iface.IfaceProp);
        iface.Dispose();
    }

    public static void test_iface_value_property()
    {
        var obj = new Dummy.TestObject();
        var prop = new MyObject();

        obj.IfaceValueProp = prop;
        Test.AssertEquals(obj.IfaceValueProp, prop);

        obj.Dispose();
        prop.Dispose();
    }

    public static void test_iface_value_from_c()
    {
        var obj = new Dummy.TestObject();

        obj.SetIfaceKlassProp(typeof(MyObject));
        Test.AssertEquals(obj.IfaceValueFromC.GetType(), typeof(MyObject));

        obj.Dispose();
    }

    public static void test_csharp_multi_valued_prop()
    {
        var obj = new Dummy.TestObject();
        obj.MultiValuedProp = (1, 2);
        var ret = obj.MultiValuedProp;
        Test.AssertEquals(ret, (1, 2));
        obj.Dispose();
    }

    public static void test_csharp_return_type_get_prop ()
    {
        var obj = new Dummy.TestObject();
        obj.ReturnTypeGetProp = 5;
        var i = obj.ReturnTypeGetProp;
    }

    public static void test_csharp_return_type_set_prop ()
    {
        var obj = new Dummy.TestObject();
        obj.ReturnTypeSetProp = 5;
        var i = obj.ReturnTypeSetProp;
    }

    public static void test_csharp_return_type_prop ()
    {
        var obj = new Dummy.TestObject();
        obj.ReturnTypeProp = 5;
        var i = obj.ReturnTypeProp;
    }
}

class TestEoGrandChildrenFinalize
{
    public sealed class Child : Dummy.TestObject
    {
        public int receivedValue = 0;
        public override Efl.Object FinalizeAdd()
        {
            receivedValue = 42;
            return this;
        }
    }

    public static void test_grand_children_finalize()
    {
        Child obj = new Child();
        Test.AssertEquals(42, obj.receivedValue);
        obj.Dispose();
    }

    public sealed class GrandChild : Dummy.Child
    {

#if EFL_BETA
        public GrandChild() : base(null, "", 0.0, 0) { }
#else
        public GrandChild() : base(null, "", 0.0) { }
#endif

        public int receivedValue = 0;
        public override Efl.Object FinalizeAdd()
        {
            receivedValue = -42;
            return this;
        }
    }

    public static void test_grand_grand_children_finalize()
    {
        GrandChild obj = new GrandChild();
        Test.AssertEquals(-42, obj.receivedValue);
        obj.Dispose();
    }
}

class TestConstructors
{
    public static void test_simple_constructor()
    {
        int iface_prop = 42;
        string a = "LFE";
        double b = 3.14;
#if EFL_BETA
        int beta = 1337;
#endif

#if EFL_BETA
        var obj = new Dummy.Child(null, a, b, beta, iface_prop, 0);
#else
        var obj = new Dummy.Child(null, a, b, iface_prop);
#endif
        Test.AssertEquals(iface_prop, obj.IfaceProp);
        obj.Dispose();
#if EFL_BETA
        obj = new Dummy.Child(parent: null, ifaceProp : iface_prop, doubleParamsA : a, doubleParamsB : b,
                              obligatoryBetaCtor : beta,
                              optionalBetaCtor : -beta);
#else
        obj = new Dummy.Child(parent: null, ifaceProp : iface_prop, doubleParamsA : a, doubleParamsB : b);
#endif
        Test.AssertEquals(iface_prop, obj.IfaceProp);

#if EFL_BETA
        Test.Assert(obj.ObligatoryBetaCtorWasCalled);
        Test.Assert(obj.OptionalBetaCtorWasCalled);
#endif
        obj.Dispose();
    }

    public static void test_optional_constructor()
    {
        string a = "LFE";
        double b = 3.14;
#if EFL_BETA
        int beta = 2241;
        var obj = new Dummy.Child(null, a, b, obligatoryBetaCtor : beta);
        Test.Assert(!obj.OptionalBetaCtorWasCalled);
#else
        var obj = new Dummy.Child(null, a, b);
#endif
        Test.Assert(!obj.IfaceWasSet);
        obj.Dispose();
    }
}

class TestInterfaceConcrete
{
    // For T7619
    public static void test_iface_concrete_methods()
    {
        var obj = new Dummy.TestObject();
        Dummy.ITestIface iface = obj.ReturnIface();

        iface.IfaceProp = 1970;
        Test.AssertEquals(iface.IfaceProp, 1970);
        obj.Dispose();
    }
}

class TestProvider
{
    public static void test_find_provider()
    {
        // Tests only the direction C# -> C
        var obj = new Dummy.TestObject();
        Dummy.Numberwrapper provider = obj.FindProvider(typeof(Dummy.Numberwrapper)) as Dummy.Numberwrapper;
        Test.AssertEquals(provider.GetType(), typeof(Dummy.Numberwrapper));
        Test.AssertEquals(provider.Number, 1999);
        obj.Dispose();
    }

    private class ProviderHolder : Dummy.TestObject
    {
        private Dummy.TestObject provider;
        public static string ProviderName
        {
            get
            {
                return "MyProvider";
            }
        }

        public ProviderHolder() : base(null)
        {
            this.provider = new Dummy.TestObject(this);
            this.provider.Name = ProviderHolder.ProviderName;
            this.provider.IfaceProp = 1997;
        }

        public override Efl.Object FindProvider(System.Type type)
        {
            Console.WriteLine("Called FindProvider");
            if (type == typeof(Dummy.ITestIface))
            {
                return this.provider;
            }
            else
            {
                return null;
            }
        }
    }

    public static void test_find_provider_iface()
    {
        var obj = new ProviderHolder();

        var provider = obj.CallFindProvider(typeof(Efl.Object));
        Test.AssertNull(provider, msg : "Unkonw provider must be null");

        provider = obj.CallFindProviderForIface();
        Test.AssertNotNull(provider, msg : "Provider of ITestIFace must not be null");
        Test.AssertEquals(provider.Name, ProviderHolder.ProviderName, "Provider name does not match expected");
        obj.Dispose();
    }
}

class TestObjectDeletion
{
    public static void test_object_deletion()
    {
        var obj = new Dummy.PartHolder();
        var part = obj.OnePart;

        Test.AssertNotNull(part);

        part.Del();

        Test.AssertNull(obj.OnePart);
        obj.Dispose();
    }
}

class TestProtectedInterfaceMembers
{

    private class MyObject : Dummy.TestObject
    {
        public MyObject(Efl.Object parent = null) : base(parent)
        {
        }

        protected override int MethodProtected(int x)
        {
            return x * x;
        }
    }

    public static void test_protected_interface_in_generated_class_called_from_c()
    {
        var obj = new Dummy.TestObject();
        Test.AssertEquals(obj.CallMethodProtected(42), -42);
        obj.Dispose();
    }

    public static void test_protected_interface_in_inherited_class_called_from_c()
    {
        var obj = new MyObject();
        Test.AssertEquals(obj.CallMethodProtected(42), 42 * 42);
        obj.Dispose();
    }

    public static void test_interface_skipped_protected()
    {
        var type = typeof(Dummy.ITestIface);
        var methods = type.GetMethods(BindingFlags.Public | BindingFlags.Instance);
        var properties = type.GetProperties(BindingFlags.Public | BindingFlags.Instance);

        // Fully protected property
        Test.AssertNull(methods.SingleOrDefault(m => m.Name == "GetProtectedProp"));
        Test.AssertNull(methods.SingleOrDefault(m => m.Name == "SetProtectedProp"));

        // Partially protected property
        Test.AssertNotNull(properties.SingleOrDefault(m => m.Name == "PublicGetterPrivateSetter"));
        Test.AssertNull(methods.SingleOrDefault(m => m.Name == "SetPublicGetterPrivateSetter"));

        Test.AssertNull(properties.SingleOrDefault(m => m.Name == "ProtectedProp"));
        Test.AssertNotNull(properties.SingleOrDefault(m => m.Name == "PublicGetterPrivateSetter"));
    }

    public static void test_interface_skipped_protected_in_implementation()
    {
        var type = typeof(Dummy.TestObject);

        // Fully internal property
        var internal_methods = type.GetMethods(BindingFlags.NonPublic | BindingFlags.Instance).Where(m => m.IsAssembly);
        Test.AssertNotNull(internal_methods.SingleOrDefault(m => m.Name == "GetProtectedProp"));
        Test.AssertNotNull(internal_methods.SingleOrDefault(m => m.Name == "SetProtectedProp"));

        // Partially protected property
        var public_methods = type.GetMethods(BindingFlags.Public | BindingFlags.Instance);
        Test.AssertNull(public_methods.SingleOrDefault(m => m.Name == "GetPublicGetterPrivateSetter"));
        Test.AssertNotNull(internal_methods.SingleOrDefault(m => m.Name == "GetPublicGetterPrivateSetter"));
        Test.AssertNotNull(internal_methods.SingleOrDefault(m => m.Name == "SetPublicGetterPrivateSetter"));

        var protected_properties = type.GetProperties(BindingFlags.NonPublic | BindingFlags.Instance);
        var prop = protected_properties.SingleOrDefault(m => m.Name == "ProtectedProp");
        Test.AssertNotNull(prop);
        Test.Assert(prop.GetMethod.IsFamily);
        Test.Assert(prop.SetMethod.IsFamily);

        var public_properties = type.GetProperties(BindingFlags.Public | BindingFlags.Instance);
        prop = public_properties.SingleOrDefault(m => m.Name == "PublicGetterPrivateSetter");
        Test.AssertNotNull(prop);
        Test.Assert(prop.GetMethod.IsPublic);
        Test.Assert(prop.SetMethod.IsFamily);
    }
}

class TestStaticInterfaceMembers
{
    public static void test_interface_static_member()
    {
        var iface = typeof(Dummy.ITestIface);
        var properties = iface.GetProperties(BindingFlags.Public | BindingFlags.Instance);
        Test.AssertNull(properties.SingleOrDefault(p => p.Name == "StaticProp"));

        var implementation = typeof(Dummy.TestObject);
        properties = implementation.GetProperties(BindingFlags.Public | BindingFlags.Static);
        Test.AssertNotNull(properties.SingleOrDefault(p => p.Name == "StaticProp"));
    }
}

class TestHiddenClasses
{
    public static void test_hidden_class()
    {
        var obj = new Dummy.TestObject();
        var hidden = obj.HiddenObject;

        Test.AssertEquals(hidden.Name, "hidden_object");
        obj.Dispose();
    }
}

class TestAliasEquality
{
    static Dummy.MyInt a = 4;
    static Dummy.MyInt b = 4;
    static Dummy.MyInt c = 5;

    public static void test_equals()
    {
        Test.AssertEquals(a, b);
        Test.AssertNotEquals(a, c);
    }

    public static void test_equals_different_types()
    {
        Test.Assert(!(a.Equals(new Object())));
    }

    public static void test_equatable()
    {
        Test.Assert(((IEquatable<Dummy.MyInt>)a).Equals(b));
        Test.Assert(!((IEquatable<Dummy.MyInt>)a).Equals(c));
    }

    public static void test_equality_operators()
    {
        Test.Assert(a == b);
        Test.Assert(a != c);
    }

    public static void test_hash_code()
    {
        Test.AssertEquals(a.GetHashCode(), b.GetHashCode());
        Test.AssertNotEquals(a.GetHashCode(), c.GetHashCode());
    }
}

}
