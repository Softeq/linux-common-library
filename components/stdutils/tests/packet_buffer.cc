#include <gtest/gtest.h>

#include <common/stdutils/packet_buffer.hh>

#include <cstdint>
#include <chrono>
#include <vector>
#include <string>
#include <thread>

using namespace softeq::common::stdutils;

TEST(PacketBuffer, DefaultConstructor)
{
    PacketBuffer<> buffer;

    EXPECT_GT(buffer.getLimit(), 0) << "Expect default limit is greater than zero after buffer creation";
    ASSERT_EQ(0, buffer.size()) << "Buffer must be empty after construction";
}

TEST(PacketBuffer, SetLimit)
{
    PacketBuffer<> buffer;

    buffer.setLimit(4096);
    EXPECT_EQ(buffer.getLimit(), 4096);

    auto const min = std::numeric_limits<std::size_t>::min();
    buffer.setLimit(min);
    EXPECT_LE(buffer.getLimit(), 0) << "Set minimum value:" << min;

    auto const max = std::numeric_limits<std::size_t>::max();
    buffer.setLimit(max);
    EXPECT_GE(buffer.getLimit(), 0) << "Set maximum value:" << max;
}

TEST(PacketBuffer, PushData)
{
    PacketBuffer<std::vector<uint8_t>> buffer;

    constexpr int const packetsToWrite = 5;
    constexpr int const packetSize = 1000;
    constexpr std::size_t const limit = packetsToWrite * packetSize;
    buffer.setLimit(limit);

    // Try to push data to the buffer up to the limit
    for (int n = 1; n <= packetsToWrite; ++n)
    {
        std::vector<uint8_t> packet(packetSize, n);

        ASSERT_TRUE(buffer.push(packet));
        // Check the size of stored data
        EXPECT_EQ(buffer.size(), n * packetSize);
    }

    // Push data above the limit
    std::vector<uint8_t> extraPacket(packetSize, 0);
    EXPECT_FALSE(buffer.push(extraPacket)) << "Packet should be discarded";

    // Check the size of the entire buffer (buffer is full at the moment)
    EXPECT_EQ(buffer.size(), limit);
}

TEST(PacketBuffer, PopData)
{
    PacketBuffer<std::string> buffer;
    std::string data;

    EXPECT_FALSE(buffer.pop(data)) << "Try to pop data just after buffer creation";

    constexpr char const *message[] = {"first message", "second message", "third message", nullptr};

    // Load buffer with the test data
    for (int i = 0; message[i]; ++i)
    {
        std::string s(message[i]);
        // Abort test if unable to put data
        ASSERT_TRUE(buffer.push(s));
    }

    // Read buffer (FIFO) and compare to test data
    std::string text;

    for (int i = 0; message[i]; ++i)
    {
        ASSERT_TRUE(buffer.pop(text));
        EXPECT_TRUE(text == message[i]);
    }

    // Buffer must be empty at this point
    EXPECT_EQ(buffer.size(), 0);
}

TEST(PacketBuffer, RollBack)
{
    PacketBuffer<std::string> buffer;

    constexpr int const packetSize = 6; // number of chars per packet
    constexpr int const packetsCount = ('Z' - 'A') + 1;
    constexpr int const dataSize = packetSize * packetsCount;

    buffer.setLimit(dataSize + 1);

    for (char ch = 'A'; ch <= 'Z'; ++ch)
    {
        std::string s(packetSize, ch);
        ASSERT_TRUE(buffer.rollBack(s));
    }
    EXPECT_EQ(buffer.size(), dataSize);

    // Try to everrun buffer
    EXPECT_FALSE(buffer.rollBack("OVERRUN DATA"));
    // Check the payload don't change
    EXPECT_EQ(buffer.size(), dataSize);
}

TEST(PacketBuffer, Concurrency)
{
    PacketBuffer<std::string> buffer;

    constexpr long const packetsCount = 1000;
    constexpr long const bufferCapacity = 1024;

    buffer.setLimit(bufferCapacity);
    std::srand(std::time(0));

    std::thread sender([&buffer]() {
        constexpr int const sleepTimeMaxMs = 100;

        for (long n = 1; n <= packetsCount;)
        {
            std::string packet{std::to_string(n)};

            if (!buffer.push(packet))
            {
                auto const delay = std::chrono::milliseconds(1 + (std::rand() % sleepTimeMaxMs));

                std::this_thread::sleep_for(delay);
            }
            else
                ++n;
        }
    });

    std::string text;
    long n{0}; // Received packets counter

    while (n < packetsCount)
    {
        if (buffer.pop(text))
        {
            auto const packetData = std::strtol(text.c_str(), nullptr, 10);

            ASSERT_EQ(packetData, ++n);

            // Simulate of a data processing
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    if (sender.joinable())
    {
        sender.join();
    }
}
