using System;
using System.Linq;
using System.Collections.Generic;

namespace TestSuite
{

class TestEo
{
    private class Derived : Dummy.TestObject
    {
    }

    public static void return_null_object()
    {
        var testing = new Dummy.TestObject();
        var o1 = testing.ReturnNullObject();
        Test.Assert(o1 == null);
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
           obj.DelEvt += (object sender, EventArgs e) => { delEventCalled = true; };
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
    }

    private static WeakReference CreateCollectableInherited()
    {
        var obj = new MyObject();
        return new WeakReference(obj);
    }

    public static void inherited_collected()
    {
        var wref = CreateCollectableInherited();
        Test.CollectAndIterate();

        Test.AssertNull(wref.Target);
    }
}

class TestEoNames
{
    public static void name_getset()
    {
        var obj = new Dummy.TestObject();

        string name = "Dummy";
        obj.SetName(name);
        Test.AssertEquals(name, obj.GetName());
    }
}

class TestEoParent
{
    public static void basic_parent()
    {
        var parent = new Dummy.TestObject(null);
        var child = new Dummy.TestObject(parent);

        Test.AssertEquals(parent, child.GetParent());

        var parent_retrieved = child.GetParent() as Dummy.TestObject;
        Test.AssertEquals(parent, parent_retrieved);
    }

    public static void parent_inherited_class()
    {
        Dummy.Numberwrapper parent = new Dummy.Numberwrapper(null);
        var child = new Dummy.TestObject(parent);

        Test.AssertEquals(parent, child.GetParent());

        Dummy.Numberwrapper parent_retrieved = child.GetParent() as Dummy.Numberwrapper;
        Test.AssertEquals(parent, parent_retrieved);
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

        Test.AssertEquals(parent, child.GetParent());

        var parent_from_cast = child.GetParent() as Derived;
        Test.AssertEquals(parent, parent_from_cast);
    }
}

class TestKlassMethods
{
    public static void basic_class_method()
    {
        int reference = 0xbeef;
        Dummy.TestObject.SetKlassProp(reference);
        Test.AssertEquals(reference, Dummy.TestObject.GetKlassProp());
    }

    public static void inherited_class_method()
    {
        int reference = 0xdead;
        Dummy.Child.SetKlassProp(reference);
        Test.AssertEquals(reference, Dummy.Child.GetKlassProp());
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
    public static void basic_eo_accessors()
    {
        var obj = new Dummy.TestObject();
        Eina.List<int> lst = new Eina.List<int>();
        lst.Append(4);
        lst.Append(3);
        lst.Append(2);
        lst.Append(5);
        Eina.Accessor<int> acc = obj.CloneAccessor(lst.GetAccessor());

        var zipped = acc.Zip(lst, (first, second) => new Tuple<int, int>(first, second));

        foreach(Tuple<int, int> pair in zipped)
        {
            Test.AssertEquals(pair.Item1, pair.Item2);
        }
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

        obj = new FirstChild();
        Test.AssertEquals(1, obj.receivedValue);
    }
}

class TestCsharpProperties
{
    public static void test_csharp_properties()
    {
        var obj = new Dummy.TestObject();
        var name = "My Name";
        obj.Name = name;

        Test.AssertEquals(name, obj.Name);
    }

    public static void test_getter_only()
    {
        var obj = new Dummy.TestObject();
        Test.Assert(!obj.Invalidating);
    }

    public static void test_setter_only()
    {
        var obj = new Dummy.TestObject();
        int val = -1984;

        obj.SetterOnly = val;
        Test.AssertEquals(val, obj.GetSetterOnly());
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
        Test.Assert(!obj.GetIfaceWasSet());
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
        Test.AssertEquals(provider.GetNumber(), 1999);
    }

    private class ProviderHolder : Dummy.TestObject
    {
        private Dummy.TestObject provider;
        public string ProviderName
        {
            get
            {
                return "MyProvider";
            }
        }

        public ProviderHolder() : base(null)
        {
            this.provider = new Dummy.TestObject(this);
            this.provider.Name = this.ProviderName;
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
        Test.AssertEquals(provider.Name, obj.ProviderName, "Provider name does not match expected");

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
    }
}

}
