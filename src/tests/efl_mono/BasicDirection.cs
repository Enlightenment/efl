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
using System.Runtime.InteropServices;
using System.Linq;

namespace TestSuite
{

class TestIntDirections
{
    public static void simple_out()
    {
        int original = 1984;
        int received;
        var t = new Dummy.TestObject();

        t.IntOut(original, out received);

        Test.AssertEquals(-original, received);
        t.Dispose();
    }

    /*
    public static void simple_ptr_out()
    {
        int original = 1984;
        int received;
        var t = new Dummy.TestObject();

        t.IntPtrOut(original, out received);

        Test.AssertEquals(original*2, received);
    }
    */
}

}

