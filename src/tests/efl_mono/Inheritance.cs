using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

using EinaTestData;
using static EinaTestData.BaseData;

namespace TestSuite
{

class TestInheritance
{
    internal class Inherit1 : Dummy.TestObject
    {
        override public void IntOut (int x, out int y)
        {
            y = 10*x;
        }
    }

    internal class Inherit2 : Dummy.TestObject, Dummy.IInheritIface
    {
        override public void IntOut (int x, out int y)
        {
            Console.WriteLine("IntOut");
            y = 10*x;
        }

        public string StringshareTest (string i)
        {
            Console.WriteLine("StringshareTest");
            return "Hello World";
        }
    }

    internal class Inherit3Parent : Dummy.TestObject
    {
        public bool disposed = false;
        public bool childDisposed = false;

        ~Inherit3Parent()
        {
            Console.WriteLine ("finalizer called for parent");
        }
        
        protected override void Dispose (bool disposing)
        {
            Console.WriteLine ("Dispose parent");
            base.Dispose(disposing);
        }
    }

    internal class Inherit3Child : Dummy.TestObject
    {
        Inherit3Parent parent;
        public Inherit3Child (Inherit3Parent parent) : base (parent)
        {
            this.parent = parent;
        }

        ~Inherit3Child()
        {
            Console.WriteLine ("finalizer called for child");
        }

        protected override void Dispose (bool disposing)
        {
            parent.childDisposed = true;
            Console.WriteLine ("Dispose parent");
            base.Dispose(disposing);
        }
    }
    
    public static void test_inherit_from_regular_class()
    {
        var obj = new Inherit1();
        int i = Dummy.InheritHelper.ReceiveDummyAndCallIntOut(obj);
        Test.AssertEquals (50, i);
    }

    public static void test_inherit_from_iface()
    {
        var obj = new Inherit2();
        int i = Dummy.InheritHelper.ReceiveDummyAndCallIntOut(obj);
        Test.AssertEquals (50, i);
        string s = Dummy.InheritHelper.ReceiveDummyAndCallInStringshare(obj);
        Test.AssertEquals ("Hello World", s);
    }

    public static void test_inherit_lifetime()
    {
        WeakReference parent_wref;
        WeakReference child_wref;
        {
            var parent = new Inherit3Parent();
            var child = new Inherit3Child(parent);

            parent_wref = new WeakReference (parent);
            child_wref = new WeakReference (child);

            Console.WriteLine ("Parent has {0} refs", Efl.Eo.Globals.efl_ref_count (parent.NativeHandle));
            Console.WriteLine ("Child has {0} refs", Efl.Eo.Globals.efl_ref_count (child.NativeHandle));
            
            child = null;
        
            System.GC.Collect(System.GC.MaxGeneration, GCCollectionMode.Forced, true, true);
            System.GC.WaitForPendingFinalizers();

            child = (Inherit3Child)child_wref.Target;
        
            Test.AssertNotEquals (parent, null);
            Test.AssertNotEquals (child, null);
            Test.AssertEquals (parent.disposed, false);
            Test.AssertEquals (parent.childDisposed, false);

            Console.WriteLine ("Parent has {0} refs", Efl.Eo.Globals.efl_ref_count (parent.NativeHandle));
            Console.WriteLine ("Child has {0} refs", Efl.Eo.Globals.efl_ref_count (child.NativeHandle));
        }

        System.GC.Collect(System.GC.MaxGeneration, GCCollectionMode.Forced, true, true);
        System.GC.WaitForPendingFinalizers();

        Inherit3Parent parent2 = (Inherit3Parent)parent_wref.Target;
        Inherit3Child child2 = (Inherit3Child)child_wref.Target;

        Test.AssertEquals (parent2, null);
        Test.AssertEquals (child2, null);
    }
}

}
