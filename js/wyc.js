const fs = require('fs');
const path = require('path');

// Wheader class
class Wheader {
  constructor(certificat, target, version, code, data, lib) {
    // Validate types and sizes of parameters
    if (!(certificat instanceof Uint8Array) || certificat.length !== 3) {
      throw new TypeError("certificat must be a Uint8Array of length 3");
    }
    this.certificat = certificat;

    if (typeof target !== 'number') {
      throw new TypeError("target must be a number");
    }
    this.target = target;

    if (typeof version !== 'number') {
      throw new TypeError("version must be a number");
    }
    this.version = version;

    if (typeof code !== 'bigint') {
      throw new TypeError("code must be a BigInt");
    }
    this.code = code;

    if (typeof data !== 'bigint') {
      throw new TypeError("data must be a BigInt");
    }
    this.data = data;

    if (typeof lib !== 'bigint') {
      throw new TypeError("lib must be a BigInt");
    }
    this.lib = lib;
  }
}

// Define targets as constants
const targets = {
  wtarg64: 2660,
  wtarg32: 2610,
  wtargmarch: 436,
  wtargfast: 2810
};

// Function to create a Wheader with default or extracted values from files
function makeHeader(target = targets.wtarg64, version = 1, 
                    code = 48, 
                    data = 0xEEEEEEEEEEEEEEEEn, 
                    libs = 0xFFFFFFFFFFFFFFFFn) {
  let wheader = new Wheader(
    new Uint8Array(['w'.charCodeAt(0), 'l'.charCodeAt(0), 'f'.charCodeAt(0)]), // certificat = ['w', 'l', 'f']
    target,
    version,
    BigInt(code),
    BigInt(data),
    BigInt(libs)
  );
  return wheader;
}

// Function to load a file if it exists
function loadFile(fileName) {
  try {
    if (fs.existsSync(fileName)) {
      const fileContent = fs.readFileSync(fileName, 'utf8');
      return fileContent;
    } else {
      console.log(`The file ${fileName} does not exist.`);
      return null;
    }
  } catch (err) {
    console.error('Error reading the file:', err);
    return null;
  }
}

// Function to create a Wheader from files
function createWheaderFromFile() {
  // Check if the main file "wyland.section.data" exists
  const dataFile = 'wyland.section.data';
  const codeFile = 'wyland.section.code';
  const libFile = 'wyland.section.lib';

  // Read the files if they exist
  const dataContent = loadFile(dataFile);
  const codeContent = loadFile(codeFile);
  const libContent = loadFile(libFile);

  // If the file 'wyland.section.data' exists, we can create a Wheader
  if (dataContent) {
    // Customize the following based on how you want to retrieve the data
    const target = targets.wtarg64; // Use the default target or read a specific value from the files
    const version = 1; // Use a default version or extract from a file if necessary
    const code = codeContent ? BigInt(codeContent.trim()) : 48n;  // If the .code file exists, use it, otherwise use the default value
    const data = BigInt('0x' + dataContent.trim());  // Assume the file content is a hexadecimal number
    const lib = libContent ? BigInt(libContent.trim()) : 0xFFFFFFFFFFFFFFFFn;  // Same for the .lib file

    // Create a Wheader with the extracted values
    const wheader = new Wheader(
      new Uint8Array(['w'.charCodeAt(0), 'l'.charCodeAt(0), 'f'.charCodeAt(0)]), // Certificat = ['w', 'l', 'f']
      target,
      version,
      code,
      data,
      lib
    );

    console.log('Wheader created from file:', wheader);
    return wheader;
  } else {
    console.log("Unable to create Wheader because the data file is missing.");
    return null;
  }
}

// Example call to the function to create a Wheader
createWheaderFromFile();