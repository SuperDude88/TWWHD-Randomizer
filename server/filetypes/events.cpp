#include "events.hpp"

std::optional<std::string> read_str_until_null_character(std::istream& fptr, int offset) { //only possible error is EventlistError::REACHED_EOF
	fptr.seekg(0, std::ios::end);
	int data_length = fptr.tellg();
	if (offset > data_length) {
		return std::nullopt;
	}

	int temp_offset = offset;
	int str_length = 0;
	while (offset < data_length) {
		fptr.seekg(temp_offset, std::ios::beg);
		char character;
		if(!fptr.read(&character, 1)) {
			return std::nullopt;
		}
		if (character == '\0') {
			break;
		}
		else {
			str_length = str_length + 1;
		}
		temp_offset = temp_offset + 1;
	}

	fptr.seekg(offset);
	std::string str;
	str.resize(str_length);
	if(!fptr.read(&str[0], str_length)) {
		return std::nullopt;
	}

	return str;
}

EventlistError Property::read(std::istream& file, int offset) {
	std::istream& fptr = file;
	this->offset = offset;

	fptr.seekg(this->offset, std::ios::beg);
	name.resize(0x20);
	if(!file.read(&name[0], 0x20)) {
		return EventlistError::REACHED_EOF;
	}
	if(!file.read((char*)&property_index, 4)) {
		return EventlistError::REACHED_EOF;
	}
	if(!file.read((char*)&data_type, 4)) {
		return EventlistError::REACHED_EOF;
	}
	if(!file.read((char*)&data_index, 4)) {
		return EventlistError::REACHED_EOF;
	}
	if(!file.read((char*)&data_size, 4)) {
		return EventlistError::REACHED_EOF;
	}
	if(!file.read((char*)&next_property_index, 4)) {
		return EventlistError::REACHED_EOF;
	}
	Utility::byteswap_inplace(property_index);
	Utility::byteswap_inplace(data_type);
	Utility::byteswap_inplace(data_index);
	Utility::byteswap_inplace(data_size);
	Utility::byteswap_inplace(next_property_index);

	if(!file.read(zero_initialized_runtime_data, 0xC)) {
		return EventlistError::REACHED_EOF;
	}
	return EventlistError::NONE;
}

void Property::save_changes(std::ostream& fptr) {
	fptr.seekp(offset, std::ios::beg);
	fptr.write(&name[0], 0x20);
	Utility::byteswap_inplace(property_index);
	Utility::byteswap_inplace(data_type);
	Utility::byteswap_inplace(data_index);
	Utility::byteswap_inplace(data_size);
	fptr.write((char*)&property_index, 4);
	fptr.write((char*)&data_type, 4);
	fptr.write((char*)&data_index, 4);
	fptr.write((char*)&data_size, 4);

	if (next_property == nullptr) {
		next_property_index = -1;
	}
	else {
		next_property_index = next_property->property_index;
	}
	Utility::byteswap_inplace(next_property_index);
	fptr.write((char*)&next_property_index, 4);
	fptr.write(zero_initialized_runtime_data, sizeof(zero_initialized_runtime_data));
	return;
}

EventlistError Action::read(std::istream& file, int offset) {
	std::istream& fptr = file;
	this->offset = offset;

	fptr.seekg(this->offset, std::ios::beg);
	name.resize(0x20);
	if(!file.read(&name[0], 0x20)) {
		return EventlistError::REACHED_EOF;
	}
	if(!file.read((char*)&duplicate_id, 4)) {
		return EventlistError::REACHED_EOF;
	}
	if(!file.read((char*)&action_index, 4)) {
		return EventlistError::REACHED_EOF;
	}
	Utility::byteswap_inplace(duplicate_id);
	Utility::byteswap_inplace(action_index);

	for (int i = 0; i < 3; i++) {
		int32_t flag_id;
		fptr.seekg(this->offset + 0x28 + i * 4, std::ios::beg);
		if(!file.read((char*)&flag_id, 4)) {
			return EventlistError::REACHED_EOF;
		}
		Utility::byteswap_inplace(flag_id);
		starting_flags[i] = flag_id;
	}

	fptr.seekg(this->offset + 0x34, std::ios::beg);
	if(!fptr.read((char*)&flag_id_to_set, 4)) {
		return EventlistError::REACHED_EOF;
	}
	if(!fptr.read((char*)&first_property_index, 4)) {
		return EventlistError::REACHED_EOF;
	}
	if(!fptr.read((char*)&next_action_index, 4)) {
		return EventlistError::REACHED_EOF;
	}
	Utility::byteswap_inplace(flag_id_to_set);
	Utility::byteswap_inplace(first_property_index);
	Utility::byteswap_inplace(next_action_index);

	if(!fptr.read(zero_initialized_runtime_data, 0x10)) {
		return EventlistError::REACHED_EOF;
	}
	return EventlistError::NONE;
}

void Action::save_changes(std::ostream& fptr) {
	fptr.seekp(offset, std::ios::beg);
	fptr.write(&name[0], 0x20);
	Utility::byteswap_inplace(duplicate_id);
	Utility::byteswap_inplace(action_index);
	fptr.write((char*)&duplicate_id, 4);
	fptr.write((char*)&action_index, 4);

	for (int i = 0; i < 3; i++) {
		int flag_id = starting_flags[i];
		Utility::byteswap_inplace(flag_id);
		fptr.seekp(offset + 0x28 + i * 4, std::ios::beg);
		fptr.write((char*)&flag_id, 4);
	}

	fptr.seekp(offset + 0x34, std::ios::beg);
	Utility::byteswap_inplace(flag_id_to_set);
	fptr.write((char*)&flag_id_to_set, 4);
	if (properties.size() == 0) {
		first_property_index = -1;
	}
	else {
		first_property_index = properties[0].property_index;
	}
	Utility::byteswap_inplace(first_property_index);
	fptr.seekp(offset + 0x38, std::ios::beg);
	fptr.write((char*)&first_property_index, 4);

	if (next_action == nullptr) {
		next_action_index = -1;
	}
	else {
		next_action_index = next_action->action_index;
	}
	fptr.seekp(offset + 0x3C, std::ios::beg);
	Utility::byteswap_inplace(next_action_index);
	fptr.write((char*)&next_action_index, 4);
	fptr.write(zero_initialized_runtime_data, sizeof(zero_initialized_runtime_data));
	return;
}

std::optional<std::reference_wrapper<Property>> Action::get_prop(const std::string& prop_name) {
	for (unsigned int i = 0; i < properties.size(); i++) {
		if (properties[i].name == prop_name) {
			return properties[i];
		}
	}
	return std::nullopt;
}

Property& Action::add_property(const std::string& name) {
	Property prop;
	prop.name = name;
	properties.push_back(prop);
	return properties.back();
}

EventlistError Actor::read(std::istream& file, int offset) {
	std::istream& fptr = file;
	this->offset = offset;

	fptr.seekg(this->offset, std::ios::beg);
	name.resize(0x20);
	if(!file.read(&name[0], 0x20)) {
		return EventlistError::REACHED_EOF;
	}
	if(!file.read((char*)&staff_identifier, 4)) {
		return EventlistError::REACHED_EOF;
	}
	if(!file.read((char*)&actor_index, 4)) {
		return EventlistError::REACHED_EOF;
	}
	if(!file.read((char*)&flag_id_to_set, 4)) {
		return EventlistError::REACHED_EOF;
	}
	if(!file.read((char*)&staff_type, 4)) {
		return EventlistError::REACHED_EOF;
	}
	if(!file.read((char*)&initial_action_index, 4)) {
		return EventlistError::REACHED_EOF;
	}
	Utility::byteswap_inplace(staff_identifier);
	Utility::byteswap_inplace(actor_index);
	Utility::byteswap_inplace(flag_id_to_set);
	Utility::byteswap_inplace(staff_type);
	Utility::byteswap_inplace(initial_action_index);

	if(!file.read((char*)&zero_initialized_runtime_data, 0x1C)) {
		return EventlistError::REACHED_EOF;
	}
	return EventlistError::NONE;
}

EventlistError Actor::save_changes(std::ostream& fptr) {
	if (actions.size() == 0) {
		return EventlistError::CANNOT_SAVE_ACTOR_WITH_NO_ACTIONS;
	}

	fptr.seekp(offset, std::ios::beg);
	fptr.write(&name[0], 0x20);
	Utility::byteswap_inplace(staff_identifier);
	Utility::byteswap_inplace(actor_index);
	Utility::byteswap_inplace(flag_id_to_set);
	Utility::byteswap_inplace(staff_type);
	fptr.write((char*)&staff_identifier, 4);
	fptr.write((char*)&actor_index, 4);
	fptr.write((char*)&flag_id_to_set, 4);
	fptr.write((char*)&staff_type, 4);

	initial_action = actions[0];
	initial_action_index = initial_action.action_index;
	Utility::byteswap_inplace(initial_action_index);
	fptr.write((char*)&initial_action_index, 4);

	fptr.write(zero_initialized_runtime_data, sizeof(zero_initialized_runtime_data));
	return EventlistError::NONE;
}

std::optional<std::reference_wrapper<Action>> Actor::add_action(const FileTypes::EventList* list, const std::string& name, const std::vector<Prop>& properties) { //only possible error is EventlistError::NO_UNUSED_FLAGS_TO_USE
	Action action;
	action.name = name;
	std::optional<int> flag_id_to_set = list->get_unused_flag_id();
	if(!flag_id_to_set.has_value()) {
		return std::nullopt;
	}
	action.flag_id_to_set = flag_id_to_set.value();
	for (const Prop& property : properties) {
		Property& prop = action.add_property(property.prop_name);
		prop.value = property.prop_value;
	}
	actions.push_back(action);
	return actions.back();
}

EventlistError Event::read(std::istream& file, int offset) {
	std::istream& fptr = file;
	this->offset = offset;

	fptr.seekg(offset, std::ios::beg);
	name.resize(0x20);
	if(!file.read(&name[0], 0x20)) {
		return EventlistError::REACHED_EOF;
	}
	if(!file.read((char*)&event_index, 4)) {
		return EventlistError::REACHED_EOF;
	}
	if(!file.read((char*)&unknown_1, 4)) {
		return EventlistError::REACHED_EOF;
	}
	if(!file.read((char*)&priority, 4)) {
		return EventlistError::REACHED_EOF;
	}
	Utility::byteswap_inplace(event_index);
	Utility::byteswap_inplace(unknown_1);
	Utility::byteswap_inplace(priority);

	for (int i = 0; i < 0x14; i++) {
		int32_t actor_index;
		fptr.seekg(this->offset + 0x2C + i * 4, std::ios::beg);
		if(!file.read((char*)&actor_index, 4)) {
			return EventlistError::REACHED_EOF;
		}
		Utility::byteswap_inplace(actor_index);
		actor_indexes[i] = actor_index;
	}
	fptr.seekg(this->offset + 0x7C, std::ios::beg);
	if(!file.read((char*)&num_actors, 4)) {
		return EventlistError::REACHED_EOF;
	}
	Utility::byteswap_inplace(num_actors);

	for (int i = 0; i < 2; i++) {
		int32_t flag_id;
		fptr.seekg(this->offset + 0x80 + i * 4, std::ios::beg);
		if(!file.read((char*)&flag_id, 4)) {
			return EventlistError::REACHED_EOF;
		}
		Utility::byteswap_inplace(flag_id);
		starting_flags[i] = flag_id;
	}

	for (int i = 0; i < 3; i++) {
		int32_t flag_id;
		fptr.seekg(this->offset + 0x88 + i * 4, std::ios::beg);
		if(!file.read((char*)&flag_id, 4)) {
			return EventlistError::REACHED_EOF;
		}
		Utility::byteswap_inplace(flag_id);
		ending_flags[i] = flag_id;
	}

	fptr.seekg(this->offset + 0x94, std::ios::beg);
	if(!file.read((char*)&play_jingle, 1)) {
		return EventlistError::REACHED_EOF;
	}

	fptr.seekg(this->offset + 0x95, std::ios::beg);
	if(!file.read(zero_initialized_runtime_data, 0x1B)) {
		return EventlistError::REACHED_EOF;
	}

	return EventlistError::NONE;
}

void Event::save_changes(std::ostream& fptr) {
	fptr.seekp(offset, std::ios::beg);
	fptr.write(&name[0], 0x20);
	Utility::byteswap_inplace(event_index);
	Utility::byteswap_inplace(unknown_1);
	Utility::byteswap_inplace(priority);
	fptr.write((char*)&event_index, 4);
	fptr.write((char*)&unknown_1, 4);
	fptr.write((char*)&priority, 4);

	for (unsigned int i = 0; i < 0x14; i++) {
		int32_t actor_index;
		if (i >= actors.size()) {
			actor_index = -1;
		}
		else {
			actor_index = actors[i].actor_index;
		}
		actor_indexes[i] = actor_index;
		fptr.seekp(offset + 0x2C + i * 4, std::ios::beg);
		Utility::byteswap_inplace(actor_index);
		fptr.write((char*)&actor_index, 4);
	}

	num_actors = actors.size();
	fptr.seekp(offset + 0x7C, std::ios::beg);
	Utility::byteswap_inplace(num_actors);
	fptr.write((char*)&num_actors, 4);

	for (int i = 0; i < 2; i++) {
		int flag_id = starting_flags[i];
		fptr.seekp(offset + 0x80 + i * 4, std::ios::beg);
		Utility::byteswap_inplace(flag_id);
		fptr.write((char*)&flag_id, 4);
	}

	for (int i = 0; i < 3; i++) {
		int flag_id = ending_flags[i];
		fptr.seekp(offset + 0x88 + i * 4, std::ios::beg);
		Utility::byteswap_inplace(flag_id);
		fptr.write((char*)&flag_id, 4);
	}

	fptr.seekp(offset + 0x94, std::ios::beg);
	fptr.write((char*)&play_jingle, 1);
	fptr.write(zero_initialized_runtime_data, sizeof(zero_initialized_runtime_data));
	return;
}

std::optional<std::reference_wrapper<Actor>> Event::get_actor(const std::string& name) {
	for (Actor& actor : actors) {
		if (actor.name == name) {
			return actor;
		}
	}
	return std::nullopt;
}

std::optional<std::reference_wrapper<Actor>> Event::add_actor(const FileTypes::EventList* list, const std::string& name) { //only possible error is EventlistError::NO_UNUSED_FLAGS_TO_USE
	Actor actor; 
	actor.name = name;
	std::optional<int> flag_id_to_set = list->get_unused_flag_id();
	if (!flag_id_to_set.has_value()) {
		return std::nullopt; 
	}
	actor.flag_id_to_set = flag_id_to_set.value();
	actors.push_back(actor);
	return actors.back();
}

namespace FileTypes
{

	const char* EventlistErrorGetName(EventlistError err) {
		switch (err) {
			case EventlistError::NONE:
                return "NONE";
			case EventlistError::COULD_NOT_OPEN:
                return "COULD_NOT_OPEN";
			case EventlistError::UNEXPECTED_EVENT_OFFSET:
                return "UNEXPECTED_EVENT_OFFSET";
			case EventlistError::NONZERO_PADDING_BYTE:
                return "NONZERO_PADDING_BYTE";
			case EventlistError::REACHED_EOF:
                return "REACHED_EOF";
			case EventlistError::DUPLICATE_EVENT_NAME:
                return "DUPLICATE_EVENT_NAME";
			case EventlistError::NON_BLANK_ACTOR_FOLLOWING_BLANK:
                return "NON_BLANK_ACTOR_FOLLOWING_BLANK";
			case EventlistError::UNKNOWN_PROPERTY_DATA_TYPE:
                return "UNKNOWN_PROPERTY_DATA_TYPE";
			case EventlistError::CANT_READ_DATA_TYPE:
                return "CANT_READ_DATA_TYPE";
			case EventlistError::NO_UNUSED_FLAGS_TO_USE:
                return "NO_UNUSED_FLAGS_TO_USE";
			case EventlistError::CANNOT_SAVE_ACTOR_WITH_NO_ACTIONS:
                return "CANNOT_SAVE_ACTOR_WITH_NO_ACTIONS";
			case EventlistError::COUNT:
				return "COUNT";
			default:
				return "UNKNOWN";
		}
	}

	EventList::EventList() {
	
	}

	void EventList::initNew() {
		event_list_offset = 0x40;
		num_events = 0;
		actor_list_offset = 0;
		num_actors = 0;
		action_list_offset = 0;
		num_actions = 0;
		property_list_offset = 0;
		num_properties = 0;
		float_list_offset = 0;
		num_floats = 0;
		integer_list_offset = 0;
		num_integers = 0;
		string_list_offset = 0;
		string_list_total_size = 0;
		memset(padding, '\0', 8);

		Events = {};
		Events_By_Name = {};

		All_Actors = {};
		All_Actions = {};
		All_Properties = {};
		All_Floats = {};
		All_Integers = {};
		All_Strings_By_Offset = {};

		unused_flag_ids = {};
	}

	EventList EventList::createNew(const std::string& filename) {
		EventList newEventList{};
		newEventList.initNew();
		return newEventList;
	}

	EventlistError EventList::loadFromFile(const std::string& filePath) {
	
		std::ifstream file(filePath, std::ios::binary);
		if (!file.is_open()) {
			return EventlistError::COULD_NOT_OPEN;
		}
		return loadFromBinary(file);
	}

	EventlistError EventList::loadFromBinary(std::istream& file_entry) {

		EventlistError err = EventlistError::NONE;

		if(!file_entry.read((char*)&event_list_offset, 4)) {
			return EventlistError::REACHED_EOF;
		}
		if(!file_entry.read((char*)&num_events, 4)) {
			return EventlistError::REACHED_EOF;
		}
		if(!file_entry.read((char*)&actor_list_offset, 4)) {
			return EventlistError::REACHED_EOF;
		}
		if(!file_entry.read((char*)&num_actors, 4)) {
			return EventlistError::REACHED_EOF;
		}
		if(!file_entry.read((char*)&action_list_offset, 4)) {
			return EventlistError::REACHED_EOF;
		}
		if(!file_entry.read((char*)&num_actions, 4)) {
			return EventlistError::REACHED_EOF;
		}
		if(!file_entry.read((char*)&property_list_offset, 4)) {
			return EventlistError::REACHED_EOF;
		}
		if(!file_entry.read((char*)&num_properties, 4)) {
			return EventlistError::REACHED_EOF;
		}
		if(!file_entry.read((char*)&float_list_offset, 4)) {
			return EventlistError::REACHED_EOF;
		}
		if(!file_entry.read((char*)&num_floats, 4)) {
			return EventlistError::REACHED_EOF;
		}
		if(!file_entry.read((char*)&integer_list_offset, 4)) {
			return EventlistError::REACHED_EOF;
		}
		if(!file_entry.read((char*)&num_integers, 4)) {
			return EventlistError::REACHED_EOF;
		}
		if(!file_entry.read((char*)&string_list_offset, 4)) {
			return EventlistError::REACHED_EOF;
		}
		if(!file_entry.read((char*)&string_list_total_size, 4)) {
			return EventlistError::REACHED_EOF;
		}
		Utility::byteswap_inplace(event_list_offset);
		Utility::byteswap_inplace(num_events);
		Utility::byteswap_inplace(actor_list_offset);
		Utility::byteswap_inplace(num_actors);
		Utility::byteswap_inplace(action_list_offset);
		Utility::byteswap_inplace(num_actions);
		Utility::byteswap_inplace(property_list_offset);
		Utility::byteswap_inplace(num_properties);
		Utility::byteswap_inplace(float_list_offset);
		Utility::byteswap_inplace(num_floats);
		Utility::byteswap_inplace(integer_list_offset);
		Utility::byteswap_inplace(num_integers);
		Utility::byteswap_inplace(string_list_offset);
		Utility::byteswap_inplace(string_list_total_size);
		if(!file_entry.read(padding, 8)) {
			return EventlistError::REACHED_EOF;
		}

		if (event_list_offset != 0x40) {
			return EventlistError::UNEXPECTED_EVENT_OFFSET;
		}

		Events.reserve(num_events); //Minimize copies
		for (unsigned int event_index = 0; event_index < num_events; event_index++) {
			int offset = event_list_offset + event_index * 0xB0;
			Event event;
			err = event.read(file_entry, offset);
			if(err != EventlistError::NONE) {
				return err;
			}
			Events.push_back(event);
			if (Events_By_Name.find(event.name) != Events_By_Name.end()) {
				return EventlistError::DUPLICATE_EVENT_NAME;
			}
			Events_By_Name[event.name] = event;
		}

		All_Actors.reserve(num_actors); //Minmize copies
		for (unsigned int actor_index = 0; actor_index < num_actors; actor_index++) {
			int offset = actor_list_offset + actor_index * 0x50;
			Actor actor;
			err = actor.read(file_entry, offset);
			if (err != EventlistError::NONE) {
				return err;
			}
			All_Actors.push_back(actor);
		}

		All_Actions.reserve(num_actions); //Minmize copies
		for (unsigned int action_index = 0; action_index < num_actions; action_index++) {
			int offset = action_list_offset + action_index * 0x50;
			Action action;
			err = action.read(file_entry, offset);
			if (err != EventlistError::NONE) {
				return err;
			}
			All_Actions.push_back(action);
		}

		All_Properties.reserve(num_properties); //Minmize copies
		for (unsigned int property_index = 0; property_index < num_properties; property_index++) { //Populate properties
			int offset = property_list_offset + property_index * 0x40;
			Property property;
			err = property.read(file_entry, offset);
			if (err != EventlistError::NONE) {
				return err;
			}
			All_Properties.push_back(property);
		}

		All_Floats.reserve(num_floats); //Minmize copies
		for (unsigned int float_index = 0; float_index < num_floats; float_index++) {
			int offset = float_list_offset + float_index * 4;
			float float_val;
			file_entry.seekg(offset, std::ios::beg);
			if(!file_entry.read((char*)&float_val, 4)) {
				return EventlistError::REACHED_EOF;
			}
			Utility::byteswap_inplace(float_val);
			All_Floats.push_back(float_val);
		}

		All_Integers.reserve(num_integers); //Minmize copies
		for (unsigned int integer_index = 0; integer_index < num_integers; integer_index++) {
			int offset = integer_list_offset + integer_index * 4;
			int32_t integer;
			file_entry.seekg(offset, std::ios::beg);
			if(!file_entry.read((char*)&integer, 4)) {
				return EventlistError::REACHED_EOF;
			}
			Utility::byteswap_inplace(integer);
			All_Integers.push_back(integer);
		}

		uint32_t offset = string_list_offset;
		while (offset < string_list_offset + string_list_total_size) {
			std::optional<std::string> string = read_str_until_null_character(file_entry, offset);
			if (!string.has_value()) {
				return EventlistError::REACHED_EOF; //only error that can happen in read_str
			}
			All_Strings_By_Offset[offset - string_list_offset] = string.value();
			int string_length_with_null = string.value().length() + 1;
			offset = offset + string_length_with_null;

			if (string_length_with_null % 8 != 0) {
				int padding_bytes_to_skip = 8 - (string_length_with_null % 8);

				for (int i = 0; i < padding_bytes_to_skip; i++) {
					char padding_byte;
					file_entry.seekg(offset + i, std::ios::beg);
					if(!file_entry.read(&padding_byte, 1)) {
						return EventlistError::REACHED_EOF;
					}
					if (padding_byte != '\0') {
						return EventlistError::NONZERO_PADDING_BYTE;
					}
				}

				offset = offset + padding_bytes_to_skip;

			}
		}

		for (Property& property : All_Properties) {
			if (property.data_type == 0) {
				std::vector<float> value(property.data_size); //Minimize copies
				for (unsigned int i = 0; i < property.data_size; i++) {
					value.push_back(All_Floats[property.data_index + i]);
				}
				property.value = value;
			}
			else if (property.data_type == 1) {
				std::vector<XYZ> value(property.data_size);
				for (unsigned int i = 0; i < property.data_size; i++) {
					XYZ temp;
					temp.x = All_Floats[property.data_index + i * 3];
					temp.y = All_Floats[property.data_index + i * 3 + 1];
					temp.z = All_Floats[property.data_index + i * 3 + 2];
					value.push_back(temp);
				}
				property.value = value;
			}
			else if (property.data_type == 3) {
				std::vector<int> value(property.data_size);
				for (unsigned int i = 0; i < property.data_size; i++) {
					value.push_back(All_Integers[property.data_index + i]);
				}
				property.value = value;
			}
			else if (property.data_type == 4) {
				property.value = All_Strings_By_Offset[property.data_index];
			}
			else {
				return EventlistError::CANT_READ_DATA_TYPE;
			}
		}

		for (Action& action : All_Actions) { //Populate properties for each action
			if (action.first_property_index == -1) {
				continue;
			}
			Property& first_property = All_Properties[action.first_property_index];
			action.properties.push_back(first_property);
			Property& property = first_property;
			while (property.next_property_index != -1) {
				Property& next_property = All_Properties[property.next_property_index];
				property.next_property = &next_property;
				action.properties.push_back(next_property);
				property = next_property;
			}
		}

		for (Actor& actor : All_Actors) { //Fill each actor's list of actions before we add them to the events because they don't update both instances if we do this after
			actor.initial_action = All_Actions[actor.initial_action_index];
			actor.actions.push_back(actor.initial_action);
			Action& action = actor.initial_action;
			while (action.next_action_index != -1) {
				Action& next_action = All_Actions[action.next_action_index];
				action.next_action = &next_action;
				actor.actions.push_back(next_action);
				action = next_action;
			}
		}

		for (Event& event : Events) {
			bool found_blank = false;
			for (const int actor_index : event.actor_indexes) {
				if (actor_index == -1) {
					found_blank = true;
				}
				else {
					if (found_blank) {
						return EventlistError::NON_BLANK_ACTOR_FOLLOWING_BLANK;
					}
					Actor& actor = All_Actors[actor_index];
					event.actors.push_back(actor);
				}

			}

		}

		unused_flag_ids.reserve(TOTAL_NUM_FLAGS);
		for (int i = 0; i < TOTAL_NUM_FLAGS; i++) { //Populate the list of all flags (SD randomizer uses range but this is c++ so we don't have a super good substitute)
			unused_flag_ids.push_back(i);
		}

		for (Event& event : Events) {
			for (Actor& actor : event.actors) {
				unused_flag_ids.erase(std::remove(unused_flag_ids.begin(), unused_flag_ids.end(), actor.flag_id_to_set), unused_flag_ids.end());
				for (Action& action : actor.actions) {
					unused_flag_ids.erase(std::remove(unused_flag_ids.begin(), unused_flag_ids.end(), action.flag_id_to_set), unused_flag_ids.end());
				}
			}
		}
		return EventlistError::NONE;
	}

	EventlistError EventList::writeToStream(std::ostream& file_entry) {
		Utility::byteswap_inplace(event_list_offset);
		Utility::byteswap_inplace(num_events);
		Utility::byteswap_inplace(actor_list_offset);
		Utility::byteswap_inplace(num_actors);
		Utility::byteswap_inplace(action_list_offset);
		Utility::byteswap_inplace(num_actions);
		Utility::byteswap_inplace(property_list_offset);
		Utility::byteswap_inplace(num_properties);
		Utility::byteswap_inplace(float_list_offset);
		Utility::byteswap_inplace(num_floats);
		Utility::byteswap_inplace(integer_list_offset);
		Utility::byteswap_inplace(num_integers);
		Utility::byteswap_inplace(string_list_offset);
		Utility::byteswap_inplace(string_list_total_size);
		file_entry.write((char*)&event_list_offset, 4);
		file_entry.write((char*)&num_events, 4);
		file_entry.write((char*)&actor_list_offset, 4);
		file_entry.write((char*)&num_actors, 4);
		file_entry.write((char*)&action_list_offset, 4);
		file_entry.write((char*)&num_actions, 4);
		file_entry.write((char*)&property_list_offset, 4);
		file_entry.write((char*)&num_properties, 4);
		file_entry.write((char*)&float_list_offset, 4);
		file_entry.write((char*)&num_floats, 4);
		file_entry.write((char*)&integer_list_offset, 4);
		file_entry.write((char*)&num_integers, 4);
		file_entry.write((char*)&string_list_offset, 4);
		file_entry.write((char*)&string_list_total_size, 4);
		file_entry.write(padding, 8);

		std::vector<Event> All_Events = Events;
		All_Actors.clear();
		All_Actions.clear();
		All_Properties.clear();
		All_Floats.clear();
		All_Integers.clear();
		std::vector<std::string> All_Strings;

		int offset = 0x40;

		int event_list_offset = offset;
		int num_events = All_Events.size();
		for (int i = 0; i < num_events; i++) {
			Event& event = All_Events[i];
			event.offset = offset;
			event.event_index = i;

			offset = offset + event.DATA_SIZE;

			All_Actors.insert(All_Actors.end(), event.actors.begin(), event.actors.end());
		}

		int actor_list_offset = offset;
		int num_actors = All_Actors.size();
		for (int i = 0; i < num_actors; i++) {
			Actor& actor = All_Actors[i];
			actor.offset = offset;
			actor.actor_index = i;

			offset = offset + actor.DATA_SIZE;

			All_Actions.reserve(actor.actions.size()); //Minimize copies
			for (unsigned int x = 0; x < actor.actions.size(); x++) {
				Action& action = actor.actions[x];
				if (x == actor.actions.size() - 1) {
					action.next_action = nullptr;
				}
				else {
					action.next_action = &actor.actions[x + 1];
				}
				All_Actions.push_back(action);
			}

		}

		int action_list_offset = offset;
		int num_actions = All_Actions.size();
		for (int i = 0; i < num_actions; i++) {
			Action& action = All_Actions[i];
			action.offset = offset;
			action.action_index = i;

			offset = offset + action.DATA_SIZE;

			All_Properties.reserve(action.properties.size()); //Minimize copies
			for (unsigned int x = 0; x < action.properties.size(); x++) {
				Property& property = action.properties[x];
				All_Properties.push_back(property);
				if (x == action.properties.size() - 1) {
					property.next_property = nullptr;
				}
				else {
					property.next_property = &action.properties[x + 1];
				}
				All_Properties[property.property_index] = property;
			}
		}

		int property_list_offset = offset;
		int num_properties = All_Properties.size();
		for (int i = 0; i < num_properties; i++) {
			Property& property = All_Properties[i];
			property.offset = offset;
			property.property_index = i;

			offset = offset + property.DATA_SIZE;

			std::variant<std::vector<float>, std::vector<XYZ>, std::vector<int>, std::string> property_value = property.value;
			if (property_value.index() != 3) {
				if (property_value.index() == 0) {
					property.data_size = std::get<0>(property_value).size();
					property.data_type = 0;
					property.data_index = All_Floats.size();

					for (const float& float_val : std::get<0>(property_value)) {
						All_Floats.push_back(float_val);
					}
				}
				else if (property_value.index() == 1) {
					property.data_size = std::get<1>(property_value).size();
					property.data_type = 1;
					property.data_index = All_Floats.size();

					for (const XYZ& vector3 : std::get<1>(property_value)) {
						All_Floats.push_back(vector3.x);
						All_Floats.push_back(vector3.y);
						All_Floats.push_back(vector3.z);
					}
				}
				else if (property_value.index() == 2) {
					property.data_size = std::get<2>(property_value).size();
					property.data_type = 3;
					property.data_index = All_Integers.size();

					for (const int& integer : std::get<2>(property_value)) {
						All_Integers.push_back(integer);
					}
				}
				else {
					return EventlistError::UNKNOWN_PROPERTY_DATA_TYPE;
				}
			}
			else if (property_value.index() == 3) {
				property.data_type = 4;
				property.data_index = 0; //gets set later
				property.data_size = 0; //gets set later

				All_Strings.push_back(std::get<3>(property_value));
			}

		}

		float_list_offset = offset;
		num_floats = All_Floats.size();
		for (float& float_val : All_Floats) {
			file_entry.seekp(offset, std::ios::beg);
			uint32_t value = reinterpret_cast<const uint32_t&>(float_val); //byteswap as a uint32 because casting it back to float in reversed order treats it as NaN and changes the value
			Utility::byteswap_inplace(value);
			file_entry.write((char*)&value, 4);
			offset = offset + 4;
		}

		integer_list_offset = offset;
		num_integers = All_Integers.size();
		for (int& integer : All_Integers) {
			file_entry.seekp(offset, std::ios::beg);
			Utility::byteswap_inplace(integer);
			file_entry.write((char*)&integer, 4);
			offset = offset + 4;
		}

		string_list_offset = offset;
		for (Property& property : All_Properties) {
			if (property.data_type == 4) {
				std::string string = std::get<std::string>(property.value);
				file_entry.write(&string[0], string.length()); //Write string
				file_entry.write("\x00", 1); //Write null byte
				uint32_t new_relative_string_offset = offset - string_list_offset;

				uint32_t string_length_with_null = string.length() + 1;
				offset = offset + string_length_with_null;

				uint32_t string_length_with_padding = string_length_with_null;
				if (string_length_with_null % 8 != 0) {
					int padding_bytes_needed = (8 - (string_length_with_null % 8));
					std::string padding;
					padding.resize(padding_bytes_needed, '\0');
					file_entry.write(&padding[0], padding_bytes_needed);
					offset = offset + padding_bytes_needed;
					string_length_with_padding = string_length_with_null + padding_bytes_needed;
				}

				property.data_index = new_relative_string_offset;
				property.data_size = string_length_with_padding;

			}
		}

		string_list_total_size = offset - string_list_offset;

		for (Event& event : All_Events) {
			event.save_changes(file_entry);
		}
		for (Actor& actor : All_Actors) {
			EventlistError err = actor.save_changes(file_entry);
			if(err != EventlistError::NONE) {
				return err;
			}
		}
		for (Action& action : All_Actions) {
			action.save_changes(file_entry);
		}
		for (Property& property : All_Properties) {
			property.save_changes(file_entry);
		}

		file_entry.seekp(0, std::ios::beg);
		Utility::byteswap_inplace(event_list_offset);
		Utility::byteswap_inplace(num_events);
		Utility::byteswap_inplace(actor_list_offset);
		Utility::byteswap_inplace(num_actors);
		Utility::byteswap_inplace(action_list_offset);
		Utility::byteswap_inplace(num_actions);
		Utility::byteswap_inplace(property_list_offset);
		Utility::byteswap_inplace(num_properties);
		Utility::byteswap_inplace(float_list_offset);
		Utility::byteswap_inplace(num_floats);
		Utility::byteswap_inplace(integer_list_offset);
		Utility::byteswap_inplace(num_integers);
		Utility::byteswap_inplace(string_list_offset);
		Utility::byteswap_inplace(string_list_total_size);

		file_entry.write((char*)&event_list_offset, 4);
		file_entry.write((char*)&num_events, 4);
		file_entry.write((char*)&actor_list_offset, 4);
		file_entry.write((char*)&num_actors, 4);
		file_entry.write((char*)&action_list_offset, 4);
		file_entry.write((char*)&num_actions, 4);
		file_entry.write((char*)&property_list_offset, 4);
		file_entry.write((char*)&num_properties, 4);
		file_entry.write((char*)&float_list_offset, 4);
		file_entry.write((char*)&num_floats, 4);
		file_entry.write((char*)&integer_list_offset, 4);
		file_entry.write((char*)&num_integers, 4);
		file_entry.write((char*)&string_list_offset, 4);
		file_entry.write((char*)&string_list_total_size, 4);
		file_entry.write(padding, sizeof(padding));
		return EventlistError::NONE;
	}

	EventlistError EventList::writeToFile(const std::string& outFilePath) {
		std::ofstream outFile(outFilePath, std::ios::binary);
		if(!outFile.is_open()) {
			return EventlistError::COULD_NOT_OPEN;
		}
		return writeToStream(outFile);
	}

	Event& EventList::add_event(const std::string& name) {
		Event event;
		event.name = name;
		Events.push_back(event);
		Events_By_Name[name] = event;
		return Events_By_Name[name];
	}

	std::optional<int> EventList::get_unused_flag_id() const { //only possible error is EventlistError::NO_UNUSED_FLAGS_TO_USE
		if (unused_flag_ids.size() == 0) {
			return std::nullopt;
		}
		int flag_id = unused_flag_ids[0];
		unused_flag_ids.erase(unused_flag_ids.begin(), unused_flag_ids.begin() + 1);
		return flag_id;
	}
}
