using Godot;
using System;
using System.Runtime.InteropServices;

namespace Wyland.Source.Kernel.VM;

public partial class Dobuyo : Node
{
  [DllImport("Source/Kernel/vm/bin/dobuyo.so", CallingConvention = CallingConvention.Cdecl)]
  private static extern IntPtr dobuyo_create();

  [DllImport("Source/Kernel/vm/bin/dobuyo.so", CallingConvention = CallingConvention.Cdecl)]
  private static extern void dobuyo_destroy(IntPtr ptr);

  [DllImport("Source/Kernel/vm/bin/dobuyo.so", CallingConvention = CallingConvention.Cdecl)]
  private static extern void dobuyo_invoke(IntPtr ptr, byte[] disk);
  
  [DllImport("Source/Kernel/vm/bin/dobuyo.so", CallingConvention = CallingConvention.Cdecl)]
  private static extern ulong dobuyo_get_ip(IntPtr ptr);
  
  [DllImport("Source/Kernel/vm/bin/dobuyo.so", CallingConvention = CallingConvention.Cdecl)]
  private static extern ulong dobuyo_get_memory(IntPtr ptr);

  [DllImport("Source/Kernel/vm/bin/dobuyo.so", CallingConvention = CallingConvention.Cdecl)]
  private static extern byte[] dobuyo_get_registers(IntPtr ptr);
  
  [DllImport("Source/Kernel/vm/bin/dobuyo.so", CallingConvention = CallingConvention.Cdecl)]
  private static extern byte[] dobuyo_get_callstack(IntPtr ptr);

  [DllImport("Source/Kernel/vm/bin/dobuyo.so", CallingConvention = CallingConvention.Cdecl)]
  private static extern byte[] dobuyo_get_stack(IntPtr ptr);

  [DllImport("Source/Kernel/vm/bin/dobuyo.so", CallingConvention = CallingConvention.Cdecl)]
  private static extern bool dobuyo_is_halted(IntPtr ptr);

  [DllImport("Source/Kernel/vm/bin/dobuyo.so", CallingConvention = CallingConvention.Cdecl)]
  private static extern void dobuyo_step(IntPtr ptr);

  [DllImport("Source/Kernel/vm/bin/dobuyo.so", CallingConvention = CallingConvention.Cdecl)]
  private static extern void dobuyo_end(IntPtr ptr);

  
}