#pragma once
#include <Windows.h>
#include "../rmapi/RainmeterAPI.h" 

#include <bluetoothapis.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>

#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Power.h>
#include <winrt/Windows.Devices.Radios.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.System.h>

using namespace std;
using namespace winrt;
using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace Windows::Devices::Bluetooth;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Power;
using namespace Windows::Devices::Radios;
using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage;

class Measure;

struct DeviceInfo {
	std::wstring name;
	uint64_t address;
	std::wstring id;
	bool connected;
	bool paired;
	bool canPair;
	std::wstring majorCategory;
	std::wstring minorCategory;
	bool hasBatteryLevel;
	uint8_t battery;
	bool isBluetoothLE;
};

wstring getBluetoothStatus();
void setBluetoothStatus(Measure* measure, RadioState targetState);
void disableBluetooth(Measure* measure);
void enableBluetooth(Measure* measure);
void toggleBluetooth(Measure* measure);
void updateBluetoothStatus(Measure* measure);

std::tuple<std::wstring, std::wstring> findBLCategory(BluetoothMajorClass majorClass, BluetoothMinorClass minorClass);
std::tuple<std::wstring, std::wstring> findBLECategory(uint16_t Category, uint16_t SubCategory);
std::tuple<bool, int8_t> fetchBatteryLevel(BluetoothLEDevice device);
void saveThumbnailInFolder(Measure* measure, winrt::hstring deviceId);
std::unordered_map<uint64_t, DeviceInfo> listBluetoothDevices(Measure* measure);
void updateDevices(Measure* measure);
