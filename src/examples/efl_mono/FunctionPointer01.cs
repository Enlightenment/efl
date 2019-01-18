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
        Eina.Config.Init();
        Efl.Eo.Config.Init();

        var obj = new Example.Numberwrapper();

        // Set internal value
        obj.SetNumber(12);

        // With static method
        obj.SetNumberCallback(twiceCb);

        var ret = obj.CallCallback();

        WriteLine($"Callback called? {static_called}.");
        WriteLine($"Returned value: {ret}.\n");

        // With lambda
        bool lamda_called = false;

        obj.SetNumberCallback(n => {
            lamda_called = true;
            return n * 3;
        });

        ret = obj.CallCallback();

        WriteLine($"Lambda called? {lamda_called}.");
        WriteLine($"Returned value: {ret}.\n");
    }
}

