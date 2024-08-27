#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <algorithm>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <sstream>


class Client
{
private:
    int buffersize =        1024;
    std::queue<std::string> buffer;
    std::mutex              buffer_mutex;
    std::condition_variable cv;


    struct sockaddr_in server_address;

    int  clientsocket;
    bool connected = false;
    bool ready =     false;

public:
    Client(int port);
    ~Client();

    bool connect_t();
    void reconnect();
    void getline();
    void send_data();
    void start();

    bool             _is_digit(const std::string & value);
    void             _quick_sort_rev(std::vector<int> & nums, int low, int high);

    std::vector<int> _line_to_vector(const std::string & value);
    std::string      _even_nums(std::vector<int> & nums);
    std::string      _get_line_sum(std::string & value);

};
