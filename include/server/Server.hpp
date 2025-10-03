#ifndef SERVER_HPP
#define SERVER_HPP

#include "EventHandler.hpp"
#include "Socket.hpp"

class Server : public EventHandler
{
    private:
        Socket socket;
    public:
        Server(FdManager &fdm);
        ~Server() {}
        int get_fd() const;
        void onReadable();
        void onWritable();
        void onError();
};

int Server::get_fd() const
{
    return socket.get_fd();
}

Server::Server(FdManager &fdm) : EventHandler(fdm), socket(AF_INET, SOCK_STREAM, 0)
{
    socket.bind();
    socket.listen();
    socket.set_non_blocking();
}


void Server::onReadable()
{
    // Accept new connection
    Socket client_socket = socket.accept();
    client_socket.set_non_blocking();
}

void Server::onWritable()
{
    // Server socket is not expected to be writable
}

void Server::onError()
{
    // Handle error, possibly log it
    socket.close();
}



#endif //SERVER_HPP