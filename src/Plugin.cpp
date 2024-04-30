#pragma comment(lib, "Bthprops.lib")

#include "Plugin.h"
#include "Measure.h"
#include <bluetoothapis.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Devices.Radios.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <set>
#include <mutex>
#include <string>
#include <thread>
#include <fstream>
#include <iostream>

using namespace std;
using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Devices::Radios;
using namespace Windows::Foundation::Collections;


HINSTANCE MODULE_INSTANCE;


// ------------ [Global Variables] ------------
#pragma region GlobalVariables

static wstring availableDevices;	// Formatted string of all devices ("device_name|connected[0,1]|Authenticated[0,1]|Remembered[0,1]|datetime_last_seen|datetime_last_used;")
static wstring devicesBuffer;		// Buffer to save devices during list updates to avoid getting back partial lists
static string fileBufferString;		// Buffer to save devices during file updates to avoid having an empty file between updates
static wstring bluetoothStatus;		// String to hold the status of the Bluetooth adapter
static std::mutex bufferMutex;

std::thread updateThread;

#pragma endregion


/*
* Entry point to Dll, run once at dll load
* Use it to store the dll instance, in case you need it for hooks and other stuff
*/
BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	LPVOID lpvReserved)  // reserved
{
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		MODULE_INSTANCE = hinstDLL;
		DisableThreadLibraryCalls(hinstDLL); // disable thread library calls, for performance improvement
	default:
		break;
	}

	return TRUE;
}


// ------------ [Plugin Functions] ------------
#pragma region PluginFunctions

/*
* Called once, at skin load or refresh
* Read any options that need to be constant here
*/
PLUGIN_EXPORT void Initialize(void** data, void* rm) {
	Measure* measure = new Measure(rm);
	*data = measure;
	measure->Initialize();
}

/*
* Called once after Initialize
* Called before every Update if DynamicVariables=1 is defined
* Read options that can require dynamic variables here
*/
PLUGIN_EXPORT void Reload(void* data, void* rm, double* maxValue) {
	Measure* measure = (Measure*)data;
}

/*
* Called at every measure update
* Update your values here
*/
PLUGIN_EXPORT double Update(void* data) {
	Measure* measure = (Measure*)data;
	if (measure->pluginType == 0) {			// Updates the Bluetooth status and Bluetooth devices
		RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Updating status & devices");
		updateDevices(measure);
		updateBluetoothStatus(measure);
	}
	else if (measure->pluginType == 1) {	// Only updates the Bluetooth status
		RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Updating status");
		updateBluetoothStatus(measure);
	}
	else if (measure->pluginType == 2) {	// Only updates the Bluetooth devices
		RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Updating devices");
		updateDevices(measure);
	}
	else {
		RmLogF(measure->rm, LOG_ERROR, L"[Bluetooth-Plugin] Invalid PluginType");
	}
	measure->Execute(measure->updateAction.c_str());

	if (measure->pluginType == 1) {
		double status = _wtof(bluetoothStatus.c_str());
		return status;
	}
	return 0.0;
}

/*
* Called every time a [MeasureThisPlugin] is resolved
* DO NOT do any lengthy operations here, use Update for that
* Should only be used if you want the string value to be different than the numeric value
*/
/*PLUGIN_EXPORT LPCWSTR GetString(void* data)
{
	Measure* measure = (Measure*)data;
	return nullptr;
}*/


PLUGIN_EXPORT void ExecuteBang(void* data, LPCWSTR args) {
	Measure* measure = (Measure*)data;
	if (measure->pluginType == 2) {
		if (_wcsicmp(args, L"UpdateDevices") == 0) {
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Updating devices");
			updateDevices(measure);
		}
		else {
			RmLogF(measure->rm, LOG_ERROR, L"[Bluetooth-Plugin] Invalid bang: %s for selected Plugin Type %s", args, measure->pluginType);
		}
	}
	else if (measure->pluginType == 1) {
		if (_wcsicmp(args, L"DisableBluetooth") == 0) {
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Disabling Bluetooth adapter");
			disableBluetooth(measure);
		}
		else if (_wcsicmp(args, L"EnableBluetooth") == 0) {
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Enabling Bluetooth adapter");
			enableBluetooth(measure);
		}
		else if (_wcsicmp(args, L"ToggleBluetooth") == 0) {
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Toggling Bluetooth adapter");
			toggleBluetooth(measure);
		}
		else if (_wcsicmp(args, L"UpdateBluetoothStatus") == 0) {
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Updating Bluetooth status");
			updateBluetoothStatus(measure);
		}
		else {
			RmLogF(measure->rm, LOG_ERROR, L"[Bluetooth-Plugin] Invalid bang: %s for selected Plugin Type %s", args, measure->pluginType);
		}
	}
	else if (measure->pluginType == 0) {
		if (_wcsicmp(args, L"DisableBluetooth") == 0) {
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Disabling Bluetooth adapter");
			disableBluetooth(measure);
		}
		else if (_wcsicmp(args, L"EnableBluetooth") == 0) {
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Enabling Bluetooth adapter");
			enableBluetooth(measure);
		}
		else if (_wcsicmp(args, L"ToggleBluetooth") == 0) {
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Toggling Bluetooth adapter");
			toggleBluetooth(measure);
		}
		else if (_wcsicmp(args, L"UpdateDevices") == 0) {
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Updating devices");
			updateDevices(measure);
		}
		else if (_wcsicmp(args, L"UpdateBluetoothStatus") == 0) {
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Updating Bluetooth status");
			updateBluetoothStatus(measure);
		}
		else {
			RmLogF(measure->rm, LOG_ERROR, L"[Bluetooth-Plugin] Invalid bang: %s", args);
		}
	}
	else {
		RmLogF(measure->rm, LOG_ERROR, L"[Bluetooth-Plugin] Invalid Plugin Type");
	}
}

/*
* Called once, at skin unload (a skin is unloaded when you Refresh it)
* Perform any necessary cleanups here
*/
PLUGIN_EXPORT void Finalize(void* data) {
	Measure* measure = (Measure*)data;

	RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Unloading");

	// Clean up resources
	delete measure;
	availableDevices.clear();
	devicesBuffer.clear();
	fileBufferString.clear();
	bluetoothStatus.clear();
}

#pragma endregion


// ------------ [Section Variables] ------------
#pragma region SectionVariables

/*
* Can be called as a section variable, to get the most updated list of devices
*/
PLUGIN_EXPORT LPCWSTR AvailableDevices(void* data, const int argc, WCHAR* argv[]) {
	return availableDevices.c_str();
}

/*
* Can be called as a section variable, to get the status of the Bluetooth adapter
*/
PLUGIN_EXPORT LPCWSTR BluetoothStatus(void* data, const int argc, WCHAR* argv[]) {
	Measure* measure = (Measure*)data;
	return bluetoothStatus.c_str();
}

#pragma endregion


// ------------ [Internal Functions] ------------
#pragma region InternalFunctions

/*
* Threaded function that updates the status of the Bluetooth adapter
*/
void updateBluetoothStatus(Measure* measure) {
	std::thread updateBluetoothStatusThread([measure]() {
		init_apartment();
		Radio::RequestAccessAsync().get();
		IVectorView<Radio> radios = Radio::GetRadiosAsync().get();
		for (Radio const& radio : radios) {
			if (radio.Kind() == RadioKind::Bluetooth) {
				if (radio.State() == RadioState::On) {
					bluetoothStatus = L"1";
				}
				else {
					bluetoothStatus = L"0";
				}
				RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Updated Bluetooth Status");
			}
		}
		});

	updateBluetoothStatusThread.detach();
}

/*
* Threaded function that updates the bluetooth devices seen in every bluetooth interface on the machine.
* It can get:
*  - device name
*  - if the device is connected
*  - if the device is authenticated
*  - if the device is remembered
*  - the device address
*  - when the device was last seen
*  - when the device was last used
*/
void updateDevices(Measure* measure) {
	updateThread = std::thread([measure]() {
		if (!bufferMutex.try_lock()) {
			RmLogF(measure->rm, LOG_ERROR, L"Another thread is already updating the devices");
			return;
		}

		devicesBuffer.clear();
		fileBufferString.clear();
		set<ULONGLONG> deviceAddresses; // To store unique device addresses
		set<wstring> deviceNames; // To store unique device names

		HANDLE hRadio;
		BLUETOOTH_FIND_RADIO_PARAMS btfrp = { sizeof(btfrp) };
		HBLUETOOTH_RADIO_FIND hFind = BluetoothFindFirstRadio(&btfrp, &hRadio);

		if (hFind != nullptr) {
			do {
				BLUETOOTH_RADIO_INFO radioInfo = { sizeof(BLUETOOTH_RADIO_INFO) };
				DWORD dwResult = BluetoothGetRadioInfo(hRadio, &radioInfo);
				if (dwResult == ERROR_SUCCESS) {
					BLUETOOTH_DEVICE_INFO_STRUCT deviceInfo = { sizeof(BLUETOOTH_DEVICE_INFO_STRUCT) };
					BLUETOOTH_DEVICE_SEARCH_PARAMS deviceSearchParams = { sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS) };
					deviceSearchParams.fReturnAuthenticated = TRUE;
					deviceSearchParams.fReturnRemembered = TRUE;
					deviceSearchParams.fReturnUnknown = TRUE;
					deviceSearchParams.fReturnConnected = TRUE;
					deviceSearchParams.fIssueInquiry = TRUE;
					deviceSearchParams.cTimeoutMultiplier = 4;
					deviceSearchParams.hRadio = hRadio;

					HBLUETOOTH_DEVICE_FIND hDeviceFind = BluetoothFindFirstDevice(&deviceSearchParams, &deviceInfo);
					if (hDeviceFind != nullptr) {
						do {
							if (deviceInfo.szName && deviceInfo.stLastUsed.wYear != 1601 && deviceAddresses.find(deviceInfo.Address.ullLong) == deviceAddresses.end() && deviceNames.find(deviceInfo.szName) == deviceNames.end()) { // Check to remove ghost devices and duplicate addresses
								deviceAddresses.insert(deviceInfo.Address.ullLong); // Add the device address to the set
								deviceNames.emplace(deviceInfo.szName); // Add the device name to the set

								wstring wDeviceName(deviceInfo.szName);
								wstring wDeviceConnected = to_wstring(deviceInfo.fConnected ? 1 : 0);
								wstring wDeviceAuthenticated = to_wstring(deviceInfo.fAuthenticated ? 1 : 0);
								wstring wDeviceRemembered = to_wstring(deviceInfo.fRemembered ? 1 : 0);
								wstring wDeviceAddress = to_wstring(deviceInfo.Address.ullLong);
								wstring wDeviceLastSeen = to_wstring(deviceInfo.stLastSeen.wMonth) + L"/" + to_wstring(deviceInfo.stLastSeen.wDay) + L"/" + to_wstring(deviceInfo.stLastSeen.wYear) + L" " + to_wstring(deviceInfo.stLastSeen.wHour) + L":" + to_wstring(deviceInfo.stLastSeen.wMinute) + L":" + to_wstring(deviceInfo.stLastSeen.wSecond);
								wstring wDeviceLastUsed = to_wstring(deviceInfo.stLastUsed.wMonth) + L"/" + to_wstring(deviceInfo.stLastUsed.wDay) + L"/" + to_wstring(deviceInfo.stLastUsed.wYear) + L" " + to_wstring(deviceInfo.stLastUsed.wHour) + L":" + to_wstring(deviceInfo.stLastUsed.wMinute) + L":" + to_wstring(deviceInfo.stLastUsed.wSecond);
								wstring wDivider = L"|";

								string deviceName(wDeviceName.begin(), wDeviceName.end());
								string DeviceConnected = to_string(deviceInfo.fConnected ? 1 : 0);
								string DeviceAuthenticated = to_string(deviceInfo.fAuthenticated ? 1 : 0);
								string DeviceRemembered = to_string(deviceInfo.fRemembered ? 1 : 0);
								string DeviceAddress = to_string(deviceInfo.Address.ullLong);
								string deviceLastSeen = to_string(deviceInfo.stLastSeen.wMonth) + "/" + to_string(deviceInfo.stLastSeen.wDay) + "/" + to_string(deviceInfo.stLastSeen.wYear) + " " + to_string(deviceInfo.stLastSeen.wHour) + ":" + to_string(deviceInfo.stLastSeen.wMinute) + ":" + to_string(deviceInfo.stLastSeen.wSecond);
								string deviceLastUsed = to_string(deviceInfo.stLastUsed.wMonth) + "/" + to_string(deviceInfo.stLastUsed.wDay) + "/" + to_string(deviceInfo.stLastUsed.wYear) + " " + to_string(deviceInfo.stLastUsed.wHour) + ":" + to_string(deviceInfo.stLastUsed.wMinute) + ":" + to_string(deviceInfo.stLastUsed.wSecond);
								string divider = "|";

								RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Found device: %s", wDeviceName.c_str());

								devicesBuffer.append(
									wDeviceName + wDivider +
									wDeviceConnected + wDivider +
									wDeviceAuthenticated + wDivider +
									wDeviceRemembered + wDivider +
									wDeviceAddress + wDivider +
									wDeviceLastSeen + wDivider +
									wDeviceLastUsed +
									L";"
								);

								fileBufferString.append(
									deviceName + divider +
									DeviceConnected + divider +
									DeviceAuthenticated + divider +
									DeviceRemembered + divider +
									DeviceAddress + divider +
									deviceLastSeen + divider +
									deviceLastUsed +
									";\n"
								);
							}
						} while (BluetoothFindNextDevice(hDeviceFind, &deviceInfo));
						BluetoothFindDeviceClose(hDeviceFind);
					}
				}
				CloseHandle(hRadio);
			} while (BluetoothFindNextRadio(hFind, &hRadio));
			BluetoothFindRadioClose(hFind);
		}
		availableDevices = devicesBuffer; // Set the availableDevices string to the content of the buffer to avoid partial lists

		bufferMutex.unlock(); // Unlock the mutex

		// Write to file if the "OutputPath" field is populated
		if (measure->outputPath != L"") {
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Writing to file");
			ofstream outputFile(measure->outputPath, ofstream::trunc);
			outputFile << fileBufferString;
			outputFile.close();
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] File written");
		}

		// Update variable of the skin if the "DevicesVariable" and "VariablesFiles" fields are populated
		if (measure->devicesVariable != L"" && measure->variablesFile != L"") {
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Updating DevicesVariable");
			wstring setVariableBang = L"[!SetVariable " + measure->devicesVariable + L" \"" + availableDevices.c_str() + L"\"]";
			RmExecute(measure->skin, setVariableBang.c_str());
			wstring writeVariableBang = L"[!WriteKeyValue Variables " + measure->devicesVariable + L" \"" + availableDevices.c_str() + L"\" \"" + measure->variablesFile + L"\"]";
			RmExecute(measure->skin, writeVariableBang.c_str());
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] DevicesVariable updated");
		}
		});

	updateThread.detach();
}

/*
* Function to disable the Bluetooth adapter
*/
void disableBluetooth(Measure* measure) {
	std::thread disableBluetoothThread([measure]() {
		init_apartment();
		Radio::RequestAccessAsync().get();
		IVectorView<Radio> radios = Radio::GetRadiosAsync().get();
		for (Radio const& radio : radios) {
			if (radio.Kind() == RadioKind::Bluetooth) {
				radio.SetStateAsync(RadioState::Off).get();
				RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Disabled Bluetooth adapter");
				bluetoothStatus = L"0";
				break;
			}
		}
		});

	disableBluetoothThread.detach();
}

/*
* Function to enable the Bluetooth adapter
*/
void enableBluetooth(Measure* measure) {
	std::thread enableBluetoothThread([measure]() {
		init_apartment();
		Radio::RequestAccessAsync().get();
		IVectorView<Radio> radios = Radio::GetRadiosAsync().get();
		for (Radio const& radio : radios) {
			if (radio.Kind() == RadioKind::Bluetooth) {
				radio.SetStateAsync(RadioState::On).get();
				RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Enabled Bluetooth adapter");
				bluetoothStatus = L"1";
				break;
			}
		}
		});

	enableBluetoothThread.detach();
}

/*
* Function to toggle ON/OFF the Bluetooth adapter
*/
void toggleBluetooth(Measure* measure) {
	std::thread toggleBluetoothThread([measure]() {
		init_apartment();
		Radio::RequestAccessAsync().get();
		IVectorView<Radio> radios = Radio::GetRadiosAsync().get();
		for (Radio const& radio : radios) {
			if (radio.Kind() == RadioKind::Bluetooth) {
				RadioState currentState = radio.State();
				if (currentState == RadioState::On) {
					radio.SetStateAsync(RadioState::Off).get();
					RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Disabled Bluetooth adapter");
					bluetoothStatus = L"0";
				}
				else if (currentState == RadioState::Off) {
					radio.SetStateAsync(RadioState::On).get();
					RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Enabled Bluetooth adapter");
					bluetoothStatus = L"1";
				}
				break;
			}
		}
		});

	toggleBluetoothThread.detach();
}

#pragma endregion
