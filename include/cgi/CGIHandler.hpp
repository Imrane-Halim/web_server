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
std::string intToString(int value);

class CGIHandler : public EventHandler
{
private:
	std::string _scriptPath;
	std::string _interpreterPath; // Store copy for argv
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
	bool _needBody;

	void push_interpreter_if_needed();
	void initEnv(HTTPParser &parser);
	void initArgv(RouteMatch const &match);

public:
	CGIHandler(HTTPParser &parser, HTTPResponse &response, ServerConfig &config, FdManager &fdm);
	~CGIHandler();
	int get_fd();
	int getStatus();
	void start(const RouteMatch &match);
	void onEvent(uint32_t events);
	void onReadable();
	void onWritable();
	void onError();
	bool isRunning() const;
	void end();
	void reset();
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
			// _response.reset();
			// _response.startLine(502);
			// _response.setBody("<html><body><h1>502 Bad Gateway</h1><p>CGI script did not produce valid output</p></body></html>");
			status = 502;
			onError();
			return;
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
			// _response.reset();
			// _response.startLine(502);
			// _response.setBody("<html><body><h1>502 Bad Gateway</h1><p>Invalid CGI response</p></body></html>");
			status = 502;
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
			strmap &headers = _cgiParser.getHeaders();
			for (strmap::iterator it = headers.begin(); it != headers.end(); ++it)
			{
				if (it->first != "status") // Skip status header
					_response.addHeader(it->first, it->second);
			}

			_response.endHeaders();
		}

		// Feed body data to response (only the actual body, not headers)
		if (_cgiParser.getState() == BODY || _cgiParser.getState() == COMPLETE)
		{
			RingBuffer body = _cgiParser.getBody();
			size_t bodySize = body.getSize();
			if (bodySize > 0)
			{
				char bodyBuffer[BUFFER_SIZE];
				size_t toRead = (bodySize > BUFFER_SIZE) ? BUFFER_SIZE : bodySize;
				size_t bytesRead = body.read(bodyBuffer, toRead);
				if (bytesRead > 0)
				{
					_response.feedRAW(bodyBuffer, bytesRead);
				}
			}
		}
	}
}

void CGIHandler::onWritable()
{
	if (!_needBody)
	{
		// No body to send, close the input pipe write end
		_fd_manager.detachFd(_inputPipe.write_fd());
		_inputPipe.closeWrite();
		return;
	}

	RingBuffer body = _Reqparser.getBody();
	size_t available = body.getSize();

	if (available == 0)
	{
		// All body data has been sent
		_fd_manager.detachFd(_inputPipe.write_fd());
		_inputPipe.closeWrite();
		return;
	}

	char buffer[BUFFER_SIZE];
	size_t toWrite = (available > BUFFER_SIZE) ? BUFFER_SIZE : available;
	size_t bytesRead = body.read(buffer, toWrite);

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
	if (body.getSize() == 0)
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

	status = 502;
	_isRunning = false;
}

void CGIHandler::initArgv(RouteMatch const &match)
{
	_argv.clear();

	// Store copies of the strings so we have valid pointers
	if (!match.scriptInterpreter.empty())
	{
		_interpreterPath = match.scriptInterpreter;
		_argv.push_back(const_cast<char *>(_interpreterPath.c_str()));
	}

	_scriptPath = match.scriptPath;
	_argv.push_back(const_cast<char *>(_scriptPath.c_str()));
	_argv.push_back(NULL); // Null-terminate for execve
}

void CGIHandler::initEnv(HTTPParser &parser)
{
	std::vector<std::string> envStrings;

	// Standard CGI environment variables
	envStrings.push_back("GATEWAY_INTERFACE=CGI/1.1");
	envStrings.push_back("SERVER_PROTOCOL=" + parser.getVers());
	envStrings.push_back("REQUEST_METHOD=" + parser.getMethod());
	envStrings.push_back("SCRIPT_NAME=" + parser.getUri());
	envStrings.push_back("SCRIPT_FILENAME=" + _scriptPath);
	envStrings.push_back("QUERY_STRING=" + parser.getQuery());

	// Server information (use defaults if not available from config)
	envStrings.push_back("SERVER_NAME=" + (_config.host.empty() ? "localhost" : _config.host));
	envStrings.push_back("SERVER_PORT=" + intToString(_config.port));
	envStrings.push_back("SERVER_SOFTWARE=WebServ/1.0");

	// Remote address (would need to be passed from connection context)
	// envStrings.push_back("REMOTE_ADDR=127.0.0.1");

	// Get headers
	strmap headers = parser.getHeaders();

	// Content-Length and Content-Type (special handling - don't need HTTP_ prefix)
	if (headers.find("content-length") != headers.end())
		envStrings.push_back("CONTENT_LENGTH=" + headers["content-length"]);
	else
		envStrings.push_back("CONTENT_LENGTH=0");

	if (headers.find("content-type") != headers.end())
		envStrings.push_back("CONTENT_TYPE=" + headers["content-type"]);

	// Adding other headers as HTTP_ environment variables
	for (strmap::const_iterator it = headers.begin(); it != headers.end(); ++it)
	{
		// Skipping Content-Length and Content-Type as they're already handled
		if (it->first == "content-length" || it->first == "content-type")
			continue;

		std::string key = it->first;
		std::transform(key.begin(), key.end(), key.begin(), ::toupper);
		std::replace(key.begin(), key.end(), '-', '_');
		envStrings.push_back("HTTP_" + key + "=" + it->second);
	}

	// Allocate and store char* pointers
	_env.clear();
	for (size_t i = 0; i < envStrings.size(); ++i)
	{
		char *envCStr = new char[envStrings[i].size() + 1];
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

	// Clean up any remaining environment variables
	for (size_t i = 0; i < _env.size(); ++i)
	{
		if (_env[i] != NULL)
			delete[] _env[i];
	}
	_env.clear();
}

void CGIHandler::start(const RouteMatch &match)
{
	if (_isRunning)
		return;

	// reset CGI handler state
	reset();

	_needBody = (_Reqparser.getMethod() == "POST" || _Reqparser.getMethod() == "PUT");
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
		RingBuffer body = _Reqparser.getBody();
		if (_needBody && body.getSize() > 0)
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
		for (size_t i = 0; i < _env.size(); ++i)
		{
			delete[] _env[i];
		}
		_env.clear();
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

	// Kill the child process
	::kill(_pid, SIGKILL);

	// Wait for the process to actually terminate (without WNOHANG)
	int waitStatus;
	waitpid(_pid, &waitStatus, 0);

	_isRunning = false;
}

void CGIHandler::reset()
{
	// End any running CGI process
	end();

	// Clean up pipes if they're still open

	if (_inputPipe.write_fd() != -1)
	{
		_fd_manager.detachFd(_inputPipe.write_fd());
		_inputPipe.closeWrite();
	}
	if (_inputPipe.read_fd() != -1)
	{
		_inputPipe.closeRead();
	}
	if (_outputPipe.read_fd() != -1)
	{
		_fd_manager.detachFd(_outputPipe.read_fd());
		_outputPipe.closeRead();
	}
	if (_outputPipe.write_fd() != -1)
	{
		_outputPipe.closeWrite();
	}

	// Clean up any remaining environment variables
	for (size_t i = 0; i < _env.size(); ++i)
	{
		if (_env[i] != NULL)
			delete[] _env[i];
	}
	_env.clear();

	// Clean up argv
	_argv.clear();

	// Reset state variables
	_scriptPath.clear();
	_interpreterPath.clear();
	_pid = -1;
	status = 0;
	responseStarted = false;
	_isRunning = false;
	_needBody = false;

	// Reset the CGI parser for the next request
	_cgiParser.reset();

	// Note: _Reqparser and _response are references and should be
	// updated externally before calling start() again
}

int CGIHandler::getStatus()
{
	return (status);
}

#endif // CGI_HANDLER_HPP
