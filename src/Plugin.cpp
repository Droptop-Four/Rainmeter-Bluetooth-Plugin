#include "Measure.h"
#include "Plugin.h"

using namespace std;
using namespace winrt;
using namespace Windows::Devices::Bluetooth;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Radios;
using namespace Windows::Foundation::Collections;
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage;

HINSTANCE MODULE_INSTANCE;

// ------------ [Global Variables] ------------
#pragma region GlobalVariables

std::unordered_map<uint64_t, DeviceInfo> deviceMap;
std::mutex bluetoothOperationMutex;

std::atomic<bool> isUpdateBluetoothStatusRunning = false;
std::atomic<bool> isSetBluetoothStatusRunning = false;
std::atomic<bool> isToggleBluetoothRunning = false;
std::atomic<bool> isUpdateDevicesRunning = false;

wstring availableDevices; // Formatted string of all devices ("device_name|device_address|device_id|connected[0,1]|paired[0,1]|can_pair[0,1]major_category|minor_category|has_battery_level[0,1]|battery|is_ble[0,1];")
wstring bluetoothStatus;  // String to hold the status of the Bluetooth adapter

#pragma endregion

// ------------ [Start Point] ------------
/**
 * Entry point to Dll, run once at dll load
 * Use it to store the dll instance, in case you need it for hooks and other stuff
 *
 * @param hinstDLL: Handle to the DLL module
 * @param fdwReason: Reason for calling function
 * @param lpvReserved: Reserved
 * @return TRUE if successful, FALSE otherwise
 */
BOOL WINAPI DllMain(
	HINSTANCE hinstDLL, // handle to DLL module
	DWORD fdwReason,	// reason for calling function
	LPVOID lpvReserved) // reserved
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

/**
 * Called once, at skin load or refresh
 * Read any options that need to be constant here
 *
 * @param data: Pointer to the Measure object
 * @param rm: Pointer to the Rainmeter object
 */
PLUGIN_EXPORT void Initialize(void** data, void* rm) {
	Measure* measure = new Measure(rm);
	*data = measure;
	measure->Initialize();
}

/**
 * Called once after Initialize
 * Called before every Update if DynamicVariables=1 is defined
 * Read options that can require dynamic variables here
 *
 * @param data: Pointer to the Measure object
 * @param rm: Pointer to the Rainmeter object
 * @param maxValue: Pointer to the maximum value of the measure
 */
PLUGIN_EXPORT void Reload(void* data, void* rm, double* maxValue) {
	Measure* measure = (Measure*)data;
}

/**
 * Called at every measure update
 * Update your values here
 *
 * @param data: Pointer to the Measure object
 * @return The numeric value of the measure
 */
PLUGIN_EXPORT double Update(void* data) {
	auto measure = static_cast<Measure*>(data);
	switch (measure->pluginRole) {
	case 0: // Updates the Bluetooth status and Bluetooth devices
		RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Updating status & devices");
		updateDevices(measure);
		updateBluetoothStatus(measure);
		break;
	case 1: // Only updates the Bluetooth status
		RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Updating status");
		updateBluetoothStatus(measure);
		break;
	case 2: // Only updates the Bluetooth devices
		RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Updating devices");
		updateDevices(measure);
		break;
	default:
		RmLogF(measure->rm, LOG_ERROR, L"[Bluetooth-Plugin] Invalid plugin Role");
		break;
	}
	measure->Execute(measure->updateAction.c_str());

	if (measure->pluginRole == 1) {
		return _wtof(bluetoothStatus.c_str());
	}
	return 0.0;
}

/**
 * Called every time a [MeasureThisPlugin] is resolved
 * DO NOT do any lengthy operations here, use Update for that
 * Should only be used if you want the string value to be different than the numeric value
 *
 * @param data: Pointer to the Measure object
 * @return The string value of the measure
 */
 /*PLUGIN_EXPORT LPCWSTR GetString(void* data)
 {
	 Measure* measure = (Measure*)data;
	 return nullptr;
 }*/

 /**
  * Executes a bang command depending on the plugin role.
  *
  * @param data: Pointer to the Measure object
  * @param args: Arguments passed to the bang command
  */
PLUGIN_EXPORT void ExecuteBang(void* data, LPCWSTR args) {
	Measure* measure = (Measure*)data;
	if (measure->pluginRole == 2) {
		if (_wcsicmp(args, L"UpdateDevices") == 0) {
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Updating devices");
			updateDevices(measure);
		} else {
			RmLogF(measure->rm, LOG_ERROR, L"[Bluetooth-Plugin] Invalid bang: %s for selected Plugin Role %s", args, measure->pluginRole);
		}
	} else if (measure->pluginRole == 1) {
		if (_wcsicmp(args, L"DisableBluetooth") == 0) {
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Disabling Bluetooth adapter");
			disableBluetooth(measure);
		} else if (_wcsicmp(args, L"EnableBluetooth") == 0) {
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Enabling Bluetooth adapter");
			enableBluetooth(measure);
		} else if (_wcsicmp(args, L"ToggleBluetooth") == 0) {
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Toggling Bluetooth adapter");
			toggleBluetooth(measure);
		} else if (_wcsicmp(args, L"UpdateBluetoothStatus") == 0) {
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Updating Bluetooth status");
			updateBluetoothStatus(measure);
		} else {
			RmLogF(measure->rm, LOG_ERROR, L"[Bluetooth-Plugin] Invalid bang: %s for selected Plugin Role %s", args, measure->pluginRole);
		}
	} else if (measure->pluginRole == 0) {
		if (_wcsicmp(args, L"DisableBluetooth") == 0) {
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Disabling Bluetooth adapter");
			disableBluetooth(measure);
		} else if (_wcsicmp(args, L"EnableBluetooth") == 0) {
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Enabling Bluetooth adapter");
			enableBluetooth(measure);
		} else if (_wcsicmp(args, L"ToggleBluetooth") == 0) {
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Toggling Bluetooth adapter");
			toggleBluetooth(measure);
		} else if (_wcsicmp(args, L"UpdateDevices") == 0) {
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Updating devices");
			updateDevices(measure);
		} else if (_wcsicmp(args, L"UpdateBluetoothStatus") == 0) {
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Updating Bluetooth status");
			updateBluetoothStatus(measure);
		} else {
			RmLogF(measure->rm, LOG_ERROR, L"[Bluetooth-Plugin] Invalid bang: %s", args);
		}
	} else {
		RmLogF(measure->rm, LOG_ERROR, L"[Bluetooth-Plugin] Invalid plugin Role");
	}
}

/**
 * Called once, at skin unload (a skin is unloaded when you Refresh it)
 * Perform any necessary cleanups here
 *
 * @param data: Pointer to the Measure object
 */
PLUGIN_EXPORT void Finalize(void* data) {
	Measure* measure = (Measure*)data;

	RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Unloading");

	// Clean up resources
	delete measure;
	availableDevices.clear();
	bluetoothStatus.clear();
	deviceMap.clear();
}

#pragma endregion

// ------------ [Section Variables] ------------
#pragma region SectionVariables

/**
 * Can be called as a section variable, to get the most updated list of devices
 *
 * @param data: Pointer to the Measure object
 * @param argc: Number of arguments
 * @param argv: Array of arguments
 * @return The list of devices as a formatted string. Each device is separated by ";" and the device details are separated by "|".
 */
PLUGIN_EXPORT LPCWSTR AvailableDevices(void* data, const int argc, WCHAR* argv[]) {
	return availableDevices.c_str();
}

/**
 * Can be called as a section variable, to get the status of the Bluetooth adapter
 *
 * @param data: Pointer to the Measure object
 * @param argc: Number of arguments
 * @param argv: Array of arguments
 * @return The status of the Bluetooth adapter as a string. "1" if the adapter is on, "0" if the adapter is off.
 */
PLUGIN_EXPORT LPCWSTR BluetoothStatus(void* data, const int argc, WCHAR* argv[]) {
	Measure* measure = (Measure*)data;
	return bluetoothStatus.c_str();
}

#pragma endregion

// ------------ [Internal Functions] ------------
#pragma region InternalFunctions

/**
 * Trims the whitespace from the beginning and end of a string.
 *
 * @param str: The string to be trimmed
 * @return The trimmed string
 */
std::wstring trim(const std::wstring& str) {
	try {
		if (str.empty())
			return str;

		size_t first = str.find_first_not_of(L" \t\n\r\f\v");
		size_t last = str.find_last_not_of(L" \t\n\r\f\v");

		if (first == std::wstring::npos) // str is all whitespace
			return L"";

		return str.substr(first, (last - first + 1));
	} catch (const std::exception& e) {
		RmLogF(nullptr, LOG_ERROR, L"[Bluetooth-Plugin] Exception in trim: %S", e.what());
	} catch (...) {
		RmLogF(nullptr, LOG_ERROR, L"[Bluetooth-Plugin] Unknown exception in trim");
	}
	return L"";
}

/**
 * Retrieves the status of the Bluetooth adapter.
 *
 * @return The status of the Bluetooth adapter as a string. "1" if the adapter is on, "0" if the adapter is off.
 */
wstring getBluetoothStatus() {
	try {
		init_apartment();
		Radio::RequestAccessAsync().get();
		auto radios = Radio::GetRadiosAsync().get();
		for (const auto& radio : radios) {
			if (radio.Kind() == RadioKind::Bluetooth) {
				return (radio.State() == RadioState::On) ? L"1" : L"0";
			}
		}
	} catch (const std::exception& e) {
		RmLogF(nullptr, LOG_ERROR, L"[Bluetooth-Plugin] Exception in getBluetoothStatus: %S", e.what());
	} catch (...) {
		RmLogF(nullptr, LOG_ERROR, L"[Bluetooth-Plugin] Unknown exception in getBluetoothStatus");
	}
	return L"0";
}

/**
 * Controls the Bluetooth adapter state.
 *
 * @param measure The Measure object.
 * @param targetState The target state of the Bluetooth adapter.
 */
void setBluetoothStatus(Measure* measure, RadioState targetState) {
	if (isSetBluetoothStatusRunning.exchange(true)) {
		RmLogF(measure->rm, LOG_WARNING, L"[Bluetooth-Plugin] setBluetoothStatus operation is already running. If you often see this warning, consider increasing the update divider");
		return;
	}
	std::thread setBluetoothStatusThread([measure, targetState]() {
		try {
			std::lock_guard<std::mutex> lock(bluetoothOperationMutex);
			init_apartment();
			Radio::RequestAccessAsync().get();
			IVectorView<Radio> radios = Radio::GetRadiosAsync().get();
			for (Radio const& radio : radios) {
				if (radio.Kind() == RadioKind::Bluetooth) {
					radio.SetStateAsync(targetState).get();
					RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Bluetooth adapter state changed");
					bluetoothStatus = (targetState == RadioState::On) ? L"1" : L"0";
					break;
				}
			}
		} catch (const std::exception& e) {
			RmLogF(nullptr, LOG_ERROR, L"[Bluetooth-Plugin] Exception in setBluetoothStatus: %S", e.what());
		} catch (...) {
			RmLogF(nullptr, LOG_ERROR, L"[Bluetooth-Plugin] Unknown exception in setBluetoothStatus");
		}
		isSetBluetoothStatusRunning = false; });

	setBluetoothStatusThread.detach();
}

/**
 * Function to disable the Bluetooth adapter
 *
 * @param measure: Pointer to the Measure object
 */
void disableBluetooth(Measure* measure) {
	setBluetoothStatus(measure, RadioState::Off);
}

/**
 * Function to enable the Bluetooth adapter
 *
 * @param measure: Pointer to the Measure object
 */
void enableBluetooth(Measure* measure) {
	setBluetoothStatus(measure, RadioState::On);
}

/**
 * Function to toggle ON/OFF the Bluetooth adapter
 *
 * @param measure: Pointer to the Measure object
 */
void toggleBluetooth(Measure* measure) {
	if (isToggleBluetoothRunning.exchange(true)) {
		RmLogF(measure->rm, LOG_WARNING, L"[Bluetooth-Plugin] toggleBluetooth operation is already running. If you often see this warning, consider increasing the update divider");
		return;
	}
	std::thread toggleBluetoothThread([measure]() {
		try {
			std::lock_guard<std::mutex> lock(bluetoothOperationMutex);
			Radio::RequestAccessAsync().get();
			IVectorView<Radio> radios = Radio::GetRadiosAsync().get();
			for (Radio const& radio : radios) {
				if (radio.Kind() == RadioKind::Bluetooth) {
					RadioState currentState = radio.State();
					RadioState targetState = (currentState == RadioState::On) ? RadioState::Off : RadioState::On;
					radio.SetStateAsync(targetState).get();
					RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Bluetooth adapter toggled");
					bluetoothStatus = (targetState == RadioState::On) ? L"1" : L"0";
					break;
				}
			}
		} catch (const std::exception& e) {
			RmLogF(nullptr, LOG_ERROR, L"[Bluetooth-Plugin] Exception in toggleBluetooth: %S", e.what());
		} catch (...) {
			RmLogF(nullptr, LOG_ERROR, L"[Bluetooth-Plugin] Unknown exception in toggleBluetooth");
		}
		isToggleBluetoothRunning = false; });

	toggleBluetoothThread.detach();
}

/**
 * Updates the Bluetooth status asynchronously.
 *
 * @param measure The Measure object.
 */
void updateBluetoothStatus(Measure* measure) {
	if (isUpdateBluetoothStatusRunning.exchange(true)) {
		RmLogF(measure->rm, LOG_WARNING, L"[Bluetooth-Plugin] updateBluetoothStatus operation is already running. If you often see this warning, consider increasing the update divider");
		return;
	}
	std::thread updateBluetoothStatusThread([measure]() {
		try {
			bluetoothStatus = getBluetoothStatus();
		} catch (const std::exception& e) {
			RmLogF(nullptr, LOG_ERROR, L"[Bluetooth-Plugin] Exception in updateBluetoothStatus: %S", e.what());
		} catch (...) {
			RmLogF(nullptr, LOG_ERROR, L"[Bluetooth-Plugin] Unknown exception in updateBluetoothStatus");
		}
		isUpdateBluetoothStatusRunning = false; });

	updateBluetoothStatusThread.detach();
}

/**
 * Retrieves the Bluetooth categories based on the major and minor class.
 *
 * @param majorClass The major class of the Bluetooth device.
 * @param minorClass The minor class of the Bluetooth device.
 * @return A tuple containing the major and minor category as strings.
 */
std::tuple<std::wstring, std::wstring> findBLCategory(BluetoothMajorClass majorClass, BluetoothMinorClass minorClass) {
	std::wstring Major = L"Unknown";
	std::wstring Minor = L"Unknown";

	switch (majorClass) {
	case BluetoothMajorClass::Miscellaneous:
		Major = L"Miscellaneous";
		switch (minorClass) {
		case BluetoothMinorClass::Uncategorized:
			Minor = L"Uncategorized";
			break;
		default:
			Minor = L"Unknown";
			break;
		}
		break;
	case BluetoothMajorClass::Computer:
		Major = L"Computer";
		switch (minorClass) {
		case BluetoothMinorClass::ComputerDesktop:
			Minor = L"Computer Desktop";
			break;
		case BluetoothMinorClass::ComputerHandheld:
			Minor = L"Computer Handheld";
			break;
		case BluetoothMinorClass::ComputerLaptop:
			Minor = L"Computer Laptop";
			break;
		case BluetoothMinorClass::ComputerPalmSize:
			Minor = L"Computer Palm Size";
			break;
		case BluetoothMinorClass::ComputerServer:
			Minor = L"Computer Server";
			break;
		case BluetoothMinorClass::ComputerTablet:
			Minor = L"Computer Tablet";
			break;
		case BluetoothMinorClass::ComputerWearable:
			Minor = L"Computer Wearable";
			break;
		case BluetoothMinorClass::Uncategorized:
			Minor = L"Uncategorized";
			break;
		default:
			Minor = L"Unknown";
			break;
		}
		break;
	case BluetoothMajorClass::Phone:
		Major = L"Phone";
		switch (minorClass) {
		case BluetoothMinorClass::PhoneCellular:
			Minor = L"Phone Cellular";
			break;
		case BluetoothMinorClass::PhoneCordless:
			Minor = L"Phone Cordless";
			break;
		case BluetoothMinorClass::PhoneIsdn:
			Minor = L"Phone ISDN";
			break;
		case BluetoothMinorClass::PhoneSmartPhone:
			Minor = L"Phone Smart Phone";
			break;
		case BluetoothMinorClass::PhoneWired:
			Minor = L"Phone Wired";
			break;
		case BluetoothMinorClass::Uncategorized:
			Minor = L"Uncategorized";
			break;
		default:
			Minor = L"Unknown";
			break;
		}
		break;
	case BluetoothMajorClass::NetworkAccessPoint:
		Major = L"Network Access Point";
		switch (minorClass) {
		case BluetoothMinorClass::NetworkFullyAvailable:
			Minor = L"Network Fully Available";
			break;
		case BluetoothMinorClass::NetworkNoServiceAvailable:
			Minor = L"Network No Service Available";
			break;
		case BluetoothMinorClass::NetworkUsed01To17Percent:
			Minor = L"Network Used 01 To 17 Percent";
			break;
		case BluetoothMinorClass::NetworkUsed17To33Percent:
			Minor = L"Network Used 17 To 33 Percent";
			break;
		case BluetoothMinorClass::NetworkUsed33To50Percent:
			Minor = L"Network Used 33 To 50 Percent";
			break;
		case BluetoothMinorClass::NetworkUsed50To67Percent:
			Minor = L"Network Used 50 To 67 Percent";
			break;
		case BluetoothMinorClass::NetworkUsed67To83Percent:
			Minor = L"Network Used 67 To 83 Percent";
			break;
		case BluetoothMinorClass::NetworkUsed83To99Percent:
			Minor = L"Network Used 83 To 99 Percent";
			break;
		default:
			Minor = L"Unknown";
			break;
		}
		break;
	case BluetoothMajorClass::AudioVideo:
		Major = L"Audio/Video";
		switch (minorClass) {
		case BluetoothMinorClass::AudioVideoCamcorder:
			Minor = L"Audio Video Camcorder";
			break;
		case BluetoothMinorClass::AudioVideoCarAudio:
			Minor = L"Audio Video Car Audio";
			break;
		case BluetoothMinorClass::AudioVideoGamingOrToy:
			Minor = L"Audio Video Gaming Or Toy";
			break;
		case BluetoothMinorClass::AudioVideoHandsFree:
			Minor = L"Audio Video Hands Free";
			break;
		case BluetoothMinorClass::AudioVideoHeadphones:
			Minor = L"Audio Video Headphones";
			break;
		case BluetoothMinorClass::AudioVideoHifiAudioDevice:
			Minor = L"Audio Video HiFi Audio Device";
			break;
		case BluetoothMinorClass::AudioVideoLoudspeaker:
			Minor = L"Audio Video Loudspeaker";
			break;
		case BluetoothMinorClass::AudioVideoMicrophone:
			Minor = L"Audio Video Microphone";
			break;
		case BluetoothMinorClass::AudioVideoPortableAudio:
			Minor = L"Audio Video Portable Audio";
			break;
		case BluetoothMinorClass::AudioVideoSetTopBox:
			Minor = L"Audio Video Set-Top Box";
			break;
		case BluetoothMinorClass::AudioVideoVcr:
			Minor = L"Audio Video VCR";
			break;
		case BluetoothMinorClass::AudioVideoVideoCamera:
			Minor = L"Audio Video Video Camera";
			break;
		case BluetoothMinorClass::AudioVideoVideoConferencing:
			Minor = L"Audio Video Video Conferencing";
			break;
		case BluetoothMinorClass::AudioVideoVideoDisplayAndLoudspeaker:
			Minor = L"Audio Video Video Display And Loudspeaker";
			break;
		case BluetoothMinorClass::AudioVideoVideoMonitor:
			Minor = L"Audio Video Video Monitor";
			break;
		case BluetoothMinorClass::AudioVideoWearableHeadset:
			Minor = L"Audio Video Wearable Headset";
			break;
		case BluetoothMinorClass::Uncategorized:
			Minor = L"Uncategorized";
			break;
		default:
			Minor = L"Unknown";
			break;
		}
		break;
	case BluetoothMajorClass::Peripheral:
		Major = L"Peripheral";
		switch (minorClass) {
		case BluetoothMinorClass::PeripheralCardReader:
			Minor = L"Peripheral Card Reader";
			break;
		case BluetoothMinorClass::PeripheralDigitalPen:
			Minor = L"Peripheral Digital Pen";
			break;
		case BluetoothMinorClass::PeripheralDigitizerTablet:
			Minor = L"Peripheral Digitizer Tablet";
			break;
		case BluetoothMinorClass::PeripheralGamepad:
			Minor = L"Peripheral Gamepad";
			break;
		case BluetoothMinorClass::PeripheralHandheldGesture:
			Minor = L"Peripheral Handheld Gesture";
			break;
		case BluetoothMinorClass::PeripheralHandheldScanner:
			Minor = L"Peripheral Handheld Scanner";
			break;
		case BluetoothMinorClass::PeripheralJoystick:
			Minor = L"Peripheral Joystick";
			break;
		case BluetoothMinorClass::PeripheralRemoteControl:
			Minor = L"Peripheral Remote Control";
			break;
		case BluetoothMinorClass::PeripheralSensing:
			Minor = L"Peripheral Sensing";
			break;
		case BluetoothMinorClass::Uncategorized:
			Minor = L"Uncategorized";
			break;
		default:
			Minor = L"Unknown";
			break;
		}
		break;
	case BluetoothMajorClass::Imaging:
		Major = L"Imaging";
		switch (minorClass) {
		case BluetoothMinorClass::Uncategorized:
			Minor = L"Uncategorized";
			break;
		default:
			Minor = L"Unknown";
			break;
		}
		break;
	case BluetoothMajorClass::Wearable:
		Major = L"Wearable";
		switch (minorClass) {
		case BluetoothMinorClass::WearableGlasses:
			Minor = L"Wearable Glasses";
			break;
		case BluetoothMinorClass::WearableHelmet:
			Minor = L"Wearable Helmet";
			break;
		case BluetoothMinorClass::WearableJacket:
			Minor = L"Wearable Jacket";
			break;
		case BluetoothMinorClass::WearablePager:
			Minor = L"Wearable Pager";
			break;
		case BluetoothMinorClass::WearableWristwatch:
			Minor = L"Wearable Wristwatch";
			break;
		case BluetoothMinorClass::Uncategorized:
			Minor = L"Uncategorized";
			break;
		default:
			Minor = L"Unknown";
			break;
		}
		break;
	case BluetoothMajorClass::Toy:
		Major = L"Toy";
		switch (minorClass) {
		case BluetoothMinorClass::ToyController:
			Minor = L"Toy Controller";
			break;
		case BluetoothMinorClass::ToyDoll:
			Minor = L"Toy Doll";
			break;
		case BluetoothMinorClass::ToyGame:
			Minor = L"Toy Game";
			break;
		case BluetoothMinorClass::ToyRobot:
			Minor = L"Toy Robot";
			break;
		case BluetoothMinorClass::ToyVehicle:
			Minor = L"Toy Vehicle";
			break;
		case BluetoothMinorClass::Uncategorized:
			Minor = L"Uncategorized";
			break;
		default:
			Minor = L"Unknown";
			break;
		}
		break;
	case BluetoothMajorClass::Health:
		Major = L"Health";
		switch (minorClass) {
		case BluetoothMinorClass::HealthAnkleProsthesis:
			Minor = L"Health Ankle Prosthesis";
			break;
		case BluetoothMinorClass::HealthBloodPressureMonitor:
			Minor = L"Health Blood Pressure Monitor";
			break;
		case BluetoothMinorClass::HealthBodyCompositionAnalyzer:
			Minor = L"Health Body Composition Analyzer";
			break;
		case BluetoothMinorClass::HealthGenericHealthManager:
			Minor = L"Health Generic Health Manager";
			break;
		case BluetoothMinorClass::HealthGlucoseMeter:
			Minor = L"Health Glucose Meter";
			break;
		case BluetoothMinorClass::HealthHealthDataDisplay:
			Minor = L"Health Health Data Display";
			break;
		case BluetoothMinorClass::HealthHeartRateMonitor:
			Minor = L"Health Heart Rate Monitor";
			break;
		case BluetoothMinorClass::HealthKneeProsthesis:
			Minor = L"Health Knee Prosthesis";
			break;
		case BluetoothMinorClass::HealthMedicationMonitor:
			Minor = L"Health Medication Monitor";
			break;
		case BluetoothMinorClass::HealthPeakFlowMonitor:
			Minor = L"Health Peak Flow Monitor";
			break;
		case BluetoothMinorClass::HealthPersonalMobilityDevice:
			Minor = L"Health Personal Mobility Device";
			break;
		case BluetoothMinorClass::HealthPulseOximeter:
			Minor = L"Health Pulse Oximeter";
			break;
		case BluetoothMinorClass::HealthStepCounter:
			Minor = L"Health Step Counter";
			break;
		case BluetoothMinorClass::HealthThermometer:
			Minor = L"Health Thermometer";
			break;
		case BluetoothMinorClass::HealthWeighingScale:
			Minor = L"Health Weighing Scale";
			break;
		case BluetoothMinorClass::Uncategorized:
			Minor = L"Uncategorized";
			break;
		default:
			Minor = L"Unknown";
			break;
		}
		break;
	default:
		Major = L"Unknown";
		Minor = L"Unknown";
		break;
	}
	return { Major, Minor };
}

/**
 * Returns the BluetoothLE categories based on the appearance category and subcategory.
 *
 * @param Category The appearance category of the BLE device.
 * @param SubCategory The appearance subcategory of the BLE device.
 * @return A tuple containing the major and minor category strings.
 */
std::tuple<std::wstring, std::wstring> findBLECategory(uint16_t Category, uint16_t SubCategory) {
	std::wstring Major = L"Unknown";
	std::wstring Minor = L"Unknown";

	if (Category == BluetoothLEAppearanceCategories::Uncategorized()) {
		Major = L"Unknown";
	}
	if (Category == BluetoothLEAppearanceCategories::Phone()) {
		Major = L"Phone";
	}
	if (Category == BluetoothLEAppearanceCategories::Computer()) {
		Major = L"Computer";
	}
	if (Category == BluetoothLEAppearanceCategories::Watch()) {
		Major = L"Watch";
	}
	if (Category == BluetoothLEAppearanceCategories::Clock()) {
		Major = L"Clock";
	}
	if (Category == BluetoothLEAppearanceCategories::Display()) {
		Major = L"Display";
	}
	if (Category == BluetoothLEAppearanceCategories::RemoteControl()) {
		Major = L"RemoteControl";
	}
	if (Category == BluetoothLEAppearanceCategories::EyeGlasses()) {
		Major = L"EyeGlasses";
	}
	if (Category == BluetoothLEAppearanceCategories::Tag()) {
		Major = L"Tag";
	}
	if (Category == BluetoothLEAppearanceCategories::Keyring()) {
		Major = L"Keyring";
	}
	if (Category == BluetoothLEAppearanceCategories::MediaPlayer()) {
		Major = L"MediaPlayer";
	}
	if (Category == BluetoothLEAppearanceCategories::BarcodeScanner()) {
		Major = L"BarcodeScanner";
	}
	if (Category == BluetoothLEAppearanceCategories::Thermometer()) {
		Major = L"Thermometer";
	}
	if (Category == BluetoothLEAppearanceCategories::HeartRate()) {
		if (SubCategory == BluetoothLEAppearanceSubcategories::HeartRateBelt()) {
			Minor = L"HeartRateBelt";
		}
		Major = L"HeartRate";
	}
	if (Category == BluetoothLEAppearanceCategories::BloodPressure()) {
		if (SubCategory == BluetoothLEAppearanceSubcategories::BloodPressureArm()) {
			Minor = L"BloodPressureArm";
		} else if (SubCategory == BluetoothLEAppearanceSubcategories::BloodPressureWrist()) {
			Minor = L"BloodPressureWrist";
		}
		Major = L"BloodPressure";
	}
	if (Category == BluetoothLEAppearanceCategories::HumanInterfaceDevice()) {
		if (SubCategory == BluetoothLEAppearanceSubcategories::BarcodeScanner()) {
			Minor = L"BarcodeScanner";
		} else if (SubCategory == BluetoothLEAppearanceSubcategories::CardReader()) {
			Minor = L"CardReader";
		} else if (SubCategory == BluetoothLEAppearanceSubcategories::DigitalPen()) {
			Minor = L"DigitalPen";
		} else if (SubCategory == BluetoothLEAppearanceSubcategories::DigitizerTablet()) {
			Minor = L"DigitizerTablet";
		} else if (SubCategory == BluetoothLEAppearanceSubcategories::Gamepad()) {
			Minor = L"Gamepad";
		} else if (SubCategory == BluetoothLEAppearanceSubcategories::Joystick()) {
			Minor = L"Joystick";
		} else if (SubCategory == BluetoothLEAppearanceSubcategories::Keyboard()) {
			Minor = L"Keyboard";
		} else if (SubCategory == BluetoothLEAppearanceSubcategories::Mouse()) {
			Minor = L"Mouse";
		}
		Major = L"HumanInterfaceDevice";
	}
	if (Category == BluetoothLEAppearanceCategories::GlucoseMeter()) {
		Major = L"GlucoseMeter";
	}
	if (Category == BluetoothLEAppearanceCategories::RunningWalking()) {
		if (SubCategory == BluetoothLEAppearanceSubcategories::RunningWalkingInShoe()) {
			Minor = L"RunningWalkingInShoe";
		} else if (SubCategory == BluetoothLEAppearanceSubcategories::RunningWalkingOnHip()) {
			Minor = L"RunningWalkingOnHip";
		} else if (SubCategory == BluetoothLEAppearanceSubcategories::RunningWalkingOnShoe()) {
			Minor = L"RunningWalkingOnShoe";
		} else if (SubCategory == BluetoothLEAppearanceSubcategories::SportsWatch()) {
			Minor = L"SportsWatch";
		} else if (SubCategory == BluetoothLEAppearanceSubcategories::ThermometerEar()) {
			Minor = L"ThermometerEar";
		}
		Major = L"RunningWalking";
	}
	if (Category == BluetoothLEAppearanceCategories::Cycling()) {
		if (SubCategory == BluetoothLEAppearanceSubcategories::CyclingCadenceSensor()) {
			Minor = L"CyclingCadenceSensor";
		} else if (SubCategory == BluetoothLEAppearanceSubcategories::CyclingComputer()) {
			Minor = L"CyclingComputer";
		} else if (SubCategory == BluetoothLEAppearanceSubcategories::CyclingPowerSensor()) {
			Minor = L"CyclingPowerSensor";
		} else if (SubCategory == BluetoothLEAppearanceSubcategories::CyclingSpeedCadenceSensor()) {
			Minor = L"CyclingSpeedCadenceSensor";
		} else if (SubCategory == BluetoothLEAppearanceSubcategories::CyclingSpeedSensor()) {
			Minor = L"CyclingSpeedSensor";
		}
		Major = L"Cycling";
	}
	if (Category == BluetoothLEAppearanceCategories::PulseOximeter()) {
		if (SubCategory == BluetoothLEAppearanceSubcategories::OximeterFingertip()) {
			Minor = L"OximeterFingertip";
		} else if (SubCategory == BluetoothLEAppearanceSubcategories::OximeterWristWorn()) {
			Minor = L"OximeterWristWorn";
		}
		Major = L"PulseOximeter";
	}
	if (Category == BluetoothLEAppearanceCategories::WeightScale()) {
		Major = L"WeightScale";
	}
	if (Category == BluetoothLEAppearanceCategories::OutdoorSportActivity()) {
		if (SubCategory == BluetoothLEAppearanceSubcategories::LocationDisplay()) {
			Minor = L"LocationDisplay";
		} else if (SubCategory == BluetoothLEAppearanceSubcategories::LocationNavigationDisplay()) {
			Minor = L"LocationNavigationDisplay";
		} else if (SubCategory == BluetoothLEAppearanceSubcategories::LocationNavigationPod()) {
			Minor = L"LocationNavigationPod";
		} else if (SubCategory == BluetoothLEAppearanceSubcategories::LocationPod()) {
			Minor = L"LocationPod";
		}
		Major = L"OutdoorSportActivity";
	}

	return { Major, Minor };
}

/**
 * Fetches the battery level of a Bluetooth LE device.
 *
 * @param device The BluetoothLEDevice object representing the device.
 * @return A tuple containing a boolean indicating if the battery level was successfully fetched and an int8_t representing the battery level.
 */
std::tuple<bool, int8_t> fetchBatteryLevel(BluetoothLEDevice device) {
	try {
		GattDeviceServicesResult result = device.GetGattServicesAsync(BluetoothCacheMode::Uncached).get();
		if (result.Status() == GattCommunicationStatus::Success) {
			IVectorView<GattDeviceService> services = result.Services();
			for (GattDeviceService service : services) {
				auto uuid = service.Uuid();

				if (uuid == GattServiceUuids::Battery()) {
					GattCharacteristicsResult characteristics = service.GetCharacteristicsAsync(BluetoothCacheMode::Uncached).get();
					if (characteristics.Status() == GattCommunicationStatus::Success) {
						IVectorView<GattCharacteristic> chars = characteristics.Characteristics();
						for (GattCharacteristic chr : chars) {
							auto uuid = chr.Uuid();

							GattReadResult readRes = chr.ReadValueAsync(BluetoothCacheMode::Uncached).get();

							if (readRes.Status() == GattCommunicationStatus::Success) {
								DataReader reader = DataReader::FromBuffer(readRes.Value());
								try {
									uint8_t battery = reader.ReadByte();
									return { true, battery };
								} catch (const std::exception& e) {
									RmLogF(nullptr, LOG_ERROR, L"[Bluetooth-Plugin] Exception in fetchBatteryLevel (inner): %S", e.what());
								} catch (...) {
									RmLogF(nullptr, LOG_ERROR, L"[Bluetooth-Plugin] Unknown exception in fetchBatteryLevel (inner)");
								}
							}
						}
					}
				}
			}
		}
	} catch (const std::exception& e) {
		RmLogF(nullptr, LOG_ERROR, L"[Bluetooth-Plugin] Exception in fetchBatteryLevel: %S", e.what());
	} catch (...) {
		RmLogF(nullptr, LOG_ERROR, L"[Bluetooth-Plugin] Unknown exception in fetchBatteryLevel");
	}
	return { false, 0 };
}

/**
 * Saves the thumbnail of a device in a specified folder.
 *
 * @param deviceId The ID of the device.
 * @param folderPath The path of the folder where the thumbnail will be saved.
 */
void saveThumbnailInFolder(Measure* measure, winrt::hstring deviceId) {
	try {
		auto device = DeviceInformation::CreateFromIdAsync(deviceId).get();
		auto thumbnail = device.GetThumbnailAsync().get();

		uint32_t thumbnailSize = static_cast<uint32_t>(thumbnail.Size());
		auto buffer = winrt::Windows::Storage::Streams::Buffer(thumbnailSize);
		thumbnail.ReadAsync(buffer, thumbnailSize, InputStreamOptions::None).get();

		std::filesystem::path currentPath = std::filesystem::current_path();
		std::wstring fileName = trim(device.Name().c_str());
		fileName += L"_thumbnail.jpg";
		std::filesystem::path filePath = std::filesystem::path(measure->thumbnailsFolder) / fileName;

		std::ofstream file(filePath, std::ios::binary);
		if (file.is_open()) {
			auto reader = DataReader::FromBuffer(buffer);
			std::vector<uint8_t> data(reader.UnconsumedBufferLength());
			reader.ReadBytes(data);
			file.write(reinterpret_cast<const char*>(data.data()), data.size());
			file.close();
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Thumbnail saved: %s", filePath.c_str());
		} else {
			RmLogF(measure->rm, LOG_ERROR, L"[Bluetooth-Plugin] Error saving thumbnail: check if the path is correct and the folder exists.");
		}
	} catch (const std::exception& e) {
		RmLogF(nullptr, LOG_ERROR, L"[Bluetooth-Plugin] Exception in saveThumbnailInFolder: %S", e.what());
	} catch (...) {
		RmLogF(nullptr, LOG_ERROR, L"[Bluetooth-Plugin] Unknown exception in saveThumbnailInFolder");
	}
}

/**
 * Lists all Bluetooth devices and Bluetooth LE devices.
 *
 * @param measure The Measure object.
 * @return A map of known Bluetooth device addresses to DeviceInfo objects.
 */
std::unordered_map<uint64_t, DeviceInfo> listBluetoothDevices(Measure* measure) {
	try {
		init_apartment();

		deviceMap.clear();

		// Find all Bluetooth devices
		hstring selector = BluetoothDevice::GetDeviceSelector();
		auto devices = DeviceInformation::FindAllAsync(selector).get();

		for (const auto& device : devices) {
			auto bluetoothDevice = BluetoothDevice::FromIdAsync(device.Id()).get();
			uint64_t address = bluetoothDevice.BluetoothAddress();

			std::wstring name = device.Name().c_str();
			name = trim(name); // Remove leading and trailing whitespace

			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Found Bluetooth device: %s", name.c_str());

			// Check if the device is already in the map
			if (deviceMap.find(address) == deviceMap.end()) {
				// Device not found, add it to the map
				DeviceInfo info;
				info.name = name.c_str();
				info.address = address;
				info.id = bluetoothDevice.BluetoothDeviceId().Id().c_str();
				info.connected = bluetoothDevice.ConnectionStatus() == BluetoothConnectionStatus::Connected;
				info.paired = bluetoothDevice.DeviceInformation().Pairing().IsPaired();
				info.canPair = bluetoothDevice.DeviceInformation().Pairing().CanPair();
				auto [major, minor] = findBLCategory(bluetoothDevice.ClassOfDevice().MajorClass(), bluetoothDevice.ClassOfDevice().MinorClass());
				info.majorCategory = major;
				info.minorCategory = minor;
				info.hasBatteryLevel = false;
				info.battery = 0;
				info.isBluetoothLE = false;
				deviceMap[address] = info;
			}

			if (measure->thumbnailsFolder != L"") {
				saveThumbnailInFolder(measure, device.Id());
			}
		}

		// Find all Bluetooth LE devices
		hstring LEselector = BluetoothLEDevice::GetDeviceSelector();
		auto LEdevices = DeviceInformation::FindAllAsync(LEselector).get();

		for (const auto& device : LEdevices) {
			auto bluetoothLEDevice = BluetoothLEDevice::FromIdAsync(device.Id()).get();
			uint64_t address = bluetoothLEDevice.BluetoothAddress();

			// Update the line where the device name is assigned to remove leading and trailing whitespace
			std::wstring name = device.Name().c_str();
			name = trim(name); // Remove leading and trailing whitespace
			bool connected = bluetoothLEDevice.ConnectionStatus() == BluetoothConnectionStatus::Connected;

			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Found BluetoothLE device: %s", name.c_str());

			// Check if the device is already in the map
			if (deviceMap.find(address) == deviceMap.end()) {
				// Device not found, add it to the map
				DeviceInfo info;
				info.name = name.c_str();
				info.address = address;
				info.id = bluetoothLEDevice.BluetoothDeviceId().Id().c_str();
				info.connected = connected;
				info.paired = bluetoothLEDevice.DeviceInformation().Pairing().IsPaired();
				info.canPair = bluetoothLEDevice.DeviceInformation().Pairing().CanPair();
				auto [major, minor] = findBLECategory(bluetoothLEDevice.Appearance().Category(), bluetoothLEDevice.Appearance().SubCategory());
				info.majorCategory = major;
				info.minorCategory = minor;
				if (connected) {
					auto [hasBatteryLevel, battery] = fetchBatteryLevel(bluetoothLEDevice);
					info.hasBatteryLevel = hasBatteryLevel;
					info.battery = battery;
				} else {
					info.hasBatteryLevel = false;
					info.battery = 0;
				}
				info.isBluetoothLE = true;
				deviceMap[address] = info;
			} else {
				// Device found, update the battery level if available
				auto& info = deviceMap[address];
				bool connected = bluetoothLEDevice.ConnectionStatus() == BluetoothConnectionStatus::Connected;
				if (connected) {
					auto [hasBatteryLevel, battery] = fetchBatteryLevel(bluetoothLEDevice);
					info.hasBatteryLevel = hasBatteryLevel;
					info.battery = battery;
				} else {
					info.hasBatteryLevel = false;
					info.battery = 0;
				}
				info.isBluetoothLE = true;
			}

			if (measure->thumbnailsFolder != L"") {
				saveThumbnailInFolder(measure, device.Id());
			}
		}
	} catch (const std::exception& e) {
		RmLogF(nullptr, LOG_ERROR, L"[Bluetooth-Plugin] Exception in listBluetoothDevices: %S", e.what());
	} catch (...) {
		RmLogF(nullptr, LOG_ERROR, L"[Bluetooth-Plugin] Unknown exception in listBluetoothDevices");
	}
	return deviceMap;
}

/**
 * Updates the Bluetooth devices list.
 *
 * @param measure A pointer to the Measure object.
 */
void updateDevices(Measure* measure) {
	if (isUpdateDevicesRunning.exchange(true)) {
		RmLogF(measure->rm, LOG_WARNING, L"[Bluetooth-Plugin] updateDevices operation is already running. If you often see this warning, consider increasing the update divider If you often see this warning, consider increasing the update divider");
		return;
	}
	std::thread updateThread = std::thread([measure]() { // launch thread as lambda expression
		try {
			std::lock_guard<std::mutex> lock(bluetoothOperationMutex);

			auto status = getBluetoothStatus();
			if (status == L"0") {
				RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Cannot update devices if Bluetooth is disabled");
				return;
			}

			auto devicesMap = listBluetoothDevices(measure);
			std::wstringstream devicesStream;
			std::wstringstream fileStream;
			for (const auto& pair : devicesMap) {
				const DeviceInfo& device = pair.second;
				devicesStream << device.name << L"|"
					<< device.address << L"|"
					<< device.id << L"|"
					<< device.connected << L"|"
					<< device.paired << L"|"
					<< device.canPair << L"|"
					<< device.majorCategory << L"|"
					<< device.minorCategory << L"|"
					<< device.hasBatteryLevel << L"|"
					<< static_cast<int>(device.battery) << L"|"
					<< device.isBluetoothLE << L";";
				fileStream << device.name << L"|"
					<< device.address << L"|"
					<< device.id << L"|"
					<< device.connected << L"|"
					<< device.paired << L"|"
					<< device.canPair << L"|"
					<< device.majorCategory << L"|"
					<< device.minorCategory << L"|"
					<< device.hasBatteryLevel << L"|"
					<< static_cast<int>(device.battery) << L"|"
					<< device.isBluetoothLE << L";\n";
			}

			availableDevices = devicesStream.str(); // Set the availableDevices string to the formatted string

			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Updated devices");
			RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Available Devices: %s", availableDevices.c_str());

			// Write to file if the "OutputPath" field is populated
			if (measure->outputFile != L"") {
				RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Writing to file");
				std::wofstream outputFile(measure->outputFile, std::wofstream::trunc);
				outputFile << fileStream.str();
				outputFile.close();
				RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] File written");
			}

			// Update variable of the skin if the "DevicesVariable" and "VariablesFiles" fields are populated
			/*if (measure->devicesVariable != L"" && measure->variablesFile != L"") {
				RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] Updating DevicesVariable");
				wstring setVariableBang = L"[!SetVariable " + measure->devicesVariable + L" \"" + availableDevices.c_str() + L"\"]";
				RmExecute(measure->skin, setVariableBang.c_str());
				wstring writeVariableBang = L"[!WriteKeyValue Variables " + measure->devicesVariable + L" \"" + availableDevices.c_str() + L"\" \"" + measure->variablesFile + L"\"]";
				RmExecute(measure->skin, writeVariableBang.c_str());
				RmLogF(measure->rm, LOG_DEBUG, L"[Bluetooth-Plugin] DevicesVariable updated");
			}*/
		} catch (const std::exception& e) {
			RmLogF(nullptr, LOG_ERROR, L"[Bluetooth-Plugin] Exception in updateDevices: %S", e.what());
		} catch (...) {
			RmLogF(nullptr, LOG_ERROR, L"[Bluetooth-Plugin] Unknown exception in updateDevices");
		}
		isUpdateDevicesRunning = false;
		});

	updateThread.detach();
}

#pragma endregion