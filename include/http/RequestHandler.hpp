#ifndef WEBSERV_REQ_HANDLER
#define WEBSERV_REQ_HANDLER

#include "HTTPParser.hpp"
#include "Response.hpp"
#include "Routing.hpp"
#include "Logger.hpp"

// the current methods we are required to handle
// static const char* methods[] = { "GET", "POST", "DELETE" };

class RequestHandler
{
    Logger  logger;
    
    Routing         _router;
    HTTPParser      _request;
    HTTPResponse    _response;

    bool            _keepAlive;

public:
    RequestHandler(ServerConfig &config);
    ~RequestHandler();

    void    feed(char* buff, size_t size);

    bool    isReqComplete();
    bool    isResComplete();
    bool    isError();
    bool    keepAlive();

    void    processRequest();
    size_t  readNextChunk(char* buff, size_t size);

    void    reset();
};


#endif