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
		std::cout << "Не задано имя файла";
		return false;
	}
	else
	{
		FileCSV.exceptions(std::ios::failbit | std::ios::badbit);
		FileCSV.open(name, std::fstream::in | std::fstream::out | std::fstream::app);
		if (!FileCSV.is_open())
		{
			std::cout << "Не удалось открыть файл";
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
			std::cout << "Не удалось открыть файл по адресу " << name;
			return;
		}
		std::cout << "Начало чтения файла..." << endl;

		// Получим указатель на связанный буферный объект
		std::filebuf* fbuf = file->rdbuf();
		// получим размер файла/буффера в байтах
		std::size_t size = fbuf->pubseekoff(0, file->end, file->in);
		fbuf->pubseekpos(0, file->in);
		// Выделим память для хранения данных файла
		vector<char> buffer(size);
		// Считаем данные из файла в буффер
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
				// конец столбца
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
				// Проверим выход за границы
				if (i < 1)
					continue;

				// конец строки
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
		std::cout << "Время чтения файла = " << elapsed_ms.count() << "ms" << EndLineChar;
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

	std::cout << "Начало генерации файла...\n";

	std::random_device rd;
	std::mt19937 mersenne(rd()); // инициализируем Вихрь Мерсенна случайным стартовым числом 
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

		ss.str(""); // очищаем буфер
		ss.clear(); // сбрасываем все флаги ошибок
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

		// Используем строковый поток, тк операции составления строк происходят быстрее чем со строкой + не нужно переводить отдельно int в строку
		ss << name << Separator << name << " " << name << " " << name << Separator << val << EndLineChar;

		// Будем писать в файл не каждую строку а по WriteLineSize (по 1 гораздо медленнее, реже чем WriteLineSize прироста нет)
		if (!(i % WriteLineSize))
		{
			write_to_file();
		}
	}
	write_to_file();
	Flush();
	auto end = std::chrono::steady_clock::now();
	auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

	std::cout << "Время генерации файла = " << elapsed_ms.count() << "ms";

	return true;
}
