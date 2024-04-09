#pragma once
#include <Windows.h>
#include "../rmapi/RainmeterAPI.h" 

class Measure;

void updateBluetoothStatus(Measure* measure);
void updateDevices(Measure* measure);
void disableBluetooth(Measure* measure);
void enableBluetooth(Measure* measure);
void toggleBluetooth(Measure* measure);
