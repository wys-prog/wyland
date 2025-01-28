using System;
using System.Runtime.InteropServices;
using Godot;

namespace Wyland.Source.Kernel.VM;

public partial class Kokuyo : Node 
{
  [DllImport("Source/Kernel/vm/bin/kokuyo-wyland.so", CallingConvention = CallingConvention.Cdecl)]
  private static extern IntPtr kokuyo_create();

  [DllImport("Source/Kernel/vm/bin/kokuyo-wyland.so", CallingConvention = CallingConvention.Cdecl)]
  private static extern void kokuyo_destroy(IntPtr ptr);

  [DllImport("Source/Kernel/vm/bin/kokuyo-wyland.so", CallingConvention = CallingConvention.Cdecl)]
  private static extern void kokuyo_invoke(IntPtr ptr, byte[] disk);


  public class KokuyoInstance 
  {
    public IntPtr Pointer;
  }

}