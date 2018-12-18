#define CODE_ANALYSIS

using System;
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
        Efl.UserModel<VeggieViewModel> veggies = new Efl.UserModel<VeggieViewModel>();
        veggies.Add (new VeggieViewModel{ Name="Tomato", Type="Fruit", Image="tomato.png"});
        veggies.Add (new VeggieViewModel{ Name="Romaine Lettuce", Type="Vegetable", Image="lettuce.png"});
        veggies.Add (new VeggieViewModel{ Name="Zucchini", Type="Vegetable", Image="zucchini.png"});

        

        Console.WriteLine ("end of test");
    }

    public static void easy_model_extraction ()
    {
        Efl.UserModel<VeggieViewModel> veggies = new Efl.UserModel<VeggieViewModel>();
        veggies.Add (new VeggieViewModel{ Name="Tomato", Type="Fruit", Image="tomato.png"});
        veggies.Add (new VeggieViewModel{ Name="Romaine Lettuce", Type="Vegetable", Image="lettuce.png"});
        veggies.Add (new VeggieViewModel{ Name="Zucchini", Type="Vegetable", Image="zucchini.png"});

        var model = new Efl.GenericModel<VeggieViewModel>(veggies);
        Console.WriteLine ("size model {0}", model.GetChildrenCount());

        

        Console.WriteLine ("end of test");
    }

    public static void factory_test ()
    {
        var factory = new Efl.Ui.ItemFactory<Efl.Object>();
        //factory.Foo();
        factory.Name().Bind("name");
    }
}

}
