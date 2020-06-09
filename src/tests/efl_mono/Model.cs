#define CODE_ANALYSIS

using System;
using System.Threading.Tasks;
using System.Diagnostics.CodeAnalysis;
using Efl.Ui;

#if EFL_BETA

namespace TestSuite {

[SuppressMessage("Gendarme.Rules.Portability", "DoNotHardcodePathsRule")]
public static class TestModel {

    private class VeggieViewModel
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

        var model = CreateModel(loop);
        model.Dispose();
    }

    internal static async Task EasyModelExtractionAsync (Efl.Loop loop)
    {
        var veggies = CreateModel(loop);

        var model = new Efl.GenericModel<VeggieViewModel>(veggies, loop);
        Test.AssertEquals(3, (int)model.ChildrenCount);

        VeggieViewModel r2 = await model.GetAtAsync(1).ConfigureAwait(false);
        Test.AssertEquals(r2.Name, "Romaine Lettuce");

        VeggieViewModel r = await model.GetAtAsync(0).ConfigureAwait(false);
        Test.AssertEquals(r.Name, "Tomato");

        loop.End();
        model.Dispose();
        veggies.Dispose();
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
        var parent = new Efl.Ui.Win(null, "", "");
        parent.Visible = false;
        var factory = new Efl.Ui.ItemFactory<Efl.Ui.Button>(parent);
        factory.PropertyBoundEvent += (object sender, Efl.Ui.PropertyBindPropertyBoundEventArgs args) => {
            propertyBound = args.Arg;
            callbackCalled = true;
        };

        factory.Style().Bind("first name");

        Test.Assert(callbackCalled, "Property bound callback must have been called.");
        Test.AssertEquals(propertyBound, "style");
        factory.Dispose();
        parent.Dispose();
    }
}

}

#endif
