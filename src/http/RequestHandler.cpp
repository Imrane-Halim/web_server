#include "RequestHandler.hpp"

RequestHandler::RequestHandler(ServerConfig &config):
    _router(config),
    _response("HTTP/1.1")
{}
RequestHandler::~RequestHandler() { reset(); }

void    RequestHandler::feed(char* buff, size_t size) { _request.addChunk(buff, size); }

bool    RequestHandler::isReqComplete() { return _request.isComplete(); }
bool    RequestHandler::isResComplete() { return _response.isComplete(); }
bool    RequestHandler::isError() { return _request.isError(); }

size_t  RequestHandler::readNextChunk(char *buff, size_t size) { return _response.readNextChunk(buff, size); }

void    RequestHandler::reset()
{
    _request.reset();
    _response.reset();
}

bool    RequestHandler::keepAlive()
{
    std::string conn = _request.getHeader("connection");
    std::transform(conn.begin(), conn.end(), conn.begin(), ::tolower);

    if (_request.getVers() == "HTTP/1.1")
        return conn != "close";
    return conn == "keep-alive";
}

void    RequestHandler::processRequest()
{
    _keepAlive = keepAlive();
    
    const RouteMatch& match = _router.match(_request.getUri(), _request.getMethod());
    if (!match.isValidMatch())
    {
        _sendErrorResponse(404);
        return;
    }
    if (match.isCGI)
    {
        // todo. don't touch for now
        return;
    }
    const std::string& method = _request.getMethod();
    if (method == "GET")
        _handleGET(match);
    else if (method == "POST")
        _handlePOST(match);
    else if (method == "DELETE")
        _handleDELETE(match);
    else
        _sendErrorResponse(501);
}

void    RequestHandler::_handleGET(const RouteMatch& match)
{
    // 1. Check redirects
    // 2. Check if path exists (404 if not)
    // 3. If directory: call _servDict
    // 4. If file: serve it

    if (match.isRedirect)
    {
        _response.startLine(301);
        _response.addHeader("location", match.redirectUrl);
        _response.endHeaders();
        return;
    }
    if (!match.doesExist)
    {
        _sendErrorResponse(404);
        return;
    }
    if (match.isDirectory)
    {
        _serveDict(match);
        return;
    }
    if (match.isFile)
    {
        _serveFile(match);
        return;
    }
}
void    RequestHandler::_handlePOST(const RouteMatch& match)
{
    (void)match;
    // 1. Check if upload is allowed (match.isUploadAllowed())
    // 2. Check body size against maxBodySize (413 if too large)
    // 3. If CGI: handle via CGI
    // 4. If upload: save file to uploadDir
    // 5. Return 201 Created or appropriate response
}
void    RequestHandler::_handleDELETE(const RouteMatch& match)
{
    (void)match;
    // 1. Check if path exists (404 if not)
    // 2. Check if it's a file (403 if directory)
    // 3. Check permissions
    // 4. Delete the file with unlink()
    // 5. Return 204 No Content or 200 OK
}

void    RequestHandler::_sendErrorResponse(int code)
{
    _response.reset();
    _response.startLine(code);
    if (!_response.attachFile(_router.getErrorPage(code)))
        _response.setBody(getErrorPage(code));
}

void    RequestHandler::_serveFile(const RouteMatch& path)
{
    _response.startLine(200);
    if (!_response.attachFile(path.fsPath))
    {
        logger.error("cant send file: " + path.fsPath);
        _sendErrorResponse(403);
    }
}
void    RequestHandler::_serveDict(const RouteMatch& path)
{
    //    - Check autoindex
    //    - Try index files
    //    - 403 if neither
    _response.startLine(200);
    if (path.autoIndex)
    {
        _response.setBody(_getDictListing(path.fsPath));
        return;
    }
    for (size_t i = 0; i < path.indexFiles.size(); ++i)
    {
        if (_response.attachFile(path.fsPath + '/' + path.indexFiles[i]))
            return;
    }
    _sendErrorResponse(403);
}

std::string RequestHandler::_getDictListing(const std::string& path)
{
    (void)path;
    return "todo";
}
