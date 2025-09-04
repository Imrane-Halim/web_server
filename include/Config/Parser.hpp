#ifndef WEBSERV_PARSER_HPP
#define WEBSERV_PARSER_HPP

#include "Directive.hpp"
#include "lexer.hpp"
#include "Validator.hpp"
#include "Error.hpp"

/**
 * @brief Parses a source file into a series of directives.
 *
 * This class is responsible for reading a source file, analyzing its content,
 * and converting it into a structured representation of directives.
 */
class Parser
{
    lexer       _file;        /**< The lexer instance for tokenizing the input file. */
    Validator   _grammar;     /**< The grammar validator for checking directive syntax. */
    int         _nestDepth;   /**< The current nesting depth of directives. */
    context_t   _currCtx;     /**< The current parsing context. */

    /**
     * @brief Parses the arguments of a directive.
     * @param dir The directive whose arguments are to be parsed.
     * @return The parsed arguments as a token.
     */
    token_t _parseArgs(directive_t& dir);

    /**
     * @brief Parses a block-level directive.
     * @param dir The directive to be parsed.
     * @return The parsed directive.
     */
    directive_t _parseBlock(directive_t& dir);

    /**
     * @brief Parses a single directive.
     * @return The parsed directive.
     */
    directive_t _parseDirective();

public:
    /**
     * @brief Constructs a Parser for a given file.
     * @param filename The name of the file to be parsed.
     */
    Parser(const std::string& filename);

    /**
     * @brief Parses the entire file into directives.
     * @return The top-level directive representing the parsed content.
     */
    directive_t parse();
};

#endif