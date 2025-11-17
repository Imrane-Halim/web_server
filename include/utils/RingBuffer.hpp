#ifndef WEBSERV_RINGBUFF_HPP
#define WEBSERV_RINGBUFF_HPP

#include <vector>
#include <cstring>
#include <string>

class Buffer
{
    std::string _buff;

    // indexs
    size_t  _start;
    size_t  _end;

    size_t  _capacity;

public:
    explicit Buffer(size_t size);

    size_t  write(const char* buff, size_t size); // write to buffer, returns bytes written
    size_t  read(char* buff, size_t size);  // read to buff, returns bytes read
    size_t  peek(char* buff, size_t size);  // read without consumming

    size_t  getCapacity(void) const;
    size_t  getSize(void) const;            // Get current data size

    bool    isFull(void) const;
    bool    isEmpty(void) const;

    void    advanceRead(size_t size);

    const std::string& getRAW(void) const { return _buff; }

    void    clear(void);
};

#endif