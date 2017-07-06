#define CODE_ANALYSIS

using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Security.Permissions;
using System.Security;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.Serialization;

using static eina.EinaNative.UnsafeNativeMethods;
using static eina.TraitFunctions;


namespace eina {

namespace EinaNative {

[SuppressUnmanagedCodeSecurityAttribute]
static internal class UnsafeNativeMethods {

    [DllImport("eina")]
    [return: MarshalAsAttribute(UnmanagedType.U1)]
    internal static extern bool eina_value_convert(IntPtr handle, IntPtr convert);

    // Wrapped and helper methods
    [DllImport("eflcustomexportsmono")]
    [return: MarshalAsAttribute(UnmanagedType.U1)]
    internal static extern int eina_value_sizeof();

    [DllImport("eflcustomexportsmono", CharSet=CharSet.Auto)]
    [return: MarshalAsAttribute(UnmanagedType.U1)]
    internal static extern bool eina_value_set_wrapper(IntPtr handle, string value);

    [DllImport("eflcustomexportsmono")]
    [return: MarshalAsAttribute(UnmanagedType.U1)]
    internal static extern bool eina_value_set_wrapper(IntPtr handle, int value);

    [DllImport("eflcustomexportsmono")]
    [return: MarshalAsAttribute(UnmanagedType.U1)]
    internal static extern bool eina_value_set_wrapper(IntPtr handle, uint value);

    [DllImport("eflcustomexportsmono")]
    [return: MarshalAsAttribute(UnmanagedType.U1)]
    internal static extern bool eina_value_setup_wrapper(IntPtr handle, IntPtr type);

    [DllImport("eflcustomexportsmono")]
    internal static extern void eina_value_flush_wrapper(IntPtr handle);

    [DllImport("eflcustomexportsmono")]
    internal static extern IntPtr eina_value_type_get_wrapper(IntPtr handle);

    [DllImport("eflcustomexportsmono")]
    [return: MarshalAsAttribute(UnmanagedType.U1)]
    internal static extern bool eina_value_get_wrapper(IntPtr handle, out IntPtr output);

    [DllImport("eflcustomexportsmono")]
    internal static extern int eina_value_compare_wrapper(IntPtr handle, IntPtr other);

    [DllImport("eina", CharSet=CharSet.Auto)]
    internal static extern String eina_value_to_string(IntPtr handle); // We take ownership of the returned string.

    [DllImport("eflcustomexportsmono")]
    [return: MarshalAsAttribute(UnmanagedType.U1)]
    internal static extern bool eina_value_array_setup_wrapper(IntPtr handle, IntPtr subtype, uint step);

    [DllImport("eflcustomexportsmono")]
    [return: MarshalAsAttribute(UnmanagedType.U1)]
    internal static extern bool eina_value_array_append_wrapper(IntPtr handle, int data);

    [DllImport("eflcustomexportsmono")]
    [return: MarshalAsAttribute(UnmanagedType.U1)]
    internal static extern bool eina_value_array_get_wrapper(IntPtr handle, int index, out IntPtr output);

    [DllImport("eflcustomexportsmono")]
    [return: MarshalAsAttribute(UnmanagedType.U1)]
    internal static extern bool eina_value_array_set_wrapper(IntPtr handle, int index, IntPtr value);

    [DllImport("eflcustomexportsmono")]
    internal static extern IntPtr eina_value_array_subtype_get_wrapper(IntPtr handle);

    [DllImport("eflcustomexportsmono")]
    internal static extern uint eina_value_array_count_wrapper(IntPtr handle);

    // Supported types
    [DllImport("eflcustomexportsmono")]
    internal static extern IntPtr type_int32();
    [DllImport("eflcustomexportsmono")]
    internal static extern IntPtr type_uint32();
    [DllImport("eflcustomexportsmono")]
    internal static extern IntPtr type_string();
    [DllImport("eflcustomexportsmono")]
    internal static extern IntPtr type_array();
}
}

/// <summary>Exception for trying to access flushed values.</summary>
[Serializable]
public class ValueFlushedException : Exception
{
    /// <summary> Default constructor.</summary>
    public ValueFlushedException() : base () { }
    /// <summary> Most commonly used contructor.</summary>
    public ValueFlushedException(string msg) : base(msg) { }
    /// <summary> Wraps an inner exception.</summary>
    public ValueFlushedException(string msg, Exception inner) : base(msg, inner) { }
    /// <summary> Serializable constructor.</summary>
    protected ValueFlushedException(SerializationInfo info, StreamingContext context) : base(info, context) { }
}

/// <summary>Exception for failures when setting an container item.</summary>
[Serializable]
public class SetItemFailedException : Exception
{
    /// <summary> Default constructor.</summary>
    public SetItemFailedException() : base () { }
    /// <summary> Most commonly used contructor.</summary>
    public SetItemFailedException(string msg) : base(msg) { }
    /// <summary> Wraps an inner exception.</summary>
    public SetItemFailedException(string msg, Exception inner) : base(msg, inner) { }
    /// <summary> Serializable constructor.</summary>
    protected SetItemFailedException(SerializationInfo info, StreamingContext context) : base(info, context) { }
}

/// <summary>Managed-side Enum to represent Eina_Value_Type constants</summary>
public enum ValueType {
    /// <summary>Signed 32 bit integer. Same as 'int'</summary>
    Int32,
    /// <summary>Unsigned 32 bit integer. Same as 'uint'</summary>
    UInt32,
    /// <summary>Strings</summary>
    String,
    /// <summary>Array of Value items.</summary>
    Array,
    /// <summary>Linked list of Value items.</summary>
    List,
    /// <summary>Map of string keys to Value items.</summary>
    Hash,
}

static class ValueTypeMethods {
    public static bool IsNumeric(this ValueType val)
    {
        switch (val) {
            case ValueType.Int32:
            case ValueType.UInt32:
                return true;
            default:
                return false;
        }
    }

    public static bool IsString(this ValueType val)
    {
        switch(val) {
            case ValueType.String:
                return true;
            default:
                return false;
        }
    }

    public static bool IsContainer(this ValueType val)
    {
        switch(val) {
            case ValueType.Array:
            case ValueType.List:
            case ValueType.Hash:
                return true;
            default:
                return false;
        }
    }
}
static class ValueTypeBridge
{
    private static Dictionary<ValueType, IntPtr> ManagedToNative = new Dictionary<ValueType, IntPtr>();
    private static Dictionary<IntPtr, ValueType> NativeToManaged = new Dictionary<IntPtr, ValueType>();
    private static bool TypesLoaded; // CLR defaults to false;

    public static ValueType GetManaged(IntPtr native)
    {
        if (!TypesLoaded)
            LoadTypes();

        return NativeToManaged[native];
    }

    public static IntPtr GetNative(ValueType valueType)
    {
        if (!TypesLoaded)
            LoadTypes();

        return ManagedToNative[valueType];
    }

    private static void LoadTypes()
    {
        ManagedToNative.Add(ValueType.Int32, type_int32());
        NativeToManaged.Add(type_int32(), ValueType.Int32);

        ManagedToNative.Add(ValueType.UInt32, type_uint32());
        NativeToManaged.Add(type_uint32(), ValueType.UInt32);

        ManagedToNative.Add(ValueType.String, type_string());
        NativeToManaged.Add(type_string(), ValueType.String);

        ManagedToNative.Add(ValueType.Array, type_array());
        NativeToManaged.Add(type_array(), ValueType.Array);

        TypesLoaded = true;
    }
}

/// <summary>Wrapper around Eina_Value generic storage.
///
/// <para>Eina_Value is EFL's main workhorse to deal with storing generic data in
/// an uniform way.</para>
///
/// <para>It comes with predefined types for numbers, strings, array, list, hash,
/// blob and structs. It is able to convert between data types, including
/// to string.</para>
/// </summary>
public class Value : IDisposable, IComparable<Value>, IEquatable<Value>
{

    // Unmanaged type - Managed type mapping
    // EINA_VALUE_TYPE_UCHAR: unsigned char -- byte
    // EINA_VALUE_TYPE_USHORT: unsigned short -- ushort
    // Ok EINA_VALUE_TYPE_UINT: unsigned int -- uint
    // EINA_VALUE_TYPE_ULONG: unsigned long -- ulong
    // EINA_VALUE_TYPE_UINT64: uint64_t -- ulong
    // EINA_VALUE_TYPE_CHAR: char -- sbyte
    // EINA_VALUE_TYPE_SHORT: short -- short
    // Ok EINA_VALUE_TYPE_INT: int -- int
    // EINA_VALUE_TYPE_LONG: long -- long
    // EINA_VALUE_TYPE_INT64: int64_t -- long
    // EINA_VALUE_TYPE_FLOAT: float -- float
    // EINA_VALUE_TYPE_DOUBLE: double -- double
    // EINA_VALUE_TYPE_STRINGSHARE: const char * -- string
    // Ok EINA_VALUE_TYPE_STRING: const char * -- string
    // EINA_VALUE_TYPE_ARRAY: Eina_Value_Array -- eina.Array?
    // EINA_VALUE_TYPE_LIST: Eina_Value_List -- eina.List?
    // EINA_VALUE_TYPE_HASH: Eina_Value_Hash -- eina.Hash?
    // EINA_VALUE_TYPE_TIMEVAL: struct timeval -- FIXME
    // EINA_VALUE_TYPE_BLOB: Eina_Value_Blob -- FIXME
    // EINA_VALUE_TYPE_STRUCT: Eina_Value_Struct -- FIXME


    private IntPtr Handle;
    private bool Disposed;
    private bool Flushed;

    /// <summary>Create a wrapper around the given value storage.</summary>
    /* public Value(IntPtr Ptr) */
    /* { */
    /*     if (!TypesLoaded) */
    /*         LoadTypes(); */

    /*     this.Handle = Ptr; */
    /* } */

    /// <summary>Creates a new value storage for values of type 'type'.</summary>
    public Value(ValueType type)
    {
        if (type.IsContainer())
            throw new ArgumentException("To use container types you must provide a subtype");
        this.Handle = Marshal.AllocHGlobal(eina_value_sizeof());
        Setup(type);
    }

    /// <summary>Constructor for container values, like Array, Hash.</summary>
    public Value(ValueType containerType, ValueType subtype, uint step=0)
    {
        if (!containerType.IsContainer())
            throw new ArgumentException("First type must be a container type.");

        this.Handle = Marshal.AllocHGlobal(eina_value_sizeof());

        Setup(containerType, subtype, step);
    }

    /// <summary>Public method to explicitly free the wrapped eina value.</summary>
    public void Dispose()
    {
        Dispose(true);
        GC.SuppressFinalize(this);
    }

    /// <summary>Actually free the wrapped eina value. Can be called from Dispose() or through the GC.</summary>
    protected virtual void Dispose(bool disposing)
    {
        if (!Disposed && (Handle != IntPtr.Zero)) {
            if (!Flushed)
                eina_value_flush_wrapper(this.Handle);

            Marshal.FreeHGlobal(this.Handle);
        }
        Disposed = true;
    }

    /// <summary>Finalizer to be called from the Garbage Collector.</summary>
    ~Value()
    {
        Dispose(false);
    }

    /// <summary>Returns the native handle wrapped by this object.</summary>
    public IntPtr NativeHandle()
    {
        if (Disposed)
            throw new ObjectDisposedException(base.GetType().Name);
        return this.Handle;
    }

    /// <summary>Converts this storage to type 'type'</summary>
    public bool Setup(ValueType type)
    {
        if (Disposed)
            throw new ObjectDisposedException(base.GetType().Name);

        if (type.IsContainer())
            throw new ArgumentException("To setup a container you must provide a subtype.");

        bool ret = eina_value_setup_wrapper(this.Handle, ValueTypeBridge.GetNative(type));
        if (ret)
            Flushed = false;
        return ret;
    }

    public bool Setup(ValueType containerType, ValueType subtype, uint step=0) {
        IntPtr native_subtype = ValueTypeBridge.GetNative(subtype);
        bool ret = false;
        switch (containerType) {
            case ValueType.Array:
                ret = eina_value_array_setup_wrapper(this.Handle, native_subtype, step);
                break;
        }

        if (ret)
            Flushed = false;

        return ret;
    }

    private void SanityChecks()
    {
        if (Disposed)
            throw new ObjectDisposedException(GetType().Name);
        if (Flushed)
            throw new ValueFlushedException("Trying to use value that has been flushed. Setup it again.");
    }

    /// <summary>Releases the memory stored by this value. It can be reused by calling setup again.
    /// </summary>
    public void Flush()
    {
        if (Disposed)
            throw new ObjectDisposedException(GetType().Name);
        eina_value_flush_wrapper(this.Handle);
        Flushed = true;
    }

    /// <summary>Stores the given uint value.</summary>
    public bool Set(uint value)
    {
        SanityChecks();
        if (!GetValueType().IsNumeric())
            throw (new ArgumentException(
                        "Trying to set numeric value on a non-numeric eina.Value"));
        return eina_value_set_wrapper(this.Handle, value);
    }

    /// <summary>Stores the given int value.</summary>
    public bool Set(int value)
    {
        SanityChecks();
        if (!GetValueType().IsNumeric())
            throw (new ArgumentException(
                        "Trying to set numeric value on a non-numeric eina.Value"));
        return eina_value_set_wrapper(this.Handle, value);
    }

    /// <summary>Stores the given string value.</summary>
    public bool Set(string value)
    {
        SanityChecks();
        if (!GetValueType().IsString())
            throw (new ArgumentException(
                        "Trying to set non-string value on a string eina.Value"));
        // No need to worry about ownership as eina_value_set will copy the passed string.
        return eina_value_set_wrapper(this.Handle, value);
    }

    /// <summary>Gets the currently stored value as an int.</summary>
    public bool Get(out int value)
    {
        SanityChecks();
        IntPtr output = IntPtr.Zero;
        if (!eina_value_get_wrapper(this.Handle, out output)) {
            value = 0;
            return false;
        }
        value = Convert.ToInt32(output.ToInt64());
        return true;
    }

    /// <summary>Gets the currently stored value as an uint.</summary>
    public bool Get(out uint value)
    {
        SanityChecks();
        IntPtr output = IntPtr.Zero;
        if (!eina_value_get_wrapper(this.Handle, out output)) {
            value = 0;
            return false;
        }
        value = Convert.ToUInt32(output.ToInt64());
        return true;
    }

    /// <summary>Gets the currently stored value as a string.</summary>
    public bool Get(out string value)
    {
        SanityChecks();
        IntPtr output = IntPtr.Zero;
        if (!eina_value_get_wrapper(this.Handle, out output)) {
            value = String.Empty;
            return false;
        }
        value = Marshal.PtrToStringAuto(output);
        return true;
    }

    /// <summary>Gets the 'Type' this value is currently configured to store.</summary>
    public ValueType GetValueType()
    {
        if (Disposed)
            throw new ObjectDisposedException(base.GetType().Name);
        IntPtr native_type = eina_value_type_get_wrapper(this.Handle);
        return ValueTypeBridge.GetManaged(native_type);
    }

    /// <summary>Converts the value on this storage to the type of 'target' and stores
    /// the result in 'target'.</summary>
    public bool ConvertTo(Value target)
    {
        if (target == null)
            return false;

        SanityChecks();

        return eina_value_convert(this.Handle, target.Handle);
    }

    /// <summary>Compare two eina values.</summary>
    public int CompareTo(Value other)
    {
        if (other == null)
            return 1;
        SanityChecks();
        other.SanityChecks();
        return eina_value_compare_wrapper(this.Handle, other.Handle);
    }

    public int Compare(Value other)
    {
        return this.CompareTo(other);
    }

    public override bool Equals(object obj)
    {
        if (obj == null)
            return false;

        Value v = obj as Value;
        if (v == null)
            return false;

        return this.Equals(v);
    }

    public bool Equals(Value other)
    {
        try {
            return this.CompareTo(other) == 0;
        } catch (ObjectDisposedException) {
            return false;
        } catch (ValueFlushedException) {
            return false;
        }
    }

    public override int GetHashCode()
    {
        return this.Handle.ToInt32();
    }

    public static bool operator==(Value x, Value y)
    {
        if (object.ReferenceEquals(x, null))
            return object.ReferenceEquals(y, null);

        return x.Equals(y);
    }

    public static bool operator!=(Value x, Value y)
    {
        if (object.ReferenceEquals(x, null))
            return !object.ReferenceEquals(y, null);
        return !x.Equals(y);
    }

    public static bool operator>(Value x, Value y)
    {
        if (object.ReferenceEquals(x, null) || object.ReferenceEquals(y, null))
            return false;
        return x.CompareTo(y) > 0;
    }

    public static bool operator<(Value x, Value y)
    {
        if (object.ReferenceEquals(x, null) || object.ReferenceEquals(y, null))
            return false;
        return x.CompareTo(y) < 0;
    }


    /// <summary>Converts value to string.</summary>
    public override String ToString()
    {
        SanityChecks();
        return eina_value_to_string(this.Handle);
    }

    // Array methods
    public bool Append(object o) {
        SanityChecks();
        return eina_value_array_append_wrapper(this.Handle, (int)o);
    }

    public object this[int i]
    {
        get {
            SanityChecks();

            IntPtr output = IntPtr.Zero;
            // FIXME Support other integer-addressable containers (list)
            if (!eina_value_array_get_wrapper(this.Handle, i, out output))
                return null;
            return UnMarshalPtr(output);
        }
        set {
            SanityChecks();
            IntPtr marshalled_value = MarshalValue(value);

            if (!eina_value_array_set_wrapper(this.Handle, i, marshalled_value)) {

                if (GetValueSubType().IsString())
                    Marshal.FreeHGlobal(marshalled_value);

                uint size = eina_value_array_count_wrapper(this.Handle);

                if (i >= size)
                    throw new System.ArgumentOutOfRangeException($"Index {i} is larger than max array index {size-1}");

                throw new SetItemFailedException($"Failed to set item at index {i}");
            }

            if (GetValueSubType().IsString())
                Marshal.FreeHGlobal(marshalled_value);
        }
    }

    public ValueType GetValueSubType()
    {
        SanityChecks();

        IntPtr native_subtype = eina_value_array_subtype_get_wrapper(this.Handle);
        return ValueTypeBridge.GetManaged(native_subtype);
    }

    private IntPtr MarshalValue(object value)
    {
        switch(GetValueSubType()) {
            case ValueType.Int32:
                {
                    int x = (int)value;
                    return new IntPtr(x);
                }
            case ValueType.UInt32:
                {
                    uint x = (uint)value;
                    return new IntPtr((int)x);
                }
            case ValueType.String:
                {
                    string x = value as string;
                    if (x == null)
                        return IntPtr.Zero;
                    // Warning: Caller will have ownership of this memory.
                    return Marshal.StringToHGlobalAnsi(x);
                }
            default:
                return IntPtr.Zero;
        }
    }

    private object UnMarshalPtr(IntPtr data)
    {
        switch(GetValueSubType()) {
            case ValueType.Int32:
                return Convert.ToInt32(data.ToInt64());
            case ValueType.UInt32:
                return Convert.ToUInt32(data.ToInt64());
            case ValueType.String:
                return Marshal.PtrToStringAuto(data);
            default:
                return null;
        }
    }

}
}
