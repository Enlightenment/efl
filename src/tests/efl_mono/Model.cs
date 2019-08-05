#define CODE_ANALYSIS

using System;
using System.Threading.Tasks;
using System.Diagnostics.CodeAnalysis;

#if EFL_BETA

namespace TestSuite {

[SuppressMessage("Gendarme.Rules.Portability", "DoNotHardcodePathsRule")]
public class TestModel {

    public class VeggieViewModel
    {
        public string Name { get; set; }
        public string Type { get; set; }
        public string Image { get; set; }
    }

    private static Efl.UserModel<VeggieViewModel> CreateModel(Efl.Loop loop)
    {
        Efl.UserModel<VeggieViewModel> veggies = new Efl.UserModel<VeggieViewModel>(loop);
        veggies.Add (new VeggieViewModel{ Name="Tomato", Type="Fruit", Image="tomato.png"});
        veggies.Add (new VeggieViewModel{ Name="Romaine Lettuce", Type="Vegetable", Image="lettuce.png"});
        veggies.Add (new VeggieViewModel{ Name="Zucchini", Type="Vegetable", Image="zucchini.png"});

        return veggies;
    }

    public static void reflection_test ()
    {
        Efl.Loop loop = Efl.App.AppMain;

        var veggies = CreateModel(loop);
    }

    internal static async Task EasyModelExtractionAsync (Efl.Loop loop)
    {
        var veggies = CreateModel(loop);

        var model = new Efl.GenericModel<VeggieViewModel>(veggies, loop);
        Test.AssertEquals(3, (int)model.GetChildrenCount());

        VeggieViewModel r2 = await model.GetAtAsync(1);
        Test.AssertEquals(r2.Name, "Romaine Lettuce");

        VeggieViewModel r = await model.GetAtAsync(0);
        Test.AssertEquals(r.Name, "Tomato");

        loop.End();
    }

    public static void easy_model_extraction ()
    {
        Efl.Loop loop = Efl.App.AppMain;
        Task task = EasyModelExtractionAsync(loop);

        loop.Begin();

        task.Wait();
    }

    public static void factory_test ()
    {
        string propertyBound = null;
        bool callbackCalled = false;
        var factory = new Efl.Ui.ItemFactory<Efl.Ui.Button>();
        factory.PropertyBoundEvt += (object sender, Efl.Ui.IPropertyBindPropertyBoundEvt_Args args) => {
            propertyBound = args.arg;
            callbackCalled = true;
        };

        factory.Style().Bind("first name");

        Test.Assert(callbackCalled, "Property bound callback must have been called.");
        Test.AssertEquals(propertyBound, "style");
    }
}

}

#endif
