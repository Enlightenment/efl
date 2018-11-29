using System;
using System.Runtime.InteropServices;
using System.Linq;

namespace TestSuite
{

class TestIntDirections
{
    public static void simple_out()
    {
        int original = 1984;
        int received;
        var t = new Dummy.TestObject();

        t.IntOut(original, out received);

        Test.AssertEquals(-original, received);
    }

    public static void simple_ptr_out()
    {
        int original = 1984;
        int received;
        var t = new Dummy.TestObject();

        t.IntPtrOut(original, out received);

        Test.AssertEquals(original*2, received);
    }
}

}

