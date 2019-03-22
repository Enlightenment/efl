using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Reflection;
using System.Linq;

class TestMain : Efl.Csharp.Application
{
    static Type[] GetTestCases(String name="")
    {
        return Assembly.GetExecutingAssembly().GetTypes().Where(t => String.Equals(t.Namespace, "TestSuite", StringComparison.Ordinal) &&
                                                                t.Name.StartsWith("Test") &&
                                                                t.Name.Contains(name)).ToArray();
    }

    // Empty as we do not actually depend on it to run the tests.
    protected override void OnInitialize(Eina.Array<System.String> args)
    {
    }

    static int Main(string[] args)
    {
        var app = new TestMain();
        app.Setup();

        bool pass = true;

        String ckRunSuite = Environment.GetEnvironmentVariable("CK_RUN_SUITE");
        String ckRunCase = Environment.GetEnvironmentVariable("CK_RUN_CASE");

        if (ckRunSuite != null && !ckRunSuite.Equals("mono"))
            return 0;

        if (ckRunCase == null)
            ckRunCase = String.Empty;

        Console.WriteLine("[ START SUITE ] " + ckRunSuite);
        var cases= GetTestCases(ckRunCase);
        foreach(var testCase in cases)
        {
            var localTestCases = testCase.GetMethods(BindingFlags.Static | BindingFlags.Public);

            var setUp = Array.Find(localTestCases, m => String.Equals(m.Name, "SetUp", StringComparison.Ordinal));
            var tearDown = Array.Find(localTestCases, m => String.Equals(m.Name, "TearDown", StringComparison.Ordinal));

            foreach(var localTestCase in localTestCases)
            {
                if (localTestCase == setUp || localTestCase == tearDown)
                    continue;

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

        Efl.Csharp.Application.Shutdown();

        if (!pass)
          return -1;

        return 0;
    }
}
