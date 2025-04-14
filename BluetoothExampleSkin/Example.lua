--[[
    Example.lua
    Rainmeter Bluetooth Plugin Example Script

    Author: Droptop Four - Bunz
    Version: 0.1.0.8
    Date: 2025
    Description: Example script to demonstrate the use of the Bluetooth plugin for Rainmeter.
    License: GNU GENERAL PUBLIC LICENSE 3.0
--]]

function Initialize()
    _Populate(1, 2)

    SKIN:Bang("!SetVariable", "PageNumber", 0)
    SKIN:Bang("!SetVariable", "PageDevice1", 1)
    SKIN:Bang("!SetVariable", "PageDevice2", 2)
end

function RefreshStatus()
    local Status = tonumber(SKIN:ReplaceVariables('[&BluetoothDevicesMeasure:BluetoothStatus()]'))
    local OldStatus = tonumber(SKIN:GetVariable('Status'))

    if (OldStatus ~= Status) then
        SKIN:Bang("!SetVariable", "Status", Status)
        SKIN:Bang("!WriteKeyValue", "Variables", "Status", string.format('%s', Status), "Example.ini")
    end
end

function Refresh()
    local DevicesString = SKIN:ReplaceVariables('[&BluetoothDevicesMeasure:AvailableDevices()]')
    local OldDevicesString = SKIN:GetVariable('Devices')
    local Status = tonumber(SKIN:GetVariable('Status'))

    if ((OldDevicesString ~= DevicesString) and (DevicesString ~= "") and (Status ~= 0)) then
        SKIN:Bang("!SetVariable", "Devices", DevicesString)
        SKIN:Bang("!WriteKeyValue", "Variables", "Devices", string.format('%s', DevicesString), "Example.ini")

        _Populate(1, 2)

        SKIN:Bang("!SetVariable", "PageNumber", 0)
        SKIN:Bang("!SetVariable", "PageDevice1", 1)
        SKIN:Bang("!SetVariable", "PageDevice2", 2)

        local Devices = _DivideDevices(DevicesString)

        SKIN:Bang("!SetVariable", "DevicesNumber", #Devices)
        SKIN:Bang("!WriteKeyValue", "Variables", "DevicesNumber", string.format('%s', #Devices), "Example.ini")
    end
end

function PageDown()
    local PageNumber = tonumber(SKIN:GetVariable('PageNumber'))
    if (PageNumber > 0) then
        PageNumber = PageNumber - 1
        local PageDevice1 = PageNumber + 1
        local PageDevice2 = PageNumber + 2
        SKIN:Bang("!SetVariable", "PageNumber", PageNumber)
        SKIN:Bang("!SetVariable", "PageDevice1", PageDevice1)
        SKIN:Bang("!SetVariable", "PageDevice2", PageDevice2)

        _Populate(PageDevice1, PageDevice2)
    end
end

function PageUp()
    local PageNumber = tonumber(SKIN:GetVariable('PageNumber'))
    local DevicesNumber = SKIN:GetVariable('DevicesNumber')
    if (PageNumber < (tonumber(DevicesNumber) - 2)) then
        PageNumber = PageNumber + 1
        local PageDevice1 = PageNumber + 1
        local PageDevice2 = PageNumber + 2
        SKIN:Bang("!SetVariable", "PageNumber", PageNumber)
        SKIN:Bang("!SetVariable", "PageDevice1", PageDevice1)
        SKIN:Bang("!SetVariable", "PageDevice2", PageDevice2)

        _Populate(PageDevice1, PageDevice2)
    end
end

---- [Private functions] ----

function _Populate(PageDevice1, PageDevice2)
    local DevicesStr = SKIN:GetVariable('Devices')

    local Devices = _DivideDevices(DevicesStr)

    local Device1 = _DivideItems(Devices[PageDevice1])
    local Device1Name = Device1[1]
    SKIN:Bang("!SetVariable", "Device1Name", Device1Name)
    SKIN:Bang("!WriteKeyValue", "Variables", "Device1Name", string.format('%s', Device1Name), "Example.ini")
    local Device1Address = Device1[2]
    SKIN:Bang("!SetVariable", "Device1Address", Device1Address)
    SKIN:Bang("!WriteKeyValue", "Variables", "Device1Address", string.format('%s', Device1Address), "Example.ini")
    local Device1Id = Device1[3]
    SKIN:Bang("!SetVariable", "Device1Id", Device1Id)
    SKIN:Bang("!WriteKeyValue", "Variables", "Device1Id", string.format('%s', Device1Id), "Example.ini")
    local Device1Connected = Device1[4]
    SKIN:Bang("!SetVariable", "Device1Connected", Device1Connected)
    SKIN:Bang("!WriteKeyValue", "Variables", "Device1Connected", string.format('%s', Device1Connected), "Example.ini")
    local Device1Paired = Device1[5]
    SKIN:Bang("!SetVariable", "Device1Paired", Device1Paired)
    SKIN:Bang("!WriteKeyValue", "Variables", "Device1Paired", string.format('%s', Device1Paired), "Example.ini")
    local Device1CanPair = Device1[6]
    SKIN:Bang("!SetVariable", "Device1CanPair", Device1CanPair)
    SKIN:Bang("!WriteKeyValue", "Variables", "Device1CanPair", string.format('%s', Device1CanPair), "Example.ini")
    local Device1MajorCategory = Device1[7]
    SKIN:Bang("!SetVariable", "Device1MajorCategory", Device1MajorCategory)
    SKIN:Bang("!WriteKeyValue", "Variables", "Device1MajorCategory", string.format('%s', Device1MajorCategory), "Example.ini")
    local Device1MinorCategory = Device1[8]
    SKIN:Bang("!SetVariable", "Device1MinorCategory", Device1MinorCategory)
    SKIN:Bang("!WriteKeyValue", "Variables", "Device1MinorCategory", string.format('%s', Device1MinorCategory), "Example.ini")
    local Device1HasBatteryLevel = Device1[9]
    SKIN:Bang("!SetVariable", "Device1HasBatteryLevel", Device1HasBatteryLevel)
    SKIN:Bang("!WriteKeyValue", "Variables", "Device1HasBatteryLevel", string.format('%s', Device1HasBatteryLevel), "Example.ini")
    local Device1Battery = Device1[10]
    SKIN:Bang("!SetVariable", "Device1Battery", Device1Battery)
    SKIN:Bang("!WriteKeyValue", "Variables", "Device1Battery", string.format('%s', Device1Battery), "Example.ini")
    local Device1IsBLE = Device1[11]
    SKIN:Bang("!SetVariable", "Device1IsBLE", Device1IsBLE)
    SKIN:Bang("!WriteKeyValue", "Variables", "Device1IsBLE", string.format('%s', Device1IsBLE), "Example.ini")

    local Device2 = _DivideItems(Devices[PageDevice2])
    local Device2Name = Device2[1]
    SKIN:Bang("!SetVariable", "Device2Name", Device2Name)
    SKIN:Bang("!WriteKeyValue", "Variables", "Device2Name", string.format('%s', Device2Name), "Example.ini")
    local Device2Address = Device2[2]
    SKIN:Bang("!SetVariable", "Device2Address", Device2Address)
    SKIN:Bang("!WriteKeyValue", "Variables", "Device2Address", string.format('%s', Device2Address), "Example.ini")
    local Device2Id = Device2[3]
    SKIN:Bang("!SetVariable", "Device2Id", Device2Id)
    SKIN:Bang("!WriteKeyValue", "Variables", "Device2Id", string.format('%s', Device2Id), "Example.ini")
    local Device2Connected = Device2[4]
    SKIN:Bang("!SetVariable", "Device2Connected", Device2Connected)
    SKIN:Bang("!WriteKeyValue", "Variables", "Device2Connected", string.format('%s', Device2Connected), "Example.ini")
    local Device2Paired = Device2[5]
    SKIN:Bang("!SetVariable", "Device2Paired", Device2Paired)
    SKIN:Bang("!WriteKeyValue", "Variables", "Device2Paired", string.format('%s', Device2Paired), "Example.ini")
    local Device2CanPair = Device2[6]
    SKIN:Bang("!SetVariable", "Device2CanPair", Device2CanPair)
    SKIN:Bang("!WriteKeyValue", "Variables", "Device2CanPair", string.format('%s', Device2CanPair), "Example.ini")
    local Device2MajorCategory = Device2[7]
    SKIN:Bang("!SetVariable", "Device2MajorCategory", Device2MajorCategory)
    SKIN:Bang("!WriteKeyValue", "Variables", "Device2MajorCategory", string.format('%s', Device2MajorCategory), "Example.ini")
    local Device2MinorCategory = Device2[8]
    SKIN:Bang("!SetVariable", "Device2MinorCategory", Device2MinorCategory)
    SKIN:Bang("!WriteKeyValue", "Variables", "Device2MinorCategory", string.format('%s', Device2MinorCategory), "Example.ini")
    local Device2HasBatteryLevel = Device2[9]
    SKIN:Bang("!SetVariable", "Device2HasBatteryLevel", Device2HasBatteryLevel)
    SKIN:Bang("!WriteKeyValue", "Variables", "Device2HasBatteryLevel", string.format('%s', Device2HasBatteryLevel), "Example.ini")
    local Device2Battery = Device2[10]
    SKIN:Bang("!SetVariable", "Device2Battery", Device2Battery)
    SKIN:Bang("!WriteKeyValue", "Variables", "Device2Battery", string.format('%s', Device2Battery), "Example.ini")
    local Device2IsBLE = Device2[11]
    SKIN:Bang("!SetVariable", "Device2IsBLE", Device2IsBLE)
    SKIN:Bang("!WriteKeyValue", "Variables", "Device2IsBLE", string.format('%s', Device2IsBLE), "Example.ini")
end

function _DivideDevices(DevicesStr)
    local Devices = {}
    for section in DevicesStr:gmatch("[^;]+") do
        table.insert(Devices, section)
    end
    return Devices
end

function _DivideItems(DeviceStr)
    local Device = {}
    for section in DeviceStr:gmatch("[^|]+") do
        table.insert(Device, section)
    end
    return Device
end
