using System;
using System.Linq;
using System.Runtime.InteropServices;

namespace TestSuite
{

class TestFunctionPointerMarshalling
{
    public static void func_pointer_marshalling()
    {
        var obj = new Dummy.TestObject();
        bool called = false;
        Eina.Strbuf buf = new Eina.Strbuf();
        string argument = "Some String";
        Eina.Value v = new Eina.Value(Eina.ValueType.String);
        v.Set(argument);
        string reference = new string(argument.ToCharArray().Reverse().ToArray());

        obj.CallFormatCb(buf, v, (Eina.Strbuf ibuf, Eina.Value val) => {
            called = true;
            string str = null;
            val.Get(out str);
            buf.Append(new string(str.ToCharArray().Reverse().ToArray()));
        });

        Test.Assert(called, "Callback was not called");
        Test.AssertEquals(reference, buf.Steal());
    }
}
}
