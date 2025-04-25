# Bluetooth Plugin for Rainmeter

{% hint style="info" %}
This plugin is still a WIP (Work In Progress) and it doesn't include all the features that could be useful to have. If you want to see planned features, please check the [#features](./#features "mention") section. If you want to help, please check the [#contributing](./#contributing "mention") section.
{% endhint %}

## Installation

Download the latest example rmskin from the [releases](https://github.com/Droptop-Four/Rainmeter-Bluetooth-Plugin/releases) page and install it: the plugin will be present in the package.

Else, you can download the zip package from the latest [release](https://github.com/Droptop-Four/Rainmeter-Bluetooth-Plugin/releases) and extract the dll that corresponds to the Windows architecture you are using (x32 or x64) to `%appdata%\Rainmeter\Plugins`.

### Minimum requirements

* Windows 10

## Basic Usage

> **NOTE:** Since this plugin is still a WIP, the usage is subject to change.

The plugin at every update, is going to scan for Bluetooth devices, and update the list of available devices. The plugin is going to store the list of devices in a formatted string, that can be accessed using the `AvailableDevices()` function.

The formatted string is going to be in the following format:

{% code overflow="wrap" %}
```plaintext
device_name|device_address|device_id|connected[0,1]|paired[0,1]|can_pair[0,1]major_category|minor_category|has_battery_level[0,1]|battery|is_ble[0,1];
```
{% endcode %}

This is an example of how you can use the plugin in a skin:

{% code overflow="wrap" %}
```ini
[Bluetooth]
Measure=Plugin
Plugin=Bluetooth
DevicesUpdatedAction=[!CommandMeasure LuaScript "Refresh()"]
UpdateDivider=30    ; Keep a relatively high value, to avoid spamming the plugin with update requests that cannot terminate

[LuaScript]
Measure=Script
ScriptFile=Example.lua
UpdateDivider=-1
```
{% endcode %}

{% code overflow="wrap" %}
```lua
function Refresh()
    local DevicesString = SKIN:ReplaceVariables('[&BluetoothMeasure:AvailableDevices()]')
    -- DevicesString = Formatted string of all devices
    -- ("device_name|device_address|device_id|connected[0,1]|paired[0,1]|can_pair[0,1]major_category|minor_category|has_battery_level[0,1]|battery|is_ble[0,1];")

    -- Do something with the devices list
    -- See example skin for an example
end
```
{% endcode %}

For a complete example, check out the [example skin](https://github.com/Droptop-Four/Rainmeter-Bluetooth-Plugin/tree/main/Bluetooth-Example-Skin).

## Features

Those are the features we'd like to implement/have in a bluetooth plugin:

* [x] Enable/Disable/Toggle Bluetooth adapter
* [x] List Bluetooth devices
* [ ] Discover Bluetooth devices
* [x] Return the Bluetooth status
* [x] List BluetoothLE devices
* [ ] Discover BluetoothLE devices
* [ ] Connect to devices
* [ ] Disconnect devices
* [ ] Pair devices
* [ ] Unpair devices
* [x] Get device thumbnail

### Device Properties

Those are the returned properties for every device:

* [x] Name
* [x] Address
* [x] Id
* [x] Connected
* [x] Paired
* [x] Can Pair
* [x] Major Category
* [x] Minor Category
* [x] Has Battery Level
* [x] Battery (it's possible that some devices' battery is not displayed)
* [x] Is Bluetooth LE
* [ ] Signal strength

To know more about these properties, read the [docs](section-variables.md#device-properties).

## Contributing

If you want to help you are encouraged to fork the repo and do your changes, then open a pull request! <mark style="color:red;">**Any help is appreciated!**</mark>

If you want to suggest a feature or report a bug, please open an issue!

## License

This project is licensed under the GPL-3.0 License - see the [LICENSE](../LICENSE/) file for details.
