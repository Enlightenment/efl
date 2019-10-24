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

namespace Efl
{

namespace Eo
{

///<summary>Wraps a native module that was opened with dlopen/LoadLibrary.</summary>
public partial class NativeModule : IDisposable
{
    private Lazy<IntPtr> module;

    ///<summary>Lazily tries to load the module with the given name.</summary>
    ///<param name="libName">The name of the module to load.</param>
    public NativeModule(string libName)
    {
        module = new Lazy<IntPtr>
            (() =>
             {
                 return LoadLibrary(libName);
             });
    }

    ///<summary>The module that was loaded.</summary>
    public IntPtr Module
    {
        get
        {
            return module.Value;
        }
    }

    ///<summary>Unload and released the handle to the wrapped module.</summary>
    public void Dispose()
    {
        UnloadLibrary(module.Value);
        module = null;
    }
}

}

}
