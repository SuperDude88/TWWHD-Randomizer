#include "common.hpp"



bool readRGBA8(std::istream& in, const std::streamoff& offset, RGBA& out) {
	in.seekg(offset, std::ios::beg);

	if(!in.read(reinterpret_cast<char*>(&out.R), sizeof(out.R))) return false;
	if(!in.read(reinterpret_cast<char*>(&out.G), sizeof(out.G))) return false;
	if(!in.read(reinterpret_cast<char*>(&out.B), sizeof(out.B))) return false;
	if(!in.read(reinterpret_cast<char*>(&out.A), sizeof(out.A))) return false;

	return true;
}

void writeRGBA8(std::ostream& out, const RGBA& color) {
	out.write(reinterpret_cast<const char*>(&color.R), sizeof(color.R));
	out.write(reinterpret_cast<const char*>(&color.G), sizeof(color.G));
	out.write(reinterpret_cast<const char*>(&color.B), sizeof(color.B));
	out.write(reinterpret_cast<const char*>(&color.A), sizeof(color.A));

	return;
}


std::string readNullTerminatedStr(std::istream& in, const unsigned int& offset) {
	in.seekg(offset, std::ios::beg);

	std::string ret;
	char character = '\0';
	do {
		if (!in.read(&character, sizeof(char))) {
			ret.clear();
			return ret;
		}
		ret += character;
	} while (character != '\0');

	return ret;
}

std::u16string readNullTerminatedWStr(std::istream& in, const unsigned int offset) {
	in.seekg(offset, std::ios::beg);

	std::u16string ret;
	char16_t character = u'\0';
	do {
		if (!in.read(reinterpret_cast<char*>(&character), sizeof(char16_t))) {
			ret.clear();
			return ret;
		}
		ret += character;
	} while (character != u'\0');

	return ret;
}


unsigned int padToLen(std::ostream& out, const unsigned int& len, const char pad) {
	if (len == 0) return 0; //don't pad to no alignment (also cant % by 0)

	unsigned int padLen = len - (out.tellp() % len);
	if (padLen == len) return 0; //doesnt write any padding, return length 0

	for (unsigned int i = 0; i < padLen; i++) {
		out.write(&pad, 1);
	}

	return padLen; //return number of bytes written
}
