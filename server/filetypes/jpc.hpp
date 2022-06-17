#pragma once

#include <vector>
#include <optional>
#include <unordered_map>
#include <fstream>

#include "../utility/common.hpp"



enum struct [[nodiscard]] JPCError {
	NONE = 0,
	REACHED_EOF,
	COULD_NOT_OPEN,
	NOT_JPC,
	UNKNOWN_CHUNK,
	DUPLICATE_CHUNK,
	MISSING_CHUNK,
	UNEXPECTED_VALUE,
	DUPLICATE_PARTICLE_ID,
	DUPLICATE_FILENAME,
	MISSING_PARTICLE,
	MISSING_TEXTURE,
	UNKNOWN,
	COUNT
};

namespace JParticle {
	enum struct VolumeType : uint8_t {
		CUBE = 0,
		SPHERE,
		CYLINDER,
		TORUS,
		POINT,
		CIRCLE,
		LINE
	};

	enum struct EmitFlags : uint8_t {
		FIXED_DENSITY = 0x1,
		FIXED_INTERVAL,
		INHERIT_SCALE = 0x4,
		FOLLOW_EMITTER = 0x8,
		FOLLOW_EMITTER_CHILD = 0x10
	};

	enum struct ShapeType : uint8_t {
		POINT = 0,
		LINE,
		BILLBOARD,
		DIRECTION,
		DIRECTION_CROSS,
		STRIPE,
		STRIPE_CROSS,
		ROTATION,
		ROTATION_CROSS,
		DIR_BILLBOARD,
		Y_BILLBOARD
	};

	enum struct DirType : uint8_t {
		VEL = 0,
		POS,
		POS_INV,
		EMTR_DIT,
		PREV_PCTL
	};

	enum struct RotType : uint8_t {
		X = 0,
		Y,
		Z,
		XYZ,
		Y_JIGGLE
	};

	enum struct PlaneType : uint8_t {
		XY = 0,
		XZ,
		X
	};

	enum struct CalcIdxType : uint8_t {
		NORMAL = 0,
		REPEAT,
		REVERSE,
		MERGE,
		RANDOM
	};

	enum struct CalcScaleAnmType : uint8_t {
		NORMAL = 0,
		REPEAT,
		REVERSE
	};

	enum struct CalcAlphaWaveType : int8_t {
		NONE = -1,
		NRM_SIN = 0,
		ADD_SIN,
		MULT_SIN
	};

	enum struct IndTextureMode : uint8_t {
		OFF = 0,
		NORMAL,
		SUB
	};

	enum struct FieldType : uint8_t {
		GRAVITY = 0,
		AIR,
		MAGNET,
		NEWTON,
		VORTEX,
		RANDOM,
		DRAG,
		CONVECTION,
		SPIN
	};

	enum struct FieldAddType : uint8_t {
		FIELD_ACCEL = 0,
		BASE_VELOCITY,
		FIELD_VELOCITY
	};

	enum struct FieldStatusFlag : uint8_t {
		NO_INHERIT_ROTATE = 0x2,
		AIR_DRAG = 0x4,

		FADE_USE_EN_TIME = 0x8,
		FADE_USE_DIS_TIME = 0x10,
		FADE_USE_FADE_IN = 0x20,
		FADE_USE_FADE_OUT = 0x40,
		FADE_FLAG_MASK = (FADE_USE_EN_TIME | FADE_USE_DIS_TIME | FADE_USE_FADE_IN | FADE_USE_FADE_OUT),

		USE_MAX_DIST = 0x80
	};

	enum struct KeyType : uint8_t {
		RATE = 0,
		VOLUME_SIZE,
		VOLUME_SWEEP,
		VOLUME_MIN_RAD,
		LIFETIME,
		MOMENT,
		INITIAL_VEL_OMNI,
		INIVIAL_VEL_AXIS,
		INIVIAL_VEL_DIR,
		SPREAD,
		SCALE
	};

	struct ColorAnimationKeyframe {
		uint16_t time;
		RGBA color;
	};

	struct CurveKeyframe {
		float time;
		float value;
		float tanIn;
		float tanOut;
	};

	class BEM1 {
	private:
		unsigned int offset;

		char magic[4];
		uint32_t sectionSize;
		uint32_t flags;
	public:
		JParticle::EmitFlags emitFlags;
		JParticle::VolumeType volumeType;
		float volumeSweep;
		float volumeMinRad;
		int16_t volumeSize;
		int16_t divNumber;
		float rate;
		float rateRndm;
		uint8_t rateStep;

		int16_t maxFrame;
		int16_t startFrame;
		int16_t lifeTime;
		float lifeTimeRndm;

		float initialVelOmni;
		float initialVelAxis;
		float initialVelRndm;
		float initialVelDir;
		float initialVelRatio;

		float spread;
		float airResist;
		float airResistRndm;

		float moment;
		float momentRndm;
		float accel;
		float accelRndm;

		vec3<float> emitterScale;
		vec3<float> emitterTrans;
		vec3<float> emitterDir;
		vec3<int16_t> emitterRot;



		JPCError read(std::istream& jpc, const unsigned int offset);
		JPCError save_changes(std::ostream& out);
	};

	class BSP1 {
	private:
		unsigned int offset;

		char magic[4];
		uint32_t sectionSize;
		uint32_t flags;
		uint8_t colorFlags;
		uint8_t texFlags;
	public:
		JParticle::ShapeType shapeType;
		JParticle::DirType dirType;
		JParticle::RotType rotType;
		JParticle::PlaneType planeType;
		vec2<float> baseSize;
		float tilingS;
		float tilingT;
		bool isDrawFwdAhead;
		bool isDrawPrntAhead;
		// stopDrawParent is in SSP1 in JPA1
		// isNoDrawChild does not exist in JPA1

		//TEV / PE Settings
		uint8_t colorInSelect;
		uint8_t alphaInSelect;
		uint16_t blendModeFlags;
		uint8_t alphaCompareFlags;
		uint8_t alphaRef0;
		uint8_t alphaRef1;
		uint8_t zModeFlags;
		int16_t anmRndm;

		//Texture palette animation
		bool isEnableTexture;
		bool isGlblTexAnm;
		JParticle::CalcIdxType texCalcIdxType;
		uint8_t texIdx;
		std::vector<uint8_t> texIdxAnimData;
		uint8_t texIdxLoopOfstMask;

		//Texture coordinate animation
		bool isEnableProjection;
		bool isEnableTexScrollAnm;
		vec2<float> texInitTrans;
		vec2<float> texInitScale;
		//float texInitRot		not a thing until JPA2?
		vec2<float> texIncTrans;
		vec2<float> texIncScale;
		float texIncRot;

		//Color animation settings
		bool isGlblClrAnm;
		JParticle::CalcIdxType colorCalcIdxType;
		RGBA colorPrm;
		RGBA colorEnv;
		std::vector<ColorAnimationKeyframe> colorPrmAnimData; //somethign iwth durations?
		std::vector<ColorAnimationKeyframe> colorEnvAnimData; //somethign iwth durations?
		uint16_t colorAnimMaxFrm;
		uint8_t colorLoopOfstMask;



		JPCError read(std::istream& jpc, const unsigned int offset);
		JPCError save_changes(std::ostream& out);
	};

	class ESP1 {
	private:
		unsigned int offset;

		char magic[4];
		uint32_t sectionSize;
		uint32_t flags;
	public:
		bool isEnableScale;
		bool isDiffXY;
		bool isEnableScaleAnmY;
		bool isEnableScaleAnmX;
		bool isEnableScaleBySpeedX;
		bool isEnableScaleBySpeedY;
		bool isEnableRotate;
		bool isEnableAlpha;
		bool isEnableSinWave;
		JParticle::CalcAlphaWaveType alphaWaveType;
		bool anmTypeX;
		bool anmTypeY;
		uint8_t pivotX;
		uint8_t pivotY;
		float scaleInTiming;
		float scaleOutTiming;
		float scaleInValueX;
		float scaleOutValueX;
		float scaleInValueY;
		float scaleOutValueY;
		float scaleOutRandom;
		vec2<uint16_t> scaleAnmMaxFrame;
		float alphaInTiming;
		float alphaOutTiming;
		float alphaInValue;
		float alphaBaseValue;
		float alphaOutValue;
		float alphaWaveParam1;
		float alphaWaveParam2;
		float alphaWaveParam3;
		float alphaWaveRandom;
		float rotateAngle;
		float rotateAngleRandom;
		float rotateSpeed;
		float rotateSpeedRandom;
		float rotateDirection;

		JPCError read(std::istream& jpc, const unsigned int offset);
		JPCError save_changes(std::ostream& out);
	};

	class ETX1 {
	private:
		unsigned int offset;

		char magic[4];
		uint32_t sectionSize;
		uint32_t flags;
	public:
		JParticle::IndTextureMode indTextureMode;
		float p00, p01, p02, p10, p11, p12, scale;
		//std::array<float, 8> indTextureMtx;
		uint8_t indTextureID;
		uint8_t subTextureID;
		uint8_t secondTextureIndex;
		bool useSecondTex;

		JPCError read(std::istream& jpc, const unsigned int offset);
		JPCError save_changes(std::ostream& out);
	};

	class SSP1 {
	private:
		unsigned int offset;

		char magic[4];
		uint32_t sectionSize;
		uint32_t flags;
	public:
		bool isInheritedScale;
		bool isInheritedRGB;
		bool isInheritedAlpha;
		bool isEnableAlphaOut;
		bool isEnableField;
		bool isEnableRotate;
		bool isEnableScaleOut;

		bool isDrawParent;

		JParticle::ShapeType shapeType;
		JParticle::DirType dirType;
		JParticle::RotType rotType;
		JParticle::PlaneType planeType;
		float posRndm;
		float baseVel;
		float baseVelRndm;
		float velInfRate;
		float gravity;
		float timing;
		uint16_t life;
		uint16_t rate;
		uint32_t step;
		vec2<float> globalScale2D;
		float rotateSpeed;
		float inheritScale;
		float inheritAlpha;
		float inheritRGB;
		RGBA colorPrm;
		RGBA colorEnv;
		uint8_t texIdx;

		JPCError read(std::istream& jpc, const unsigned int offset);
		JPCError save_changes(std::ostream& out);
	};

	class FLD1 {
	private:
		unsigned int offset;

		char magic[4];
		uint32_t sectionSize;
		uint32_t flags;
	public:
		JParticle::FieldStatusFlag statusFlag;
		JParticle::FieldType type;
		JParticle::FieldAddType addType;

		// Used by Gravity, Air, Magnet, Newton, Vortex, Random, Drag, Convection, Spin
		float mag;
		// Used by Drag
		float magRndm;

		float maxDist;
		vec3<float> pos;
		vec3<float> dir;
		float fadeIn;
		float fadeOut;
		float enTime;
		float disTime;
		uint8_t cycle;
		uint8_t unk1;

		float param1;
		float param2;
		float param3;
		// Used by Newton, Air and Convection
		//float refDistance;
		// Used by Vortex and Spin
		//float innerSpeed;
		// Used by Vortex
		//float outerSpeed;



		JPCError read(std::istream& jpc, const unsigned int offset);
		JPCError save_changes(std::ostream& out);
	};

	class KFA1 {
	private:
		unsigned int offset;

		char magic[4];
		uint32_t sectionSize;
	public:
		JParticle::KeyType keyType;
		std::vector<CurveKeyframe> keys;
		bool isLoopEnable;
		uint8_t unk1;

		JPCError read(std::istream& jpc, const unsigned int offset);
		JPCError save_changes(std::ostream& out);
	};

	class TDB1 {
	private:
		unsigned int offset;

		char magic[4];
		uint32_t sectionSize;
		std::vector<uint16_t> texIDs;
	public:
		std::vector<std::string> texFilenames;

		JPCError read(std::istream& jpc, const unsigned int offset, const uint8_t texCount);
		JPCError populateFilenames(const std::vector<std::string>& texList);
		JPCError save_changes(std::ostream& out, const std::unordered_map<std::string, size_t>& textures);
	};

}


class Particle {
public:
	char magicJEFF[8];
	uint32_t unknown_1;
	uint32_t num_chunks;
	uint32_t size; //sometimes inaccurate

	uint8_t num_kfa1_chunks;
	uint8_t num_fld1_chunks;
	uint8_t num_textures;
	uint8_t unknown_5;

	uint16_t particle_id;

	uint8_t unknown_6[6];

	std::optional<JParticle::BEM1> emitter = std::nullopt;
	std::optional<JParticle::BSP1> baseShape = std::nullopt;
	std::optional<JParticle::ESP1> extraShape = std::nullopt;
	std::optional<JParticle::ETX1> extraTex = std::nullopt;
	std::optional<JParticle::SSP1> childShape = std::nullopt;
	std::vector<JParticle::FLD1> fields = {};
	std::vector<JParticle::KFA1> curves = {};
	std::optional<JParticle::TDB1> texDatabase = std::nullopt;

	JPCError read(std::istream& jpc, unsigned int particle_offset);
	JPCError save_changes(std::ostream& out, const std::unordered_map<std::string, size_t>& textures);
};

namespace FileTypes {

	const char* JPCErrorGetName(JPCError err);

	class JPC {
	public:
		char magicJPAC[8];
		uint16_t num_particles;
		uint16_t num_textures;

		std::unordered_map<uint16_t, size_t> particle_index_by_id;
		std::vector<Particle> particles;

		std::unordered_map<std::string, size_t> textures; //store index to preserve original order

		JPC();
		static JPC createNew(const std::string& filename);
		JPCError loadFromBinary(std::istream& jpc);
		JPCError loadFromFile(const std::string& filePath);
		JPCError addParticle(const Particle& particle);
		JPCError replaceParticle(const Particle& particle);
		JPCError addTexture(const std::string& filename);
		JPCError writeToStream(std::ostream& out);
		JPCError writeToFile(const std::string& outFilePath);
	private:
		void initNew();
	};
}
