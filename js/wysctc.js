const fs = require("fs");
const { execSync } = require("child_process");

class Task {
    constructor(name, config) {
        this.name = name;
        this.compiler = config.compiler || "gcc";
        this.files = config.files || [];
        this.includes = config.includes || [];
        this.std = config.std || "c++17";
        this.output = config.output || "a.out";
        this.args = config.args || [];
    }

    compile() {
        if (this.files.length === 0) {
            console.log(`[WARNING] No source files for task '${this.name}', skipping...`);
            return;
        }

        const includeFlags = this.includes.map(inc => `-I${inc}`).join(" ");
        const files = this.files.join(" ");
        const args = this.args.join(" ");
        const command = `${this.compiler} ${files} ${includeFlags} -std=${this.std} -o ${this.output} ${args}`;

        console.log(`[INFO] Compiling task '${this.name}' with command: ${command}`);
        try {
            execSync(command, { stdio: "inherit" });
        } catch (error) {
            console.error(`[ERROR] Compilation failed for task '${this.name}'`);
        }
    }
}

class Wysc {
    constructor(configPath = "wysc.json") {
        this.tasks = [];
        this.loadConfig(configPath);
    }

    loadConfig(configPath) {
        if (!fs.existsSync(configPath)) {
            console.error(`[ERROR] Configuration file '${configPath}' not found!`);
            return;
        }

        try {
            const config = JSON.parse(fs.readFileSync(configPath, "utf-8"));
            for (const [taskName, taskConfig] of Object.entries(config)) {
                this.tasks.push(new Task(taskName, taskConfig));
            }
        } catch (error) {
            console.error(`[ERROR] Failed to parse '${configPath}': ${error.message}`);
        }
    }

    run() {
        for (const task of this.tasks) {
            task.compile();
        }
    }
}

if (require.main === module) {
    new Wysc().run();
}
