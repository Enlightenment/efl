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

public class ExampleEinaArray01
{
    public static void Main()
    {
        Eina.Config.Init();

        var strings = new string[]{
            "helo", "hera", "starbuck", "kat", "boomer",
            "hotdog", "longshot", "jammer", "crashdown", "hardball",
            "duck", "racetrack", "apolo", "husker", "freaker",
            "skulls", "bulldog", "flat top", "hammerhead", "gonzo"
        };

        var array = new Eina.Array<string>(20U);

        // Push new elements
        foreach (string s in strings)
        {
            WriteLine("push: " + s);
            array.Push(s);
        }

        // Check count
        WriteLine("array count: " + array.Count());

        // Iterate over the array
        int idx = 0;
        foreach (string s in array)
        {
            WriteLine($"at[{idx}]: {s}");
            ++idx;
        }

        // Remove one by one
        while (array.Length != 0)
            WriteLine("pop: " + array.Pop());
    }
}

