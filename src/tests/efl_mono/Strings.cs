using System;

namespace TestSuite {

class TestStrings
{
    /* The managed call is still owner of the sent string */
    public static void in_string()
    {
        {
            var obj = new Dummy.TestObject();
            String sent = "in_string";
            String returned = obj.InString(sent);
            Test.AssertEquals(sent, returned);
        }
        System.GC.Collect();
    }

    /* The managed call must not keep ownership of the C string after the
     * call */
    public static void in_own_string()
    {
        {
            var obj = new Dummy.TestObject();
            String sent = "in_own_string";
            String returned = obj.InOwnString(sent);
            Test.AssertEquals(sent, returned);
        }
        System.GC.Collect();
    }

    /* The managed call must not take ownership of the returned string */
    public static void return_string()
    {
        {
            var obj = new Dummy.TestObject();
            Test.AssertEquals("string", obj.ReturnString());
        }
        System.GC.Collect();
    }

    /* The managed call is free to own the returned string */
    public static void return_own_string()
    {
        {
            var obj = new Dummy.TestObject();
            Test.AssertEquals("own_string", obj.ReturnOwnString());
        }
        System.GC.Collect();
    }

    /* The managed call is *not* the owner of the string put in the out argument */
    public static void out_string()
    {
        {
            String str = String.Empty;
            var obj = new Dummy.TestObject();
            obj.OutString(out str);
            Test.AssertEquals("out_string", str);
        }
        System.GC.Collect();
    }

    /* The managed call is the owner of the string in the out parameter */
    public static void out_own_string()
    {
        {
            String str = String.Empty;
            var obj = new Dummy.TestObject();
            obj.OutOwnString(out str);
            Test.AssertEquals(str.ToString(), "out_own_string");
        }
        System.GC.Collect();
    }

    private class StringReturner : Dummy.TestObject
    {
        public String received_in;
        public String received_in_own;
        public StringReturner() : base(null) {
            received_in = String.Empty;
            received_in_own = String.Empty;
        }

        public override String InString(String str)
        {
            received_in = str;
            return String.Empty;
        }

        public override String InOwnString(String str)
        {
            /* Console.WriteLine("Called my own virtual"); */
            received_in_own = str;
            return String.Empty;
        }

        public override String ReturnString()
        {
            return "inherited";
        }

        public override String ReturnOwnString()
        {
            return "own_inherited";
        }

        public override void OutString(out String str)
        {
            str = "out_inherited";
        }

        public override void OutOwnString(out System.String str)
        {
            str = "out_own_inherited";
        }
    }

    /* The managed wrapper must not take ownership of the in parameter */
    public static void in_string_from_virtual()
    {
        StringReturner obj = new StringReturner();
        /* for (int i = 0; i < 10000; i++) { */
            String sent = "in_inherited";
            obj.CallInString(sent);
            Test.AssertEquals(sent, obj.received_in);
        /* } */
        System.GC.Collect();
    }

    /* The managed wrapper should take ownership of the in parameter */
    public static void in_own_string_from_virtual()
    {
        StringReturner obj = new StringReturner();
        /* for (int i = 0; i < 10000; i++) { */
            String sent = "in_own_inherited";
            obj.CallInOwnString(sent);
            Test.AssertEquals(sent, obj.received_in_own);
        /* } */
        System.GC.Collect();
    }

    /* The managed wrapper still owns the returned C string. We need to cache it until
     * some time in the future */
    public static void return_string_from_virtual()
    {
        var obj = new StringReturner();
        /* for (int i = 0; i < 10000; i ++) // Uncomment this to check for memory leaks. */
        Test.AssertEquals("inherited", obj.CallReturnString());
        System.GC.Collect();
    }

    /* The managed wrapper must surrender the ownership to the C after the virtual call. */
    public static void return_own_string_from_virtual()
    {
        var obj = new StringReturner();
        /* for (int i = 0; i < 10000; i ++) // Uncomment this to check for memory leaks. */
        Test.AssertEquals("own_inherited", obj.CallReturnOwnString());
        System.GC.Collect();
    }

    /* The managed wrapper still owns the C string after the call. Like return without own, we may
     * need to cache it until some time in the future. */
    public static void out_string_from_virtual()
    {
        var obj = new StringReturner();
        /* for (int i = 0; i < 10000; i ++) // Uncomment this to check for memory leaks. */
        Test.AssertEquals("out_inherited", obj.CallOutString());
        System.GC.Collect();
    }

    /* The managed wrapper gives C the ownership of the filled out parameter */
    public static void out_own_string_from_virtual()
    {
        var obj = new StringReturner();
        /* for (int i = 0; i < 10000; i ++) // Uncomment this to check for memory leaks. */
        Test.AssertEquals("out_own_inherited", obj.CallOutOwnString());
        System.GC.Collect();
    }

}

class TestStringshare
{
    public static void in_stringshare()
    {
        {
            var obj = new Dummy.TestObject();
            String sent = "in_stringshare";
            String returned = obj.InStringshare(sent);
            Test.AssertEquals(sent, returned);
        }
        System.GC.Collect();
    }

    public static void in_own_stringshare()
    {
        {
            var obj = new Dummy.TestObject();
            String sent = "in_own_stringshare";
            String returned = obj.InOwnStringshare(sent);
            Test.AssertEquals(sent, returned);
        }
        System.GC.Collect();
    }

    public static void return_stringshare()
    {
        {
            var obj = new Dummy.TestObject();
            Test.AssertEquals("stringshare", obj.ReturnStringshare());
        }
        System.GC.Collect();
    }

    public static void return_own_stringshare()
    {
        {
            var obj = new Dummy.TestObject();
            Test.AssertEquals("own_stringshare", obj.ReturnOwnStringshare());
        }
        System.GC.Collect();
    }

    public static void out_stringshare()
    {
        {
            String str = String.Empty;
            var obj = new Dummy.TestObject();
            obj.OutStringshare(out str);
            Test.AssertEquals("out_stringshare", str);
        }
        System.GC.Collect();
    }

    public static void out_own_stringshare()
    {
        {
            String str = String.Empty;
            var obj = new Dummy.TestObject();
            obj.OutOwnStringshare(out str);
            Test.AssertEquals(str.ToString(), "out_own_stringshare");
        }
        System.GC.Collect();
    }

    private class StringshareReturner : Dummy.TestObject
    {
        public String received_in;
        public String received_in_own;
        public StringshareReturner() : base(null) {
            received_in = String.Empty;
            received_in_own = String.Empty;
        }

        public override String InStringshare(String str)
        {
            received_in = str;
            return String.Empty;
        }

        public override String InOwnStringshare(String str)
        {
            received_in_own = str;
            return String.Empty;
        }

        public override String ReturnStringshare()
        {
            return "inherited";
        }

        public override String ReturnOwnStringshare()
        {
            return "own_inherited";
        }

        public override void OutStringshare(out String str)
        {
            str = "out_inherited";
        }

        public override void OutOwnStringshare(out System.String str)
        {
            str = "out_own_inherited";
        }
    }

    public static void in_stringshare_from_virtual()
    {
        StringshareReturner obj = new StringshareReturner();
        String sent = "in_inherited";
        obj.CallInStringshare(sent);
        Test.AssertEquals(sent, obj.received_in);
    }

    public static void in_own_stringshare_from_virtual()
    {
        StringshareReturner obj = new StringshareReturner();
        String sent = "in_own_inherited";
        obj.CallInOwnStringshare(sent);
        Test.AssertEquals(sent, obj.received_in_own);
    }

    public static void return_stringshare_from_virtual()
    {
        var obj = new StringshareReturner();
        // for (int i = 0; i < 1000000; i ++) // Uncomment this to check for memory leaks.
        Test.AssertEquals("inherited", obj.CallReturnStringshare());
    }

    public static void return_own_stringshare_from_virtual()
    {
        var obj = new StringshareReturner();
        // for (int i = 0; i < 1000000; i ++) // Uncomment this to check for memory leaks.
        Test.AssertEquals("own_inherited", obj.CallReturnOwnStringshare());
    }

    public static void out_stringshare_from_virtual()
    {
        var obj = new StringshareReturner();
        // for (int i = 0; i < 1000000; i ++) // Uncomment this to check for memory leaks.
        Test.AssertEquals("out_inherited", obj.CallOutStringshare());
    }

    public static void out_own_stringshare_from_virtual()
    {
        var obj = new StringshareReturner();
        // for (int i = 0; i < 1000000; i ++) // Uncomment this to check for memory leaks.
        Test.AssertEquals("out_own_inherited", obj.CallOutOwnStringshare());
    }
}

}
