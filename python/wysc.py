import os
import json
import subprocess
from pathlib import Path

class Task:
    def __init__(self, name, config):
        self.name = name
        self.compiler = config.get("compiler", "gcc")
        self.files = config.get("files", [])
        self.includes = config.get("includes", [])
        self.std = config.get("std", "c++17")
        self.output = config.get("output", "a.out")
        self.platform = config.get("platform", "all")
        
    def compile(self):
        if not self.files:
            print(f"[WARNING] No source files for task '{self.name}', skipping...")
            return
        
        include_flags = " ".join(f"-I{inc}" for inc in self.includes)
        files = " ".join(self.files)
        
        command = f"{self.compiler} {files} {include_flags} -std={self.std} -o {self.output}"
        print(f"[INFO] Compiling task '{self.name}' with command: {command}")
        subprocess.run(command, shell=True, check=True)

class Wysc:
    def __init__(self, config_path="wysc.json"):
        self.tasks = []
        self.load_config(config_path)

    def load_config(self, config_path):
        if not Path(config_path).exists():
            print(f"[ERROR] Configuration file '{config_path}' not found!")
            return
        
        with open(config_path, "r") as cfgfile:
            try:
                config = json.load(cfgfile)
                for task_name, task_config in config.items():
                    self.tasks.append(Task(task_name, task_config))
            except json.JSONDecodeError as e:
                print(f"[ERROR] Failed to parse '{config_path}': {e}")

    def run(self):
        for task in self.tasks:
            task.compile()

if __name__ == "__main__":
    wysc = Wysc()
    wysc.run()
