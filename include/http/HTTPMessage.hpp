#ifndef WEBSERV_REQUEST_HPP
#define WEBSERV_REQUEST_HPP

#include <map>
#include <string>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <sstream>
#include <cstdlib>

// =========================================================
//          THIS CODE ASSUMES THE HTTP MESSAGE IS COMPLETE 
//              TODO: USE A STATE MACHINE
// =========================================================


#define CRLF     "\r\n"
#define CRLFSIZE sizeof(CRLF) - 1

typedef std::vector<std::string>            strvec;
typedef std::map<std::string, std::string>  strmap;

struct HTTPMessage
{
    strvec      startLine;
    strmap      headers;
    std::string body;
};

class HTTPMessageParser
{
    static strvec       _getStartLine(const std::string& msg, size_t& pos);
    static strmap       _getHeaders(const std::string& msg, size_t& pos);
    static std::string  _getBody(const std::string& msg, size_t& pos, const strmap& headers);
public:
    static HTTPMessage parse(const std::string& msg);
};

#endif