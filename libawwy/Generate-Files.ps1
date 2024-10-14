function Generate-Files {
    param(
        [string]$FilePath
    )

    $allowedDirectories = @{
        'model' = '#875mpibn7z4'
    }

    $baseDir = "include"
    $srcDir = "src"
    $allowedKeys = @("model")

    $splitPath = $FilePath.Split("/")
    if ($splitPath.Count -ne 2) {
        Write-Error "Invalid file path. It should be in the format: <type>/<name>"
        return
    }

    $key = $splitPath[0]
    $name = $splitPath[1]

    if (-not $allowedKeys -contains $key) {
        Write-Error "Invalid key: '$($key)'. Allowed keys are: '$($allowedKeys)'"
        return
    }

    $marker = $allowedDirectories[$key]

    $headerFile = Join-Path -Path $ThisScriptFolderPath -ChildPath "$($baseDir)/$($key)/$($name).hpp"
    $sourceFile = Join-Path -Path $ThisScriptFolderPath -ChildPath "$($srcDir)/$($key)/$($name).cpp"

    if (Test-Path $headerFile) {
        Write-Error "File $($headerFile) already exists. Aborting."
        return
    }

    if (Test-Path $sourceFile) {
        Write-Error "File $($sourceFile) already exists. Aborting."
        return
    }

    $headerContent = @"
#pragma once
#ifndef $($name.ToUpper())_HPP
#define $($name.ToUpper())_HPP

class $($name) {
public:
    $($name)() = default;
    ~$($name)() = default;
};

#endif // $($name.ToUpper())_HPP
"@

    $sourceContent = @"
#include "$($key)/$($name).hpp"

"@

    # Update CMakeLists.txt
    $cmakeFile = Join-Path -Path $ThisScriptFolderPath -ChildPath "CMakeLists.txt"

    $cmakeLinesArray = [System.IO.File]::ReadAllLines($cmakeFile)
    $cmakeLines = [System.Collections.Generic.List[string]]::new($cmakeLinesArray)

    $markerFound = $false
    $insertIndex = -1

    Write-Host "Searching for '$marker' in CMakeLists.txt"
    for ($i = 0; $i -lt $cmakeLines.Count; $i++) {

        $line = $cmakeLines[$i]

        if ($line.IndexOf($marker) -ge 0) {
            $markerFound = $true
        }
        elseif ($markerFound -and [string]::IsNullOrWhiteSpace($line)) {
            Write-Host "Found insertion point at line $($i)"
            $insertIndex = $i
            break
        }
    }

    if (-not $markerFound) {
        Write-Error "Marker '$($marker)' not found in CMakeLists.txt. Aborting."
        return
    }

    if ($insertIndex -eq -1) {
        Write-Error "Could not find insertion point in CMakeLists.txt. Aborting."
        return
    }

    $newEntry = "    $srcDir/$key/$name.cpp"
    $cmakeLines.Insert($insertIndex, $newEntry)

    # Write changes to CMakeLists.txt
    try {
        New-Item -ItemType File -Path $headerFile -Force
        New-Item -ItemType File -Path $sourceFile -Force
        Set-Content -Path $headerFile -Value $headerContent -Encoding UTF8
        Set-Content -Path $sourceFile -Value $sourceContent -Encoding UTF8

        Write-Host "Files generated:" -ForegroundColor Green
        Write-Host "  $headerFile" -ForegroundColor Green
        Write-Host "  $sourceFile" -ForegroundColor Green


        [System.IO.File]::WriteAllLines($cmakeFile, $cmakeLines)
        Write-Host "CMakeLists.txt updated." -ForegroundColor Green
    } catch {
        Write-Error "Failed to write changes to CMakeLists.txt. Aborting."
        return
    }
}