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

    # Check if inside a Git repository
    try {
        $isRepo = git rev-parse --is-inside-work-tree 2>$null
        if ($isRepo -ne "true") {
            throw "Current directory is not a Git repository."
        }
    } catch {
        throw "Current directory is not a Git repository."
    }

    # Get the current branch name
    try {
        $currentBranch = git symbolic-ref --short HEAD
        if (-not $currentBranch) {
            throw "Unable to determine the current Git branch."
        }
    } catch {
        throw "Unable to determine the current Git branch."
    }

    # Get the upstream branch
    try {
        $upstream = git for-each-ref --format='%(upstream:short)' refs/heads/$currentBranch
        if (-not $upstream) {
            throw "No upstream branch set for '$currentBranch'. Please set an upstream branch."
        }
    } catch {
        throw "Error retrieving the upstream branch for '$currentBranch'."
    }

    # Check if there are unpushed commits
    try {
        $unpushedCommits = git log --oneline $upstream..$currentBranch
        if (-not $unpushedCommits) {
            $output += "No unpushed commits found. Your local branch '$currentBranch' is up to date with '$upstream'.`n"
            return $output
        }
    } catch {
        throw "Error retrieving unpushed commits."
    }

    $output += "Unpushed commits on branch '$currentBranch' compared to '$upstream':`n`n"

    # Split the commits into an array
    $commitList = $unpushedCommits -split "`n"

    foreach ($commit in $commitList) {
        # Extract commit hash and message
        if ($commit -match "^([a-f0-9]{7,40})\s+(.*)$") {
            $hash = $matches[1]
            $message = $matches[2]
        } else {
            $hash = "Unknown"
            $message = "No commit message available."
        }

        $output += "Commit: $hash - $message`n`n"

        # Generate diff for the specific commit
        try {
            # Get the parent commit
            $parent = git rev-parse "$hash^"

            # Generate a diff including entire file (-U999999 for maximum context)
            $diff = git diff $parent $hash -U999999 --color

            if ($diff) {
                $output += "Diff for commit $($hash):`n"
                $output += "$diff`n"
                $output += "`n"  # Add an empty line for readability
            } else {
                $output += "No changes detected in commit $hash.`n`n"
            }
        } catch {
            $output += "Unable to generate diff for commit $hash.`n`n"
        }
    }

    return $output
}
