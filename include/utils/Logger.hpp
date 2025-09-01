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
 * @brief Logging utility supporting console and file output with formatting.
 */
class Logger
{
private:
    std::ofstream _outputFile; ///< File stream for file logging (empty if console)

    /** Returns current timestamp in "[HH:MM:SS DD-MM-YYYY]" format. */
    std::string _currentTimestamp(void) const;

    /**
     * Formats a log level tag.
     * @param level Log level text (e.g., "[INFO]")
     * @param rgbColor RGB color for console output
     * @return Formatted level tag with color if console output
     */
    std::string _formatLevelTag(const std::string& level, int rgbColor) const;

    /**
     * Combines timestamp, level tag, and message into a log line.
     * @param level Log level text
     * @param message Message content
     * @param rgbColor Color for level tag
     * @return Formatted log line string
     */
    std::string _formatLogLine(const std::string& level, const std::string& message, int rgbColor) const;

    /**
     * Writes the log line to file or console.
     * @param level Log level text
     * @param msg Message content
     * @param color RGB color for level tag
     */
    void _log(const std::string& level, const std::string& msg, int color);

public:
    /** Constructs Logger for console output with colors. */
    Logger();

    /** Constructs Logger to write to specified file. */
    Logger(const std::string& filename);

    /** Cleans up resources (closes file if open). */
    ~Logger();

    /** Logs informational message (blue). */
    void info(const std::string& message);

    /** Logs error message (red). */
    void error(const std::string& message);

    /** Logs debug message (purple). */
    void debug(const std::string& message);

    /** Logs warning message (yellow). */
    void warning(const std::string& message);

    /** Logs success message (green). */
    void success(const std::string& message);

    /**
     * Logs a custom level message with specified color.
     * @param tag Custom level tag
     * @param msg Message content
     * @param rgbColor RGB color value
     */
    void custom(const std::string& tag, const std::string& msg, int rgbColor);

    /**
     * Formats a custom log line without outputting it.
     * @param tag Custom level tag
     * @param msg Message content
     * @param rgbColor RGB color value
     * @return Formatted log line string
     */
    std::string getCustomLine(const std::string& tag, const std::string& msg, int rgbColor);
};

#endif