#ifndef SOFTEQ_COMMON_PACKET_BUFFER_HH_
#define SOFTEQ_COMMON_PACKET_BUFFER_HH_

#include <deque>
#include <vector>
#include <stdint.h>
#include <mutex>

namespace softeq
{
namespace common
{
/*!
  \brief The class is implementation of packet buffer.
*/
template <class T = std::vector<uint8_t>>
class PacketBuffer final
{
public:
    PacketBuffer();
    PacketBuffer(const PacketBuffer &) = delete;
    PacketBuffer(PacketBuffer &&) = delete;

    /*!
        Insert the packet front in the deque
        \param[in] val Packet
    */
    bool push(const T &val);
    /*!
       Delete the packet from front in the deque
        \param[in] v Packet
    */
    bool pop(T &v);
    /*!
        Insert the packet back in the queue
        \param[in] v Packet
    */
    bool rollBack(const T &v);
    /*!
        Get buffer limit
        \return Buffer limit
    */
    size_t getLimit() const
    {
        return _buf_limit_size;
    }
    /*!
        Set buffer limit
        \param[in] limit Buffer limit
   */
    void setLimit(size_t limit)
    {
        _buf_limit_size = limit;
    }
    /*!
       Get size of buffer
        \return Size of buffer
      */
    size_t size() const
    {
        return _buf_size;
    }

private:
    std::deque<T> _buffer;
    std::mutex _mtx;
    size_t _buf_limit_size; // maximum size of data allowed to be saved in the buffer
    size_t _buf_size;       // size in bytes of the data saved in the buffer
};

} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_PACKET_BUFFER_HH_
