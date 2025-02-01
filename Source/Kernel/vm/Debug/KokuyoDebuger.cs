using Godot;
using System;

namespace Wyland.Source.Kernel.VM.Debug;

public partial class KokuyoDebuger : Control
{
	private TabContainer OutputsTab;
	private TabContainer StakcsTab;
	private TabContainer RegistersTab;

	private TextEdit DebugText;
	private TextEdit OutputText;

	private TextEdit StackText;
	private TextEdit CallStackText;

	private TextEdit RegistersText;

	private Label MemoryUsageLabel;

	private readonly Dobuyo VM = new();

	private void WriteRegisters()
	{
		string[] regs = VM.GetRegisters();
		int i = 0;

		RegistersText.Text = "";

		foreach (var item in regs)
		{
			if (ulong.TryParse(item, out ulong value))
				RegistersText.Text += $"R{i}: 0x{value:X}\n";
			else
				RegistersText.Text += $"R{i}: Invalid\n";

			i++;
		}
	}

	private void WriteStack()
	{
		string[] stack = VM.GetStack();

		StackText.Text = "";

		foreach (var item in stack)
		{
			if (ulong.TryParse(item, out ulong value))
				RegistersText.Text += $"0x{value:X}\n";
			else
				RegistersText.Text += $"Invalid\n";
		}
	}

	private void WriteCallStack()
	{
		string[] callStack = VM.GetCallStack();

		StackText.Text = "";

		foreach (var item in callStack)
		{
			if (ulong.TryParse(item, out ulong value))
				RegistersText.Text += $"0x{value:X}\n";
			else
				RegistersText.Text += $"Invalid\n";
		}
	}

	private void WriteLogs()
	{
		DebugText.Text = VM.GetLogs() + '\n';
	}

	private void WriteMemory()
	{
		MemoryUsageLabel.Text = $"Used Memory: {VM.GetMemoryUse()}B | Physical: {VM.GetPhysicalMemory()}B | IP: {VM.GetIP()}";
	}

	public override void _Process(double delta)
	{
		if (!VM.IsHalt())
		{
			VM.Step();
			WriteCallStack();
			WriteStack();
			WriteLogs();
			WriteMemory();
		}
	}

	public override void _Ready()
	{
		OutputsTab = GetNode<TabContainer>("TabContainer");
		StakcsTab = GetNode<TabContainer>("TabContainer2");
		RegistersTab = GetNode<TabContainer>("TabContainer3");

		DebugText = GetNode<TextEdit>("TabContainer/Debug");
		OutputText = GetNode<TextEdit>("TabContainer/Output");
		StackText = GetNode<TextEdit>("TabContainer2/Stack");
		CallStackText = GetNode<TextEdit>("TabContainer2/CallStack");
		RegistersText = GetNode<TextEdit>("TabContainer3/Registers");

		MemoryUsageLabel = GetNode<Label>("MemoryUsage");

		VM.Create();
		VM.Invoke("./README.md");
	}
}
