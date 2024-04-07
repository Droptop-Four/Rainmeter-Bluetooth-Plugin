#include "Plugin.h"
#include "Measure.h"
#include <bluetoothapis.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>


using namespace std;


HINSTANCE MODULE_INSTANCE;

static wstring availableDevices;	// Formatted string of all devices ("device_name|connected[0,1]|Authenticated[0,1]|Remembered[0,1]|datetime_last_seen|datetime_last_used;")
static wstring devicesBuffer;		// Buffer to save devices during list updates to avoid getting back partial lists
static string fileBufferString;		// Buffer to save devices during file updates to avoid having an empty file between updates

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
		//DisableThreadLibraryCalls(hinstDLL); // disable thread library calls, for performance improvement
	default:
		break;
	}

	return TRUE;
}

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
	RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Reloading");
	UpdateDevices(measure);
	measure->Execute(measure->devicesUpdatedAction.c_str());
}

/*
* Called at every measure update
* Update your values here
*/
PLUGIN_EXPORT double Update(void* data) {
	Measure* measure = (Measure*)data;
	RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Updating");
	UpdateDevices(measure);
	return 0.0;
}


/*
* Called everytime a [MeasureThisPlugin] is resolved
* DO NOT do any lengthy operations here, use Update for that
* Should only be used if you want the string value to be different than the numeric value
*/
/*PLUGIN_EXPORT LPCWSTR GetString(void* data)
{
	Measure* measure = (Measure*)data;
	return nullptr;
}*/


/*
* Called once, at skin unload (a skin is unloaded when you Refresh it)
* Perform any necessary cleanups here
*/
PLUGIN_EXPORT void Finalize(void* data) {
	Measure* measure = (Measure*)data;
	delete measure;
	availableDevices.clear();
	devicesBuffer.clear();
}


/*
* Can be called with a Bang, to get the most updated list of devices
*/
PLUGIN_EXPORT LPCWSTR AvailableDevices(void* data, const int argc, WCHAR* argv[]) {
	return availableDevices.c_str();
}


/*
* Threaded function that updates the bluetooth devices seen in every bluetooth interface on the machine.
* It can get:
*  - device name
*  - if the device is connected
*  - if the device is authenticated
*  - if the device is remembered
*  - when the device was last seen
*  - when the device was last used
*/
void UpdateDevices(Measure* measure) {
	std::thread updateThread([measure]() {

		devicesBuffer.clear();
		fileBufferString.clear();

		HANDLE hRadio;
		BLUETOOTH_FIND_RADIO_PARAMS btfrp = { sizeof(btfrp) };
		HBLUETOOTH_RADIO_FIND hFind = BluetoothFindFirstRadio(&btfrp, &hRadio);

		if (hFind != NULL) {
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
					if (hDeviceFind != NULL) {
						do {
							if (deviceInfo.szName && deviceInfo.stLastUsed.wYear != 1601) {		// Check to remove ghost devices
								wstring wDeviceName(deviceInfo.szName);
								wstring wDeviceConnected = to_wstring((deviceInfo.fConnected ? 1 : 0));
								wstring wDeviceAuthenticated = to_wstring((deviceInfo.fAuthenticated ? 1 : 0));
								wstring wDeviceRemembered = to_wstring((deviceInfo.fRemembered ? 1 : 0));
								wstring wDeviceLastSeen = to_wstring(deviceInfo.stLastSeen.wMonth) + L"/" + to_wstring(deviceInfo.stLastSeen.wDay) + L"/" + to_wstring(deviceInfo.stLastSeen.wYear) + L" " + to_wstring(deviceInfo.stLastSeen.wHour) + L":" + to_wstring(deviceInfo.stLastSeen.wMinute) + L":" + to_wstring(deviceInfo.stLastSeen.wSecond);
								wstring wDeviceLastUsed = to_wstring(deviceInfo.stLastUsed.wMonth) + L"/" + to_wstring(deviceInfo.stLastUsed.wDay) + L"/" + to_wstring(deviceInfo.stLastUsed.wYear) + L" " + to_wstring(deviceInfo.stLastUsed.wHour) + L":" + to_wstring(deviceInfo.stLastUsed.wMinute) + L":" + to_wstring(deviceInfo.stLastUsed.wSecond);
								wstring wDivider = L"|";

								string deviceName(wDeviceName.begin(), wDeviceName.end());
								string DeviceConnected = to_string((deviceInfo.fConnected ? 1 : 0));
								string DeviceAuthenticated = to_string((deviceInfo.fAuthenticated ? 1 : 0));
								string DeviceRemembered = to_string((deviceInfo.fRemembered ? 1 : 0));
								string deviceLastSeen = to_string(deviceInfo.stLastSeen.wMonth) + "/" + to_string(deviceInfo.stLastSeen.wDay) + "/" + to_string(deviceInfo.stLastSeen.wYear) + " " + to_string(deviceInfo.stLastSeen.wHour) + ":" + to_string(deviceInfo.stLastSeen.wMinute) + ":" + to_string(deviceInfo.stLastSeen.wSecond);
								string deviceLastUsed = to_string(deviceInfo.stLastUsed.wMonth) + "/" + to_string(deviceInfo.stLastUsed.wDay) + "/" + to_string(deviceInfo.stLastUsed.wYear) + " " + to_string(deviceInfo.stLastUsed.wHour) + ":" + to_string(deviceInfo.stLastUsed.wMinute) + ":" + to_string(deviceInfo.stLastUsed.wSecond);
								string divider = "|";

								RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Found device: %s", wDeviceName.c_str());

								devicesBuffer.append(
									wDeviceName + wDivider +
									wDeviceConnected + wDivider +
									wDeviceAuthenticated + wDivider +
									wDeviceRemembered + wDivider +
									wDeviceLastSeen + wDivider +
									wDeviceLastUsed +
									L";"
								);

								fileBufferString.append(
									deviceName + divider +
									DeviceConnected + divider +
									DeviceAuthenticated + divider +
									DeviceRemembered + divider +
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
		availableDevices = devicesBuffer;	// Set the availableDevices string to the content of the buffer to avoid partial lists

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
