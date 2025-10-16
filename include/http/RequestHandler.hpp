#ifndef WEBSERV_REQ_HANDLER
#define WEBSERV_REQ_HANDLER

#include <dirent.h>

#include "HTTPParser.hpp"
#include "Response.hpp"
#include "Routing.hpp"
#include "Logger.hpp"
#include "SpecialResponse.hpp"

// the current methods we are required to handle
// static const char* methods[] = { "GET", "POST", "DELETE" };

class RequestHandler
{
    Logger  logger;
    
    Routing         _router;
    HTTPParser      _request;
    HTTPResponse    _response;

    bool            _keepAlive;

    // i wanted to use an iteface for this, but it's overkill
    void    _handleGET(const RouteMatch& match);
    void    _handlePOST(const RouteMatch& match);
    void    _handleDELETE(const RouteMatch& match);

    // helper methods
    void        _sendErrorResponse(int code);
    void        _serveFile(const RouteMatch& path);
    void        _serveDict(const RouteMatch& match);
    std::string _getDictListing(const std::string& path);

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