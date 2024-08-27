#include <iostream>
#include <vector>
#include <thread>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
#include <cerrno>
#include <stdexcept>

class Server
{
private:
    char buffer[1024];
    int serversocket;
    int clientsocket;

    struct sockaddr_in serveraddress;
    struct sockaddr_in clientaddress;

    socklen_t serverlen;

public:
    Server(int port);
    ~Server();

    void start();
    void stop();
};
