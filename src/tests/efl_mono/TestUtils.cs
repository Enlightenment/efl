using System;
using System.Runtime.CompilerServices;
using System.Runtime.Serialization;
using System.Diagnostics.CodeAnalysis;


/// <summary>Exception for assertion failures.</summary>
[Serializable]
public class AssertionException : Exception
{
    /// <summary> Default constructor.</summary>
    public AssertionException() : base () { }
    /// <summary> Most commonly used contructor.</summary>
    public AssertionException(string msg) : base(msg) { }
    /// <summary> Wraps an inner exception.</summary>
    public AssertionException(string msg, Exception inner) : base(msg, inner) { }
    /// <summary> Serializable constructor.</summary>
    protected AssertionException(SerializationInfo info, StreamingContext context) : base(info, context) { }
}

/// <summary> Helper class for Mono EFL bindings tests.</summary>
public static class Test
{
    /// <summary> Asserts a boolean condition.</summary>
    public static void Assert(bool res, String msg = "Assertion failed",
                              [CallerLineNumber] int line = 0,
                              [CallerFilePath] string file = null,
                              [CallerMemberName] string member = null)
    {
        if (msg == null)
            msg = "Assertion failed.";
        if (file == null)
            file = "(unknown file)";
        if (member == null)
            member = "(unknown member)";
        if (!res)
            throw new AssertionException($"Assertion failed: {file}:{line} ({member}) {msg}");
    }

    /// <summary> Asserts if expected is equal to actual, using expected.Equals(actual).</summary>
    public static void AssertEquals<T>(T expected, T actual, String msg = null,
                              [CallerLineNumber] int line = 0,
                              [CallerFilePath] string file = null,
                              [CallerMemberName] string member = null)
    {
        if (file == null)
            file = "(unknown file)";
        if (member == null)
            member = "(unknown member)";
        if (expected == null)
            throw new AssertionException($"{file}:{line} ({member}) Null expected value. Use AssertNull.");
        if (!expected.Equals(actual)) {
            if (msg == null || msg.Length == 0)
                msg = $"Expected \"{expected}\", actual \"{actual}\"";
            throw new AssertionException($"{file}:{line} ({member}) {msg}");
        }
    }

    /// <summary> Asserts if expected is not equal to actual, using !expected.Equals(actual).</summary>
    public static void AssertNotEquals<T>(T expected, T actual, String msg = null,
                              [CallerLineNumber] int line = 0,
                              [CallerFilePath] string file = null,
                              [CallerMemberName] string member = null)
    {
        if (file == null)
            file = "(unknown file)";
        if (member == null)
            member = "(unknown member)";
        if (expected == null)
            throw new AssertionException($"{file}:{line} ({member}) Null expected value. Use AssertNull.");
        if (expected.Equals(actual)) {
            if (msg == null || msg.Length == 0)
                msg = $"Expected \"{expected}\" shouldn't be equal to actual \"{actual}\"";
            throw new AssertionException($"{file}:{line} ({member}) {msg}");
        }
    }

    /// <summary> Asserts if actual is near enough of expected, using the optional tolerance (default 0.00001).</summary>
    public static void AssertAlmostEquals(double expected, double actual, double tolerance=0.00001,
                              String msg = null,
                              [CallerLineNumber] int line = 0,
                              [CallerFilePath] string file = null,
                              [CallerMemberName] string member = null)
    {
        if (file == null)
            file = "(unknown file)";
        if (member == null)
            member = "(unknown member)";
        double difference = Math.Abs(expected - actual);
        if (difference > tolerance) {
            if (msg == null || msg.Length == 0)
                msg = $"Expected \"{expected}\". Difference: \"{difference}\"";
            throw new AssertionException($"{file}:{line} ({member}) {msg}");
        }
    }

    /// <summary> Asserts if greater is greater than smaller , using greater.CompareTo(smaller) > 0.</summary>
    public static void AssertGreaterThan<T>(T greater, T smaller, String msg = null,
                              [CallerLineNumber] int line = 0,
                              [CallerFilePath] string file = null,
                              [CallerMemberName] string member = null) where T : System.IComparable<T>
    {
        if (file == null)
            file = "(unknown file)";
        if (member == null)
            member = "(unknown member)";
        if (greater == null || smaller == null)
            throw new AssertionException($"{file}:{line} ({member}) Null input value. Use AssertNull.");
        if (greater.CompareTo(smaller) <= 0) {
            if (msg == null || msg.Length == 0)
                msg = $"Greater \"{greater}\" is not greater than smaller \"{smaller}\"";
            throw new AssertionException($"{file}:{line} ({member}) {msg}");
        }
    }

    /// <summary> Asserts if smaller is smaller than greater, using greater.CompareTo(smaller) &lt; 0.</summary>
    public static void AssertLessThan<T>(T smaller, T greater, String msg = null,
                              [CallerLineNumber] int line = 0,
                              [CallerFilePath] string file = null,
                              [CallerMemberName] string member = null) where T : System.IComparable<T>
    {
        if (file == null)
            file = "(unknown file)";
        if (member == null)
            member = "(unknown member)";
        if (greater == null || smaller == null)
            throw new AssertionException($"{file}:{line} ({member}) Null input value. Use AssertNull.");
        if (smaller.CompareTo(greater) >= 0) {
            if (msg == null || msg.Length == 0)
                msg = $"Smaller \"{smaller}\" is not smaller than greater \"{greater}\"";
            throw new AssertionException($"{file}:{line} ({member}) {msg}");
        }
    }

    /// <summary> Asserts if op, when called, raises the exception T.</summary>
    [SuppressMessage("Gendarme.Rules.Design.Generic", "AvoidMethodWithUnusedGenericTypeRule")]
    public static void AssertRaises<T>(Action op, String msg = null,
                              [CallerLineNumber] int line = 0,
                              [CallerFilePath] string file = null,
                              [CallerMemberName] string member = null) where T: Exception
    {
        if (msg == null)
            msg = "Exception not raised.";
        if (file == null)
            file = "(unknown file)";
        if (member == null)
            member = "(unknown member)";
        if (op == null)
            throw new AssertionException($"Assertion failed: {file}:{line} ({member}) Null operation.");
        try {
            op();
        } catch (T) {
            return;
        }
        throw new AssertionException($"Assertion failed: {file}:{line} ({member}) {msg}");
    }

    /// <summary> Asserts if op, when called, does not raise the exception T.</summary>
    [SuppressMessage("Gendarme.Rules.Design.Generic", "AvoidMethodWithUnusedGenericTypeRule")]
    public static void AssertNotRaises<T>(Action op, String msg = null,
                              [CallerLineNumber] int line = 0,
                              [CallerFilePath] string file = null,
                              [CallerMemberName] string member = null) where T: Exception
    {
        if (msg == null)
            msg = "Exception raised.";
        if (file == null)
            file = "(unknown file)";
        if (member == null)
            member = "(unknown member)";
        if (op == null)
            throw new AssertionException($"Assertion failed: {file}:{line} ({member}) Null operation.");
        try {
            op();
        } catch (T) {
            throw new AssertionException($"Assertion failed: {file}:{line} ({member}) {msg}");
        }
    }

    /// <summary> Asserts if the given reference is null.</summary>
    public static void AssertNull(object reference, String msg = "Reference not null",
                              [CallerLineNumber] int line = 0,
                              [CallerFilePath] string file = null,
                              [CallerMemberName] string member = null)
    {
        if (reference != null)
            throw new AssertionException($"Assertion failed: {file}:{line} ({member}) {msg}");
    }

    /// <summary> Asserts if the given reference is not null.</summary>
    public static void AssertNotNull(object reference, String msg = "Reference is null",
                              [CallerLineNumber] int line = 0,
                              [CallerFilePath] string file = null,
                              [CallerMemberName] string member = null)
    {
        if (reference == null)
            throw new AssertionException($"Assertion failed: {file}:{line} ({member}) {msg}");
    }
}


