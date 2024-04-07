#include "Plugin.h"
#include "Measure.h"

using namespace std;

Measure::Measure(void* _rm) :
	rm(_rm),
	skin(RmGetSkin(_rm)) {
}

Measure::~Measure() {
}

void Measure::Initialize() {
	outputPath = RmReadString(rm, L"OutputPath", L"", FALSE);
	devicesUpdatedAction = RmReadString(rm, L"DevicesUpdatedAction", L"", FALSE);
	devicesVariable = RmReadString(rm, L"DevicesVariable", L"", FALSE);
	variablesFile = RmReadString(rm, L"VariablesFile", L"", FALSE);
}

void Measure::Execute(LPCWSTR bang) {
	RmExecute(skin, RmReplaceVariables(rm, bang));
}