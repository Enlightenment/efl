using static System.Console;

class PlusTenNumberWrapper : example.NumberwrapperInherit
{
    public PlusTenNumberWrapper(efl.Object parent = null)
        : base(parent)
    {}

    public bool derivedCalled = false;

    override public void SetNumber(int n)
    {
        // Call native EFL method
        base.SetNumber(n + 10);
        derivedCalled = true;
    }
}

public class ExampleEoInherit01
{
    public static void Main()
    {
        eina.Config.Init();
        efl.eo.Config.Init();

        var inheritObj = new PlusTenNumberWrapper();

        WriteLine("## Using inherit object ##\n");

        int given = 111;

        // Call the C# override from the C method
        example.NumberwrapperConcrete.example_numberwrapper_number_set(inheritObj.raw_handle, given);

        WriteLine($"Override successfully called? {inheritObj.derivedCalled}!\n");

        // Call C function from C# object
        int stored = inheritObj.GetNumber();

        WriteLine($"Given value: {given}");
        WriteLine($"Stored value: {stored}\n");

        // Call C# override directly
        given = 333;
        inheritObj.SetNumber(given);

        stored = inheritObj.GetNumber();

        WriteLine($"Given value: {given}");
        WriteLine($"Stored value: {stored}\n");

        WriteLine("## Using original object ##\n");

        // Check original EFL object
        var origObj = new example.NumberwrapperConcrete();
        given = 111;
        origObj.SetNumber(given);
        stored = origObj.GetNumber();

        WriteLine($"Given value: {given}");
        WriteLine($"Stored value: {stored}\n");
    }
}

