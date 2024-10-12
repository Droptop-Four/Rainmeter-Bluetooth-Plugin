<#
.SYNOPSIS
Anonymizes device information in an INI file and renames associated image files.

.DESCRIPTION
This script reads an INI file containing device information, anonymizes the device names, addresses, and IDs, and updates the INI file with the new information. It also renames associated image files in the "img" folder to match the new device names.

.PARAMETER FilePath
The path to the INI file that needs to be anonymized.

.INPUTS
None. The script does not accept pipeline input.

.OUTPUTS
None. The script does not produce output objects.

.NOTES
- The script checks if the specified INI file and the "img" folder exist.
- It reads the INI file content and parses the DevicesNumber and Devices string.
- It generates new device names, addresses, and IDs, and updates the INI file accordingly.
- It renames associated image files in the "img" folder to match the new device names.
- If any errors occur during file operations, appropriate error messages are displayed.

.EXAMPLE
.\anonymize.ps1 -FilePath "C:\path\to\your\file.ini"
Anonymizes the device information in the specified INI file and renames associated image files.
#>

param (
    [Parameter(Mandatory = $true)]
    [string]$FilePath
)

# Function to generate a random Bluetooth address
function Get-RandomBluetoothAddress {
    return (1..6 | ForEach-Object { '{0:X2}' -f (Get-Random -Minimum 0 -Maximum 255) }) -join ':'
}

# Function to generate a random device ID
function Get-RandomDeviceId {
    $randomBTAddress = Get-RandomBluetoothAddress
    return "Bluetooth#Bluetoothab:cd:ef:01:23:45-$randomBTAddress"
}

# Function to replace content in the INI file
function Replace-IniContent {
    param (
        [string]$Content,
        [string]$Pattern,
        [string]$Replacement
    )
    return [System.Text.RegularExpressions.Regex]::Replace($Content, $Pattern, $Replacement)
}

# Check if the file exists
if (-not (Test-Path $FilePath)) {
    Write-Error "The specified file does not exist: $FilePath"
    exit 1
}

# Get the directory of the INI file
$iniDirectory = Split-Path -Parent $FilePath

# Define the path to the img folder
$imgFolderPath = Join-Path $iniDirectory "img"

# Check if the img folder exists
if (-not (Test-Path $imgFolderPath)) {
    Write-Error "The img folder does not exist in the same directory as the INI file."
    exit 1
}

# Read the INI file content
try {
    $iniContent = Get-Content -Path $FilePath -Raw -ErrorAction Stop
}
catch {
    Write-Error "Unable to read the file: $_"
    exit 1
}

# Parse the DevicesNumber
$devicesNumberMatch = [regex]::Match($iniContent, '(?m)^DevicesNumber=(\d+)')
if (-not $devicesNumberMatch.Success) {
    Write-Error "DevicesNumber not found in the INI file."
    exit 1
}
$devicesNumber = [int]$devicesNumberMatch.Groups[1].Value

# Parse the Devices string
$devicesMatch = [regex]::Match($iniContent, '(?m)^Devices=(.+)')
if (-not $devicesMatch.Success) {
    Write-Error "Devices string not found in the INI file."
    exit 1
}
$devices = $devicesMatch.Groups[1].Value -split ';'

# Hashtable to store old and new device names
$deviceNameMap = @{}

# Modify each device
for ($i = 0; $i -lt $devicesNumber; $i++) {
    if ($i -ge $devices.Count) {
        Write-Warning "DevicesNumber is greater than the actual number of devices in the Devices string."
        break
    }
    $deviceParts = $devices[$i] -split '\|'
    
    # Store the old and new device names
    $oldName = $deviceParts[0]
    $newName = "Device$($i+1)"
    $deviceNameMap[$oldName] = $newName
    
    # Change device name
    $deviceParts[0] = $newName
    
    # Change device address (randomly generated)
    $deviceParts[1] = Get-Random -Minimum 100000000000000 -Maximum 999999999999999
    
    # Change device ID (randomly generated)
    $deviceParts[2] = Get-RandomDeviceId
    
    $devices[$i] = $deviceParts -join '|'
    
    # Update individual device variables
    $iniContent = Replace-IniContent -Content $iniContent -Pattern "(?m)^Device$($i+1)Name=.*" -Replacement "Device$($i+1)Name=$($deviceParts[0])"
    $iniContent = Replace-IniContent -Content $iniContent -Pattern "(?m)^Device$($i+1)Address=.*" -Replacement "Device$($i+1)Address=$($deviceParts[1])"
    $iniContent = Replace-IniContent -Content $iniContent -Pattern "(?m)^Device$($i+1)Id=.*" -Replacement "Device$($i+1)Id=$($deviceParts[2])"
}

# Update the Devices variable
$newDevicesString = $devices -join ';'
$iniContent = Replace-IniContent -Content $iniContent -Pattern "(?m)^Devices=.*" -Replacement "Devices=$newDevicesString"

# Write the modified content back to the INI file
try {
    $iniContent | Set-Content -Path $FilePath -Encoding Unicode -ErrorAction Stop
    Write-Host "INI file has been updated successfully."
}
catch {
    Write-Error "Unable to write to the file: $_"
    exit 1
}

# Rename image files
foreach ($oldName in $deviceNameMap.Keys) {
    $newName = $deviceNameMap[$oldName]
    $oldImagePath = Join-Path $imgFolderPath "$($oldName)_thumbnail.jpg"

    if (Test-Path $oldImagePath) {
        $newImagePath = Join-Path $imgFolderPath "$($newName)_thumbnail.jpg"
        try {
            # Use Move-Item instead of Rename-Item
            Move-Item -Path $oldImagePath -Destination $newImagePath -Force -ErrorAction Stop
            Write-Host "Renamed image: $($oldName)_thumbnail.jpg to $($newName)_thumbnail.jpg"
        }
        catch {
            Write-Error "Unable to rename image $($oldName)_thumbnail.jpg: $_"
            
            # If Move-Item fails, try an alternative method
            try {
                Copy-Item -Path $oldImagePath -Destination $newImagePath -Force -ErrorAction Stop
                Remove-Item -Path $oldImagePath -Force -ErrorAction Stop
                Write-Host "Renamed image (using copy and delete): $($oldName)_thumbnail.jpg to $($newName)_thumbnail.jpg"
            }
            catch {
                Write-Error "Failed to rename image using alternative method: $_"
            }
        }
    }
    else {
        Write-Warning "Image file not found: $oldImagePath"
    }
}

Write-Host "Script execution completed."
