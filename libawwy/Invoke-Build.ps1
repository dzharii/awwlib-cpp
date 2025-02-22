function Invoke-Build {
    param(
        [Parameter(Mandatory = $true)]
        [string]$BuildType = "Release"
    )

    $buildScript = $null
    $platform = Get-Platform

    if ($platform -eq "Unix") {
        if ($BuildType -eq "Release") {
            $buildScript = "./build-release-on-linux.sh"
        }
        elseif ($BuildType -eq "Debug"){
            $buildScript = "./build-debug-on-linux.sh"
        } else {
            Write-Error "Invalid build type: $BuildType"
        }
    }
    elseif ($platform -eq "Windows") {
        if ($BuildType -eq "Release") {
            $buildScript = ".\build-release-on-windows.cmd"
        }
        elseif ($BuildType -eq "Debug"){
            $buildScript = ".\build-debug-on-windows.cmd"
        } else {
            Write-Error "Invalid build type: $BuildType"
        }
    }

    Write-Host "Starting build process using: $buildScript" -ForegroundColor Cyan
    & $buildScript
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build failed with exit code: $LASTEXITCODE" -ForegroundColor Red
        exit 1
    }

    $unitTestExecutableRelease = ""
    $unitTestExecutableDebug = ""

    if ($platform -eq "Unix") {
        $unitTestExecutableRelease = "./build/Release/unit_tests"
        $unitTestExecutableDebug = "./build/Debug/unit_tests"
    }
    elseif ($platform -eq "Windows") {
        $unitTestExecutableRelease = "./build/Release/unit_tests.exe"
        $unitTestExecutableDebug = "./build/Debug/unit_tests.exe"
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
        & $unitTestExecutable "--success=1"
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Unit tests failed with exit code: $LASTEXITCODE" -ForegroundColor Red
            exit 1
        }
    }

    Write-Host "Build and test process completed successfully." -ForegroundColor Green
}