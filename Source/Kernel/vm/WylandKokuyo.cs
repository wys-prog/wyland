using System;
using System.Runtime.InteropServices;
using Godot;

namespace Wyland.Source.Kernel.VM;

public partial class Kokuyo : Node 
{
  [DllImport("Source/Kernel/vm/bin/kokuyo-wyland.so", CallingConvention = CallingConvention.Cdecl)]
  private static extern IntPtr KokuyoCreate();

  [DllImport("Source/Kernel/vm/bin/kokuyo-wyland.so", CallingConvention = CallingConvention.Cdecl)]
  private static extern void KokuyoDestroy(IntPtr ptr);

  [DllImport("Source/Kernel/vm/bin/kokuyo-wyland.so", CallingConvention = CallingConvention.Cdecl)]
  private static extern void KokuyoInvoke(IntPtr ptr, byte[] disk);


  public class KokuyoInstance 
  {
    public IntPtr Pointer;
  }

}