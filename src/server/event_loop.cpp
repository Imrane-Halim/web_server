#include "Epoll.hpp"
#include "Socket.hpp"
#include "Client.hpp"
#include "../utils/Logger.hpp"
#include <map>
#include <iostream>
#include <csignal>

std::string intToString(int value);

// External shutdown flag (defined in main.cpp)
extern volatile sig_atomic_t g_shutdown;

// Forward declarations for helper functions
static void setupServerSocket(Socket& server_socket, Epoll& epoll);
static void cleanupClient(int client_fd, std::map<int, Client*>& clients, Epoll& epoll);
static void handleSocketError(Socket* event_socket, const Socket& server_socket, 
                             std::map<int, Client*>& clients, Epoll& epoll);
static void handleNewConnection(Socket& server_socket, std::map<int, Client*>& clients, 
                               Epoll& epoll, Logger& logger);
static void handleClientReadEvent(Socket* event_socket, std::map<int, Client*>& clients, 
                                 Epoll& epoll);
static void handleClientWriteEvent(Socket* event_socket, std::map<int, Client*>& clients, 
                                  Epoll& epoll, Logger& logger);
static void handleException(Socket* event_socket, std::map<int, Client*>& clients, 
                           Epoll& epoll);
static void cleanupAllClients(std::map<int, Client*>& clients, Epoll& epoll, Logger& logger);

void event_loop()
{
    Epoll epoll;
    Socket server_socket;
    std::map<int, Client*> clients;
    Logger logger;
    
    // Initialize server
    setupServerSocket(server_socket, epoll);
    logger.info("Event loop started");
    
    // Main event loop
    while (!g_shutdown) 
    {
        std::vector<Socket> events = epoll.wait();
        
        for (size_t i = 0; i < events.size(); i++) 
        {
            try 
            {
                Socket* event_socket = &events[i];
                
                // Handle socket errors first
                if (EVENT_HAS_ERROR(event_socket->get_event())) 
                {
                    handleSocketError(event_socket, server_socket, clients, epoll);
                    continue;
                }
                
                // Handle server socket events (new connections)
                if (*event_socket == server_socket) 
                {
                    handleNewConnection(const_cast<Socket&>(server_socket), clients, epoll, logger);
                }
                else 
                {
                    // Handle client socket events
                    if (EVENT_HAS_READ(event_socket->get_event()))
                    {
                        handleClientReadEvent(event_socket, clients, epoll);
                    }
                    
                    if (EVENT_HAS_WRITE(event_socket->get_event()))
                    {
                        handleClientWriteEvent(event_socket, clients, epoll, logger);
                    }
                }
            } 
            catch (const std::exception &e) 
            {
                handleException(&events[i], clients, epoll);
            }
        }
    }
    
    // Cleanup on shutdown
    cleanupAllClients(clients, epoll, logger);
}

// Helper function implementations
static void setupServerSocket(Socket& server_socket, Epoll& epoll)
{
    server_socket.bind();
    server_socket.listen();
    server_socket.set_non_blocking();
    epoll.add_fd(server_socket);
}

static void cleanupClient(int client_fd, std::map<int, Client*>& clients, Epoll& epoll)
{
    std::map<int, Client*>::iterator it = clients.find(client_fd);
    if (it != clients.end()) 
    {
        epoll.remove_fd(client_fd);
        delete it->second;
        clients.erase(it);
    }
}

static void handleSocketError(Socket* event_socket, const Socket& server_socket, 
                             std::map<int, Client*>& clients, Epoll& epoll)
{
    if (*event_socket == server_socket) 
    {
        throw std::runtime_error("Error on server socket");
    }
    else 
    {
        cleanupClient(event_socket->get_fd(), clients, epoll);
    }
}

static void handleNewConnection(Socket& server_socket, std::map<int, Client*>& clients, 
                               Epoll& epoll, Logger& logger)
{
    logger.debug("New incoming connection from client " + intToString(server_socket.get_fd()));
    
    Socket client_socket = server_socket.accept();
    client_socket.set_non_blocking();
    epoll.add_fd(client_socket, EPOLLIN);
    clients[client_socket.get_fd()] = new Client(client_socket);
}

static void handleClientReadEvent(Socket* event_socket, std::map<int, Client*>& clients, 
                                 Epoll& epoll)
{
    Client* client = clients[event_socket->get_fd()];
    bool needMoreData = client->readRequest();
    
    if (client->hasError()) 
    {
        cleanupClient(event_socket->get_fd(), clients, epoll);
        return;
    }
    
    if (!needMoreData) 
    {
        if (client->getState() == SENDING_RESPONSE) 
        {
            epoll.modify_fd(*event_socket, EPOLLOUT);
        }
        else if (client->getState() == CLOSED) 
        {
            cleanupClient(event_socket->get_fd(), clients, epoll);
        }
    }
}

static void handleClientWriteEvent(Socket* event_socket, std::map<int, Client*>& clients, 
                                  Epoll& epoll, Logger& logger)
{
    Client* client = clients[event_socket->get_fd()];
    bool needMoreWrite = client->sendResponse();
    
    if (client->hasError()) 
    {
        cleanupClient(event_socket->get_fd(), clients, epoll);
        return;
    }
    
    if (!needMoreWrite) 
    {
        logger.debug("Response sent to client " + intToString(event_socket->get_fd()));
        
        if (client->getState() == CLOSED) 
        {
            cleanupClient(event_socket->get_fd(), clients, epoll);
        }
        else if (client->getState() == KEEP_ALIVE) 
        {
            client->reset();
            epoll.modify_fd(*event_socket, EPOLLIN);
        }
    }
}

static void handleException(Socket* event_socket, std::map<int, Client*>& clients, 
                           Epoll& epoll)
{
    cleanupClient(event_socket->get_fd(), clients, epoll);
}

static void cleanupAllClients(std::map<int, Client*>& clients, Epoll& epoll, Logger& logger)
{
    logger.info("Cleaning up connections...");
    
    for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
    {
        epoll.remove_fd(it->first);
        delete it->second;
    }
    clients.clear();
    
    logger.info("Event loop shutdown complete");
}