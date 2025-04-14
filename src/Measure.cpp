#include "Plugin.h"
#include "Measure.h"

using namespace std;

Measure::Measure(void* _rm) :
	rm(_rm),
	skin(RmGetSkin(_rm)) {}

Measure::~Measure() {}

void Measure::Initialize() {
	pluginRole = RmReadInt(rm, L"Role", 0);
	updateAction = RmReadString(rm, L"UpdateAction", L"", FALSE);
	outputFile = RmReadString(rm, L"OutputFile", L"", FALSE);
	thumbnailsFolder = RmReadString(rm, L"ThumbnailsFolder", L"", FALSE);
	//devicesVariable = RmReadString(rm, L"DevicesVariable", L"", FALSE);
	//variablesFile = RmReadString(rm, L"VariablesFile", L"", FALSE);
}

void Measure::Execute(LPCWSTR bang) {
	RmExecute(skin, RmReplaceVariables(rm, bang));
}
