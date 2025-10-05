#ifndef ROUTING_HPP
#define ROUTING_HPP

#include "ConfigParser.hpp"
#include "HTTPParser.hpp"

class Routing
{
private:
    WebConfigFile &_config;

public:
    Routing(WebConfigFile &config);

    ServerConfig *findServer(const std::string &host);
    Location *findLocation(ServerConfig &server, const std::string &request_path);
    bool isMethodAllowed(Location &loc, const std::string &method);
};

#endif
