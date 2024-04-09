#pragma once
#include <string>

using namespace std;
class Measure {
public:

	void* rm;
	void* skin;

	int pluginType;
	std::wstring updateAction;
	std::wstring outputPath;
	std::wstring devicesVariable;
	std::wstring variablesFile;

	Measure(void* _rm);
	~Measure();

	void Initialize();
	void Execute(LPCWSTR bang);
};
