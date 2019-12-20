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

namespace eldbus
{

/// <summary>Initializes Eldbus.
/// <para>Since EFL 1.23.</para>
/// </summary>
public static class Config
{
    [DllImport(efl.Libs.Eldbus)] private static extern int eldbus_init();
    [DllImport(efl.Libs.Eldbus)] private static extern int eldbus_shutdown();

    /// <summary>
    /// Initialization of the eldbus.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public static void Init()
    {
        if (eldbus_init() == 0)
        {
            throw new Efl.EflException("Failed to initialize Eldbus");
        }
    }

    /// <summary>
    /// Shutdown the eldbus.
    /// <para>Since EFL 1.23.</para>
    /// </summary>
    public static void Shutdown()
    {
        eldbus_shutdown();
    }

}

}
