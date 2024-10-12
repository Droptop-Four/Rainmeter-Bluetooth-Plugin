param(
    [string]$SolutionPath, # Path to the solution file (.sln)
    [string]$PluginVersion   # Version of the plugin
)

# Function to prompt for missing parameters
function Prompt-ForInput {
    param (
        [string]$message
    )
    Read-Host -Prompt $message
}

# Function to validate the plugin version
function Validate-Version {
    param (
        [string]$version
    )
    # Regular expression to match the format X.X.X.X where X is a number
    return $version -match '^\d+\.\d+\.\d+\.\d+$'
}

# Ask user for missing parameters
if (-not $SolutionPath) {
    $SolutionPath = Prompt-ForInput "Enter the path to the solution file (.sln)"
}

if (-not $PluginVersion) {
    do {
        $PluginVersion = Prompt-ForInput "Enter the plugin version (e.g., 1.0.0.0)"
        if (-not (Validate-Version $PluginVersion)) {
            Write-Host "Invalid version format. Please enter a version in the format X.X.X.X (e.g., 1.0.0.0)."
        }
    } while (-not (Validate-Version $PluginVersion))
}

# Path to MSBuild (it should find the path automatically, after Visual Studio 2017)
$msbuildPath = &"${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe

# Check if the solution file exists
if (-not (Test-Path $SolutionPath)) {
    Write-Host "Solution file not found: $SolutionPath"
    exit 1
}

# Path to the .rc file
$rcFilePath = Join-Path (Split-Path $SolutionPath) "src\BluetoothPlugin.rc"

# Check if the .rc file exists
if (-not (Test-Path $rcFilePath)) {
    Write-Host "RC file not found: $rcFilePath"
    exit 1
}

# Update the plugin version in the .rc file while retaining the encoding
$rcFile = Get-Content $rcFilePath -Encoding Unicode

# Convert the plugin version to the format required for FILEVERSION (comma-separated)
$versionParts = $PluginVersion -split '\.'
if ($versionParts.Length -lt 4) {
    $versionParts += (0 * ($versionParts.Length..3))
}
$commaSeparatedVersion = $versionParts -join ','

# Replace the FILEVERSION line in the .rc file
$content = $rcFile -replace "(FILEVERSION )\d+,\d+,\d+,\d+", "FILEVERSION $commaSeparatedVersion"
$content = $content -replace "(VALUE `"FileVersion`", `")[^`"]*(`")", "VALUE `"FileVersion`", `"$PluginVersion`""
Set-Content -Path $rcFilePath -Value $content -Encoding Unicode

Write-Host "Updated plugin version in $rcFilePath to $PluginVersion"

# Build the solution for Release|Win32
Write-Host "Building solution for Release|Win32..."
& $msbuildPath $SolutionPath /p:Configuration=Release /p:Platform=Win32 /t:Build /m
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed for Release|Win32"
    exit 1
}

# Path to the Win32 output folder
$win32OutputPath = Join-Path (Split-Path $SolutionPath) "build\Release\32-bit"
$win32Dll = Join-Path $win32OutputPath "Bluetooth.dll"

# Check if the DLL exists
if (-not (Test-Path $win32Dll)) {
    Write-Host "Bluetooth.dll not found for Win32 build in $win32OutputPath"
    exit 1
}

# Build the solution for Release|x64
Write-Host "Building solution for Release|x64..."
& $msbuildPath $SolutionPath /p:Configuration=Release /p:Platform=x64 /t:Build /m
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed for Release|x64"
    exit 1
}

# Path to the x64 output folder
$x64OutputPath = Join-Path (Split-Path $SolutionPath) "build\Release\64-bit"
$x64Dll = Join-Path $x64OutputPath "Bluetooth.dll"

# Check if the DLL exists
if (-not (Test-Path $x64Dll)) {
    Write-Host "Bluetooth.dll not found for x64 build in $x64OutputPath"
    exit 1
}

# Create a ZIP file name based on the provided version
$zipFileName = "RainmeterBluetoothPlugin_${PluginVersion}_x86_x64_dll.zip"
# Save the ZIP file inside the build folder
$zipFilePath = Join-Path (Split-Path $SolutionPath) "build\$zipFileName"
Write-Host "Creating ZIP file: $zipFilePath"

# Add DLLs to the ZIP archive
Add-Type -AssemblyName 'System.IO.Compression.FileSystem'

# Create a temporary directory to store DLLs
$tempDir = [System.IO.Path]::Combine([System.IO.Path]::GetTempPath(), [System.IO.Path]::GetRandomFileName())
[System.IO.Directory]::CreateDirectory($tempDir)

# Create subdirectories for 32-bit and 64-bit
$win32Dir = Join-Path $tempDir "x86"
$x64Dir = Join-Path $tempDir "x64"
[System.IO.Directory]::CreateDirectory($win32Dir)
[System.IO.Directory]::CreateDirectory($x64Dir)

# Copy both DLLs to the respective subdirectories
Copy-Item -Path $win32Dll -Destination (Join-Path $win32Dir "Bluetooth.dll")
Copy-Item -Path $x64Dll -Destination (Join-Path $x64Dir "Bluetooth.dll")

# Zip the folder containing the DLLs
[System.IO.Compression.ZipFile]::CreateFromDirectory($tempDir, $zipFilePath)

# Clean up the temporary directory
Remove-Item -Recurse -Force $tempDir

Write-Host "Build outputs successfully zipped in: $zipFilePath"
