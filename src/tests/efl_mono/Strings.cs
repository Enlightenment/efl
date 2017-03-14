using System;

namespace TestSuite {

class TestStrings
{
    public static void in_string()
    {
        {
            test.Testing obj = new test.TestingConcrete();
            String sent = "in_string";
            String returned = obj.in_string(sent);
            Test.AssertEquals(sent, returned);
        }
        System.GC.Collect();
    }

    public static void in_own_string()
    {
        {
            test.Testing obj = new test.TestingConcrete();
            String sent = "in_own_string";
            String returned = obj.in_own_string(sent);
            Test.AssertEquals(sent, returned);
        }
        System.GC.Collect();
    }

    public static void return_string()
    {
        {
            test.Testing obj = new test.TestingConcrete();
            Test.AssertEquals("string", obj.return_string());
        }
        System.GC.Collect();
    }

    public static void return_own_string()
    {
        {
            test.Testing obj = new test.TestingConcrete();
            Test.AssertEquals("own_string", obj.return_own_string());
        }
        System.GC.Collect();
    }

    public static void out_string()
    {
        {
            String str = String.Empty;
            test.Testing obj = new test.TestingConcrete();
            obj.out_string(out str);
            Test.AssertEquals("out_string", str);
        }
        System.GC.Collect();
    }

    public static void out_own_string()
    {
        {
            String str = String.Empty;
            test.Testing obj = new test.TestingConcrete();
            obj.out_own_string(out str);
            Test.AssertEquals(str.ToString(), "out_own_string");
        }
        System.GC.Collect();
    }

    private class StringReturner : test.TestingInherit
    {
        public String received_in;
        public String received_in_own;
        public StringReturner() : base(null) {
            received_in = String.Empty;
            received_in_own = String.Empty;
        }

        public override String in_string(String str)
        {
            received_in = str;
            return String.Empty;
        }

        public override String in_own_string(String str)
        {
            received_in_own = str;
            return String.Empty;
        }

        public override String return_string()
        {
            return "inherited";
        }

        public override String return_own_string()
        {
            return "own_inherited";
        }

        public override void out_string(out String str)
        {
            str = "out_inherited";
        }

        public override void out_own_string(out System.String str)
        {
            str = "out_own_inherited";
        }
    }

    public static void in_string_from_virtual()
    {
        StringReturner obj = new StringReturner();
        String sent = "in_inherited";
        obj.call_in_string(sent);
        Test.AssertEquals(sent, obj.received_in);
    }

    public static void in_own_string_from_virtual()
    {
        StringReturner obj = new StringReturner();
        String sent = "in_own_inherited";
        obj.call_in_own_string(sent);
        Test.AssertEquals(sent, obj.received_in_own);
    }

    public static void return_string_from_virtual()
    {
        test.Testing obj = new StringReturner();
        // for (int i = 0; i < 1000000; i ++) // Uncomment this to check for memory leaks.
        Test.AssertEquals("inherited", obj.call_return_string());
    }

    public static void return_own_string_from_virtual()
    {
        test.Testing obj = new StringReturner();
        // for (int i = 0; i < 1000000; i ++) // Uncomment this to check for memory leaks.
        Test.AssertEquals("own_inherited", obj.call_return_own_string());
    }

    public static void out_string_from_virtual()
    {
        test.Testing obj = new StringReturner();
        // for (int i = 0; i < 1000000; i ++) // Uncomment this to check for memory leaks.
        Test.AssertEquals("out_inherited", obj.call_out_string());
    }

    public static void out_own_string_from_virtual()
    {
        test.Testing obj = new StringReturner();
        // for (int i = 0; i < 1000000; i ++) // Uncomment this to check for memory leaks.
        Test.AssertEquals("out_own_inherited", obj.call_out_own_string());
    }

}

class TestStringshare
{
    /* public static void in_stringshare() */
    /* { */
    /*     { */
    /*         test.Testing obj = new test.TestingConcrete(); */
    /*         String sent = "in_stringshare"; */
    /*         String returned = obj.in_stringshare(sent); */
    /*         Test.AssertEquals(sent, returned); */
    /*     } */
    /*     System.GC.Collect(); */
    /* } */

    /* public static void in_own_stringshare() */
    /* { */
    /*     { */
    /*         test.Testing obj = new test.TestingConcrete(); */
    /*         String sent = "in_own_stringshare"; */
    /*         String returned = obj.in_own_stringshare(sent); */
    /*         Test.AssertEquals(sent, returned); */
    /*     } */
    /*     System.GC.Collect(); */
    /* } */

    public static void return_stringshare()
    {
        {
            test.Testing obj = new test.TestingConcrete();
            Test.AssertEquals("stringshare", obj.return_stringshare());
        }
        System.GC.Collect();
    }

    public static void return_own_stringshare()
    {
        {
            test.Testing obj = new test.TestingConcrete();
            Test.AssertEquals("own_stringshare", obj.return_own_stringshare());
        }
        System.GC.Collect();
    }

    public static void out_stringshare()
    {
        {
            String str = String.Empty;
            test.Testing obj = new test.TestingConcrete();
            obj.out_stringshare(out str);
            Test.AssertEquals("out_stringshare", str);
        }
        System.GC.Collect();
    }

    public static void out_own_stringshare()
    {
        {
            String str = String.Empty;
            test.Testing obj = new test.TestingConcrete();
            obj.out_own_stringshare(out str);
            Test.AssertEquals(str.ToString(), "out_own_stringshare");
        }
        System.GC.Collect();
    }

    private class StringshareReturner : test.TestingInherit
    {
        public String received_in;
        public String received_in_own;
        public StringshareReturner() : base(null) {
            received_in = String.Empty;
            received_in_own = String.Empty;
        }

        /* public override String in_stringshare(String str) */
        /* { */
        /*     received_in = str; */
        /*     return String.Empty; */
        /* } */

        /* public override String in_own_stringshare(String str) */
        /* { */
        /*     received_in_own = str; */
        /*     return String.Empty; */
        /* } */

        public override String return_stringshare()
        {
            return "inherited";
        }

        public override String return_own_stringshare()
        {
            return "own_inherited";
        }

        public override void out_stringshare(out String str)
        {
            str = "out_inherited";
        }

        public override void out_own_stringshare(out System.String str)
        {
            str = "out_own_inherited";
        }
    }

    /* public static void in_stringshare_from_virtual() */
    /* { */
    /*     StringshareReturner obj = new StringshareReturner(); */
    /*     String sent = "in_inherited"; */
    /*     obj.call_in_stringshare(sent); */
    /*     Test.AssertEquals(sent, obj.received_in); */
    /* } */

    /* public static void in_own_stringshare_from_virtual() */
    /* { */
    /*     StringshareReturner obj = new StringshareReturner(); */
    /*     String sent = "in_own_inherited"; */
    /*     obj.call_in_own_stringshare(sent); */
    /*     Test.AssertEquals(sent, obj.received_in_own); */
    /* } */

    public static void return_stringshare_from_virtual()
    {
        test.Testing obj = new StringshareReturner();
        // for (int i = 0; i < 1000000; i ++) // Uncomment this to check for memory leaks.
        Test.AssertEquals("inherited", obj.call_return_stringshare());
    }

    public static void return_own_stringshare_from_virtual()
    {
        test.Testing obj = new StringshareReturner();
        // for (int i = 0; i < 1000000; i ++) // Uncomment this to check for memory leaks.
        Test.AssertEquals("own_inherited", obj.call_return_own_stringshare());
    }

    public static void out_stringshare_from_virtual()
    {
        test.Testing obj = new StringshareReturner();
        // for (int i = 0; i < 1000000; i ++) // Uncomment this to check for memory leaks.
        Test.AssertEquals("out_inherited", obj.call_out_stringshare());
    }

    public static void out_own_stringshare_from_virtual()
    {
        test.Testing obj = new StringshareReturner();
        // for (int i = 0; i < 1000000; i ++) // Uncomment this to check for memory leaks.
        Test.AssertEquals("out_own_inherited", obj.call_out_own_stringshare());
    }
}

}
