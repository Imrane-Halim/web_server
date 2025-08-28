#ifndef WEBSERV_LOGGER_HPP
#define WEBSERV_LOGGER_HPP

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>
#include <exception>
#include <cerrno>
#include <cstring>

/**
 * @brief A flexible logging utility that supports both console and file output.
 * 
 * The Logger class provides formatted logging with timestamps, colored output
 * (when writing to console), and multiple log levels. It can write to either
 * standard output with ANSI color codes or to a specified file without colors.
 * 
 * Features:
 * - Automatic timestamping in format "[HH:MM:SS DD-MM-YYYY]"
 * - Color-coded log levels for console output
 * - Support for custom log levels with custom colors
 * - Thread-safe for single-threaded applications (not thread-safe for multi-threaded)
 * - Automatic file handling with proper cleanup
 */
class Logger
{
private:
    std::ofstream _outputFile; ///< File stream for file-based logging (empty if console logging)

    /**
     * @brief Generates a formatted timestamp string.
     * @return String containing current time in format "[HH:MM:SS DD-MM-YYYY]"
     * 
     * Uses system time functions to create a consistently formatted timestamp
     * that will be prepended to each log entry.
     */
    std::string _currentTimestamp(void) const;

    /**
     * @brief Formats a log level tag with appropriate styling.
     * @param level The log level text (e.g., "INFO", "ERROR")
     * @param rgbColor RGB color value in format 0xRRGGBB for console coloring
     * @return Formatted level tag with brackets and color codes (if console output)
     * 
     * For console output: applies ANSI color codes using RGB values
     * For file output: returns simple bracketed text without color codes
     * 
     * Color format used: \x1b[38;2;R;G;Bm[LEVEL]\x1b[0m
     */
    std::string _formatLevelTag(const std::string& level, int rgbColor) const;

    /**
     * @brief Composes a complete formatted log line.
     * @param level The log level string (e.g., "INFO", "ERROR")
     * @param message The actual log message content
     * @param rgbColor RGB color value for level tag formatting
     * @return Complete formatted log line: "[timestamp][level]: message"
     * 
     * Combines timestamp, colored level tag, and message into a single
     * formatted string ready for output.
     */
    std::string _formatLogLine(const std::string& level, const std::string& message, int rgbColor) const;

    /**
     * @brief Internal logging function that handles actual output.
     * @param level Log level string for formatting
     * @param msg Message content to log
     * @param color RGB color value for level formatting
     * 
     * Routes output to either file or console based on Logger configuration.
     * Handles the actual writing and ensures proper line termination.
     */
    void _log(const std::string& level, const std::string& msg, int color);

public:
    /**
     * @brief Default constructor for console logging.
     * 
     * Creates a Logger instance that outputs to standard output (console)
     * with ANSI color codes for different log levels.
     */
    Logger();

    /**
     * @brief Destructor that ensures proper file cleanup.
     * 
     * Automatically closes the output file if one was opened,
     * ensuring no file handles are leaked.
     */
    ~Logger();

    /**
     * @brief Constructor for file-based logging.
     * @param filename Path to the log file (will be created if it doesn't exist)
     * @throws std::runtime_error if the file cannot be opened for writing
     * 
     * Creates a Logger instance that appends to the specified file.
     * The file is opened in append mode, so existing content is preserved.
     * No color codes are applied to file output.
     */
    Logger(const std::string& filename);

    /**
     * @brief Logs an informational message.
     * @param message The message content to log
     * 
     * Uses blue color (0x4976ba) for console output.
     * Suitable for general information, status updates, and non-critical notifications.
     */
    void info(const std::string& message);

    /**
     * @brief Logs an error message.
     * @param message The error message content to log
     * 
     * Uses red color (0xc91432) for console output.
     * Should be used for errors, exceptions, and critical failures.
     */
    void error(const std::string& message);

    /**
     * @brief Logs a debug message.
     * @param message The debug message content to log
     * 
     * Uses purple color (0x8800ff) for console output.
     * Intended for detailed debugging information, typically filtered out in production.
     */
    void debug(const std::string& message);

    /**
     * @brief Logs a warning message.
     * @param message The warning message content to log
     * 
     * Uses yellow color (0xffbc11) for console output.
     * For non-critical issues that should be noted but don't prevent operation.
     */
    void warning(const std::string& message);

    /**
     * @brief Logs a success message.
     * @param message The success message content to log
     * 
     * Uses green color (0x138636) for console output.
     * For positive confirmations, successful operations, and achievements.
     */
    void success(const std::string& message);

    /**
     * @brief Logs a message with a custom level and color.
     * @param tag Custom level tag text (e.g., "CUSTOM", "NETWORK", "HTTP")
     * @param msg The message content to log
     * @param rgbColor RGB color value in format 0xRRGGBB
     * 
     * Allows for application-specific log levels with custom formatting.
     * Useful for domain-specific logging (e.g., "HTTP", "CGI", "CONFIG").
     */
    void custom(const std::string& tag, const std::string& msg, int rgbColor);

    /**
     * @brief Formats a log line without outputting it.
     * @param tag Custom level tag text
     * @param msg The message content
     * @param rgbColor RGB color value for formatting
     * @return Formatted log line string
     * 
     * Useful for preparing log messages for later output or for sending
     * to other logging systems. Returns the same format as other log methods
     * but doesn't write to console or file.
     */
    std::string getCustomLine(const std::string& tag, const std::string& msg, int rgbColor);
};

#endif