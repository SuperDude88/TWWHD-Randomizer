#include "yaz0.hpp"

#include <string>
#include <bitset>

#include <utility/endian.hpp>
#include <command/Log.hpp>

#if (defined(__GNUC__) && (__GNUC__ >= 3)) || defined(__INTEL_COMPILER) || defined(__clang__)
#  define LIKELY_NULL(x)        __builtin_expect((x) != 0, 0)
#  define LIKELY(x)             __builtin_expect(!!(x), 1)
#  define UNLIKELY(x)           __builtin_expect(!!(x), 0)
#else
#  define LIKELY_NULL(x)        x
#  define LIKELY(x)             x
#  define UNLIKELY(x)           x
#endif /* (un)likely */

using eType = Utility::Endian::Type;

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

//idea from https://github.com/zeldamods/oead/blob/master/src/yaz0.cpp#L50
//matching stuff from https://github.com/zlib-ng/zlib-ng
constexpr size_t ChunksPerGroup = 8;
constexpr size_t MaximumMatchLength = 0xFF + 0x12;

constexpr size_t STD_MIN_MATCH = 3;
constexpr size_t WANT_MIN_MATCH = 4;
constexpr size_t STD_MAX_MATCH = 258;
constexpr size_t MIN_LOOKAHEAD = STD_MAX_MATCH + STD_MIN_MATCH + 1;

static inline uint32_t compare256(const uint8_t *src0, const uint8_t *src1) {
    uint32_t len = 0;

    do {
        if (*src0 != *src1)
            return len;
        src0 += 1, src1 += 1, len += 1;
        if (*src0 != *src1)
            return len;
        src0 += 1, src1 += 1, len += 1;
        if (*src0 != *src1)
            return len;
        src0 += 1, src1 += 1, len += 1;
        if (*src0 != *src1)
            return len;
        src0 += 1, src1 += 1, len += 1;
        if (*src0 != *src1)
            return len;
        src0 += 1, src1 += 1, len += 1;
        if (*src0 != *src1)
            return len;
        src0 += 1, src1 += 1, len += 1;
        if (*src0 != *src1)
            return len;
        src0 += 1, src1 += 1, len += 1;
        if (*src0 != *src1)
            return len;
        src0 += 1, src1 += 1, len += 1;
    } while (len < 256);

    return 256;
}

class Compressor {
private:
    static constexpr size_t w_bits = 12;
    static constexpr size_t w_size = 1 << w_bits;
    static constexpr size_t w_mask = w_size - 1;
    static constexpr size_t window_size = 2 * w_size;

    static constexpr size_t good_match = 8;
    static constexpr size_t nice_match = 128;
    static constexpr size_t max_lazy_match = 32;
    static constexpr size_t max_chain_length = 256;

    static constexpr size_t HASH_BITS = 16u; /* log2(HASH_SIZE) */
    static constexpr size_t HASH_SIZE = 65536u; /* number of elements in hash table */
    static constexpr size_t HASH_MASK = (HASH_SIZE - 1u); /* HASH_SIZE-1 */

    static constexpr size_t MAX_DIST = w_size - MIN_LOOKAHEAD;

    std::ostream& output;
    std::bitset<8> groupHeader{0};
    std::array<uint8_t, 26> groupBuf{0}; //allocate extra byte in case every group is distance (uint32_t* cast saves a byte past the end)
    uint8_t savedChunks = 0;
    uint8_t bufPos = 1; //reserve first byte for header

    const uint8_t *input = nullptr;
    size_t inputLeft = 0;

    unsigned int  lookahead = 0; /* number of valid bytes ahead in window */

    std::array<uint16_t, w_size> prev{};
    /* Link to older string with same hash index. To limit the size of this
     * array to 64K, this link is maintained only for the last 32K strings.
     * An index in this array is thus a window index modulo 32K.
     */

    std::array<uint16_t, HASH_SIZE> head{}; /* Heads of the hash chains or 0. */

    unsigned int strstart = 0; /* start of string to insert */
    unsigned int match_start = 0; /* start of matching string */

    unsigned int prev_length = 0;
    /* Length of the best match at previous step. Matches not greater than this
     * are discarded. This is used in the lazy match evaluation.
     */

    void store_dist_chunk(const uint32_t& distance, const uint32_t& length) {
        if (length < 18) {
            const uint16_t write = Utility::Endian::toPlatform<uint16_t>(eType::Big, ((length - 2) << 12) | distance);
            *reinterpret_cast<uint16_t*>(&groupBuf[bufPos]) = write;
            bufPos += 2;
        } else {
            // If the match is longer than 18 bytes, 3 bytes are needed to write the match.
            const uint8_t len_to_write = std::min<size_t>(MaximumMatchLength, length) - 0x12;
            const uint32_t write = Utility::Endian::toPlatform(eType::Big, (distance << 16) | (len_to_write << 8));
            *reinterpret_cast<uint32_t*>(&groupBuf[bufPos]) = write;
            bufPos += 3;
        }

        ++savedChunks;
        if (savedChunks == ChunksPerGroup) {
            groupBuf[0] = uint8_t(groupHeader.to_ulong());
            output.write(reinterpret_cast<const char*>(groupBuf.data()), bufPos);
            reset_group();
        }
    }

    void store_lit_chunk(const uint8_t& lit) {
        groupHeader.set(7 - savedChunks);
        groupBuf[bufPos] = lit;
        bufPos += 1;
        
        ++savedChunks;
        if (savedChunks == ChunksPerGroup) {
            groupBuf[0] = uint8_t(groupHeader.to_ulong());
            output.write(reinterpret_cast<const char*>(groupBuf.data()), bufPos);
            reset_group();
        }
    }

    void reset_group() {
        groupHeader.reset();
        savedChunks = 0;

        groupBuf.fill(0);
        bufPos = 1;
    }

    uint16_t quick_insert_string(uint32_t str) {
        const uint8_t *pStr = input + str;

        const uint32_t val = *reinterpret_cast<const uint32_t*>(pStr);
        const uint32_t h = ((val * 2654435761U) >> 16) & HASH_MASK;

        uint16_t orig_head = head[h];
        if (LIKELY(orig_head != str)) {
            prev[str & w_mask] = orig_head;
            head[h] = (uint16_t)str;
        }
        return orig_head;
    }

    void insert_string(uint32_t str, uint32_t count) {
        const uint8_t *pStr = input + str;
        const uint8_t *strend = pStr + count;

        for (uint16_t idx = (uint16_t)str; pStr < strend; idx++, pStr++) {
            const uint32_t val = *reinterpret_cast<const uint32_t*>(pStr);
            const uint32_t h = ((val * 2654435761U) >> 16) & HASH_MASK;

            uint16_t orig_head = head[h];
            if (LIKELY(orig_head != idx)) {
                prev[idx & w_mask] = orig_head;
                head[h] = idx;
            }
        }
    }

    void slide_hash_c_chain(uint16_t* table, uint32_t entries) {
        table += entries;
        do {
            unsigned m = *--table;
            *table = (uint16_t)(m >= w_size ? m - w_size : 0);
            /* If entries is not on any hash chain, prev[entries] is garbage but
             * its value will never be used.
             */
        } while (--entries);
    }

    void slide_hash() {
        slide_hash_c_chain(head.data(), head.size());
        slide_hash_c_chain(prev.data(), prev.size());
    }

    size_t updateInputLen(const size_t& size) {
        const size_t len = std::min(inputLeft, size);
        inputLeft  -= len;
        return len;
    }

    void fill_window() {
        size_t more = window_size - lookahead - strstart; /* Amount of free space at the end of the window. */

        /* If the window is almost full and there is insufficient lookahead,
         * move the upper half to the lower one to make room in the upper half.
         */
        if (strstart >= window_size - MIN_LOOKAHEAD) {
            input += w_size;
            if (match_start >= w_size) {
                match_start -= w_size;
            } else {
                match_start = 0;
                prev_length = 0;
            }
            strstart -= w_size; /* we now have strstart >= MAX_DIST */
            slide_hash();
            more += w_size;
        }
        if (inputLeft == 0) return;

        lookahead += updateInputLen(more);

        /* Initialize the hash value now that we have some input: */
        if (lookahead >= STD_MIN_MATCH) {
            if (strstart >= 1) {
                quick_insert_string(strstart + 2 - STD_MIN_MATCH);
            }
        }
    }

    uint32_t longest_match(uint16_t cur_match) {

    #define GOTO_NEXT_CHAIN \
        if (--chain_length && (cur_match = prev[cur_match & w_mask]) > limit) continue; \
        return best_len;

        uint32_t best_len = prev_length ? prev_length : STD_MIN_MATCH-1;

        uint32_t offset = best_len-1; //if string doesn't match at this offset, can't be longer than the one we already have

        //don't search as far if match is already good
        uint32_t chain_length = max_chain_length;
        if (best_len >= good_match) chain_length >>= 2;

        //furthest location cur_match can be, stop once it is too far away
        const uint16_t limit = strstart > MAX_DIST ? (uint16_t)(strstart - MAX_DIST) : 0;
        
        for (;;) {
            if (cur_match >= strstart) break;

            /* Skip to next match if the match length cannot increase or if the match length is
             * less than 2. Note that the checks below for insufficient lookahead only occur
             * occasionally for performance reasons.
             * Therefore uninitialized memory will be accessed and conditional jumps will be made
             * that depend on those values. However the length of the match is limited to the
             * lookahead, so the output of deflate is not affected by the uninitialized values.
             */
            for (;;) {
                if (*reinterpret_cast<const uint16_t*>(input + strstart + offset) == *reinterpret_cast<const uint16_t*>(input + cur_match + offset) &&
                    *reinterpret_cast<const uint16_t*>(input + strstart) == *reinterpret_cast<const uint16_t*>(input + cur_match))
                    break;
                GOTO_NEXT_CHAIN;
            }
            
            uint32_t len = compare256(input + strstart + 2, input + cur_match + 2) + 2;

            if (len > best_len) {
                match_start = cur_match;

                /* Do not look for matches beyond the end of the input. */
                if (len > lookahead) return lookahead;
                best_len = len;
                if (best_len >= nice_match) return best_len;

                offset = best_len-1;
            }

            GOTO_NEXT_CHAIN;
        }

        return best_len;
    }

public:
    Compressor(const uint8_t* in, const size_t inSize, std::ostream& out) :
        input(in),
        inputLeft(inSize),
        output(out)
    {}

    int encode() {
        uint16_t prev_match = 0;
        bool match_available = 0;

        /* Process the input block. */
        for (;;) {
            /* Make sure that we always have enough lookahead, except
             * at the end of the input file. We need STD_MAX_MATCH bytes
             * for the next match, plus WANT_MIN_MATCH bytes to insert the
             * string following the next match.
             */
            if (lookahead < MIN_LOOKAHEAD) {
                fill_window();
                if (lookahead == 0) break; /* flush the current block */
            }

            /* Insert the string window[strstart .. strstart+2] in the
             * dictionary, and set hash_head to the head of the hash chain:
             */
            uint16_t hash_head = 0;
            if (LIKELY(lookahead >= WANT_MIN_MATCH)) {
                hash_head = quick_insert_string(strstart);
            }

            /* Find the longest match, discarding those <= prev_length.
             */
            prev_match = (uint16_t)match_start;
            uint32_t match_len = STD_MIN_MATCH - 1;
            int64_t dist = (int64_t)strstart - hash_head;

            if (dist <= MAX_DIST && dist > 0 && prev_length < max_lazy_match && hash_head != 0) {
                /* To simplify the code, we prevent matches with the string
                 * of window index 0 (in particular we have to avoid a match
                 * of the string with itself at the start of the input file).
                 */
                match_len = longest_match(hash_head);
                /* longest_match() sets match_start */
            }
            /* If there was a match at the previous step and the current
             * match is not better, output the previous match:
             */
            if (prev_length >= STD_MIN_MATCH && match_len <= prev_length) {
                unsigned int max_insert = strstart + lookahead - STD_MIN_MATCH;
                /* Do not insert strings in hash table beyond this. */

                //-2 from distance, instead of -1 here and -1 in store func
                //dont subtract min_match, we would add it again in store func anyway
                store_dist_chunk(strstart - 2 - prev_match, prev_length);

                /* Insert in hash table all strings up to the end of the match.
                 * strstart-1 and strstart are already inserted. If there is not
                 * enough lookahead, the last two strings are not inserted in
                 * the hash table.
                 */
                prev_length -= 1;
                lookahead -= prev_length;

                unsigned int mov_fwd = prev_length - 1;
                if (max_insert > strstart) {
                    unsigned int insert_cnt = mov_fwd;
                    if (UNLIKELY(insert_cnt > max_insert - strstart)) insert_cnt = max_insert - strstart;
                    insert_string(strstart + 1, insert_cnt);
                }
                prev_length = 0;
                match_available = false;
                strstart += mov_fwd + 1;
            } else if (match_available) {
                /* If there was no match at the previous position, output a
                 * single literal. If there was a match but the current match
                 * is longer, truncate the previous match to a single literal.
                 */
                store_lit_chunk(input[strstart-1]);
                prev_length = match_len;
                strstart++;
                lookahead--;
            } else {
                /* There is no previous match to compare with, wait for
                 * the next step to decide.
                 */
                prev_length = match_len;
                match_available = true;
                strstart++;
                lookahead--;
            }
        }
        if (UNLIKELY(match_available)) {
            store_lit_chunk(input[strstart-1]);
            match_available = false;
        }

        if (savedChunks != 0) {
            groupBuf[0] = uint8_t(groupHeader.to_ulong());
            output.write(reinterpret_cast<const char*>(groupBuf.data()), bufPos);
        }
        
        return 0;
    }
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

        //Put compressor on heap, hash table array makes it too large for Wii U thread stack
        std::unique_ptr<Compressor> compressor = std::make_unique<Compressor>(reinterpret_cast<const uint8_t*>(inData.data()), inData.size(), out);
        compressor->encode();

        return YAZ0Error::NONE;
    }
}
