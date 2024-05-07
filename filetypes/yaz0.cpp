#include "yaz0.hpp"

#include <string>
#include <cstring>
#include <bitset>
#include <sstream>

#include <utility/endian.hpp>
#include <utility/math.hpp>
#include <command/Log.hpp>

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

// Pulled from reverse engineered MK8 code at https://github.com/riidefi/RiiStudio/blob/master/source/szs/src/MK8.hpp
class Compressor {
public:
	static constexpr uint32_t getRequiredMemorySize() {
		return cWorkSize0 + cWorkSize1 + cWorkSize2;
	}

	static uint32_t encode(uint8_t* p_dst, const uint8_t* p_src, uint32_t src_size, uint8_t* p_work);

private:
	enum {
		cWorkNum1 = 0x8000,
		cWorkNum2 = 0x1000,

		cWorkSize0 = 0x2000,
		cWorkSize1 = cWorkNum1 * sizeof(int32_t),
		cWorkSize2 = cWorkNum2 * sizeof(int32_t)
	};

	struct Context {
		uint8_t* p_buffer;
		uint32_t _4;
		int32_t* p_work_1;
		int32_t* p_work_2;
		int32_t buffer_size;
	};

	struct Match {
		int32_t len;
		int32_t pos;
	};

	class PosIndex {
	public:
		PosIndex() : mValue(0) {}

		void pushBack(uint32_t value) {
			mValue <<= 5;
			mValue ^= value;
			mValue &= 0x7fff;
		}

		uint32_t value() const { return mValue; }

	private:
		uint32_t mValue;
	};

	static bool search(Match& match, int32_t pos, const Context& context);
};

bool Compressor::search(Match& match, int32_t pos, const Context& context) {
	const uint8_t* cmp2 = context.p_buffer + context._4;

	int32_t v0 = context._4 > 0x1000 ? static_cast<int32_t>(context._4 - 0x1000) : -1;

	int32_t cmp_pos = 2;
	match.len = cmp_pos;

	if (context._4 - pos <= 0x1000) {
		for (uint32_t i = 0; i < 0x1000; i++) {
			const uint8_t* cmp1 = context.p_buffer + pos;
			if (cmp1[0] == cmp2[0] && cmp1[1] == cmp2[1] && cmp1[cmp_pos] == cmp2[cmp_pos]) {
				int32_t len;

				for (len = 2; len < 0x111; len++) {
					if (cmp1[len] != cmp2[len]) break;
				}

				if (len > cmp_pos) {
					match.len = len;
					match.pos = cmp2 - cmp1;

					cmp_pos = context.buffer_size;
					if (len <= cmp_pos) {
						cmp_pos = match.len;
					}
					else {
						match.len = cmp_pos;
					}

					if (len >= 0x111) break;
				}
			}

			pos = context.p_work_2[pos & 0xfff];
			if (pos <= v0) break;
		}

		if (cmp_pos >= 3) return true;
	}

	return false;
}

uint32_t Compressor::encode(uint8_t* p_dst, const uint8_t* p_src, uint32_t src_size,
													 uint8_t* p_work) {
	uint8_t temp_buffer[24];
	uint32_t temp_size = 0;

	int32_t pos = -1;
	int32_t v1 = 0;
	int32_t bit = 8;

	Context context;

	context.p_buffer = p_work;

	context.p_work_1 = reinterpret_cast<int32_t *>(p_work + cWorkSize0);
	memset(context.p_work_1, static_cast<uint8_t>(-1), cWorkSize1);

	context.p_work_2 = reinterpret_cast<int32_t *>(p_work + (cWorkSize0 + cWorkSize1));
	memset(context.p_work_2, static_cast<uint8_t>(-1), cWorkSize2);

	context._4 = 0;

	uint32_t out_size = 0x10; // Header size
	uint32_t flag = 0;

	memcpy(p_dst, "Yaz0", 4);
	p_dst[4] = (src_size >> 24) & 0xff;
	p_dst[5] = (src_size >> 16) & 0xff;
	p_dst[6] = (src_size >> 8) & 0xff;
	p_dst[7] = (src_size >> 0) & 0xff;
	memset(p_dst + 8, 0, 0x10); // They probably meant to put 8 in the size here?

	PosIndex v2;

	context.buffer_size = std::min<uint32_t>(cWorkSize0, src_size);
	memcpy(context.p_buffer, p_src, context.buffer_size);

	v2.pushBack(context.p_buffer[0]);
	v2.pushBack(context.p_buffer[1]);

	Match match, next_match;
	match.len = 2;

	int32_t buffer_size_0 = context.buffer_size;
	int32_t buffer_size_1;

	while (context.buffer_size > 0) {
		while (true) {
			if (v1 == 0) {
				v2.pushBack(context.p_buffer[context._4 + 2]);

				context.p_work_2[context._4 & 0xfff] = context.p_work_1[v2.value()];
				context.p_work_1[v2.value()] = context._4;

				pos = context.p_work_2[context._4 & 0xfff];
			}
			else {
				v1 = 0;
			}

			if (pos != -1) {
				search(match, pos, context);
				if (2 < match.len && match.len < 0x111) {
					context._4++;
					context.buffer_size--;

					v2.pushBack(context.p_buffer[context._4 + 2]);

					context.p_work_2[context._4 & 0xfff] = context.p_work_1[v2.value()];
					context.p_work_1[v2.value()] = context._4;

					pos = context.p_work_2[context._4 & 0xfff];
					search(next_match, pos, context);
					if (match.len < next_match.len)
						match.len = 2;

					v1 = 1;
				}
			}

			if (match.len > 2) {
				flag = (flag & 0x7f) << 1;

				uint8_t low = match.pos - 1;
				uint8_t high = (match.pos - 1) >> 8;

				if (match.len < 18) {
					temp_buffer[temp_size++] = static_cast<uint8_t>((match.len - 2) << 4) | high;
					temp_buffer[temp_size++] = low;
				}
				else {
					temp_buffer[temp_size++] = high;
					temp_buffer[temp_size++] = low;
					temp_buffer[temp_size++] = static_cast<uint8_t>(match.len) - 18;
				}

				context.buffer_size -= match.len - v1;
				match.len -= v1 + 1;

				do {
					context._4++;

					v2.pushBack(context.p_buffer[context._4 + 2]);

					context.p_work_2[context._4 & 0xfff] = context.p_work_1[v2.value()];
					context.p_work_1[v2.value()] = context._4;

					pos = context.p_work_2[context._4 & 0xfff];
				} while (--match.len != 0);

				context._4++;
				v1 = 0;
				match.len = 0;
			}
			else {
				flag = (flag & 0x7f) << 1 | 1;

				temp_buffer[temp_size++] = context.p_buffer[context._4 - v1];

				if (v1 == 0) {
					context._4++;
					context.buffer_size--;
				}
			}

			if (--bit == 0) {
				p_dst[out_size++] = flag;

				memcpy(p_dst + out_size, temp_buffer, temp_size);
				out_size += temp_size;

				flag = 0;
				temp_size = 0;
				bit = 8;
			}

			if (context.buffer_size < 0x111 + 2) break;
		}

		int32_t v3 = context._4 - 0x1000;
		int32_t v4 = cWorkSize0 - v3;

		buffer_size_1 = buffer_size_0;

		if (context._4 >= 0x1000 + 14 * 0x111) {
			memcpy(context.p_buffer, context.p_buffer + v3, v4);

			int32_t v5 = cWorkSize0 - v4;
			buffer_size_1 = buffer_size_0 + v5;
			if (src_size < uint32_t(buffer_size_1)) {
				v5 = src_size - buffer_size_0;
				buffer_size_1 = src_size;
			}
			memcpy(context.p_buffer + v4, p_src + buffer_size_0, v5);
			context.buffer_size += v5;
			context._4 -= v3;

			for (uint32_t i = 0; i < cWorkNum1; i++) {
				context.p_work_1[i] = context.p_work_1[i] >= v3 ? context.p_work_1[i] - v3 : -1;
			}

			for (uint32_t i = 0; i < cWorkNum2; i++) {
				context.p_work_2[i] = context.p_work_2[i] >= v3 ? context.p_work_2[i] - v3 : -1;
			}
		}
		buffer_size_0 = buffer_size_1;
	}

	p_dst[out_size++] = flag << (bit & 0x3f);

	memcpy(p_dst + out_size, temp_buffer, temp_size);
	out_size += temp_size;

	return out_size;
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
			case YAZ0Error::ZNG_ERROR:
				return "ZNG_ERROR";
			case YAZ0Error::REACHED_EOF:
				return "REACHED_EOF";
			default:
				return "UNKNOWN";
		}
	}

	//TODO: not thread-safe
	//larger buffer is faster, but too large for stack
	constexpr uint32_t STATIC_READ_CHUNK_SIZE = 1024 * 1024 * 2;
	static char readChunkBuf[STATIC_READ_CHUNK_SIZE];

	YAZ0Error yaz0Decode(std::istream& in, std::ostream& out)
	{
		std::string inData{};

		Yaz0Header header{};

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
		// CLion claims this is always false
		if (const YAZ0Error error = yaz0DataDecode(inData.data(), &outData[0], header.uncompressedSize); error != YAZ0Error::NONE) {
			ErrorLog::getInstance().log(std ::string("Encountered error on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
			return error;
		}
		out.write(&outData[0], header.uncompressedSize);

		return YAZ0Error::NONE;
	}

	YAZ0Error yaz0Decode(std::stringstream& in, std::ostream& out)
	{
		Yaz0Header header{};

		LOG_AND_RETURN_IF_ERR(readYaz0Header(in, header));

		std::string outData(header.uncompressedSize, '\0'); //string instead of char array to avoid manual deletion
		if (const YAZ0Error error = yaz0DataDecode(&in.str()[0x10], &outData[0], header.uncompressedSize); error != YAZ0Error::NONE) {
			ErrorLog::getInstance().log(std ::string("Encountered error on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
			return error;
		}
		out.write(&outData[0], header.uncompressedSize);

		return YAZ0Error::NONE;
	}
	
	YAZ0Error yaz0Encode(const std::stringstream& in, std::ostream& out, uint32_t compressionLevel)
	{
		const std::string& inData = in.str();
    	std::vector<uint8_t> tmp(16 + roundUp<size_t>(inData.size(), 8) / 8 * 9 - 1);
		std::vector<uint8_t> work(Compressor::getRequiredMemorySize());
		const uint32_t outSize = Compressor::encode(tmp.data(), reinterpret_cast<const uint8_t*>(inData.data()), inData.size(), work.data());
		out.write(reinterpret_cast<const char*>(tmp.data()), outSize);

		return YAZ0Error::NONE;
	}
}
