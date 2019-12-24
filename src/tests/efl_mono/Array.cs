using System;
using System.Collections.Generic;
using System.Linq;

namespace TestSuite 
{

class TestArray
{
    public static void TestAdd()
    {
        IList<int> array = new Eina.Array<int>();
        Test.AssertEquals(array.Count, 0);
        array.Add(1);
        Test.AssertEquals(array.Count, 1);
        array.Add(2);
        Test.AssertEquals(array.Count, 2);
    }

    public static void TestRemoveAt()
    {
        IList<int> array = new Eina.Array<int>();
        Test.AssertEquals(array.Count, 0);
        array.Add(1);
        Test.AssertEquals(array.Count, 1);
        Test.AssertEquals(array[0], 1);
        array.RemoveAt(0);
        Test.AssertEquals(array.Count, 0);
        array.Add(1);
        array.Add(0);
        array.Add(1);
        Test.AssertEquals(array.Count, 3);
        Test.AssertEquals(array[2], 1);
        array.RemoveAt(2);
        Test.AssertEquals(array.Count, 2);
        Test.AssertEquals(array[0], 1);
    }

    public static void TestRemove()
    {
        IList<int> array = new Eina.Array<int>();
        Test.Assert(!array.Remove(0));
        Test.AssertEquals(array.Count, 0);
        array.Add(1);
        Test.AssertEquals(array.Count, 1);
        Test.Assert(array.Remove(1));
        Test.AssertEquals(array.Count, 0);
        array.Add(1);
        array.Add(1);
        Test.AssertEquals(array.Count, 2);
        Test.Assert(array.Remove(1));
        Test.AssertEquals(array.Count, 1);
        array.Add(0);
        array.Add(1);
        Test.AssertEquals(array[1], 0);
        Test.AssertEquals(array.Count, 3);
        Test.Assert(!array.Remove(2));
        Test.Assert(array.Remove(1));
        Test.AssertEquals(array[1], 1);
        Test.AssertEquals(array.Count, 2);
    }

    public static void TestContains()
    {
        IList<int> array = new Eina.Array<int>();
        Test.AssertEquals(array.Count, 0);
        Test.Assert(!array.Contains(0));
        array.Add(0);
        Test.Assert(array.Contains(0));
        Test.Assert(array.Remove(0));
        Test.Assert(!array.Contains(0));
        array.Add(1);
        array.Add(0);
        Test.Assert(array.Contains(0));
    }

    public static void TestClear()
    {
        IList<int> array = new Eina.Array<int>();
        Test.AssertEquals(array.Count, 0);
        array.Clear();
        Test.AssertEquals(array.Count, 0);
        array.Add(0);
        Test.AssertEquals(array.Count, 1);
        array.Clear();
        Test.AssertEquals(array.Count, 0);
        array.Add(0);
        array.Add(0);
        Test.AssertEquals(array.Count, 2);
        array.Clear();
        Test.AssertEquals(array.Count, 0);
        array.Add(0);
        array.Add(0);
        Test.AssertEquals(array.Count, 2);
        Test.Assert(array.Remove(0));
        Test.AssertEquals(array.Count, 1);
        array.Clear();
        Test.AssertEquals(array.Count, 0);
    }

    public static void TestInsert()
    {
        IList<int> array = new Eina.Array<int>();
        array.Add(99);
        Test.AssertEquals(array.Count, 1);
        array.Insert(0, 6);
        Test.AssertEquals(array.Count, 2);
        Test.AssertEquals(array[0], 6);
        array.Insert(1, 5);
        Test.AssertEquals(array.Count, 3);
        Test.AssertEquals(array[1], 5);
        array.Insert(1, 10);
        Test.AssertEquals(array.Count, 4);
        Test.AssertEquals(array[1], 10);
        array.RemoveAt(1);
        Test.AssertEquals(array.Count, 3);
        Test.AssertEquals(array[1], 5);
        array.Insert(1, 42);
        Test.AssertEquals(array.Count, 4);
        Test.AssertEquals(array[1], 42);
    }

    public static void TestIndexOf()
    {
        IList<int> array = new Eina.Array<int>();
        Test.AssertEquals(array.Count, 0);
        array.Add(1);
        Test.AssertEquals(array.Count, 1);
        Test.AssertEquals(0, array.IndexOf(1));
        array.Insert(0, 0);
        Test.AssertEquals(array.Count, 2);
        Test.AssertEquals(0, array.IndexOf(0));
        Test.AssertEquals(1, array.IndexOf(1));
        array.Insert(0, 1);
        Test.AssertEquals(array.Count, 3);
        Test.AssertEquals(0, array.IndexOf(1));
        Test.AssertEquals(1, array.IndexOf(0));
        array.Insert(0, 1);
        Test.AssertEquals(array.Count, 4);
        Test.AssertEquals(0, array.IndexOf(1));
        Test.AssertEquals(2, array.IndexOf(0));
    }

    public static void TestCopyTo()
    {
        IList<int> array = new Eina.Array<int>();
        Test.AssertEquals(array.Count, 0);
        int[] random = {4, 40, 10, 42, 99};
        array.Add(43);
        array.Insert(0, 1);
        array.Insert(1, 50);
        Test.AssertEquals(array.Count, 3);
        Array.ForEach(random, (n) => array.Add(n));
        Test.AssertEquals(array.Count, 8);
        int[] expected = {1, 50, 43, 4, 40, 10, 42, 99};
        var result = new int[expected.Length];
        array.CopyTo(result, 0);
        for (int i = 0; i < expected.Length; ++i)
        {
            Test.AssertEquals(expected[i], result[i]);
        }
    }

    public static void TestIdxOutBounds()
    {
        var array = new Eina.Array<int>();
        array.Add(4);
        Test.AssertRaises<ArgumentOutOfRangeException>(()=>array.At(1));
        Test.AssertRaises<ArgumentOutOfRangeException>(()=>array.At(-1));
        Test.AssertNotRaises<ArgumentOutOfRangeException>
            (()=>array.At(array.IndexOf(4)));
    }

    public static void TestIsReadOnly()
    {
        var array = new Eina.Array<int>();
        int[] tmp = {1, 3, 2, 6 ,5};
        array.Append(tmp);
        Test.AssertEquals(array.Count, 5);
        array.SetOwnership(false);
        Test.AssertRaises<NotSupportedException>(() => array.Add(4));
        Test.AssertRaises<NotSupportedException>(() => array.Push(6));
        Test.AssertRaises<NotSupportedException>(() => array.Append(tmp));
        Test.AssertEquals(array.Count, 5);
        Test.AssertRaises<NotSupportedException>(() => array.DataSet(2, 4));
        Test.Assert(array.ToArray().SequenceEqual(tmp));
    }
}
}
