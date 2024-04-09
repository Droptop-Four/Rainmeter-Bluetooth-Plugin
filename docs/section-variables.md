---
description: Possible section variables that can be used
---

# Section Variables

## \[\&BluetoothMeasure:AvailableDevices()]

Returns the list of devices in a string.

`device_name|connected[0,1]|Authenticated[0,1]|Remembered[0,1]|datetime_last_seen|datetime_last_used;`

Every device is separated by `;`, and evey item of a device is separated by `|`.

It's useful for example to be used in a lua script as in the [example skin](https://github.com/66Bunz/Rainmeter-Bluetooth-Plugin/tree/main/Bluetooth-Example-Skin).

## \[\&BluetoothMeasure:BluetoothStatus()]

Returns the status of the Bluetooth adapter:

* <mark style="color:red;">`0`</mark> if the Bluetooth adapter is <mark style="color:red;">OFF</mark>
* <mark style="color:green;">`1`</mark> if the Bluetooth adapter is <mark style="color:green;">ON</mark>
