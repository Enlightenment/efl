#pragma warning disable 1591

using System;
using System.Runtime.InteropServices;


namespace Eina
{

// TODO: move all native functions to a "NativeMethods" class
public static partial class NativeMethods
{
    [DllImport(efl.Libs.Eina)] public static extern IntPtr
        eina_stringshare_add(IntPtr str);
    [DllImport(efl.Libs.Eina)] public static extern System.IntPtr
        eina_stringshare_add_length(IntPtr str, UInt32 slen);
    [DllImport(efl.Libs.Eina)] public static extern void
        eina_stringshare_del(IntPtr str);
    [DllImport(efl.Libs.CustomExports)] public static extern void
        efl_mono_native_stringshare_del_ref(IntPtr str);
    [DllImport(efl.Libs.CustomExports)] public static extern IntPtr
        efl_mono_native_stringshare_del_addr_get();
}

/// <summary>
/// Placeholder type to interact with the native type Eina_Stringshare, mainly for eina containers.
/// </summary>
/// <remarks>
/// Using System.String and merely converting this type to it (by cast or implicitly)
/// is recommended for simplicity and ease of use.
///
/// This type is just a System.String wrapper that works as a placeholder for
/// instrumentalizing proper interaction with native EFL API that demands
/// strings to be stringshares.
///
/// Both C strings and eina stringshares are bound as regular strings in EFL#,
/// as working directly with these types would demand unnecessary hassle from
/// the user viewpoint.
/// But for eina containers this distinction is important, and since C# generics
/// do not provide a convenient way of dealing with the same type requiring
/// a different management based on some other condition (at least not without
/// compromising the usability for other types), we use this string wrapper
/// in order to signal this distinction.
///
/// Implements equality/inequality methods for easier comparison with strings and
/// other Stringshare objects. For more specific operations convert to a string.
/// </remarks>
public class Stringshare : IEquatable<Stringshare>, IEquatable<string>
{
    /// <sumary>
    /// Main constructor. Wrap the given string.
    /// Have private acess to avoid wrapping a null reference,
    /// use convertion or the factory method to create a new instance.
    /// <see cref="Stringshare.Create(string s)"/>
    /// <see cref="Stringshare.operator Stringshare(string s)"/>
    /// </sumary>
    private Stringshare(string s)
    {
        Str = s;
    }

    /// <sumary>
    /// Auto-implemented property that holds the wrapped string value.
    /// </sumary>
    public string Str { get; private set; }

    /// <sumary>
    /// Factory method to instantiate a new object.
    /// Get a wrappper for the given string or a null reference if the given
    /// string reference is also null.
    /// <seealso cref="Stringshare.operator Stringshare(string s)"/>
    /// </sumary>
    /// <returns>
    /// A new instance wrapping the given string, or a null reference if
    /// the given string reference is also null.
    /// </returns>
    public static Stringshare Create(string s)
    {
        if (s == null)
        {
            return null;
        }

        return new Stringshare(s);
    }

    /// <sumary>
    /// Implicit convertion to string.
    /// </sumary>
    public static implicit operator string(Stringshare ss)
    {
        if (ReferenceEquals(null, ss))
        {
            return null;
        }

        return ss.Str;
    }

    /// <sumary>
    /// Implicit convertion from string to Stringshare.
    /// </sumary>
    /// <remarks>
    /// Note that this method can be used to create an instance of this class,
    /// either via an explicit cast or an implicit convertion.
    /// <seealso cref="Stringshare.Create(string s)"/>
    /// </remarks>
    public static implicit operator Stringshare(string s)
    {
        if (ReferenceEquals(null, s))
        {
            return null;
        }

        return new Stringshare(s);
    }

    /// <sumary>
    /// Check two Stringshare objects for equality.
    /// </sumary>
    /// <returns>
    /// True if both wrapped strings have the same content or if both
    /// references are null, false otherwise.
    /// </returns>
    public static bool operator==(Stringshare ss1, Stringshare ss2)
    {
        return ((string)ss1) == ((string)ss2);
    }


    /// <sumary>
    /// Check two Stringshare objects for inequality.
    /// </sumary>
    /// <returns>
    /// True if the wrapped strings have different content or if one reference is null
    /// and the other is not, false otherwise.
    /// </returns>
    public static bool operator!=(Stringshare ss1, Stringshare ss2)
    {
        return !(ss1 == ss2);
    }

    /// <sumary>
    /// Returns the wrapped string.
    /// <seealso cref="Stringshare.Str"/>
    /// <seealso cref="Stringshare.Get()"/>
    /// </sumary>
    /// <returns>The wrapped string. Same as the property Str.</returns>
    public override string ToString()
    {
        return Str;
    }

    /// <sumary>
    /// Override of GetHashCode for consistency with user-defined equality methods.
    /// </sumary>
    /// <returns>
    /// The wrapped string hash code.
    /// </returns>
    public override int GetHashCode()
    {
        return Str.GetHashCode();
    }

    /// <sumary>
    /// Check the given object for equality.
    /// </sumary>
    /// <returns>
    /// True if the given object is the same object or if it is another Stringshare object
    /// and both wrapped strings are equal or if it is a string object and its content
    /// is the same of the wrapped string.
    /// In any other case it returns false.
    /// </returns>
    public override bool Equals(object other)
    {
        if (ReferenceEquals(null, other))
        {
            return false;
        }

        if (ReferenceEquals(this, other))
        {
            return true;
        }

        if (other.GetType() == typeof(string))
        {
            return this.Str == (string)other;
        }

        return other.GetType() == typeof(Stringshare) && this == ((Stringshare)other);
    }

    /// <sumary>
    /// Check the given Stringshare for equality.
    /// </sumary>
    /// <returns>
    /// True if the given Stringshare object is not null and its wrapped string
    /// have the same content of this.Str, false otherwise.
    /// </returns>
    public bool Equals(Stringshare other)
    {
        return this == other;
    }

    /// <sumary>
    /// Check the given Stringshare for equality.
    /// </sumary>
    /// <returns>
    /// True if the given string is not null and the wrapped string have the same
    /// content of the given string, false otherwise.
    /// </returns>
    public bool Equals(string other)
    {
        return this.Str == other;
    }

    /// <sumary>
    /// Get the wrapped string.
    /// <seealso cref="Stringshare.Str"/>
    /// <seealso cref="Stringshare.ToString()"/>
    /// </sumary>
    /// <returns>The wrapped string. Same as the property Str.</returns>
    public string Get()
    {
        return Str;
    }
}

}
