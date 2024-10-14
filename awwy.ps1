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
$libPath = Join-Path -Path $ThisScriptFolderPath -ChildPath "libawwy"

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

. $libPath/Get-Platform.ps1
. $libPath/Invoke-Build.ps1
. $libPath/Create-MDTREE.ps1
. $libPath/Generate-Files.ps1


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
