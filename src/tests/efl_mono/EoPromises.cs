using System;
using System.Threading.Tasks;
using System.Threading;

namespace TestSuite
{

class TestEoPromises
{
    public static void test_simple_task_run()
    {
        efl.ILoop loop = efl.App.GetLoopMain();
        eina.Future future = loop.Idle();

        bool callbackCalled = false;
        int ret_code = 1992;

        future.Then((eina.Value value) => {
            callbackCalled = true;
            eina.Value v = new eina.Value(eina.ValueType.Int32);
            v.Set(ret_code);
            loop.Quit(v);
            return value;
        });
        eina.Value ret_value = loop.Begin();

        Test.Assert(callbackCalled, "Future loop callback must have been called.");

        Test.AssertEquals(ret_value.GetValueType(), eina.ValueType.Int32);

        int ret_from_value;
        Test.Assert(ret_value.Get(out ret_from_value));
        Test.AssertEquals(ret_from_value, ret_code);

    }

    public static void test_object_promise()
    {
        efl.ILoop loop = efl.App.GetLoopMain();
        test.Testing obj = new test.Testing();

        eina.Future future = obj.GetFuture();

        bool callbackCalled = false;
        int receivedValue = -1;
        int sentValue = 1984;
        future.Then((eina.Value value) => {
            callbackCalled = true;
            Test.AssertEquals(value.GetValueType(), eina.ValueType.Int32);
            value.Get(out receivedValue);

            return value;
        });

        obj.FulfillPromise(sentValue);

        loop.Iterate();
        Test.Assert(callbackCalled, "Future callback must have been called.");
        Test.AssertEquals(receivedValue, sentValue);
    }

    public static void test_object_promise_cancel()
    {
        efl.ILoop loop = efl.App.GetLoopMain();
        test.Testing obj = new test.Testing();

        eina.Future future = obj.GetFuture();

        bool callbackCalled = false;
        eina.Error receivedError = -1;
        eina.Error sentError = 120;
        future.Then((eina.Value value) => {
            callbackCalled = true;
            Test.AssertEquals(value.GetValueType(), eina.ValueType.Error);
            value.Get(out receivedError);

            return value;
        });

        obj.RejectPromise(sentError);

        loop.Iterate();
        Test.Assert(callbackCalled, "Future callback must have been called.");
        Test.AssertEquals(receivedError, sentError);
    }

}

class LoopConsumer
{
    public static async Task Consume(efl.ILoop loop)
    {
        Task<eina.Value> task = loop.IdleAsync();
        eina.Value v = await task;
        loop.Quit(v);
    }
}

class TestLoopEoAsyncMethods
{
    public static void test_simple_async()
    {
        efl.ILoop loop = efl.App.GetLoopMain();
        Task t = LoopConsumer.Consume(loop);

        loop.Begin();
        Test.Assert(t.Wait(1000), "Task should have been completed in time.");
    }
}

class TestEoAsyncMethods
{

    public static void test_async_fulfill()
    {
        efl.ILoop loop = efl.App.GetLoopMain();
        test.ITesting obj = new test.Testing();

        Task<eina.Value> task = obj.GetFutureAsync();

        int sentValue = 1337;

        obj.FulfillPromise(sentValue);
        loop.Iterate();

        eina.Value v = task.Result;
        Test.AssertEquals(v.GetValueType(), eina.ValueType.Int32);

        int receivedValue;
        v.Get(out receivedValue);
        Test.AssertEquals(receivedValue, sentValue);
    }

    public static void test_async_cancel()
    {
        efl.ILoop loop = efl.App.GetLoopMain();
        test.ITesting obj = new test.Testing();

        CancellationTokenSource cancelSrc = new CancellationTokenSource();
        Task<eina.Value> task = obj.GetFutureAsync(cancelSrc.Token);

        cancelSrc.Cancel();
        loop.Iterate();

        bool raised = false;
        try
        {
            eina.Value v = task.Result;
        }
        catch (AggregateException ae)
        {
            raised = true;
            ae.Handle((x) =>
            {
                Test.Assert(x is TaskCanceledException, "AggregateException must have been TaskCanceledException");
                return true;
            });
        }

        Test.Assert(raised, "AggregateException must have been raised.");
    }

    public static void test_async_reject()
    {
        efl.ILoop loop = efl.App.GetLoopMain();
        test.ITesting obj = new test.Testing();

        Task<eina.Value> task = obj.GetFutureAsync();

        eina.Error sentError = 1337;
        obj.RejectPromise(sentError);

        loop.Iterate();

        bool raised = false;
        try
        {
            eina.Value v = task.Result;
        }
        catch (AggregateException ae)
        {
            raised = true;
            ae.Handle((x) =>
            {
                Test.Assert(x is efl.FutureException, "AggregateException must have been TaskCanceledException");
                efl.FutureException ex = x as efl.FutureException;
                Test.AssertEquals(ex.Error, sentError);
                return true;
            });
        }

        Test.Assert(raised, "AggregateException must have been raised.");
    }
}
}
