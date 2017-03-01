using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Reflection;
using System.Linq;

public class Test
{
    public static void Assert(bool res, String msg = "Assertion failed",
                              [CallerLineNumber] int line = 0,
                              [CallerFilePath] string file = null,
                              [CallerMemberName] string member = null)
    {
        if (!res)
            throw new Exception($"Assertion failed: {file}:{line} ({member}) {msg}");
    }
}

class TestMain
{
    [DllImport("eo")] static extern void efl_object_init();
    [DllImport("ecore")] static extern void ecore_init();
    [DllImport("evas")] static extern void evas_init();

    static Type[] GetTestSuites()
    {
        return Assembly.GetExecutingAssembly().GetTypes().Where(t => String.Equals(t.Namespace, "TestSuite", StringComparison.Ordinal) &&
                                                                t.Name.StartsWith("Test")).ToArray();
    }

    static int Main(string[] args)
    {
        eina.Log.Init();
        efl_object_init();
        ecore_init();
        evas_init();

        bool pass = true;

        var suites = GetTestSuites();
        foreach(var suite in suites)
        {
            var testCases = suite.GetMethods(BindingFlags.Static | BindingFlags.Public);
            Console.WriteLine("[ START SUITE ] " + suite.Name);
            foreach(var testCase in testCases)
            {
                Console.WriteLine("[ RUN         ] " + suite.Name + "." + testCase.Name);
                bool caseResult = true;
                try
                {
                    testCase.Invoke(null, null);
                }
                catch (Exception e)
                {
                    pass = false;
                    caseResult = false;
                    Console.WriteLine("[ ERROR       ] " + e.InnerException.Message);
                }
                Console.WriteLine("[        " + (caseResult ? "PASS" : "FAIL") + " ] " + suite.Name + "." + testCase.Name);
            }
            Console.WriteLine("[   END SUITE ] " + suite.Name);
        }

        if (!pass)
          return -1;

        return 0;
    }
}
