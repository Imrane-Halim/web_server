#ifndef ERROR_PAGES_HPP
#define ERROR_PAGES_HPP

#include <iostream>
#include <string>
#include <map>

// global map holding default HTML pages for common HTTP errors
extern std::map<int, std::string> defaultErrorPages;

// return the HTML content for a given HTTP error code
const std::string getErrorPage(int code);

void initErrorPages();

#endif
