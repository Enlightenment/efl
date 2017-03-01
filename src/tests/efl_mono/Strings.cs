using System;

namespace TestSuite {

class TestStrings
{
    public static void return_string()
    {
        {
            test.Testing obj = new test.TestingConcrete();
            Test.Assert(obj.return_string().Equals("string"));
        }
        System.GC.Collect();
    }

    public static void return_own_string()
    {
        {
            test.Testing obj = new test.TestingConcrete();
            Test.Assert(obj.return_own_string().Equals("own_string"));
        }
        System.GC.Collect();
    }

    public static void out_string()
    {
        {
            String str = String.Empty;
            test.Testing obj = new test.TestingConcrete();
            obj.out_string(out str);
            Test.Assert(str.Equals("out_string"));
        }
        System.GC.Collect();
    }

    public static void out_own_string()
    {
        {
            String str = String.Empty;
            test.Testing obj = new test.TestingConcrete();
            obj.out_string(out str);
            Test.Assert(str.Equals("out_own_string"));
        }
        System.GC.Collect();
    }

    private class StringReturner : test.TestingInherit
    {
        public StringReturner() : base(null) {
        }
        public override String return_string()
        {
            return "inherited";
        }
    }

    public static void return_string_to_c()
    {
        test.Testing obj = new StringReturner();
        // for (int i = 0; i < 1000000; i ++) // Uncomment this to check for memory leaks.
        Test.Assert(obj.call_return_string().Equals("inherited"));
    }
}

}
