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
using System.Linq;
using System.Collections.Generic;

namespace TestSuite
{

class TestEoEvents
{
    public bool called = false;
    public bool correct_sender = false;
    public Efl.Loop loop { get; set; }
    protected void callback(object sender, EventArgs e) {
        called = true;
        Efl.Object obj = sender as Efl.Object;
        if (obj != null)
        {
            obj.Name = "loop_called";
            correct_sender = true;
        }

        Eina.Value v = new Eina.Value(Eina.ValueType.Int32);
        v.Set(0);
        loop.Quit(v);
        v.Dispose();
    }
    protected void another_callback(object sender, EventArgs e) { }

    public static void idle_event()
    {
        Efl.Loop loop = Efl.App.AppMain;
        loop.Name = "loop";
        TestEoEvents listener = new TestEoEvents();
        listener.loop = loop;
        loop.IdleEvent += listener.callback;

        Test.Assert(!listener.called);
        Test.Assert(!listener.correct_sender);
        Test.AssertEquals("loop", loop.Name);
        loop.Begin();
        Test.Assert(listener.called);
        Test.Assert(listener.correct_sender);
        Test.AssertEquals("loop_called", loop.Name);

        loop.IdleEvent -= listener.callback;
    }

    public static void event_with_string_payload()
    {
        var obj = new Dummy.TestObject();
        string received_string = null;

        obj.EvtWithStringEvent += (object sender, Dummy.TestObjectEvtWithStringEventArgs e) => {
            received_string = e.Arg;
        };

        obj.EmitEventWithString("Some args");

        Test.AssertEquals("Some args", received_string);
        obj.Dispose();
    }

    public static void event_with_int_payload()
    {
        var obj = new Dummy.TestObject();
        int received_int= 0;

        obj.EvtWithIntEvent += (object sender, Dummy.TestObjectEvtWithIntEventArgs e) => {
            received_int = e.Arg;
        };

        obj.EmitEventWithInt(-1984);

        Test.AssertEquals(-1984, received_int);
        obj.Dispose();
    }

    public static void event_with_bool_payload()
    {
        var obj = new Dummy.TestObject();
        bool received_bool = false;

        obj.EvtWithBoolEvent += (object sender, Dummy.TestObjectEvtWithBoolEventArgs e) => {
            received_bool = e.Arg;
        };

        obj.EmitEventWithBool(true);

        Test.AssertEquals(true, received_bool);

        obj.EmitEventWithBool(false);

        Test.AssertEquals(false, received_bool);
        obj.Dispose();
    }

    public static void event_with_uint_payload()
    {
        var obj = new Dummy.TestObject();
        uint received_uint = 0;
        obj.EvtWithUintEvent += (object sender, Dummy.TestObjectEvtWithUintEventArgs e) => {
            received_uint = e.Arg;
        };

        obj.EmitEventWithUint(0xbeef);

        Test.AssertEquals<uint>(0xbeef, received_uint);
        obj.Dispose();
    }

    public static void event_with_float_payload()
    {
        var obj = new Dummy.TestObject();
        float received_float = 0;
        obj.EvtWithFloatEvent += (object sender, Dummy.TestObjectEvtWithFloatEventArgs e) => {
            received_float = e.Arg;
        };

        obj.EmitEventWithFloat(3.14f);

        Test.AssertAlmostEquals(3.14f, received_float);
        obj.Dispose();
    }

    public static void event_with_double_payload()
    {
        var obj = new Dummy.TestObject();
        double received_double = 0;
        double reference = float.MaxValue + 42;
        obj.EvtWithDoubleEvent += (object sender, Dummy.TestObjectEvtWithDoubleEventArgs e) => {
            received_double = e.Arg;
        };

        obj.EmitEventWithDouble(reference);

        Test.AssertAlmostEquals(reference, received_double);
        obj.Dispose();
    }

    public static void event_with_object_payload()
    {
        var obj = new Dummy.TestObject();
        Dummy.TestObject received_obj = null;

        obj.EvtWithObjEvent += (object sender, Dummy.TestObjectEvtWithObjEventArgs e) => {
            received_obj = e.Arg;
        };

        var sent_obj = new Dummy.TestObject();

        obj.EmitEventWithObj(sent_obj);

        Test.AssertEquals(sent_obj, received_obj);
        sent_obj.Dispose();
        obj.Dispose();
    }

    public static void event_with_error_payload()
    {
        var obj = new Dummy.TestObject();
        Eina.Error received_error = 0;

        obj.EvtWithErrorEvent += (object sender, Dummy.TestObjectEvtWithErrorEventArgs e) => {
            received_error = e.Arg;
        };

        Eina.Error sent_error = -2001;

        obj.EmitEventWithError(sent_error);

        Test.AssertEquals(sent_error, received_error);
        obj.Dispose();
    }

    public static void event_with_struct_payload()
    {
        var obj = new Dummy.TestObject();
        Dummy.StructSimple received_struct = default(Dummy.StructSimple);

        obj.EvtWithStructEvent += (object sender, Dummy.TestObjectEvtWithStructEventArgs e) => {
            received_struct = e.Arg;
        };

        Dummy.StructSimple sent_struct = new Dummy.StructSimple(fstring: "Struct Event");

        obj.EmitEventWithStruct(sent_struct);

        Test.AssertEquals(sent_struct.Fstring, received_struct.Fstring);
        obj.Dispose();
    }

#if EFL_BETA
    public static void event_with_struct_complex_payload()
    {
        var obj = new Dummy.TestObject();
        Dummy.StructComplex received_struct = default(Dummy.StructComplex);

        obj.EvtWithStructComplexEvent += (object sender, Dummy.TestObjectEvtWithStructComplexEventArgs e) => {
            received_struct = e.Arg;
        };

        Dummy.StructComplex sent_struct = StructHelpers.structComplexWithValues();

        obj.EmitEventWithStructComplex(sent_struct);

        Test.AssertEquals(sent_struct.Fobj, received_struct.Fobj);
        obj.Dispose();
    }
#endif

    public static void event_with_array_payload()
    {
        var obj = new Dummy.TestObject();
        List<string> received = null;
        Eina.Array<string> sent = new Eina.Array<string>();

        sent.Append("Abc");
        sent.Append("Def");
        sent.Append("Ghi");

        obj.EvtWithArrayEvent += (object sender, Dummy.TestObjectEvtWithArrayEventArgs e) => {
            received = e.Arg as List<string>;
        };

        obj.EmitEventWithArray(sent);

        Test.AssertEquals(sent.Count, received.Count);
        var pairs = sent.Zip(received, (string sentItem, string receivedItem) => new { Sent = sentItem, Received = receivedItem } );
        foreach (var pair in pairs)
        {
            Test.AssertEquals(pair.Sent, pair.Received);
        }
        sent.Dispose();
        obj.Dispose();
    }
}

class TestEventAddRemove
{
    public static void test_add_remove_event()
    {
        var obj = new Dummy.TestObject();
        bool called = true;

        EventHandler<Dummy.TestObjectEvtWithIntEventArgs> evtCb = (object sender, Dummy.TestObjectEvtWithIntEventArgs e) => {
            called = true;
        };

        obj.EvtWithIntEvent += evtCb;
        obj.EmitEventWithInt(42);
        Test.Assert(called);

        called = false;
        obj.EvtWithIntEvent -= evtCb;
        obj.EmitEventWithInt(42);
        Test.Assert(!called);
        obj.Dispose();
    }
}

class TestInterfaceEvents
{
    public static void test_nonconflicting_events()
    {
        var obj = new Dummy.TestObject();
        var called = false;

        EventHandler cb = (object sender, EventArgs e) => {
            called = true;
        };

        obj.NonconflictedEvent += cb;
        obj.EmitNonconflicted();
        Test.Assert(called);
        obj.Dispose();
    }
}

class TestEventNaming
{
    // For events named line focus_geometry,changed
    public static void test_event_naming()
    {
        var obj = new Dummy.TestObject();
        var test_called = false;

        EventHandler cb = (object sender, EventArgs e) => {
            test_called = true;
        };

        obj.EvtWithUnderEvent += cb;

        obj.EmitEventWithUnder();

        Test.Assert(test_called);
        obj.Dispose();
    }
}

class TestEventWithDeadWrappers
{

    private static WeakReference AttachToManager(Dummy.EventManager manager,
                                          EventHandler<Dummy.TestObjectEvtWithIntEventArgs> cb)
    {
        var obj = new Dummy.TestObject();
        manager.SetEmitter(obj);

        obj.EvtWithIntEvent += cb;
        return new WeakReference(obj);
    }

    public static void test_event_from_c_owned_wrapper()
    {
        // Set upon object instantiation
        WeakReference wref = null;

        // Checks in the callback called
        bool callbackCalled = false;
        int received = -1;

        // attach to evt with int
        EventHandler<Dummy.TestObjectEvtWithIntEventArgs> cb = (object sender, Dummy.TestObjectEvtWithIntEventArgs args) => {
            callbackCalled = true;
            received = args.Arg;
            Test.Assert(Object.ReferenceEquals(sender, wref.Target));
        };

        Dummy.EventManager manager = new Dummy.EventManager();
        wref = AttachToManager(manager, cb);

        Test.CollectAndIterate();

        manager.EmitWithInt(42);

        Test.CollectAndIterate();

        Test.Assert(callbackCalled, "Callback must have been called.");
        Test.AssertEquals(42, received, "Wrong value received.");

        // Cleanup checks
        manager.Release();

        // Make sure the released wrapper is collected and release the Eo object
        Test.CollectAndIterate();

        Test.AssertNull(wref.Target);
        manager.Dispose();
    }

}

}
