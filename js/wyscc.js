const fs = require("fs");
const { execSync } = require("child_process");
const https = require("https");
const { exec } = require("child_process");

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
            console.log(`[w] No source files for task '${this.name}', skipping...`);
            return false;
        }

        const includeFlags = this.includes.map(inc => `-I${inc}`).join(" ");
        const files = this.files.join(" ");
        const args = this.args.join(" ");
        const command = `${this.compiler} ${files} ${includeFlags} -std=${this.std} -o ${this.output} ${args}`;

        console.log(`[i] Compiling task '${this.name}' with command: ${command}`);
        try {
            execSync(command, { stdio: "inherit" });
            console.log(`[s] Compilation successful for task '${this.name}'`);
            return true;
        } catch (error) {
            console.error(`[e] Compilation failed for task '${this.name}'`);
            return false;
        }
    }
}

class Wysc {
    constructor(configPath = "wysc.json") {
        this.tasks = [];
        this.dependencies = [];
        this.loadConfig(configPath);
    }

    loadConfig(configPath) {
        if (!fs.existsSync(configPath)) {
            console.error(`[e] Configuration file '${configPath}' not found!`);
            return;
        }

        try {
            const config = JSON.parse(fs.readFileSync(configPath, "utf-8"));
            for (const [key, value] of Object.entries(config)) {
                if (key === "install") {
                    this.dependencies = value;
                } else {
                    this.tasks.push(new Task(key, value));
                }
            }
        } catch (error) {
            console.error(`[e] Failed to parse '${configPath}': ${error.message}`);
        }
    }

    installDependencies() {
        console.log("[i] Installing dependencies...");
        if (this.dependencies.length === 0) {
            console.log("[i] No dependencies to install.");
            return;
        }

        this.dependencies.forEach(dep => {
            console.log(`[i] Cloning repository ${dep}`);
            exec(`git clone ${dep}`, (err) => {
                if (err) console.error(`[e] Failed to clone ${dep}: ${err.message}`);
                else console.log(`[s] Cloned ${dep}`);
            });
        });
    }

    downloadFile(url, dest) {
        console.log(`[i] Downloading ${url} to ${dest}`);
        const file = fs.createWriteStream(dest);
        https.get(url, response => {
            response.pipe(file);
            file.on("finish", () => {
                file.close();
                console.log(`[s] Downloaded ${url}`);
            });
        }).on("error", error => {
            fs.unlink(dest, () => {});
            console.error(`[e] Failed to download ${url}: ${error.message}`);
        });
    }

    run() {
        let successCount = 0;
        let failureCount = 0;
        const startTime = Date.now();
        
        for (const task of this.tasks) {
            if (task.compile()) {
                successCount++;
            } else {
                failureCount++;
            }
        }
        
        const totalTime = ((Date.now() - startTime) / 1000).toFixed(2);
        console.log(`\n[i] Summary: Compilation completed in ${totalTime} seconds`);
        console.log(`[i] Successful compilations: ${successCount}`);
        console.log(`[i] Failed compilations: ${failureCount}`);
    }

    clean() {
        console.log("[i] Cleaning build artifacts...");
        for (const task of this.tasks) {
            if (fs.existsSync(task.output)) {
                fs.unlinkSync(task.output);
                console.log(`[i] Removed ${task.output}`);
            }
        }
    }
}

if (require.main === module) {
    console.log('--- Wys C++ Compiler ---');
    const action = process.argv[2] || "build";
    const wysc = new Wysc();

    switch (action) {
        case "build":
            wysc.run();
            break;
        case "clean":
            wysc.clean();
            break;
        case "install":
            wysc.installDependencies();
            break;
        default:
            console.log("Usage: node wysc.js [build|clean|install]");
    }
}
