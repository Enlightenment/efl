using System;

class InheritedConstructibleObject : Dummy.ConstructibleObject
{
    public InheritedConstructibleObject() : base()
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

    public bool InheritedFlag
    {
        get { return inherited; }
    }

    public override int MultiplyIntegerValue(int v)
    {
        return 3 * v;
    }
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
        Test.AssertEquals(obj.MultiplyIntegerValue(21), 42);
        obj.Dispose();
    }

    public static void TestInheritedEoDirectConstruction()
    {
        var obj = new InheritedConstructibleObject();
        Test.AssertEquals(obj.InheritedFlag, true);
        Test.AssertEquals(obj.NativeConstructionCount, 1);
        Test.AssertEquals(obj.DefaultConstructionCount, 1);
        Test.AssertEquals(obj.SpecialConstructionCount, 0);
        Test.AssertEquals(obj.DefaultConstrutorCallCount, 1);
        Test.AssertEquals(obj.SpecialConstrutorCallCount, 0);
        Test.AssertEquals(obj.MultiplyIntegerValue(21), 63);
        obj.Dispose();
    }

    public static void TestInheritedEoIndirectConstruction()
    {
        var obj = new Dummy.ConstructibleObject();
        Test.AssertEquals(obj.NativeConstructionCount, 1);
        Test.AssertEquals(obj.DefaultConstructionCount, 0);
        Test.AssertEquals(obj.SpecialConstructionCount, 0);
        Test.AssertEquals(obj.MultiplyIntegerValue(21), 42);

        var obj2 = (InheritedConstructibleObject) obj.ConstructTypeAndStore(typeof(InheritedConstructibleObject));
        Test.AssertEquals(obj2.InheritedFlag, true);
        Test.AssertEquals(obj2.NativeConstructionCount, 1);
        Test.AssertEquals(obj2.DefaultConstructionCount, 0);
        Test.AssertEquals(obj2.SpecialConstructionCount, 1);
        Test.AssertEquals(obj2.DefaultConstrutorCallCount, 0);
        Test.AssertEquals(obj2.SpecialConstrutorCallCount, 1);
        Test.AssertEquals(obj2.MultiplyIntegerValue(21), 63);

        var internalObj = obj.InternalObject;
        Test.Assert(ReferenceEquals(obj2, internalObj)); // Ensure it always use the same object instance
        Test.AssertEquals(obj2.NativeConstructionCount, 1); // And that constructors are not called again
        Test.AssertEquals(obj2.MultiplyIntegerValue(32), 96); // And that it is still usable

        obj.Dispose();
        obj2.Dispose();
    }
}

}
