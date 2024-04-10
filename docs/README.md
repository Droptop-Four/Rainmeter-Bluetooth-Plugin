# Bluetooth Plugin for Rainmeter

This plugin is still a WIP (Work In Progress) and it doesn't include all the features that could be useful to have. If you want to see planned features, please check the [#features](./#features "mention") section. If you want to help, please check the [#contributing](./#contributing "mention") section.

## Installation

Download the latest example rmskin from the [releases](https://github.com/Droptop-Four/Rainmeter-Bluetooth-Plugin/releases) page and install it: the plugin will be present in the package.

Else, you can download the zip package from the latest [release](https://github.com/Droptop-Four/Rainmeter-Bluetooth-Plugin/releases) and extract the dll that corresponds to the Windows architecture you are using (x32 or x64) to `%appdata%\Rainmeter\Plugins`.

### Requirements

_TODO_

## Documentation

[https://droptop.gitbook.io/rainmeter-bluetooth-plugin](https://droptop.gitbook.io/rainmeter-bluetooth-plugin)

## Basic Usage

> **NOTE:** Since this plugin is still a WIP, the usage is subject to change.

The plugin at every update, is going to scan for Bluetooth devices, and update the list of available devices. The plugin is going to store the list of devices in a formatted string, that can be accessed using the `AvailableDevices()` function.

The formatted string is going to be in the following format:

```plaintext
device_name|connected[0,1]|authenticated[0,1]|remembered[0,1]|datetime_last_seen|datetime_last_used;
```

This is an example of how you can use the plugin in a skin:

```ini
[Bluetooth]
Measure=Plugin
Plugin=Bluetooth
UpdateDivider=10    ; Keep a relatively high value, to avoid spamming the plugin with update requests that cannot terminate
DevicesUpdatedAction=[!CommandMeasure LuaScript "Refresh()"]

[LuaScript]
Measure=Script
ScriptFile=Example.lua
UpdateDivider=-1
```

```lua
function Refresh()
    local DevicesString = SKIN:ReplaceVariables('[&BluetoothMeasure:AvailableDevices()]')
    -- DevicesString = Formatted string of all devices
    -- ("device_name|connected[0,1]|Authenticated[0,1]|Remembered[0,1]|datetime_last_seen|datetime_last_used;")

    -- Do something with the devices list
    -- See example skin for an example
end
```

For a complete example, check out the [example skin](https://github.com/Droptop-Four/Rainmeter-Bluetooth-Plugin/tree/main/Bluetooth-Eample-Skin).

## Features

Those are the features I'd like to implement/have in a bluetooth plugin:

* [x] Enable/Disable/Toggle Bluetooth adapter
* [x] Scan & list Bluetooth devices
* [x] Return the Bluetooth adapter status
* [ ] Scan & list BluetoothLE devices
* [ ] Display Device Battery
* [ ] Connect to devices
* [ ] Disconnect devices

## Contributing

If you want to help you are encouraged to fork the repo and do your changes, then open a pull request! <mark style="color:red;">**Any help is appreciated!**</mark>

If you want to suggest a feature or report a bug, please open an issue!

## License

This project is licensed under the GPL-3.0 License - see the [LICENSE](../LICENSE/) file for details.
