param(
    [string]$FilePath
)

if (-not $FilePath) {
    Write-Host "Usage: .\format.ps1 <filename>"
    exit 1
}

if (-Not (Test-Path $FilePath)) {
    Write-Error "File not found: $FilePath"
    exit 1
}

clang-format -i $FilePath
Write-Host "Formatted: $FilePath"