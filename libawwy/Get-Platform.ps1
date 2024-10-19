function Get-Platform {
    if ([Environment]::OSVersion.Platform -eq "Unix") {
        return "Unix"
    }
    elseif ([Environment]::OSVersion.Platform -eq "Win32NT") {
        return "Windows"
    }
    else {
        Write-Error "Unsupported platform: $([Environment]::OSVersion.Platform)"
    }
}