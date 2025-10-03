#ifndef EVENT_HANDLER_HPP
#define EVENT_HANDLER_HPP

#include "FdManager.hpp"

class EventHandler
{
    protected:
        FdManager &_fd_manager;
    public:
        EventHandler(FdManager &fdm);
        virtual ~EventHandler() {}
        virtual int get_fd() const = 0;
        virtual void onReadable() {};
        virtual void onWritable() {};
        virtual void onError() {};
};

EventHandler::EventHandler(FdManager &fdm) : _fd_manager(fdm) {}

#endif //EVENT_HANDLER_HPP