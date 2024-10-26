function Invoke-ClangFormatAutofix {
    <#
    .SYNOPSIS
    Automatically formats source code files using clang-format.exe.

    .DESCRIPTION
    This function recursively searches for C/C++ source code files (*.cpp, *.h, *.c, *.hpp)
    in the current directory and its subdirectories, excluding any files in the 'third-party' folder.
    It applies clang-format.exe to each file to fix the formatting in place.

    .EXAMPLE
    Invoke-ClangFormatAutofix
    #>

    # Define the file extensions to process
    $fileExtensions = @("*.cpp", "*.h", "*.c", "*.hpp")

    # Check if command clang-format is installed
    if (-not (Get-Command clang-format -ErrorAction SilentlyContinue)) {
        Write-Error "Error: clang-format is not installed. Please install clang-format and add it to the system PATH."
        return
    }

    try {
        Write-Host "Starting Clang-Format Autofix..."

        # Retrieve all matching files recursively
        $allFiles = Get-ChildItem -Recurse -Include $fileExtensions -File

        foreach ($file in $allFiles) {
            # Check if the file is inside the 'third-party' directory

            # split the path by the directory separator use .NET method
            $directories = $file.FullName.Split([System.IO.Path]::DirectorySeparatorChar)
            $shoulkSkip = $directories.Contains("third-party") -or $directories.Contains("build")
            if ($shoulkSkip) {
                Write-Host "Skipping file: $($file.FullName)"
                continue
            }

            Write-Host "Formatting file: $($file.FullName)"
            # Apply clang-format with in-place editing
            clang-format -i "$($file.FullName)"
        }

        Write-Host "Clang-Format Autofix completed successfully."
    }
    catch {
        Write-Error "An error occurred while running Clang-Format Autofix: $_"
    }
}
