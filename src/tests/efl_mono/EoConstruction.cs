using System;

class InheritedConstructibleObject : Dummy.ConstructibleObject
{
    public InheritedConstructibleObject()
    {
        if (this.NativeConstructionCount != 1)
        {
            DefaultConstrutorCallCount = -100;
        }

        ++DefaultConstrutorCallCount;
        this.IncrementDefaultConstructionCount();
    }

    private InheritedConstructibleObject(ConstructingHandle ch) : base(ch)
    {
        if (this.NativeConstructionCount != 1)
        {
            SpecialConstrutorCallCount = -100;
        }

        ++SpecialConstrutorCallCount;
        this.IncrementSpecialConstructionCount();
    }

    public int DefaultConstrutorCallCount { get; set; } = 0;
    public int SpecialConstrutorCallCount { get; set; } = 0;
}

namespace TestSuite
{

class TestEoConstruction
{
    public static void TestGeneratedEoDirectConstruction()
    {
        var obj = new Dummy.ConstructibleObject();
        Test.AssertEquals(obj.NativeConstructionCount, 1);
        Test.AssertEquals(obj.DefaultConstructionCount, 0);
        Test.AssertEquals(obj.SpecialConstructionCount, 0);
        obj.Dispose();
    }

    public static void TestInheritedEoDirectConstruction()
    {
        var obj = new InheritedConstructibleObject();
        Test.AssertEquals(obj.NativeConstructionCount, 1);
        Test.AssertEquals(obj.DefaultConstructionCount, 1);
        Test.AssertEquals(obj.SpecialConstructionCount, 0);
        Test.AssertEquals(obj.DefaultConstrutorCallCount, 1);
        Test.AssertEquals(obj.SpecialConstrutorCallCount, 0);
        obj.Dispose();
    }

    public static void TestInheritedEoIndirectConstruction()
    {
        var obj = new Dummy.ConstructibleObject();
        Test.AssertEquals(obj.NativeConstructionCount, 1);
        Test.AssertEquals(obj.DefaultConstructionCount, 0);
        Test.AssertEquals(obj.SpecialConstructionCount, 0);

        var obj2 = (InheritedConstructibleObject) obj.ConstructTypeAndStore(typeof(InheritedConstructibleObject));
        Test.AssertEquals(obj2.NativeConstructionCount, 1);
        Test.AssertEquals(obj2.DefaultConstructionCount, 0);
        Test.AssertEquals(obj2.SpecialConstructionCount, 1);
        Test.AssertEquals(obj2.DefaultConstrutorCallCount, 0);
        Test.AssertEquals(obj2.SpecialConstrutorCallCount, 1);

        var internalObj = obj.InternalObject;
        Test.Assert(obj2 == internalObj); // Ensure it always use the same object instance
        Test.AssertEquals(obj2.NativeConstructionCount, 1); // And that constructors are not called again

        obj.Dispose();
        obj2.Dispose();
    }
}

}
