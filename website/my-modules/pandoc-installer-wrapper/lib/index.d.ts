/**
 * Runs Pandoc with the specified command-line arguments.
 * @param args Optional array of command-line arguments.
 * @param options Optional options for the child process.
 * @returns A promise that resolves with the output from Pandoc.
 */
export function runPandoc(args?: string[], options?: object): Promise<string>;

/**
 * Returns the full path to the Pandoc binary.
 * @returns The path to the Pandoc executable.
 * @throws An error if the Pandoc binary is not found.
 */
export function getPandocPath(): string;
