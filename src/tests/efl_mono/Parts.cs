#define CODE_ANALYSIS

#pragma warning disable 1591

using System;
using System.Diagnostics.CodeAnalysis;

namespace TestSuite {

#if EFL_BETA

[SuppressMessage("Gendarme.Rules.Portability", "DoNotHardcodePathsRule")]
public static class TestParts
{
    public static void basic_part_test()
    {
        var t = new Dummy.PartHolder();
        do_part_test(t);
    }

    private class Child : Dummy.PartHolder
    {
        public Child() : base(null) {}
    }

    public static void inherited_part_test() {
        var t = new Child();
        do_part_test(t);
    }

    private static void do_part_test(Dummy.PartHolder t)
    {
        var p1 = t.PartOne;
        var p2 = t.PartTwo;
        Test.Assert(p1 is Dummy.TestObject);
        Test.AssertEquals("part_one", p1.GetName());
        Test.Assert(p2 is Dummy.TestObject);
        Test.AssertEquals("part_two", p2.GetName());
    }
}

#endif

}
