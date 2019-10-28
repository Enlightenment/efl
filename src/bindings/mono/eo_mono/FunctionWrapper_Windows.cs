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
using System.Runtime.InteropServices;

namespace Efl.Eo
{

static partial class FunctionInterop
{
    [DllImport(efl.Libs.Libdl)]
    internal static extern IntPtr GetProcAddress(IntPtr handle, string symbol);

    private static IntPtr LoadFunctionPointer(IntPtr nativeLibraryHandle, string functionName)
        => FunctionInterop.GetProcAddress(nativeLibraryHandle, functionName);
}

}
