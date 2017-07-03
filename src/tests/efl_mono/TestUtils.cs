using System;
using System.Runtime.CompilerServices;

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

    public delegate void Operation();

    public static void AssertRaises<T>(Operation op, String msg = "Exception not raised",
                              [CallerLineNumber] int line = 0,
                              [CallerFilePath] string file = null,
                              [CallerMemberName] string member = null) where T: Exception
    {
        try {
            op();
        } catch (T) {
            return;
        }
        throw new Exception($"Assertion failed: {file}:{line} ({member}) {msg}");
    }

    public static void AssertNotRaises<T>(Operation op, String msg = "Exception raised.",
                              [CallerLineNumber] int line = 0,
                              [CallerFilePath] string file = null,
                              [CallerMemberName] string member = null) where T: Exception
    {
        try {
            op();
        } catch (T) {
            throw new Exception($"Assertion failed: {file}:{line} ({member}) {msg}");
        }
    }
}


