using static System.Console;

public class ExampleEinaHash01
{
    public static void Main()
    {
        Eina.Config.Init();

        var phone_book = new Eina.Hash<string, string>();

        // Add initial entries to our hash
        phone_book.Add("Wolfgang Amadeus Mozart", "+01 23 456-78910");
        phone_book.Add("Ludwig van Beethoven", "+12 34 567-89101");
        phone_book.Add("Richard Georg Strauss", "+23 45 678-91012");
        phone_book.Add("Heitor Villa-Lobos", "+34 56 789-10123");

        // Look for a specific entry and get its phone number
        var entry_name = "Heitor Villa-Lobos";
        var phone = phone_book.Find(entry_name);

        WriteLine("Printing entry.");
        WriteLine($"Name: {entry_name}");
        WriteLine($"Number: {phone}\n");

        // Delete this entry
        var r = phone_book.DelByKey(entry_name);
        WriteLine($"Hash entry successfully deleted? {r}!\n");

        // Modify the pointer data of an entry and free the old one
        phone_book.Modify("Richard Georg Strauss", "+23 45 111-11111");

        // Modify or add an entry to the hash
        // Let's first add a new entry
        entry_name = "Raul Seixas";
        phone_book[entry_name] = "+55 01 234-56789";
        WriteLine("Printing entry.");
        WriteLine($"Name: {entry_name}");
        WriteLine($"Number: {phone_book[entry_name]}\n");

        // Now change the phone number
        phone_book["Raul Seixas"] = "+55 02 222-22222";
        WriteLine("Printing entry.");
        WriteLine($"Name: {entry_name}");
        WriteLine($"Number: {phone_book[entry_name]}\n");

        // Check how many items are in the phone book
        WriteLine("There are {0} items in the hash.\n", phone_book.Count);

        // Change the name (key) on an entry
        phone_book.Move("Raul Seixas", "Alceu Valenca");
        entry_name = "Alceu Valenca";
        WriteLine("Printing entry.");
        WriteLine($"Name: {entry_name}");
        WriteLine($"Number: {phone_book[entry_name]}\n");

        // Empty the phone book, but don't destroy it
        phone_book.FreeBuckets();
        WriteLine("There are {0} items in the hash.\n", phone_book.Count);

        // Phone book could still be used, but we are freeing it since we are
        // done for now
        phone_book.Dispose();
    }
}


