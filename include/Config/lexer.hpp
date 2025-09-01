#ifndef WEBSERV_LEXER_HPP
#define WEBSERV_LEXER_HPP

#include <fstream>
#include <cstring>
#include <exception>
#include <string>
#include <sstream>
#include <stdint.h>

#define SYMBOLS "~;{}"

/**
 * @brief Token types for webserv configuration parsing.
 * Defines possible token types encountered during lexical analysis.
 */
enum token_types_t
{
    TKN_WORD    = 0,
    TKN_SEMCLN  = ';',
    TKN_LCBRAC  = '{',
    TKN_RCBRAC  = '}',
    TKN_TILDA   = '~',      ///< Tilde '~' (e.g., for regex)
    TKN_QUOTED,             ///< Quoted strings
    TKN_EOF
};

/**
 * @brief Represents a single lexical token.
 */
struct token_t
{
    token_types_t   type;   ///< Token classification
    std::string     value;  ///< Token content (quotes removed if quoted)
};

/**
 * @brief Lazy lexer for webserv configuration files.
 * Reads the input file and produces tokens on demand.
 */
class lexer
{
private:
    std::ifstream   _inFile;    ///< Input file stream

    uint32_t    _currLine;  ///< Current line number (1-based)
    uint32_t    _currColm;  ///< Column where current token started (1-based)
    uint32_t    _scanColm;  ///< Current scanning column (1-based)

public:
    /**
     * @brief Constructs a lexer for a configuration file.
     * @param filepath Path to the file to tokenize
     * @throws std::runtime_error if file cannot be opened
     */
    lexer(const std::string& filepath);

    /**
     * @brief Closes the input file on destruction.
     */
    ~lexer();

    /**
     * @brief Returns the next token from the input.
     * @return Next token (type and value)
     */
    token_t getNextToken(void);

    /**
     * @brief Gets current line number for error reporting.
     * @return Current line (1-based)
     */
    uint32_t getCurrLine(void) const;

    /**
     * @brief Gets the column where the current token started.
     * @return Column number (1-based)
     */
    uint32_t getCurrColm(void) const;
};


#endif