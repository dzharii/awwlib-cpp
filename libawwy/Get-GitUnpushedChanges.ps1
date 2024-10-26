function Get-GitUnpushedChanges {
    <#
    .SYNOPSIS
    Retrieves unpushed Git commits and their comprehensive diffs as a single text string.

    .DESCRIPTION
    This function identifies all commits in the current branch that have not been pushed to the remote repository.
    For each unpushed commit, it generates a diff that includes the entire file with marked changes.
    The function returns all information as one large text string.

    .EXAMPLE
    $unpushedChanges = Get-GitUnpushedChanges
    Write-Output $unpushedChanges

    .NOTES
    - Requires Git to be installed and accessible in the system's PATH.
    - Should be run inside a Git repository.
    #>

    # Initialize a variable to store the output
    $output = ""

    # Ensure Git is installed
    if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
        throw "Git is not installed or not found in PATH."
    }

    # Verify if the script is being run inside a Git repository
    try {
        Write-Host "Checking if the current directory is a Git repository..."
        $isRepo = git rev-parse --is-inside-work-tree 2>&1
        if ($isRepo -ne "true") {
            throw "Current directory is not a Git repository."
        }
    } catch {
        throw "Failed to verify Git repository status: $($_.Exception.Message)"
    }

    # Fetch the latest commits from the remote to ensure up-to-date information
    try {
        Write-Host "Fetching latest commits from the remote repository..."
        git fetch --quiet 2>&1
    } catch {
        throw "Failed to fetch updates from the remote repository."
    }

    # Get the current branch name
    try {
        Write-Host "Determining the current branch name..."
        $currentBranch = git symbolic-ref --short HEAD 2>&1
        if (-not $currentBranch) {
            throw "Unable to determine the current Git branch."
        }
    } catch {
        throw "Failed to retrieve the current Git branch name: $($_.Exception.Message)"
    }

    # Get the upstream branch
    try {
        Write-Host "Retrieving upstream branch for '$($currentBranch)'..."
        $upstreamBranch = git rev-parse --abbrev-ref "$($currentBranch)@{upstream}" 2>&1
        if (-not $upstreamBranch) {
            throw "No upstream branch set for '$($currentBranch)'. Use 'git branch --set-upstream-to' to set an upstream branch."
        }
    } catch {
        throw "Failed to retrieve the upstream branch for '$($currentBranch)': $($_.Exception.Message)"
    }

    # Use git cherry to find unpushed commits
    try {
        Write-Host "Identifying unpushed commits on branch '$($currentBranch)'..."
        $cherryOutput = git cherry -v $upstreamBranch $currentBranch 2>&1
        if (-not $cherryOutput) {
            $output += "No unpushed commits found. Local branch '$($currentBranch)' is up to date with '$($upstreamBranch)'.`n"
            return $output
        }
    } catch {
        throw "Failed to retrieve unpushed commits: $($_.Exception.Message)"
    }

    $output += "Unpushed commits on branch '$($currentBranch)' compared to '$($upstreamBranch)':`n`n"

    # Process each line of the cherry output
    $cherryLines = $cherryOutput -split "`n"

    foreach ($line in $cherryLines) {
        # Each line is prefixed with '+' (unpushed) or '-' (remote-only)
        if ($line.StartsWith('+')) {
            # Extract commit hash and message
            if ($line -match '^\+\s+([a-f0-9]{7,40})\s+(.*)$') {
                $commitHash = $matches[1]
                $commitMessage = $matches[2]
            } else {
                $commitHash = "Unknown"
                $commitMessage = "No commit message available."
            }

            $output += "Commit: $($commitHash) - $($commitMessage)`n`n"

            # Generate diff for the specific commit
            try {
                Write-Host "Generating diff for commit $($commitHash)..."

                # Get the parent commit for the diff
                $parentCommit = git rev-parse "$($commitHash)^" 2>&1
                if (-not $parentCommit) {
                    throw "Failed to retrieve parent commit for $($commitHash)"
                }

                # Generate a diff including entire file (-U999999 for maximum context)
                $diffOutput = git diff $parentCommit $commitHash -U999999 2>&1

                if ($diffOutput) {
                    $output += "Diff for commit $($commitHash):`n"
                    $output += "$diffOutput`n"
                    $output += "`n"  # Add an empty line for readability
                } else {
                    $output += "No changes detected in commit $($commitHash).`n`n"
                }
            } catch {
                $output += "Unable to generate diff for commit $($commitHash): $($_.Exception.Message)`n`n"
            }
        }
    }

    return $output
}
