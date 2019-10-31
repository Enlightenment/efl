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
#if !WIN32

using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

using static EldbusTestUtil;
using static Test;

public static class EldbusTestUtil
{
    public const string DBusBus = "org.freedesktop.DBus";
    public const string DBusInterface = "org.freedesktop.DBus";
    public const string DBusPath = "/org/freedesktop/DBus";

    [return: MarshalAs(UnmanagedType.U1)] public delegate bool Ecore_Task_Cb(IntPtr data);
}

namespace TestSuite
{

class TestEldbusConnection
{
    public static void eldbus_connection_new_session()
    {
        var conn = new eldbus.Connection(eldbus.Connection.Type.Session);
        conn.Dispose();
    }

    public static void eldbus_connection_new_starter()
    {
        var conn = new eldbus.Connection(eldbus.Connection.Type.Starter);
        conn.Dispose();
    }

    public static void eldbus_connection_new_private_session()
    {
        var conn = eldbus.Connection.GetPrivate(eldbus.Connection.Type.Session);
        conn.Dispose();
    }

    public static void eldbus_connection_new_private_starter()
    {
        var conn = eldbus.Connection.GetPrivate(eldbus.Connection.Type.Starter);
        conn.Dispose();
    }

    public static void eldbus_connection_get_unique_name()
    {
        var conn = new eldbus.Connection(eldbus.Connection.Type.Session);
        Console.WriteLine(conn.GetUniqueName());
        conn.Dispose();
    }
}

class TestEldbusObject
{
    public static void utc_eldbus_object_send_info_get_p()
    {
        var app = Efl.App.AppMain;
        var conn = new eldbus.Connection(eldbus.Connection.Type.Session);

        var obj = new eldbus.Object(conn, DBusBus, DBusPath);

        string busFromObject = obj.GetBusName();

        AssertEquals(DBusBus, busFromObject);

        string pathFromObject = obj.GetPath();

        AssertEquals(DBusPath, pathFromObject);

        obj.Ref();
        obj.Unref();

        {
            var connectionFromObj = obj.GetConnection();
            Assert(conn.Handle == connectionFromObj.Handle);
            connectionFromObj.Dispose();
        }

        int messageCapture = 0;
        bool isSuccess = false;

        eldbus.MessageDelegate objectMessageCb = delegate(eldbus.Message msg, eldbus.Pending p)
        {
            try
            {
                string errname;
                string errmsg;

                if (messageCapture == 5)
                {
                    if (!msg.GetError(out errname, out errmsg))
                    {
                        string txt;
                        if (msg.Get(out txt))
                        {
                            if (!String.IsNullOrEmpty(txt))
                                isSuccess = true;
                        }
                    }
                }
            }
            finally
            {
                app.Quit(0);
            }
        };

        var methodName = "GetId";
        var message = obj.NewMethodCall(DBusInterface, methodName);

        eldbus.Pending pending = obj.Send(message, objectMessageCb, -1);

        AssertEquals(pending.GetMethod(), methodName);

        var timer = new Efl.LoopTimer(app, 2.0);
        timer.TimerTickEvent += (object sender, EventArgs e) => {
            app.Quit(0);
        };

        messageCapture = 5;

        app.Begin();

        Assert(isSuccess, $"Method {methodName} is not call");

        timer.Dispose();
        message.Dispose();
        obj.Dispose();
        conn.Dispose();
    }

    public static void utc_eldbus_introspect_p()
    {
        var app = Efl.App.AppMain;
        var conn = new eldbus.Connection(eldbus.Connection.Type.Session);

        var obj = new eldbus.Object(conn, DBusBus, DBusPath);

        int messageCapture = 0;
        bool isSuccess = false;

        eldbus.MessageDelegate objectMessageCb = delegate(eldbus.Message msg, eldbus.Pending p)
        {
            try
            {
                string errname;
                string errmsg;

                if (messageCapture == 5)
                {
                    if (!msg.GetError(out errname, out errmsg))
                    {
                        string txt;
                        if (msg.Get(out txt))
                        {
                            if (!String.IsNullOrEmpty(txt))
                                isSuccess = true;
                        }
                    }
                }
            }
            finally
            {
                app.Quit(0);
            }
        };

        eldbus.Pending pending = obj.Introspect(objectMessageCb);

        AssertEquals(pending.GetMethod(), "Introspect");

        var timer = new Efl.LoopTimer(app, 2.0);
        timer.TimerTickEvent += (object sender, EventArgs e) => {
            app.Quit(0);
        };

        messageCapture = 5;

        app.Begin();

        Assert(isSuccess, "Method Introspect is not call");

        timer.Dispose();
        obj.Dispose();
        conn.Dispose();
    }
}

class TestEldbusMessage
{
    private static bool isSuccess = false;


    private static void ActivatableList(eldbus.MessageDelegate messageCb)
    {
        isSuccess = false;

        var app = Efl.App.AppMain;
        var conn = new eldbus.Connection(eldbus.Connection.Type.Session);

        eldbus.Pending pending = conn.ActivatableList(messageCb);

        AssertEquals(pending.GetMethod(), "ListActivatableNames");

        var timer = new Efl.LoopTimer(app, 2.0);
        timer.TimerTickEvent += (object sender, EventArgs e) => {
            app.Quit(0);
        };

        app.Begin();

        Assert(isSuccess, "Method ListActivatableNames is not call");

        timer.Dispose();
        conn.Dispose();
    }

    public static void utc_eldbus_message_iterator_activatable_list_p()
    {
        eldbus.MessageDelegate responseMessageCb = delegate(eldbus.Message msg, eldbus.Pending p)
        {
            try
            {
                string errname, errmsg;
                if (msg.GetError(out errname, out errmsg))
                {
                    return;
                }

                eldbus.MessageIterator iterMain = msg.GetMessageIterator();

                string signature = iterMain.GetSignature();
                if (String.IsNullOrEmpty(signature))
                {
                    return;
                }

                eldbus.MessageIterator iterator;
                iterMain.Get(out iterator, signature);

                string bus_name;
                bool isHasData = false;

                while (iterator.GetAndNext(out bus_name))
                {
                    if (String.IsNullOrEmpty(bus_name))
                    {
                        return;
                    }
                    isHasData = true;
                }

                isSuccess = isHasData;
            }
            finally
            {
                Efl.App.AppMain.Quit(0);
            }
        };

        ActivatableList(responseMessageCb);
    }

    public static void utc_eldbus_message_info_data_get_p()
    {
        isSuccess = false;

        var app = Efl.App.AppMain;
        var conn = new eldbus.Connection(eldbus.Connection.Type.Session);

        string methodName = "GetId";
        eldbus.Message msg = eldbus.Message.NewMethodCall(DBusBus, DBusPath, DBusInterface, methodName);

        string interfaceMsg = msg.GetInterface();
        AssertEquals(DBusInterface, interfaceMsg);

        string methodMsg = msg.GetMember();
        AssertEquals(methodName, methodMsg);

        string pathMsg = msg.GetPath();
        AssertEquals(DBusPath, pathMsg);

        eldbus.MessageDelegate messageMethodCb = delegate(eldbus.Message m, eldbus.Pending p)
        {
            try
            {
                string errname, errmsg;
                if (!m.GetError(out errname, out errmsg))
                {
                    string txt;
                    if (m.Get(out txt))
                    {
                        if (!String.IsNullOrEmpty(txt))
                        {
                            if (m.GetSender() == DBusBus &&
                                !String.IsNullOrEmpty(m.GetDestination()))
                            {
                                isSuccess = true;
                            }
                        }
                    }
                }
            }
            finally
            {
                app.Quit(0);
            }
        };

        const int timeoutSendMs = 1000;
        eldbus.Pending pending = conn.Send(msg, messageMethodCb, timeoutSendMs);

        AssertEquals(pending.GetMethod(), methodName);

        var timer = new Efl.LoopTimer(app, 2.0);
        timer.TimerTickEvent += (object sender, EventArgs e) => {
            app.Quit(0);
        };

        app.Begin();

        Assert(isSuccess, $"Method {methodName} is not call");

        timer.Dispose();
        msg.Dispose();
        conn.Dispose();
    }

    public static void utc_eldbus_message_ref_unref_p()
    {
        var conn = new eldbus.Connection(eldbus.Connection.Type.Session);

        eldbus.Message msg = eldbus.Message.NewMethodCall(DBusBus, DBusPath, DBusInterface, "GetId");

        msg.Ref();
        msg.Unref();

        string pathMsg = msg.GetPath();
        AssertEquals(DBusPath, pathMsg);

        msg.Unref();
        msg.Own = false;

        msg.Dispose();
        conn.Dispose();
    }

    public static void utc_eldbus_message_iter_next_p()
    {
        eldbus.MessageDelegate activatableListResponseCb = delegate(eldbus.Message msg, eldbus.Pending p)
        {
            try
            {
                string errname, errmsg;
                if (msg.GetError(out errname, out errmsg))
                {
                    return;
                }

                eldbus.MessageIterator iterMain = msg.GetMessageIterator();

                string signature = iterMain.GetSignature();
                if (String.IsNullOrEmpty(signature))
                {
                    return;
                }

                eldbus.MessageIterator iterator;
                iterMain.Get(out iterator, signature);

                bool isHasData = false;
                do
                {
                    string busName;
                    iterator.Get(out busName);
                    if (String.IsNullOrEmpty(busName))
                    {
                        return;
                    }
                    isHasData = true;
                } while (iterator.Next());

                isSuccess = isHasData;
            }
            finally
            {
                Efl.App.AppMain.Quit(0);
            }
        };
        ActivatableList(activatableListResponseCb);
    }
}

}

#endif
