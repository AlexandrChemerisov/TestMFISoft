#include "CSVFile.h"
#include <cstdio>
#include <vector>
#include <thread>
#include <random>

CSVFile::CSVFile(const string& name)
{
	if (!name.empty())
	{
		OpenFile(name);
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

bool CSVFile::OpenFile(const string& name)
{
	if (name.empty())
	{
		std::cout << "�� ������ ��� �����";
		return false;
	}
	else
	{
		FileCSV.exceptions(std::ios::failbit | std::ios::badbit);
		FileCSV.open(name, std::fstream::in | std::fstream::out | std::fstream::app);
		if (!FileCSV.is_open())
		{
			std::cout << "�� ������� ������� ����";
			return false;
		}
	}
	return true;
}

void CSVFile::AddLine(const string& line)
{
	if (FileCSV.is_open())
		FileCSV << line;
}

void CSVFile::ReadAll(const string& name, shared_ptr<phone_data> data_maps)
{
	thread loading_file_thread = thread([&](const string& name) {
		shared_ptr<fstream> file(make_shared<fstream>());
		file->exceptions(std::ios::failbit | std::ios::badbit);
		file->open(name, std::fstream::in | std::fstream::out | std::fstream::app);
		if (!file->is_open())
		{
			std::cout << "�� ������� ������� ���� �� ������ " << name;
			return;
		}
		std::cout << "������ ������ �����..." << endl;

		// ������� ��������� �� ��������� �������� ������
		std::filebuf* fbuf = file->rdbuf();
		// ������� ������ �����/������� � ������
		std::size_t size = fbuf->pubseekoff(0, file->end, file->in);
		fbuf->pubseekpos(0, file->in);
		// ������� ������ ��� �������� ������ �����
		vector<char> buffer(size);
		// ������� ������ �� ����� � ������
		fbuf->sgetn(&buffer[0], size);

		size_t start = 0;
		int col_num = 0;
		size_t phone_start = 0;
		size_t phone_sz = 0;
		size_t fio_start = 0;
		size_t fio_sz = 0;
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
					phone_start = start;
					phone_sz = sz;
				}
				else if (1 == col_num)
				{
					fio_start = start;
					fio_sz = sz;
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
					data_maps->OfflineMap.emplace(std::make_pair(string(&buffer[phone_start], phone_sz), string(&buffer[fio_start], fio_sz)));
				}
				else
				{
					data_maps->OnlineMap.emplace(std::make_pair(string(&buffer[phone_start], phone_sz), string(&buffer[fio_start], fio_sz)));
				}
				
				start = ++i;
				col_num = 0;
			}
		}

		auto end = std::chrono::steady_clock::now();
		auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
		std::cout << "����� ������ ����� = " << elapsed_ms.count() << "ms" << EndLineChar;
		data_maps->IsFileRead.store(true);
		file->close();
		}, name
	);

	loading_file_thread.detach();
}

bool CSVFile::CreateNewFile(const string& file_name)
{
	if (!OpenFile(file_name))
		return false;

	std::cout << "������ ��������� �����...\n";

	std::random_device rd;
	std::mt19937 mersenne(rd()); // �������������� ����� �������� ��������� ��������� ������ 
	size_t last_length = 0;
	string str;// 
	string num_s;
	size_t str_length;
	int val;
	string line;
	string name;
	stringstream ss;

	auto write_to_file = [&]() {
		line = ss.str();
		AddLine(line);

		ss.str(""); // ������� �����
		ss.clear(); // ���������� ��� ����� ������
	};

	auto begin = std::chrono::steady_clock::now();
	for (uint64_t i = 1; i <= LineSize; ++i)
	{
		num_s = std::to_string(i);
		str_length = num_s.length();
		if (last_length != str_length)
		{
			str = string(WordSize - str_length, FillChar);
			last_length = str_length;
		}
		name = str + num_s;

		val = mersenne() % Mod;

		// ���������� ��������� �����, �� �������� ����������� ����� ���������� ������� ��� �� ������� + �� ����� ���������� �������� int � ������
		ss << name << Separator << name << " " << name << " " << name << Separator << val << EndLineChar;

		// ����� ������ � ���� �� ������ ������ � �� WriteLineSize (�� 1 ������� ���������, ���� ��� WriteLineSize �������� ���)
		if (!(i % WriteLineSize))
		{
			write_to_file();
		}
	}
	write_to_file();
	Flush();
	auto end = std::chrono::steady_clock::now();
	auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

	std::cout << "����� ��������� ����� = " << elapsed_ms.count() << "ms";

	return true;
}
