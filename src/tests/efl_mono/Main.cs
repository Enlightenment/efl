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
using System.Runtime.CompilerServices;
using System.Reflection;
using System.Linq;

class TestMain
{
    static Type[] GetTestCases(String name="")
    {
        return Assembly.GetExecutingAssembly().GetTypes().Where(t => String.Equals(t.Namespace, "TestSuite", StringComparison.Ordinal) &&
                                                                t.Name.StartsWith("Test") &&
                                                                t.Name.Contains(name)).ToArray();
    }

    static int Main(string[] args)
    {
        if (Environment.GetEnvironmentVariable("ELM_ENGINE") == null)
            Environment.SetEnvironmentVariable("ELM_ENGINE", "buffer");

        Efl.All.Init(Efl.Csharp.Components.Ui);

        bool pass = true;

        String ckRunSuite = Environment.GetEnvironmentVariable("CK_RUN_SUITE");
        String ckRunCase = Environment.GetEnvironmentVariable("CK_RUN_CASE");

        if (ckRunSuite != null && !ckRunSuite.Equals("mono"))
            return 0;

        if (ckRunCase == null)
            ckRunCase = String.Empty;

        Console.WriteLine("[ START SUITE ] " + ckRunSuite);
        var cases= GetTestCases(ckRunCase);
        foreach (var testCase in cases)
        {
            var localTestCases = testCase.GetMethods(BindingFlags.Static | BindingFlags.Public);

            var setUp = Array.Find(localTestCases, m => String.Equals(m.Name, "SetUp", StringComparison.Ordinal));
            var tearDown = Array.Find(localTestCases, m => String.Equals(m.Name, "TearDown", StringComparison.Ordinal));

            foreach (var localTestCase in localTestCases)
            {
                if (localTestCase == setUp || localTestCase == tearDown)
                    continue;

                // Cleanup garbage collector and job queue
                Test.CollectAndIterate(1);

                Console.WriteLine("[ RUN         ] " + testCase.Name + "." + localTestCase.Name);
                bool caseResult = true;

                if (setUp != null)
                {
                    try
                    {
                        setUp.Invoke(null, null);
                    }
                    catch (Exception e)
                    {
                        pass = false;
                        caseResult = false;
                        Console.WriteLine("[ ERROR       ] SetUp fail: " + e.InnerException.ToString());
                    }
                }

                if (caseResult)
                {
                    try
                    {
                        localTestCase.Invoke(null, null);
                    }
                    catch (Exception e)
                    {
                        pass = false;
                        caseResult = false;
                        Console.WriteLine("[ ERROR       ] " + e.InnerException.ToString());
                    }
                }

                if (caseResult && tearDown != null)
                {
                    try
                    {
                        tearDown.Invoke(null, null);
                    }
                    catch (Exception e)
                    {
                        pass = false;
                        caseResult = false;
                        Console.WriteLine("[ ERROR       ] TearDown failed: " + e.InnerException.ToString());
                    }
                }

                Console.WriteLine("[        " + (caseResult ? "PASS" : "FAIL") + " ] " + testCase.Name + "." + localTestCase.Name);
            }
        }
        Console.WriteLine("[   END SUITE ] " + ckRunSuite);

        Efl.All.Shutdown();

        if (!pass)
          return -1;

        return 0;
    }
}
