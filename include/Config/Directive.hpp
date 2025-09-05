#ifndef WEBSERV_DIRECTIVE_HPP
#define WEBSERV_DIRECTIVE_HPP

#include <string>
#include <vector>
#include <stdint.h>
#include <map>

enum context_t
{
    CTX_MAIN     = (1 << 0),
    CTX_SERVER   = (1 << 1),
    CTX_LOCATION = (1 << 2)
};

enum arg_type_t
{
    ARG_NAN  = (1 << 0),
    ARG_STR  = (1 << 1),
    ARG_INT  = (1 << 2),
    ARG_BOOL = (1 << 3),
    ARG_ANY  = (1 << 4),
    ARG_HOST = (1 << 5),
    ARG_PATH = (1 << 7),
    ARG_METHODS = (1 << 6)
};

struct directive_t
{
    std::string              name;
    std::vector<std::string> args;
    bool                     is_block;
    std::vector<directive_t> children;
    uint32_t                 line;
    uint32_t                 colm;
    context_t                ctx;
};

// class IDirective
// {
//     std::string              _name;
//     std::vector<std::string> _args;
//     IDirective*              _parent;

//     uint32_t    _line;
//     uint32_t    _colm;

// public:
//     IDirective();
//     ~IDirective();

//     const std::string&               getName(void) const;
//     const std::vector<std::string>&  getArgs(void) const;

//     void    setName(const std::string& name);
//     void    setName(const std::vector<std::string>& args);

//     IDirective* getParent(void) const;
//     void        setParent(IDirective * prnt);

//     uint32_t    getLine(void) const;
//     uint32_t    getColm(void) const;

//     void        setLine(uint32_t line);
//     void        setColm(uint32_t colm);
// };

// class IBlock: public IDirective
// {
// public:
//     IBlock();
//     ~IBlock();

//     // server can have multiple 'listen' directives
//     std::vector<IDirective*>& getDirectives(const std::string& name) const;
//     std::vector<IDirective*>& getDirectives(void) const;

//     IDirective* getParent(void) const;
//     void        setParent(IDirective* dir) const;
// };

#endif