#include "CSVFile.h"
#include <iostream>
#include <cstdio>
#include <vector>
#include <chrono>
#include <sstream>

CSVFile::CSVFile(const string& name)
{
	if (name.empty())
	{
		cout << "�� ������ ��� �����";
	}
	else
	{
		FileCSV.exceptions(std::ios::failbit | std::ios::badbit);
		FileCSV.open(name, std::fstream::in | std::fstream::out | std::fstream::app);
		if (!FileCSV.is_open())
		{
			cout << "�� ������� ������� ����";
		}
	}
}

CSVFile::~CSVFile()
{
	if (FileCSV.is_open())
	{
		FileCSV.flush();
		FileCSV.close();
	}
}

void CSVFile::AddLine(const string& line)
{
	if (FileCSV.is_open())
		FileCSV << line;
}

const string& CSVFile::ReadLine()
{
	if (FileCSV.is_open() && !FileCSV.eof())
	{
		// �� ��������� ����� �����
		FileCSV >> Line;
		return Line;
	}
	else
	{
		Line.clear();
		return Line;
	}
}

bool CSVFile::ReadAll(unordered_map<string, string>& online, unordered_map<string, string>& offline)
{
	if (FileCSV.is_open() && !FileCSV.eof())
	{
		cout << "������ ������ �����...\n";

		// �� ��������� ����� �����
		FileCSV.seekg(0, FileCSV.end);
		uint64_t file_size = FileCSV.tellg();
		FileCSV.seekg(0, FileCSV.beg);

		std::string str;
		str.reserve(file_size + 1);

		string s;
		std::stringstream ss;
		ss << FileCSV.rdbuf();

		// ������� ��������� �� ��������� �������� ������
		std::filebuf* fbuf = FileCSV.rdbuf();
		// ������� ������ �����/������� � ������
		std::size_t size = fbuf->pubseekoff(0, FileCSV.end, FileCSV.in);
		fbuf->pubseekpos(0, FileCSV.in);
		// ������� ������ ��� �������� ������ �����
		vector<char> buffer(size);
		// ������� ������ �� ����� � ������
		fbuf->sgetn(&buffer[0], size);
		
		size_t start = 0;
		int col_num = 0;
		string phone_number;
		string fio;
		size_t sz = 0;
		auto begin = std::chrono::steady_clock::now();
		for (size_t i = 0; i < size; ++i)
		{
			if (Separator == buffer[i])
			{
				// ����� �������
				sz = i - start;
				if (0 == col_num)
				{
					phone_number.assign(&buffer[start], sz);
				}
				else if (1 == col_num)
				{
					fio.assign(&buffer[start], sz);
				}
				start = ++i;
				++col_num;
			}
			else if (buffer[i] == EndLineChar)
			{
				// �������� ����� �� �������
				if (i < 1)
					continue;

				// ����� ������
				sz = i - start;
				if (OfflineChar == buffer[i - 1])
				{
					offline.insert(std::make_pair(phone_number, fio));
				}
				else
				{
					online.insert(std::make_pair(phone_number, fio));
				}
				start = ++i;
				col_num = 0;
			}

		}
		
		auto end = std::chrono::steady_clock::now();
		auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
		cout << "����� ������ ����� = " << elapsed_ms.count() << "ms" << EndLineChar;

		return true;
	}
	return false;
}

