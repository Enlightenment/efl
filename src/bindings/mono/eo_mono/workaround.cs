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
using System.Collections.Generic;

///<summary>Eo class description, passed to efl_class_new.</summary>
[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi)]
internal struct ClassDescription : IEquatable<ClassDescription>
{
    ///<summary>Current Eo version.</summary>
    internal uint version;
    ///<summary>Name of the class.</summary>
    [MarshalAs(UnmanagedType.LPStr)] internal String name;
    ///<summary>Class type.</summary>
    internal int class_type;
    ///<summary>Size of data (private + protected + public) per instance.</summary>
    internal UIntPtr data_size;
    ///<summary>Initializer for the class.</summary>
    internal IntPtr class_initializer;
    ///<summary>Constructor of the class.</summary>
    internal IntPtr class_constructor;
    ///<summary>Destructor of the class.</summary>
    internal IntPtr class_destructor;

    /// <summary>
    ///   Gets a hash for <see cref="ClassDescription" />.
    /// <para>Since EFL 1.24.</para>
    /// </summary>
    /// <returns>A hash code.</returns>
    public override int GetHashCode()
        => version.GetHashCode() ^ name.GetHashCode(StringComparison.Ordinal)
        ^ class_type ^ data_size.GetHashCode();

    /// <summary>Returns whether this <see cref="ClassDescription" />
    /// is equal to the given <see cref="object" />.
    /// <para>Since EFL 1.24.</para>
    /// </summary>
    /// <param name="other">The <see cref="object" /> to be compared to.</param>
    /// <returns><c>true</c> if is equal to <c>other</c>.</returns>
    public override bool Equals(object other)
        => (!(other is ClassDescription)) ? false
        : Equals((ClassDescription)other);


    /// <summary>Returns whether this <see cref="ClassDescription" /> is equal
    /// to the given <see cref="ClassDescription" />.
    /// <para>Since EFL 1.24.</para>
    /// </summary>
    /// <param name="other">The <see cref="ClassDescription" /> to be compared to.</param>
    /// <returns><c>true</c> if is equal to <c>other</c>.</returns>
    public bool Equals(ClassDescription other)
        => (name == other.name) && (class_type == other.class_type);

    /// <summary>Returns whether <c>lhs</c> is equal to <c>rhs</c>.
    /// <para>Since EFL 1.24.</para>
    /// </summary>
    /// <param name="lhs">The left hand side of the operator.</param>
    /// <param name="rhs">The right hand side of the operator.</param>
    /// <returns><c>true</c> if <c>lhs</c> is equal
    /// to <c>rhs</c>.</returns>
    public static bool operator==(ClassDescription lhs, ClassDescription rhs)
        => lhs.Equals(rhs);

    /// <summary>Returns whether <c>lhs</c> is not equal to <c>rhs</c>.
    /// <para>Since EFL 1.24.</para>
    /// </summary>
    /// <param name="lhs">The left hand side of the operator.</param>
    /// <param name="rhs">The right hand side of the operator.</param>
    /// <returns><c>true</c> if <c>lhs</c> is not equal
    /// to <c>rhs</c>.</returns>
    public static bool operator!=(ClassDescription lhs, ClassDescription rhs)
        => !(lhs == rhs);
}

///<summary>Description of an Eo API operation.</summary>
[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi)]
internal struct EflOpDescription
{
    ///<summary>The EAPI function offering this op. (String with the name of the function on Windows)</summary>
    internal IntPtr api_func;
    ///<summary>The static function to be called for this op</summary>
    internal IntPtr func;
}

///<summary>List of operations on a given Object.</summary>
[StructLayout(LayoutKind.Sequential, CharSet=CharSet.Ansi)]
internal struct EflObjectOps
{
    ///<summary>The op descriptions array of size count.</summary>
    internal IntPtr descs;
    ///<summary>Number of op descriptions.</summary>
    internal UIntPtr count;
};

namespace Efl
{

/// <summary>
/// This struct holds the description of a specific event.
/// <para>Since EFL 1.22.</para>
/// </summary>
[StructLayout(LayoutKind.Sequential)]
internal struct EventDescription
{
    ///<summary>Name of the event.</summary>
    public IntPtr Name;
    ///<summary><c>true</c> if the event cannot be frozen.</summary>
    [MarshalAs(UnmanagedType.U1)] public bool Unfreezable;
    ///<summary>Internal use: <c>true</c> if this is a legacy event.</summary>
    [MarshalAs(UnmanagedType.U1)] public bool Legacy_is;
    ///<summary><c>true</c> if when the even is triggered again from a callback it
    ///will start from where it was.</summary>
    [MarshalAs(UnmanagedType.U1)] public bool Restart;

    private static Dictionary<string, IntPtr> descriptions = new Dictionary<string, IntPtr>();

    ///<summary>Constructor for EventDescription</summary>
    ///<param name="moduleName">The name of the module containing the event.</param>
    ///<param name="name">The name of the event.</param>
    public EventDescription(string moduleName, string name)
    {
        this.Name = GetNative(moduleName, name);
        this.Unfreezable = false;
        this.Legacy_is = false;
        this.Restart = false;
    }

    ///<summary>Get the native structure.</summary>
    ///<param name="moduleName">The name of the module containing the event.</param>
    ///<param name="name">The name of the event.</param>
    ///<returns>Pointer to the native structure.</returns>
    public static IntPtr GetNative(string moduleName, string name)
    {
        if (!descriptions.ContainsKey(name))
        {
            IntPtr data = Efl.Eo.FunctionInterop.LoadFunctionPointer(moduleName, name);

            if (data == IntPtr.Zero)
            {
                string error = Eina.StringConversion.NativeUtf8ToManagedString(Efl.Eo.Globals.dlerror());
                throw new Exception(error);
            }

            descriptions.Add(name, data);
        }

        return descriptions[name];
    }
};

/// <summary>
/// A parameter passed in event callbacks holding extra event parameters.
/// This is the full event information passed to callbacks in C.
/// <para>Since EFL 1.22.</para>
/// </summary>
[StructLayout(LayoutKind.Sequential)]
[Efl.Eo.BindingEntity]
internal struct Event
{
    /// <summary>Internal wrapper for field Object</summary>
    private System.IntPtr obj;
    /// <summary>Internal wrapper for field Desc</summary>
    private System.IntPtr desc;
    /// <summary>Internal wrapper for field Info</summary>
    private System.IntPtr info;

    /// <summary>
    /// The object the callback was called on.
    /// <para>Since EFL 1.22.</para>
    /// </summary>
    public Efl.Object Object { get => (Efl.Object) Efl.Eo.Globals.CreateWrapperFor(obj); }

    /// <summary>
    /// The event description.
    /// <para>Since EFL 1.22.</para>
    /// </summary>
    public Efl.EventDescription Desc { get => Eina.PrimitiveConversion.PointerToManaged<Efl.EventDescription>(desc); }

    /// <summary>
    /// Extra event information passed by the event caller.
    /// Must be cast to the event type declared in the EO file. Keep in mind that:
    /// 1) Objects are passed as a normal Eo*. Event subscribers can call functions on these objects.
    /// 2) Structs, built-in types and containers are passed as const pointers, with one level of indirection.
    /// <para>Since EFL 1.22.</para>
    /// </summary>
    public System.IntPtr Info { get => info; }

    /// <summary>Constructor for Event.</summary>
    public Event(
        Efl.Object obj = default(Efl.Object),
        Efl.EventDescription desc = default(Efl.EventDescription),
        System.IntPtr info = default(System.IntPtr))
    {
        this.obj = obj?.NativeHandle ?? System.IntPtr.Zero;
        this.desc = Eina.PrimitiveConversion.ManagedToPointerAlloc(desc);
        this.info = info;
    }

    /// <summary>Implicit conversion to the managed representation from a native pointer.</summary>
    /// <param name="ptr">Native pointer to be converted.</param>
    public static implicit operator Event(IntPtr ptr)
    {
        var tmp = (Event) Marshal.PtrToStructure(ptr, typeof(Event));
        return tmp;
    }
}

internal delegate void EventCb(System.IntPtr data, ref Event evt);
internal delegate void FreeWrapperSupervisorCb(System.IntPtr obj);

} // namespace Efl
