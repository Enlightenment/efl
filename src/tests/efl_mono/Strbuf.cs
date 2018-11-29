using System;

namespace TestSuite {

class TestStrBuf
{
    public static void test_steal()
    {
        Eina.Strbuf buf = new Eina.Strbuf();

        buf.Append("Here's");
        buf.Append(' ');
        buf.Append("Johnny!");

        Test.AssertEquals("Here's Jonnny!".Length, buf.Length);
        Test.AssertEquals("Here's Johnny!", buf.Steal());
    }

    public static void test_eolian()
    {
        var obj = new Dummy.TestObject();
        Eina.Strbuf buf = new Eina.Strbuf();

        obj.AppendToStrbuf(buf, "Appended");
        obj.AppendToStrbuf(buf, " to buf");

        Test.AssertEquals("Appended to buf", buf.Steal());
    }

    private class Appender : Dummy.TestObject
    {
        public bool called;
        public Appender() : base(null)
        {
            called = false;
        }

        public override void AppendToStrbuf(Eina.Strbuf buf, string str)
        {
            Eina.Log.Error("Virtual wrapper called");
            called = true;
            buf.Append(str);
        }
    }

    public static void test_virtual_eolian()
    {
        Appender obj = new Appender();
        Eina.Strbuf buf = new Eina.Strbuf();

        obj.CallAppendToStrbuf(buf, "Is");
        obj.CallAppendToStrbuf(buf, " this");
        obj.CallAppendToStrbuf(buf, " virtual?");

        Test.Assert(obj.called);
        Test.AssertEquals("Is this virtual?", buf.Steal());
    }
}
} // namespace TestSuite
