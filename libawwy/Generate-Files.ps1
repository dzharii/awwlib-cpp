function Generate-Files {
    param (
        [Parameter(Mandatory = $true)]
        [ValidateScript({ ($_ -notmatch '\\') -and ($_ -notmatch '/') -and ($_ -notmatch '\.') })] # Validate filename without path or extension
        [string]$FileName
    )

    # Convert filename to lowercase to ensure consistency in file naming
    $FileName = $FileName.ToLower()

    # Define file paths to be generated
    $faPath = "include\$($FileName)\$($FileName).hpp"
    $fbPath = "src\$($FileName)\$($FileName).cpp"
    $fcPath = "tests\$($FileName)\$($FileName).cpp"

    # Generate file content in memory
    $faContent = @"
#pragma once
#ifndef $($FileName.ToUpper())_HPP
#define $($FileName.ToUpper())_HPP

namespace aww {

} // end of namespace
#endif // $($FileName.ToUpper())_HPP
"@

    $fbContent = @"
#include "$($FileName)/$($FileName).hpp"

namespace aww {

} // end of namespace
"@

    $fcContent = @"
#include "doctest/doctest.h"
#include "$($FileName)/$($FileName).hpp"

TEST_CASE("testcase template 1") {
    SUBCASE("sub testcase template 1") {

    }
}
"@

    # Check if files already exist
    $existingFiles = @()
    if (Test-Path -Path $faPath) { $existingFiles += $faPath }
    if (Test-Path -Path $fbPath) { $existingFiles += $fbPath }
    if (Test-Path -Path $fcPath) { $existingFiles += $fcPath }

    if ($existingFiles.Count -gt 0) {
        Write-Error "Error: The following files already exist:"
        $existingFiles | ForEach-Object { Write-Error $_ }
        return
    }

    # Create the directories if they do not exist
    if (-not (Test-Path -Path "include/$($FileName)")) {
        New-Item -Path "include/$($FileName)" -ItemType Directory | Out-Null
    }
    Set-Content -Path $faPath -Value $faContent

    if (-not (Test-Path -Path "src/$($FileName)")) {
        New-Item -Path "src/$($FileName)" -ItemType Directory | Out-Null
    }
    Set-Content -Path $fbPath -Value $fbContent

    if (-not (Test-Path -Path "tests/$($FileName)")) {
        New-Item -Path "tests/$($FileName)" -ItemType Directory | Out-Null
    }
    Set-Content -Path $fcPath -Value $fcContent

    # Modify CMakeLists.txt files
    $cmakeListPath = "CMakeLists.txt"
    $testsCmakeListPath = "tests/CMakeLists.txt"

    # Update main CMakeLists.txt
    if (Test-Path -Path $cmakeListPath) {
        try {
            $cmakeContent = Get-Content -Path $cmakeListPath
            $marker = "#ge0mh0v43gk"
            $index = $cmakeContent.IndexOf($marker)
            if ($index -ge 0) {
                $cmakeContent = $cmakeContent.Insert($index, "    src/$($FileName)/$($FileName).cpp`n"); Set-Content -Path $cmakeListPath -Value $cmakeContent
                Write-Host "Modified: $($cmakeListPath)"
            }
        } catch {
            Write-Error "Failed to read or modify $($cmakeListPath): $_"
            return
        }
    }

    # Update tests CMakeLists.txt
    if (Test-Path -Path $testsCmakeListPath) {
        try {
            $testsCmakeContent = Get-Content -Path $testsCmakeListPath
            $testsMarker = "#faudv6fbgzt"
            $testsIndex = $testsCmakeContent.IndexOf($testsMarker)
            if ($testsIndex -ge 0) {
                $testsCmakeContent = $testsCmakeContent.Insert($testsIndex, "    $($FileName)/$($FileName).cpp`n"); Set-Content -Path $testsCmakeListPath -Value $testsCmakeContent
                Write-Host "Modified: $($testsCmakeListPath)"
            }
        } catch {
            Write-Error "Failed to read or modify $($testsCmakeListPath): $_"
            return
        }
    }

    # Report
    Write-Host "Generated files:"
    Write-Host " - $($faPath)"
    Write-Host " - $($fbPath)"
    Write-Host " - $($fcPath)"
}
