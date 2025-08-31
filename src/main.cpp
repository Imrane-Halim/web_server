#include <iostream>
#include "lexer.hpp"

std::string getType(token_types_t type)
{
    switch (type)
    {
    case TKN_WORD:
        return "TKN_WORD";
    case TKN_SEMCLN:
        return "TKN_SEMCLN";
    case TKN_COMMA:
        return "TKN_COMMA";
    case TKN_LCBRAC:
        return "TKN_LCBRAC";
    case TKN_RCBRAC:
        return "TKN_RCBRAC";
    case TKN_TILDA:
        return "TKN_TILDA";
    case TKN_QUOTED:
        return "TKN_QUOTED";
    case TKN_EOF:
        return "TKN_EOF";
    }
    return "UNKNOWN";
}

int main(int ac, char **av)
{
    if (ac != 2)
    {
        std::cerr << "usage: ./webserv [CONFIG]" << std::endl;
        return 1;
    }

    lexer conf(av[1]);
    token_t tkn;

    int indent = 0;
    while ((tkn = conf.getNextToken()).type != TKN_EOF)
    {
        if (tkn.type == TKN_RCBRAC) indent -= 4;
        for (int i = 0; i < indent; ++i) std::cout << ' ';
        std::cout << getType(tkn.type) << ": " << tkn.value << std::endl;
        if (tkn.type == TKN_LCBRAC) indent += 4;
    }

    return 0;
}