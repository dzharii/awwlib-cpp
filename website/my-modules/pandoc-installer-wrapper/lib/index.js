/**
 * @fileoverview Module for running Pandoc.
 * Provides functions to locate and execute the Pandoc binary with custom arguments.
 * Assumes that the Pandoc binary is installed in the "pandoc-bin" directory inside the module.
 */

const { spawn } = require('child_process');
const path = require('path');
const fs = require('fs');

/**
 * Retrieves the path to the locally installed Pandoc binary.
 * The binary is expected to reside in the "pandoc-bin" directory inside the module folder.
 *
 * @returns {string} The full path to the Pandoc binary.
 * @throws Will throw an error if the Pandoc binary is not found.
 */
function getPandocPath() {
  // Use __dirname to base the location relative to the module folder.
  const baseDir = path.join(__dirname, '..');
  const binDir = path.join(baseDir, 'pandoc-bin', `pandoc-3.6.3`);
  let binaryName = 'pandoc';
  if (process.platform === 'win32') {
    binaryName += '.exe';
  }
  const fullPath = path.join(binDir, binaryName);
  if (fs.existsSync(fullPath)) {
    return fullPath;
  }
  throw new Error('Pandoc binary not found at ' + fullPath);
}

/**
 * Runs the Pandoc process with the specified command-line arguments.
 *
 * @param {string[]} [args=[]] - An array of command-line arguments to pass to Pandoc.
 * @param {object} [options={}] - Options for the child_process.spawn method.
 * @returns {Promise<string>} A promise that resolves with Pandoc's standard output upon success.
 * @throws Will reject with an error if Pandoc exits with a non-zero exit code.
 */
function runPandoc(args = [], options = {}) {
  return new Promise((resolve, reject) => {
    const pandocPath = getPandocPath();
    const pandocProc = spawn(pandocPath, args, options);

    let stdout = '';
    let stderr = '';

    pandocProc.stdout.on('data', data => {
      stdout += data;
    });

    pandocProc.stderr.on('data', data => {
      stderr += data;
    });

    pandocProc.on('close', code => {
      if (code !== 0) {
        reject(new Error(`Pandoc exited with code ${code}: ${stderr}`));
      } else {
        resolve(stdout);
      }
    });
  });
}

module.exports = { runPandoc, getPandocPath };
