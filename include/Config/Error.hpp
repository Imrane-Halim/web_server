#ifndef WEBSERV_ERROR_HPP
#define WEBSERV_ERROR_HPP

#include <exception>
#include <string>
#include <stdint.h>
#include <sstream>

#define SSTR(x) static_cast<std::ostringstream &>( \
    std::ostringstream() << x).str()


class ParserError: public std::exception
{
    std::string _err;
    public:
    ParserError(const std::string& msg, uint32_t line, uint32_t colm);
    ~ParserError() throw();
    const char* what() const throw();
};

#endif