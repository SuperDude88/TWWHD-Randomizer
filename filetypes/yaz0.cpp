#include "yaz0.hpp"

#include <cstring>
#include <string>
#include <bitset>

#include <utility/endian.hpp>
#include <command/Log.hpp>
#include <zlib-ng.h>

constexpr uint32_t READ_CHUNK_SIZE = 4096;

struct Yaz0Header 
{
    char magic[4];
    uint32_t uncompressedSize;
    char _unused0[8];
};

namespace {
    YAZ0Error readYaz0Header(std::istream& in, Yaz0Header& header)
    {
        if(!in.read(header.magic, sizeof(header.magic))) LOG_ERR_AND_RETURN(YAZ0Error::REACHED_EOF);
        // check magic string in header
        if(std::strncmp(header.magic, "Yaz0", 4) != 0) LOG_ERR_AND_RETURN(YAZ0Error::NOT_YAZ0);
        if(!in.read(reinterpret_cast<char*>(&header.uncompressedSize), sizeof(header.uncompressedSize))) LOG_ERR_AND_RETURN(YAZ0Error::REACHED_EOF);
        Utility::Endian::toPlatform_inplace(Utility::Endian::Type::Big, header.uncompressedSize);
        if(!in.read(header._unused0, sizeof(header._unused0))) LOG_ERR_AND_RETURN(YAZ0Error::REACHED_EOF);
        return YAZ0Error::NONE;
    }
    
    YAZ0Error yaz0DataDecode(const char* in, char* out, uint32_t outTotalSize)
    {
        uint32_t runLength = 0, runOffset = 0;
        uint8_t codeInfoBlockMSB = 0, codeInfoBlockLSB = 0;
        uint8_t codingModeByte = 0;
        uint8_t validCodingBitCount = 0;
        char* lastByte = out + outTotalSize;
        while(out < lastByte)
        {
            if(validCodingBitCount == 0)
            {
                codingModeByte = *(in++);
                validCodingBitCount = 8;
            }
    
            if((codingModeByte & 0x80) == 0)
            {
                // read two bytes for coding information
                codeInfoBlockMSB = in[0];
                codeInfoBlockLSB = in[1];
                in += 2;
                runOffset = ((codeInfoBlockMSB & 0x0F) << 8) | codeInfoBlockLSB;
                runLength = codeInfoBlockMSB >> 4;
                // if upper nibble is zero, run length is in (optional) third code info byte
                if(runLength == 0)
                {
                    runLength = *(reinterpret_cast<const uint8_t*>(in++));
                    runLength += 0x12;
                }
                else
                {
                    runLength += 2;
                }
                char* pRun = out - runOffset - 1;
                char* pEndRun = pRun + runLength;
                while(pRun < pEndRun)
                {
                    // copy from run location and advance
                    *(out++) = *(pRun++);
                }
            }
            else 
            {
                // copy single byte and move both pointers forward one
                *(out++) = *(in++);
            }
    
            codingModeByte <<= 1;
            validCodingBitCount -= 1;
        }
    
        return YAZ0Error::NONE;
    }
}

//from https://github.com/zeldamods/oead/blob/master/src/yaz0.cpp#L50
constexpr size_t ChunksPerGroup = 8;
constexpr size_t MaximumMatchLength = 0xFF + 0x12;

class GroupWriter {
public:
  GroupWriter(std::ostream& result) : m_result{result} { Reset(); }

  void HandleZlibMatch(uint32_t dist, uint32_t lc) {
    if (dist == 0) {
      // Literal.
      m_group_header.set(7 - m_pending_chunks);
      m_result.put(uint8_t(lc));
    } else {
      // Back reference.
      constexpr uint32_t ZlibMinMatch = 3;
      WriteMatch(dist - 1, lc + ZlibMinMatch);
    }

    ++m_pending_chunks;
    if (m_pending_chunks == ChunksPerGroup) {
      const std::streamoff old = m_result.tellp();
      m_result.seekp(m_group_header_offset, std::ios::beg).put(uint8_t(m_group_header.to_ulong()));
      m_result.seekp(old, std::ios::beg);
      Reset();
    }
  }

  // Must be called after zlib has completed to ensure the last group is written.
  void Finalise() {
    if (m_pending_chunks != 0) {
      const std::streamoff old = m_result.tellp();
      m_result.seekp(m_group_header_offset, std::ios::beg).put(uint8_t(m_group_header.to_ulong()));
      m_result.seekp(old, std::ios::beg);
    }
  }

private:
  void Reset() {
    m_pending_chunks = 0;
    m_group_header.reset();
    m_group_header_offset = m_result.tellp();
    m_result.put(0xFF);
  }

  void WriteMatch(uint32_t distance, uint32_t length) {
    if (length < 18) {
      m_result.put(((length - 2) << 4) | uint8_t(distance >> 8));
      m_result.put(distance);
    } else {
      // If the match is longer than 18 bytes, 3 bytes are needed to write the match.
      const size_t actual_length = std::min<size_t>(MaximumMatchLength, length);
      m_result.put(uint8_t(distance >> 8));
      m_result.put(uint8_t(distance));
      m_result.put(uint8_t(actual_length - 0x12));
    }
  }

  std::ostream& m_result;
  size_t m_pending_chunks;
  std::bitset<8> m_group_header;
  std::size_t m_group_header_offset;
};

namespace FileTypes {
    const char* YAZ0ErrorGetName(YAZ0Error err) {
        switch (err) {
            case YAZ0Error::NONE:
                return "NONE";
            case YAZ0Error::COULD_NOT_OPEN:
                return "COULD_NOT_OPEN";
            case YAZ0Error::NOT_YAZ0:
                return "NOT_YAZ0";
            case YAZ0Error::ZNG_ERROR:
                return "ZNG_ERROR";
            case YAZ0Error::REACHED_EOF:
                return "REACHED_EOF";
            default:
                return "UNKNOWN";
        }
    }

    //TODO: not thread-safe, unpacking is currently single threaded (can't use thread_local on Wii U)
    //larger buffer is faster, but too large for stack
    constexpr uint32_t STATIC_READ_CHUNK_SIZE = 1024 * 1024 * 2;
    static char readChunkBuf[STATIC_READ_CHUNK_SIZE];

    YAZ0Error yaz0Decode(std::istream& in, std::ostream& out)
    {
        std::string inData{};

        Yaz0Header header;

        LOG_AND_RETURN_IF_ERR(readYaz0Header(in, header));

        // IMPROVEMENT: for now we are reading entire file into memory
        // and allocating a full size output buffer. This can
        // take up quite a lot of memory, so if it becomes a
        // problem, we can switch to decoding as chunks. This
        // will make decoding _slightly_ slower, but much more
        // memory efficient

        // read rest of file into memory
        while(in.read(readChunkBuf, STATIC_READ_CHUNK_SIZE))
        {
            inData.append(readChunkBuf, in.gcount());
        }
        inData.append(readChunkBuf, in.gcount());

        std::string outData(header.uncompressedSize, '\0'); //string instead of char array to avoid manual deletion
        if (const YAZ0Error error = yaz0DataDecode(inData.data(), &outData[0], header.uncompressedSize); error != YAZ0Error::NONE) {
            ErrorLog ::getInstance().log(std ::string("Encountered error on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
            return error;
        }
        out.write(&outData[0], header.uncompressedSize);

        return YAZ0Error::NONE;
    }

    YAZ0Error yaz0Decode(std::stringstream& in, std::ostream& out)
    {
        Yaz0Header header;

        LOG_AND_RETURN_IF_ERR(readYaz0Header(in, header));

        std::string outData(header.uncompressedSize, '\0'); //string instead of char array to avoid manual deletion
        if (const YAZ0Error error = yaz0DataDecode(&in.str()[0x10], &outData[0], header.uncompressedSize); error != YAZ0Error::NONE) {
            ErrorLog ::getInstance().log(std ::string("Encountered error on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
            return error;
        }
        out.write(&outData[0], header.uncompressedSize);

        return YAZ0Error::NONE;
    }
    
    static std::array<uint8_t, 8> dummy{};
    YAZ0Error yaz0Encode(std::stringstream& in, std::ostream& out, uint32_t compressionLevel)
    {
        const std::string& inData = in.str();
        char dummyData[8] = {0};

        // write header
        out.write("Yaz0", 4);
        const uint32_t dataSize = inData.size();
        const uint32_t outDataSize = Utility::Endian::toPlatform(Utility::Endian::Type::Big, dataSize);
        out.write(reinterpret_cast<const char*>(&outDataSize), 4);
        out.write(dummyData, 8);

        GroupWriter group_writer{out};

        // Let zlib-ng do the heavy lifting.
        size_t dummy_size = dummy.size();
        const int ret = zng_compress2(
            dummy.data(), &dummy_size, reinterpret_cast<const uint8_t*>(inData.data()), inData.size(), std::clamp<int>(compressionLevel, 6, 9),
            [](void* w, uint32_t dist, uint32_t lc) { static_cast<GroupWriter*>(w)->HandleZlibMatch(dist, lc); },
            &group_writer);
        if (ret != Z_OK) return YAZ0Error::ZNG_ERROR;

        group_writer.Finalise();
        return YAZ0Error::NONE;
    }
}
