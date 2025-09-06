#include <iostream>
#include "Logger.hpp"
#include "Parser.hpp"

// print simple directives
void    print_simple(directives simple, int indent = 4)
{
    directives::iterator it;

    for (it = simple.begin(); it != simple.end(); ++it)
    {
        for (int i = 0; i < indent; ++i) std::cout << " ";
        std::cout << it->first << ": ";
        for (size_t j = 0; j < it->second.size(); ++j) 
            std::cout << it->second[j] << " ";
        std::cout << std::endl;
    }
}

void    print_block(std::vector<locationConf> lc)
{
    for (size_t i = 0; i < lc.size(); ++i)
    {
        std::cout << "    location: {\n";
        print_simple(lc[i], 4 * 2);
        std::cout << "    }\n";
    }
}

void    print_conf(Config cnf)
{
    for (size_t i = 0; i < cnf.size(); ++i)
    {
        std::cout << "server: {\n";
        print_simple(cnf[i].first);
        print_block(cnf[i].second);
        std::cout << "}\n";
    }
}

int main(int ac, char **av)
{
    if (ac != 2)
    {
        std::cerr << "usage: ./webserv [CONFIG]" << std::endl;
        return 1;
    }

    Logger console;
    try {
        Config cnf = Parser::parse(av[1]);
        print_conf(cnf);
    } catch (const std::exception& e) {
        console.error(e.what());
    }

    return 0;
}