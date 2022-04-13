#include "yaz0.hpp"

#include <cstring>
#include <string>

#include "../utility/endian.hpp"

constexpr uint32_t READ_CHUNK_SIZE = 4096;
constexpr uint32_t MAX_SEARCH_RANGE = 0x1000;
constexpr uint32_t MAX_ENCODED_SIZE = 0x111;

struct Yaz0Header 
{
    char magic[4];
    uint32_t uncompressedSize;
    char _unused0[8];
};

namespace {
    YAZ0Error readYaz0Header(std::istream& in, Yaz0Header& header)
    {
        if(!in.read(header.magic, sizeof(header.magic))) return YAZ0Error::REACHED_EOF;
        // check magic string in header
        if(std::strncmp(header.magic, "Yaz0", 4) != 0) return YAZ0Error::NOT_YAZ0;
        if(!in.read(reinterpret_cast<char*>(&header.uncompressedSize), sizeof(header.uncompressedSize))) return YAZ0Error::REACHED_EOF;
        Utility::Endian::toPlatform_inplace(Utility::Endian::Type::Big, header.uncompressedSize);
        if(!in.read(header._unused0, sizeof(header._unused0))) return YAZ0Error::REACHED_EOF;
        return YAZ0Error::NONE;
    }
    
    // simple and straight encoding scheme for Yaz0
    inline uint32_t simpleEnc(const uint8_t* src, uint32_t size, uint32_t pos, uint32_t& matchPosOut, uint32_t searchRange)
    {
        uint32_t startPos = pos - searchRange;
        uint32_t i = 0;
        uint32_t runLen = 0, maxRunLen;
        // numBytes is length of best match found
        uint32_t numBytes = 1;
        // matchPos is position of start of best match found
        uint32_t matchPos = 0;
    
        maxRunLen = MAX_ENCODED_SIZE;
        // if we could possibly fall off the end of the file, 
        // max run len is the remaining bytes
        if (maxRunLen + pos > size)
        {
            maxRunLen = size - pos;
        }
        // don't bother if we're right at the end of the file
        if (maxRunLen < 3)
        {
            return 1;
        }
    
        for (i = startPos; i < pos; i++)
        {
            // loop unwind pre-check
            if (src[i] == src[pos] && src[i + 1] == src[pos + 1] && src[i + 2] == src[pos + 2])
            {
                for (runLen = 3; runLen < maxRunLen; runLen++)
                {
                    if (src[i + runLen] != src[pos + runLen])
                    {
                        break;
                    }
                }
    
                // if we have found a longer copy match
                if (runLen > numBytes)
                {
                    numBytes = runLen;
                    matchPos = i;
                }
            }
        }
        matchPosOut = matchPos;
        if (numBytes == 2)
        {
            numBytes = 1;
        }
        return numBytes;
    }
    
    // a lookahead encoding scheme for ngc Yaz0
    uint32_t nintendoEnc(const uint8_t* src, uint32_t size, uint32_t pos, uint32_t& matchPosOut, uint32_t searchRange)
    {
        uint32_t numBytes = 1;
        static thread_local uint32_t numBytes1;
        static thread_local uint32_t matchPos;
        static thread_local int prevFlag = 0;
    
        // if prevFlag is set, it means that the previous position was determined by look-ahead try.
        // so just use it. this is not the best optimization, but nintendo's choice for speed.
        if (prevFlag == 1) {
            matchPosOut = matchPos;
            prevFlag = 0;
            return numBytes1;
        }
        prevFlag = 0;
        numBytes = simpleEnc(src, size, pos, matchPos, searchRange);
        matchPosOut = matchPos;
    
        // if this position is RLE encoded, then compare to copying 1 byte and next position(pos+1) encoding
        if (numBytes >= 3) {
            numBytes1 = simpleEnc(src, size, pos+1, matchPos, searchRange);
            // if the next position encoding is +2 longer than current position, choose it.
            // this does not guarantee the best optimization, but fairly good optimization with speed.
            if (numBytes1 >= numBytes + 2) {
                numBytes = 1;
                prevFlag = 1;
            }
        }
        return numBytes;
    }
    
    uint32_t yaz0DataEncode(const uint8_t* src, uint32_t srcSize, std::ostream& out, uint32_t level = 9)
    {
        uint32_t srcPos = 0;
        uint32_t dstPos = 0;
    
        uint8_t dst[24];    // 8 codes * 3 bytes maximum
        uint32_t dstSize = 0;
        
        uint32_t validBitCount = 0; //number of valid bits left in "code" byte
        uint8_t currCodeByte = 0;
        uint32_t searchRange = MAX_SEARCH_RANGE;
        if (level > 0 && level < 9)
        {
            searchRange >>= (9 - level);
        }
    
        while(srcPos < srcSize)
        {
            uint32_t numBytes;
            uint32_t matchPos;
    
            numBytes = nintendoEnc(src, srcSize, srcPos, matchPos, searchRange);
            if (numBytes < 3)
            {
                //straight copy
                dst[dstPos] = src[srcPos];
                dstPos++;
                srcPos++;
                //set flag for straight copy
                currCodeByte |= (0x80 >> validBitCount);
            }
            else 
            {
                //RLE part
                uint32_t dist = srcPos - matchPos - 1; 
                uint8_t byte1, byte2, byte3;
    
                if (numBytes >= 0x12)  // 3 byte encoding
                {
                    byte1 = 0 | (dist >> 8);
                    byte2 = dist & 0xff;
                    dst[dstPos++] = byte1;
                    dst[dstPos++] = byte2;
                    // maximum runlength for 3 byte encoding
                    if (numBytes > 0xff + 0x12)
                    {
                        numBytes = 0xff + 0x12;
                    }
                    byte3 = numBytes - 0x12;
                    dst[dstPos++] = byte3;
                } 
                else  // 2 byte encoding
                {
                    byte1 = ((numBytes - 2) << 4) | (dist >> 8);
                    byte2 = dist & 0xff;
                    dst[dstPos++] = byte1;
                    dst[dstPos++] = byte2;
                }
                srcPos += numBytes;
            }
            validBitCount++;
            //write eight codes
            if(validBitCount == 8)
            {
                out.put(currCodeByte);
                out.write(reinterpret_cast<char*>(dst), dstPos);
                dstSize += dstPos+1;
    
                currCodeByte = 0;
                validBitCount = 0;
                dstPos = 0;
            }
        }
        if(validBitCount > 0)
        {
            out.put(currCodeByte);
            out.write(reinterpret_cast<char*>(dst), dstPos);
            dstSize += dstPos + 1;
    
            currCodeByte = 0;
            validBitCount = 0;
            dstPos = 0;
        }
    
        return dstSize;
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

namespace FileTypes {
    const char* YAZ0ErrorGetName(YAZ0Error err) {
        switch (err) {
            case YAZ0Error::NONE:
                return "NONE";
            case YAZ0Error::COULD_NOT_OPEN:
                return "COULD_NOT_OPEN";
            case YAZ0Error::NOT_YAZ0:
                return "NOT_YAZ0";
            case YAZ0Error::DATA_SIZE_0:
                return "DATA_SIZE_0";
            case YAZ0Error::REACHED_EOF:
                return "REACHED_EOF";
            default:
                return "UNKNOWN";
        }
    }

    YAZ0Error yaz0Encode(std::istream& in, std::ostream& out, uint32_t compressionLevel)
    {
        char readChunkBuf[READ_CHUNK_SIZE];
        std::string inData{};
        char dummyData[8] = {0};

        // read entire file
        while(in.read(readChunkBuf, READ_CHUNK_SIZE))
        {
            inData.append(readChunkBuf, in.gcount());
        }
        inData.append(readChunkBuf, in.gcount());

        // write magic
        out.write("Yaz0", 4);
        uint32_t dataSize = inData.size();
        uint32_t outDataSize = Utility::Endian::toPlatform(Utility::Endian::Type::Big, dataSize);
        out.write(reinterpret_cast<char*>(&outDataSize), 4);
        out.write(dummyData, 8);

        if (yaz0DataEncode(reinterpret_cast<const uint8_t*>(inData.data()), dataSize, out, compressionLevel) == 0) return YAZ0Error::DATA_SIZE_0;
        return YAZ0Error::NONE;
    }

    YAZ0Error yaz0Decode(std::istream& in, std::ostream& out)
    {
        char readChunkBuf[READ_CHUNK_SIZE];
        std::string inData{};

        Yaz0Header header;

        YAZ0Error err = YAZ0Error::NONE;
        err = readYaz0Header(in, header);
        if(err != YAZ0Error::NONE)
        {
            return err;
        }

        // TODO: for now we are reading entire file into memory
        // and allocating a full size output buffer. This can
        // take up quite a lot of memory, so if it becomes a
        // problem, we can switch to decoding as chunks. This
        // will make decoding _slightly_ slower, but much more
        // memory efficient

        // read rest of file into memory
        while(in.read(readChunkBuf, READ_CHUNK_SIZE))
        {
            inData.append(readChunkBuf, in.gcount());
        }
        inData.append(readChunkBuf, in.gcount());

        char* outData = new char[header.uncompressedSize];
        err = yaz0DataDecode(inData.data(), outData, header.uncompressedSize);
        if(err != YAZ0Error::NONE)
        {
            delete[] outData;
            return err;
        }
        out.write(outData, header.uncompressedSize);

        delete[] outData;
        return YAZ0Error::NONE;
    }
}
