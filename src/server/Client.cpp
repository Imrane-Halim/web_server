#include "Client.hpp"

std::string intToString(int value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

Client::Client(Socket &socket, ServerConfig &config, FdManager &fdm):
    EventHandler(config, fdm, socket),
    _strFD(intToString(_socket.get_fd())),
    _state(ST_READING)
{}
Client::~Client() { _socket.close(); }

int Client::get_fd() const { return _socket.get_fd(); }

void    Client::onError()
{
    logger.error("Error event on client fd: " + _strFD);
    _socket.close();
    _fd_manager.remove(get_fd());
}
void    Client::onReadable()
{
    _readData();
    switch (_state)
    {
    case ST_READING     : break;
    case ST_PROCESSING  : _processRequest(); break;
    case ST_PARSEERROR  : _processError(); break;
    case ST_ERROR       : _processError(); break;
    case ST_CLOSED      : _closeConnection(); break;
    default: break;
    }
}
void    Client::onWritable()
{
    _sendData();
    switch (_state)
    {
    case ST_SENDING     : break;
    case ST_ERROR       : _processError(); break;
    case ST_SENDCOMPLETE:
        if (_keepAlive) reset();
        else _state = ST_CLOSED;
        break;
    case ST_CLOSED: _closeConnection(); break;
    default: break;
    }
}

bool    Client::_readData()
{
    if (_state != ST_READING)
        return false;

    ssize_t size = _socket.recv(_readBuff, BUFF_SIZE - 1, 0);
    if (size < 0)
    {
        logger.error("Error on client fd: " + _strFD);
        _state = ST_ERROR;
        return false;
    }
    if (size == 0)
    {
        logger.warning("Connetion closed on fd: " + _strFD);
        _state = ST_CLOSED;
        return false;
    }
    _readBuff[size] = '\0';
    _request.addChunk(_readBuff, size);
    if (_request.isError())
    {
        logger.debug("Parsing error on client fd: " + _strFD);
        _state = ST_PARSEERROR;
        return false;
    }
    if (_request.isComplete())
    {
        _state = ST_PROCESSING;
        return false;
    }

    return true;
}
bool    Client::_sendData()
{
    char    buff[BUFF_SIZE];
    ssize_t toSend = _response.readNextChunk(buff, BUFF_SIZE);
    if (toSend < 0)
    {
        logger.error("Error on Client fd: " + _strFD);
        _state = ST_ERROR;
        return false;
    }
    if (toSend == 0)
    {
        logger.debug("Client send response complete fd: " + _strFD);
        _state = ST_SENDCOMPLETE;
        return false;
    }
    ssize_t sent = _socket.send(buff, toSend, 0);
    if (sent < 0)
    {
        logger.error("Can't send data on client fd: " + _strFD);
        _state = ST_ERROR;
        return false;
    }
    if (sent < toSend)
    {
        logger.warning("Partial send on client fd: " + _strFD);
        return true;
    }
    if (_response.isComplete())
    {
        logger.debug("Sending response complete on client fd: " + _strFD);
        _state = ST_SENDCOMPLETE;
        return false;
    }
    return true;
}

void    Client::_closeConnection()
{
    _response.closeFile();
    _socket.close();
    _fd_manager.remove(get_fd());
}

void    Client::reset()
{
    _request.reset();
    _response.reset();
    _state = ST_READING;
    _fd_manager.modify(this, EPOLLIN);
}

void    Client::_processError()
{
    if (_state == ST_ERROR)
    {
        logger.error("Error on client fd: " + _strFD);
        _closeConnection();
        return;
    }
    logger.debug("Parsing error on client fd: " + _strFD);
    // todo: send status code '400 bad request'
    // for now I am just going to ignore it
}
void Client::_processRequest()
{
    _keepAlive = _shouldKeepAlive();
    // Add basic headers
    _response.startLine();
    _response.addHeader("Server", "WebServ/1.0");
    _response.addHeader("Connection", _keepAlive ? "keep-alive" : "close");
    _response.addHeader("content-type", "text/html");
    
    // Add logging to verify file attachment
    if (!_response.attachFile("./www/index.html"))
        logger.error("Failed to attach file ./www/index.html on fd: " + _strFD);
        // Fallback to error response
        // _buildErrorResponse(404, "File not found");
    else
        logger.debug("Successfully attached file on fd: " + _strFD);
    
    _state = ST_SENDING;
    _fd_manager.modify(this, EPOLLOUT);
}

bool    Client::_shouldKeepAlive()
{
    std::string conn = _request.getHeader("connection");
    std::transform(conn.begin(), conn.end(), conn.begin(), ::tolower);

    if (_request.getVers() == "HTTP/1.1")
        return conn != "close";
    return conn == "keep-alive";
}
