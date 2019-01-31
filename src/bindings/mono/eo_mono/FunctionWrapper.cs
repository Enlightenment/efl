using System;
using System.Runtime.InteropServices;

namespace Efl { namespace Eo {

public partial class FunctionInterop
{
    public static IntPtr LoadFunctionPointer(string moduleName, string functionName)
    {
        NativeModule module = new NativeModule(moduleName);
        Console.WriteLine("searching {0} in library {1}", module.Module, functionName);
        var s = FunctionInterop.dlsym(module.Module, functionName);
        Console.WriteLine("searching {0} in library {1}, result {2}", module.Module, functionName, s);
        return s;
    }
    public static IntPtr LoadFunctionPointer(string functionName)
    {
        Console.WriteLine("searching {0} in library {1}", null, functionName);
        var s = FunctionInterop.dlsym(IntPtr.Zero, functionName);
        Console.WriteLine("searching {0} in library {1}, result {2}", null, functionName, s);
        return s;
    }
}
        
public class FunctionWrapper<T>
{
    private Lazy<FunctionLoadResult<T>> loadResult;
    private NativeModule module; // so it doesn't get unloaded

    private static FunctionLoadResult<T> LazyInitialization(NativeModule module, string functionName)
    {
        if (module.Module == IntPtr.Zero)
            return new FunctionLoadResult<T>(FunctionLoadResultKind.LibraryNotFound);
        else
        {
            IntPtr funcptr = FunctionInterop.LoadFunctionPointer(module.Module, functionName);
            if (funcptr == IntPtr.Zero)
                return new FunctionLoadResult<T>(FunctionLoadResultKind.FunctionNotFound);
            else
                return new FunctionLoadResult<T>(Marshal.GetDelegateForFunctionPointer<T>(funcptr));
        }
    }
    
    public FunctionWrapper(string moduleName, string functionName)
        : this (new NativeModule(moduleName), functionName)
    {
    }
    
    public FunctionWrapper(NativeModule module, string functionName)
    {
        this.module = module;
        loadResult = new Lazy<FunctionLoadResult<T>>
            (() =>
            {
                return LazyInitialization(module, functionName);
            });
    }

    public FunctionLoadResult<T> Value
    {
        get
        {
            return loadResult.Value;
        }
    }
}

public enum FunctionLoadResultKind { Success, LibraryNotFound, FunctionNotFound }
    
public class FunctionLoadResult<T>
{
    public FunctionLoadResultKind Kind;
    public T Delegate;

    public FunctionLoadResult(FunctionLoadResultKind kind)
    {
        this.Kind = kind;
    }
    public FunctionLoadResult(T Delegate)
    {
        this.Delegate = Delegate;
        this.Kind = FunctionLoadResultKind.Success;
    }
}


} }
