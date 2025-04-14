#pragma once
#include <string>
#include <Windows.h>

using namespace std;

class Measure {
public:
	void* rm;
	void* skin;

	int pluginRole;
	std::wstring updateAction;
	std::wstring outputFile;
	std::wstring thumbnailsFolder;
	//std::wstring devicesVariable;
	//std::wstring variablesFile;

	Measure(void* _rm);
	~Measure();

	void Initialize();
	void Execute(LPCWSTR bang);
};
