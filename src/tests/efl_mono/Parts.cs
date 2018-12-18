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
        var t = new Dummy.TestObject();
        do_part_test(t);
    }

    public static void dynamic_parts()
    {
        var t = new DynamicChild();
        var s = t["a"];
    }

    private class DynamicChild : Dummy.TestObject
    {
        public string this[string key]
        {
            get { return ""; }
            //set { dictionary[key] = value == null ? null : value.Trim(); }
        }
    }
    
    private class Child : Dummy.TestObject
    {
        public Child() : base(null) {}
    }

    public static void inherited_part_test() {
        var t = new Child();
        do_part_test(t);
    }

    private static void do_part_test(Dummy.TestObject t)
    {
        var p1 = t.PartOne;
        var p2 = t.PartTwo;
        Test.Assert(p1 is Dummy.TestObject);
        Test.AssertEquals("part_one", p1.GetName());
        Test.Assert(p2 is Dummy.TestObject);
        Test.AssertEquals("part_two", p2.GetName());
    }
}

}
