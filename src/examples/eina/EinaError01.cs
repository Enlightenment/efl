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

public class ExampleEinaError01
{
    private static bool RegisteredErrors = false;
    private static Eina.Error MyErrorNegative;
    private static Eina.Error MyErrorNull;

    private static void testFunc(int n, string s)
    {
        if (!RegisteredErrors)
        {
            MyErrorNegative = Eina.Error.Register("Negative number");
            MyErrorNull = Eina.Error.Register("NULL pointer");
            RegisteredErrors = true;
        }

        if (n < 0)
        {
            Eina.Error.Set(MyErrorNegative);
            return;
        }

        if (s == null)
        {
            Eina.Error.Set(MyErrorNull);
            return;
        }
    }

    public static void Main()
    {
        Eina.Config.Init();
        Efl.Eo.Config.Init();

        // Handling Eina_Error with exception
        try
        {
            testFunc(-1, "abc");
            Eina.Error.RaiseIfUnhandledException();
        }
        catch(Efl.EflException e)
        {
            WriteLine($"Caught error: {e.Message}");
        }

        // Handling Eina_Error directly
        testFunc(42, null);
        Eina.Error err = Eina.Error.Get();
        if (err != 0)
        {
            WriteLine($"Error set: {err.Message}");
        }
        Eina.Error.Clear();

        // No error set
        try
        {
            testFunc(42, "abc");

            Eina.Error.RaiseIfUnhandledException();

            err = Eina.Error.Get();
            WriteLine($"Really no error? {err == Eina.Error.NO_ERROR}.");
        }
        catch
        {
            WriteLine("Unspected error!!!");
        }

        WriteLine("No error message is empty string: \"{0}\"", Eina.Error.NO_ERROR.Message);
        WriteLine("No error message is empty string: \"{0}\"", Eina.Error.MsgGet(0));
    }
}

