using System;

namespace TestSuite
{

class TestEinaError
{
    public static void basic_test()
    {
        eina.Error.Clear();
        Test.AssertNotRaises<efl.eo.EflException>(eina.Error.RaiseIfOcurred);
        eina.Error.Set(eina.Error.ENOENT);
        Test.AssertRaises<efl.eo.EflException>(eina.Error.RaiseIfOcurred);
    }
}

class TestEolianError
{

    public static void global_eina_error()
    {
        test.Testing obj = new test.TestingConcrete();
        Test.AssertRaises<efl.eo.EflException>(() => obj.raises_eina_error());
    }

    class Child : test.TestingInherit {
    }

    public static void global_eina_error_inherited()
    {
        test.Testing obj = new Child();
        Test.AssertRaises<efl.eo.EflException>(() => obj.raises_eina_error());
    }

    // return eina_error
    // events
    // virtual callbacks
}
}
