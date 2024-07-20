#include "events.hpp"

#include <cstring>
#include <algorithm>
#include <functional>
#include <memory>

#include <utility/endian.hpp>
#include <utility/file.hpp>
#include <command/Log.hpp>

using eType = Utility::Endian::Type;



EventlistError Property::read(std::istream& in) {
    name.resize(0x20);
    if(!in.read(&name[0], 0x20)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&property_index), 4)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&data_type), 4)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&data_index), 4)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&data_size), 4)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&next_property_index), 4)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    name = name.substr(0, name.find('\0')); //remove null characters so we can work with the string
    Utility::Endian::toPlatform_inplace(eType::Big, property_index);
    Utility::Endian::toPlatform_inplace(eType::Big, data_type);
    Utility::Endian::toPlatform_inplace(eType::Big, data_index);
    Utility::Endian::toPlatform_inplace(eType::Big, data_size);
    Utility::Endian::toPlatform_inplace(eType::Big, next_property_index);

    if(!in.read(zero_initialized_runtime_data, 0xC)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    return EventlistError::NONE;
}

void Property::save_changes(std::ostream& out) {
    name.resize(0x20, '\0'); //pad to length with null
    Utility::Endian::toPlatform_inplace(eType::Big, property_index);
    Utility::Endian::toPlatform_inplace(eType::Big, data_type);
    Utility::Endian::toPlatform_inplace(eType::Big, data_index);
    Utility::Endian::toPlatform_inplace(eType::Big, data_size);

    out.write(&name[0], 0x20);
    out.write(reinterpret_cast<const char*>(&property_index), 4);
    out.write(reinterpret_cast<const char*>(&data_type), 4);
    out.write(reinterpret_cast<const char*>(&data_index), 4);
    out.write(reinterpret_cast<const char*>(&data_size), 4);

    if (next_property == nullptr) {
        next_property_index = -1;
    }
    else {
        next_property_index = next_property->property_index;
    }
    Utility::Endian::toPlatform_inplace(eType::Big, next_property_index);
    out.write(reinterpret_cast<const char*>(&next_property_index), 4);
    out.write(zero_initialized_runtime_data, sizeof(zero_initialized_runtime_data));

}

EventlistError Action::read(std::istream& in) {
    name.resize(0x20);
    if(!in.read(&name[0], 0x20)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&duplicate_id), 4)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&action_index), 4)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    name = name.substr(0, name.find('\0')); //remove null characters so we can work with the string
    Utility::Endian::toPlatform_inplace(eType::Big, duplicate_id);
    Utility::Endian::toPlatform_inplace(eType::Big, action_index);

    for (unsigned int i = 0; i < 3; i++) {
        int32_t flag_id;
        if(!in.read(reinterpret_cast<char*>(&flag_id), 4)) {
            LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
        }
        Utility::Endian::toPlatform_inplace(eType::Big, flag_id);
        starting_flags[i] = flag_id;
    }

    if(!in.read(reinterpret_cast<char*>(&flag_id_to_set), 4)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&first_property_index), 4)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&next_action_index), 4)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    Utility::Endian::toPlatform_inplace(eType::Big, flag_id_to_set);
    Utility::Endian::toPlatform_inplace(eType::Big, first_property_index);
    Utility::Endian::toPlatform_inplace(eType::Big, next_action_index);

    if(!in.read(zero_initialized_runtime_data, 0x10)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    return EventlistError::NONE;
}

void Action::save_changes(std::ostream& out) {
    name.resize(0x20, '\0'); //pad to length with null
    Utility::Endian::toPlatform_inplace(eType::Big, duplicate_id);
    Utility::Endian::toPlatform_inplace(eType::Big, action_index);

    out.write(&name[0], 0x20);
    out.write(reinterpret_cast<const char*>(&duplicate_id), 4);
    out.write(reinterpret_cast<const char*>(&action_index), 4);

    for (unsigned int i = 0; i < 3; i++) {
        int32_t flag_id = starting_flags[i];
        Utility::Endian::toPlatform_inplace(eType::Big, flag_id);
        out.write(reinterpret_cast<const char*>(&flag_id), 4);
    }

    Utility::Endian::toPlatform_inplace(eType::Big, flag_id_to_set);
    out.write(reinterpret_cast<const char*>(&flag_id_to_set), 4);
    if (properties.empty()) {
        first_property_index = -1;
    }
    else {
        first_property_index = properties[0]->property_index;
    }
    Utility::Endian::toPlatform_inplace(eType::Big, first_property_index);
    out.write(reinterpret_cast<const char*>(&first_property_index), 4);

    if (next_action == nullptr) {
        next_action_index = -1;
    }
    else {
        next_action_index = next_action->action_index;
    }
    Utility::Endian::toPlatform_inplace(eType::Big, next_action_index);
    out.write(reinterpret_cast<const char*>(&next_action_index), 4);
    out.write(zero_initialized_runtime_data, sizeof(zero_initialized_runtime_data));
}

std::shared_ptr<Property> Action::get_prop(const std::string& prop_name) {
    for (std::shared_ptr<Property>& property : properties) {
        if (property->name == prop_name) {
            return property;
        }
    }
    return nullptr;
}

Property& Action::add_property(const std::string& name) {
    std::shared_ptr<Property> prop = properties.emplace_back(std::make_shared<Property>());
    prop->name = name;
    return *prop;
}

EventlistError Actor::read(std::istream& in) {
    name.resize(0x20);
    if(!in.read(&name[0], 0x20)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&staff_identifier), 4)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&actor_index), 4)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&flag_id_to_set), 4)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&staff_type), 4)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&initial_action_index), 4)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    name = name.substr(0, name.find('\0')); //remove null characters so we can work with the string
    Utility::Endian::toPlatform_inplace(eType::Big, staff_identifier);
    Utility::Endian::toPlatform_inplace(eType::Big, actor_index);
    Utility::Endian::toPlatform_inplace(eType::Big, flag_id_to_set);
    Utility::Endian::toPlatform_inplace(eType::Big, staff_type);
    Utility::Endian::toPlatform_inplace(eType::Big, initial_action_index);

    if(!in.read(reinterpret_cast<char*>(&zero_initialized_runtime_data), 0x1C)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    return EventlistError::NONE;
}

EventlistError Actor::save_changes(std::ostream& out) {
    if (actions.empty()) {
        LOG_ERR_AND_RETURN(EventlistError::CANNOT_SAVE_ACTOR_WITH_NO_ACTIONS);
    }
    
    name.resize(0x20, '\0'); //pad to length with null
    Utility::Endian::toPlatform_inplace(eType::Big, staff_identifier);
    Utility::Endian::toPlatform_inplace(eType::Big, actor_index);
    Utility::Endian::toPlatform_inplace(eType::Big, flag_id_to_set);
    Utility::Endian::toPlatform_inplace(eType::Big, staff_type);

    out.write(&name[0], 0x20);
    out.write(reinterpret_cast<const char*>(&staff_identifier), 4);
    out.write(reinterpret_cast<const char*>(&actor_index), 4);
    out.write(reinterpret_cast<const char*>(&flag_id_to_set), 4);
    out.write(reinterpret_cast<const char*>(&staff_type), 4);

    initial_action = actions[0];
    initial_action_index = initial_action->action_index;
    Utility::Endian::toPlatform_inplace(eType::Big, initial_action_index);
    out.write(reinterpret_cast<const char*>(&initial_action_index), 4);

    out.write(zero_initialized_runtime_data, sizeof(zero_initialized_runtime_data));
    return EventlistError::NONE;
}

std::shared_ptr<Action> Actor::add_action(const FileTypes::EventList& list, const std::string& name, const std::vector<Prop>& properties) { //only possible error is EventlistError::NO_UNUSED_FLAGS_TO_USE
    std::shared_ptr<Action> action = actions.emplace_back(std::make_shared<Action>());
    action->name = name;
    std::optional<int> flag_id_to_set = list.get_unused_flag_id();
    if(!flag_id_to_set.has_value()) {
        return nullptr;
    }
    action->flag_id_to_set = flag_id_to_set.value();
    for (const Prop& property : properties) {
        Property& prop = action->add_property(property.prop_name);
        prop.value = property.prop_value;
    }
    return action;
}

EventlistError Event::read(std::istream& in) {
    name.resize(0x20);
    if(!in.read(&name[0], 0x20)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&event_index), 4)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&unknown_1), 4)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    if(!in.read(reinterpret_cast<char*>(&priority), 4)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    name = name.substr(0, name.find('\0')); //remove null characters so we can work with the string
    Utility::Endian::toPlatform_inplace(eType::Big, event_index);
    Utility::Endian::toPlatform_inplace(eType::Big, unknown_1);
    Utility::Endian::toPlatform_inplace(eType::Big, priority);

    for (unsigned int i = 0; i < 0x14; i++) {
        int32_t actor_index;
        if(!in.read(reinterpret_cast<char*>(&actor_index), 4)) {
            LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
        }
        Utility::Endian::toPlatform_inplace(eType::Big, actor_index);
        actor_indexes[i] = actor_index;
    }

    if(!in.read(reinterpret_cast<char*>(&num_actors), 4)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }
    Utility::Endian::toPlatform_inplace(eType::Big, num_actors);

    for (unsigned int i = 0; i < 2; i++) {
        int32_t flag_id;
        if(!in.read(reinterpret_cast<char*>(&flag_id), 4)) {
            LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
        }
        Utility::Endian::toPlatform_inplace(eType::Big, flag_id);
        starting_flags[i] = flag_id;
    }

    for (unsigned int i = 0; i < 3; i++) {
        int32_t flag_id;
        if(!in.read(reinterpret_cast<char*>(&flag_id), 4)) {
            LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
        }
        Utility::Endian::toPlatform_inplace(eType::Big, flag_id);
        ending_flags[i] = flag_id;
    }

    if(!in.read(reinterpret_cast<char*>(&play_jingle), 1)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }

    if(!in.read(zero_initialized_runtime_data, 0x1B)) {
        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
    }

    return EventlistError::NONE;
}

void Event::save_changes(std::ostream& out) {
    name.resize(0x20, '\0'); //pad to length with null
    Utility::Endian::toPlatform_inplace(eType::Big, event_index);
    Utility::Endian::toPlatform_inplace(eType::Big, unknown_1);
    Utility::Endian::toPlatform_inplace(eType::Big, priority);
    
    out.write(&name[0], 0x20);
    out.write(reinterpret_cast<const char*>(&event_index), 4);
    out.write(reinterpret_cast<const char*>(&unknown_1), 4);
    out.write(reinterpret_cast<const char*>(&priority), 4);

    for (unsigned int i = 0; i < 0x14; i++) {
        int32_t actor_index;
        if (i >= actors.size()) {
            actor_index = -1;
        }
        else {
            actor_index = actors[i]->actor_index;
        }
        actor_indexes[i] = actor_index;
        Utility::Endian::toPlatform_inplace(eType::Big, actor_index);
        out.write(reinterpret_cast<const char*>(&actor_index), 4);
    }

    num_actors = actors.size();
    Utility::Endian::toPlatform_inplace(eType::Big, num_actors);
    out.write(reinterpret_cast<const char*>(&num_actors), 4);

    for (unsigned int i = 0; i < 2; i++) {
        int flag_id = starting_flags[i];
        Utility::Endian::toPlatform_inplace(eType::Big, flag_id);
        out.write(reinterpret_cast<const char*>(&flag_id), 4);
    }

    for (unsigned int i = 0; i < 3; i++) {
        int32_t flag_id = ending_flags[i];
        Utility::Endian::toPlatform_inplace(eType::Big, flag_id);
        out.write(reinterpret_cast<const char*>(&flag_id), 4);
    }

    out.write(reinterpret_cast<const char*>(&play_jingle), 1);
    out.write(zero_initialized_runtime_data, sizeof(zero_initialized_runtime_data));
}

std::shared_ptr<Actor> Event::get_actor(const std::string& name) {
    for (auto actor : actors) {
        if (actor->name == name) {
            return actor;
        }
    }
    return nullptr;
}

std::shared_ptr<Actor> Event::add_actor(const FileTypes::EventList& list, const std::string& name) { //only possible error is EventlistError::NO_UNUSED_FLAGS_TO_USE
    std::shared_ptr<Actor> actor = actors.emplace_back(std::make_shared<Actor>()); 
    actor->name = name;
    std::optional<int32_t> flag_id_to_set = list.get_unused_flag_id();
    if (!flag_id_to_set.has_value()) {
        return nullptr; 
    }
    actor->flag_id_to_set = flag_id_to_set.value();
    return actor;
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
            default:
                return "UNKNOWN";
        }
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

    EventList EventList::createNew() {
        EventList newEventList{};
        newEventList.initNew();
        return newEventList;
    }

    EventlistError EventList::loadFromFile(const fspath& filePath) {
    
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            LOG_ERR_AND_RETURN(EventlistError::COULD_NOT_OPEN);
        }
        return loadFromBinary(file);
    }

    EventlistError EventList::loadFromBinary(std::istream& in) {
        if(!in.read(reinterpret_cast<char*>(&event_list_offset), 4)) {
            LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&num_events), 4)) {
            LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&actor_list_offset), 4)) {
            LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&num_actors), 4)) {
            LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&action_list_offset), 4)) {
            LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&num_actions), 4)) {
            LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&property_list_offset), 4)) {
            LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&num_properties), 4)) {
            LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&float_list_offset), 4)) {
            LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&num_floats), 4)) {
            LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&integer_list_offset), 4)) {
            LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&num_integers), 4)) {
            LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&string_list_offset), 4)) {
            LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
        }
        if(!in.read(reinterpret_cast<char*>(&string_list_total_size), 4)) {
            LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
        }
        if(!in.read(padding, 8)) {
            LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
        }

        Utility::Endian::toPlatform_inplace(eType::Big, event_list_offset);
        Utility::Endian::toPlatform_inplace(eType::Big, num_events);
        Utility::Endian::toPlatform_inplace(eType::Big, actor_list_offset);
        Utility::Endian::toPlatform_inplace(eType::Big, num_actors);
        Utility::Endian::toPlatform_inplace(eType::Big, action_list_offset);
        Utility::Endian::toPlatform_inplace(eType::Big, num_actions);
        Utility::Endian::toPlatform_inplace(eType::Big, property_list_offset);
        Utility::Endian::toPlatform_inplace(eType::Big, num_properties);
        Utility::Endian::toPlatform_inplace(eType::Big, float_list_offset);
        Utility::Endian::toPlatform_inplace(eType::Big, num_floats);
        Utility::Endian::toPlatform_inplace(eType::Big, integer_list_offset);
        Utility::Endian::toPlatform_inplace(eType::Big, num_integers);
        Utility::Endian::toPlatform_inplace(eType::Big, string_list_offset);
        Utility::Endian::toPlatform_inplace(eType::Big, string_list_total_size);

        if (event_list_offset != 0x40) {
            LOG_ERR_AND_RETURN(EventlistError::UNEXPECTED_EVENT_OFFSET);
        }

        Events.reserve(num_events); //Minimize copies
        for (uint32_t event_index = 0; event_index < num_events; event_index++) {
            std::shared_ptr<Event> event = Events.emplace_back(std::make_shared<Event>());
            LOG_AND_RETURN_IF_ERR(event->read(in));
            if (Events_By_Name.contains(event->name)) {
                LOG_ERR_AND_RETURN(EventlistError::DUPLICATE_EVENT_NAME);
            }
            
            Events_By_Name[event->name] = event;
        }

        All_Actors.reserve(num_actors); //Minimize copies
        for (uint32_t actor_index = 0; actor_index < num_actors; actor_index++) {
            std::shared_ptr<Actor> actor = All_Actors.emplace_back(std::make_shared<Actor>());
            LOG_AND_RETURN_IF_ERR(actor->read(in));
        }

        All_Actions.reserve(num_actions); //Minimize copies
        for (uint32_t action_index = 0; action_index < num_actions; action_index++) {
            std::shared_ptr<Action> action = All_Actions.emplace_back(std::make_shared<Action>());
            LOG_AND_RETURN_IF_ERR(action->read(in));
        }

        All_Properties.reserve(num_properties); //Minimize copies
        for (uint32_t property_index = 0; property_index < num_properties; property_index++) { //Populate properties
            std::shared_ptr<Property> property = All_Properties.emplace_back(std::make_shared<Property>());
            LOG_AND_RETURN_IF_ERR(property->read(in));
        }

        All_Floats.reserve(num_floats); //Minimize copies
        for (uint32_t float_index = 0; float_index < num_floats; float_index++) {
            float float_val;
            if(!in.read(reinterpret_cast<char*>(&float_val), 4)) {
                LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
            }
            Utility::Endian::toPlatform_inplace(eType::Big, float_val);
            All_Floats.push_back(float_val);
        }

        All_Integers.reserve(num_integers); //Minimize copies
        for (uint32_t integer_index = 0; integer_index < num_integers; integer_index++) {
            int32_t integer;
            if(!in.read(reinterpret_cast<char*>(&integer), 4)) {
                LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
            }
            Utility::Endian::toPlatform_inplace(eType::Big, integer);
            All_Integers.push_back(integer);
        }

        uint32_t offset = string_list_offset;
        while (offset < string_list_offset + string_list_total_size) {
            std::string string = readNullTerminatedStr(in, offset);
            if (string.empty()) {
                LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF); //only error that can happen in read_str
            }
            All_Strings_By_Offset[offset - string_list_offset] = string;
            offset += string.length();

            if (string.length() % 8 != 0) {
                unsigned int padding_bytes_to_skip = 8 - (string.length() % 8);

                for (unsigned int i = 0; i < padding_bytes_to_skip; i++) {
                    char padding_byte;
                    if(!in.read(&padding_byte, 1)) {
                        LOG_ERR_AND_RETURN(EventlistError::REACHED_EOF);
                    }
                    if (padding_byte != '\0') {
                        LOG_ERR_AND_RETURN(EventlistError::NONZERO_PADDING_BYTE);
                    }
                }
                offset += padding_bytes_to_skip;
            }
        }

        for (const auto& property : All_Properties) {
            if (property->data_type == 0) {
                std::vector<float>& value = property->value.emplace<std::vector<float>>();
                value.reserve(property->data_size); //Minimize copies
                for (unsigned int i = 0; i < property->data_size; i++) {
                    value.push_back(All_Floats[property->data_index + i]);
                }
                property->value = value;
            }
            else if (property->data_type == 1) {
                std::vector<vec3<float>>& value = property->value.emplace<std::vector<vec3<float>>>();
                value.reserve(property->data_size);
                for (unsigned int i = 0; i < property->data_size; i++) {
                    vec3<float>& temp = value.emplace_back();
                    temp.X = All_Floats[property->data_index + i * 3];
                    temp.Y = All_Floats[property->data_index + i * 3 + 1];
                    temp.Z = All_Floats[property->data_index + i * 3 + 2];
                }
            }
            else if (property->data_type == 3) {
                std::vector<int32_t>& value = property->value.emplace<std::vector<int32_t>>();
                value.reserve(property->data_size);
                for (uint32_t i = 0; i < property->data_size; i++) {
                    value.push_back(All_Integers[property->data_index + i]);
                }
            }
            else if (property->data_type == 4) {
                property->value = All_Strings_By_Offset[property->data_index];
            }
            else {
                LOG_ERR_AND_RETURN(EventlistError::CANT_READ_DATA_TYPE);
            }
        }

        for (const auto& action : All_Actions) { //Populate properties for each action
            if (action->first_property_index == -1) {
                continue;
            }
            std::shared_ptr<Property> first_property = All_Properties[action->first_property_index];
            action->properties.push_back(first_property);
            std::shared_ptr<Property> property = first_property;
            while (property->next_property_index != -1) {
                std::shared_ptr<Property> next_property = All_Properties[property->next_property_index];
                property->next_property = next_property;
                action->properties.push_back(next_property);
                property = next_property;
            }
            property->next_property = nullptr; //index is -1, no next property
        }

        for (const auto& actor : All_Actors) { //Fill each actor's list of actions before we add them to the events because they don't update both instances if we do this after
            actor->initial_action = All_Actions[actor->initial_action_index];
            actor->actions.push_back(actor->initial_action);
            std::shared_ptr<Action> action = actor->initial_action;
            while (action->next_action_index != -1) {
                std::shared_ptr<Action> next_action = All_Actions[action->next_action_index];
                action->next_action = next_action;
                actor->actions.push_back(next_action);
                action = next_action;
            }
            action->next_action = nullptr;
        }

        for (const auto& event : Events) {
            bool found_blank = false;
            for (const int32_t actor_index : event->actor_indexes) {
                if (actor_index == -1) {
                    found_blank = true;
                }
                else {
                    if (found_blank) {
                        LOG_ERR_AND_RETURN(EventlistError::NON_BLANK_ACTOR_FOLLOWING_BLANK);
                    }
                    std::shared_ptr<Actor> actor = All_Actors[actor_index];
                    event->actors.push_back(actor);
                }

            }
        }

        unused_flag_ids.reserve(TOTAL_NUM_FLAGS);
        for (int32_t i = 0; i < TOTAL_NUM_FLAGS; i++) { //Populate the list of all flags (SD randomizer uses range but this is c++ so we don't have a super good substitute)
            unused_flag_ids.push_back(i);
        }

        for (const auto& event : Events) {
            for (const auto& actor : event->actors) {
                std::erase(unused_flag_ids, actor->flag_id_to_set);
                for (const auto& action : actor->actions) {
                    std::erase(unused_flag_ids, action->flag_id_to_set);
                }
            }
        }
        return EventlistError::NONE;
    }

    EventlistError EventList::writeToStream(std::ostream& out) {
        All_Actors.clear();
        All_Actions.clear();
        All_Properties.clear();
        All_Floats.clear();
        All_Integers.clear();
        std::vector<std::string> All_Strings;

        uint32_t offset = 0x40;
        Utility::seek(out, 0x40, std::ios::beg);

        event_list_offset = offset;
        num_events = Events_By_Name.size();
        for (uint32_t i = 0; i < num_events; i++) {
            auto event = Events[i];
            event->event_index = static_cast<int32_t>(i);

            offset += Event::DATA_SIZE;

            All_Actors.insert(All_Actors.end(), event->actors.begin(), event->actors.end());
        }

        actor_list_offset = offset;
        num_actors = All_Actors.size();
        for (uint32_t i = 0; i < num_actors; i++) {
            auto actor = All_Actors[i];
            actor->actor_index = static_cast<int32_t>(i);

            offset += Actor::DATA_SIZE;

            All_Actions.reserve(actor->actions.size()); //Minimize copies
            for (unsigned int x = 0; x < actor->actions.size(); x++) {
                auto action = actor->actions[x];
                if (x == actor->actions.size() - 1) {
                    action->next_action = nullptr;
                }
                else {
                    action->next_action = actor->actions[x + 1];
                }
                All_Actions.push_back(action);
            }

        }

        action_list_offset = offset;
        num_actions = All_Actions.size();
        for (uint32_t i = 0; i < num_actions; i++) {
            auto action = All_Actions[i];
            action->action_index = static_cast<int32_t>(i);

            offset += Action::DATA_SIZE;

            for (unsigned int x = 0; x < action->properties.size(); x++) {
                auto property = action->properties[x];
                if (x == action->properties.size() - 1) {
                    property->next_property = nullptr;
                }
                else {
                    property->next_property = action->properties[x + 1];
                }
                All_Properties.push_back(property);
            }
        }

        property_list_offset = offset;
        num_properties = All_Properties.size();
        for (uint32_t i = 0; i < num_properties; i++) {
            auto property = All_Properties[i];
            property->property_index = static_cast<int32_t>(i);

            offset += Property::DATA_SIZE;

            if (const auto& property_value = property->value; property_value.index() != 3) {
                if (property_value.index() == 0) {
                    property->data_size = std::get<std::vector<float>>(property_value).size();
                    property->data_type = 0;
                    property->data_index = All_Floats.size();

                    for (const float& float_val : std::get<std::vector<float>>(property_value)) {
                        All_Floats.push_back(float_val);
                    }
                }
                else if (property_value.index() == 1) {
                    property->data_size = std::get<std::vector<vec3<float>>>(property_value).size();
                    property->data_type = 1;
                    property->data_index = All_Floats.size();

                    for (const vec3<float>& vector3 : std::get<std::vector<vec3<float>>>(property_value)) {
                        All_Floats.push_back(vector3.X);
                        All_Floats.push_back(vector3.Y);
                        All_Floats.push_back(vector3.Z);
                    }
                }
                else if (property_value.index() == 2) {
                    property->data_size = std::get<std::vector<int32_t>>(property_value).size();
                    property->data_type = 3;
                    property->data_index = All_Integers.size();

                    for (const int32_t& integer : std::get<std::vector<int32_t>>(property_value)) {
                        All_Integers.push_back(integer);
                    }
                }
                else {
                    LOG_ERR_AND_RETURN(EventlistError::UNKNOWN_PROPERTY_DATA_TYPE);
                }
            }
            else if (property_value.index() == 3) {
                property->data_type = 4;
                property->data_index = 0; //gets set later
                property->data_size = 0; //gets set later

                All_Strings.push_back(std::get<std::string>(property_value));
            }

        }

        float_list_offset = offset;
        num_floats = All_Floats.size();
        for (float& float_val : All_Floats) {
            Utility::seek(out, offset, std::ios::beg);
            uint32_t value = std::bit_cast<const uint32_t>(float_val); //byteswap as uint32 because casting it back to float in reversed order can make it NaN and change the value
            Utility::Endian::toPlatform_inplace(eType::Big, value);
            out.write(reinterpret_cast<const char*>(&value), 4);
            offset += 4;
        }

        integer_list_offset = offset;
        num_integers = All_Integers.size();
        for (int32_t& integer : All_Integers) {
            Utility::Endian::toPlatform_inplace(eType::Big, integer);
            out.write(reinterpret_cast<const char*>(&integer), 4);
            offset += 4;
        }

        string_list_offset = offset;
        for (const auto& property : All_Properties) {
            if (property->data_type == 4) {
                uint32_t new_relative_string_offset = offset - string_list_offset;

                std::string string = std::get<std::string>(property->value);
                if (string.length() % 8 != 0) {
                    unsigned int padding_bytes_needed = (8 - (string.length() % 8));
                    string.resize(string.length() + padding_bytes_needed, '\0');
                }

                out.write(&string[0], string.length()); //Write string
                offset += string.length();

                property->data_index = new_relative_string_offset;
                property->data_size = string.length();

            }
        }

        string_list_total_size = offset - string_list_offset;

        out.seekp(0, std::ios::beg);
        Utility::Endian::toPlatform_inplace(eType::Big, event_list_offset);
        Utility::Endian::toPlatform_inplace(eType::Big, num_events);
        Utility::Endian::toPlatform_inplace(eType::Big, actor_list_offset);
        Utility::Endian::toPlatform_inplace(eType::Big, num_actors);
        Utility::Endian::toPlatform_inplace(eType::Big, action_list_offset);
        Utility::Endian::toPlatform_inplace(eType::Big, num_actions);
        Utility::Endian::toPlatform_inplace(eType::Big, property_list_offset);
        Utility::Endian::toPlatform_inplace(eType::Big, num_properties);
        Utility::Endian::toPlatform_inplace(eType::Big, float_list_offset);
        Utility::Endian::toPlatform_inplace(eType::Big, num_floats);
        Utility::Endian::toPlatform_inplace(eType::Big, integer_list_offset);
        Utility::Endian::toPlatform_inplace(eType::Big, num_integers);
        Utility::Endian::toPlatform_inplace(eType::Big, string_list_offset);
        Utility::Endian::toPlatform_inplace(eType::Big, string_list_total_size);

        out.write(reinterpret_cast<const char*>(&event_list_offset), 4);
        out.write(reinterpret_cast<const char*>(&num_events), 4);
        out.write(reinterpret_cast<const char*>(&actor_list_offset), 4);
        out.write(reinterpret_cast<const char*>(&num_actors), 4);
        out.write(reinterpret_cast<const char*>(&action_list_offset), 4);
        out.write(reinterpret_cast<const char*>(&num_actions), 4);
        out.write(reinterpret_cast<const char*>(&property_list_offset), 4);
        out.write(reinterpret_cast<const char*>(&num_properties), 4);
        out.write(reinterpret_cast<const char*>(&float_list_offset), 4);
        out.write(reinterpret_cast<const char*>(&num_floats), 4);
        out.write(reinterpret_cast<const char*>(&integer_list_offset), 4);
        out.write(reinterpret_cast<const char*>(&num_integers), 4);
        out.write(reinterpret_cast<const char*>(&string_list_offset), 4);
        out.write(reinterpret_cast<const char*>(&string_list_total_size), 4);
        out.write(padding, 8);

        for (const auto& event : Events) {
            event->save_changes(out);
        }
        for (const auto& actor : All_Actors) {
            LOG_AND_RETURN_IF_ERR(actor->save_changes(out));
        }
        for (const auto& action : All_Actions) {
            action->save_changes(out);
        }
        for (const auto& property : All_Properties) {
            property->save_changes(out);
        }

        return EventlistError::NONE;
    }

    EventlistError EventList::writeToFile(const fspath& outFilePath) {
        std::ofstream outFile(outFilePath, std::ios::binary);
        if(!outFile.is_open()) {
            LOG_ERR_AND_RETURN(EventlistError::COULD_NOT_OPEN);
        }
        return writeToStream(outFile);
    }

    Event& EventList::add_event(const std::string& name) {
        std::shared_ptr<Event> event = Events.emplace_back(std::make_shared<Event>());
        event->name = name;
        Events_By_Name[name] = event;
        return *Events_By_Name[name];
    }

    std::optional<int32_t> EventList::get_unused_flag_id() const { //only possible error is EventlistError::NO_UNUSED_FLAGS_TO_USE
        if (unused_flag_ids.empty()) {
            return std::nullopt;
        }
        int32_t flag_id = unused_flag_ids[0];
        unused_flag_ids.erase(unused_flag_ids.begin(), unused_flag_ids.begin() + 1);
        return flag_id;
    }
}
