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
        buf.Dispose();
    }

    public static void test_tostring()
    {
        Eina.Strbuf buf = new Eina.Strbuf();
        buf.Append("Hello");
        buf.Append(' ');
        buf.Append("World!");

        Test.AssertEquals("Hello World!", buf.ToString());
        buf.Dispose();
    }

    public static void test_eolian()
    {
        var obj = new Dummy.TestObject();
        Eina.Strbuf buf = new Eina.Strbuf();

        obj.AppendToStrbuf(buf, "Appended");
        obj.AppendToStrbuf(buf, " to buf");

        Test.AssertEquals("Appended to buf", buf.Steal());
        buf.Dispose();
        obj.Dispose();
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
        buf.Dispose();
        obj.Dispose();
    }
}
} // namespace TestSuite
