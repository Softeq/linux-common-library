#include "packet_buffer.hh"

#include <cstring>

using namespace softeq::common::stdutils;

namespace
{
constexpr size_t DEFAULT_BUFFER_LIMIT = (1024 * 1024);
}

template <class T>
PacketBuffer<T>::PacketBuffer(void)
{
    _buf_limit_size = DEFAULT_BUFFER_LIMIT;
    _buf_size = 0;
}

template <class T>
bool PacketBuffer<T>::push(const T &data)
{
    std::lock_guard<std::mutex> guard(_mtx);

    if ((_buf_size + data.size()) > _buf_limit_size)
        return false;

    _buffer.push_back(data);

    _buf_size += data.size();
    return true;
}

template <class T>
bool PacketBuffer<T>::pop(T &v)
{
    std::lock_guard<std::mutex> guard(_mtx);

    if (_buffer.size())
    {
        v = _buffer.front();

        _buf_size -= v.size();

        _buffer.pop_front();

        return true;
    }

    // queue is empty
    return false;
}

// insert the packet back in the queue
template <class T>
bool PacketBuffer<T>::rollBack(const T &v)
{
    std::lock_guard<std::mutex> guard(_mtx);

    if ((_buf_size + v.size()) > _buf_limit_size)
        return false;

    _buffer.push_front(v);

    _buf_size += v.size();

    return true;
}

template class softeq::common::stdutils::PacketBuffer<std::vector<uint8_t>>;
template class softeq::common::stdutils::PacketBuffer<std::string>;
