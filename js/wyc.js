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

// Function to read file content
function readFileSync(filePath) {
  try {
    if (fs.existsSync(filePath)) {
      return fs.readFileSync(filePath);
    } else {
      console.log(`File not found: ${filePath}`);
      return null;
    }
  } catch (error) {
    console.error(`Error reading file ${filePath}:`, error);
    return null;
  }
}

// Function to create Wheader and resolve addresses
function createWheaderFromFiles() {
  const dataFile = 'wyland.section.data';
  const codeFile = 'wyland.section.code';
  const libFile = 'wyland.section.lib';

  // Read the contents of the files
  const dataContent = readFileSync(dataFile);
  const codeContent = readFileSync(codeFile);
  const libContent = readFileSync(libFile);

  if (dataContent === null || codeContent === null || libContent === null) {
    console.log("Error: one or more input files are missing.");
    return null;
  }

  // Define default values if files are empty
  const target = targets.wtarg64; // You can modify this based on your needs
  const version = 1; // You can modify this based on your needs

  // Calculate the sizes of each section (in bytes)
  const headerSize = 3 + 2 + 4 + 8 + 8 + 8; // certificat (3 bytes) + target (2 bytes) + version (4 bytes) + code (8 bytes) + data (8 bytes) + lib (8 bytes)
  const codeSize = codeContent.length;
  const dataSize = dataContent.length;
  const libSize = libContent.length;

  // Resolve the addresses based on section sizes
  const codeAddress = BigInt(headerSize);
  const dataAddress = codeAddress + BigInt(codeSize);
  const libAddress = dataAddress + BigInt(dataSize);

  // Create the Wheader object
  const wheader = new Wheader(
    new Uint8Array([119, 108, 102]), // 'wlf' as certificat
    target,
    version,
    codeAddress,
    dataAddress,
    libAddress
  );

  console.log("Wheader created with addresses:");
  console.log("Code address:", codeAddress);
  console.log("Data address:", dataAddress);
  console.log("Lib address:", libAddress);

  return { wheader, codeContent, dataContent, libContent };
}

// Function to write Wheader to file
function writeWheaderToFile(wheader, codeContent, dataContent, libContent) {
  const fs = require('fs');

  // Combine header and sections into a single buffer
  const headerBuffer = Buffer.from([
    ...wheader.certificat,
    ...Buffer.alloc(2).writeUInt16LE(wheader.target, 0), // target as 2 bytes (little-endian)
    ...Buffer.alloc(4).writeUInt32LE(wheader.version, 0), // version as 4 bytes (little-endian)
    ...Buffer.alloc(8).writeBigInt64LE(wheader.code, 0), // code address as 8 bytes (little-endian)
    ...Buffer.alloc(8).writeBigInt64LE(wheader.data, 0), // data address as 8 bytes (little-endian)
    ...Buffer.alloc(8).writeBigInt64LE(wheader.lib, 0), // lib address as 8 bytes (little-endian)
  ]);

  // Write the combined data to the output file
  const outputFile = 'wyland.section.all';
  const combinedBuffer = Buffer.concat([headerBuffer, codeContent, dataContent, libContent]);

  fs.writeFile(outputFile, combinedBuffer, (err) => {
    if (err) {
      console.error('Error writing to wyland.section.all:', err);
      return;
    }
    console.log('Wheader successfully written to wyland.section.all');
  });
}

// Main function to compile the files and write the output
function compileAndWrite() {
  const { wheader, codeContent, dataContent, libContent } = createWheaderFromFiles();

  if (wheader) {
    writeWheaderToFile(wheader, codeContent, dataContent, libContent);
  } else {
    console.log("Failed to create Wheader due to missing files.");
  }
}

// Run the compilation process
compileAndWrite();

/*
  files: 
    input(s):
      wyland.section.data
        content: data section. Can be null or empty.
      wyland.section.code
        content: code section. Can be null or empty.
      wyland.section.lib
        content: lib section. Can be null or empty.

    output:
      wyland.section.all
        content:
        ! 3 bytes: "wlf" string
        ! 2 bytes: target
        ! 4 bytes: version
        ! 8 bytes: address of the 'code' section
        ! 8 bytes: address of the 'data' section
        ! 8 bytes: address of the 'lib' section

    compilation:
      open each files.
      Resorlve addresses.
      The addresse of the code section is:
      sizeof(header)
      same for other addresses.
      Address of data is <code> + sizeof(code)
      etc !

typedef struct {
  uint8_t  certificat[3];
  uint16_t target;
  uint32_t version;
  uint64_t code;
  uint64_t data;
  uint64_t lib;
} wheader_t;

    */