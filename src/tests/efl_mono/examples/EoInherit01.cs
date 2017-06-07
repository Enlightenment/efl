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
        var obj = new PlusTenNumberWrapper();

        int given = 111;

        // Call the C# override from the C method
        test.NumberwrapperConcrete.test_numberwrapper_number_set(obj.raw_handle, given);

        WriteLine($"Override successfully called? {obj.derivedCalled}!\n");

        // Call C function from C# obj
        int stored = obj.number_get();

        // Print actual value
        WriteLine($"Given value: {given}");
        WriteLine($"Stored value: {stored}\n");

        // Call C# override directly
        given = 333;
        obj.number_set(given);

        stored = obj.number_get();

        // Print actual value
        WriteLine($"Given value: {given}");
        WriteLine($"Stored value: {stored}\n");
    }
}

}

