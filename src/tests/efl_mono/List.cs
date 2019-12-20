using System;
using System.Collections.Generic;
using System.Linq;

namespace TestSuite {

class TestList
{
    public static void TestAdd()
    {
        IList<int> list = new Eina.List<int>();
        Test.AssertEquals(list.Count, 0);
        list.Add(1);
        Test.AssertEquals(list.Count, 1);
        list.Add(2);
        Test.AssertEquals(list.Count, 2);
    }

    public static void TestRemoveAt()
    {
        IList<int> list = new Eina.List<int>();
        Test.AssertEquals(list.Count, 0);
        list.Add(1);
        Test.AssertEquals(list.Count, 1);
        Test.AssertEquals(list[0], 1);
        list.RemoveAt(0);
        Test.AssertEquals(list.Count, 0);
        list.Add(1);
        list.Add(0);
        list.Add(1);
        Test.AssertEquals(list.Count, 3);
        Test.AssertEquals(list[2], 1);
        list.RemoveAt(2);
        Test.AssertEquals(list.Count, 2);
        Test.AssertEquals(list[0], 1);
    }

    public static void TestRemove()
    {
        IList<int> list = new Eina.List<int>();
        Test.Assert(!list.Remove(0));
        Test.AssertEquals(list.Count, 0);
        list.Add(1);
        Test.AssertEquals(list.Count, 1);
        Test.Assert(list.Remove(1));
        Test.AssertEquals(list.Count, 0);
        list.Add(1);
        list.Add(1);
        Test.AssertEquals(list.Count, 2);
        Test.Assert(list.Remove(1));
        Test.AssertEquals(list.Count, 1);
        list.Add(0);
        list.Add(1);
        Test.AssertEquals(list[1], 0);
        Test.AssertEquals(list.Count, 3);
        Test.Assert(!list.Remove(2));
        Test.Assert(list.Remove(1));
        Test.AssertEquals(list[1], 1);
        Test.AssertEquals(list.Count, 2);
    }

    public static void TestContains()
    {
        IList<int> list = new Eina.List<int>();
        Test.AssertEquals(list.Count, 0);
        Test.Assert(!list.Contains(0));
        list.Add(0);
        Test.Assert(list.Contains(0));
        Test.Assert(list.Remove(0));
        Test.Assert(!list.Contains(0));
        list.Add(1);
        list.Add(0);
        Test.Assert(list.Contains(0));
    }

    public static void TestClear()
    {
        IList<int> list = new Eina.List<int>();
        Test.AssertEquals(list.Count, 0);
        list.Clear();
        Test.AssertEquals(list.Count, 0);
        list.Add(0);
        Test.AssertEquals(list.Count, 1);
        list.Clear();
        Test.AssertEquals(list.Count, 0);
        list.Add(0);
        list.Add(0);
        Test.AssertEquals(list.Count, 2);
        list.Clear();
        Test.AssertEquals(list.Count, 0);
        list.Add(0);
        list.Add(0);
        Test.AssertEquals(list.Count, 2);
        Test.Assert(list.Remove(0));
        Test.AssertEquals(list.Count, 1);
        list.Clear();
        Test.AssertEquals(list.Count, 0);
    }

    public static void TestInsert()
    {
        IList<int> list = new Eina.List<int>();
        list.Add(99);
        Test.AssertEquals(list.Count, 1);
        list.Insert(0, 6);
        Test.AssertEquals(list.Count, 2);
        Test.AssertEquals(list[0], 6);
        list.Insert(1, 5);
        Test.AssertEquals(list.Count, 3);
        Test.AssertEquals(list[1], 5);
        list.Insert(1, 10);
        Test.AssertEquals(list.Count, 4);
        Test.AssertEquals(list[1], 10);
        list.RemoveAt(1);
        Test.AssertEquals(list.Count, 3);
        Test.AssertEquals(list[1], 5);
        list.Insert(1, 42);
        Test.AssertEquals(list.Count, 4);
        Test.AssertEquals(list[1], 42);
    }

    public static void TestIndexOf()
    {
        IList<int> list = new Eina.List<int>();
        Test.AssertEquals(list.Count, 0);
        list.Add(1);
        Test.AssertEquals(list.Count, 1);
        Test.AssertEquals(0, list.IndexOf(1));
        list.Insert(0, 0);
        Test.AssertEquals(list.Count, 2);
        Test.AssertEquals(0, list.IndexOf(0));
        Test.AssertEquals(1, list.IndexOf(1));
        list.Insert(0, 1);
        Test.AssertEquals(list.Count, 3);
        Test.AssertEquals(0, list.IndexOf(1));
        Test.AssertEquals(1, list.IndexOf(0));
        list.Insert(0, 1);
        Test.AssertEquals(list.Count, 4);
        Test.AssertEquals(0, list.IndexOf(1));
        Test.AssertEquals(2, list.IndexOf(0));
    }

    public static void TestCopyTo()
    {
        IList<int> list = new Eina.List<int>();
        Test.AssertEquals(list.Count, 0);
        int[] random = {4, 40, 10, 42, 99};
        list.Add(43);
        list.Insert(0, 1);
        list.Insert(1, 50);
        Test.AssertEquals(list.Count, 3);
        Array.ForEach(random, (n) => list.Add(n));
        Test.AssertEquals(list.Count, 8);
        int[] expected = {1, 50, 43, 4, 40, 10, 42, 99};
        var result = new int[expected.Length];
        list.CopyTo(result, 0);
        for (int i = 0; i < expected.Length; ++i)
        {
            Test.AssertEquals(expected[i], result[i]);
        }
    }

    public static void TestIdxOutBounds()
    {
        var list = new Eina.List<int>();
        list.Add(4);
        Test.AssertRaises<ArgumentOutOfRangeException>(()=>list.Nth(1));
        Test.AssertRaises<ArgumentOutOfRangeException>(()=>list.Nth(-1));
        Test.AssertNotRaises<ArgumentOutOfRangeException>
            (()=>list.Nth(list.IndexOf(4)));
    }

    public static void TestReadOnly()
    {
        var list = new Eina.List<int>();
        int[] array = {6, 5, 4, 3, 2};
        list.Append(array);
        Test.AssertEquals(list.Count, 5);
        Test.AssertRaises<InvalidOperationException>(() => list.SetOwnership(false, true));
        list.SetOwnership(false);
        Test.AssertRaises<NotSupportedException>(() => list.Add(4));
        Test.AssertRaises<NotSupportedException>(() => list.Append(6));
        Test.AssertRaises<NotSupportedException>(() => list.Append(array));
        Test.AssertRaises<NotSupportedException>(() => list.Remove(6));
        Test.AssertEquals(list.Count, 5);
        Test.AssertRaises<NotSupportedException>(() => list.RemoveAt(2));
        Test.AssertRaises<NotSupportedException>(() => list.Insert(4, 4));
        Test.AssertRaises<NotSupportedException>(() => list.Clear());
        Test.AssertRaises<NotSupportedException>(() => list.Sort());
        Test.Assert(list.ToArray().SequenceEqual(array));
    }
}
};
