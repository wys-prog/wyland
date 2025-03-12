using Godot;
using System;

namespace Wyland.VMHandleNode;

public partial class WylandVmHandle : Node
{
	public int Invoke(string path, string[] args) 
	{
		OS.Execute(path, args);
		return 0;
	}
}
