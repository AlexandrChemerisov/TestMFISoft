#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <atomic>
#include <chrono>
#include <sstream>

using namespace std;

typedef unordered_map<string, string> phone_map;
struct phone_data
{
	phone_map OnlineMap;
	phone_map OfflineMap;
	atomic_bool IsFileRead = false;
};

constexpr char Separator = ',';
constexpr char EndLineChar = '\n';
constexpr char OnlineChar = '1';
constexpr char OfflineChar = '0';
constexpr char FillChar = '0';
constexpr size_t WordSize = 8;
constexpr size_t LineSize = 25000000;
constexpr size_t WriteLineSize = 100;
constexpr size_t Mod = 2;

class CSVFile
{
public:
	CSVFile(const string& name = "");
	~CSVFile();

	bool OpenFile(const string& name);
	bool CreateNewFile(const string& file_name);
	void AddLine(const string& line);
	static void ReadAll(const string& name, shared_ptr<phone_data> data_maps);
	void Flush() { FileCSV.flush(); }
	bool IsOpen() { return FileCSV.is_open(); }
private:
	fstream FileCSV;
};

