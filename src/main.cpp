#include <iostream>
#include <cstdlib>
#include "Logger.hpp"
#include "HTTPMessage.hpp"

void print_msg(HTTPMessage& msg)
{
    for (size_t i = 0; i < msg.startLine.size(); ++i)
    std::cout << msg.startLine[i] << "..";
    std::cout << std::endl << std::endl;
    for (strmap::iterator it = msg.headers.begin(); it != msg.headers.end(); ++it)
        std::cout << it->first << " : " << it->second << std::endl;
    std::cout << std::endl;;
    std::cout << msg.body << std::endl;
}

int main(const int ac, const char **av, const char **env)
{
    if (ac != 2)
    {
        std::cerr << "usage: ./webserv [CONFIG]" << std::endl;
        return EXIT_FAILURE;
    }
    (void)av; (void)env;
    try
    {
        HTTPMessage req = HTTPMessageParser::parse("GET /path HTTP/1.1\r\nHost: example.com\r\nAccept: */*\r\nConnection: close\r\n\r\n");
        HTTPMessage res = HTTPMessageParser::parse("HTTP/1.1 200 ok\r\ncontent: something\r\nconnection: smth\r\n\r\nthis is a body");

        std::cout << "request:\n";
        print_msg(req);

        std::cout << "response:\n";
        print_msg(res);

        
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}