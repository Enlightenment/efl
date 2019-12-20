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

    /// <summary> Asserts if lhs is equal to rhs, using lhs.Equals(rhs).</summary>
    public static void AssertEquals<T>(T lhs, T rhs, String msg = null,
                              [CallerLineNumber] int line = 0,
                              [CallerFilePath] string file = null,
                              [CallerMemberName] string member = null)
    {
        if (lhs == null && rhs == null)
            return;
        if (lhs == null || !lhs.Equals(rhs))
        {
            if (file == null)
                file = "(unknown file)";
            if (member == null)
                member = "(unknown member)";
            if (msg == null || msg.Length == 0)
                msg = $"Left hand side \"{lhs}\", right hand side \"{rhs}\"";
            throw new AssertionException($"{file}:{line} ({member}) {msg}");
        }
    }

    /// <summary> Asserts if lhs is not equal to rhs, using !lhs.Equals(rhs).</summary>
    public static void AssertNotEquals<T>(T lhs, T rhs, String msg = null,
                              [CallerLineNumber] int line = 0,
                              [CallerFilePath] string file = null,
                              [CallerMemberName] string member = null)
    {
        if (lhs == null ? rhs == null : lhs.Equals(rhs))
        {
            if (file == null)
                file = "(unknown file)";
            if (member == null)
                member = "(unknown member)";
            if (msg == null || msg.Length == 0)
                msg = $"Left hand side \"{lhs}\" shouldn't be equal to right hand side \"{rhs}\"";
            throw new AssertionException($"{file}:{line} ({member}) {msg}");
        }
    }

    /// <summary> Asserts if rhs is near enough of lhs, using the optional tolerance (default 0.00001).</summary>
    public static void AssertAlmostEquals(double lhs, double rhs, double tolerance=0.00001,
                              String msg = null,
                              [CallerLineNumber] int line = 0,
                              [CallerFilePath] string file = null,
                              [CallerMemberName] string member = null)
    {
        if (file == null)
            file = "(unknown file)";
        if (member == null)
            member = "(unknown member)";
        double difference = Math.Abs(lhs - rhs);
        if (difference > tolerance) {
            if (msg == null || msg.Length == 0)
                msg = $"Left hand side \"{lhs}\". Difference: \"{difference}\"";
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

    /// <summary>Runs a number of garbage collections and iterate the main loop.
    /// The iteration is needed to make sure objects collected in the GC thread
    /// are efl_unref'd in the main thread.</summary>
    public static void CollectAndIterate(int iterations=1000, int global_iterations=1)
    {
        for (int g = 0; g < global_iterations; ++g)
        {
            for (int i = 0; i < iterations; ++i)
            {
                System.GC.Collect();
            }
            System.GC.WaitForPendingFinalizers();
            Efl.App.AppMain.Iterate();
        }
    }

}


