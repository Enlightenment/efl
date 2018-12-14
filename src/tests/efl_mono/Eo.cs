using System;
using System.Linq;

namespace TestSuite
{

class TestEo
{
    private class Derived : Dummy.TestObject
    {
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


class MyLoop : Efl.Loop
{
    public MyLoop() : base(null) { }
}

class TestEoInherit
{
    public static void instantiate_inherited()
    {
        Efl.Loop loop = new MyLoop();
        Test.Assert(loop.NativeHandle != System.IntPtr.Zero);
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

class TestEoConstructingMethods
{
    public static void constructing_method()
    {
        bool called = false;
        string name = "Test object";
        var obj = new Dummy.TestObject(null, (Dummy.TestObject a) => {
                called = true;
                Console.WriteLine("callback: obj NativeHandle: {0:x}", a.NativeHandle);
                a.SetName(name);
            });

        Test.Assert(called);
        Test.AssertEquals(name, obj.GetName());
    }

    private class Derived : Dummy.TestObject
    {
        public Derived(Dummy.TestObject parent = null,
                       Dummy.TestObject.ConstructingMethod cb = null) : base(parent, cb) {
        }
    }

    public static void constructing_method_inherit()
    {
        bool called = false;
        string name = "Another test object";
        Derived obj = new Derived(null, (Dummy.TestObject a) => {
                called = true;
                a.SetComment(name);
            });

        Test.Assert(called);
        Test.AssertEquals(name, obj.GetComment());
    }
}

class TestEoParent
{
    public static void basic_parent()
    {
        var parent = new Dummy.TestObject(null);
        var child = new Dummy.TestObject(parent);

        Test.AssertEquals(parent, child.GetParent());

        var parent_retrieved = Dummy.TestObject.static_cast(child.GetParent());
        Test.AssertEquals(parent, parent_retrieved);
    }

    public static void parent_inherited_class()
    {
        Dummy.Numberwrapper parent = new Dummy.Numberwrapper(null);
        var child = new Dummy.TestObject(parent);

        Test.AssertEquals(parent, child.GetParent());

        Dummy.Numberwrapper parent_retrieved = Dummy.Numberwrapper.static_cast(child.GetParent());
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

        var parent_from_cast = Dummy.TestObject.static_cast(child.GetParent());
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
        Dummy.TestIface iface = new Dummy.TestObject();
        iface.IfaceProp = val;
        Test.AssertEquals(val, iface.IfaceProp);
    }
}

}
