#pragma once

#include <cstdint>
#include <vector>

// Packed Bits classes copied from the original Wind Waker Randomizer
class PackedBitsWriter
{
private:
    size_t bits_left_in_byte = 8;
    uint8_t current_byte = 0;
    std::vector<uint8_t> bytes = {};

public:
    PackedBitsWriter() = default;
    ~PackedBitsWriter() = default;

    template <typename T>
    void write(T value, size_t length)
    {
        size_t bits_to_read = 0;
        while (length > 0)
        {
            if (length >= bits_left_in_byte)
            {
                bits_to_read = bits_left_in_byte;
            }
            else
            {
                bits_to_read = length;
            }

            size_t mask = (1 << bits_to_read) - 1;
            current_byte |= (value & mask) << (8 - bits_left_in_byte);

            bits_left_in_byte -= bits_to_read;
            length -= bits_to_read;
            value >>= bits_to_read;

            if (bits_left_in_byte > 0)
            {
                continue;
            }

            flush();
        }
    }

    void flush()
    {
        bytes.push_back(current_byte);
        current_byte = 0;
        bits_left_in_byte = 8;
    }

    const std::vector<uint8_t>& getBytes() const {
        return bytes;
    }
};

class PackedBitsReader
{
private:
    size_t current_bit_index = 0;
    size_t current_byte_index = 0;
    std::vector<uint8_t> bytes = {};

public:
    PackedBitsReader(const std::vector<uint8_t>& bytes_) : bytes(bytes_) {}
    ~PackedBitsReader() = default;

    size_t read(size_t length)
    {
        size_t bits_read = 0;
        size_t value = 0;
        size_t bits_left_to_read = length;

        while (bits_read != length)
        {
            size_t bits_to_read = 0;
            if (bits_left_to_read > 8)
            {
                bits_to_read = 8;
            }
            else
            {
                bits_to_read = bits_left_to_read;
            }

            if (bits_to_read + current_bit_index > 8)
            {
                bits_to_read = 8 - current_bit_index;
            }

            size_t mask = ((1 << bits_to_read) - 1) << current_bit_index;

            if (current_byte_index >= bytes.size())
            {
                return static_cast<size_t>(-1);
            }

            size_t current_byte = bytes[current_byte_index];
            value = ((current_byte & mask) >> current_bit_index) << bits_read | value;

            current_bit_index += bits_to_read;
            current_byte_index += current_bit_index >> 3;
            current_bit_index %= 8;
            bits_left_to_read -= bits_to_read;
            bits_read += bits_to_read;
        }

        return value;
    }

    bool reachedLastByte() const {
        return current_byte_index == bytes.size() - 1;
    }
};
