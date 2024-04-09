---
description: Possible Bangs that can be used
---

# Bangs

## UpdateBluetoothStatus

Manually updates the status of the Bluetooth adapter. The status can be retrieved with [#and-bluetoothmeasure-bluetoothstatus](section-variables.md#and-bluetoothmeasure-bluetoothstatus "mention").

{% hint style="warning" %}
Available if the [#type](options.md#type "mention") is set to `1` or `0`.
{% endhint %}

{% code title="Example Bang" %}
```ini
[!CommandMeasure "BluetoothMeasure" "UpdateBluetoothStatus"]
```
{% endcode %}

## DisableBluetooth

Disables the Bluetooth adapter.

{% hint style="warning" %}
Available if the [#type](options.md#type "mention") is set to `2` or `0`.
{% endhint %}

{% code title="Example Bang" %}
```ini
[!CommandMeasure "BluetoothMeasure" "DisableBluetooth"]
```
{% endcode %}

## EnableBluetooth

Enables the Bluetooth adapter.

{% hint style="warning" %}
Available if the [#type](options.md#type "mention") is set to `2` or `0`.
{% endhint %}

{% code title="Example Bang" %}
```ini
[!CommandMeasure "BluetoothMeasure" "EnableBluetooth"]
```
{% endcode %}

## ToggleBluetooth

Toggles the Bluetooth adapter.

{% code title="Example Bang" %}
```ini
[!CommandMeasure "BluetoothMeasure" "ToggleBluetooth"]
```
{% endcode %}

## UpdateDevices

Manually starts the update of devices that are stored inside the plugin. They can be retrieved with [#and-bluetoothmeasure-availabledevices](section-variables.md#and-bluetoothmeasure-availabledevices "mention").

{% hint style="warning" %}
Available if the [#type](options.md#type "mention") is set to `2` or `0`.
{% endhint %}

{% code title="Example Bang" %}
```ini
[!CommandMeasure "BluetoothMeasure" "UpdateDevices"]
```
{% endcode %}
