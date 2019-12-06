using System;
using System.Runtime.InteropServices;

namespace Efl
{
    namespace Eo
    {
        class StringPassOwnershipMarshaler : ICustomMarshaler
        {
            public object MarshalNativeToManaged(IntPtr pNativeData)
            {
                var ret = Eina.StringConversion.NativeUtf8ToManagedString(pNativeData);
                Eina.MemoryNative.Free(pNativeData);
                return ret;
            }

            public IntPtr MarshalManagedToNative(object managedObj)
            {
                return Eina.MemoryNative.StrDup((string)managedObj);
            }

            public void CleanUpNativeData(IntPtr pNativeData)
            {
                // No need to cleanup. C will take care of it.
            }

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
                    marshaler = new StringPassOwnershipMarshaler();
                }

                return marshaler;
            }

            static private StringPassOwnershipMarshaler marshaler;
        }

        class StringKeepOwnershipMarshaler: ICustomMarshaler
        {
            public object MarshalNativeToManaged(IntPtr pNativeData)
            {
                return Eina.StringConversion.NativeUtf8ToManagedString(pNativeData);
            }

            public IntPtr MarshalManagedToNative(object managedObj)
            {
                return Eina.StringConversion.ManagedStringToNativeUtf8Alloc((string)managedObj);
            }

            public void CleanUpNativeData(IntPtr pNativeData)
            {
                // No need to free. The Native side will keep the ownership.
            }

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
                    marshaler = new StringKeepOwnershipMarshaler();
                }

                return marshaler;
            }

            static private StringKeepOwnershipMarshaler marshaler;
        }

    }

}