#pragma once
#include <cstring>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <fstream>
#include "../utility/byteswap.hpp"

enum struct JPCError {
	NONE = 0,
	COULD_NOT_OPEN,
	NOT_JPC,
	UNEXPECTED_VALUE,
	DUPLICATE_PARTICLE_ID,
	DUPLICATE_FILENAME,
	MISSING_PARTICLE,
	MISSING_TEXTURE,
	REACHED_EOF,
	HEADER_DATA_NOT_LOADED,
	FILE_DATA_NOT_LOADED,
	UNKNOWN,
	COUNT
};

struct RGBA {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

struct ColorAnimationKeyframe {
	uint16_t time;
	RGBA color;
};

struct BSP1 {
	uint8_t color_flags;
	RGBA color_prm;
	RGBA color_env;

	uint16_t color_prm_anm_data_offset;
	uint8_t color_prm_anm_data_count;
	std::vector<ColorAnimationKeyframe> color_prm_anm_table;

	uint16_t color_env_anm_data_offset;
	uint8_t color_env_anm_data_count;
	std::vector<ColorAnimationKeyframe> color_env_anm_table;
};

struct SSP1 {
	RGBA color_prm;
	RGBA color_env;
};

struct TDB1 {
	std::vector<uint16_t> texture_ids;
	std::vector<std::string> texture_filenames;
};

struct TEX1 {
	std::string filename; //In SD the texture data is also stored, but not in HD
	//Size field looks to be leftover from SD and should be ignored
};

struct chunk {
	std::string magic;
	int size;
	std::string data;
};

namespace chunks {
	void BSP1_read(BSP1& bsp, const chunk& chunk);
	void SSP1_read(SSP1& ssp, const chunk& chunk);
	void TDB1_read(TDB1& tdb, const chunk& chunk);
	void TEX1_read(TEX1& tex, const chunk& chunk);

	void BSP1_save(BSP1& object, chunk& chunk);
	void SSP1_save(SSP1& object, chunk& chunk);
	void TDB1_save(TDB1& object, chunk& chunk);
	void TEX1_save(TEX1& object, chunk& chunk);
}

class Particle {
public:
	std::string data;

	char magicJEFF[8];
	uint32_t unknown_1;
	uint32_t num_chunks;
	uint32_t size;

	uint8_t num_kfa1_chunks;
	uint8_t num_fld1_chunks;
	uint8_t num_textures;
	uint8_t unknown_5;

	uint16_t particle_id;

	uint8_t unknown_6[6];

	std::vector<chunk> chunks;
	std::unordered_map<std::string, chunk> chunks_by_type;

	TDB1 tdb1;

	JPCError read(std::istream& jpc, int particle_offset);
	JPCError save_changes(); //Instead of updating all chunk data with variables here, rely on user to save them to help eliminate dynamic types (chunk save will update the data field)
};

namespace FileTypes {

	const char* JPCErrorGetName(JPCError err);

	class JPC {
	public:
		char magicJPAC[8];
		uint16_t num_particles;
		uint16_t num_textures;

		std::vector<Particle> particles;
		std::unordered_map<int, Particle> particles_by_id;

		std::vector<chunk> textures;
		std::unordered_map<std::string, chunk> textures_by_filename;

		JPC();
		static JPC createNew(const std::string& filename);
		JPCError loadFromBinary(std::istream& jpc);
		JPCError loadFromFile(const std::string& filePath);
		JPCError addParticle(Particle particle);
		JPCError replaceParticle(Particle particle);
		JPCError addTexture(TEX1& tex);
		JPCError replaceTexture(TEX1& tex);
		JPCError writeToStream(std::ostream& out);
		JPCError writeToFile(const std::string& outFilePath);
	private:
		void initNew();
	};
}
