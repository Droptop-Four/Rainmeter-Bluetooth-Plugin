---
description: Possible options that can be used in the plugin measure
---

# Options

## OutputPath

The path of an output file you want to set.

The file will have this structure:

{% code lineNumbers="true" fullWidth="false" %}
```
device_name|connected[0,1]|authenticated[0,1]|remembered[0,1]|datetime_last_seen|datetime_last_used;
device_name|connected[0,1]|authenticated[0,1]|remembered[0,1]|datetime_last_seen|datetime_last_used;
...
```
{% endcode %}

### Example

{% code lineNumbers="true" %}
```ini
[BluetoothMeasure]
Measure=Plugin
Plugin=Bluetooth
OutputPath=#@#output.txt
```
{% endcode %}

This example saves the list of devices in a file in `@Resources/output.txt`.

## DevicesUpdatedAction

The list of bangs that needs to be executed when the list of devices is updated.

### Example

```ini
[BluetoothMeasure]
Measure=Plugin
Plugin=Bluetooth
DevicesUpdatedAction=[!Log "Bang1" Debug][!Log "Bang2" Debug]
```

## DevicesVariable

{% hint style="warning" %}
If this is defined, also [#variablesfile](options.md#variablesfile "mention") needs to be defined
{% endhint %}

The name of the variable that you want the plugin to update when&#x20;

## VariablesFile

{% hint style="warning" %}
If this is defined, also [#devicesvariable](options.md#devicesvariable "mention") needs to be defined
{% endhint %}

The name of the file that contains the [#devicesvariable](options.md#devicesvariable "mention").

### Example

```ini
[Variables]
Variable1=device_name|connected[0,1]|Authenticated[0,1]|Remembered[0,1]|datetime_last_seen|datetime_last_used;...

[BluetoothMeasure]
Measure=Plugin
Plugin=Bluetooth
DevicesVariable=Variable1
VariablesFile=#CURRENTFILE#
```

