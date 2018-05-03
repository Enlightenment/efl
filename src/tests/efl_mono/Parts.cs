#define CODE_ANALYSIS

#pragma warning disable 1591

using System;
using System.Diagnostics.CodeAnalysis;

namespace TestSuite {


[SuppressMessage("Gendarme.Rules.Portability", "DoNotHardcodePathsRule")]
public static class TestParts
{
    public static void basic_part_test()
    {
        test.ITesting t = new test.Testing();
        do_part_test(t);
    }

    private class Child : test.TestingInherit
    {
        public Child() : base(null) {}
    }

    public static void inherited_part_test() {
        var t = new Child();
        do_part_test(t);
    }

    private static void do_part_test(test.ITesting t)
    {
        var p1 = t.Part1;
        var p2 = t.Part2;
        Test.Assert(p1 is test.Testing);
        Test.AssertEquals("part1", p1.GetName());
        Test.Assert(p2 is test.Testing);
        Test.AssertEquals("part2", p2.GetName());
    }
}

}
