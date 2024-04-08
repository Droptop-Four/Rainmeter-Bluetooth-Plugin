---
description: Possible bangs to be used
---

# Bangs

## AvailableDevices()

This bang returns the list of devices in a string.

{% hint style="info" %}
Example:\
`device_name|connected[0,1]|Authenticated[0,1]|Remembered[0,1]|datetime_last_seen|datetime_last_used;`
{% endhint %}

Every device is separated by `;`, and evey item of a device is separated by `|`.

It's useful for example to be used in a lua script as in the [example skin](https://github.com/66Bunz/Rainmeter-Bluetooth-Plugin/tree/main/Bluetooth-Example-Skin).

