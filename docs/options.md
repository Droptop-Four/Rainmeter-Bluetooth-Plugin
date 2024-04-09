---
description: Possible options that can be used in the plugin measure
---

# Options

## Type

The type of operations the plugin will perform when its measure is updated.

{% tabs %}
{% tab title="Type=0" %}
{% hint style="info" %}
This is the DEFAULT `Type`
{% endhint %}

**Description:** `Get/Update the Bluetooth status and Bluetooth devices`

**Plugin Measure:** `Returns 0`

**Available Bangs:**&#x20;

* [#updatebluetoothstatus](bangs.md#updatebluetoothstatus "mention")
* [#disablebluetooth](bangs.md#disablebluetooth "mention")
* [#enablebluetooth](bangs.md#enablebluetooth "mention")
* [#togglebluetooth](bangs.md#togglebluetooth "mention")
* [#updatedevices](bangs.md#updatedevices "mention")
{% endtab %}

{% tab title="Type=1" %}
**Description:** `Get/Update the Bluetooth status.`

**Plugin Measure:** Returns the Bluetooth Status

**Available Bangs:**&#x20;

* [#updatebluetoothstatus](bangs.md#updatebluetoothstatus "mention")
{% endtab %}

{% tab title="Type=2" %}
**Description:** `Get/Update the Bluetooth devices`

**Plugin Measure:** `Returns 0`

**Available Bangs:**&#x20;

* [#disablebluetooth](bangs.md#disablebluetooth "mention")
* [#enablebluetooth](bangs.md#enablebluetooth "mention")
* [#togglebluetooth](bangs.md#togglebluetooth "mention")
* [#updatedevices](bangs.md#updatedevices "mention")
{% endtab %}
{% endtabs %}

{% code title="Example" lineNumbers="true" %}
```ini
[BluetoothMeasure]
Measure=Plugin
Plugin=Bluetooth
Type=[0/1/2]
```
{% endcode %}

## UpdateAction

The list of bangs that needs to be executed when the measure is updated.

{% code title="Example" lineNumbers="true" %}
```ini
[BluetoothMeasure]
Measure=Plugin
Plugin=Bluetooth
UpdateAction=[!Log "Bang1" Debug][!Log "Bang2" Debug]
```
{% endcode %}

## OutputPath

The path of an output file you want to set.

The file will have this structure:

{% code title="@Resources/output.txt" lineNumbers="true" fullWidth="false" %}
```
device_name|connected[0,1]|authenticated[0,1]|remembered[0,1]|datetime_last_seen|datetime_last_used;
device_name|connected[0,1]|authenticated[0,1]|remembered[0,1]|datetime_last_seen|datetime_last_used;
...
```
{% endcode %}

{% code title="Example" lineNumbers="true" %}
```ini
[BluetoothMeasure]
Measure=Plugin
Plugin=Bluetooth
OutputPath=#@#output.txt
```
{% endcode %}

This example saves the list of devices in a file in `@Resources/output.txt`.

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

{% code title="Example" lineNumbers="true" %}
```ini
[Variables]
Variable1=device_name|connected[0,1]|Authenticated[0,1]|Remembered[0,1]|datetime_last_seen|datetime_last_used;...

[BluetoothMeasure]
Measure=Plugin
Plugin=Bluetooth
DevicesVariable=Variable1
VariablesFile=#CURRENTFILE#
```
{% endcode %}

