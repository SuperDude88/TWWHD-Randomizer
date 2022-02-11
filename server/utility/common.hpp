#pragma once

#include <string>
#include <fstream>

#include "byteswap.hpp"



struct RGBA {
	uint8_t R;
	uint8_t G;
	uint8_t B;
	uint8_t A;
};

template<typename T>
struct vec2 {
	T X;
	T Y;
};

template<typename T>
struct vec3 {
	T X;
	T Y;
	T Z;
};


template<typename T>
bool readVec2(std::istream& in, const std::streamoff offset, vec2<T>& out) {
	in.seekg(offset, std::ios::beg);

	if (!in.read(reinterpret_cast<char*>(&out.X), sizeof(out.X))) return false;
	if (!in.read(reinterpret_cast<char*>(&out.Y), sizeof(out.Y))) return false;

	if constexpr (sizeof(T) > 1) {
		Utility::byteswap_inplace(out.X);
		Utility::byteswap_inplace(out.Y);
	}


	return true;
}

template<typename T>
void writeVec2(std::ostream& out, const vec2<T>& vec) {
	if constexpr (sizeof(T) > 1) {
		T X_BE = Utility::byteswap(vec.X);
		T Y_BE = Utility::byteswap(vec.Y);

		out.write(reinterpret_cast<const char*>(&X_BE), sizeof(X_BE));
		out.write(reinterpret_cast<const char*>(&Y_BE), sizeof(Y_BE));
		return;
	}
	else {
		out.write(reinterpret_cast<const char*>(&vec.X), sizeof(vec.X));
		out.write(reinterpret_cast<const char*>(&vec.Y), sizeof(vec.Y));
		return;
	}
}


template<typename T>
bool readVec3(std::istream& in, const std::streamoff offset, vec3<T>& out) {
	in.seekg(offset, std::ios::beg);

	if (!in.read(reinterpret_cast<char*>(&out.X), sizeof(out.X))) return false;
	if (!in.read(reinterpret_cast<char*>(&out.Y), sizeof(out.Y))) return false;
	if (!in.read(reinterpret_cast<char*>(&out.Z), sizeof(out.Z))) return false;

	if constexpr (sizeof(T) > 1) {
		Utility::byteswap_inplace(out.X);
		Utility::byteswap_inplace(out.Y);
		Utility::byteswap_inplace(out.Z);
	}

	return true;
}

template<typename T>
void writeVec3(std::ostream& out, const vec3<T>& vec) {
	if constexpr (sizeof(T) > 1) {
		T X_BE = Utility::byteswap(vec.X);
		T Y_BE = Utility::byteswap(vec.Y);
		T Z_BE = Utility::byteswap(vec.Z);

		out.write(reinterpret_cast<const char*>(&X_BE), sizeof(X_BE));
		out.write(reinterpret_cast<const char*>(&Y_BE), sizeof(Y_BE));
		out.write(reinterpret_cast<const char*>(&Z_BE), sizeof(Z_BE));
		return;
	}
	else {
		out.write(reinterpret_cast<const char*>(&vec.X), sizeof(vec.X));
		out.write(reinterpret_cast<const char*>(&vec.Y), sizeof(vec.Y));
		out.write(reinterpret_cast<const char*>(&vec.Z), sizeof(vec.Z));
		return;
	}
}


bool readRGBA8(std::istream& in, const std::streamoff& offset, RGBA& out);

void writeRGBA8(std::ostream& out, const RGBA& color);


std::string readNullTerminatedStr(std::istream& in, const unsigned int& offset);

unsigned int padToLen(std::ostream& out, const unsigned int& len, const char pad = '\x00');