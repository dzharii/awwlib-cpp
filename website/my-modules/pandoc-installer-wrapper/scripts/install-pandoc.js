#!/usr/bin/env node

/**
 * @fileoverview Post-installation script to download and install the Pandoc binary.
 * The module version is synchronized with the Pandoc version (3.6.3).
 * This script downloads the appropriate release archive for your OS from GitHub,
 * displays a progress bar during download, extracts the archive, and saves
 * the binary to the local "pandoc-bin" folder relative to the module's directory.
 */

const fs = require('fs');
const os = require('os');
const path = require('path');
const fetch = require('node-fetch');
const AdmZip = require('adm-zip');
const tar = require('tar');
const ProgressBar = require('progress');

const PANDOC_VERSION = '3.6.3';
const BASE_URL = `https://github.com/jgm/pandoc/releases/download/${PANDOC_VERSION}`;

let platform = os.platform();
let archiveName = '';

// Determine the appropriate archive based on the operating system.
if (platform === 'win32') {
  archiveName = `pandoc-${PANDOC_VERSION}-windows-x86_64.zip`;
} else if (platform === 'darwin') {
  archiveName = `pandoc-${PANDOC_VERSION}-macOS.zip`;
} else if (platform === 'linux') {
  archiveName = `pandoc-${PANDOC_VERSION}-linux-amd64.tar.gz`;
} else {
  console.error(`Unsupported platform: ${platform}`);
  process.exit(1);
}

const downloadUrl = `${BASE_URL}/${archiveName}`;

// Use __dirname to base the location inside the module folder (i.e. inside node_modules).
const baseDir = path.join(__dirname, '..');
const downloadDir = path.join(baseDir, 'pandoc-bin');
const pandocBinaryName = platform === 'win32' ? 'pandoc.exe' : 'pandoc';
const pandocBinaryPath = path.join(downloadDir, pandocBinaryName);

// Check if Pandoc is already downloaded.
if (fs.existsSync(pandocBinaryPath)) {
  console.log(`Pandoc is already installed at ${pandocBinaryPath}. Skipping download.`);
  process.exit(0);
}

// Ensure the download directory exists.
if (!fs.existsSync(downloadDir)) {
  fs.mkdirSync(downloadDir);
}

console.log(`Downloading Pandoc from ${downloadUrl} ...`);

(async () => {
  try {
    const response = await fetch(downloadUrl);
    if (!response.ok) {
      throw new Error(`Failed to download: ${response.statusText}`);
    }

    // Get the total file size for the progress bar (if available)
    const totalLength = response.headers.get('content-length');
    let bar;
    if (totalLength) {
      bar = new ProgressBar('Downloading Pandoc [:bar] :percent :etas', {
        width: 40,
        complete: '=',
        incomplete: ' ',
        total: parseInt(totalLength, 10)
      });
    } else {
      console.log('Total file size unknown');
    }

    const archivePath = path.join(downloadDir, archiveName);
    const fileStream = fs.createWriteStream(archivePath);
    
    // Stream the download and update the progress bar.
    await new Promise((resolve, reject) => {
      response.body.on('data', (chunk) => {
        if (bar) bar.tick(chunk.length);
      });
      response.body.pipe(fileStream);
      response.body.on('error', reject);
      fileStream.on('finish', resolve);
      fileStream.on('error', reject);
    });
    
    console.log('\nDownload complete.');
    console.log('Extracting Pandoc ...');
    
    // Extract the downloaded archive.
    if (archiveName.endsWith('.zip')) {
      const zip = new AdmZip(archivePath);
      zip.extractAllTo(downloadDir, true);
    } else if (archiveName.endsWith('.tar.gz')) {
      await tar.x({
        file: archivePath,
        cwd: downloadDir
      });
    }
    
    // Remove the archive file after extraction.
    fs.unlinkSync(archivePath);
    console.log('Pandoc installed to:', downloadDir);
  } catch (error) {
    console.error('Error during Pandoc installation:', error);
    process.exit(1);
  }
})();
