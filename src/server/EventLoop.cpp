#include "EventLoop.hpp"

EventLoop::EventLoop() : epoll(), fd_manager(epoll)
{
    
}

void EventLoop::run()
{
    logger.info("Event loop started");
    while(!g_shutdown)
    {
        std::vector<epoll_event> events = epoll.wait();
        for (size_t i = 0; i < events.size(); i++) 
        {
            try 
            {
                EventHandler* handler = fd_manager.getOwner(events[i].data.fd);
                if (handler == NULL) 
                    continue;
                if (READ_EVENT(events[i].events)) handler->onReadable();
                if (WRITE_EVENT(events[i].events)) handler->onWritable();
                if (ERROR_EVENT(events[i].events)) handler->onError();
            }
            catch (const std::exception &e) 
            {
                //TODO : Handle exception, possibly log it
                logger.error("Exception in event loop: " + std::string(e.what()));
                // Note: Deletion of handler should be handled in onError or by the handler itself
            }
        }
    }
}

EventLoop::~EventLoop()
{
    logger.info("Event loop terminated");
}
