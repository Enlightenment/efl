using System;
using System.Collections.Generic;

namespace TestSuite
{

class TestPromises
{
    public static void test_simple_cancel()
    {
        bool cleanCalled = false;
        Eina.Promise promise = new Eina.Promise(() => { cleanCalled = true; });
        Eina.Future future = new Eina.Future(promise);
        future.Cancel();
        Test.Assert(cleanCalled, "Promise clean callback should have been called.");
        Test.AssertRaises<ObjectDisposedException>(() => { promise.Resolve(null); });
        Test.AssertRaises<ObjectDisposedException>(future.Cancel);
    }

    public static void test_simple_resolve()
    {
        bool callbackCalled = false;
        Eina.Value received_value = null;

        Efl.Loop loop = Efl.App.GetLoopMain();
        Eina.Promise promise = new Eina.Promise();
        Eina.Future future = new Eina.Future(promise);

        future = future.Then((Eina.Value value) => {
            callbackCalled = true;
            received_value = value;
            return value;
        } );

        Eina.Value reference_value = new Eina.Value(Eina.ValueType.Int32);
        reference_value.Set(1984);
        promise.Resolve(reference_value);

        loop.Iterate();

        Test.Assert(callbackCalled, "Future callback should have been called.");
        Test.AssertEquals(received_value, reference_value);
    }

    public static void test_simple_reject()
    {
        bool callbackCalled = false;
        Eina.Error received_error = Eina.Error.NO_ERROR;

        Efl.Loop loop = Efl.App.GetLoopMain();
        Eina.Promise promise = new Eina.Promise();
        Eina.Future future = new Eina.Future(promise);

        future = future.Then((Eina.Value value) => {
            callbackCalled = true;
            value.Get(out received_error);
            return value;
        });

        promise.Reject(Eina.Error.EPERM);

        loop.Iterate();

        Test.Assert(callbackCalled, "Future callback should have been called.");
        Test.AssertEquals(received_error, Eina.Error.EPERM);

        Test.AssertRaises<ObjectDisposedException>(() => { promise.Resolve(null); });
        Test.AssertRaises<ObjectDisposedException>(future.Cancel);
    }

    public static void test_simple_future_cancel()
    {
        bool callbackCalled = false;
        bool promiseCallbackCalled = false;
        Eina.Error received_error = Eina.Error.NO_ERROR;

        Eina.Promise promise = new Eina.Promise(() => { promiseCallbackCalled = true; });
        Eina.Future future = new Eina.Future(promise);

        future = future.Then((Eina.Value value) => {
            callbackCalled = true;
            value.Get(out received_error);
            return value;
        });

        future.Cancel();

        Test.Assert(promiseCallbackCalled, "Promise cancel callback should have been called.");
        Test.Assert(callbackCalled, "Future callback should have been called.");
        Test.AssertEquals(received_error, Eina.Error.ECANCELED);
    }


    private delegate Eina.Future.ResolvedCb FutureCbGenerator(int x);
    public static void test_then_chaining()
    {
        bool[] callbacksCalled = {false, false, false, false};
        Eina.Value[] received_value = {null, null, null, null};

        FutureCbGenerator genResolvedCb = (int i) => {
            return (Eina.Value value) => {
                callbacksCalled[i] = true;
                int x;
                value.Get(out x);
                value.Set(x + i);
                received_value[i] = value;
                return value;
            };
        };

        Efl.Loop loop = Efl.App.GetLoopMain();
        Eina.Promise promise = new Eina.Promise();
        Eina.Future future = new Eina.Future(promise);
        for (int i = 0; i < 4; i++)
            future = future.Then(genResolvedCb(i));

        Eina.Value reference_value = new Eina.Value(Eina.ValueType.Int32);
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
        Eina.Value[] received_value = {null, null, null, null};

        FutureCbGenerator genResolvedCb = (int i) => {
            return (Eina.Value value) => {
                callbacksCalled[i] = true;
                int x;
                value.Get(out x);
                value.Set(x + i);
                received_value[i] = value;
                return value;
            };
        };

        var cbs = new List<Eina.Future.ResolvedCb>();
        for (int i = 0; i < 4; i++)
            cbs.Add(genResolvedCb(i));

        Efl.Loop loop = Efl.App.GetLoopMain();
        Eina.Promise promise = new Eina.Promise();
        Eina.Future future = new Eina.Future(promise);
        future = future.Chain(cbs);

        Eina.Value reference_value = new Eina.Value(Eina.ValueType.Int32);
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
        Eina.Error received_error = Eina.Error.NO_ERROR;

        Efl.Loop loop = Efl.App.GetLoopMain();
        Eina.Promise promise = new Eina.Promise();
        Eina.Future future = new Eina.Future(promise);

        future = future.Then((Eina.Value value) => {
            callbackCalled = true;
            value.Get(out received_error);
            return value;
        });

        promise.Reject(Eina.Error.EPERM);
        future.Cancel();

        loop.Iterate();

        Test.Assert(callbackCalled, "Future callback should have been called.");
        Test.AssertEquals(received_error, Eina.Error.ECANCELED);

        Test.AssertRaises<ObjectDisposedException>(() => { promise.Resolve(null); });
        Test.AssertRaises<ObjectDisposedException>(future.Cancel);
    }

    public static void test_constructor_with_callback()
    {
        bool callbackCalled = false;
        Eina.Value received_value = null;

        Efl.Loop loop = Efl.App.GetLoopMain();
        Eina.Promise promise = new Eina.Promise();
#pragma warning disable 0219
        Eina.Future future = new Eina.Future(promise,(Eina.Value value) => {
            callbackCalled = true;
            received_value = value;
            return value;
        } );
#pragma warning restore 0219

        Eina.Value reference_value = new Eina.Value(Eina.ValueType.Int32);
        reference_value.Set(1984);
        promise.Resolve(reference_value);

        loop.Iterate();

        Test.Assert(callbackCalled, "Future callback should have been called.");
        Test.AssertEquals(received_value, reference_value);
    }

    public static void test_reject_on_disposal()
    {
        bool callbackCalled = false;
        Eina.Error received_error = Eina.Error.NO_ERROR;

        Efl.Loop loop = Efl.App.GetLoopMain();
        Eina.Promise promise = new Eina.Promise();
        Eina.Future future = new Eina.Future(promise);

        future = future.Then((Eina.Value value) => {
            callbackCalled = true;
            value.Get(out received_error);
            return value;
        });

        promise.Dispose();

        loop.Iterate();

        Test.Assert(callbackCalled, "Future callback should have been called.");
        Test.AssertEquals(received_error, Eina.Error.ECANCELED);

        Test.AssertRaises<ObjectDisposedException>(() => { promise.Resolve(null); });
        Test.AssertRaises<ObjectDisposedException>(future.Cancel);
    }
}

}
