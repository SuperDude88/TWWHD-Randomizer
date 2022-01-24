#include "jpc.hpp"

std::vector<ColorAnimationKeyframe> read_color_table(const std::string& data, int color_data_offset, int color_data_count) {
	std::vector<ColorAnimationKeyframe> table;
	table.reserve(color_data_count);
	for (int i = 0; i < color_data_count; i++) {
		ColorAnimationKeyframe keyframe;
		keyframe.time = *(uint16_t*)&data[color_data_offset + i * 6];
		Utility::byteswap_inplace(keyframe.time);

		keyframe.color.r = *(uint8_t*)&data[color_data_offset + i * 6 + 2];
		keyframe.color.g = *(uint8_t*)&data[color_data_offset + i * 6 + 3];
		keyframe.color.b = *(uint8_t*)&data[color_data_offset + i * 6 + 4];
		keyframe.color.a = *(uint8_t*)&data[color_data_offset + i * 6 + 5];
		table.push_back(keyframe);
	}
	return table;
}

void save_color_table(std::string& data, std::vector<ColorAnimationKeyframe> color_table, int color_data_offset) {
	for (unsigned int i = 0; i < color_table.size(); i++) {
		ColorAnimationKeyframe& keyframe = color_table[i];
		Utility::byteswap_inplace(keyframe.time);
		data.replace(color_data_offset + i * 6, 2, (char*)&keyframe.time, 2);

		data.replace(color_data_offset + i * 6 + 2, 1, (char*)&keyframe.color.r, 1);
		data.replace(color_data_offset + i * 6 + 3, 1, (char*)&keyframe.color.g, 1);
		data.replace(color_data_offset + i * 6 + 4, 1, (char*)&keyframe.color.b, 1);
		data.replace(color_data_offset + i * 6 + 5, 1, (char*)&keyframe.color.a, 1);
	}
	return;
}

std::vector<uint16_t> read_texture_ids(const chunk& chunk, int num_tex_ids) {
	std::vector<uint16_t> temp;
	for (int tex_id_index = 0; tex_id_index < num_tex_ids; tex_id_index++) {
		uint16_t tex_id = *(uint16_t*)&chunk.data[0xC + tex_id_index * 2];
		Utility::byteswap_inplace(tex_id);

		temp.push_back(tex_id);
	}
	return temp;
}

namespace chunks {
	void BSP1_read(BSP1& bsp, const chunk& chunk) {
		bsp.color_flags = *(uint8_t*)&chunk.data[0xC + 0x1B];

		bsp.color_prm.r = *(uint8_t*)&chunk.data[0xC + 0x20];
		bsp.color_prm.g = *(uint8_t*)&chunk.data[0xC + 0x21];
		bsp.color_prm.b = *(uint8_t*)&chunk.data[0xC + 0x22];
		bsp.color_prm.a = *(uint8_t*)&chunk.data[0xC + 0x23];

		bsp.color_env.r = *(uint8_t*)&chunk.data[0xC + 0x24];
		bsp.color_env.g = *(uint8_t*)&chunk.data[0xC + 0x25];
		bsp.color_env.b = *(uint8_t*)&chunk.data[0xC + 0x26];
		bsp.color_env.a = *(uint8_t*)&chunk.data[0xC + 0x27];

		bsp.color_prm_anm_data_count = 0;
		bsp.color_prm_anm_table = {};
		if ((bsp.color_flags & 0x02) != 0) {
			bsp.color_prm_anm_data_offset = *(uint16_t*)&chunk.data[0xC + 0x4];
			Utility::byteswap_inplace(bsp.color_prm_anm_data_offset);

			bsp.color_prm_anm_data_count = *(uint8_t*)&chunk.data[0xC + 0x1C];
			bsp.color_prm_anm_table = read_color_table(chunk.data, bsp.color_prm_anm_data_offset, bsp.color_prm_anm_data_count);
		}

		bsp.color_env_anm_data_count = 0;
		bsp.color_env_anm_table = {};
		if ((bsp.color_flags & 0x08) != 0) {
			bsp.color_env_anm_data_offset = *(uint16_t*)&chunk.data[0xC + 0x6];
			Utility::byteswap_inplace(bsp.color_env_anm_data_offset);

			bsp.color_env_anm_data_count = *(uint8_t*)&chunk.data[0xC + 0x1D];
			bsp.color_env_anm_table = read_color_table(chunk.data, bsp.color_env_anm_data_offset, bsp.color_env_anm_data_count);
		}

		return;
	}

	void SSP1_read(SSP1& ssp, const chunk& chunk) {
		ssp.color_prm.r = *(uint8_t*)&chunk.data[0xC + 0x3C];
		ssp.color_prm.g = *(uint8_t*)&chunk.data[0xC + 0x3D];
		ssp.color_prm.b = *(uint8_t*)&chunk.data[0xC + 0x3E];
		ssp.color_prm.a = *(uint8_t*)&chunk.data[0xC + 0x3F];

		ssp.color_env.r = *(uint8_t*)&chunk.data[0xC + 0x40];
		ssp.color_env.g = *(uint8_t*)&chunk.data[0xC + 0x41];
		ssp.color_env.b = *(uint8_t*)&chunk.data[0xC + 0x42];
		ssp.color_env.a = *(uint8_t*)&chunk.data[0xC + 0x43];
		return;
	}

	void TDB1_read(TDB1& tdb, const chunk& chunk) {
		tdb.texture_ids = {};
		tdb.texture_filenames = {};
		return;
	}

	void TEX1_read(TEX1& tex, const chunk& chunk) {
		int data_length = chunk.data.length();
		int temp_offset = 0xC;
		int str_length = 0;
		while (temp_offset < data_length) {
			char character = chunk.data[temp_offset];
			if (character == '\0') {
				break;
			}
			else {
				str_length = str_length + 1;
			}
			temp_offset = temp_offset + 1;
		}

		tex.filename.resize(str_length);
		strncpy(&tex.filename[0], &chunk.data[0xC], str_length);
		return;
	}

	void BSP1_save(BSP1& bsp, chunk& chunk) {
		chunk.data.replace(0xC + 0x1B, 1, (char*)&bsp.color_flags, 1);

		chunk.data.replace(0xC + 0x20, 1, (char*)&bsp.color_prm.r, 1);
		chunk.data.replace(0xC + 0x21, 1, (char*)&bsp.color_prm.g, 1);
		chunk.data.replace(0xC + 0x22, 1, (char*)&bsp.color_prm.b, 1);
		chunk.data.replace(0xC + 0x23, 1, (char*)&bsp.color_prm.a, 1);

		chunk.data.replace(0xC + 0x24, 1, (char*)&bsp.color_env.r, 1);
		chunk.data.replace(0xC + 0x25, 1, (char*)&bsp.color_env.g, 1);
		chunk.data.replace(0xC + 0x26, 1, (char*)&bsp.color_env.b, 1);
		chunk.data.replace(0xC + 0x27, 1, (char*)&bsp.color_env.a, 1);

		if ((bsp.color_flags & 0x02) != 0) {
			if (bsp.color_prm_anm_table.size() != bsp.color_prm_anm_data_count) { //Changing size not implemented
				return;
			}
			save_color_table(chunk.data, bsp.color_prm_anm_table, bsp.color_prm_anm_data_offset);
		}

		if ((bsp.color_flags & 0x08) != 0) {
			if (bsp.color_env_anm_table.size() != bsp.color_env_anm_data_count) { //Changing size not implemented
				return;
			}
			save_color_table(chunk.data, bsp.color_env_anm_table, bsp.color_env_anm_data_offset);
		}

		return;
	}

	void SSP1_save(SSP1& ssp, chunk& chunk) {
		chunk.data.replace(0xC + 0x3C, 1, (char*)&ssp.color_prm.r, 1);
		chunk.data.replace(0xC + 0x3D, 1, (char*)&ssp.color_prm.g, 1);
		chunk.data.replace(0xC + 0x3E, 1, (char*)&ssp.color_prm.b, 1);
		chunk.data.replace(0xC + 0x3F, 1, (char*)&ssp.color_prm.a, 1);

		chunk.data.replace(0xC + 0x40, 1, (char*)&ssp.color_env.r, 1);
		chunk.data.replace(0xC + 0x41, 1, (char*)&ssp.color_env.g, 1);
		chunk.data.replace(0xC + 0x42, 1, (char*)&ssp.color_env.b, 1);
		chunk.data.replace(0xC + 0x43, 1, (char*)&ssp.color_env.a, 1);
		return;
	}

	void TDB1_save(TDB1& tdb, chunk& chunk) {
		for (unsigned int tex_id_index = 0; tex_id_index < tdb.texture_ids.size(); tex_id_index++) {
			Utility::byteswap_inplace(tdb.texture_ids[tex_id_index]);
			chunk.data.replace(0xC + tex_id_index * 2, 2, (char*)&tdb.texture_ids[tex_id_index], 2);
		}
		return;
	}

	void TEX1_save(TEX1& tex, chunk& chunk) {
		return;
	}
}

JPCError Particle::read(std::istream& jpc, int particle_offset) {
	jpc.seekg(particle_offset, std::ios::beg);
	if (!jpc.read(magicJEFF, 8)) {
		return JPCError::REACHED_EOF;
	}
	if (strncmp(magicJEFF, "JEFFjpa1", 8) != 0) {
		return JPCError::UNEXPECTED_VALUE;
	}
	if (!jpc.read((char*)&unknown_1, sizeof(unknown_1))) {
		return JPCError::REACHED_EOF;
	}
	if (!jpc.read((char*)&num_chunks, sizeof(num_chunks))) {
		return JPCError::REACHED_EOF;
	}
	if (!jpc.read((char*)&size, sizeof(size))) {
		return JPCError::REACHED_EOF;
	}
	if (!jpc.read((char*)&num_kfa1_chunks, sizeof(num_kfa1_chunks))) {
		return JPCError::REACHED_EOF;
	}
	if (!jpc.read((char*)&num_fld1_chunks, sizeof(num_fld1_chunks))) {
		return JPCError::REACHED_EOF;
	}
	if (!jpc.read((char*)&num_textures, sizeof(num_textures))) {
		return JPCError::REACHED_EOF;
	}
	if (!jpc.read((char*)&unknown_5, sizeof(unknown_5))) {
		return JPCError::REACHED_EOF;
	}
	if (!jpc.read((char*)&particle_id, sizeof(particle_id))) {
		return JPCError::REACHED_EOF;
	}
	if (!jpc.read((char*)&unknown_6, sizeof(unknown_6))) {
		return JPCError::REACHED_EOF;
	}

	Utility::byteswap_inplace(unknown_1);
	Utility::byteswap_inplace(num_chunks);
	Utility::byteswap_inplace(size);
	Utility::byteswap_inplace(particle_id);

	int chunk_offset = particle_offset + 0x20;
	chunks.reserve(num_chunks);
	for (unsigned int chunk_index = 0; chunk_index < num_chunks; chunk_index++) {
		chunk tempChunk;
		jpc.seekg(chunk_offset, std::ios::beg);
		tempChunk.magic.resize(4);
		if (!jpc.read(&tempChunk.magic[0], 4)) {
			return JPCError::REACHED_EOF;
		}
		if (!jpc.read((char*)&tempChunk.size, 4)) {
			return JPCError::REACHED_EOF;
		}
		Utility::byteswap_inplace(tempChunk.size);

		jpc.seekg(chunk_offset, std::ios::beg);
		tempChunk.data.resize(tempChunk.size);
		if (!jpc.read(&tempChunk.data[0], tempChunk.size)) {
			return JPCError::REACHED_EOF;
		}
		chunks.push_back(tempChunk);
		chunks_by_type[tempChunk.magic] = tempChunk;

		chunk_offset += tempChunk.size;
	}

	tdb1.texture_ids = read_texture_ids(chunks_by_type["TDB1"], num_textures);

	int true_size = chunk_offset - particle_offset;
	jpc.seekg(particle_offset, std::ios::beg);
	data.resize(true_size);
	if (!jpc.read(&data[0], true_size)) {
		return JPCError::REACHED_EOF;
	}

	return JPCError::NONE;
}

JPCError Particle::save_changes() {
	Particle::num_textures = num_textures;

	int offset = 0x20;

	num_textures = tdb1.texture_ids.size();

	for (const chunk& chunk : Particle::chunks) {
		data.replace(offset, chunk.data.size(), chunk.data);
		offset += chunk.data.size();
	}

	data.replace(0, 8, magicJEFF, 8);
	Utility::byteswap_inplace(size);
	data.replace(0x10, 4, (char*)&size, 4);

	return JPCError::NONE;
}

namespace FileTypes {

	const char* JPCErrorGetName(JPCError err) {
		switch (err) {
		case JPCError::NONE:
			return "NONE";
		case JPCError::COULD_NOT_OPEN:
			return "COULD_NOT_OPEN";
		case JPCError::NOT_JPC:
			return "NOT_JPC";
		case JPCError::UNEXPECTED_VALUE:
			return "UNEXPECTED_VALUE";
		case JPCError::DUPLICATE_PARTICLE_ID:
			return "DUPLICATE_PARTICLE_ID";
		case JPCError::DUPLICATE_FILENAME:
			return "DUPLICATE_FILENAME";
		case JPCError::MISSING_PARTICLE:
			return "MISSING_PARTICLE";
		case JPCError::MISSING_TEXTURE:
			return "MISSING_TEXTURE";
		case JPCError::REACHED_EOF:
			return "REACHED_EOF";
		case JPCError::HEADER_DATA_NOT_LOADED:
			return "HEADER_DATA_NOT_LOADED";
		case JPCError::FILE_DATA_NOT_LOADED:
			return "FILE_DATA_NOT_LOADED";
		case JPCError::COUNT:
			return "COUNT";
		default:
			return "UNKNOWN";
		}

	}

	JPC::JPC() {

	}

	void JPC::initNew() {
		memcpy(magicJPAC, "JPAC1-00", 8);
		num_particles = 0;
		num_textures = 0;

		particles = {};
		particles_by_id = {};

		textures = {};
		textures_by_filename = {};
	}

	JPC JPC::createNew(const std::string& filename) {
		JPC newJPC{};
		newJPC.initNew();
		return newJPC;
	}

	JPCError JPC::loadFromBinary(std::istream& jpc) {
		JPCError err = JPCError::NONE;
		if (!jpc.read(magicJPAC, 8)) {
			return JPCError::REACHED_EOF;
		}
		if (strncmp(magicJPAC, "JPAC1-00", 8) != 0) {
			return JPCError::NOT_JPC;
		}
		if (!jpc.read((char*)&num_particles, sizeof(num_particles))) {
			return JPCError::REACHED_EOF;
		}
		if (!jpc.read((char*)&num_textures, sizeof(num_textures))) {
			return JPCError::REACHED_EOF;
		}
		Utility::byteswap_inplace(num_particles);
		Utility::byteswap_inplace(num_textures);

		particles = {};
		particles_by_id = {};
		int offset = 0x20;
		particles.reserve(num_particles);
		particles_by_id.reserve(num_particles);
		for (int particle_index = 0; particle_index < num_particles; particle_index++) {
			Particle particle;
			err = particle.read(jpc, offset);
			if (err != JPCError::NONE) {
				return err;
			}
			particles.push_back(particle);

			if (particles_by_id.find(particle.particle_id) != particles_by_id.end()) {
				return JPCError::DUPLICATE_PARTICLE_ID;
			}
			particles_by_id[particle.particle_id] = particle;

			offset += 0x20;
			for (const chunk& chunk : particle.chunks) {
				offset += chunk.size;
			}
		}

		textures = {};
		textures_by_filename = {};
		textures.reserve(num_textures);
		for (int tex_index = 0; tex_index < num_textures; tex_index++) {
			chunk tex;
			char chunk_magic[4];
			if (!jpc.read(chunk_magic, 4)) {
				return JPCError::REACHED_EOF;
			}
			if (strncmp(chunk_magic, "TEX1", 4) != 0) {
				return JPCError::UNEXPECTED_VALUE;
			}

			tex.magic.resize(4);
			strncpy(&tex.magic[0], chunk_magic, 4);
			if (!jpc.read((char*)&tex.size, 4)) {
				return JPCError::REACHED_EOF;
			}
			Utility::byteswap_inplace(tex.size);

			jpc.seekg(-8, std::ios::cur);
			tex.data.resize(0x20); //Stored value is inaccurate
			if (!jpc.read(&tex.data[0], 0x20)) { //HD only includes texture name, so its always 0x20
				return JPCError::REACHED_EOF;
			}

			TEX1 texClass;
			chunks::TEX1_read(texClass, tex);
			textures.push_back(tex);

			if (textures_by_filename.find(texClass.filename) != textures_by_filename.end()) {
				return JPCError::DUPLICATE_FILENAME;
			}
			textures_by_filename[texClass.filename] = tex;

			offset += tex.size;
		}

		for (Particle& particle : particles) {
			for (uint16_t tex_id : particle.tdb1.texture_ids) {
				TEX1 texClass;
				chunks::TEX1_read(texClass, textures[tex_id]);
				particle.tdb1.texture_filenames.push_back(texClass.filename);
			}
		}

		return JPCError::NONE;
	}

	JPCError JPC::loadFromFile(const std::string& filePath) {
		std::ifstream file(filePath, std::ios::binary);
		if (!file.is_open()) {
			return JPCError::COULD_NOT_OPEN;
		}
		return loadFromBinary(file);
	}

	JPCError JPC::addParticle(const Particle& particle) {
		if (particles_by_id.find(particle.particle_id) != particles_by_id.end()) {
			return JPCError::DUPLICATE_PARTICLE_ID;
		}
		particles.push_back(particle);
		particles_by_id[particle.particle_id] = particle;
		return JPCError::NONE;
	}

	JPCError JPC::replaceParticle(const Particle& newParticle) {
		if (particles_by_id.find(newParticle.particle_id) == particles_by_id.end()) {
			return JPCError::MISSING_PARTICLE;
		}
		Particle old_particle = particles_by_id[newParticle.particle_id];
		auto it = std::find_if(particles.begin(), particles.end(), [old_particle](const Particle& particle) {return particle.data == old_particle.data; }); //get iterator to old_particle in the vector
		if (it == particles.end()) {
			return JPCError::MISSING_PARTICLE;
		}
		int particle_index = it - particles.begin(); //Get old_particle index in particles vector
		particles[particle_index] = newParticle;
		particles_by_id[newParticle.particle_id] = newParticle;
		return JPCError::NONE;
	}

	JPCError JPC::addTexture(TEX1& tex) {
		if (textures_by_filename.find(tex.filename) != textures_by_filename.end()) {
			return JPCError::DUPLICATE_FILENAME;
		}
		chunk tempChunk;
		chunks::TEX1_save(tex, tempChunk);
		textures.push_back(tempChunk);
		textures_by_filename[tex.filename] = tempChunk;
		return JPCError::NONE;
	}

	JPCError JPC::replaceTexture(TEX1& tex) {
		if (textures_by_filename.find(tex.filename) == textures_by_filename.end()) {
			return JPCError::MISSING_TEXTURE;
		}
		TEX1 old_tex;
		chunks::TEX1_read(old_tex, textures_by_filename[tex.filename]);
		auto it = std::find_if(textures.begin(), textures.end(), [this, tex](const chunk& chunk) {return chunk.data == textures_by_filename[tex.filename].data; });
		if (it == textures.end()) {
			return JPCError::MISSING_TEXTURE;
		}
		uint16_t tex_id = it - textures.begin();
		chunk tempChunk;
		chunks::TEX1_save(tex, tempChunk);
		textures[tex_id] = tempChunk;
		textures_by_filename[tex.filename] = tempChunk;
		return JPCError::NONE;
	}

	JPCError JPC::writeToStream(std::ostream& out) {
		num_particles = static_cast<uint16_t>(particles.size());
		num_textures = static_cast<uint16_t>(textures.size());
		out.write(magicJPAC, 8);

		Utility::byteswap_inplace(num_particles);
		Utility::byteswap_inplace(num_textures);
		out.write((char*)&num_particles, sizeof(num_particles));
		out.write((char*)&num_textures, sizeof(num_textures));

		out.seekp(0x20, std::ios::beg);

		for (Particle& particle : particles) {
			Utility::byteswap_inplace(num_textures); //Swap this back so we can use it

			particle.tdb1.texture_ids = {};
			for (const std::string& texture_filename : particle.tdb1.texture_filenames) {
				const chunk& texture = textures_by_filename[texture_filename];
				auto it = std::find_if(textures.begin(), textures.end(), [texture](const chunk& tex) {return tex.data == texture.data; });
				uint16_t texture_id = it - textures.begin();
				particle.tdb1.texture_ids.push_back(texture_id);
			}

			particle.save_changes();
			out.write(&particle.data[0], particle.data.length());
		}

		for (const chunk& texture : textures) {
			out.write(&texture.data[0], texture.data.size());
		}

		return JPCError::NONE;

	}

	JPCError JPC::writeToFile(const std::string& outFilePath) {
		std::ofstream outFile(outFilePath, std::ios::binary);
		if (!outFile.is_open()) {
			return JPCError::COULD_NOT_OPEN;
		}
		return writeToStream(outFile);
	}
}
