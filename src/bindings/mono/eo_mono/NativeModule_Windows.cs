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
#if WIN32

using System;
using System.Runtime.InteropServices;

namespace Efl.Eo
{

internal partial class NativeModule
{
    [DllImport(efl.Libs.Kernel32, EntryPoint = "LoadLibrary", CharSet = CharSet.Unicode, SetLastError = true)]
    private static extern IntPtr _LoadLibrary(string libFilename);

    internal static IntPtr LoadLibrary(string libFilename)
    {
        if (!libFilename.StartsWith("lib"))
        {
                libFilename = "lib" + libFilename + "-1";
        }
        return NativeModule._LoadLibrary(libFilename);
    }

    [DllImport(efl.Libs.Kernel32, CharSet = CharSet.Unicode, SetLastError = true)]
    internal static extern void UnloadLibrary(IntPtr handle);
}

}

#endif