using static System.Console;

public class ExampleFunctionPointer01
{
    private static bool static_called = false;

    private static int twiceCb(int n)
    {
        static_called = true;
        return n * 2;
    }

    public static void Main()
    {
        eina.Config.Init();
        efl.eo.Config.Init();

        var obj = new example.NumberwrapperConcrete();

        // Set internal value
        obj.number_set(12);

        // With static method
        obj.number_callback_set(twiceCb);

        var ret = obj.callback_call();

        WriteLine($"Callback called? {static_called}.");
        WriteLine($"Returned value: {ret}.\n");

        // With lambda
        bool lamda_called = false;

        obj.number_callback_set(n => {
            lamda_called = true;
            return n * 3;
        });

        ret = obj.callback_call();

        WriteLine($"Lambda called? {lamda_called}.");
        WriteLine($"Returned value: {ret}.\n");
    }
}

