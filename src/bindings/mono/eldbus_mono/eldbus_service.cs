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
using System.ComponentModel;

using static eldbus.EldbusServiceNativeFunctions;

namespace eldbus
{

[EditorBrowsable(EditorBrowsableState.Never)]
internal static class EldbusServiceNativeFunctions
{
    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_service_interface_register(IntPtr conn, string path, IntPtr desc);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_service_interface_fallback_register(IntPtr conn, string path, IntPtr desc);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_service_interface_register2(IntPtr conn, string path, IntPtr desc);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_service_interface_fallback_register2(IntPtr conn, string path, IntPtr desc);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_service_interface_unregister(IntPtr iface);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_service_object_unregister(IntPtr iface);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_service_connection_get(IntPtr iface);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_service_object_path_get(IntPtr iface);

//     [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
//         eldbus_service_signal_emit(IntPtr iface, uint signal_id, ...);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_service_signal_new(IntPtr iface, uint signal_id);

    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_service_signal_send(IntPtr iface, IntPtr signal_msg);

    [DllImport(efl.Libs.Eldbus)] public static extern void
        eldbus_service_object_data_set(IntPtr iface, string key, IntPtr data);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_service_object_data_get(IntPtr iface, string key);

    [DllImport(efl.Libs.Eldbus)] public static extern IntPtr
        eldbus_service_object_data_del(IntPtr iface, string key);

    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_service_property_changed(IntPtr iface, string name);

    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_service_property_invalidate_set(IntPtr iface, string name, [MarshalAs(UnmanagedType.U1)] bool is_invalidate);

    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_service_object_manager_attach(IntPtr iface);

    [DllImport(efl.Libs.Eldbus)] [return: MarshalAs(UnmanagedType.U1)] public static extern bool
        eldbus_service_object_manager_detach(IntPtr iface);
}

}
