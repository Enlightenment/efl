using static System.Console;

class PlusTenNumberWrapper : test.NumberwrapperInherit
{
    public PlusTenNumberWrapper(efl.Object parent = null)
        : base(parent)
    {}

    public bool derivedCalled = false;

    override public void number_set(int n)
    {
        // Call native EFL method
        base.number_set(n + 10);
        derivedCalled = true;
    }
}

namespace TestSuite
{

class TestExampleEoInherit01
{
    public static void EoInherit01()
    {
        var inheritObj = new PlusTenNumberWrapper();

        WriteLine("## Using inherit object ##\n");

        int given = 111;

        // Call the C# override from the C method
        test.NumberwrapperConcrete.test_numberwrapper_number_set(inheritObj.raw_handle, given);

        WriteLine($"Override successfully called? {inheritObj.derivedCalled}!\n");

        // Call C function from C# object
        int stored = inheritObj.number_get();

        // Print actual value
        WriteLine($"Given value: {given}");
        WriteLine($"Stored value: {stored}\n");

        // Call C# override directly
        given = 333;
        inheritObj.number_set(given);

        stored = inheritObj.number_get();

        // Print actual value
        WriteLine($"Given value: {given}");
        WriteLine($"Stored value: {stored}\n");


        WriteLine("## Using original object ##\n");

        // Check original EFL object
        var origObj = new test.NumberwrapperConcrete();
        given = 111;
        origObj.number_set(given);
        stored = origObj.number_get();

        // Print actual value
        WriteLine($"Given value: {given}");
        WriteLine($"Stored value: {stored}\n");
    }
}

}

