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
}

}
