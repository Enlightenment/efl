using System;
using System.Collections.Generic;

namespace TestSuite
{

class TestPromises
{
    public static void test_simple_cancel()
    {
        bool cleanCalled = false;
        eina.Promise promise = new eina.Promise(() => { cleanCalled = true; });
        eina.Future future = new eina.Future(promise);
        future.Cancel();
        Test.Assert(cleanCalled, "Promise clean callback should have been called.");
        Test.AssertRaises<ObjectDisposedException>(() => { promise.Resolve(null); });
        Test.AssertRaises<ObjectDisposedException>(future.Cancel);
    }

    public static void test_simple_resolve()
    {
        bool callbackCalled = false;
        eina.Value received_value = null;

        efl.ILoop loop = efl.App.GetLoopMain();
        eina.Promise promise = new eina.Promise();
        eina.Future future = new eina.Future(promise);

        future = future.Then((eina.Value value) => {
            callbackCalled = true;
            received_value = value;
            return value;
        } );

        eina.Value reference_value = new eina.Value(eina.ValueType.Int32);
        reference_value.Set(1984);
        promise.Resolve(reference_value);

        loop.Iterate();

        Test.Assert(callbackCalled, "Future callback should have been called.");
        Test.AssertEquals(received_value, reference_value);
    }

    public static void test_simple_reject()
    {
        bool callbackCalled = false;
        eina.Error received_error = eina.Error.NO_ERROR;

        efl.ILoop loop = efl.App.GetLoopMain();
        eina.Promise promise = new eina.Promise();
        eina.Future future = new eina.Future(promise);

        future = future.Then((eina.Value value) => {
            callbackCalled = true;
            value.Get(out received_error);
            return value;
        });

        promise.Reject(eina.Error.EPERM);

        loop.Iterate();

        Test.Assert(callbackCalled, "Future callback should have been called.");
        Test.AssertEquals(received_error, eina.Error.EPERM);

        Test.AssertRaises<ObjectDisposedException>(() => { promise.Resolve(null); });
        Test.AssertRaises<ObjectDisposedException>(future.Cancel);
    }

    public static void test_simple_future_cancel()
    {
        bool callbackCalled = false;
        bool promiseCallbackCalled = false;
        eina.Error received_error = eina.Error.NO_ERROR;

        eina.Promise promise = new eina.Promise(() => { promiseCallbackCalled = true; });
        eina.Future future = new eina.Future(promise);

        future = future.Then((eina.Value value) => {
            callbackCalled = true;
            value.Get(out received_error);
            return value;
        });

        future.Cancel();

        Test.Assert(promiseCallbackCalled, "Promise cancel callback should have been called.");
        Test.Assert(callbackCalled, "Future callback should have been called.");
        Test.AssertEquals(received_error, eina.Error.ECANCELED);
    }


    private delegate eina.Future.ResolvedCb FutureCbGenerator(int x);
    public static void test_then_chaining()
    {
        bool[] callbacksCalled = {false, false, false, false};
        eina.Value[] received_value = {null, null, null, null};

        FutureCbGenerator genResolvedCb = (int i) => {
            return (eina.Value value) => {
                callbacksCalled[i] = true;
                int x;
                value.Get(out x);
                value.Set(x + i);
                received_value[i] = value;
                return value;
            };
        };

        efl.ILoop loop = efl.App.GetLoopMain();
        eina.Promise promise = new eina.Promise();
        eina.Future future = new eina.Future(promise);
        for (int i = 0; i < 4; i++)
            future = future.Then(genResolvedCb(i));

        eina.Value reference_value = new eina.Value(eina.ValueType.Int32);
        reference_value.Set(0);
        promise.Resolve(reference_value);

        loop.Iterate();

        int current_value = 0;
        for (int i = 0; i < 4; i++)
        {
            current_value += i;
            Test.Assert(callbacksCalled[i], $"Future callback {i} should have been called.");
            int received;
            received_value[i].Get(out received);
            Test.AssertEquals(received, current_value);
        }

        Test.AssertRaises<ObjectDisposedException>(() => { promise.Resolve(null); });
        Test.AssertRaises<ObjectDisposedException>(future.Cancel);
    }

    public static void test_then_chain_array()
    {
        bool[] callbacksCalled = {false, false, false, false};
        eina.Value[] received_value = {null, null, null, null};

        FutureCbGenerator genResolvedCb = (int i) => {
            return (eina.Value value) => {
                callbacksCalled[i] = true;
                int x;
                value.Get(out x);
                value.Set(x + i);
                received_value[i] = value;
                return value;
            };
        };

        var cbs = new List<eina.Future.ResolvedCb>();
        for (int i = 0; i < 4; i++)
            cbs.Add(genResolvedCb(i));

        efl.ILoop loop = efl.App.GetLoopMain();
        eina.Promise promise = new eina.Promise();
        eina.Future future = new eina.Future(promise);
        future = future.Chain(cbs);

        eina.Value reference_value = new eina.Value(eina.ValueType.Int32);
        reference_value.Set(0);
        promise.Resolve(reference_value);

        loop.Iterate();

        int current_value = 0;
        for (int i = 0; i < 4; i++)
        {
            current_value += i;
            Test.Assert(callbacksCalled[i], $"Future chained callback {i} should have been called.");
            int received;
            received_value[i].Get(out received);
            Test.AssertEquals(received, current_value);
        }

        Test.AssertRaises<ObjectDisposedException>(() => { promise.Resolve(null); });
        Test.AssertRaises<ObjectDisposedException>(future.Cancel);
    }

    public static void test_cancel_after_resolve()
    {
        bool callbackCalled = false;
        eina.Error received_error = eina.Error.NO_ERROR;

        efl.ILoop loop = efl.App.GetLoopMain();
        eina.Promise promise = new eina.Promise();
        eina.Future future = new eina.Future(promise);

        future = future.Then((eina.Value value) => {
            callbackCalled = true;
            value.Get(out received_error);
            return value;
        });

        promise.Reject(eina.Error.EPERM);
        future.Cancel();

        loop.Iterate();

        Test.Assert(callbackCalled, "Future callback should have been called.");
        Test.AssertEquals(received_error, eina.Error.ECANCELED);

        Test.AssertRaises<ObjectDisposedException>(() => { promise.Resolve(null); });
        Test.AssertRaises<ObjectDisposedException>(future.Cancel);
    }

    public static void test_constructor_with_callback()
    {
        bool callbackCalled = false;
        eina.Value received_value = null;

        efl.ILoop loop = efl.App.GetLoopMain();
        eina.Promise promise = new eina.Promise();
#pragma warning disable 0219
        eina.Future future = new eina.Future(promise,(eina.Value value) => {
            callbackCalled = true;
            received_value = value;
            return value;
        } );
#pragma warning restore 0219

        eina.Value reference_value = new eina.Value(eina.ValueType.Int32);
        reference_value.Set(1984);
        promise.Resolve(reference_value);

        loop.Iterate();

        Test.Assert(callbackCalled, "Future callback should have been called.");
        Test.AssertEquals(received_value, reference_value);
    }
}

}
