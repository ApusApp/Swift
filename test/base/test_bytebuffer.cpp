#include <gtest/gtest.h>

#include <swift/base/bytebuffer.h>

class test_ByteBuffer : public testing::Test
{
public:
    test_ByteBuffer () {}
    ~test_ByteBuffer () {}

    virtual void SetUp (void)
    {

    }

    virtual void TearDown (void)
    {

    }
};

TEST_F (test_ByteBuffer, BufferLength) 
{
    swift::ByteBuffer buffer;
    size_t size = 0;
    EXPECT_EQ (size, buffer.Length ());

    buffer.WriteUInt8 (1);
    ++size;
    EXPECT_EQ (size, buffer.Length ());

    buffer.WriteUInt16 (1);
    size += 2;
    EXPECT_EQ (size, buffer.Length ());

    buffer.WriteUInt32 (1);
    size += 4;
    EXPECT_EQ (size, buffer.Length ());

    buffer.WriteUInt64 (1);
    size += 8;
    EXPECT_EQ (size, buffer.Length ());

    EXPECT_TRUE (buffer.Consume (0));
    EXPECT_EQ (size, buffer.Length ());

    EXPECT_TRUE (buffer.Consume (4));
    size -= 4;
    EXPECT_EQ (size, buffer.Length ());
}

TEST_F (test_ByteBuffer, GetSetReadPosition) 
{
    swift::ByteBuffer buffer ("ABCDEF", 6);
    EXPECT_EQ (6U, buffer.Length ());
    swift::ByteBuffer::ReadPosition pos (buffer.GetReadPosition ());
    EXPECT_TRUE (buffer.SetReadPosition (pos));
    EXPECT_EQ (6U, buffer.Length ());
    std::string read;
    EXPECT_TRUE (buffer.ReadString (&read, 3));
    EXPECT_EQ ("ABC", read);
    EXPECT_EQ (3U, buffer.Length ());
    EXPECT_TRUE (buffer.SetReadPosition (pos));
    EXPECT_EQ (6U, buffer.Length ());
    read.clear ();
    EXPECT_TRUE (buffer.ReadString (&read, 3));
    EXPECT_EQ ("ABC", read);
    EXPECT_EQ (3U, buffer.Length ());
    // For a resize by writing Capacity() number of bytes.
    size_t capacity = buffer.Capacity ();
    buffer.ReserveWriteBuffer (buffer.Capacity ());
    EXPECT_EQ (capacity + 3U, buffer.Length ());
    EXPECT_FALSE (buffer.SetReadPosition (pos));
    read.clear ();
    EXPECT_TRUE (buffer.ReadString (&read, 3));
    EXPECT_EQ ("DEF", read);
}

TEST_F (test_ByteBuffer, ReadWriteBuffer) 
{
    swift::ByteOrderHelper::ByteOrder orders[2] = { 
        swift::ByteOrderHelper::ByteOrder::BYTE_ORDER_HOST,
        swift::ByteOrderHelper::ByteOrder::BYTE_ORDER_NETWORKER
    };

    for (size_t i = 0; i < sizeof(orders); i++) {
        swift::ByteBuffer buffer (orders[i]);
        EXPECT_EQ (orders[i], buffer.Order ());
        uint8_t ru8;
        EXPECT_FALSE (buffer.ReadUInt8 (&ru8));

        // Write and read uint8.
        uint8_t wu8 = 1;
        buffer.WriteUInt8 (wu8);
        EXPECT_TRUE (buffer.ReadUInt8 (&ru8));
        EXPECT_EQ (wu8, ru8);
        EXPECT_EQ (0U, buffer.Length ());

        // Write and read uint16.
        uint16_t wu16 = (1 << 8) + 1;
        buffer.WriteUInt16 (wu16);
        uint16_t ru16;
        EXPECT_TRUE (buffer.ReadUInt16 (&ru16));
        EXPECT_EQ (wu16, ru16);
        EXPECT_EQ (0U, buffer.Length ());

        // Write and read uint32_t.
        uint32_t wu32 = (4 << 24) + (3 << 16) + (2 << 8) + 1;
        buffer.WriteUInt32 (wu32);
        uint32_t ru32;
        EXPECT_TRUE (buffer.ReadUInt32 (&ru32));
        EXPECT_EQ (wu32, ru32);
        EXPECT_EQ (0U, buffer.Length ());

        // Write and read uint64_t.
        uint32_t another32 = (8 << 24) + (7 << 16) + (6 << 8) + 5;
        uint64_t wu64 = (static_cast<uint64_t>(another32) << 32) + wu32;
        buffer.WriteUInt64 (wu64);
        uint64_t ru64;
        EXPECT_TRUE (buffer.ReadUInt64 (&ru64));
        EXPECT_EQ (wu64, ru64);
        EXPECT_EQ (0U, buffer.Length ());

        // Write and read string.
        std::string write_string ("hello");
        buffer.WriteString (write_string);
        std::string read_string;
        EXPECT_TRUE (buffer.ReadString (&read_string, write_string.size ()));
        EXPECT_EQ (write_string, read_string);
        EXPECT_EQ (0U, buffer.Length ());

        // Write and read bytes
        char write_bytes[] = "foo";
        buffer.WriteBytes (write_bytes, 3);
        char read_bytes[3];
        EXPECT_TRUE (buffer.ReadBytes (read_bytes, 3));
        for (int i = 0; i < 3; ++i) {
            EXPECT_EQ (write_bytes[i], read_bytes[i]);
        }
        EXPECT_EQ (0U, buffer.Length ());

        // Write and read reserved buffer space
        char* write_dst = buffer.ReserveWriteBuffer (3);
        memcpy (write_dst, write_bytes, 3);
        memset (read_bytes, 0, 3);
        EXPECT_TRUE (buffer.ReadBytes (read_bytes, 3));
        for (int i = 0; i < 3; ++i) {
            EXPECT_EQ (write_bytes[i], read_bytes[i]);
        }
        EXPECT_EQ (0U, buffer.Length ());

        // Write and read in order.
        buffer.WriteUInt8 (wu8);
        buffer.WriteUInt16 (wu16);
        buffer.WriteUInt32 (wu32);
        buffer.WriteUInt64 (wu64);
        EXPECT_TRUE (buffer.ReadUInt8 (&ru8));
        EXPECT_EQ (wu8, ru8);
        EXPECT_TRUE (buffer.ReadUInt16 (&ru16));
        EXPECT_EQ (wu16, ru16);
        EXPECT_TRUE (buffer.ReadUInt32 (&ru32));
        EXPECT_EQ (wu32, ru32);
        EXPECT_TRUE (buffer.ReadUInt64 (&ru64));
        EXPECT_EQ (wu64, ru64);
        EXPECT_EQ (0U, buffer.Length ());
    }
}

TEST_F (test_ByteBuffer, Others) 
{
    std::string str ("abcdefgh");
    swift::ByteBuffer buffer (str.c_str (), str.size (), swift::ByteOrderHelper::ByteOrder::BYTE_ORDER_NETWORKER);

    swift::ByteBuffer::ReadPosition pos (buffer.GetReadPosition ());
    char c;
    EXPECT_TRUE (buffer.ReadBytes (&c, 1));
    EXPECT_EQ ('a', c);
    EXPECT_TRUE (buffer.ReadBytes (&c, 1));
    EXPECT_EQ ('b', c);
    EXPECT_TRUE (buffer.Consume (2));
    EXPECT_FALSE (buffer.Consume (5));
    EXPECT_TRUE (buffer.ReadBytes (&c, 1));
    EXPECT_EQ (c, 'e');

    //swift::ByteBuffer buffer1 (swift::ByteOrderHelper::ByteOrder::BYTE_ORDER_NETWORKER);
    //buffer1(std::move (buffer));
    //EXPECT_EQ (0, buffer.Length ());
    //EXPECT_EQ (nullptr, buffer.Data ());

    EXPECT_TRUE (buffer.SetReadPosition (pos));
    std::string s = std::move (buffer.ToString ());
    EXPECT_EQ (str, s);
    EXPECT_EQ (buffer.Order (), swift::ByteOrderHelper::ByteOrder::BYTE_ORDER_NETWORKER);

    swift::ByteBuffer b;
    const char* d = b.Data ();
    buffer.Swap (b);
    EXPECT_EQ (b.Order (), swift::ByteOrderHelper::ByteOrder::BYTE_ORDER_NETWORKER);
    EXPECT_EQ (buffer.Order (), swift::ByteOrderHelper::ByteOrder::BYTE_ORDER_HOST);
    EXPECT_EQ (d, buffer.Data ());
    EXPECT_EQ (0, buffer.Length ());
    EXPECT_EQ (str.size (), b.Length ());
    b.Clear ();
    EXPECT_EQ (0, b.Length ());
}
