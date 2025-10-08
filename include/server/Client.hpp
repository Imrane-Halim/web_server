#ifndef WEBSERV_CLIENT_HPP
#define WEBSERV_CLIENT_HPP

#include "EventHandler.hpp"
#include "FdManager.hpp"
#include "Socket.hpp"
#include "HTTPParser.hpp"
#include "Response.hpp"
#include "ConfigParser.hpp"
#include "Logger.hpp"


enum ClientState
{
    ST_READING      ,  // reading the request and parsing it
    ST_PROCESSING   ,  // processing the request and build a response
    ST_PARSEERROR   ,  // bad request
    ST_SENDING      ,
    ST_SENDCOMPLETE ,
    ST_CLOSED       ,
    ST_ERROR
};

class Client: public EventHandler
{
    Logger logger;

    char         _readBuff[BUFF_SIZE];
    HTTPParser   _request;
    HTTPResponse _response;

    std::string  _strFD;
    ClientState  _state;

    bool         _keepAlive;

    bool    _shouldKeepAlive();

    void    _closeConnection();

    void    _processError();
    void    _processRequest();

    bool    _readData(); // returns true if more data is expected
    bool    _sendData(); // returns true if more data needs to be sent

public:
    Client(Socket &socket, ServerConfig &config, FdManager &fdm);
    ~Client();

    void    reset();

    // EventHandler interface
    int get_fd() const;
    void onReadable();
    void onWritable();
    void onError();
};

#endif //CLIENT_HPP