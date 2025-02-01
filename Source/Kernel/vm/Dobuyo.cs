using Godot;
using System;
using System.Text;
using System.Runtime.InteropServices;

namespace Wyland.Source.Kernel.VM;

public partial class Dobuyo
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

  [DllImport("Source/Kernel/vm/bin/dobuyo.so", CallingConvention = CallingConvention.Cdecl)]
  private static extern ulong dobuyo_get_physical_memory(IntPtr ptr);

  [DllImport("Source/Kernel/vm/bin/dobuyo.so", CallingConvention = CallingConvention.Cdecl)]
  private static extern byte[] dobuyo_get_buff(IntPtr ptr);

  private IntPtr DobuyoPointer;

  public void Step() 
  {
    dobuyo_step(DobuyoPointer);
  }

  public void End()
  {
    dobuyo_end(DobuyoPointer);
  }

  public bool IsHalt() 
  {
    return dobuyo_is_halted(DobuyoPointer);
  }

  public string[] GetStack()
  {
    return Encoding.UTF8.GetString(dobuyo_get_stack(DobuyoPointer)).Split('\n');
  }

  public string[] GetCallStack() 
  {
    return Encoding.UTF8.GetString(dobuyo_get_callstack(DobuyoPointer)).Split('\n');
  }

  public string[] GetRegisters()
  {
    return Encoding.UTF8.GetString(dobuyo_get_registers(DobuyoPointer)).Split('\n');
  }

  public string GetLogs()
  {
    return Encoding.UTF8.GetString(dobuyo_get_buff(DobuyoPointer));
  }

  public ulong GetMemoryUse()
  {
    return dobuyo_get_memory(DobuyoPointer);
  }

  public ulong GetIP()
  {
    return dobuyo_get_ip(DobuyoPointer);
  }

  public ulong GetPhysicalMemory()
  {
    return dobuyo_get_physical_memory(DobuyoPointer);
  }

  public void Create()
  {
    DobuyoPointer = dobuyo_create();
  }

  public void Invoke(string path)
  {
    dobuyo_invoke(DobuyoPointer, Encoding.UTF8.GetBytes(path));
  }
}