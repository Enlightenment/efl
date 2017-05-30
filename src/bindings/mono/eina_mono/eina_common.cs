using System;
using System.Runtime.InteropServices;

namespace eina
{
namespace Callbacks
{

public delegate int Eina_Compare_Cb(IntPtr data1, IntPtr data2);
public delegate void Eina_Free_Cb(IntPtr data);

}
}
