using static System.Console;

public class ExampleEinaBinbuf01
{
    public static void Main()
    {
        Eina.Config.Init();

        var bytes = new byte[]{0, 1, 2, 3, 4, 5, 6};

        var bb = new Eina.Binbuf();

        // Append initial bytes
        bb.Append(bytes);

        WriteLine("Printing each byte.");

        int idx = 0;
        foreach (byte b in bb.GetBytes())
        {
            WriteLine($"byte at[{idx}]: {b}");
            ++idx;
        }

        // Append more byte by byte
        bb.Append((byte) 7);
        bb.Append((byte) 8);
        bb.Append((byte) 9);
        bb.Append((byte) 0);

        WriteLine("\nPrinting each byte.");

        idx = 0;
        foreach (byte b in bb.GetBytes())
        {
            WriteLine($"byte at[{idx}]: {b}");
            ++idx;
        }

        // Remove some
        bb.Remove(2, 5);

        WriteLine("\nPrinting each byte.");

        idx = 0;
        foreach (byte b in bb.GetBytes())
        {
            WriteLine($"byte at[{idx}]: {b}");
            ++idx;
        }

        // Insert new bytes in the middle
        bb.Insert(new byte[]{22, 33, 44}, 2);

        WriteLine("\nPrinting each byte.");

        idx = 0;
        foreach (byte b in bb.GetBytes())
        {
            WriteLine($"byte at[{idx}]: {b}");
            ++idx;
        }
    }
}

