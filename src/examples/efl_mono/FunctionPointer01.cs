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
using static System.Console;

public class ExampleFunctionPointer01
{
    private static bool static_called = false;

    private static int twiceCb(int n)
    {
        static_called = true;
        return n * 2;
    }

    public static void Main()
    {
        Eina.Config.Init();
        Efl.Eo.Config.Init();

        var obj = new Example.Numberwrapper();

        // Set internal value
        obj.SetNumber(12);

        // With static method
        obj.SetNumberCallback(twiceCb);

        var ret = obj.CallCallback();

        WriteLine($"Callback called? {static_called}.");
        WriteLine($"Returned value: {ret}.\n");

        // With lambda
        bool lamda_called = false;

        obj.SetNumberCallback(n => {
            lamda_called = true;
            return n * 3;
        });

        ret = obj.CallCallback();

        WriteLine($"Lambda called? {lamda_called}.");
        WriteLine($"Returned value: {ret}.\n");
    }
}

