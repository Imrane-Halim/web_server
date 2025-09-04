#include "Parser.hpp"

Parser::Parser(const std::string &filename) : _file(filename.c_str()),
                                              _grammar(),
                                              _nestDepth(0),
                                              _currCtx(CTX_MAIN)
{
}

token_t Parser::_parseArgs(directive_t &dir)
{
    token_t tkn;

    while ((tkn = _file.getNextToken()).type != TKN_EOF)
    {
        if (tkn.type == TKN_SEMCLN || tkn.type == TKN_LCBRAC)
            break;
        dir.args.push_back(tkn.value);
    }

    return tkn;
}

directive_t Parser::_parseBlock(directive_t &dir)
{
    dir.is_block = true;
    ++_nestDepth;

    context_t savedCtx = _currCtx;
    if (dir.name == "server")
        _currCtx = CTX_SERVER;
    if (dir.name == "location")
        _currCtx = CTX_LOCATION;

    while (true)
    {
        directive_t child = _parseDirective();
        if (child.name.empty())
            break;
        dir.children.push_back(child);
    }

    _currCtx = savedCtx;

    return dir;
}

directive_t Parser::_parseDirective()
{
    directive_t dir;
    token_t tkn = _file.getNextToken();

    if (tkn.type == TKN_EOF)
        return dir;
    if (tkn.type == TKN_RCBRAC)
    {
        if (_nestDepth > 0)
            --_nestDepth;
        else
            throw ParserError("'}' has no matching '{'", tkn.line, tkn.colm);
        return dir;
    }

    if (tkn.type != TKN_WORD)
        throw ParserError("unexpected token '" + tkn.value + "'", tkn.line, tkn.colm);

    dir.name = tkn.value;
    dir.line = tkn.line;
    dir.colm = tkn.colm;
    dir.ctx = _currCtx;

    token_t terminator = _parseArgs(dir);

    _grammar.evaluate(dir);

    if (dir.name == "server" || dir.name == "location")
    {
        if (terminator.type != TKN_LCBRAC)
            throw ParserError("directive '" + dir.name + "' must be a block", dir.line, dir.colm);
    }
    else
    {
        if (terminator.type == TKN_LCBRAC)
            throw ParserError("directive '" + dir.name + "' must not be a block", dir.line, dir.colm);
    }
    if (terminator.type == TKN_LCBRAC)
        _parseBlock(dir);
    if (terminator.type == TKN_EOF)
        throw ParserError("unexpected EOF '" + dir.name + "'", dir.line, dir.colm);

    return dir;
}

directive_t Parser::parse(void)
{
    directive_t main;

    main.name = "main";
    main.is_block = true;
    main.ctx = CTX_MAIN;

    while (true)
    {
        directive_t child = _parseDirective();
        if (child.name.empty())
            break;
        main.children.push_back(child);
    }
    if (_nestDepth != 0)
        throw ParserError("'{' has no matching '}'",
                          _file.getCurrLine(), _file.getCurrColm());
    return main;
}