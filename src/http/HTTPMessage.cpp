#include "HTTPMessage.hpp"

strvec      HTTPMessageParser::_getStartLine(const std::string& msg, size_t& pos)
{
    pos = msg.find(CRLF);

    std::istringstream  is(msg.substr(0, pos));
    std::string         token;
    strvec              res;
    
    while((is >> token))
        res.push_back(token);
    pos += CRLFSIZE;

    return res;
}

strmap      HTTPMessageParser::_getHeaders(const std::string& msg, size_t& pos)
{
    strmap res;
    size_t end;

    while (true)
    {
        end = msg.find(CRLF, pos);
        if (end == pos) // mens end of header
        {
            pos += CRLFSIZE;
            break;
        }
        std::string line = msg.substr(pos, end - pos);
        size_t colon_pos = line.find(':');

        if (colon_pos != std::string::npos)
        {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            size_t start = value.find_first_not_of(" ");

            if (start != std::string::npos)
                value = value.substr(start);
            
            res[key] = value;
        }
        pos = end + CRLFSIZE;
    }

    return res;
}

std::string HTTPMessageParser::_getBody(const std::string& msg, size_t& pos, const strmap& headers)
{
    strmap::const_iterator it = headers.find("Content-Length");
    if (it != headers.end()) {
        int content_length = std::atol(it->second.c_str());
        return msg.substr(pos, content_length);
    }
    return msg.substr(pos);
}

HTTPMessage HTTPMessageParser::parse(const std::string& msg)
{
    HTTPMessage res;
    size_t      pos;

    
    res.startLine = _getStartLine(msg, pos);
    res.headers   = _getHeaders(msg, pos);
    res.body      = _getBody(msg, pos, res.headers);

    return res;
}