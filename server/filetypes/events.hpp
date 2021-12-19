#pragma once
#include <fstream>
#include <cstring>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <string>
#include <array>
#include <variant>
#include <optional>
#include <functional>
#include "../utility/byteswap.hpp"

enum struct EventlistError
{
	NONE = 0,
	COULD_NOT_OPEN,
	UNEXPECTED_EVENT_OFFSET,
	NONZERO_PADDING_BYTE,
	REACHED_EOF,
	DUPLICATE_EVENT_NAME,
	NON_BLANK_ACTOR_FOLLOWING_BLANK,
	UNKNOWN_PROPERTY_DATA_TYPE,
	CANT_READ_DATA_TYPE,
	NO_UNUSED_FLAGS_TO_USE,
	CANNOT_SAVE_ACTOR_WITH_NO_ACTIONS,
	UNKNOWN,
	COUNT
};

struct XYZ {
	float x, y, z;
};

struct Prop {
	std::string prop_name;
	std::variant<std::vector<float>, std::vector<XYZ>, std::vector<int>, std::string> prop_value;
};

namespace FileTypes {
	class EventList;
};

class Property {
public:
	static constexpr int DATA_SIZE = 0x40; //could be a define?

	std::string name;
	int32_t property_index;
	uint32_t data_type;
	uint32_t data_index;
	uint32_t data_size;
	int32_t next_property_index;
	char zero_initialized_runtime_data[0xC] = { "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" };
	Property* next_property; //Pointer because of initialization stuff
	std::variant<std::vector<float>, std::vector<XYZ>, std::vector<int>, std::string> value;
	int offset;

	EventlistError read(std::istream& file, int offset);
	void save_changes(std::ostream& fptr);
};

class Action {
public:
	static constexpr int DATA_SIZE = 0x50; //could be a define?

	std::string name;
	uint32_t duplicate_id = 0;
	int32_t action_index;
	std::array<int32_t, 3> starting_flags = { -1, -1, -1 };
	int32_t flag_id_to_set;
	int32_t first_property_index;
	int32_t next_action_index;
	char zero_initialized_runtime_data[0x10] = { "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" };
	std::vector<Property> properties;
	Action* next_action; //Pointer because of initialization stuff
	int offset;

	EventlistError read(std::istream& file, int offset);
	void save_changes(std::ostream& fptr);
	std::optional<std::reference_wrapper<Property>> get_prop(std::string prop_name);
	Property& add_property(std::string name);
};

class Actor {
public:
	static constexpr int DATA_SIZE = 0x50; //could be a define?

	std::string name;
	uint32_t staff_identifier = 0;
	int32_t actor_index;
	int32_t flag_id_to_set;
	uint32_t staff_type = 0;
	int32_t initial_action_index;
	char zero_initialized_runtime_data[0x1C] = { "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" };
	std::vector<Action> actions;
	Action initial_action;
	int offset;

	EventlistError read(std::istream& file, int offset);
	EventlistError save_changes(std::ostream& fptr);
	std::optional<std::reference_wrapper<Action>> add_action(FileTypes::EventList* list, std::string name, std::vector<Prop> properties);
};

class Event {
public:
	static constexpr int DATA_SIZE = 0xB0; //could be a define?

	std::string name;
	int32_t event_index;
	uint32_t unknown_1 = 0;
	uint32_t priority = 0;
	std::array<int32_t, 0x14> actor_indexes = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	uint32_t num_actors = 0;
	std::array<int32_t, 2> starting_flags = { -1, -1 };
	std::array<int32_t, 3> ending_flags = { -1, -1, -1 };
	bool play_jingle = false;
	char zero_initialized_runtime_data[0x1B] = { "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" };
	std::vector<Actor> actors;
	int offset;

	EventlistError read(std::istream& file, int offset);
	void save_changes(std::ostream& fptr);
	std::optional<std::reference_wrapper<Actor>> get_actor(std::string name);
	std::optional<std::reference_wrapper<Actor>> add_actor(FileTypes::EventList* list, std::string name);
};

namespace FileTypes
{

	const char* EventlistErrorGetName(EventlistError err);

	class EventList {
	public:
		static constexpr int32_t TOTAL_NUM_FLAGS = 0x2800; //could be a define?

		uint32_t event_list_offset;
		uint32_t num_events;
		uint32_t actor_list_offset;
		uint32_t num_actors;
		uint32_t action_list_offset;
		uint32_t num_actions;
		uint32_t property_list_offset;
		uint32_t num_properties;
		uint32_t float_list_offset;
		uint32_t num_floats;
		uint32_t integer_list_offset;
		uint32_t num_integers;
		uint32_t string_list_offset;
		uint32_t string_list_total_size;
		char padding[8];

		std::vector<Event> Events;
		std::unordered_map<std::string, Event> Events_By_Name;

		std::vector<Actor> All_Actors;
		std::vector<Action> All_Actions;
		std::vector<Property> All_Properties;
		std::vector<float> All_Floats;
		std::vector<int> All_Integers;
		std::unordered_map<int, std::string> All_Strings_By_Offset;

		std::vector<uint32_t> unused_flag_ids;

		EventList();
		static EventList createNew(const std::string& filename);
		EventlistError loadFromBinary(std::istream& file_entry);
		EventlistError loadFromFile(const std::string& filePath);
		EventlistError writeToStream(std::ostream& file_entry);
		EventlistError writeToFile(const std::string& outFilePath);
		Event& add_event(std::string name);
		std::optional<int> get_unused_flag_id();
	private:
		void initNew();
	};
}
