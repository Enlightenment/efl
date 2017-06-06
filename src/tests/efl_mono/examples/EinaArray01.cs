using static System.Console;

namespace TestSuite
{

class TestExampleEinaArray01
{
    public static void EinaArray01()
    {
        var strings = new string[]{
            "helo", "hera", "starbuck", "kat", "boomer",
            "hotdog", "longshot", "jammer", "crashdown", "hardball",
            "duck", "racetrack", "apolo", "husker", "freaker",
            "skulls", "bulldog", "flat top", "hammerhead", "gonzo"
        };

        var array = new eina.Array<string>(20U);

        for (int i = 0; i < 20; i++)
        {
            WriteLine("push: " + strings[i]);
            array.Push(strings[i]);
        }

        WriteLine("array count: " + array.Count());

        int idx = 0;
        foreach (string s in array)
        {
            WriteLine("at[" + idx + "]: " + s);
            ++idx;
        }

        while (array.Length != 0)
            WriteLine("pop: " + array.Pop());

        array.Dispose();
    }
}

}
