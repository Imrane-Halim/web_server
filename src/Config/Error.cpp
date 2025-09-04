#include "Error.hpp"

ParserError::ParserError(const std::string &msg, uint32_t line, uint32_t colm)
{
    std::ostringstream err;

    err << line << ":" << colm << ": " << msg;
    _err = err.str();
}

ParserError::~ParserError() throw() {}

const char *ParserError::what() const throw()
{
    return _err.c_str();
}
