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
using System.Collections.Generic;
using System.Linq;

namespace Efl
{
    namespace Eo
    {
        /// <summary>
        /// Helper class with method to convert between managed and native collections
        /// </summary>
        internal class CollectionConversions
        {

            internal static IEnumerable<T> AccessorToIEnumerable<T>(IntPtr accessor, bool isMoved)
            {
                if (accessor == IntPtr.Zero)
                {
                throw new ArgumentException("accessor is null", nameof(accessor));
                }

                IntPtr data = IntPtr.Zero;
                uint position = 0;

                while (Eina.AccessorNativeFunctions.eina_accessor_data_get(accessor, position, out data))
                {
                    yield return Eina.TraitFunctions.NativeToManaged<T>(data);
                    position += 1;
                }

                if (isMoved)
                {
                    Eina.AccessorNativeFunctions.eina_accessor_free(accessor);
                }
            }

            internal static IntPtr IEnumerableToAccessor<T>(IEnumerable<T> enumerable, bool isMoved)
            {
                if (enumerable == null)
                {
                    throw new ArgumentException("enumerable is null", nameof(enumerable));
                }

                // If we are a wrapper around an existing Eina.Accessor, we can just forward
                // it and avoid unnecessary copying.
                var wrappedAccessor = enumerable as Eina.Accessor<T>;

                if (wrappedAccessor != null)
                {
                    if (isMoved)
                    {
                        wrappedAccessor.Own = false;
                    }
                    return wrappedAccessor.Handle;
                }

                // TODO: Check if we're either an Eina.List or Eina.Collection?
                // We could just rewrap their native accessors
                IntPtr[] intPtrs = new IntPtr[enumerable.Count()];

                int i = 0;
                foreach (T data in enumerable)
                {
                    intPtrs[i] = Eina.TraitFunctions.ManagedToNativeAlloc<T>(data);
                    i++;
                }
                IntPtr[] dataArray = intPtrs.ToArray();
                GCHandle pinnedArray = GCHandle.Alloc(dataArray, GCHandleType.Pinned);

                IntPtr nativeAccessor = IntPtr.Zero;

                if (isMoved)
                {
                    // We need a custom accessor that would unpin the data when freed.
                    nativeAccessor = Eina.AccessorNativeFunctions.eina_mono_owned_carray_length_accessor_new(pinnedArray.AddrOfPinnedObject(),
                                                                                                            (uint)IntPtr.Size,
                                                                                                            (uint)dataArray.Length,
                                                                                                            Efl.Eo.Globals.free_gchandle,
                                                                                                            GCHandle.ToIntPtr(pinnedArray));
                }
                else
                {
                    // FIXME: Leaking....
                    nativeAccessor = Eina.AccessorNativeFunctions.eina_carray_length_accessor_new(pinnedArray.AddrOfPinnedObject(), (uint)(IntPtr.Size), (uint)dataArray.Length);
                }

                if (nativeAccessor == IntPtr.Zero)
                {
                    pinnedArray.Free();
                    throw new InvalidOperationException("Failed to get native accessor for the given container");
                }

                return nativeAccessor;
            }

            internal static IEnumerable<T> IteratorToIEnumerable<T>(IntPtr iterator)
            {
                if (iterator == IntPtr.Zero)
                throw new ArgumentException("iterator is null", nameof(iterator));

                while (Eina.IteratorNativeFunctions.eina_iterator_next(iterator, out IntPtr data))
                {
                    yield return Eina.TraitFunctions.NativeToManaged<T>(data);
                }
            }

            internal static IntPtr IEnumerableToIterator<T>(IEnumerable<T> enumerable)
            {
                if (enumerable == null)
                    throw new ArgumentException("enumerable is null", nameof(enumerable));

                IntPtr[] intPtrs = new IntPtr[enumerable.Count()];

                int i = 0;
                foreach (T data in enumerable)
                {
                    intPtrs[i] = Eina.TraitFunctions.ManagedToNativeAlloc<T>(data);
                    i++;
                }

                IntPtr[] dataArray = intPtrs.ToArray();
                GCHandle pinnedArray = GCHandle.Alloc(dataArray, GCHandleType.Pinned); //FIXME: Need to free.
                return Eina.IteratorNativeFunctions.eina_carray_length_iterator_new(pinnedArray.AddrOfPinnedObject(), (uint)(IntPtr.Size), (uint)dataArray.Length);
            }

            internal static IEnumerable<T> ListToIEnumerable<T>(IntPtr list)
            {
                if (list == IntPtr.Zero)
                    throw new ArgumentException("list is null", nameof(list));

                IntPtr l;

                for (l = list; l != IntPtr.Zero; l = Eina.ListNativeFunctions.eina_list_next_custom_export_mono(l))
                {
                    yield return Eina.TraitFunctions.NativeToManaged<T>(Eina.ListNativeFunctions.eina_list_data_get_custom_export_mono(l));
                }
            }

            internal static IntPtr IEnumerableToList<T>(IEnumerable<T> enumerable)
            {
                if (enumerable == null)
                    throw new ArgumentException("enumerable is null", nameof(enumerable));

                IntPtr list = IntPtr.Zero;
                foreach (T data in enumerable)
                {
                    list = Eina.ListNativeFunctions.eina_list_append(list, Eina.TraitFunctions.ManagedToNativeAlloc(data)); //FIXME: need to free
                }
                return list;
            }

        }
    }
}