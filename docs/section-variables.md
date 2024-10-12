---
description: Possible section variables that can be used
---

# Section Variables

## \[\&BluetoothMeasure:BluetoothStatus()]

Returns the status of the Bluetooth adapter:

* <mark style="color:red;">`0`</mark> if the Bluetooth adapter is <mark style="color:red;">OFF</mark>
* <mark style="color:green;">`1`</mark> if the Bluetooth adapter is <mark style="color:green;">ON</mark>

## \[\&BluetoothMeasure:AvailableDevices()]

Returns the list of devices in a formatted string.

Every device is separated by `;`, and evey item of a device is separated by `|`.

It's useful for example to be used in a lua script as in the [example skin](https://github.com/66Bunz/Rainmeter-Bluetooth-Plugin/tree/main/Bluetooth-Example-Skin).

### Device Properties

<mark style="color:green;">`device_name|device_address|device_id|connected[0,1]|paired[0,1]|can_pair[0,1]major_category|minor_category|has_battery_level[0,1]|battery|is_ble[0,1];`</mark>

#### Device name

This is the name of the device, with any whitespace removed.

#### Device address

The address of the device. At the moment it is not used for anything, probably in the future it could be used for pairing/unpairing or connection/disconnection operations.

#### Device Id

The Id of the device. At the moment it is not used for anything, probably in the future it could be used for pairing/unpairing or connection/disconnection operations.

#### Connected

If the device is connected or not.

#### Paired

If the device is paired or not.

#### Can Pair

If it's possible to pair the device.

#### Major Category

The major category of the device. Can be useful to change the appearance of the device.

* For Bluetooth Classic devices: [learn.microsoft.com](https://learn.microsoft.com/en-us/uwp/api/windows.devices.bluetooth.bluetoothmajorclass?view=winrt-26100#fields)
* For Bluetooth LE devices: [learn.microsoft.com](https://learn.microsoft.com/en-us/uwp/api/windows.devices.bluetooth.bluetoothleappearancecategories?view=winrt-26100#properties)

#### Minor Category

The minor category of the device. Can be useful to change the appearance of the device.&#x20;

* For Bluetooth Classic devices: [learn.microsoft.com](https://learn.microsoft.com/en-us/uwp/api/windows.devices.bluetooth.bluetoothminorclass?view=winrt-26100#fields)
* For Bluetooth LE devices: [learn.micosoft.com](https://learn.microsoft.com/en-us/uwp/api/windows.devices.bluetooth.bluetoothleappearancesubcategories?view=winrt-26100#properties)

#### Has Battery Level

If the plugin was able to obtain the battery level of the device.

#### Battery

The percentage of the battery of the device if present.

#### Is BLE

If the device has Bluetooth Low Energy capabilities.
