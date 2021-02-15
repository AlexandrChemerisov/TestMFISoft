#include <random>
#include <sstream>
#include <chrono>
#include <conio.h>
#include "CSVFile.h"
#include "httplib.h"
constexpr char FillChar = '0';
constexpr size_t WordSize = 8;
constexpr size_t LineSize = 25000000;
constexpr size_t WriteLineSize = 100;
constexpr size_t Mod = 2;
constexpr char PhoneNumberNotFound[] = "Номер телефона не найден";//"Phone number not found!";
constexpr char NoActiveSubscribers[] = "Нет активных абонентов";//"No active subscribers!";
constexpr char Host[] = "localhost";
constexpr int Port = 8080;
constexpr char FileName[] = "test.csv";//"Phone number not found!";

void createCSVFile(const string& file_name)
{
	try
	{
		CSVFile csv_file(file_name);
		if (!csv_file.IsOpen())
			return;

		cout << "Начало генерации файла...\n";

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

		auto begin = std::chrono::steady_clock::now();
		for (uint64_t i = 1; i < LineSize; ++i)
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
				line = ss.str();
				csv_file.AddLine(line);

				ss.str(""); // очищаем буфер
				ss.clear(); // сбрасываем все флаги ошибок
			}
		}
		csv_file.Flush();
		auto end = std::chrono::steady_clock::now();
		auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

		cout << "Время генерации файла = " << elapsed_ms.count() << "ms";
	}
	catch (std::ifstream::failure e)
	{
		error_code cd = e.code();
		std::cerr << e.what() << '\n';
		std::cerr << "Exception opening/reading/closing file\n";
	}
	catch (...)
	{
		std::cerr << "Some other error\n";
	}
}

bool readFile(const string& file_name, unordered_map<string, string>& online, unordered_map<string, string>& offline)
{
	try
	{
		CSVFile csv_file(file_name);
		return csv_file.ReadAll(online, offline);
	}
	catch (std::ifstream::failure e)
	{
		error_code cd = e.code();
		std::cerr << e.what() << EndLineChar;
		std::cerr << "Exception opening/reading/closing file\n";
	}
	catch (...)
	{
		std::cerr << "Some other error\n";
	}
	return false;
}

int main(int argc, char* argv[])
{


	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	
	std::cout << "1 - Генерировать файл\n";
	std::cout << "2 - Загрузить существующий файл и запустить http сервер\n";
	std::cout << "ESC - Выйти\n";
	char c = _getch();

	if (c == '1')
	{
		createCSVFile(FileName);
	}
	else if (c == '2')
	{
		unordered_map<string, string> online_map;
		unordered_map<string, string> offline_map;
		if (!readFile(FileName, online_map, offline_map))
		{
			return 1;
		}

		// HTTP
		httplib::Server svr;

		svr.Get("/GetFio", [&](const httplib::Request& req, httplib::Response& res) {
			auto begin = std::chrono::steady_clock::now();
			string content(PhoneNumberNotFound);
			auto it = req.headers.find("PhoneNumber");
			if (it != req.headers.end())
			{
				string phone = it->second;
				auto phone_it = online_map.find(phone);
				bool phone_is_find = false;
				if (phone_it != online_map.end())
				{
					content = phone_it->second + Separator + OnlineChar;
				}
				else
				{
					phone_it = offline_map.find(phone);
					if (phone_it != offline_map.end())
					{
						content = phone_it->second + Separator + OfflineChar;
					}
				}
			}
			res.set_content(content, "text/plain");
			auto end = std::chrono::steady_clock::now();
			cout << "Время обработки запроса = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << EndLineChar;
			});

		svr.Get("/GetOnline", [&](const httplib::Request&, httplib::Response& res) {
			auto begin = std::chrono::steady_clock::now();
			string content(NoActiveSubscribers);
			if (!online_map.empty())
			{
				std::stringstream ss;
				for (auto it : online_map)
				{
					ss << it.first << Separator << it.second << EndLineChar;
				}
				content = ss.str();
			}
			res.set_content(content, "text/plain");
			auto end = std::chrono::steady_clock::now();
			cout << "Время обработки запроса = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << EndLineChar;
			});

		cout << "Http server listen host = " << Host << " port = " << Port << EndLineChar;
		svr.listen(Host, Port);

	}
	else if (c == 27)
	{
		return 0;
	}
	else
	{
		std::cout << "Неверная команда\n";
	}
	
	return 0;
}