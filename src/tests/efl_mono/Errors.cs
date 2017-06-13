using System;

namespace TestSuite
{

class TestEinaError
{
    public static void basic_test()
    {
        eina.Error.Clear();
        Test.AssertNotRaises<efl.EflException>(eina.Error.RaiseIfOcurred);
        eina.Error.Set(eina.Error.ENOENT);
        Test.AssertRaises<efl.EflException>(eina.Error.RaiseIfOcurred);
    }
}

class TestEolianError
{

    public static void global_eina_error()
    {
        test.Testing obj = new test.TestingConcrete();
        Test.AssertRaises<efl.EflException>(() => obj.raises_eina_error());
    }

    class Child : test.TestingInherit {
    }

    public static void global_eina_error_inherited()
    {
        test.Testing obj = new Child();
        Test.AssertRaises<efl.EflException>(() => obj.raises_eina_error());
    }

    class CustomException : Exception {
        public CustomException(string msg): base(msg) {}
    }

    class Overrider : test.TestingInherit {
        public override void children_raise_error() {
            throw (new CustomException("Children error"));
        }
    }

    public static void exception_raised_from_inherited_virtual()
    {
        test.Testing obj = new Overrider();

        Test.AssertRaises<efl.EflException>(obj.call_children_raise_error);
    }

    // return eina_error
    // events
    // virtual callbacks
}
}
