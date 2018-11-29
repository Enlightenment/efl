using static System.Console;

public class ExampleEinaArray01
{
    public static void Main()
    {
        Eina.Config.Init();

        var strings = new string[]{
            "helo", "hera", "starbuck", "kat", "boomer",
            "hotdog", "longshot", "jammer", "crashdown", "hardball",
            "duck", "racetrack", "apolo", "husker", "freaker",
            "skulls", "bulldog", "flat top", "hammerhead", "gonzo"
        };

        var array = new Eina.Array<string>(20U);

        // Push new elements
        foreach (string s in strings)
        {
            WriteLine("push: " + s);
            array.Push(s);
        }

        // Check count
        WriteLine("array count: " + array.Count());

        // Iterate over the array
        int idx = 0;
        foreach (string s in array)
        {
            WriteLine($"at[{idx}]: {s}");
            ++idx;
        }

        // Remove one by one
        while (array.Length != 0)
            WriteLine("pop: " + array.Pop());
    }
}

