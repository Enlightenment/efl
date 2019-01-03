using System;
using System.Runtime.InteropServices;
using System.Threading;
using static Efl.UnsafeNativeMethods;

static class UnsafeNativeMethods {
    [DllImport(efl.Libs.Ecore)] public static extern void ecore_init();
    [DllImport(efl.Libs.Ecore)] public static extern void ecore_shutdown();
    [DllImport(efl.Libs.Evas)] public static extern void evas_init();
    [DllImport(efl.Libs.Evas)] public static extern void evas_shutdown();
    [DllImport(efl.Libs.Elementary)] public static extern int elm_init(int argc, IntPtr argv);
    [DllImport(efl.Libs.Elementary)] public static extern void elm_policy_set(int policy, int policy_detail);
    [DllImport(efl.Libs.Elementary)] public static extern void elm_shutdown();
    [DllImport(efl.Libs.Elementary)] public static extern void elm_run();
    [DllImport(efl.Libs.Elementary)] public static extern void elm_exit();
}


namespace Efl {
  namespace Csharp {
    public enum Components {
        Basic,
        Ui
    }
    /// <summary>
    /// This represents the entry point for the EFL framework
    /// You can use this class to implement the 4 abstract methods which will then be called accordingly
    /// All subsystems of efl are booted up correctly when the abstract methods of this class are called.
    /// </summary>
    /// <remarks>
    /// Note: Calls to efl outside those efl-callbacks or outside the mainloop are not allowed and will lead to issues
    /// </remarks>
    /// <example>
    /// UserApp is the class that implements the Application abstract
    /// <code>
    /// public static void Main() {
    ///   UserApp editor = new UserApp();
    ///   editor.launch(editor);
    /// }
    /// </code>
    /// </example>
    public abstract class Application {
      //the initializied components
      private static Components initComponent;
      //what follows are 3 private functions to boot up the internals of efl
      private static void Init(Efl.Csharp.Components component) {
        Eina.Config.Init();
        Efl.Eo.Config.Init();
        ecore_init();
        evas_init();
        eldbus.Config.Init();

        if (component == Components.Ui) {
          // TODO Support elm command line arguments
#if WIN32 // Not a native define, we define it in our build system
          // Ecore_Win32 uses OleInitialize, which requires single thread apartments
          if (System.Threading.Thread.CurrentThread.GetApartmentState() != ApartmentState.STA)
              throw new InvalidOperationException("UI Applications require STAThreadAttribute in Main()");
#endif
          elm_init(0, IntPtr.Zero);

          elm_policy_set((int)Elm.Policy.Quit, (int)Elm.PolicyQuit.LastWindowHidden);
        }
        initComponent = component;
      }
      private static void Shutdown() {
        // Try to cleanup everything before actually shutting down.
        System.GC.Collect();
        System.GC.WaitForPendingFinalizers();

        if (initComponent == Components.Ui) {
          elm_shutdown();
        }
        eldbus.Config.Shutdown();
        evas_shutdown();
        ecore_shutdown();
        Efl.Eo.Config.Shutdown();
        Eina.Config.Shutdown();
      }
      private static void Run() {
        elm_run();
      }
      /// <summary>
      /// Called when the application is started, the inital arguments from the arguments are passed here.
      /// </summary>
      public abstract void args(Efl.LoopArguments args);
      /// <summary>
      /// Called when the application is not going be displayed, or is not used by a user for some time
      /// </summary>
      public abstract void pause();
      /// <summary>
      /// Called before a app is used again after a call to pause.
      /// </summary>
      public abstract void resume();
      /// <summary>
      /// Called before starting the shutdown of the application.
      /// </summary>
      public abstract void terminate();
      /// <summary>
      /// This function initializices everything in EFL and runs your application.
      /// This call will result in a call to args
      /// </summary>
      public void launch(string[] argv=null, Efl.Csharp.Components components=Components.Ui) {
        Init(components);
        Efl.App app = Efl.App.AppMain;
        app.ArgumentsEvt += (object sender, LoopArgumentsEvt_Args evt) => {
          args(evt.arg);
        };
        if (argv != null)
            // Does not seem to be being passed to the callback.
            foreach (var arg in argv)
                app.AppendArg(arg);
        app.PauseEvt += (object sender, EventArgs e) => {
          pause();
        };
        app.ResumeEvt += (object sender, EventArgs e) => {
          resume();
        };
        app.TerminateEvt += (object sender, EventArgs e) => {
          terminate();
        };
        Run();
        Shutdown();
      }
    }
    /// <summary>
    /// Use this class instead of Application if you don't want to use the pause,resume & terminate functions
    /// </summary>
    public abstract class SimpleApplication : Application {
      public override void pause() {

      }
      public override void resume() {

      }
      public override void terminate() {

      }
    }
  }
}


