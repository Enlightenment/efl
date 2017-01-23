using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Reflection;
using System.Linq;

public class Test
{
    public static void Assert(bool res, String msg = "Assertion failed")
    {
        if (!res)
            throw new Exception(msg);
    }
}

public class MyBox : evas.BoxInherit
{
    public MyBox(efl.Object parent) : base(parent) {}

    [DllImport("evas")] static extern void evas_obj_box_layout_vertical(IntPtr obj, IntPtr data, IntPtr privdata);
    [DllImport("evas")] static extern void evas_obj_box_layout_horizontal(IntPtr obj, IntPtr data, IntPtr privdata);
    [DllImport("evas")] static extern void evas_object_box_layout_horizontal(IntPtr obj, IntPtr data, IntPtr privdata);
    [DllImport("evas")] static extern IntPtr evas_object_evas_get(IntPtr obj);
    [DllImport("evas")] static extern void evas_event_freeze(IntPtr obj);
    [DllImport("evas")] static extern void evas_event_thaw(IntPtr obj);

    override public void group_calculate()
    {
        IntPtr evas = evas_object_evas_get(raw_handle);
        evas_event_freeze(evas);
        Console.WriteLine("called group_calculate");
        layouting_set(true);
        evas_obj_box_layout_vertical(raw_handle, IntPtr.Zero, IntPtr.Zero);
        layouting_set(false);
        children_changed_set(false);
        evas_event_thaw(evas);
    }
}

namespace TestSuite {

class Core
{
    public static void return_same_object()
    {
        test.Testing testing = new test.TestingConcrete();
        test.Testing o1 = testing.return_object();
        Test.Assert(o1.raw_handle != IntPtr.Zero);
        Test.Assert(o1.raw_handle == testing.raw_handle);
        test.Testing o2 = o1.return_object();
        Test.Assert(o2.raw_handle != IntPtr.Zero);
        Test.Assert(o2.raw_handle == o1.raw_handle);
    }

    public static void destructor_really_frees()
    {
       bool delEventCalled = false;
       {
           test.Testing obj = new test.TestingConcrete();
           obj.DEL += (object sender, EventArgs e) => { delEventCalled = true; };
       }

       System.GC.WaitForPendingFinalizers();
       System.GC.Collect();
       System.GC.WaitForPendingFinalizers();
       System.GC.Collect();
       System.GC.WaitForPendingFinalizers();

       Test.Assert(delEventCalled, "Destructor not called");
    }
}

class MyLoop : efl.LoopInherit
{
    public MyLoop() : base(null) { }
}

class EoInherit
{
    public static void instantiate_inherited()
    {
        efl.Loop loop = new MyLoop();
        Test.Assert(loop.raw_handle != System.IntPtr.Zero);

        // Evas
        EcoreEvas ecore_evas = new EcoreEvas();
        efl.canvas.Object canvas = ecore_evas.canvas;
        evas.Box box = new MyBox(canvas);
        Test.Assert(box.raw_handle != System.IntPtr.Zero);
    }
}

class EoEvents
{
    public bool called = false;
    protected void callback(object sender, EventArgs e) {
        called = true;
    }
    protected void another_callback(object sender, EventArgs e) { }

    public static void callback_add_event()
    {
        efl.Loop loop = new efl.LoopConcrete();
        EoEvents listener = new EoEvents();
        loop.CALLBACK_ADD += listener.callback;

        Test.Assert(!listener.called);
        loop.IDLE += listener.another_callback;
        Test.Assert(listener.called);
    }
}

class Evas
{
    private static string ImagePath([CallerFilePath] string folder="")
    {
        return System.IO.Path.GetDirectoryName(folder);
    }

    public static void simple_layout()
    {
//         efl.Loop loop = new efl.LoopConcrete();

        EcoreEvas ecore_evas = new EcoreEvas();

        System.Console.WriteLine("Hello World");

        efl.canvas.Object canvas = ecore_evas.canvas;
        canvas.visible_set(true);

        efl.Object parent = canvas.parent_get();
        Test.Assert(parent.raw_handle != IntPtr.Zero);

        efl.canvas.Rectangle rect = new efl.canvas.RectangleConcrete(canvas);
        rect.color_set(255, 255, 255, 255);
        rect.size_set(640, 480);
        rect.visible_set(true);

        evas.Box box = new MyBox(canvas);
        rect.size_set(320, 240);
        box.visible_set(true);

        efl.canvas.Image image1 = new efl.canvas.ImageConcrete(canvas);
        image1.file_set(ImagePath() + "/../../examples/elementary/sphere_hunter/score.jpg", "");
        image1.hint_min_set(160, 240);
        image1.visible_set(true);

        efl.canvas.Image image2 = new efl.canvas.ImageConcrete(canvas);
        image2.file_set(ImagePath() + "/../../examples/evas/shooter/assets/images/bricks.jpg", "");
        image2.hint_min_set(160, 120);
        image2.visible_set(true);

        box.append(image1);
        box.append(image2);

//         loop.begin();
    }
}

}

class TestMain
{
    [DllImport("eo")] static extern void efl_object_init();
    [DllImport("ecore")] static extern void ecore_init();
    [DllImport("evas")] static extern void evas_init();

    static Type[] GetTestSuites()
    {
        return Assembly.GetExecutingAssembly().GetTypes().Where(t => String.Equals(t.Namespace, "TestSuite", StringComparison.Ordinal)).ToArray();
    }

    static int Main(string[] args)
    {
        efl_object_init();
        ecore_init();
        evas_init();

        bool pass = true;

        var suites = GetTestSuites();
        foreach(var suite in suites)
        {
            var testCases = suite.GetMethods(BindingFlags.Static | BindingFlags.Public);
            Console.WriteLine("[ START SUITE ] " + suite.Name);
            foreach(var testCase in testCases)
            {
                Console.WriteLine("[ RUN         ] " + suite.Name + "." + testCase.Name);
                bool caseResult = true;
                try
                {
                    testCase.Invoke(null, null);
                }
                catch (Exception e)
                {
                    pass = false;
                    caseResult = false;
                    Console.WriteLine("[ ERROR       ] " + e.InnerException.Message);
                }
                Console.WriteLine("[        " + (caseResult ? "PASS" : "FAIL") + " ] " + suite.Name + "." + testCase.Name);
            }
            Console.WriteLine("[ END SUITE ] " + suite.Name);
        }

        if (!pass)
          return -1;

        return 0;
    }
}
