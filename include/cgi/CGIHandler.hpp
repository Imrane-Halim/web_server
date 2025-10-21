#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include "EventHandler.hpp"
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <fstream>
#include <signal.h>
#include "Pipe.hpp"
#include "HTTPParser.hpp"
#include "Epoll.hpp"
#include "Response.hpp"
#include "FdManager.hpp"
#include "Routing.hpp"
#include <ctype.h>
#include "../utils/Logger.hpp"
#define BUFFER_SIZE 4096

// Helper function to convert int to string
inline std::string intToString(int value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

class CGIHandler : public EventHandler
{
    private:
        std::string _scriptPath;
        std::vector<char *> _env;
        std::vector<char *> _argv;
        Pipe _inputPipe;
        Pipe _outputPipe;
        pid_t _pid;
        int status;
        bool responseStarted;
        HTTPParser &_Reqparser;
		HTTPParser _cgiParser;
		HTTPResponse &_response;
        bool _isRunning;
		bool  _needBody;

        void push_interpreter_if_needed();
        void initEnv(HTTPParser &parser);
        void initArgv(RouteMatch const& match);
    public:
        CGIHandler(HTTPParser &parser, HTTPResponse &response, ServerConfig &config, FdManager &fdm);
        ~CGIHandler();
        int get_fd();
        void start(const RouteMatch& match, bool needBody);
        void onEvent(uint32_t events);
        void onReadable();
        void onWritable();
        void onError();
        bool isRunning() const;
        void end();
};

void CGIHandler::onEvent(uint32_t events)
{
    if (IS_ERROR_EVENT(events))
    {
        onError();
        return;
    }
    if (IS_READ_EVENT(events))
        onReadable();
    if (IS_WRITE_EVENT(events))
        onWritable();
}

void CGIHandler::onReadable()
{
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead = _outputPipe.read(buffer, BUFFER_SIZE);
    
    if (bytesRead < 0)
    {
        // Handle read error
        Logger logger;
        logger.error("CGI read error");
        onError();
        return;
    }
    else if (bytesRead == 0)
    {
        // EOF, CGI process has finished output
        _fd_manager.detachFd(_outputPipe.read_fd());
        _outputPipe.closeRead();
        
        // Check if CGI parser is complete and valid
        if (!_cgiParser.isComplete() && _cgiParser.getState() != HEADERS)
        {
            Logger logger;
            logger.error("CGI output incomplete or malformed");
            _response.reset();
            _response.startLine(502);
            _response.setBody("<html><body><h1>502 Bad Gateway</h1><p>CGI script did not produce valid output</p></body></html>");
        }
        
        _isRunning = false;
        return;
    }
    else
    {
        // Parse CGI output headers
        _cgiParser.addChunk(buffer, bytesRead);
        
        if (_cgiParser.isError())
        {
            Logger logger;
            logger.error("CGI response parsing error");
            _response.reset();
            _response.startLine(502);
            _response.setBody("<html><body><h1>502 Bad Gateway</h1><p>Invalid CGI response</p></body></html>");
            onError();
            return;
        }
        
        // If this is the first time we're getting data, start the response
        if (!responseStarted && _cgiParser.getState() >= BODY)
        {
            responseStarted = true;
            
            // Check for Status header in CGI response
            std::string statusHeader = _cgiParser.getHeader("status");
            int statusCode = 200;
            if (!statusHeader.empty())
            {
                statusCode = ft_atoi<int>(statusHeader.substr(0, 3));
            }
            
            _response.startLine(statusCode);
            
            // Copy headers from CGI output to response
            strmap& headers = _cgiParser.getHeaders();
            for (strmap::iterator it = headers.begin(); it != headers.end(); ++it)
            {
                if (it->first != "status")  // Skip status header
                    _response.addHeader(it->first, it->second);
            }
            
            _response.endHeaders();
        }
        
        // Feed body data to response
        if (_cgiParser.getState() == BODY || _cgiParser.getState() == COMPLETE)
        {
            _response.feedRAW(buffer, bytesRead);
        }
    }
}

void CGIHandler::onWritable()
{
    if (!_needBody || _Reqparser.getBody().getSize() == 0)
    {
        // No more data to write, close the input pipe write end
        _fd_manager.detachFd(_inputPipe.write_fd());
        _inputPipe.closeWrite();
        return;
    }
    
    size_t available = _Reqparser.getBody().getCapacity() - _Reqparser.getBody().getSize();
    if (available == 0)
    {
        // All body data has been sent
        _fd_manager.detachFd(_inputPipe.write_fd());
        _inputPipe.closeWrite();
        return;
    }
    
    char buffer[BUFFER_SIZE];
    size_t toWrite = (available > BUFFER_SIZE) ? BUFFER_SIZE : available;
    size_t bytesRead = _Reqparser.getBody().read(buffer, toWrite);
    
    if (bytesRead > 0)
    {
        ssize_t bytesWritten = _inputPipe.write(buffer, bytesRead);
        if (bytesWritten < 0)
        {
            // Write error
            Logger logger;
            logger.error("CGI write error");
            onError();
            return;
        }
    }
    
    // Check if we've written all the body data
    if (_Reqparser.getBody().getSize() == 0)
    {
        _fd_manager.detachFd(_inputPipe.write_fd());
        _inputPipe.closeWrite();
    }
}

void CGIHandler::onError()
{
    Logger logger;
    
    // Clean up pipes
    if (_inputPipe.write_fd() != -1)
    {
        _fd_manager.detachFd(_inputPipe.write_fd());
        _inputPipe.closeWrite();
    }
    if (_outputPipe.read_fd() != -1)
    {
        _fd_manager.detachFd(_outputPipe.read_fd());
        _outputPipe.closeRead();
    }
    
    // Check process status
    int waitStatus = 0;
    pid_t result = waitpid(_pid, &waitStatus, WNOHANG);
    
    if (result == 0)
    {
        // Process still running, kill it
        logger.warning("CGI process still running, sending SIGKILL");
        ::kill(_pid, SIGKILL);
        waitpid(_pid, &waitStatus, 0);
    }
    else if (result > 0)
    {
        // Process has exited
        if (WIFEXITED(waitStatus))
        {
            int exitStatus = WEXITSTATUS(waitStatus);
            logger.error("CGI process exited with status: " + intToString(exitStatus));
        }
        else if (WIFSIGNALED(waitStatus))
        {
            int signal = WTERMSIG(waitStatus);
            logger.error("CGI process terminated by signal: " + intToString(signal));
        }
    }
    
    // If response hasn't started yet, send an error response
    if (!responseStarted)
    {
        _response.reset();
        _response.startLine(502);
        _response.setBody("<html><body><h1>502 Bad Gateway</h1><p>CGI script error</p></body></html>");
    }
    
    _isRunning = false;
}

// std::map <std::string, std::string> initInterpreterMap()
// {
//     std::map <std::string, std::string> interpreterMap;
//     // --- Python ---
//     interpreterMap[".py"]   = "/usr/bin/python3";
//     interpreterMap[".py2"]  = "/usr/bin/python2";
//     interpreterMap[".py3"]  = "/usr/bin/python3";

//     // --- Perl ---
//     interpreterMap[".pl"]   = "/usr/bin/perl";
//     interpreterMap[".cgi"]  = "/usr/bin/perl";   // legacy CGI often Perl

//     // --- Ruby ---
//     interpreterMap[".rb"]   = "/usr/bin/ruby";

//     // --- PHP ---
//     interpreterMap[".php"]  = "/usr/bin/php";

//     // --- Shell / POSIX ---
//     interpreterMap[".sh"]   = "/bin/sh";
//     interpreterMap[".bash"] = "/bin/bash";
//     interpreterMap[".ksh"]  = "/bin/ksh";
//     interpreterMap[".csh"]  = "/bin/csh";
//     interpreterMap[".tcsh"] = "/bin/tcsh";
//     interpreterMap[".zsh"]  = "/bin/zsh";

//     // --- Tcl ---
//     interpreterMap[".tcl"]  = "/usr/bin/tclsh";

//     // --- Lua ---
//     interpreterMap[".lua"]  = "/usr/bin/lua";

//     // --- JavaScript / Node.js ---
//     interpreterMap[".js"]   = "/usr/bin/node";
//     interpreterMap[".mjs"]  = "/usr/bin/node";
//     interpreterMap[".cjs"]  = "/usr/bin/node";

//     // --- awk / sed ---
//     interpreterMap[".awk"]  = "/usr/bin/awk";
//     interpreterMap[".sed"]  = "/bin/sed";

//     // --- R language ---
//     interpreterMap[".r"]    = "/usr/bin/Rscript";

//     // --- Java ---
//     interpreterMap[".java"] = "/usr/bin/java";       // needs compiled class
//     interpreterMap[".jar"]  = "/usr/bin/java -jar";  // run JAR directly

//     // --- Scala / Kotlin (JVM-based) ---
//     interpreterMap[".scala"]  = "/usr/bin/scala";
//     interpreterMap[".kt"]     = "/usr/bin/kotlinc";   // compile first
//     interpreterMap[".kts"]    = "/usr/bin/kotlin";    // Kotlin script

//     // --- Groovy ---
//     interpreterMap[".groovy"] = "/usr/bin/groovy";

//     // --- Haskell ---
//     interpreterMap[".hs"]   = "/usr/bin/runhaskell";

//     // --- Scheme / Lisp ---
//     interpreterMap[".scm"]  = "/usr/bin/guile";
//     interpreterMap[".ss"]   = "/usr/bin/guile";
//     interpreterMap[".lisp"] = "/usr/bin/clisp";

//     // --- OCaml ---
//     interpreterMap[".ml"]   = "/usr/bin/ocaml";

//     // --- Erlang / Elixir ---
//     interpreterMap[".erl"]  = "/usr/bin/escript";
//     interpreterMap[".exs"]  = "/usr/bin/elixir";

//     // --- Julia ---
//     interpreterMap[".jl"]   = "/usr/bin/julia";

//     // --- Go (script mode with yaegi / go run) ---
//     interpreterMap[".go"]   = "/usr/bin/go run";

//     // --- Swift ---
//     interpreterMap[".swift"] = "/usr/bin/swift";

//     // --- Dart ---
//     interpreterMap[".dart"]  = "/usr/bin/dart";

//     // --- PowerShell (cross-platform) ---
//     interpreterMap[".ps1"]   = "/usr/bin/pwsh";      // PowerShell Core
//     // Windows may use: "C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe"

//     // --- Miscellaneous ---
//     interpreterMap[".raku"]  = "/usr/bin/raku";   // formerly Perl 6
//     interpreterMap[".cr"]    = "/usr/bin/crystal";
//     interpreterMap[".nim"]   = "/usr/bin/nim";    // usually compiles to binary

//     return interpreterMap;
// }

// void CGIHandler::push_interpreter_if_needed()
// {
//     std::string interpreter;
//     std::string extension;
//     std::map <std::string, std::string> interpreterMap;

//     if (access(_scriptPath.c_str(), X_OK) != 0)
//         return ; // not runnable at all

//     // 2. Read first two bytes
//     std::ifstream file(_scriptPath, std::ios::binary);
//     if (!file) return;

//     char header[2];
//     file.read(header, 2);

//     // Case A: shebang present -> kernel handles it
//     if (header[0] == '#' && header[1] == '!') {
//         interpreter.clear();  // no interpreter needed from map
//         return  ;
//     }

//     // Case B: ELF binary -> no interpreter
//     if ((unsigned char)header[0] == 0x7f && header[1] == 'E') {
//         interpreter.clear();
//         return ;
//     }

//     size_t dotPos = _scriptPath.rfind('.');
//     if (dotPos != std::string::npos)
//     {
//         extension = _scriptPath.substr(dotPos);
//         interpreterMap = initInterpreterMap();
//         if (interpreterMap.find(extension) != interpreterMap.end())
//         {
//             interpreter = interpreterMap[extension];
//             _argv.push_back(const_cast<char *>(interpreter.c_str()));
//         }
//         return;
//     }
//     throw std::runtime_error(_scriptPath + ": Unknown script type or no interpreter found");
//     return;
// }

void CGIHandler::initArgv(RouteMatch const& match)
{
    //implement look up for interpreter if needed
    _argv.clear();
    if (!match.scriptInterpreter.empty())
        _argv.push_back(const_cast<char *>(match.scriptInterpreter.c_str()));
    _argv.push_back(const_cast<char *>(match.scriptPath.c_str()));
    _argv.push_back(NULL); // Null-terminate for execve
}

void CGIHandler::initEnv(HTTPParser &parser)
{
    std::vector<std::string> envStrings;

    envStrings.push_back("GATEWAY_INTERFACE=CGI/1.1");
    envStrings.push_back("SERVER_PROTOCOL=" + parser.getVers());
    envStrings.push_back("REQUEST_METHOD=" + parser.getMethod());
    //envStrings.push_back("REQUEST_URI=" + parser.getUri());
    envStrings.push_back("QUERY_STRING=" + parser.getQuery());
    // Add headers as HTTP_ environment variables
    strmap headers = parser.getHeaders();
    for (strmap::const_iterator it = headers.begin(); it != headers.end(); ++it)
    {
        std::string key = it->first;
        std::transform(key.begin(), key.end(), key.begin(), ::toupper);
        std::replace(key.begin(), key.end(), '-', '_');
        envStrings.push_back("HTTP_" + key + "=" + it->second);
    }

    // Content-Length and Content-Type
    if (headers.find("content-length") != headers.end())
        envStrings.push_back("CONTENT_LENGTH=" + headers["content-length"]);
    if (headers.find("content-type") != headers.end())
        envStrings.push_back("CONTENT_TYPE=" + headers["content-type"]);

    // Allocate and store char* pointers
    _env.clear();
    for (size_t i = 0; i < envStrings.size(); ++i)
    {
        char* envCStr = new char[envStrings[i].size() + 1];
        std::strcpy(envCStr, envStrings[i].c_str());
        _env.push_back(envCStr);
    }
    _env.push_back(NULL); // Null-terminate for execve
}

CGIHandler::CGIHandler(HTTPParser &parser, HTTPResponse &response, ServerConfig &config, FdManager &fdm)
    : EventHandler(config, fdm),
      _scriptPath(""),
      _pid(-1),
      status(0),
      responseStarted(false),
      _Reqparser(parser),
      _cgiParser(),
      _response(response),
      _isRunning(false),
      _needBody(false)
{
    _cgiParser.setCGIMode(true);
}

CGIHandler::~CGIHandler()
{
    end();
}

void CGIHandler::start(const RouteMatch& match, bool needBody)
{
    if (_isRunning)
        return;
    
    _needBody = needBody;
    _scriptPath = match.scriptPath;
    
    try
    {
        initArgv(match);
        initEnv(_Reqparser);
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Failed to initialize CGI: " + std::string(e.what()));
    }
    
    _pid = fork();
    if (_pid < 0)
    {
        throw std::runtime_error("Failed to fork process for CGI");
    }
    else if (_pid == 0)
    {
        // Child process
        // Redirect stdin to input pipe
        if (dup2(_inputPipe.read_fd(), STDIN_FILENO) == -1)
        {
            std::perror("dup2 stdin");
            std::exit(1);
        }
        
        // Redirect stdout to output pipe
        if (dup2(_outputPipe.write_fd(), STDOUT_FILENO) == -1)
        {
            std::perror("dup2 stdout");
            std::exit(1);
        }
        
        // Close all pipe file descriptors in child
        _inputPipe.close();
        _outputPipe.close();
        
        // Execute the CGI script
        execve(_argv[0], _argv.data(), _env.data());
        
        // If execve fails
        std::perror("execve");
        std::exit(1);
    }
    else
    {
        // Parent process
        // Close unused pipe ends
        _inputPipe.closeRead();
        _outputPipe.closeWrite();
        
        // Set pipes to non-blocking mode
        _inputPipe.set_non_blocking();
        _outputPipe.set_non_blocking();
        
        // Register pipes with epoll
        if (needBody && _Reqparser.getBody().getSize() > 0)
        {
            _fd_manager.add(_inputPipe.write_fd(), this, EPOLLOUT);
        }
        else
        {
            // No body to send, close write end immediately
            _inputPipe.closeWrite();
        }
        
        _fd_manager.add(_outputPipe.read_fd(), this, EPOLLIN);
        
        _isRunning = true;
    }
}

int CGIHandler::get_fd()
{
    // Return the file descriptor being monitored
    // This could be either the input (write) or output (read) pipe
    // depending on what's currently active
    if (_outputPipe.read_fd() != -1)
        return _outputPipe.read_fd();
    if (_inputPipe.write_fd() != -1)
        return _inputPipe.write_fd();
    return -1;
}

bool CGIHandler::isRunning() const
{
    return _isRunning;
}


void CGIHandler::end()
{
    if (!_isRunning)
        return;

    ::kill(_pid, SIGKILL);
    int status;
    waitpid(_pid, &status, 0);
    _isRunning = false;

    for (size_t i = 0; i < _env.size() - 1; ++i)
        delete[] _env[i];
    _env.clear();
}


#endif //CGI_HANDLER_HPP
 