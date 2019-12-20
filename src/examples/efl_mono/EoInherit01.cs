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

class PlusTenNumberWrapper : Example.Numberwrapper
{
    public PlusTenNumberWrapper(Efl.Object parent = null)
        : base(parent)
    {}

    public bool derivedCalled = false;

    override public void SetNumber(int n)
    {
        // Call native EFL method
        base.SetNumber(n + 10);
        derivedCalled = true;
    }
}

public class ExampleEoInherit01
{
    public static void Main()
    {
        Eina.Config.Init();
        Efl.Eo.Config.Init();

        var inheritObj = new PlusTenNumberWrapper();

        WriteLine("## Using inherit object ##\n");

        int given = 111;

        // Call the C# override from the C method
        inheritObj.CallNumberSet(given);

        WriteLine($"Override successfully called? {inheritObj.derivedCalled}!\n");

        // Call C function from C# object
        int stored = inheritObj.GetNumber();

        WriteLine($"Given value: {given}");
        WriteLine($"Stored value: {stored}\n");

        // Call C# override directly
        given = 333;
        inheritObj.SetNumber(given);

        stored = inheritObj.GetNumber();

        WriteLine($"Given value: {given}");
        WriteLine($"Stored value: {stored}\n");

        WriteLine("## Using original object ##\n");

        // Check original EFL object
        var origObj = new Example.Numberwrapper();
        given = 111;
        origObj.SetNumber(given);
        stored = origObj.GetNumber();

        WriteLine($"Given value: {given}");
        WriteLine($"Stored value: {stored}\n");
    }
}

