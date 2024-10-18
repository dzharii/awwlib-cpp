function Generate-Files {
    param (
        [Parameter(Mandatory = $true)]
        [ValidateScript({ ($_ -notmatch '\') -and ($_ -notmatch '/') -and ($_ -notmatch '\.') })] # Validate filename without path or extension
        [string]$FileName
    )

    # Convert filename to lowercase to ensure consistency in file naming
    $FileName = $FileName.ToLower()

    # Define file paths to be generated
    $faPath = "include\$($FileName)\$($FileName).hpp"
    $fbPath = "src\$($FileName)\$($FileName).cpp"
    $fcPath = "tests\$($FileName)\$($FileName).cpp"

    function Get-CppValidIdentifier {
        param (
            [string]$inputString
        )
        return ($inputString -replace '[^a-zA-Z0-9]', '_').ToUpper()
    }

    function New-DirectoryIfNotExists {
        param (
            [string]$directoryPath
        )
        if (-not (Test-Path -Path $directoryPath)) {
            New-Item -Path $directoryPath -ItemType Directory | Out-Null
        }
    }

    # Generate file content in memory
    $cppIdentifier = Get-CppValidIdentifier -inputString $FileName
    $faContent = @"
#pragma once
#ifndef ${cppIdentifier}_HPP
#define ${cppIdentifier}_HPP

namespace aww {

} // end of namespace
#endif // ${cppIdentifier}_HPP
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
    New-DirectoryIfNotExists -directoryPath "include/$($FileName)"
    Set-Content -Path $faPath -Value $faContent

    New-DirectoryIfNotExists -directoryPath "src/$($FileName)"
    Set-Content -Path $fbPath -Value $fbContent

    New-DirectoryIfNotExists -directoryPath "tests/$($FileName)"
    Set-Content -Path $fcPath -Value $fcContent

    # Modify CMakeLists.txt files
    $cmakeListPath = "CMakeLists.txt"
    $testsCmakeListPath = "tests/CMakeLists.txt"

    function Set-ContentLineBeforeMarker {
        param (
            [string]$filePath,
            [string]$marker,
            [string]$lineToInsert
        )

        if (Test-Path -Path $filePath) {
            try {
                $content = Get-Content -Path $filePath
                $newContent = New-Object System.Collections.Generic.List[System.String]
                $inserted = $false

                foreach ($line in $content) {
                    if (-not $inserted -and $line -match $marker) {
                        $newContent.Add($lineToInsert)
                        $inserted = $true
                    }
                    $newContent.Add($line)
                }

                Set-Content -Path $filePath -Value $newContent
                Write-Host "Modified: $($filePath)"
            } catch {
                Write-Error "Failed to read or modify $($filePath): $_"
                return
            }
        }
    }

    # Update main CMakeLists.txt
    Set-ContentLineBeforeMarker -filePath $cmakeListPath -marker "#ge0mh0v43gk" -lineToInsert "    src/$($FileName)/$($FileName).cpp"

    # Update tests CMakeLists.txt
    Set-ContentLineBeforeMarker -filePath $testsCmakeListPath -marker "#faudv6fbgzt" -lineToInsert "    $($FileName)/$($FileName).cpp"

    # Report
    Write-Host "Generated files:"
    Write-Host " - $($faPath)"
    Write-Host " - $($fbPath)"
    Write-Host " - $($fcPath)"
}
