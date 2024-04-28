//Event lists store event information for in-game sequences
//Larger cutscenes have their own model archives and use .stb files

#pragma once

#include <memory>
#include <utility>
#include <vector>
#include <unordered_map>
#include <string>
#include <array>
#include <variant>
#include <optional>

#include <utility/common.hpp>
#include <filetypes/baseFiletype.hpp>


enum struct [[nodiscard]] EventlistError {
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

namespace FileTypes {
    class EventList; //forward declare
}

struct Prop {
    std::string prop_name;
    std::variant<std::vector<float>, std::vector<vec3<float>>, std::vector<int32_t>, std::string> prop_value;

    Prop() = default;

    Prop(std::string name, const std::vector<float> &val) : prop_name(std::move(name)),
                                                            prop_value(val) {
    }

    Prop(std::string name, const float &val) : prop_name(std::move(name)),
                                               prop_value(std::vector<float>{val}) {
    }

    Prop(std::string name, const std::vector<vec3<float>> &val) : prop_name(std::move(name)),
                                                                  prop_value(val) {
    }

    Prop(std::string name, const vec3<float> &val) : prop_name(std::move(name)),
                                                     prop_value(std::vector<vec3<float>>{val}) {
    }

    Prop(std::string name, const std::vector<int32_t> &val) : prop_name(std::move(name)),
                                                              prop_value(val) {
    }

    Prop(std::string name, const int32_t &val) : prop_name(std::move(name)),
                                                 prop_value(std::vector<int32_t>{val}) {
    }

    Prop(std::string name, const std::string &val) : prop_name(std::move(name)),
                                                     prop_value(val) {
    }
};

class Property {
public:
    static constexpr int DATA_SIZE = 0x40; //could be a define?

    std::string name;
    std::shared_ptr<Property> next_property; //Pointer because of initialization stuff
    std::variant<std::vector<float>, std::vector<vec3<float>>, std::vector<int>, std::string> value;

    EventlistError read(std::istream &in);

    void save_changes(std::ostream &out);

private:
    int32_t property_index;
    uint32_t data_type;
    uint32_t data_index;
    uint32_t data_size;
    int32_t next_property_index;
    char zero_initialized_runtime_data[0xC] = {"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"};

    friend class FileTypes::EventList;
    friend class Action;
    friend class Actor;
    friend class Event;
};

class Action {
public:
    static constexpr int DATA_SIZE = 0x50; //could be a define?

    std::string name;
    std::array<int32_t, 3> starting_flags = {-1, -1, -1};
    int32_t flag_id_to_set;
    std::vector<std::shared_ptr<Property>> properties;
    std::shared_ptr<Action> next_action; //Pointer because of initialization stuff
    uint32_t duplicate_id = 0;

    EventlistError read(std::istream &in);

    void save_changes(std::ostream &out);

    std::shared_ptr<Property> get_prop(const std::string &prop_name);

    Property &add_property(const std::string &name);

private:
    int32_t action_index;
    int32_t first_property_index;
    int32_t next_action_index;
    char zero_initialized_runtime_data[0x10] = {"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"};

    friend class FileTypes::EventList;
    friend class Actor;
    friend class Event;
};

class Actor {
public:
    static constexpr int DATA_SIZE = 0x50; //could be a define?

    std::string name;
    uint32_t staff_identifier = 0;
    int32_t flag_id_to_set;
    uint32_t staff_type = 0;
    std::vector<std::shared_ptr<Action>> actions;
    std::shared_ptr<Action> initial_action;

    EventlistError read(std::istream &in);

    EventlistError save_changes(std::ostream &out);

    std::shared_ptr<Action> add_action(const FileTypes::EventList &list, const std::string &name,
                                       const std::vector<Prop> &properties);

private:
    int32_t actor_index;
    int32_t initial_action_index;
    char zero_initialized_runtime_data[0x1C] = {
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    };

    friend class FileTypes::EventList;
    friend class Action;
    friend class Event;
};

class Event {
public:
    static constexpr int DATA_SIZE = 0xB0; //could be a define?

    std::string name;
    uint32_t unknown_1 = 0;
    uint32_t priority = 0;
    std::array<int32_t, 2> starting_flags = {-1, -1};
    std::array<int32_t, 3> ending_flags = {-1, -1, -1};
    bool play_jingle = false;
    std::vector<std::shared_ptr<Actor>> actors;

    EventlistError read(std::istream &in);

    void save_changes(std::ostream &out);

    std::shared_ptr<Actor> get_actor(const std::string &name);

    std::shared_ptr<Actor> add_actor(const FileTypes::EventList &list, const std::string &name);

private:
    int32_t event_index;
    std::array<int32_t, 0x14> actor_indexes = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
    };
    uint32_t num_actors = 0;
    char zero_initialized_runtime_data[0x1B] = {
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
    };

    friend class FileTypes::EventList;
    friend class Action;
    friend class Actor;
};

namespace FileTypes {
    const char *EventlistErrorGetName(EventlistError err);

    class EventList : public FileType {
    public:
        static constexpr int32_t TOTAL_NUM_FLAGS = 0x2800; //could be a define?

        std::unordered_map<std::string, std::shared_ptr<Event>> Events_By_Name;

        EventList() = default;

        static EventList createNew();

        EventlistError loadFromBinary(std::istream &in);

        EventlistError loadFromFile(const std::string &filePath);

        EventlistError writeToStream(std::ostream &out);

        EventlistError writeToFile(const std::string &outFilePath);

        Event &add_event(const std::string &name);

        std::optional<int32_t> get_unused_flag_id() const;

    private:
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

        std::vector<std::shared_ptr<Event>> Events;

        std::vector<std::shared_ptr<Actor>> All_Actors;
        std::vector<std::shared_ptr<Action>> All_Actions;
        std::vector<std::shared_ptr<Property>> All_Properties;
        std::vector<float> All_Floats;
        std::vector<int32_t> All_Integers;
        std::unordered_map<uint32_t, std::string> All_Strings_By_Offset;

        mutable std::vector<int32_t> unused_flag_ids;

        void initNew() override;
    };
}
