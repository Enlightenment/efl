#define CODE_ANALYSIS

using System;
using System.Threading.Tasks;
using System.Diagnostics.CodeAnalysis;

namespace TestSuite {

[SuppressMessage("Gendarme.Rules.Portability", "DoNotHardcodePathsRule")]
public static class TestModel {

    public class VeggieViewModel
    {
        public string Name { get; set; }
        public string Type { get; set; }
        public string Image { get; set; }        
    }
    
    public static void reflection_test ()
    {
        Efl.Loop loop = Efl.App.AppMain;

        Efl.UserModel<VeggieViewModel> veggies = new Efl.UserModel<VeggieViewModel>(loop);
        veggies.Add (new VeggieViewModel{ Name="Tomato", Type="Fruit", Image="tomato.png"});
        veggies.Add (new VeggieViewModel{ Name="Romaine Lettuce", Type="Vegetable", Image="lettuce.png"});
        veggies.Add (new VeggieViewModel{ Name="Zucchini", Type="Vegetable", Image="zucchini.png"});

        Console.WriteLine ("end of test");
    }

    internal static async Task EasyModelExtractionAsync (Efl.Loop loop)
    {
        Efl.UserModel<VeggieViewModel> veggies = new Efl.UserModel<VeggieViewModel>(loop);
        veggies.Add (new VeggieViewModel{ Name="Tomato", Type="Fruit", Image="tomato.png"});
        veggies.Add (new VeggieViewModel{ Name="Romaine Lettuce", Type="Vegetable", Image="lettuce.png"});
        veggies.Add (new VeggieViewModel{ Name="Zucchini", Type="Vegetable", Image="zucchini.png"});

        var model = new Efl.GenericModel<VeggieViewModel>(veggies, loop);
        Console.WriteLine ("size model {0}", model.GetChildrenCount());

        VeggieViewModel r = await model.GetAtAsync(0);

        Test.AssertEquals(r.Name, "Tomato");
        VeggieViewModel r2 = await model.GetAtAsync(1);
        Test.AssertEquals(r2.Name, "Romaine Lettuce");

        Console.WriteLine ("end of test, Name of result is {0}", r.Name);
        loop.End();
    }

    public static void easy_model_extraction ()
    {
        Efl.Loop loop = Efl.App.AppMain;
        
        Task task = EasyModelExtractionAsync(loop);

        loop.Begin();
    }

    public static void factory_test ()
    {
        var factory = new Efl.Ui.ItemFactory<Efl.Object>();
        //factory.Foo();
        factory.Name().Bind("first name");
    }
}

}
