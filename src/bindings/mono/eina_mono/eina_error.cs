using System;
using System.Runtime.InteropServices;

namespace eina {

public struct Error
{
    int code;

    public string Message
    {
        get { return MsgGet(this); }
    }

    public static Error EFL_ERROR;

    public static Error NO_ERROR = new Error(0);
    public static Error EPERM = new Error(1);
    public static Error ENOENT = new Error(2);

    public Error(int value) { code = value; }
    static public implicit operator Error(int val)
    {
        return new Error(val);
    }
    static public implicit operator int(Error error)
    {
        return error.code;
    }

    public static void Init()
    {
        if (eina_init() == 0)
            throw (new efl.EflException("Failed to init Eina"));

        EFL_ERROR = eina_error_msg_register("Managed Code Error");
    }

    [DllImport("eina")] private static extern int eina_init();
    [DllImport("eina")] static extern Error eina_error_msg_register(string msg);
    [DllImport("eina")] static extern Error eina_error_get();
    [DllImport("eina")] static extern void eina_error_set(Error error);
    [DllImport("eina")] static extern IntPtr eina_error_msg_get(Error error);

    public static void Set(Error error)
    {
        eina_error_set(error);
    }

    public static Error Get()
    {
        return eina_error_get();
    }

    public static String MsgGet(Error error)
    {
        IntPtr cstr = eina_error_msg_get(error);
        return Marshal.PtrToStringAuto(cstr);
    }

    public static void RaiseIfOcurred()
    {
        Error e = Get();
        Clear();
        Raise(e);
    }

    public static void Raise(Error e)
    {
        if (e != 0)
            throw (new efl.EflException(MsgGet(e)));
    }

    public static void Clear()
    {
        Set(0);
    }

    public static Error Register(string msg)
    {
        return eina_error_msg_register(msg);
    }
}
}
