#pragma once

#include <string>
#include <vector>

#include "json.hpp"

class CommandHandler
{
public:
    using json = nlohmann::json;
    bool handleCommand(const std::string& command, std::string& response);
    struct Command {
        std::string name;
        std::vector<json> args; 
    };
private:
    bool getBinaryData(const std::vector<json>& args, std::string& response);
    void makeErrorJson(std::string errorMessage, std::string& response);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CommandHandler::Command, name, args);
