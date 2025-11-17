#include "RingBuffer.hpp"
#include <iostream>
#include <algorithm>

/*
    _buff [....................]
              |              |
            start           end
    capacity = 20
    size     = end - start;
*/

Buffer::Buffer(size_t size):
    _buff(size, 0),
    _start(0),
    _end(0),
    _capacity(size)
{
}

size_t  Buffer::getCapacity(void) const { return _capacity; }
size_t  Buffer::getSize(void) const { return _end - _start; }

void    Buffer::clear()
{
    _start = 0;
    _end = 0;
}

size_t  Buffer::write(const char *buff, size_t size)
{
    if (!buff || !size || !_capacity)
        return 0;
    if (isFull()) clear();
    size_t toWrite = (_end + size <= _capacity ? size : _capacity - _end);
    std::memcpy((char*)_buff.data() + _end, buff, toWrite);
    _end += toWrite;
    return toWrite;
}
size_t  Buffer::read(char *buff, size_t size)
{
    size_t read = peek(buff, size);
    _start += read;
    return read;
}
size_t  Buffer::peek(char *buff, size_t size)
{
    if (!buff || !size || !_capacity)
        return 0;
    size_t toRead = (_end - _start <= size ? _end - _start : size);
    std::memcpy(buff, _buff.data() + _start, toRead);
    return toRead;
}

bool    Buffer::isFull() const { return _end - _start == _capacity; }
bool    Buffer::isEmpty() const { return !(_end - _start); }

void    Buffer::advanceRead(size_t size)
{
    _start += std::min(size, getSize());
}

// int main()
// {
//     Buffer buff(5);
//     std::string test = "123456789";
//     char tmp[10];

//     size_t written = buff.write(test.c_str(), test.size());
//     size_t s = buff.peek(tmp, sizeof(tmp));
//     buff.advanceRead(s);

//     std::cout << "bytes written: " << written << std::endl;
//     std::cout << "bytes read   : " << s << std::endl;
//     std::cout.write(tmp, s);
//     std::cout << std::endl;
//     std::cout << "bytes left: " << buff.getSize() << std::endl;

//     return 0;
// }
