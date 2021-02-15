#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>

using namespace std;

constexpr char Separator = ',';
constexpr char EndLineChar = '\n';
constexpr char OnlineChar = '1';
constexpr char OfflineChar = '0';

class CSVFile
{
public:
	CSVFile(const string& name);
	~CSVFile();

	void AddLine(const string& line);
	const string& ReadLine();
	bool ReadAll(unordered_map<string, string>& online, unordered_map<string, string>& offline);
	void Flush() { FileCSV.flush(); }
	bool IsOpen() { return FileCSV.is_open(); }
private:
	fstream FileCSV;
	string Line;
};

