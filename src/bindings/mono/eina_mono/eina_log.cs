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
#pragma warning disable 1591

using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Diagnostics.Contracts;

namespace Eina
{
// Manual wrappers around eina functions

/// <summary>EFL Logging facilities.
/// <para>Since EFL 1.23.</para>
/// </summary>
public static class Log
{
    [DllImport(efl.Libs.Eina)] private static extern void eina_log_print(
            int domain,
            Level level,
            [MarshalAs(UnmanagedType.LPStr)] String file,
            [MarshalAs(UnmanagedType.LPStr)] String function,
            int line,
            [MarshalAs(UnmanagedType.LPStr)] String message);

    [DllImport(efl.Libs.Eina)] private static extern int eina_log_domain_register(
            [MarshalAs(UnmanagedType.LPStr)] String name,
            [MarshalAs(UnmanagedType.LPStr)] String color);

    [DllImport(efl.Libs.Eina)] private static extern void eina_log_level_set(Level level);

    [DllImport(efl.Libs.Eina)] private static extern Level eina_log_level_get();

    /// <summary>The levels of logging.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public enum Level
    {
        /// <summary>Critical events.
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        Critical,
        /// <summary>Error events.
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        Error,
        /// <summary>Warning events.
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        Warning,
        /// <summary>Informative events.
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        Info,
        /// <summary>Debugging messages.
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        Debug,
        /// <summary>Unknown events.
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        Unkown = (-2147483647 - 1)
    }

    /// <summary>The colors to be used by the logging system.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    internal static class Color
    {
        /// <summary>Light red
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        public const string LIGHTRED  = "\033[31;1m";
        /// <summary>Red
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        public const string RED       = "\033[31m";
        /// <summary>Light blue
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        public const string LIGHTBLUE = "\033[34;1m";
        /// <summary>Blue
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        public const string BLUE      = "\033[34m";
        /// <summary>Green
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        public const string GREEN     = "\033[32;1m";
        /// <summary>Yellow
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        public const string YELLOW    = "\033[33;1m";
        /// <summary>Orange
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        public const string ORANGE    = "\033[0;33m";
        /// <summary>White
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        public const string WHITE     = "\033[37;1m";
        /// <summary>Light cyan
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        public const string LIGHTCYAN = "\033[36;1m";
        /// <summary>Cyan
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        public const string CYAN      = "\033[36m";
        /// <summary>Reset
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        public const string RESET     = "\033[0m";
        /// <summary>Bold
        /// <para>Since EFL 1.23.</para>
        /// </summary>
        public const string HIGH      = "\033[1m";
    }

    private static int domain = -1;

    /// <summary>Static class initializer.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    static Log()
    {
        const String name = "mono";
        const String color = Color.GREEN;

        // Maybe move this check outside when other eina stuff get support?
        domain = eina_log_domain_register(name, color);
        if (domain < 0)
        {
            Console.WriteLine("Error: Couldn't register Eina log domain for name {0}.", name);
        }
        else
        {
            Info($"Registered mono domain with number {domain}");
        }
    }

    private static void EnsureDomainRegistered()
    {
        if (domain < 0)
        {
            throw new InvalidOperationException("Log domain is not registered.");
        }
    }

    /// <summary>Prints a critical message with context info. This context is
    /// filled automatically by the C# compiler.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="message">The message to be printed.</param>
    /// <param name="line">The line number this method was called from.</param>
    /// <param name="file">The file this method was called from.</param>
    /// <param name="member">The enlosing method this method was called from.</param>
    public static void Critical(String message, [CallerLineNumber] int line = 0, [CallerFilePath] string file = null, [CallerMemberName] string member = null)
    {
        EnsureDomainRegistered();
        eina_log_print(domain, Level.Critical, file, member, line, message);
    }

    /// <summary>Prints an error message with context info. This context is
    /// filled automatically by the C# compiler.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="message">The message to be printed.</param>
    /// <param name="line">The line number this method was called from.</param>
    /// <param name="file">The file this method was called from.</param>
    /// <param name="member">The enlosing method this method was called from.</param>
    public static void Error(String message, [CallerLineNumber] int line = 0, [CallerFilePath] string file = null, [CallerMemberName] string member = null)
    {
        EnsureDomainRegistered();
        eina_log_print(domain, Level.Error, file, member, line, message);
    }

    /// <summary>Prints a warning message with context info. This context is
    /// filled automatically by the C# compiler.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="message">The message to be printed.</param>
    /// <param name="line">The line number this method was called from.</param>
    /// <param name="file">The file this method was called from.</param>
    /// <param name="member">The enlosing method this method was called from.</param>
    public static void Warning(String message, [CallerLineNumber] int line = 0, [CallerFilePath] string file = null, [CallerMemberName] string member = null)
    {
        EnsureDomainRegistered();
        eina_log_print(domain, Level.Warning, file, member, line, message);
    }

    /// <summary>Prints an informative message with context info. This context
    /// is filled automatically by the C# compiler.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="message">The message to be printed.</param>
    /// <param name="line">The line number this method was called from.</param>
    /// <param name="file">The file this method was called from.</param>
    /// <param name="member">The enlosing method this method was called from.</param>
    public static void Info(String message, [CallerLineNumber] int line = 0, [CallerFilePath] string file = null, [CallerMemberName] string member = null)
    {
        EnsureDomainRegistered();
        eina_log_print(domain, Level.Info, file, member, line, message);
    }

    /// <summary>Prints a debug message with context info. This context is
    /// filled automatically by the C# compiler.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="message">The message to be printed.</param>
    /// <param name="line">The line number this method was called from.</param>
    /// <param name="file">The file this method was called from.</param>
    /// <param name="member">The enlosing method this method was called from.</param>
    public static void Debug(String message, [CallerLineNumber] int line = 0, [CallerFilePath] string file = null, [CallerMemberName] string member = null)
    {
        EnsureDomainRegistered();
        eina_log_print(domain, Level.Debug, file, member, line, message);
    }

    /// <summary>Sets the highest level log messages should be printed. Values
    /// larger than this one are ignored.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <param name="level">The global message level.</param>
    public static void GlobalLevelSet(Level level)
    {
        eina_log_level_set(level);
    }

    /// <summary>Gets the lowest level of messages that are not ignored.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    /// <returns>The current message level.</returns>
    public static Level GlobalLevelGet()
    {
        return eina_log_level_get();
    }
}

}
