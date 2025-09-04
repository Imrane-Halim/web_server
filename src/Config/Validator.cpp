#include "Validator.hpp"

Validator::Validator()
{
    // Main context directives
    _rules["max_connections"] = (rule_t){CTX_MAIN, 1, 1};
    _rules["access_log"] = (rule_t){CTX_MAIN, 1, 1};
    _rules["error_log"] = (rule_t){CTX_MAIN, 1, 1};
    _rules["server"] = (rule_t){CTX_MAIN, 0, 0};

    // Server context directives
    _rules["listen"] = (rule_t){CTX_SERVER, 1, 1};
    _rules["server_name"] = (rule_t){CTX_SERVER, 1, -1};
    _rules["client_max_body_size"] = (rule_t){CTX_SERVER, 1, 1};
    _rules["error_page"] = (rule_t){CTX_SERVER, 2, 2};
    _rules["location"] = (rule_t){CTX_SERVER, 1, 2};

    // Server and Location context directives
    _rules["root"] = (rule_t){CTX_SERVER | CTX_LOCATION, 1, 1};
    _rules["index"] = (rule_t){CTX_SERVER | CTX_LOCATION, 1, -1};

    // Location context directives
    _rules["methods"] = (rule_t){CTX_LOCATION, 1, 3};
    _rules["autoindex"] = (rule_t){CTX_LOCATION, 1, 1};
    _rules["upload_store"] = (rule_t){CTX_LOCATION, 1, 1};
    _rules["return"] = (rule_t){CTX_LOCATION, 2, 2};
    _rules["cgi_pass"] = (rule_t){CTX_LOCATION, 1, 1};
}
void Validator::_isValidArgs(directive_t &dir, rule_t rule)
{
    int32_t n = dir.args.size();

    if (n < rule.min_args)
    {
        throw ParserError("diretives '" + dir.name + "' requires at least " + SSTR(rule.min_args) + " argument(s)", dir.line, dir.colm);
    }

    if (n > rule.max_args && rule.max_args != -1)
    {
        throw ParserError("diretives '" + dir.name + "' can have at most " + SSTR(rule.max_args) + " argument(s)", dir.line, dir.colm);
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
