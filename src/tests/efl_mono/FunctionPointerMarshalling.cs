using System;
using System.Linq;
using System.Runtime.InteropServices;

namespace TestSuite
{

class TestFunctionPointerMarshalling
{
    public static void func_pointer_marshalling()
    {
        test.ITesting obj = new test.Testing();
        bool called = false;
        eina.Strbuf buf = new eina.Strbuf();
        string argument = "Some String";
        eina.Value v = new eina.Value(eina.ValueType.String);
        v.Set(argument);
        string reference = new string(argument.ToCharArray().Reverse().ToArray());

        obj.CallFormatCb(buf, v, (eina.Strbuf ibuf, eina.Value val) => {
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
