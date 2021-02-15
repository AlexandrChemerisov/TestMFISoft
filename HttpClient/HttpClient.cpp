#include <iostream>
#include <string>
#include <conio.h>
#include <chrono>
#include "httplib.h"
using namespace std;

constexpr char PhoneNumber[] = "PhoneNumber";
constexpr int StatusOk = 200;
constexpr char EndLineChar = '\n';

int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    
    // HTTP Client
    httplib::Client cli("localhost", 8080);
    
    std::cout << "1 - Запросить данные абонента по номеру телефона\n";
    std::cout << "2 - Запросить данные всех абонентов онлайн\n";
    std::cout << "ESC - Выйти\n";
    string phone_num;
    char c = '0';
    int status = -1;
    string body;
    std::chrono::steady_clock::time_point begin;
    std::chrono::steady_clock::time_point end;
    while (true)
    {
        c = _getch();
        if (c == '1')
        {
            std::cout << "Введите номер телефона абонента\n";
            std::cin >> phone_num;
            httplib::Headers p;
            p.insert(std::make_pair(PhoneNumber, phone_num));

            begin = std::chrono::steady_clock::now();
            auto res = cli.Get("/GetFio", p);
            end = std::chrono::steady_clock::now();
            
            status = res->status;
            if (status == StatusOk)
            {
                cout << res->body << EndLineChar;
                cout << "Время выполнения запроса = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << EndLineChar;

            }
            else
            {
                std::cout << "Result status = " << status << EndLineChar;
            }
        }
        else if (c == '2')
        {
            begin = std::chrono::steady_clock::now();
            auto res = cli.Get("/GetOnline");
            end = std::chrono::steady_clock::now();

            status = res->status;
            body = res->body;
            if (status == StatusOk)
            {
                cout << "Запрос выполнен успешно, но результат не выведен в консоль (закоментировано) тк это займет много времени" << EndLineChar;
                //cout << res->body << EndLineChar;
                cout << "Время выполнения запроса = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << EndLineChar;
            }
            else
            {
                std::cout << "Неудачный запрос. Статус = " << status << EndLineChar;
            }
        }
        else if (c == 27)
        {
            break;
        }
        else
        {
            std::cout << "Неверная команда\n";
        }
    }
    return 0;
}
