#include "server.h"


Server::Server(int port)
{
    // fill server prop
    serveraddress.sin_family      = AF_INET;
    serveraddress.sin_port        = htons(port);
    serveraddress.sin_addr.s_addr = INADDR_ANY;
}
Server::~Server()
{
    std::cout << "Server closed.\n";
    close(serversocket);
}

void Server::start()
{
    // create master socket
    serversocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serversocket < 0)
    {
        std::cerr << "Не удалось создать сокет\n" << std::strerror(errno) << "\n";
        exit(1);
    }

    struct timeval timeout;
    timeout.tv_sec = 30;  // time before connection ll destoyed
    timeout.tv_usec = 0;

    // set read timeout
    if (setsockopt(serversocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        std::cerr << "Не удалось установить тайм-аут для чтения сокета\n" << std::strerror(errno) << "\n";
        exit(1);
    }

    // set write timeout (optional)
    if (setsockopt(serversocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        std::cerr << "Не удалось установить тайм-аут для записи сокета\n" << std::strerror(errno) << "\n";
        exit(1);
    }

    // bind socket
    if (bind(serversocket, (struct sockaddr*)&serveraddress, sizeof(serveraddress)) != 0)
    {
        std::cerr << "Не удалось привязать сокет" << std::strerror(errno) << "\n";
        exit(1);
    }
    // listen master socket
    listen(serversocket, 2);

    this->serverlen = sizeof(serveraddress);
    std::cout << "Server started.\n";

    //connection* loop
    while(true)
    {
        // accept connection, return new socket from master socket
        int clientsocket = accept(serversocket, (struct sockaddr*)&serveraddress, &serverlen);
        if(clientsocket < 0)
        {
            std::cerr << "Ошибка при принятии соединения " << std::strerror(errno) << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(3));
            continue;
        }

        std::cout << "Клиент подключился.\n";
        
        // client loop    
        while (true)
        {
            unsigned long number;
            memset(buffer, '\0', sizeof(buffer)); // allocate mem to buffer

            ssize_t ac_bytes = recv(clientsocket, buffer, sizeof(buffer), 0);
            if (ac_bytes < 0){
                std::cerr << "Ошибка при чтении данных из сокета\n";
                //close(clientsocket);
                break;
            } else if (ac_bytes == 0){             // catch connection break
                std::cerr << "Соединение разорвано.\n";
                //close(clientsocket);
                break;         
            }
            // send return msg
            send(clientsocket, buffer, ac_bytes, 0);

            std::string received(buffer);

            try
            {
                number = std::stoul(received);
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
                number = 0;
            }
            
            
            // check for valid data
            if((received.size() > 2) && (number%32 == 0)){
                std::cout << "Данные приняты: " << number << std::endl;
            } else {
                std::cout << "Ошибочные данные. Ожидание новых данных...\n";
            }
        }     
        std::cout << "Соединение с клиентом разорвано.\n";
        close(clientsocket);
    }  
    stop();
}

void Server::stop()
{
    close(serversocket);
    std::cout << "Server stoped.\n";
}

int main()
{
    Server serv(8888);
    serv.start();
    return 0;
}
