using System;
using System.Runtime.InteropServices;

namespace Efl
{
    namespace Eo
    {
        /// <summary>
        /// Marshaler for <c>@in</c> parameters that have their ownership transfered.
        /// </summary>
        class StringInPassOwnershipMarshaler : ICustomMarshaler
        {
            // Called when C calls a C# method passing data to it
            public object MarshalNativeToManaged(IntPtr pNativeData)
            {
                var ret = Eina.StringConversion.NativeUtf8ToManagedString(pNativeData);
                Eina.MemoryNative.Free(pNativeData);
                return ret;
            }

            // Called when C# calls a C method passing data to it
            public IntPtr MarshalManagedToNative(object managedObj)
            {
                return Eina.MemoryNative.StrDup((string)managedObj);
            }

            // Called when the C call returns, cleaning the result from MarshalManagedToNative
            public void CleanUpNativeData(IntPtr pNativeData)
            {
            }

            // Called when the C# call returns, cleaning the result from MarshalNativeToManaged
            public void CleanUpManagedData(object managedObj)
            {
            }

            public int GetNativeDataSize()
            {
                return -1;
            }

            internal static ICustomMarshaler GetInstance(string cookie)
            {
                if (marshaler == null)
                {
                    marshaler = new StringInPassOwnershipMarshaler();
                }

                return marshaler;
            }

            static private ICustomMarshaler marshaler;
        }

        /// <summary>
        /// Marshaler for <c>@in</c> parameters that do not change their ownership.
        /// </summary>
        class StringInKeepOwnershipMarshaler: ICustomMarshaler
        {
            // Called when C calls a C# method passing data to it
            public object MarshalNativeToManaged(IntPtr pNativeData)
            {
                return Eina.StringConversion.NativeUtf8ToManagedString(pNativeData);
            }

            // Called when C# calls a C method passing data to it
            public IntPtr MarshalManagedToNative(object managedObj)
            {
                return Eina.StringConversion.ManagedStringToNativeUtf8Alloc((string)managedObj);
            }

            // Called when the C call returns, cleaning the result from MarshalManagedToNative
            public void CleanUpNativeData(IntPtr pNativeData)
            {
                Eina.MemoryNative.Free(pNativeData);
            }

            // Called when the C# call returns, cleaning the result from MarshalNativeToManaged
            public void CleanUpManagedData(object managedObj)
            {
                // GC will should take care of it.
            }

            public int GetNativeDataSize()
            {
                return -1;
            }

            internal static ICustomMarshaler GetInstance(string cookie)
            {
                if (marshaler == null)
                {
                    marshaler = new StringInKeepOwnershipMarshaler();
                }

                return marshaler;
            }

            static private ICustomMarshaler marshaler;
        }

        class StringOutPassOwnershipMarshaler : ICustomMarshaler
        {
            // Called when C# calls a C method and receives data from it
            public object MarshalNativeToManaged(IntPtr pNativeData)
            {
                var ret = Eina.StringConversion.NativeUtf8ToManagedString(pNativeData);
                // C gave us ownership. Let's free.
                Eina.MemoryNative.Free(pNativeData);
                return ret;
            }

            // Called when C calls a C# method and receives data from it
            public IntPtr MarshalManagedToNative(object managedObj)
            {
                // As we're passing up the ownership, no need to free it.
                return Eina.MemoryNative.StrDup((string)managedObj);
            }

            // Called when the C call returns, cleaning the result it gave to C#
            public void CleanUpNativeData(IntPtr pNativeData)
            {
            }

            // Called when the C# call returns, cleaning the result it gave to C.
            public void CleanUpManagedData(object managedObj)
            {
            }

            public int GetNativeDataSize()
            {
                return -1;
            }

            internal static ICustomMarshaler GetInstance(string cookie)
            {
                if (marshaler == null)
                {
                    marshaler = new StringOutPassOwnershipMarshaler();
                }

                return marshaler;
            }

            static private ICustomMarshaler marshaler;
        }

        class StringOutKeepOwnershipMarshaler: ICustomMarshaler
        {
            // Called when C# calls a C method and receives data from it
            public object MarshalNativeToManaged(IntPtr pNativeData)
            {
                return Eina.StringConversion.NativeUtf8ToManagedString(pNativeData);
            }

            // Called when C calls a C# method and receives data from it
            public IntPtr MarshalManagedToNative(object managedObj)
            {
                // FIXME This will be the tricky one, as it can leak.
                return Eina.StringConversion.ManagedStringToNativeUtf8Alloc((string)managedObj);
            }

            // Called when the C call returns, cleaning the result it gave to C#
            public void CleanUpNativeData(IntPtr pNativeData)
            {
                // No need to free. The Native side will keep the ownership.
            }

            // Called when the C call returns, cleaning the result it gave to C#
            public void CleanUpManagedData(object managedObj)
            {
            }

            public int GetNativeDataSize()
            {
                return -1;
            }

            internal static ICustomMarshaler GetInstance(string cookie)
            {
                if (marshaler == null)
                {
                    marshaler = new StringOutKeepOwnershipMarshaler();
                }

                return marshaler;
            }

            static private ICustomMarshaler marshaler;
        }

    }

}