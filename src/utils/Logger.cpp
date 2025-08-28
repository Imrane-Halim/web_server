#include "Logger.hpp"

Logger::Logger() 
{
    // Console logging mode - no file stream initialization needed
}

Logger::Logger(const std::string& filepath) :
    _outputFile(filepath.c_str(), std::ofstream::out | std::ofstream::app)
{
    if (!_outputFile.is_open())
    {
        std::string errorMsg = "Failed to open log file '" + filepath + "': " + std::strerror(errno);
        throw std::runtime_error(errorMsg);
    }
}

Logger::~Logger()
{
    if (_outputFile.is_open())
        _outputFile.close();
}

std::string Logger::_currentTimestamp(void) const
{
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    char buff[32] = {0};
    size_t bytes = strftime(buff, sizeof(buff), "[%H:%M:%S %d-%m-%Y]", timeinfo);

    if (bytes == 0)
        return "[TIMESTAMP_ERROR]";

    return std::string(buff);
}

std::string Logger::_formatLevelTag(const std::string& level, int color) const
{
    if (_outputFile.is_open())
        return "[" + level + "]";
    else
    {
        // Console output with ANSI color codes
        // Format: \x1b[38;2;R;G;Bm[LEVEL]\x1b[0m
        
        int r = (color >> 16) & 0xFF;
        int g = (color >> 8)  & 0xFF;
        int b = color & 0xFF;
        
        std::ostringstream oss;
        oss << "[\x1b[38;2;" << r << ";" << g << ";" << b << "m" 
            << level 
            << "\x1b[0m]";
        
        return oss.str();
    }
}

std::string Logger::_formatLogLine(const std::string& type, const std::string& msg, int color) const
{
    return _currentTimestamp() + _formatLevelTag(type, color) + ": " + msg;
}

void Logger::_log(const std::string& level, const std::string& msg, int color) 
{
    std::string line = _formatLogLine(level, msg, color);
    
    if (_outputFile.is_open())
    {
        _outputFile << line << std::endl;
        // immediate write to file (important for crash scenarios)
        _outputFile.flush();
    }
    else
        std::cout << line << std::endl;
}

// Predefined log levels with their respective colors
void Logger::info(const std::string& msg)    { _log("INFO", msg, 0x4976ba); }
void Logger::error(const std::string& msg)   { _log("ERROR", msg, 0xc91432); }
void Logger::success(const std::string& msg) { _log("SUCCESS", msg, 0x138636); }
void Logger::warning(const std::string& msg) { _log("WARNING", msg, 0xffbc11); }
void Logger::debug(const std::string& msg)   { _log("DEBUG", msg, 0x8800ff); }

void Logger::custom(const std::string& tag, const std::string& msg, int rgbColor) 
{ 
    _log(tag, msg, rgbColor); 
}

std::string Logger::getCustomLine(const std::string& tag, const std::string& msg, int rgbColor) 
{ 
    return _formatLogLine(tag, msg, rgbColor); 
}
