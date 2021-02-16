#include <conio.h>
#include "CSVFile.h"
#include "httplib.h"

constexpr char PhoneNumberNotFound[] = "Номер телефона не найден";
constexpr char NoActiveSubscribers[] = "Нет активных абонентов";
constexpr char Host[] = "localhost";
constexpr int Port = 8080;
constexpr char FileName[] = "test.csv";
constexpr char MoreData[] = "MoreData"; // Заголовок указывающий что не все еще данные переданы. Можно так же указывать размер или идентификатор клиента.
constexpr char NeedToWait[] = "NeedToWait";
constexpr char FileNotLoad[] = "Данные незагружены. Попробуйте позже.";
constexpr size_t MaxLineCounter = 100000; // Отправлять по http строк в пакете

int main(int argc, char* argv[])
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	
	std::cout << "1 - Генерировать файл\n";
	std::cout << "2 - Загрузить существующий файл и запустить http сервер\n";
	std::cout << "ESC - Выйти\n";
	
	try
	{
		CSVFile csv_file;
		
		char c = _getch();

		size_t data_size = 0;
		if (c == '1')
		{
			if (!csv_file.CreateNewFile(FileName))
				cout << "Ошибка генерации файла\n";
		}
		else if (c == '2')
		{
			shared_ptr<phone_data> phone_maps(make_shared<phone_data>());

			CSVFile::ReadAll(FileName, phone_maps);

			// HTTP
			httplib::Server svr;

			svr.Get("/GetFio", [&](const httplib::Request& req, httplib::Response& res) {
				auto begin = std::chrono::steady_clock::now();

				if (phone_maps->IsFileRead.load())
				{
					string content(PhoneNumberNotFound);
					auto it = req.headers.find("PhoneNumber");
					if (it != req.headers.end())
					{
						string phone = it->second;
						auto phone_it = phone_maps->OnlineMap.find(phone);
						bool phone_is_find = false;
						if (phone_it != phone_maps->OnlineMap.end())
						{
							content = phone_it->second + Separator + OnlineChar;
						}
						else
						{
							phone_it = phone_maps->OfflineMap.find(phone);
							if (phone_it != phone_maps->OfflineMap.end())
							{
								content = phone_it->second + Separator + OfflineChar;
							}
						}
					}
					res.set_content(content, "text/plain");
				}
				else
				{
					res.set_header(NeedToWait, NeedToWait);
					res.set_content(FileNotLoad, "text/plain");
				}

				auto end = std::chrono::steady_clock::now();
				cout << "Время обработки запроса = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << EndLineChar;
				});

			phone_map::iterator last_it;
			bool iter_init = false;

			svr.Get("/GetOnline", [&](const httplib::Request&, httplib::Response& res) {
				string content(NoActiveSubscribers);

				if (phone_maps->IsFileRead.load())
				{
					if (!iter_init)
					{
						iter_init = true;
						last_it = phone_maps->OnlineMap.begin();
					}

					if (!phone_maps->OnlineMap.empty() && last_it != phone_maps->OnlineMap.end())
					{
						int i = 1;
						if (last_it != phone_maps->OnlineMap.begin())
							++last_it;
						content.clear();

						std::stringstream ss;
						for (; last_it != phone_maps->OnlineMap.end(); ++last_it)
						{
							ss << last_it->first << Separator << last_it->second << EndLineChar;
							if (!(i % MaxLineCounter))
							{
								// Еще не все данные переданы
								res.set_header(MoreData, &OnlineChar);
								break;
							}
							++i;
						}
						content = ss.str();
					}

					if (last_it == phone_maps->OnlineMap.end())
						last_it = phone_maps->OnlineMap.begin();

					res.set_content(content, "text/plain");
					data_size += content.length();
					cout << "Передан пакет данных = " << content.length() << "байт" << EndLineChar;
				}
				else
				{
					res.set_header(NeedToWait, NeedToWait);
					res.set_content(FileNotLoad, "text/plain");
				}
				});


			cout << "Http server listen host = " << Host << " port = " << Port << endl;
			svr.listen(Host, Port);

		}
		else if (c == 27)
		{
			return 0;
		}
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
	
	return 0;
}