#include "Validator.hpp"

Validator::Validator()
{
    // Main context directives
    _rules["max_connections"] = (rule_t){CTX_MAIN, 1, 1, {ARG_INT}};
    _rules["access_log"] = (rule_t){CTX_MAIN, 1, 1, {ARG_PATH}};
    _rules["error_log"] = (rule_t){CTX_MAIN, 1, 1, {ARG_PATH}};
    _rules["server"] = (rule_t){CTX_MAIN, 0, 0, {ARG_NAN}};

    // Server context directives
    _rules["listen"] = (rule_t){CTX_SERVER, 1, 1, {ARG_HOST}};
    _rules["server_name"] = (rule_t){CTX_SERVER, 1, -1, {ARG_ANY}};
    _rules["client_max_body_size"] = (rule_t){CTX_SERVER, 1, 1, {ARG_INT}};
    _rules["error_page"] = (rule_t){CTX_SERVER, 2, 2, {ARG_INT, ARG_STR}};
    _rules["location"] = (rule_t){CTX_SERVER, 1, 1, {ARG_STR}};

    // Server and Location context directives
    _rules["root"] = (rule_t){CTX_SERVER | CTX_LOCATION, 1, 1, {ARG_PATH}};
    _rules["index"] = (rule_t){CTX_SERVER | CTX_LOCATION, 1, -1, {ARG_ANY}};

    // Location context directives
    _rules["methods"] = (rule_t){CTX_LOCATION, 1, 3, {ARG_METHODS}};
    _rules["autoindex"] = (rule_t){CTX_LOCATION, 1, 1, {ARG_BOOL}};
    _rules["upload_store"] = (rule_t){CTX_LOCATION, 1, 1, {ARG_PATH}};
    _rules["return"] = (rule_t){CTX_LOCATION, 2, 2, {ARG_INT, ARG_STR}};
    _rules["cgi_pass"] = (rule_t){CTX_LOCATION, 1, 1, {ARG_STR}};
}

bool Validator::_isInt(const std::string &s)
{
    for (size_t i = 0; i < s.length(); ++i)
    {
        if (!std::isdigit(s[i]))
            return false;
    }
    return true;
}
bool Validator::_isBool(const std::string &s)
{
    return (s == "on" || s == "off" || s == "true" || s == "false");
}
bool Validator::_isHost(const std::string &s)
{
    size_t pos = s.find(":");

    if (pos == std::string::npos)
        return false;

    // Extract host and port parts
    std::string host = s.substr(0, pos);
    std::string port = s.substr(pos + 1);

    // Check port is valid
    if (!_isInt(port))
        return false;
    long p = std::atol(port.c_str());
    if (p < 1 || p > 65535)
        return false;

    // Check if host is a valid IP address
    unsigned char buff[16];
    if (inet_pton(AF_INET, host.c_str(), buff) == 1)
        return true; // Valid IPv4
    if (inet_pton(AF_INET6, host.c_str(), buff) == 1)
        return true; // Valid IPv6

    // accept non-empty hostnames
    return !host.empty();
}
bool Validator::_isMethod(const std::string &s)
{
    return (s == "GET" || s == "POST" || s == "DELETE");
}
bool Validator::_isPath(const std::string &s)
{
    if (s.empty())
        return false;

    if (s.find("..") != std::string::npos)
        return false;
    if (s.find("//") != std::string::npos)
        return false;

    return true;
}

void Validator::_isValidArgs(directive_t &dir, rule_t rule)
{
    int32_t n = dir.args.size();

    if (n < rule.min_args)
        throw ParserError("diretives '" + dir.name + "' requires at least " + SSTR(rule.min_args) + " argument(s)", dir.line, dir.colm);

    if (n > rule.max_args && rule.max_args != -1)
        throw ParserError("diretives '" + dir.name + "' can have at most " + SSTR(rule.max_args) + " argument(s)", dir.line, dir.colm);

    if (dir.name == "methods")
    {
        for (int32_t i = 0; i < n; ++i)
        {
            if (!_isMethod(dir.args[i]))
                throw ParserError("directive 'methods' has wrong type of arguments", dir.line, dir.colm);
        }
        return;
    }

    for (int32_t i = 0; i < n && i < MAX_TYPES; ++i)
    {
        arg_type_t type = rule.type[i];

        if (type == ARG_STR && dir.args[i].empty())
            throw ParserError("Empty string argument for directive '" + dir.name + "'", dir.line, dir.colm);
        else if (type == ARG_INT && !_isInt(dir.args[i]))
            throw ParserError("Expected integer argument for directive '" + dir.name + "'", dir.line, dir.colm);
        else if (type == ARG_BOOL && !_isBool(dir.args[i]))
            throw ParserError("Expected boolean argument for directive '" + dir.name + "'", dir.line, dir.colm);
        else if (type == ARG_HOST && !_isHost(dir.args[i]))
            throw ParserError("Invalid host:port format for directive '" + dir.name + "'", dir.line, dir.colm);
        else if (type == ARG_PATH && !_isPath(dir.args[i]))
            throw ParserError("Invalid path for directive '" + dir.name + "'", dir.line, dir.colm);
    }
}

void Validator::_isValidInContext(directive_t &dir, uint8_t allowed)
{
    if (!(dir.ctx & allowed))
    {
        throw ParserError("directive '" + dir.name + "' is not allowed in context",
                          dir.line, dir.colm);
    }
}

void Validator::evaluate(directive_t &dir)
{
    std::map<std::string, rule_t>::iterator it;

    it = _rules.find(dir.name);
    if (it == _rules.end())
    {
        throw ParserError("unknow or unsupported directive '" + dir.name + "'",
                          dir.line, dir.colm);
    }

    // validate context
    _isValidInContext(dir, it->second.allowed_context);

    // validate args
    _isValidArgs(dir, it->second);
}
