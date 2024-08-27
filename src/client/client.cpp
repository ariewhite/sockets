#include "client.h"
#define BUFFER_SIZE 1024


Client::Client(int port)
{
    server_address.sin_family      = AF_INET;
    server_address.sin_port        = htons(port);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
}

Client::~Client() {
    close(clientsocket);
}
/*
 * Создает сокет, конектится к серверу*
 * возращает истину если коннект удачный
 */
bool Client::connect_t()
{
    clientsocket = socket(AF_INET, SOCK_STREAM, 0); 
    if (clientsocket < 0) {
        std::cerr << "Ошибка при создании сокета: " << strerror(errno) << '\n';
        connected = false;
        return false;
    }
    if (connect(clientsocket, (struct sockaddr*)&server_address, sizeof(server_address)) != 0) {
        std::cerr << "Ошибка при подключении к серверу: " << strerror(errno) << '\n';
        connected = false;
        return false;
    }

    connected = true;
    return true;
}
/*
 * Функция переподключения к серверу*
 * запускается в цикле функцию Client::connect_t
 */
void Client::reconnect()
{
    while (!connected)
    {   
        std::cerr << "Попытка переподключения... след через 3 сек" << std::endl;
        if (connect_t())
        {
            std::cerr << "Успех!\n";
        } else {
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }       
    } 
}
/*
 * Метод исполняемый в первом* потоке
 * Обрабатывает вводимую строку от пользователя и записывает ее в буфер
 */
void Client::getline()
{
    std::cout << "Введи строку из чисел (через пробелы):" << std::endl;
    do {
        std::string input;
        std::getline(std::cin, input);
        {
            std::unique_lock<std::mutex> lock(buffer_mutex);
            cv.wait(lock, [this]{return !ready; });


            if (_is_digit(input) && input.size() <= 64) {
                std::vector<int> arr = _line_to_vector(input);
                _quick_sort_rev(arr, 0, arr.size() - 1);
                input = _even_nums(arr);
                buffer.push(input);
                ready = true;
            } else {
                std::cerr << "Вводимая строка некорректна. Введите новую!\n";
                continue;
            }

            cv.notify_one();
        }

    } while(true);
}
/*
 * Метод исполняемый во втором* потоке
 * Считывает данные из буфера, затирает буфер, подсчитывает сумму строки
 * и отправляет ее через сокет на сервер*
 */
void Client::send_data()
{
    char buf[1024];

    do {
        memset(buf, '\0', buffersize);
        std::unique_lock<std::mutex> lock(buffer_mutex);
        cv.wait(lock, [this]{ return ready && connected; });

        std::string data = buffer.front();

        std::cout << "Thread2 Полученные данные: " << data << std::endl;

        data = _get_line_sum(data);

        ssize_t bytesSent = send(clientsocket, data.c_str(), data.size(), 0); 
        if (bytesSent == -1) {
            std::cerr << "Ошибка отправки данных: " << strerror(errno) << '\n';
            ready = false;
            continue;
        }
        ssize_t bytesReceived = recv(clientsocket, buf, sizeof(buf), 0);
        if (bytesReceived < 0) {
            std::cerr << "Ошибка приема данных, соединения разорвано: " << strerror(errno) << '\n';
            reconnect();
            continue;
        } 
        
        // проверка корректности соединения
        if (bytesSent != bytesReceived){
            std::cerr << "Соединения разорвано!\n";
            connected = false;
            reconnect();
            continue;
        } else {
            // std::cout << "Данные корректно отправлены\n";
            buffer.pop();
        }

        ready = false;
        cv.notify_all();

    } while(true);
}
/*
 * Запускает "подключение" к серверу* и потоки с функциями.
 */
void Client::start()
{
    if(connect_t()){
        std::thread th1(&Client::getline, this);
        std::thread th2(&Client::send_data, this);

        th1.join();
        th2.join();

        std::cout << "stop client\n";
    }
}
/*
 * Проверка строки на наличие цифр
 */
bool Client::_is_digit(const std::string & value)
{
    return std::all_of(value.begin(), value.end(), [](char x) {  // "бежим" по элем-м строки
        return std::isdigit(x) || std::isspace(x);               // возв истину, если пробел или цифра
    });
}
/*
 * Конвертирует строку в целочисленный вектор
 */
std::vector<int> Client::_line_to_vector(const std::string & value)
{
    std::vector<int> result;
    std::istringstream stream(value);
    int x;
    while (stream >> x) {
        result.push_back(x);
    }
    return result;
}
/*
 * Обратная быстрая сортировка (nlogn)
 */
void Client::_quick_sort_rev(std::vector<int> & nums, int low, int high)
{
    int i = low, j = high;
    int pivot = nums[(i + j) / 2];
    while (i <= j) {
        while (nums[i] > pivot) i++;
        while (nums[j] < pivot) j--;
        if (i <= j) {
            std::swap(nums[i], nums[j]);
            i++;
            j--;
        }
    }
    if (j > low) _quick_sort_rev(nums, low, j);
    if (i < high) _quick_sort_rev(nums, i, high);
}
/*
 * Возвращает строку без четных чисел.
 */
std::string Client::_even_nums(std::vector<int> & nums)
{
    std::string result;
    for (int num : nums) {
        if (num % 2 == 0) {
            result.append("КВ ");
        } else {
            result.append(std::to_string(num) + " ");
        }
    }
    return result;
}
/*
 * Возвращает "сумму" элементов строки
 */
std::string Client::_get_line_sum(std::string & value)
{
    unsigned long sum = 0;
    std::istringstream stream(value);
    std::string x;
    while (stream >> x) {
        if (x != "КВ") {
            sum += std::stoi(x);
        }
    }
    return std::to_string(sum);
}

int main()
{
    Client cli(8888);
    cli.start();
    std::cout << "end";
    return 0;
}