using static System.Console;

public class ExampleEinaError01
{
    private static bool RegisteredErrors = false;
    private static eina.Error MyErrorNegative;
    private static eina.Error MyErrorNull;

    private static void testFunc(int n, string s)
    {
        if (!RegisteredErrors)
        {
            MyErrorNegative = eina.Error.Register("Negative number");
            MyErrorNull = eina.Error.Register("NULL pointer");
            RegisteredErrors = true;
        }

        if (n < 0)
        {
            eina.Error.Set(MyErrorNegative);
            return;
        }

        if (s == null)
        {
            eina.Error.Set(MyErrorNull);
            return;
        }
    }

    public static void Main()
    {
        eina.Config.Init();
        efl.eo.Config.Init();

        // Handling Eina_Error with exception
        try
        {
            testFunc(-1, "abc");
            eina.Error.RaiseIfOcurred();
        }
        catch(efl.EflException e)
        {
            WriteLine($"Caught error: {e.Message}");
        }

        // Handling Eina_Error directly
        testFunc(42, null);
        eina.Error err = eina.Error.Get();
        if (err != 0)
        {
            WriteLine($"Error set: {err.Message}");
        }
        eina.Error.Clear();

        // No error set
        try
        {
            testFunc(42, "abc");

            eina.Error.RaiseIfOcurred();

            err = eina.Error.Get();
            WriteLine($"Really no error? {err == eina.Error.NO_ERROR}.");
        }
        catch
        {
            WriteLine("Unspected error!!!");
        }

        // With object
        try
        {
            var obj = new example.NumberwrapperConcrete();
            obj.callback_call();
        }
        catch (efl.EflException e)
        {
            WriteLine("Exception message: " + e.Message);
        }


        WriteLine("No error message is empty string: \"{0}\"", eina.Error.NO_ERROR.Message);
        WriteLine("No error message is empty string: \"{0}\"", eina.Error.MsgGet(0));
    }
}

