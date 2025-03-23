const fs = require('fs');
const path = require('path');

// Function to get the size of a file
function getFileSize(filePath) {
  try {
    const stats = fs.statSync(filePath);
    return stats.size;
  } catch (err) {
    console.error(`Error getting file size for ${filePath}:`, err);
    return null;
  }
}

// Function to get all files starting with a specific prefix in a directory and its subdirectories
function getAllFilesWithPrefix(dir, prefix) {
  let results = [];

  function readDirRecursive(currentDir) {
    const items = fs.readdirSync(currentDir);

    items.forEach(item => {
      const fullPath = path.join(currentDir, item);
      const stat = fs.statSync(fullPath);

      if (stat.isDirectory()) {
        readDirRecursive(fullPath);
      } else if (stat.isFile() && path.basename(fullPath).startsWith(prefix)) {
        results.push(fullPath);
        console.info('New file found: ', fullPath);
      }
    });
  }

  readDirRecursive(dir);
  return results;
}

// Function to parse command line arguments
function parseArgs() {
  const args = process.argv.slice(2);
  const options = {
    version: 1,
    target: 'wtarg64'
  };

  for (let i = 0; i < args.length; i++) {
    if (args[i] === '-version' && i + 1 < args.length) {
      options.version = parseInt(args[i + 1], 10);
      i++;
    } else if (args[i] === '-target' && i + 1 < args.length) {
      options.target = args[i + 1];
      i++;
    }
  }

  return options;
}

// Main function
function main() {
  const options = parseArgs();

  const targets = {
    wtarg64: 2660,
    wtarg32: 2610,
    wtargmarch: 436,
    wtargfast: 2810
  };

  let targetValue = targets[options.target];
  if (targetValue === undefined) {
    console.error(`Invalid target: ${options.target}`);
    return;
  }

  let codeSections = getAllFilesWithPrefix('./', 'wyland.section.code');
  let dataSections = getAllFilesWithPrefix('./', 'wyland.section.data');
  let libsSections = getAllFilesWithPrefix('./', 'wyland.section.libs'); 

  let codeSectionSize = 0;
  let dataSectionSize = 0;
  let libsSectionSize = 0;

  codeSections.forEach(file => codeSectionSize += getFileSize(file));
  dataSections.forEach(file => dataSectionSize += getFileSize(file));
  libsSections.forEach(file => libsSectionSize += getFileSize(file));

  console.info('--- Detection of file ended ---');
  console.info(`Section: code: ${codeSectionSize} (${codeSections.length} elements)`);
  console.info(`Section: data: ${dataSectionSize} (${dataSections.length} elements)`);
  console.info(`Section: lib: ${libsSectionSize} (${libsSections.length} elements)`);

  console.info('Building disk config file...');

  let combinedBuffer = `wyland -build-disk wyland.section.bin -version ${options.version} -target ${options.target} -data ${codeSectionSize} -code ${40} -libs ${dataSectionSize + codeSectionSize} ${codeSections.join(' ')}  ${dataSections.join(' ')}  ${libsSections.join(' ')}`

  fs.writeFile('wyland.buildcommand.txt', combinedBuffer, (err) => {
    if (err) {
      console.error('Error writing to wyland.section.all:', err);
      return;
    }
    console.log('Wheader successfully written to wyland.section.all');
  });

  // Example of using the parsed options
  console.log(`Using version: ${options.version}`);
  console.log(`Using target: ${options.target} (${targetValue})`);
}

main();

