using System;

namespace TestSuite {

class TestHash
{

    public static void test_del_value()
    {
        var hash = new Eina.Hash<int, int>();
        Test.AssertEquals(hash.Count, 0);
        hash.Add(0, 1);
        Test.Assert(hash.DelByValue(1));
        Test.AssertEquals(hash.Count, 0);

        hash.Add(0, 1);
        hash.Add(1, 100);
        hash.Add(2, 101);

        Test.Assert(hash.DelByValue(100));
        Test.AssertEquals(hash.Count, 2);

        Test.Assert(!hash.DelByValue(200));
        hash.Dispose();
    }

    public static void test_del_value_string()
    {
        var hash = new Eina.Hash<int, string>();
        Test.AssertEquals(hash.Count, 0);
        hash.Add(0, "E F L");
        Test.Assert(hash.DelByValue("E F L"));
        Test.AssertEquals(hash.Count, 0);

        hash.Add(0, "Eina");
        hash.Add(1, "Eo");
        hash.Add(2, "Ecore");

        Test.Assert(hash.DelByValue("Ecore"));
        Test.AssertEquals(hash.Count, 2);

        Test.Assert(!hash.DelByValue("Elementary"));
        hash.Dispose();
    }

    public static void test_del_key()
    {
        var hash = new Eina.Hash<int, int>();
        hash.Add(0, 1);
        hash.Add(1, 100);
        hash.Add(2, 101);

        hash.DelByKey(1);
        Test.AssertEquals(hash.Count, 2);
        hash.Dispose();
    }
}
} // namespace TestSuite
