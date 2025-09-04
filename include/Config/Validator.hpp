#ifndef WEBSERV_VALIDATOR_HPP
#define WEBSERV_VALIDATOR_HPP

#include "Directive.hpp"
#include "Error.hpp"

/**
 * @brief Validates the syntax and context of directives.
 *
 * This class ensures that each directive adheres to the defined grammar rules,
 * including correct argument counts and valid context usage.
 */
class Validator
{
    /**
     * @brief Represents validation rules for a directive.
     *
     * This structure holds the allowed contexts and the minimum and maximum
     * argument counts for a directive.
     */
    struct rule_t
    {
        uint8_t allowed_context; /**< Allowed contexts for the directive. */
        int8_t  min_args;        /**< Minimum number of arguments required. */
        int8_t  max_args;        /**< Maximum number of arguments allowed. */
    };

    std::map<std::string, rule_t> _rules; /**< Map of directive names to their validation rules. */

    /**
     * @brief Checks if a directive is valid within a specific context.
     *
     * @param dir The directive to be checked.
     * @param allowed The context in which the directive is used.
     */
    void _isValidInContext(directive_t& dir, uint8_t allowed);

    /**
     * @brief Checks if a directive has a valid number of arguments.
     *
     * @param dir The directive to be checked.
     * @param rules The validation rules to be applied.
     */
    void _isValidArgs(directive_t& dir, rule_t rules);

public:
    /**
     * @brief Constructs a Validator with predefined rules.
     *
     * Initializes the validation rules for various directives.
     */
    Validator();

    /**
     * @brief Evaluates the validity of a directive.
     *
     * @param dir The directive to be evaluated.
     */
    void evaluate(directive_t& dir);
};

#endif