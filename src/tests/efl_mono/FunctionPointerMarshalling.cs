/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
        v.Dispose();
        buf.Dispose();
        obj.Dispose();
    }
}
}
