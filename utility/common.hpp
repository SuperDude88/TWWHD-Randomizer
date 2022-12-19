#pragma once

#include <string>
#include <fstream>
#include <limits>
#include <type_traits>

#include <utility/endian.hpp>


template<typename T> requires std::is_arithmetic_v<T>
struct RGBA {
	T R = 0;
	T G = 0;
	T B = 0;
	T A = std::numeric_limits<T>::max();

	RGBA() {}

	RGBA(const T& val, const T& alpha) :
		R(val),
		G(val),
		B(val),
		A(alpha)
	{}

	RGBA(const T& r_, const T& g_, const T& b_ , const T& a_) :
		R(r_),
		G(g_),
		B(b_),
		A(a_)
	{}
};

template<typename T> requires std::is_arithmetic_v<T>
struct vec2 {
	T X;
	T Y;

	vec2() {}
	vec2(const T& val) :
		X(val),
		Y(val)
	{}
};

template<typename T> requires std::is_arithmetic_v<T>
struct vec3 {
	T X;
	T Y;
	T Z;

	vec3() {}
	vec3(const T& val) :
		X(val),
		Y(val),
		Z(val)
	{}
};

template<typename T> requires std::is_arithmetic_v<T>
struct vec4 {
	T A;
	T B;
	T C;
	T D;

	vec4() {}
	vec4(const T& val) :
		A(val),
		B(val),
		C(val),
		D(val)
	{}
};


template<typename T>
bool readVec2(std::istream& in, const std::streamoff offset, vec2<T>& out) {
	in.seekg(offset, std::ios::beg);

	if (!in.read(reinterpret_cast<char*>(&out.X), sizeof(out.X))) return false;
	if (!in.read(reinterpret_cast<char*>(&out.Y), sizeof(out.Y))) return false;

	if constexpr (sizeof(T) > 1) {
		Utility::Endian::toPlatform_inplace(Utility::Endian::Type::Big, out.X);
		Utility::Endian::toPlatform_inplace(Utility::Endian::Type::Big, out.Y);
	}

	return true;
}

template<typename T>
void writeVec2(std::ostream& out, const vec2<T>& vec) {
	if constexpr (sizeof(T) > 1) {
		T X_BE = Utility::Endian::toPlatform(Utility::Endian::Type::Big, vec.X);
		T Y_BE = Utility::Endian::toPlatform(Utility::Endian::Type::Big, vec.Y);

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
		Utility::Endian::toPlatform_inplace(Utility::Endian::Type::Big, out.X);
		Utility::Endian::toPlatform_inplace(Utility::Endian::Type::Big, out.Y);
		Utility::Endian::toPlatform_inplace(Utility::Endian::Type::Big, out.Z);
	}

	return true;
}

template<typename T>
void writeVec3(std::ostream& out, const vec3<T>& vec) {
	if constexpr (sizeof(T) > 1) {
		T X_BE = Utility::Endian::toPlatform(Utility::Endian::Type::Big, vec.X);
		T Y_BE = Utility::Endian::toPlatform(Utility::Endian::Type::Big, vec.Y);
		T Z_BE = Utility::Endian::toPlatform(Utility::Endian::Type::Big, vec.Z);

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


template<typename T>
bool readVec4(std::istream& in, const std::streamoff offset, vec4<T>& out) {
	in.seekg(offset, std::ios::beg);

	if (!in.read(reinterpret_cast<char*>(&out.A), sizeof(out.A))) return false;
	if (!in.read(reinterpret_cast<char*>(&out.B), sizeof(out.B))) return false;
	if (!in.read(reinterpret_cast<char*>(&out.C), sizeof(out.C))) return false;
	if (!in.read(reinterpret_cast<char*>(&out.D), sizeof(out.D))) return false;

	if constexpr (sizeof(T) > 1) {
		Utility::Endian::toPlatform_inplace(Utility::Endian::Type::Big, out.A);
		Utility::Endian::toPlatform_inplace(Utility::Endian::Type::Big, out.B);
		Utility::Endian::toPlatform_inplace(Utility::Endian::Type::Big, out.C);
		Utility::Endian::toPlatform_inplace(Utility::Endian::Type::Big, out.D);
	}

	return true;
}

template<typename T>
void writeVec4(std::ostream& out, const vec4<T>& vec) {
	if constexpr (sizeof(T) > 1) {
		T A_BE = Utility::Endian::toPlatform(Utility::Endian::Type::Big, vec.A);
		T B_BE = Utility::Endian::toPlatform(Utility::Endian::Type::Big, vec.B);
		T C_BE = Utility::Endian::toPlatform(Utility::Endian::Type::Big, vec.C);
		T D_BE = Utility::Endian::toPlatform(Utility::Endian::Type::Big, vec.D);

		out.write(reinterpret_cast<const char*>(&A_BE), sizeof(A_BE));
		out.write(reinterpret_cast<const char*>(&B_BE), sizeof(B_BE));
		out.write(reinterpret_cast<const char*>(&C_BE), sizeof(C_BE));
		out.write(reinterpret_cast<const char*>(&D_BE), sizeof(D_BE));
		return;
	}
	else {
		out.write(reinterpret_cast<const char*>(&vec.A), sizeof(vec.A));
		out.write(reinterpret_cast<const char*>(&vec.B), sizeof(vec.B));
		out.write(reinterpret_cast<const char*>(&vec.C), sizeof(vec.C));
		out.write(reinterpret_cast<const char*>(&vec.D), sizeof(vec.D));
		return;
	}
}


template<typename T>
bool readRGBA(std::istream& in, const std::streamoff& offset, RGBA<T>& out) {
	in.seekg(offset, std::ios::beg);

	if(!in.read(reinterpret_cast<char*>(&out.R), sizeof(out.R))) return false;
	if(!in.read(reinterpret_cast<char*>(&out.G), sizeof(out.G))) return false;
	if(!in.read(reinterpret_cast<char*>(&out.B), sizeof(out.B))) return false;
	if(!in.read(reinterpret_cast<char*>(&out.A), sizeof(out.A))) return false;
	
	if constexpr (sizeof(T) > 1) {
		Utility::Endian::toPlatform_inplace(Utility::Endian::Type::Big, out.R);
		Utility::Endian::toPlatform_inplace(Utility::Endian::Type::Big, out.G);
		Utility::Endian::toPlatform_inplace(Utility::Endian::Type::Big, out.B);
		Utility::Endian::toPlatform_inplace(Utility::Endian::Type::Big, out.A);
	}

	return true;
}

template<typename T>
void writeRGBA(std::ostream& out, const RGBA<T>& color) {
	if constexpr (sizeof(T) > 1) {
		T R_BE = Utility::Endian::toPlatform(Utility::Endian::Type::Big, color.R);
		T G_BE = Utility::Endian::toPlatform(Utility::Endian::Type::Big, color.G);
		T B_BE = Utility::Endian::toPlatform(Utility::Endian::Type::Big, color.B);
		T A_BE = Utility::Endian::toPlatform(Utility::Endian::Type::Big, color.A);

		out.write(reinterpret_cast<const char*>(&R_BE), sizeof(R_BE));
		out.write(reinterpret_cast<const char*>(&G_BE), sizeof(G_BE));
		out.write(reinterpret_cast<const char*>(&B_BE), sizeof(B_BE));
		out.write(reinterpret_cast<const char*>(&A_BE), sizeof(A_BE));
		return;
	}
	else {
		out.write(reinterpret_cast<const char*>(&color.R), sizeof(color.R));
		out.write(reinterpret_cast<const char*>(&color.G), sizeof(color.G));
		out.write(reinterpret_cast<const char*>(&color.B), sizeof(color.B));
		out.write(reinterpret_cast<const char*>(&color.A), sizeof(color.A));
	}

	return;	
}

std::string readNullTerminatedStr(std::istream& in, const unsigned int& offset);

std::u16string readNullTerminatedWStr(std::istream& in, const unsigned int offset);

template<typename error_enum> requires requires {
	requires std::is_enum_v<error_enum>;
	error_enum::NONE;
	error_enum::REACHED_EOF;
	error_enum::UNEXPECTED_VALUE;
}
error_enum readPadding(std::istream& in, const unsigned int& len, const char* val = nullptr) {
	if (in.tellg() % len != 0) {
	    const size_t& padding_size = len - (static_cast<size_t>(in.tellg()) % len);

	    std::string padding(padding_size, '\0');
	    if (!in.read(&padding[0], static_cast<std::streamoff>(padding_size))) return error_enum::REACHED_EOF;

		if(val != nullptr) {
	    	for (const char& character : padding) {
	    	    if (character != *val) return error_enum::UNEXPECTED_VALUE;
	    	}
		}
	}

	return error_enum::NONE;
}

size_t padToLen(std::ostream& out, const unsigned int& len, const char pad = '\x00');

typedef RGBA<uint8_t> RGBA8;
