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

    public static void AssertEquals<T>(T expected, T actual, String msg = "",
                              [CallerLineNumber] int line = 0,
                              [CallerFilePath] string file = null,
                              [CallerMemberName] string member = null) where T : System.IComparable<T>
    {
        if (expected.CompareTo(actual) != 0) {
            if (msg == "")
                msg = $"Expected \"{expected}\", actual \"{actual}\"";
            throw new Exception($"{file}:{line} ({member}) {msg}");
        }
    }
}

class TestMain
{
    [DllImport("eo")] static extern void efl_object_init();
    [DllImport("ecore")] static extern void ecore_init();
    [DllImport("evas")] static extern void evas_init();

    static Type[] GetTestCases(String name="")
    {
        return Assembly.GetExecutingAssembly().GetTypes().Where(t => String.Equals(t.Namespace, "TestSuite", StringComparison.Ordinal) &&
                                                                t.Name.StartsWith("Test") &&
                                                                t.Name.Contains(name)).ToArray();
    }

    static int Main(string[] args)
    {
        eina.Log.Init();
        efl_object_init();
        ecore_init();
        evas_init();

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
            foreach(var localTestCase in localTestCases)
            {

                Console.WriteLine("[ RUN         ] " + testCase.Name + "." + localTestCase.Name);
                bool caseResult = true;
                try
                {
                    localTestCase.Invoke(null, null);
                }
                catch (Exception e)
                {
                    pass = false;
                    caseResult = false;
                    Console.WriteLine("[ ERROR       ] " + e.InnerException.Message);
                }
                Console.WriteLine("[        " + (caseResult ? "PASS" : "FAIL") + " ] " + testCase.Name + "." + localTestCase.Name);
            }
        }
        Console.WriteLine("[   END SUITE ] " + ckRunSuite);

        if (!pass)
          return -1;

        return 0;
    }
}
