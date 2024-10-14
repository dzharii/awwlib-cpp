#!/bin/env pwsh
param(
    [Parameter(Mandatory = $true)]
    [string]$Command,
    [Parameter(Mandatory = $false)]
    [string]$Name,
    [Parameter(Mandatory = $false)]
    [string]$OnlyFilePrefix = ""

)

$ErrorActionPreference = "Stop"
$ThisScriptFolderPath = Split-Path -Parent $MyInvocation.MyCommand.Definition

$COMMAND_HELP = "help"
$COMMAND_MDTREE = "mdtree"
$COMMAND_BUILD_RELEASE = "build-release"
$COMMAND_BUILD_DEBUG = "build-debug"
$COMMAND_GEN = "gen"

$HELP_MESSAGE = @"
Usage:
   huddy.ps1 -Command <command> -Name <component_name>
   aww run huddy -Command <command> -Name <component_name>

Commands:
    $($COMMAND_HELP):
      Shows this help message

    $($COMMAND_MDTREE) [-OnlyFilePrefix <prefix>]:
      Generates a markdown documentation for C++ files.
        - OnlyFilePrefix: Generates documentation only for files that have the specified prefix in their name.

    $($COMMAND_BUILD_RELEASE):
      Builds the project using platform-specific build scripts and runs unit tests.

    $($COMMAND_DEBUG_RELEASE):
      Builds the project using platform-specific build scripts and runs unit tests.

    $($COMMAND_GEN) -Name <file_path>:
      Generates boilerplate header and source files for a new component. The file path should be in the format <type>/<name>,
      such as 'model/componentName', which will create 'include/model/componentName.hpp' and 'src/model/componentName.cpp'.
      The new source file is also automatically added to the CMakeLists.txt file in the appropriate section.
"@

function Invoke-Build {
    param(
        [Parameter(Mandatory = $true)]
        [string]$BuildType = "Release"
    )

    $buildScript = $null

    if ([Environment]::OSVersion.Platform -eq "Unix") {
        if ($BuildType -eq "Release") {
            $buildScript = "./build-release-on-linux.sh"
        }
        elseif ($BuildType -eq "Debug"){
            $buildScript = "./build-debug-on-linux.sh"
        } else {
            Write-Error "Invalid build type: $BuildType"
        }
    }
    elseif ([Environment]::OSVersion.Platform -eq "Win32NT"){
        if ($BuildType -eq "Release") {
            $buildScript = ".\build-release-on-windows.cmd"
        }
        elseif ($BuildType -eq "Debug"){
            $buildScript = ".\build-debug-on-windows.cmd"
        } else {
            Write-Error "Invalid build type: $BuildType"
        }
    }
    else {
        Write-Error "Unsupported platform: $([Environment]::OSVersion.Platform)"
    }

    Write-Host "Starting build process using: $buildScript" -ForegroundColor Cyan
    & $buildScript
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build failed with exit code: $LASTEXITCODE" -ForegroundColor Red
        exit 1
    }


    if ([Environment]::OSVersion.Platform -eq "Unix") {
        $unitTestExecutableRelease = "./build/Release/unit_tests"
        $unitTestExecutableDebug = "./build/Debug/unit_tests"
    }
    elseif ([Environment]::OSVersion.Platform -eq "Win32NT") {
        $unitTestExecutableRelease = "./build/Release/unit_tests.exe"
        $unitTestExecutableDebug = "./build/Debug/unit_tests.exe"
    } else {
        Write-Error "Unsupported platform: $([Environment]::OSVersion.Platform)"
    }

    $unitTestExecutable = $null

    # Check which files exist and determine the most recent one
    $releaseExists = Test-Path $unitTestExecutableRelease
    $debugExists = Test-Path $unitTestExecutableDebug

    if ($releaseExists -and $debugExists) {
        # Both executables exist, compare last write times
        $releaseInfo = Get-Item $unitTestExecutableRelease
        $debugInfo = Get-Item $unitTestExecutableDebug

        if ($releaseInfo.LastWriteTime -gt $debugInfo.LastWriteTime) {
            $unitTestExecutable = $unitTestExecutableRelease
        }
        else {
            $unitTestExecutable = $unitTestExecutableDebug
        }
    }
    elseif ($releaseExists) {
        # Only the Release executable exists
        $unitTestExecutable = $unitTestExecutableRelease
    }
    elseif ($debugExists) {
        # Only the Debug executable exists
        $unitTestExecutable = $unitTestExecutableDebug
    }
    else {
        # Neither executable exists
        Write-Error "No unit test executable found in the Release or Debug directories."
    }

    # If $unitTestExecutable is set, it means one or both files were found and one is chosen
    if ($unitTestExecutable) {
        Write-Output "Using unit test executable at '$unitTestExecutable'"
        & $unitTestExecutable
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Unit tests failed with exit code: $LASTEXITCODE" -ForegroundColor Red
            exit 1
        }
    }

    Write-Host "Build and test process completed successfully." -ForegroundColor Green
}

function Create-MDTREE {
    param(
        [Parameter(Mandatory = $false)]
        [string]$OnlyFilePrefix = ""
    )

    $trippleBacktick = "``````"
    $oneBacktick = "``"

    $outputFile = "2024-04-21-mdtree-output.md"
    $outputFilePath = Join-Path -Path $ThisScriptFolderPath -ChildPath $outputFile
    $cppExtensions = @("*.h", "*.hpp", "*.c", "*.cpp", "*.cxx")
    $folders = @("app", "include", "src", "tests")

    $markdown = ""
    $markdown += "# MDTREE ($($outputFile))`n`n"
    $markdown += "Generated on **$((Get-Date).ToString('yyyy-MM-dd HH:mm:ss'))**`n`n"
    $markdown += "This document contains the content of all C++ files in the current project. `n`n"
    $markdown += "[[_TOC_]]`n`n"

    foreach ($folder in $folders) {
        $path = Join-Path -Path $ThisScriptFolderPath -ChildPath $folder
        $files = Get-ChildItem -Path $path -Include $cppExtensions -Recurse

        foreach ($file in $files) {
            if ($OnlyFilePrefix) {
                if (-not($file.Name.StartsWith($OnlyFilePrefix, [System.StringComparison]::OrdinalIgnoreCase))) {
                    Write-Host "Skipping file: $($file.FullName) (filter:OnlyFilePrefix)" -ForegroundColor Yellow
                    continue
                }
            }
            $fileContent = Get-Content $file.FullName -Raw
            $relativePath = $file.FullName.Replace($ThisScriptFolderPath + '\', '')

            $markdown += "## **$($relativePath)**:`n`n"
            $markdown += "$($trippleBacktick)cpp`n$($fileContent)`n$($trippleBacktick)`n`n"
        }
    }

    $markdown | Out-File $outputFilePath -Encoding utf8
    Write-Host "Markdown documentation generated at: $outputFilePath" -ForegroundColor Green
}

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



switch ($Command.ToLower()) {
    $COMMAND_HELP {
        Write-Host $HELP_MESSAGE
    }

    $COMMAND_MDTREE {
        Create-MDTREE -OnlyFilePrefix $OnlyFilePrefix
    }

    $COMMAND_BUILD_DEBUG {
        Invoke-Build -BuildType "Debug"
    }

    $COMMAND_BUILD_RELEASE {
        Invoke-Build -BuildType "Release"
    }

    $COMMAND_GEN {
        if (-not $Name) {
            Write-Host "Error: -Name parameter is required for the $($COMMAND_GEN) command" -ForegroundColor Red
            exit 1
        }
        Generate-Files -FilePath $Name
    }

    Default {
        Write-Host $("=" * 80) -ForegroundColor Red
        Write-Host "Unknown command: $Command" -ForegroundColor Red
        Write-Host $("=" * 80) -ForegroundColor Red
        Write-Host $HELP_MESSAGE
        exit 1
    }
}

Write-Host "Done: $(Get-Date -Format o)"
