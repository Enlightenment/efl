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
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Diagnostics.CodeAnalysis;

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
            Console.WriteLine("finalizer called for parent");
        }

        protected override void Dispose(bool disposing)
        {
            Console.WriteLine("Dispose parent");
            base.Dispose(disposing);
        }
    }

    internal class Inherit3Child : Dummy.TestObject
    {
        //Inherit3Parent parent;
        public Inherit3Child(Inherit3Parent parent) : base(parent)
        {
            // WARNING: Uncommenting the line below causes the parent-child cycle to leak.
            // The GC won't be able to collect it.
            // this.parent = parent;
        }

        ~Inherit3Child()
        {
            Console.WriteLine("finalizer called for child");
        }

        protected override void Dispose(bool disposing)
        {
            /* parent.childDisposed = true; */
            Console.WriteLine("Dispose parent");
            base.Dispose(disposing);
        }
    }

    public static void test_inherit_from_regular_class()
    {
        var obj = new Inherit1();
        int i = Dummy.InheritHelper.ReceiveDummyAndCallIntOut(obj);
        Test.AssertEquals (50, i);
        obj.Dispose();
    }

    public static void test_inherit_from_iface()
    {
        var obj = new Inherit2();
        int i = Dummy.InheritHelper.ReceiveDummyAndCallIntOut(obj);
        Test.AssertEquals (50, i);
        string s = Dummy.InheritHelper.ReceiveDummyAndCallInStringshare(obj);
        Test.AssertEquals ("Hello World", s);
        obj.Dispose();
    }

    [SuppressMessage("Microsoft.Reliability", "CA2000:DisposeObjectBeforeLosingScope", Justification = "It is expected to lose scope.")]
    private static void CreateAndCheckInheritedObjects(out WeakReference parentWRef, out WeakReference childWRef)
    {
        var parent = new Inherit3Parent();
        var child = new Inherit3Child(parent);

        parentWRef = new WeakReference(parent);
        childWRef = new WeakReference(child);

        child = null;

        System.GC.Collect(System.GC.MaxGeneration, GCCollectionMode.Forced, true, true);
        System.GC.WaitForPendingFinalizers();
        Efl.App.AppMain.Iterate();

        child = (Inherit3Child) childWRef.Target;

        Test.AssertNotNull(parent);
        Test.AssertNotNull(child);
        Test.AssertEquals(false, parent.disposed);
        Test.AssertEquals(false, parent.childDisposed);

        child.Dispose();
        parent.Dispose();
    }

    public static void test_inherit_lifetime()
    {
        WeakReference parentWRef;
        WeakReference childWRef;

        CreateAndCheckInheritedObjects(out parentWRef, out childWRef);

        // We need some extra iterations of the main loop to allow the async callbacks
        // registered from the Dispose method to the main loop to run.
        Test.CollectAndIterate(10, 10);

        var parent = (Dummy.TestObject) parentWRef.Target;
        var child = (Dummy.TestObject) childWRef.Target;

        Test.AssertNull(parent);
        Test.AssertNull(child);
    }
}

}
