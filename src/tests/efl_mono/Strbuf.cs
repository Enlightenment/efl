using System;

namespace TestSuite {

class TestStrBuf
{
    public static void test_steal()
    {
        eina.Strbuf buf = new eina.Strbuf();

        buf.Append("Here's");
        buf.Append(' ');
        buf.Append("Johnny!");

        Test.AssertEquals("Here's Jonnny!".Length, buf.Length);
        Test.AssertEquals("Here's Johnny!", buf.Steal());
    }

    public static void test_eolian()
    {
        test.ITesting obj = new test.Testing();
        eina.Strbuf buf = new eina.Strbuf();

        obj.AppendToStrbuf(buf, "Appended");
        obj.AppendToStrbuf(buf, " to buf");

        Test.AssertEquals("Appended to buf", buf.Steal());
    }

    private class Appender : test.TestingInherit
    {
        public bool called;
        public Appender() : base(null)
        {
            called = false;
        }

        public override void AppendToStrbuf(eina.Strbuf buf, string str)
        {
            eina.Log.Error("Virtual wrapper called");
            called = true;
            buf.Append(str);
        }
    }

    public static void test_virtual_eolian()
    {
        Appender obj = new Appender();
        eina.Strbuf buf = new eina.Strbuf();

        obj.CallAppendToStrbuf(buf, "Is");
        obj.CallAppendToStrbuf(buf, " this");
        obj.CallAppendToStrbuf(buf, " virtual?");

        Test.Assert(obj.called);
        Test.AssertEquals("Is this virtual?", buf.Steal());
    }
}
} // namespace TestSuite
