/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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

    /// <summary>Pointer to the native class description.</summary>
    public bool IsInheritedClass
    {
        get { return !this.IsGeneratedBindingClass; }
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
        Test.AssertEquals(obj.IsInheritedClass, true);
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
        Test.AssertEquals(obj2.IsInheritedClass, true);
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
