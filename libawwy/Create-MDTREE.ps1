function Create-MDTREE {
    param(
        [Parameter(Mandatory = $false)]
        [string]$OnlyFilePrefix = ""
    )

    $tripleBacktick = "``````"
    $oneBacktick = "``"

    $outputFile = "2024-04-21-mdtree-output.md"
    $outputFilePath = Join-Path -Path $ThisScriptFolderPath -ChildPath $outputFile
    $cppExtensions = @("*.h", "*.hpp", "*.c", "*.cpp", "*.cxx")
    $allowedFolders = @("include", "src")  # Only scan these folders

    $markdown = ""
    $markdown += "# MDTREE ($($outputFile))`n`n"
    $markdown += "Generated on **$((Get-Date).ToString('yyyy-MM-dd HH:mm:ss'))**`n`n"
    $markdown += "This document contains the content of all C++ files in the current project. `n`n"
    $markdown += "[[_TOC_]]`n`n"

    foreach ($folder in $allowedFolders) {
        $path = Join-Path -Path $ThisScriptFolderPath -ChildPath $folder
        if (-not (Test-Path $path)) {
            Write-Host "Skipping non-existent folder: $($folder)" -ForegroundColor Yellow
            continue
        }
        
        $files = Get-ChildItem -Path $path -Include $cppExtensions -Recurse

        foreach ($file in $files) {
            if ($OnlyFilePrefix) {
                if (-not($file.Name.StartsWith($OnlyFilePrefix, [System.StringComparison]::OrdinalIgnoreCase))) {
                    Write-Host "Skipping file: $($file.FullName) (filter:OnlyFilePrefix)" -ForegroundColor Yellow
                    continue
                }
            }
            $fileContent = Get-Content $file.FullName -Raw
            $relativePath = $file.FullName.Replace($ThisScriptFolderPath + '\\', '')

            $markdown += "## **$($relativePath)**:`n`n"
            $markdown += "$($tripleBacktick)cpp`n$($fileContent)`n$($tripleBacktick)`n`n"
        }
    }

    $markdown | Out-File $outputFilePath -Encoding utf8
    Write-Host "Markdown documentation generated at: $outputFilePath" -ForegroundColor Green
}
