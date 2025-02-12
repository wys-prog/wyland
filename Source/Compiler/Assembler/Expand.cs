using Godot;
using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;

namespace Wyland.Source.Compiler.Assembler;

public partial class MacroProcessor : Node
{
  public static Dictionary<string, (List<string> Params, string Body)> ParseMacros(string[] lines)
  {
    var macros = new Dictionary<string, (List<string>, string)>();
    var pattern = new Regex(@"#define\s+(\w+)\((.*?)\)\s+(.+)");

    foreach (var line in lines)
    {
      var match = pattern.Match(line);
      if (match.Success)
      {
        string name = match.Groups[1].Value;
        List<string> paramList = new List<string>(match.Groups[2].Value.Split(new[] { ',' }, StringSplitOptions.RemoveEmptyEntries));
        string body = match.Groups[3].Value;

        for (int i = 0; i < paramList.Count; i++)
        {
          paramList[i] = paramList[i].Trim();
        }

        macros[name] = (paramList, body);
      }
    }
    return macros;
  }

  public static string ExpandMacros(string content, Dictionary<string, (List<string> Params, string Body)> macros)
  {
    string MacroReplacement(Match match)
    {
      string name = match.Groups[1].Value;
      if (!macros.ContainsKey(name))
        return match.Value;

      string[] args = match.Groups[2].Value.Split(new[] { ',' }, StringSplitOptions.RemoveEmptyEntries);
      var (paramList, body) = macros[name];

      if (args.Length != paramList.Count)
        return match.Value;

      string expandedBody = body;
      for (int i = 0; i < paramList.Count; i++)
      {
        expandedBody = expandedBody.Replace(paramList[i], args[i].Trim());
      }

      return ExpandMacros(expandedBody, macros);
    }

    var macroCallPattern = new Regex(@"(\w+)\(([^)]*)\)");
    return macroCallPattern.Replace(content, MacroReplacement);
  }
}
