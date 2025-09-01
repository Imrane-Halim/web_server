#include "Parser.hpp"
#include <iostream>

Parser::ParserError::ParserError(const std::string& msg, unsigned int line, unsigned int colm)
{
    std::ostringstream error;
    error << line << ":" << colm << ": " << msg;
    _error = error.str().c_str();
}
Parser::ParserError::~ParserError() throw()
{
    
}
const char* Parser::ParserError::what() const throw()
{
    return _error.c_str();
}


Parser::Parser(const std::string& filepath):
    _file(filepath.c_str())
{

}

// parse a single block
// note: 'getNextToken' is a lazy lexer method, 
// each call returns a new token from file
directive_t Parser::_parseDirective()
{
    directive_t dir;
    token_t     token = _file.getNextToken();

    // '}' means end of a block
    if (token.type == TKN_RCBRAC) return dir;
    if (token.type == TKN_EOF) return dir;

    // dires start with there name
    if (token.type != TKN_WORD)
        throw ParserError("unexpected token '" + token.value + "'", _file.getCurrLine(), _file.getCurrColm());
        
    // assign name to dir    
    if (dir.name.empty())
        dir.name = token.value;

    // get arguments
    while ((token = _file.getNextToken()).type != TKN_EOF)
    {
        if (token.type == TKN_WORD || token.type == TKN_QUOTED || token.type == TKN_TILDA)
            dir.args.push_back(token.value);
        if (token.type == TKN_SEMCLN) break;
        if (token.type == TKN_LCBRAC) break;
    }

    if (token.type == TKN_LCBRAC)
    {
        dir.is_block = true;
        while (true)
        {
            directive_t child = _parseDirective();
            if (child.name.empty()) break;
            dir.children.push_back(child);
        }
    }

    return dir;
}

directive_t Parser::buildTree(void)
{
    directive_t mainContext;
    directive_t tmp;

    mainContext.name = "main";
    mainContext.is_block = true;
    // keep parsing directives until no more
    while((tmp = _parseDirective()).name != "")
        mainContext.children.push_back(tmp);
    return mainContext;
}