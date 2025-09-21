// Example usage in event loop (event_loop.cpp)

#include "../../include/server/Client.hpp"
#include "../../include/server/Epoll.hpp"
#include <map>

// In your event loop, you'd maintain a map of clients
std::map<int, Client*> clients;

void handleClientEvent(int clientFd, uint32_t events) {
    Client* client = clients[clientFd];
    
    if (!client) {
        return; // Client not found
    }
    
    // Handle read events
    if (events & EPOLLIN) {
        switch (client->getState()) {
            case READING_REQUEST:
                if (!client->readRequest()) {
                    // Request complete or error, switch to sending response
                    if (client->hasError()) {
                        // Clean up and close
                        delete client;
                        clients.erase(clientFd);
                        close(clientFd);
                    }
                    // Note: Client automatically transitions to SENDING_RESPONSE state
                }
                break;
                
            case KEEP_ALIVE:
                // New request on keep-alive connection
                if (!client->readRequest()) {
                    if (client->hasError()) {
                        delete client;
                        clients.erase(clientFd);
                        close(clientFd);
                    }
                }
                break;
                
            default:
                // Unexpected read event
                break;
        }
    }
    
    // Handle write events
    if (events & EPOLLOUT) {
        if (client->getState() == SENDING_RESPONSE) {
            if (!client->sendResponse()) {
                // Response complete
                if (client->getState() == CLOSED) {
                    // Close connection
                    delete client;
                    clients.erase(clientFd);
                    close(clientFd);
                } else if (client->getState() == KEEP_ALIVE) {
                    // Keep connection open for next request
                    // Switch epoll interest back to EPOLLIN only
                }
            }
            // If returns true, more data to send, keep EPOLLOUT
        }
    }
    
    // Handle errors or connection closed
    if (events & (EPOLLHUP | EPOLLERR)) {
        delete client;
        clients.erase(clientFd);
        close(clientFd);
    }
}

// When accepting new connections:
void acceptNewClient(Socket& serverSocket, Epoll& epoll) {
    Socket clientSocket = serverSocket.accept();
    clientSocket.set_non_blocking();
    
    Client* client = new Client(clientSocket);
    clients[clientSocket.get_fd()] = client;
    
    // Register with epoll for reading
    epoll.add(clientSocket.get_fd(), EPOLLIN | EPOLLET);
}
